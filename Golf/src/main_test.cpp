#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <chrono> 

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader.hpp"

// Define the tinyobjloader implementation macro exactly once in this translation unit
#define TINYOBJLOADER_IMPLEMENTATION
#include "ObjLoader.h"

using namespace glm;
using namespace std;

const char *getError()
{
    const char *errorDescription;
    glfwGetError(&errorDescription);
    return errorDescription;
}

inline void startUpGLFW()
{
    glewExperimental = true; // Needed for core profile
    if (!glfwInit())
    {
        throw getError();
    }
}

inline void startUpGLEW()
{
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        throw getError();
    }
}

inline GLFWwindow *setUp()
{
    startUpGLFW();
    glfwWindowHint(GLFW_SAMPLES, 4);               // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want old OpenGL
    
    GLFWwindow *window;
    window = glfwCreateWindow(1280, 720, "Team's Golf Course - Person C Test Ground", NULL, NULL);
    if (window == NULL)
    {
        cout << getError() << endl;
        glfwTerminate();
        throw "Failed to open GLFW window.\n";
    }
    glfwMakeContextCurrent(window); 
    startUpGLEW();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    return window;
}

int main()
{
    GLFWwindow *window;
    try
    {
        window = setUp();
    }
    catch (const char *e)
    {
        cout << e << endl;
        return -1;
    }

    // 1. Compile baseline test shaders through your team's utility file logic
    GLuint shaderProgram = LoadShaders("shaders/vertex_test.glsl", "shaders/fragment_test.glsl");
    if (shaderProgram == 0) {
        std::cerr << "CRITICAL: Test shader loading failed! Check 'shaders/' directory paths." << std::endl;
        glfwTerminate();
        return -1;
    }

    // 2. Flyweight Optimization: Parse and stream heavy triangle data exactly ONCE into VRAM
    Geom::ObjMesh rockMesh;
    if (!rockMesh.loadFromFile("models/rocks.obj")) {
        std::cerr << "CRITICAL: Could not find or parse models/rocks.obj!" << std::endl;
        std::cerr << "Verify that rocks.obj is present inside the models/ folder." << std::endl;
        glDeleteProgram(shaderProgram);
        glfwTerminate();
        return -1;
    }

    // 3. Extrinsic Situational Mapping: Scatter separate lightweight instances across the green
    std::vector<Geom::MeshInstance> rockHazards;
    
    Geom::MeshInstance rock1(&rockMesh);
    rock1.position = glm::vec3(-0.4f, -0.1f, 0.0f);
    rock1.rotation = glm::vec3(0.0f, 35.0f, 0.0f);
    rock1.scale    = glm::vec3(0.3f);
    rockHazards.push_back(rock1);

    Geom::MeshInstance rock2(&rockMesh);
    rock2.position = glm::vec3(0.4f, 0.2f, 0.0f);
    rock2.rotation = glm::vec3(15.0f, 160.0f, -10.0f);
    rock2.scale    = glm::vec3(0.6f);
    rockHazards.push_back(rock2);

    // Get transform shader link slot location address 
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "uModel");

    std::cout << "\n[Smoke Test Ground Active] Both rock instances pointing to single geometry buffer entry." << std::endl;

    // Runtime Render Loop
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Standard gray clean screen clear operations
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind verification shader program context
        glUseProgram(shaderProgram);

        // Fast state rendering dispatch iteration loop
        for (const auto& instance : rockHazards) {
            // Note: pass 'true' instead of 'false' to instantly render primitive line wireframes
            instance.render(modelLoc, false); 
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up graphics engine allocations safely
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}