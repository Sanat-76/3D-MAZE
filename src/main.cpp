#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include "maze.h"

#define MAZE_SIZE 20
const float spacing = 4.0f;
const float totalSize = MAZE_SIZE * spacing;
const float mazeCenterX = totalSize / 2.0f;    // 40.0f
const float mazeCenterZ = -totalSize / 2.0f;   // -40.0f
const float mazeCenterY = 0.0f;

float yaw = -90.0f;
float pitch = -30.0f;
float lastX = 0.0f;
float lastY = 0.0f;
bool firstMouse = true;
float radius = totalSize * 1.5f;  // camera distance to see full maze

bool leftMousePressed = false;

float radians(float degrees) {
    return degrees * M_PI / 180.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Prevent divide by zero
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            leftMousePressed = true;
            firstMouse = true; // reset to avoid jump when drag starts
        }
        else if (action == GLFW_RELEASE)
        {
            leftMousePressed = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!leftMousePressed)
    {
        firstMouse = true;  // reset when not dragging
        return;
    }

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    // Flip horizontal movement for natural feel
    float xoffset = lastX - xpos;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.2f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Clamp pitch so camera doesn't flip upside down
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Maze", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // Show cursor normally
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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
        gluPerspective(60.0, aspect, 0.1, 300.0);  // larger far plane for bigger maze

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float camX = mazeCenterX + radius * cos(radians(pitch)) * cos(radians(yaw));
        float camY = mazeCenterY + radius * sin(radians(pitch));
        float camZ = mazeCenterZ + radius * cos(radians(pitch)) * sin(radians(yaw));

        gluLookAt(camX, camY, camZ, mazeCenterX, 0.0f, mazeCenterZ, 0.0f, 1.0f, 0.0f);

        drawMaze();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
