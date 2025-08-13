#include <GL/glut.h>
#include <cmath>
#include <set>
#include <chrono>
#include <stdio.h>

// Maze size
const int WIDTH = 10, HEIGHT = 6;

// Maze cell structure
struct Cell {
    bool north, south, east, west;
};
Cell maze[WIDTH][HEIGHT];

// Camera parameters
float cameraX = 10.0f, cameraY = 25.0f, cameraZ = 6.0f; // Start high above center of maze
float cameraYaw = 0.0f;
float cameraPitch = -70.0f; // Looking down initially
float moveSpeed = 3.0f;
float turnSpeed = 90.0f;

// Animation state
bool isTransitioning = true;
float transitionTime = 0.0f;
const float TRANSITION_DURATION = 5.0f; // 5 seconds transition

// Target positions for transition
float startX = 10.0f, startY = 25.0f, startZ = 6.0f; // High above maze center
float startYaw = 0.0f, startPitch = -70.0f;
float endX = -4.0f, endY = 1.0f, endZ = 6.0f; // Further back from entrance
float endYaw = 0.0f, endPitch = 0.0f;

// Input state
std::set<unsigned char> pressedKeys;

// Timing
auto lastTime = std::chrono::high_resolution_clock::now();

// Smooth interpolation function with more dramatic easing
float smoothstep(float t) {
    // More dramatic easing curve
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

// Draw a proper cube wall at specific position
void drawWallCube(float x, float y, float z, float width, float height, float depth) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(width, height, depth);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// Draw wall shadows (darker quads on the ground)
void drawWallShadows() {
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f); // Semi-transparent black
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float shadowOffset = 0.01f; // Slightly above ground to avoid Z-fighting
    float cellSize = 2.0f;
    float shadowWidth = 0.5f; // Shadow width
    
    for (int i = 0; i < WIDTH; ++i) {
        for (int j = 0; j < HEIGHT; ++j) {
            float cellX = i * cellSize;
            float cellZ = j * cellSize;
            
            // North wall shadow
            if (maze[i][j].north) {
                glBegin(GL_QUADS);
                    glVertex3f(cellX, shadowOffset, cellZ + shadowWidth);
                    glVertex3f(cellX + cellSize, shadowOffset, cellZ + shadowWidth);
                    glVertex3f(cellX + cellSize, shadowOffset, cellZ);
                    glVertex3f(cellX, shadowOffset, cellZ);
                glEnd();
            }
            
            // South wall shadow
            if (maze[i][j].south) {
                glBegin(GL_QUADS);
                    glVertex3f(cellX, shadowOffset, cellZ + cellSize);
                    glVertex3f(cellX + cellSize, shadowOffset, cellZ + cellSize);
                    glVertex3f(cellX + cellSize, shadowOffset, cellZ + cellSize - shadowWidth);
                    glVertex3f(cellX, shadowOffset, cellZ + cellSize - shadowWidth);
                glEnd();
            }
            
            // West wall shadow
            if (maze[i][j].west) {
                glBegin(GL_QUADS);
                    glVertex3f(cellX, shadowOffset, cellZ);
                    glVertex3f(cellX + shadowWidth, shadowOffset, cellZ);
                    glVertex3f(cellX + shadowWidth, shadowOffset, cellZ + cellSize);
                    glVertex3f(cellX, shadowOffset, cellZ + cellSize);
                glEnd();
            }
            
            // East wall shadow
            if (maze[i][j].east) {
                glBegin(GL_QUADS);
                    glVertex3f(cellX + cellSize - shadowWidth, shadowOffset, cellZ);
                    glVertex3f(cellX + cellSize, shadowOffset, cellZ);
                    glVertex3f(cellX + cellSize, shadowOffset, cellZ + cellSize);
                    glVertex3f(cellX + cellSize - shadowWidth, shadowOffset, cellZ + cellSize);
                glEnd();
            }
        }
    }
    
    glDisable(GL_BLEND);
}

// Draw walls using properly positioned cubes
void drawMazeWalls() {
    glColor3f(0.7f, 0.5f, 0.3f); // Lighter brown wall color
    
    float wallHeight = 2.0f;
    float wallThickness = 0.1f;
    float cellSize = 2.0f;
    
    for (int i = 0; i < WIDTH; ++i) {
        for (int j = 0; j < HEIGHT; ++j) {
            float cellX = i * cellSize;
            float cellZ = j * cellSize;
            
            // North wall (top of cell)
            if (maze[i][j].north) {
                drawWallCube(cellX + cellSize/2.0f, wallHeight/2.0f, cellZ, 
                           cellSize + wallThickness, wallHeight, wallThickness);
            }
            
            // South wall (bottom of cell)
            if (maze[i][j].south) {
                drawWallCube(cellX + cellSize/2.0f, wallHeight/2.0f, cellZ + cellSize, 
                           cellSize + wallThickness, wallHeight, wallThickness);
            }
            
            // West wall (left of cell)
            if (maze[i][j].west) {
                drawWallCube(cellX, wallHeight/2.0f, cellZ + cellSize/2.0f, 
                           wallThickness, wallHeight, cellSize + wallThickness);
            }
            
            // East wall (right of cell)
            if (maze[i][j].east) {
                drawWallCube(cellX + cellSize, wallHeight/2.0f, cellZ + cellSize/2.0f, 
                           wallThickness, wallHeight, cellSize + wallThickness);
            }
        }
    }
}

// Draw corner posts to clean up wall intersections
void drawCornerPosts() {
    glColor3f(0.6f, 0.4f, 0.2f); // Darker brown for posts
    
    float wallHeight = 2.0f;
    float postSize = 0.1f;
    float cellSize = 2.0f;
    
    // Draw posts at all grid intersections
    for (int i = 0; i <= WIDTH; ++i) {
        for (int j = 0; j <= HEIGHT; ++j) {
            float x = i * cellSize;
            float z = j * cellSize;
            drawWallCube(x, wallHeight/2.0f, z, postSize, wallHeight, postSize);
        }
    }
}

// Draw entrance area with more details
void drawEntrance() {
    // Entrance marker - exactly where player starts
    glColor3f(0.0f, 1.0f, 0.0f); // Green for entrance
    glPushMatrix();
    glTranslatef(-0.3f, 0.1f, 6.0f);
    glScalef(0.2f, 0.2f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Entrance door frame
    glColor3f(0.4f, 0.2f, 0.1f); // Dark brown for door frame
    glPushMatrix();
    glTranslatef(-0.1f, 1.0f, 6.0f);
    glScalef(0.2f, 2.0f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Door frame sides
    glPushMatrix();
    glTranslatef(-0.1f, 1.0f, 5.0f);
    glScalef(0.2f, 2.0f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-0.1f, 1.0f, 7.0f);
    glScalef(0.2f, 2.0f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Door frame top
    glPushMatrix();
    glTranslatef(-0.1f, 2.0f, 6.0f);
    glScalef(0.2f, 0.1f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Welcome mat
    glColor3f(0.4f, 0.2f, 0.1f); // Brown mat
    glPushMatrix();
    glTranslatef(-0.5f, 0.02f, 6.0f);
    glScalef(1.0f, 0.05f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// Render the complete maze
void drawMaze() {
    // Extended ground area - larger for better aerial view
    glColor3f(0.2f, 0.6f, 0.2f); // Green ground
    glBegin(GL_QUADS);
        glVertex3f(-10.0f, 0.0f, -10.0f);
        glVertex3f(WIDTH*2.0f+10.0f, 0.0f, -10.0f);
        glVertex3f(WIDTH*2.0f+10.0f, 0.0f, HEIGHT*2.0f+10.0f);
        glVertex3f(-10.0f, 0.0f, HEIGHT*2.0f+10.0f);
    glEnd();

    // Maze floor - darker green
    glColor3f(0.15f, 0.4f, 0.15f);
    glBegin(GL_QUADS);
        glVertex3f(0.0f, 0.005f, 0.0f);
        glVertex3f(WIDTH*2.0f, 0.005f, 0.0f);
        glVertex3f(WIDTH*2.0f, 0.005f, HEIGHT*2.0f);
        glVertex3f(0.0f, 0.005f, HEIGHT*2.0f);
    glEnd();
    
    // Draw wall shadows first (so they appear under walls)
    drawWallShadows();
    
    // Draw walls and corner posts
    drawMazeWalls();
    drawCornerPosts();
    
    // Draw entrance with more detail
    drawEntrance();
    
    // Draw exit marker
    glColor3f(1.0f, 0.0f, 0.0f); // Red for exit
    glPushMatrix();
    glTranslatef(WIDTH*2.0f + 0.3f, 0.1f, (HEIGHT-1)*2.0f + 1.0f);
    glScalef(0.2f, 0.2f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Add some trees around the perimeter for atmosphere
    glColor3f(0.0f, 0.5f, 0.0f); // Dark green for trees
    for (int i = 0; i < 8; ++i) {
        float angle = i * 45.0f * 3.14159f / 180.0f;
        float radius = 15.0f;
        float x = WIDTH + cos(angle) * radius;
        float z = HEIGHT + sin(angle) * radius;
        
        glPushMatrix();
        glTranslatef(x, 1.5f, z);
        glScalef(0.5f, 3.0f, 0.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}

// Update camera position based on input
void updateMovement(float deltaTime) {
    if (isTransitioning) return; // Don't allow movement during transition
    
    float rad = cameraYaw * 3.14159f / 180.0f;
    float dirX = cos(rad), dirZ = sin(rad);
    float moveDistance = moveSpeed * deltaTime;
    float turnAmount = turnSpeed * deltaTime;
    
    // Movement
    if (pressedKeys.count('w') || pressedKeys.count('W')) {
        cameraX += moveDistance * dirX;
        cameraZ += moveDistance * dirZ;
    }
    if (pressedKeys.count('s') || pressedKeys.count('S')) {
        cameraX -= moveDistance * dirX;
        cameraZ -= moveDistance * dirZ;
    }
    if (pressedKeys.count('a') || pressedKeys.count('A')) {
        cameraX += moveDistance * dirZ;
        cameraZ -= moveDistance * dirX;
    }
    if (pressedKeys.count('d') || pressedKeys.count('D')) {
        cameraX -= moveDistance * dirZ;
        cameraZ += moveDistance * dirX;
    }
    
    // Rotation
    if (pressedKeys.count('q') || pressedKeys.count('Q')) {
        cameraYaw -= turnAmount;
    }
    if (pressedKeys.count('e') || pressedKeys.count('E')) {
        cameraYaw += turnAmount;
    }
    
    // Reset position to entrance
    if (pressedKeys.count('r') || pressedKeys.count('R')) {
        cameraX = -4.0f;
        cameraY = 1.0f;
        cameraZ = 6.0f;
        cameraYaw = 0.0f;
        cameraPitch = 0.0f;
        printf("Reset to entrance position\n");
    }
    
    // Keep yaw in range [0, 360)
    while (cameraYaw < 0) cameraYaw += 360.0f;
    while (cameraYaw >= 360.0f) cameraYaw -= 360.0f;
}

// Update transition animation
void updateTransition(float deltaTime) {
    if (!isTransitioning) return;
    
    transitionTime += deltaTime;
    float t = transitionTime / TRANSITION_DURATION;
    
    if (t >= 1.0f) {
        // Transition complete
        isTransitioning = false;
        cameraX = endX;
        cameraY = endY;
        cameraZ = endZ;
        cameraYaw = endYaw;
        cameraPitch = endPitch;
        printf("Transition complete! You can now move with WASD.\n");
        printf("Walk forward to enter the maze through the green entrance!\n");
    } else {
        // Smooth interpolation with dramatic easing
        float smoothT = smoothstep(t);
        cameraX = startX + (endX - startX) * smoothT;
        cameraY = startY + (endY - startY) * smoothT;
        cameraZ = startZ + (endZ - startZ) * smoothT;
        cameraYaw = startYaw + (endYaw - startYaw) * smoothT;
        cameraPitch = startPitch + (endPitch - startPitch) * smoothT;
    }
}

// Camera setup
void updateCamera() {
    float rad = cameraYaw * 3.14159f / 180.0f;
    float pitchRad = cameraPitch * 3.14159f / 180.0f;
    
    float dirX = cos(rad) * cos(pitchRad);
    float dirY = sin(pitchRad);
    float dirZ = sin(rad) * cos(pitchRad);
    
    gluLookAt(cameraX, cameraY, cameraZ,
              cameraX + dirX, cameraY + dirY, cameraZ + dirZ,
              0.0f, 1.0f, 0.0f);
}

// GLUT display callback
void display() {
    // Calculate delta time
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime);
    float deltaTime = duration.count() / 1000000.0f;
    lastTime = currentTime;
    
    // Update transition and movement
    updateTransition(deltaTime);
    updateMovement(deltaTime);
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    updateCamera();
    drawMaze();
    
    glutSwapBuffers();
}

// Keyboard down event
void keyboardDown(unsigned char key, int x, int y) {
    pressedKeys.insert(key);
    if (key == 27) exit(0); // ESC
    
    // Skip transition
    if (key == ' ' && isTransitioning) {
        isTransitioning = false;
        cameraX = endX;
        cameraY = endY;
        cameraZ = endZ;
        cameraYaw = endYaw;
        cameraPitch = endPitch;
        printf("Transition skipped!\n");
    }
}

// Keyboard up event
void keyboardUp(unsigned char key, int x, int y) {
    pressedKeys.erase(key);
}

// Idle function to continuously update
void idle() {
    glutPostRedisplay();
}

// Create a simple but solvable maze
void initMaze() {
    // Initialize all cells with all walls
    for (int i = 0; i < WIDTH; ++i)
        for (int j = 0; j < HEIGHT; ++j)
            maze[i][j] = {true, true, true, true};

    // Create entrance in middle of left wall (cell 0,3) - this is at Z=6 in world coordinates
    maze[0][3].west = false;
    
    // Create exit at bottom right (cell 9,5)
    maze[WIDTH-1][HEIGHT-1].east = false;
    
    // Create main solution path
    // From entrance (0,3) go right
    maze[0][3].east = false; maze[1][3].west = false;
    maze[1][3].east = false; maze[2][3].west = false;
    maze[2][3].east = false; maze[3][3].west = false;
    maze[3][3].east = false; maze[4][3].west = false;
    maze[4][3].east = false; maze[5][3].west = false;
    maze[5][3].east = false; maze[6][3].west = false;
    maze[6][3].east = false; maze[7][3].west = false;
    
    // Go up from (7,3) to (7,1)
    maze[7][3].north = false; maze[7][2].south = false;
    maze[7][2].north = false; maze[7][1].south = false;
    
    // Go right from (7,1) to (9,1)
    maze[7][1].east = false; maze[8][1].west = false;
    maze[8][1].east = false; maze[9][1].west = false;
    
    // Go down from (9,1) to (9,5)
    maze[9][1].south = false; maze[9][2].north = false;
    maze[9][2].south = false; maze[9][3].north = false;
    maze[9][3].south = false; maze[9][4].north = false;
    maze[9][4].south = false; maze[9][5].north = false;
    
    // Add some dead ends and alternate paths
    maze[0][3].south = false; maze[0][4].north = false;
    maze[0][4].south = false; maze[0][5].north = false;
    maze[0][5].east = false; maze[1][5].west = false;
    
    maze[2][3].south = false; maze[2][4].north = false;
    maze[2][4].south = false; maze[2][5].north = false;
    
    maze[4][3].south = false; maze[4][4].north = false;
    maze[4][4].south = false; maze[4][5].north = false;
    maze[4][5].east = false; maze[5][5].west = false;
    maze[5][5].east = false; maze[6][5].west = false;
    maze[6][5].east = false; maze[7][5].west = false;
    maze[7][5].east = false; maze[8][5].west = false;
    maze[8][5].east = false; maze[9][5].west = false;
    
    // Add some top row connections
    maze[0][0].east = false; maze[1][0].west = false;
    maze[1][0].east = false; maze[2][0].west = false;
    maze[2][0].east = false; maze[3][0].west = false;
    maze[3][0].south = false; maze[3][1].north = false;
    maze[3][1].south = false; maze[3][2].north = false;
    maze[3][2].east = false; maze[4][2].west = false;
    maze[4][2].east = false; maze[5][2].west = false;
    maze[5][2].east = false; maze[6][2].west = false;
    maze[6][2].north = false; maze[6][1].south = false;
    maze[6][1].north = false; maze[6][0].south = false;
    maze[6][0].east = false; maze[7][0].west = false;
    maze[7][0].east = false; maze[8][0].west = false;
    maze[8][0].east = false; maze[9][0].west = false;
    maze[9][0].south = false; maze[9][1].north = false;
}

// Window resize
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)width / (double)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// OpenGL/GLUT setup
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Set up light for better shadows
    GLfloat light_pos[] = {10.0f, 30.0f, 10.0f, 1.0f}; // Higher light for aerial view
    GLfloat light_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat light_diffuse[] = {0.9f, 0.9f, 0.9f, 1.0f};
    GLfloat light_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    
    // Material properties
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Smooth shading
    glShadeModel(GL_SMOOTH);
    
    // Enable alpha blending for shadows
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(900, 700);
    glutCreateWindow("3D Maze - Cinematic Transition");

    initGL();
    initMaze();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutIdleFunc(idle);

    printf("\t3D Maze with Cinematic Transition\n");
    printf("\tWatch the dramatic descent from aerial view to ground level!\n");
    printf("\tControls (after transition):\n");
    printf("WASD - Move\n");
    printf("Q/E - Turn left/right\n");
    printf("R - Reset to entrance\n");
    printf("SPACE - Skip transition\n");
    printf("ESC - Exit\n");
    printf("Starting cinematic transition...\n");

    glutMainLoop();
    return 0;
}