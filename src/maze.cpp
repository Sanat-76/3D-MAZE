#include "maze.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

unsigned int VAO, VBO;
unsigned int floorVAO, floorVBO;
extern const float spacing = 4.0f;

int maze[MAZE_SIZE][MAZE_SIZE] = {
    {1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1},
    {1,1,1,1,0,0,1,1,0,0,1,0,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,0,0,1,1,0,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,1,1,0,0,0,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
    {1,1,1,0,1,0,1,0,0,1,1,1,0,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1},
    {1,0,1,1,1,1,1,0,0,1,0,1,1,1,0,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,1,0,0,0,0,1,0,1,1,0,1,0,1,1},
    {1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1}
};

float vertices[] = {
    // positions         // normals
    -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
     0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
     0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
     0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
    -0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
    -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,

    -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
     0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
     0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
     0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
    -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,

    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f,-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

     0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
     0.5f, 0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
     0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
     0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
     0.5f,-0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
     0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,

    -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
     0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
     0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
     0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
    -0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
    -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,

    -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
     0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f
};

void initMaze() {
    // Wall VAO/VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Floor plane
    float floorVertices[] = {
        // positions          // normals
        -2.0f, 0.0f,  2.0f,   0.0f, 1.0f, 0.0f,
         2.0f, 0.0f,  2.0f,   0.0f, 1.0f, 0.0f,
         2.0f, 0.0f, -2.0f,   0.0f, 1.0f, 0.0f,
         2.0f, 0.0f, -2.0f,   0.0f, 1.0f, 0.0f,
        -2.0f, 0.0f, -2.0f,   0.0f, 1.0f, 0.0f,
        -2.0f, 0.0f,  2.0f,   0.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void drawMaze(unsigned int shaderProgram) {
    // Draw walls
    glBindVertexArray(VAO);
    for (int i = 0; i < MAZE_SIZE; ++i) {
        for (int j = 0; j < MAZE_SIZE; ++j) {
            if (maze[i][j] == 1) {
                glm::mat4 model = glm::mat4(1.0f);
                float x = j * spacing + spacing / 2.0f;
                float z = -i * spacing - spacing / 2.0f;
                model = glm::translate(model, glm::vec3(x, (spacing * 10.0f) / 2.0f, z));
                model = glm::scale(model, glm::vec3(spacing, spacing * 10.0f, spacing));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
    }

    // Draw floor
    glBindVertexArray(floorVAO);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(MAZE_SIZE * spacing / 2.0f, 0.0f, -MAZE_SIZE * spacing / 2.0f));
    model = glm::scale(model, glm::vec3(MAZE_SIZE * spacing + 8.0f, 1.0f, MAZE_SIZE * spacing + 8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool checkCollision(float x, float z, float spacing) {
    int col = static_cast<int>(x / spacing);
    int row = static_cast<int>(-z / spacing);
    if (row < 0 || row >= MAZE_SIZE || col < 0 || col >= MAZE_SIZE)
        return false;
    return maze[row][col] == 1;
}