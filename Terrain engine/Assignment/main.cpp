#include <iostream>
#include <cmath>
#include <vector>
#include <map>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL.h>

// Other includes
#include "shader.h"
#include "camera.h"


// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLuint loadTexture(const char* path);
GLuint loadWaterTexture(const char* path);


// Window dimensions
GLuint WIDTH = 800, HEIGHT = 600;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
GLfloat lastX  =  WIDTH  / 2.0;
GLfloat lastY  =  HEIGHT / 2.0;
bool    keys[1024];
bool firstMouse = true;
GLfloat scale = 30.0;

// Deltatime
GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
GLfloat lastFrame = 0.0f;      // Time of last frame

// The MAIN function, from here we start the application and run the game loop
int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Terrain", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    
    // Set the required callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // GLFW Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Initialize GLAD to setup the OpenGL Function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    

    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build and compile our shader program
    Shader skyboxShader("skybox.vert.glsl", "skybox.frag.glsl");
    Shader mainShader("main.vert.glsl", "main.frag.glsl");
    Shader waterShader("water.vert.glsl", "water.frag.glsl");
    Shader terrainShader("terrain.vert.glsl", "terrain.frag.glsl");

    // Load CubeMap

    GLfloat skyboxVertices[] = {
        // Positions     // Texture coords     
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, // ++
        -1.0f, -1.0f, -1.0f, 1.0f,  0.0f,
         1.0f, -1.0f, -1.0f, 0.0f,  0.0f,
         1.0f, -1.0f, -1.0f, 0.0f,  0.0f,
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,  1.0f,  0.0f, // ++
        -1.0f, -1.0f, -1.0f, 0.0f,  0.0f,
        -1.0f,  1.0f, -1.0f, 0.0f,  1.0f,
        -1.0f,  1.0f, -1.0f, 0.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,  1.0f,  0.0f,
                    
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f, // ++
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,
         1.0f,  1.0f, -1.0f,  1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,
                    
        -1.0f, -1.0f,  1.0f, 0.0f,  0.0f, // ++
        -1.0f,  1.0f,  1.0f, 0.0f,  1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,  1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,  0.0f,
        -1.0f, -1.0f,  1.0f, 0.0f,  0.0f,
                    
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f, // ++
         1.0f,  1.0f, -1.0f, 1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,  0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,  0.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,

        -10.0f, 0.0f, -10.0f,  0.0f,  0.0f,
        -10.0f, 0.0f,  10.0f, 0.0f,  100.0f,
         10.0f, 0.0f, -10.0f, 100.0f,  0.0f,
         10.0f, 0.0f, -10.0f, 100.0f,  0.0f,
        -10.0f, 0.0f,  10.0f,  0.0f,  100.0f,
         10.0f, 0.0f,  10.0f, 100.0f,  100.0f
    };
    GLfloat skyboxVerticesReversed[180];
    memcpy(skyboxVerticesReversed, skyboxVertices, sizeof(skyboxVerticesReversed));
    // Change init to a better-looking style
    for (int i = 1; i < sizeof(skyboxVerticesReversed) / sizeof(*skyboxVerticesReversed); i+=5) {
        skyboxVerticesReversed[i] = skyboxVertices[i] * (-1.0f);
    }
    // Setup skybox VAO
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);

    // Setup reversed skybox VAO
    GLuint reversedskyboxVAO, reversedskyboxVBO;
    glGenVertexArrays(1, &reversedskyboxVAO);
    glGenBuffers(1, &reversedskyboxVBO);
    glBindVertexArray(reversedskyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, reversedskyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVerticesReversed), &skyboxVerticesReversed, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);

    GLuint frontTexture = loadTexture("data/SkyBox/SkyBox0.bmp");
    GLuint rightTexture = loadTexture("data/SkyBox/SkyBox1.bmp");
    GLuint backTexture = loadTexture("data/SkyBox/SkyBox2.bmp");
    GLuint leftTexture = loadTexture("data/SkyBox/SkyBox3.bmp");
    GLuint topTexture = loadTexture("data/SkyBox/SkyBox4.bmp");
    GLuint waterTexture = loadWaterTexture("data/SkyBox/SkyBox5.bmp");
    GLuint terrainTexture = loadTexture("data/terrain-texture3.bmp");

    // load terrain (need to put in a class

    int depthimage_width, depthimage_height;
    unsigned char *heightimage = SOIL_load_image("data/heightmap.bmp", &depthimage_width, &depthimage_height, nullptr, SOIL_LOAD_L);
    std::vector <float> heights_init{};
    std::vector <float> heights{};

    for (int x = 0; x < depthimage_width; x++) {
        for (int z = 0; z < depthimage_height; z++) {
            float h = heightimage[z * depthimage_width + x]; // ?????

            /* Normalize height to [-1, 1] */
            h = h / 127.5;
            heights_init.push_back(h / scale);
        }
    }

    for (int x = 0; x < depthimage_height - 1; x++) {
        for (int z = 0; z < depthimage_width - 1; z++) {
            float h = heights_init[z * depthimage_width + x];
            heights.push_back(x * 1.0 / depthimage_width);
            heights.push_back(h);
            heights.push_back(z * 1.0 / depthimage_width);
            heights.push_back(0.0f);
            heights.push_back(0.0f);

            h = heights_init[z * depthimage_width + x + 2]; // ?
            heights.push_back((x + 1.0) / depthimage_width);
            heights.push_back(h);
            heights.push_back(z * 1.0 / depthimage_width);
            heights.push_back(0.0f);
            heights.push_back(1.0f);

            h = heights_init[z * depthimage_width + x + 3]; // ?
            heights.push_back((x + 1.0) / depthimage_width);
            heights.push_back(h);
            heights.push_back((z + 1.0) * 1.0 / depthimage_width);
            heights.push_back(1.0f);
            heights.push_back(1.0f);

            h = heights_init[z * depthimage_width + x + 3]; // ?
            heights.push_back((x + 1.0) / depthimage_width);
            heights.push_back(h);
            heights.push_back((z + 1.0) * 1.0 / depthimage_width);
            heights.push_back(1.0f);
            heights.push_back(1.0f);

            h = heights_init[z * depthimage_width + x + 1];
            heights.push_back(x * 1.0 / depthimage_width);
            heights.push_back(h);
            heights.push_back((z + 1.0) / depthimage_width);
            heights.push_back(1.0f);
            heights.push_back(0.0f);

            h = heights_init[z * depthimage_width + x];
            heights.push_back(x * 1.0 / depthimage_width);
            heights.push_back(h);
            heights.push_back(z * 1.0 / depthimage_width);
            heights.push_back(0.0f);
            heights.push_back(0.0f);
        }
    }
    SOIL_free_image_data(heightimage);

    // setup terrain VAO and VBO
    GLuint terrainVAO, terrainVBO;
    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glBindVertexArray(terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, heights.size() * sizeof(float), &heights[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);



    
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        do_movement();
        
        // Clear the colorbuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw Skybox
        skyboxShader.Use();
        // Camera/View transformation
        // Projection
        glm::mat4 projection(1);
        glm::mat4 view(1);
        glm::mat4 model(1);

        model = glm::translate(model, glm::vec3(0.0f, 0.9f * scale, 0.0f));
        projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        // Get the uniform locations
        GLint modelLoc = glGetUniformLocation(skyboxShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(skyboxShader.Program, "view");
        GLint projLoc = glGetUniformLocation(skyboxShader.Program, "projection");
        GLint scaleLoc = glGetUniformLocation(skyboxShader.Program, "scale");
        // Pass the matrices to the shader
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Draw skybox as last
        

        glDepthMask(GL_FALSE);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // skybox cube
        glUniform1f(scaleLoc, scale);

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_2D, backTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_2D, leftTexture);
        glDrawArrays(GL_TRIANGLES, 6, 6);
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_2D, rightTexture);
        glDrawArrays(GL_TRIANGLES, 12, 6);
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_2D, frontTexture);
        glDrawArrays(GL_TRIANGLES, 18, 6);
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_2D, topTexture);
        glDrawArrays(GL_TRIANGLES, 24, 6);

        // draw reversed skybox

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0.0f, -1.1f * scale, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(0);

        glBindVertexArray(reversedskyboxVAO);
        glBindTexture(GL_TEXTURE_2D, backTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(reversedskyboxVAO);
        glBindTexture(GL_TEXTURE_2D, leftTexture);
        glDrawArrays(GL_TRIANGLES, 6, 6);
        glBindVertexArray(reversedskyboxVAO);
        glBindTexture(GL_TEXTURE_2D, rightTexture);
        glDrawArrays(GL_TRIANGLES, 12, 6);
        glBindVertexArray(reversedskyboxVAO);
        glBindTexture(GL_TEXTURE_2D, frontTexture);
        glDrawArrays(GL_TRIANGLES, 18, 6);
        glBindVertexArray(reversedskyboxVAO);
        glBindTexture(GL_TEXTURE_2D, topTexture);
        glDrawArrays(GL_TRIANGLES, 24, 6);
        

        // render water

        waterShader.Use();

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f * scale, 0.0f));
        GLint watermodelLoc = glGetUniformLocation(waterShader.Program, "model");
        GLint waterviewLoc = glGetUniformLocation(waterShader.Program, "view");
        GLint waterprojLoc = glGetUniformLocation(waterShader.Program, "projection");
        GLint waterscaleLoc = glGetUniformLocation(waterShader.Program, "scale");
        GLint color_location = glGetUniformLocation(waterShader.Program, "my_color");
        GLint watertimeLoc = glGetUniformLocation(waterShader.Program, "u_time");

        glDepthMask(GL_TRUE);
        view = camera.GetViewMatrix();

        // Pass the matrices to the shader
        glUniformMatrix4fv(waterprojLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(watermodelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(waterviewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1f(waterscaleLoc, scale * 2);
        glUniform4f(color_location, 1.0, 1.0, 1.0, 0.8);
        glUniform1f(watertimeLoc, currentFrame);
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_2D, waterTexture);
        glDrawArrays(GL_TRIANGLES, 30, 6);

        // Render terrain // btw it should be rendered before the water so the reflections are calculated

        terrainShader.Use();
        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f * scale, 0.0f));
        GLint terrainmodelLoc = glGetUniformLocation(terrainShader.Program, "model");
        GLint terrainviewLoc = glGetUniformLocation(terrainShader.Program, "view");
        GLint terrainprojLoc = glGetUniformLocation(terrainShader.Program, "projection");
        GLint terrainscaleLoc = glGetUniformLocation(terrainShader.Program, "scale");
        
        view = camera.GetViewMatrix();

        // Pass the matrices to the shader
        glUniformMatrix4fv(terrainprojLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(terrainmodelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(terrainviewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1f(terrainscaleLoc, scale);
        glBindVertexArray(terrainVAO);
        glBindTexture(GL_TEXTURE_2D, terrainTexture);
        glDrawArrays(GL_TRIANGLES, 0, heights.size());

        


        
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_R])
        camera.ProcessKeyboard(UP, deltaTime);
    if (keys[GLFW_KEY_F])
        camera.ProcessKeyboard(DOWN, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left
    
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

GLuint loadTexture(const char* path)
{
    //Generate texture ID and load texture data
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height;
    unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);

    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);


    // Parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    // Use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes value from next repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    SOIL_free_image_data(image);
    return textureID;
}

GLuint loadWaterTexture(const char* path)
{
    //Generate texture ID and load texture data
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height;
    unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGBA);

    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glGenerateMipmap(GL_TEXTURE_2D);


    // Parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // Use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes value from next repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    SOIL_free_image_data(image);
    return textureID;
}

