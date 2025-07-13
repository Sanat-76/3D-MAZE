#include <GL/glew.h>
#include "maze.h"

int maze[MAZE_SIZE][MAZE_SIZE] = {
    // Entrance at (0,2) and (0,3)
    {1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1},

    {1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1},
    {1,1,1,1,1,1,1,1,0,0,1,0,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1},
    
    {1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,1,1,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
    {1,1,1,0,1,0,1,1,0,1,1,1,0,1,1,1,0,1,0,1},

    {1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1},
    {1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,1,0,1,0,1,1,0,1,1,0,1,0,1,1},
    {1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1}  // Exit at (19,10)
};


void drawCube(float x, float y, float z)
{
    float spacing = 4.0f;
    float half = spacing / 2.0f;

    glPushMatrix();
    glTranslatef(x, y, z);

    // Solid cube
    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.6f, 1.0f);

    // Front
    glVertex3f(-half, -half, half);
    glVertex3f(half, -half, half);
    glVertex3f(half, half, half);
    glVertex3f(-half, half, half);

    // Back
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, -half, -half);

    // Left
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, -half, half);
    glVertex3f(-half, half, half);
    glVertex3f(-half, half, -half);

    // Right
    glVertex3f(half, -half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, half, half);
    glVertex3f(half, -half, half);

    // Top
    glVertex3f(-half, half, -half);
    glVertex3f(-half, half, half);
    glVertex3f(half, half, half);
    glVertex3f(half, half, -half);

    // Bottom
    glVertex3f(-half, -half, -half);
    glVertex3f(half, -half, -half);
    glVertex3f(half, -half, half);
    glVertex3f(-half, -half, half);
    glEnd();

    // Wireframe edges
    glColor3f(0, 0, 0);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP); glVertex3f(-half, -half, half); glVertex3f(half, -half, half); glVertex3f(half, half, half); glVertex3f(-half, half, half); glEnd(); // Front
    glBegin(GL_LINE_LOOP); glVertex3f(-half, -half, -half); glVertex3f(-half, half, -half); glVertex3f(half, half, -half); glVertex3f(half, -half, -half); glEnd(); // Back
    glBegin(GL_LINE_LOOP); glVertex3f(-half, -half, -half); glVertex3f(-half, -half, half); glVertex3f(-half, half, half); glVertex3f(-half, half, -half); glEnd(); // Left
    glBegin(GL_LINE_LOOP); glVertex3f(half, -half, -half); glVertex3f(half, half, -half); glVertex3f(half, half, half); glVertex3f(half, -half, half); glEnd(); // Right
    glBegin(GL_LINE_LOOP); glVertex3f(-half, half, -half); glVertex3f(-half, half, half); glVertex3f(half, half, half); glVertex3f(half, half, -half); glEnd(); // Top
    glBegin(GL_LINE_LOOP); glVertex3f(-half, -half, -half); glVertex3f(half, -half, -half); glVertex3f(half, -half, half); glVertex3f(-half, -half, half); glEnd(); // Bottom

    glPopMatrix();
}

void drawBasePlatform(float totalSize)
{
    float height = 1.0f;
    float y = -height;

    glPushMatrix();
    glTranslatef(0.0f, y, 0.0f);

    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f);

    // Top
    glVertex3f(-4.0f, height, 4.0f);                     // Add extra margin
    glVertex3f(totalSize + 4.0f, height, 4.0f);
    glVertex3f(totalSize + 4.0f, height, -totalSize - 4.0f);
    glVertex3f(-4.0f, height, -totalSize - 4.0f);
    glEnd();

    glPopMatrix();
}

void drawMaze()
{
    float spacing = 4.0f;
    float half = spacing / 2.0f;
    float totalSize = MAZE_SIZE * spacing;

    for (int i = 0; i < MAZE_SIZE; ++i)
    {
        for (int j = 0; j < MAZE_SIZE; ++j)
        {
            if (maze[i][j] == 1)
                drawCube(j * spacing + half, half, -i * spacing - half);
        }
    }

    drawBasePlatform(totalSize);
}

bool checkCollision(float x, float z, float spacing)
{
    int col = static_cast<int>(x / spacing);
    int row = static_cast<int>(-z / spacing);

    if (row < 0 || row >= MAZE_SIZE || col < 0 || col >= MAZE_SIZE)
        return false;  // outside maze is allowed

    return maze[row][col] == 1;  // collision if wall
}
