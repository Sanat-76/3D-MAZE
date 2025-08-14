// main.cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "maze.h"

#define MAZE_SIZE 20
const float spacing = 4.0f;

// Camera variables
float camX = spacing * 3;
float camZ = spacing * 2;
float camY = 4.0f;
float yaw = -90.0f, pitch = 0.0f;
float frontX = 0.0f, frontY = 0.0f, frontZ = -1.0f;

// Adjusted speeds for full-screen feel
float speedForward = 8.0f;  // units per second
float speedTurn = 180.0f;   // degrees per second

// Timing variables
float deltaTime = 0.0f;  // Time between current frame and last frame
float lastFrame = 0.0f;

// Function declarations
GLuint loadShader(const char* vertexPath, const char* fragmentPath);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Maze - Phong", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Callback for window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    GLuint shaderProgram = loadShader("shader.vert", "shader.frag");
    initMaze();

    while (!glfwWindowShouldClose(window)) {
        // Calculate deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process keyboard input
        processInput(window);

        // Clear buffers
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera and projection matrices
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glm::mat4 view = glm::lookAt(glm::vec3(camX, camY, camZ),
                                     glm::vec3(camX + frontX, camY, camZ + frontZ),
                                     glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projection = glm::perspective(glm::radians(65.0f),
                                                (float)width / (float)height,
                                                0.1f, 300.0f);

        glm::vec3 lightPos(camX, camY, camZ);
        glm::vec3 lightDir(frontX, frontY, frontZ); // NEW

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(glm::vec3(camX, camY, camZ)));

        glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir)); // NEW

        // Spotlight cutoff angles (in radians)
        float innerAngle = glm::cos(glm::radians(8.5f)); // NEW
        float outerAngle = glm::cos(glm::radians(15.0f)); 
        glUniform1f(glGetUniformLocation(shaderProgram, "cutOff"), innerAngle); // NEW
        glUniform1f(glGetUniformLocation(shaderProgram, "outerCutOff"), outerAngle); // NEW

        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.2f, 0.6f, 1.0f);

        drawMaze(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents(); // process events and callbacks
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// --- Smooth keyboard input using deltaTime ---
void processInput(GLFWwindow* window) {
    float moveSpeed = speedForward * deltaTime;
    float turnSpeed = speedTurn * deltaTime;

    float deltaX = 0.0f, deltaZ = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        deltaX = frontX * moveSpeed;
        deltaZ = frontZ * moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        deltaX = -frontX * moveSpeed;
        deltaZ = -frontZ * moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) yaw -= turnSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yaw += turnSpeed;

    frontX = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    frontY = 0.0f;
    frontZ = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

    if (!checkCollision(camX + deltaX, camZ + deltaZ, spacing)) {
        camX += deltaX;
        camZ += deltaZ;
    }
}

// --- Window resize callback ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// --- Shader loader ---
GLuint loadShader(const char* vertexPath, const char* fragmentPath) {
    std::ifstream vFile(vertexPath), fFile(fragmentPath);
    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();

    std::string vCode = vStream.str();
    std::string fCode = fStream.str();
    const char* vShaderCode = vCode.c_str();
    const char* fShaderCode = fCode.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}
