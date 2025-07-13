#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include "maze.h"

#define MAZE_SIZE 20
const float spacing = 4.0f;
const float totalSize = MAZE_SIZE * spacing;

// Camera state
float camX = spacing * 3;     // Start near entrance
float camZ = spacing * 2;     // Extra row in front of maze
float camY = 2.0f;
float yaw = -90.0f, pitch = 0.0f;
float frontX = 0.0f, frontY = 0.0f, frontZ = -1.0f;
float speedForward = 0.025f;
float speedTurn = 0.15f;

float radians(float degrees) {
    return degrees * M_PI / 180.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
}

void initOpenGL()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW.\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Maze (First-Person)", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW.\n";
        return -1;
    }

    initOpenGL();

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        glViewport(0, 0, width, height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(65.0, aspect, 0.1, 300.0);

        // Handle input
        float deltaX = 0.0f, deltaZ = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            deltaX = frontX * speedForward;
            deltaZ = frontZ * speedForward;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            deltaX = -frontX * speedForward;
            deltaZ = -frontZ * speedForward;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            yaw -= speedTurn;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            yaw += speedTurn;
        }

        // Update front vector
        frontX = cos(radians(pitch)) * cos(radians(yaw));
        frontY = 0.0f;
        frontZ = cos(radians(pitch)) * sin(radians(yaw));

        // Try to move if no collision
        if (!checkCollision(camX + deltaX, camZ + deltaZ, spacing)) {
            camX += deltaX;
            camZ += deltaZ;
        }

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float lookX = camX + frontX;
        float lookY = camY;
        float lookZ = camZ + frontZ;

        gluLookAt(camX, camY, camZ,
                  lookX, lookY, lookZ,
                  0.0f, 1.0f, 0.0f);

        drawMaze();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
