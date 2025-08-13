#include <GL/glew.h>
#include "maze.h"

int maze[MAZE_SIZE][MAZE_SIZE] = {
    // 20x20 maze example with entrance on top edge (two open blocks) and 3x3 open center
    {1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // Entrance at (0,2) and (0,3)
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,0,1,0,1,1,0,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1},
    {1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,1},

    {1,0,1,1,1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
    {1,1,1,0,1,0,1,1,0,1,1,1,0,1,1,1,0,1,0,1},

    {1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1},
    {1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},

    // Center 3x3 clear space at maze[8..10][8..10]
    {1,0,0,1,1,0,1,0,1,0,1,1,0,1,1,0,1,0,1,1}, // row 18
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1} // row 19
};

void drawCube(float x, float y, float z)
{
    float spacing = 4.0f;
    float half = spacing / 2.0f;

    glPushMatrix();
    glTranslatef(x, y, z);

    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.6f, 1.0f);

    // Front face
    glVertex3f(-half, -half, half);
    glVertex3f(half, -half, half);
    glVertex3f(half, half, half);
    glVertex3f(-half, half, half);

    // Back face
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, -half, -half);

    // Left face
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, -half, half);
    glVertex3f(-half, half, half);
    glVertex3f(-half, half, -half);

    // Right face
    glVertex3f(half, -half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, half, half);
    glVertex3f(half, -half, half);

    // Top face
    glVertex3f(-half, half, -half);
    glVertex3f(-half, half, half);
    glVertex3f(half, half, half);
    glVertex3f(half, half, -half);

    // Bottom face
    glVertex3f(-half, -half, -half);
    glVertex3f(half, -half, -half);
    glVertex3f(half, -half, half);
    glVertex3f(-half, -half, half);

    glEnd();

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
    glVertex3f(0, height, 0);
    glVertex3f(totalSize, height, 0);
    glVertex3f(totalSize, height, -totalSize);
    glVertex3f(0, height, -totalSize);

    // Bottom
    glVertex3f(0, 0, 0);
    glVertex3f(totalSize, 0, 0);
    glVertex3f(totalSize, 0, -totalSize);
    glVertex3f(0, 0, -totalSize);

    // Front
    glVertex3f(0, 0, 0);
    glVertex3f(totalSize, 0, 0);
    glVertex3f(totalSize, height, 0);
    glVertex3f(0, height, 0);

    // Back
    glVertex3f(0, 0, -totalSize);
    glVertex3f(0, height, -totalSize);
    glVertex3f(totalSize, height, -totalSize);
    glVertex3f(totalSize, 0, -totalSize);

    // Left
    glVertex3f(0, 0, -totalSize);
    glVertex3f(0, 0, 0);
    glVertex3f(0, height, 0);
    glVertex3f(0, height, -totalSize);

    // Right
    glVertex3f(totalSize, 0, -totalSize);
    glVertex3f(totalSize, height, -totalSize);
    glVertex3f(totalSize, height, 0);
    glVertex3f(totalSize, 0, 0);

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
            {
                drawCube(j * spacing + half, half, -i * spacing - half);
            }
        }
    }

    drawBasePlatform(totalSize);
}


