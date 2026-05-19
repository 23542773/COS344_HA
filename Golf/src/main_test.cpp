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

// Interactive runtime rendering state toggles
bool wireframeMode = false;
bool keyWasPressed = false;

const char *getError()
{
    const char *errorDescription;
    glfwGetError(&errorDescription);
    return errorDescription;
}

inline void startUpGLFW()
{
    glewExperimental = true; 
    if (!glfwInit())
    {
        throw getError();
    }
}

inline void startUpGLEW()
{
    glewExperimental = true; 
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        throw getError();
    }
}

inline GLFWwindow *setUp()
{
    startUpGLFW();
    glfwWindowHint(GLFW_SAMPLES, 4);               
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    
    GLFWwindow *window;
    window = glfwCreateWindow(1280, 720, "Stoneridge Complete Model Catalog Test - Person C", NULL, NULL);
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

    // Compile unlit verification shaders
    GLuint shaderProgram = LoadShaders("shaders/vertex_test.glsl", "shaders/fragment_test.glsl");
    if (shaderProgram == 0) {
        std::cerr << "CRITICAL: Test shader loading failed!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // =========================================================================
    // 1. INTRINSIC FLYWEIGHT MEMORY BUFFER (Declarations Fixed!)
    // =========================================================================
    std::cout << "\n[Streaming Complete Model Catalog into GPU Caches...]" << std::endl;
    
    // Allocate instances so identifiers are completely defined for the compiler
    Geom::ObjMesh rockMesh;
    Geom::ObjMesh treeMesh;
    Geom::ObjMesh barrierMesh;
    Geom::ObjMesh flagMesh;
    Geom::ObjMesh benchMesh;
    Geom::ObjMesh bridgeMesh;

    // Loading sequences accompanied by trace print statements
    std::cout << "Loading Rocks..." << std::endl;
    bool rocksLoaded = rockMesh.loadFromFile("models/Rocks.obj");

    std::cout << "Loading Palm Tree..." << std::endl;
    bool treesLoaded = treeMesh.loadFromFile("models/Palm_Tree.obj");

    std::cout << "Loading Wooden Barrier..." << std::endl;
    bool barriersLoaded = barrierMesh.loadFromFile("models/Wooden_Barrier.obj");

    std::cout << "Loading Flag..." << std::endl;
    bool flagsLoaded = flagMesh.loadFromFile("models/Flag.obj");

    std::cout << "Loading Bench..." << std::endl;
    bool benchLoaded = benchMesh.loadFromFile("models/Bench.obj");

    std::cout << "Loading Bridge..." << std::endl;
    bool bridgeLoaded = bridgeMesh.loadFromFile("models/Bridge.obj");

    std::cout << "All meshes finished processing!" << std::endl;

    // Print quick parsing success summaries
    std::cout << "\n--- CORE ASSET INTEGRITY PASS ---" << std::endl;
    std::cout << "  Rocks.obj:          " << (rocksLoaded ? "ONLINE" : "OFFLINE") << std::endl;
    std::cout << "  Palm_Tree.obj:      " << (treesLoaded ? "ONLINE" : "OFFLINE") << std::endl;
    std::cout << "  Wooden_Barrier.obj: " << (barriersLoaded ? "ONLINE" : "OFFLINE") << std::endl;
    std::cout << "  Flag.obj:           " << (flagsLoaded ? "ONLINE" : "OFFLINE") << std::endl;
    std::cout << "  Bench.obj:          " << (benchLoaded ? "ONLINE" : "OFFLINE") << std::endl;
    std::cout << "  Bridge.obj:         " << (bridgeLoaded ? "ONLINE" : "OFFLINE") << std::endl;
    std::cout << "---------------------------------\n" << std::endl;

    // =========================================================================
    // 2. EXTRINSIC POSITION VECTOR ARRAYS (Distributing Grid Contexts)
    // =========================================================================
    std::vector<Geom::MeshInstance> testSceneGrid;

    // A. Populate Rows of Obstacles
    if (rocksLoaded) {
        for (int i = 0; i < 5; ++i) {
            Geom::MeshInstance instance(&rockMesh);
            instance.position = glm::vec3(-2.2f + (i * 0.4f), -0.2f, -0.5f);
            instance.scale    = glm::vec3(0.15f);
            testSceneGrid.push_back(instance);
        }
    }
    if (barriersLoaded) {
        for (int i = 0; i < 5; ++i) {
            Geom::MeshInstance instance(&barrierMesh);
            instance.position = glm::vec3(-1.8f + (i * 0.8f), -0.3f, -1.2f);
            instance.scale    = glm::vec3(0.25f, 0.15f, 0.25f);
            testSceneGrid.push_back(instance);
        }
    }

    // B. Populate Rows of Decor & Landscaping elements
    if (treesLoaded) {
        for (int i = 0; i < 6; ++i) {
            Geom::MeshInstance instance(&treeMesh);
            instance.position = glm::vec3(-2.0f + (i * 0.8f), -0.3f, 1.2f);
            instance.scale    = glm::vec3(0.35f);
            testSceneGrid.push_back(instance);
        }
    }
    if (flagsLoaded) {
        for (int i = 0; i < 4; ++i) {
            Geom::MeshInstance instance(&flagMesh);
            instance.position = glm::vec3(-1.4f + (i * 0.7f), -0.3f, 0.4f);
            instance.scale    = glm::vec3(0.25f);
            testSceneGrid.push_back(instance);
        }
    }
    if (benchLoaded) {
        Geom::MeshInstance instance(&benchMesh);
        instance.position = glm::vec3(0.0f, -0.3f, 0.0f);
        instance.scale    = glm::vec3(0.3f);
        testSceneGrid.push_back(instance);
    }
    if (bridgeLoaded) {
        Geom::MeshInstance instance(&bridgeMesh);
        instance.position = glm::vec3(0.8f, -0.2f, -0.2f);
        instance.scale    = glm::vec3(0.4f);
        testSceneGrid.push_back(instance);
    }

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "uModel");

    // Main Loop
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Key W switch mode processing with debounce checks
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            if (!keyWasPressed) {
                wireframeMode = !wireframeMode;
                keyWasPressed = true;
            }
        } else {
            keyWasPressed = false;
        }

        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Batch execution loop drawing every element on screen cleanly
        for (const auto& instance : testSceneGrid) {
            instance.render(modelLoc, wireframeMode); 
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}