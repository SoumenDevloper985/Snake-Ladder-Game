#define GL_SILENCE_DEPRECATION
#include <GL/glut.h> 
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int BOARD_SIZE = 10;
const float CELL_SIZE = 60.0f;
const int NUM_SNAKES = 7;
const int NUM_LADDERS = 5;

struct Position {
    int x, y;
};

struct Snake {
    int head;
    int tail;
    std::vector<Position> path;
};

struct Ladder {
    int bottom;
    int top;
    std::vector<Position> path;
};

struct Player {
    int position;
    float r, g, b;
    std::string name;
};

std::vector<Snake> snakes;
std::vector<Ladder> ladders;
std::vector<Player> players;
int currentPlayer = 0;
int diceValue = 0;
bool diceRolled = false;
bool gameOver = false;

void drawText(float x, float y, std::string text) {
    glRasterPos2f(x, y);
    for(char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

Position getBoardCoordinates(int position) {
    position--; // Convert to 0-based index
    int row = 9 - (position / 10);
    int col = (row % 2 == 0) ? (position % 10) : (9 - (position % 10));
    return {col, row};
}

int getBoardPosition(int x, int y) {
    int position = (9 - y) * 10;
    position += (y % 2 == 0) ? x : (9 - x);
    return position + 1;
}

void drawSnakes() {
    for(const auto& snake : snakes) {
        Position headPos = getBoardCoordinates(snake.head);
        Position tailPos = getBoardCoordinates(snake.tail);
        
        float startX = headPos.x * CELL_SIZE + 100 + CELL_SIZE/2;
        float startY = headPos.y * CELL_SIZE + 100 + CELL_SIZE/2;
        float endX = tailPos.x * CELL_SIZE + 100 + CELL_SIZE/2;
        float endY = tailPos.y * CELL_SIZE + 100 + CELL_SIZE/2;
        
        // Draw snake body with curve
        glColor3f(0.5f, 0.0f, 0.0f); // Dark red for snake body
        glBegin(GL_LINE_STRIP);
        for(float t = 0; t <= 1; t += 0.05) {
            float x = startX;
            float y = startY;
            
            float amplitude = CELL_SIZE * 0.5;
            float frequency = 4 * M_PI;
            
            float dx = endX - startX;
            float dy = endY - startY;
            float length = sqrt(dx*dx + dy*dy);
            
            float nx = -dy / length;
            float ny = dx / length;
            
            x = startX + t * dx + sin(t * frequency) * amplitude * nx;
            y = startY + t * dy + sin(t * frequency) * amplitude * ny;
            
            glVertex2f(x, y);
        }
        glEnd();
        
        // Draw snake head
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
        float headRadius = CELL_SIZE/4;
        for(int i = 0; i < 360; i += 10) {
            float angle = i * M_PI / 180.0f;
            glVertex2f(startX + cos(angle) * headRadius,
                      startY + sin(angle) * headRadius);
        }
        glEnd();
        
        // Draw snake tail
        glColor3f(0.7f, 0.0f, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
        float tailRadius = CELL_SIZE/6;
        for(int i = 0; i < 360; i += 10) {
            float angle = i * M_PI / 180.0f;
            glVertex2f(endX + cos(angle) * tailRadius,
                      endY + sin(angle) * tailRadius);
        }
        glEnd();
    }
}

void drawLadders() {
    for(const auto& ladder : ladders) {
        Position bottomPos = getBoardCoordinates(ladder.bottom);
        Position topPos = getBoardCoordinates(ladder.top);
        
        float startX = bottomPos.x * CELL_SIZE + 100 + CELL_SIZE/2;
        float startY = bottomPos.y * CELL_SIZE + 100 + CELL_SIZE/2;
        float endX = topPos.x * CELL_SIZE + 100 + CELL_SIZE/2;
        float endY = topPos.y * CELL_SIZE + 100 + CELL_SIZE/2;
        
        float dx = endX - startX;
        float dy = endY - startY;
        float length = sqrt(dx*dx + dy*dy);
        float angle = atan2(dy, dx);
        
        float ladderWidth = CELL_SIZE * 0.3;
        
        glColor3f(0.7f, 0.5f, 0.0f);
        glLineWidth(3.0);
        
        // Left rail
        glBegin(GL_LINE_STRIP);
        float leftX = -ladderWidth/2;
        glVertex2f(startX + leftX * cos(angle + M_PI/2),
                  startY + leftX * sin(angle + M_PI/2));
        glVertex2f(endX + leftX * cos(angle + M_PI/2),
                  endY + leftX * sin(angle + M_PI/2));
        glEnd();
        
        // Right rail
        glBegin(GL_LINE_STRIP);
        float rightX = ladderWidth/2;
        glVertex2f(startX + rightX * cos(angle + M_PI/2),
                  startY + rightX * sin(angle + M_PI/2));
        glVertex2f(endX + rightX * cos(angle + M_PI/2),
                  endY + rightX * sin(angle + M_PI/2));
        glEnd();
        
        // Draw rungs
        int numRungs = length / (CELL_SIZE * 0.5);
        for(int i = 0; i <= numRungs; i++) {
            float t = i / float(numRungs);
            float x = startX + t * dx;
            float y = startY + t * dy;
            
            glBegin(GL_LINES);
            glVertex2f(x + leftX * cos(angle + M_PI/2),
                      y + leftX * sin(angle + M_PI/2));
            glVertex2f(x + rightX * cos(angle + M_PI/2),
                      y + rightX * sin(angle + M_PI/2));
            glEnd();
        }
        
        glLineWidth(1.0);
    }
}

void drawBoard() {
    // Draw grid
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            float x = j * CELL_SIZE + 100;
            float y = i * CELL_SIZE + 100;
            
            // Alternate cell colors
            if((i + j) % 2 == 0) {
                glColor3f(0.9f, 0.9f, 0.8f);
            } else {
                glColor3f(0.8f, 0.8f, 0.7f);
            }
            
            // Draw cell
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + CELL_SIZE, y);
            glVertex2f(x + CELL_SIZE, y + CELL_SIZE);
            glVertex2f(x, y + CELL_SIZE);
            glEnd();
            
            // Draw cell border
            glColor3f(0.6f, 0.6f, 0.6f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + CELL_SIZE, y);
            glVertex2f(x + CELL_SIZE, y + CELL_SIZE);
            glVertex2f(x, y + CELL_SIZE);
            glEnd();
            
            // Draw cell number
            glColor3f(0.0f, 0.0f, 0.0f);
            int number = getBoardPosition(j, i);
            drawText(x + 5, y + 5, std::to_string(number));
        }
    }
    
    drawLadders();
    drawSnakes();
    
    // Draw players
    for(const auto& player : players) {
        Position pos = getBoardCoordinates(player.position);
        glColor3f(player.r, player.g, player.b);
        glBegin(GL_TRIANGLE_FAN);
        float centerX = pos.x * CELL_SIZE + 100 + CELL_SIZE/2;
        float centerY = pos.y * CELL_SIZE + 100 + CELL_SIZE/2;
        float radius = CELL_SIZE/4;
        for(int i = 0; i < 360; i += 10) {
            float angle = i * M_PI / 180.0f;
            glVertex2f(centerX + cos(angle) * radius,
                      centerY + sin(angle) * radius);
        }
        glEnd();
    }
}

void initializeSnakesAndLadders() {
    // Initialize snakes
    int snakeHeads[NUM_SNAKES] = {98, 95, 92, 83, 73, 69, 64};
    int snakeTails[NUM_SNAKES] = {28, 24, 51, 19, 1, 33, 36};
    
    for(int i = 0; i < NUM_SNAKES; i++) {
        Snake snake;
        snake.head = snakeHeads[i];
        snake.tail = snakeTails[i];
        snakes.push_back(snake);
    }
    
    // Initialize ladders
    int ladderBottoms[NUM_LADDERS] = {2, 7, 22, 42, 51};
    int ladderTops[NUM_LADDERS] = {38, 14, 79, 99, 67};
    
    for(int i = 0; i < NUM_LADDERS; i++) {
        Ladder ladder;
        ladder.bottom = ladderBottoms[i];
        ladder.top = ladderTops[i];
        ladders.push_back(ladder);
    }
}

void initializePlayers() {
    Player p1 = {1, 1.0f, 0.0f, 0.0f, "Player 1"}; // Red
    Player p2 = {1, 0.0f, 0.0f, 1.0f, "Player 2"}; // Blue
    players.push_back(p1);
    players.push_back(p2);
}

void checkSnakesAndLadders() {
    for(const auto& snake : snakes) {
        if(players[currentPlayer].position == snake.head) {
            players[currentPlayer].position = snake.tail;
            return;
        }
    }
    
    for(const auto& ladder : ladders) {
        if(players[currentPlayer].position == ladder.bottom) {
            players[currentPlayer].position = ladder.top;
            return;
        }
    }
}

void rollDice() {
    if(!diceRolled && !gameOver) {
        diceValue = rand() % 6 + 1;
        int newPosition = players[currentPlayer].position + diceValue;
        
        if(newPosition <= 100) {
            players[currentPlayer].position = newPosition;
            checkSnakesAndLadders();
            
            if(players[currentPlayer].position == 100) {
                gameOver = true;
            }
        }
        
        diceRolled = true;
    }
}

void nextTurn() {
    if(diceRolled && !gameOver) {
        currentPlayer = (currentPlayer + 1) % players.size();
        diceRolled = false;
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    
    drawBoard();
    
    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(20, WINDOW_HEIGHT - 30, players[currentPlayer].name + "'s Turn");
    
    if(diceRolled) {
        drawText(20, WINDOW_HEIGHT - 60, "Dice: " + std::to_string(diceValue));
        drawText(20, WINDOW_HEIGHT - 90, "Press SPACE for next turn");
    } else {
        drawText(20, WINDOW_HEIGHT - 60, "Press ENTER to roll dice");
    }
    
    if(gameOver) {
        std::string winMessage = players[currentPlayer].name + " Wins!";
        drawText(WINDOW_WIDTH/2 - 50, WINDOW_HEIGHT/2, winMessage);
        drawText(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 30, "Press 'R' to restart");
    }
    
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    if(key == 13) { // Enter key
        rollDice();
    }
    else if(key == 32) { // Space key
        nextTurn();
    }
    else if((key == 'r' || key == 'R') && gameOver) {
        for(auto& player : players) {
            player.position = 1;
        }
        currentPlayer = 0;
        diceRolled = false;
        gameOver = false;
    }
    glutPostRedisplay();
}

void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    
    srand(time(nullptr));
    initializeSnakesAndLadders();
    initializePlayers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Snakes and Ladders");
    
    init();
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    
    glutMainLoop();
    return 0;
}

