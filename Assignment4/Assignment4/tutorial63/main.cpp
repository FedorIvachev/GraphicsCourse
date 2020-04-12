#include <iostream>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "shader.h"
#include "ParticleSystem.h"
#include "vector"


// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
GLuint loadTexture(const char* path, GLboolean alpha = false);


// Window dimensions
GLuint WIDTH = 800, HEIGHT = 600;
// Deltatime
GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
GLfloat lastFrame = 0.0f;      // Time of last frame
std::vector <SnowFlake> snowflakes;
int snowflakeNum = 2;


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
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Snowflakes", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    
    // Set the required callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    
    // Initialize GLAD to setup the OpenGL Function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    
    // Build and compile our shader program
    Shader particleShader("main.vert.glsl", "main.frag.glsl");
    
    

    for (int i = 0; i < snowflakeNum; ++i)
    {
        snowflakes.push_back(SnowFlake(0.5, glm::vec3(rand()%(WIDTH), rand() % HEIGHT, 0),
                                    glm::vec3(0.0, rand() % 7 * -1, 0.0), glm::vec3(0.0, -0.5, 0.0), (double)rand() / (double)RAND_MAX * 10 + 32, 10.0f + rand() % 40));

    }

    
    
    // Set up mesh and attribute properties
    GLuint VBO, VAO;
    GLfloat particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    // Fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    // Set mesh attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);
    


    GLuint backgroundVBO, backgroundVAO;
    GLfloat background_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.3f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.3f, 1.0f, 1.0f, 1.0f,
        1.3f, 0.0f, 1.0f, 0.0f
    };
    glGenVertexArrays(1, &backgroundVAO);
    glGenBuffers(1, &backgroundVBO);
    glBindVertexArray(backgroundVAO);
    // Fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(background_quad), background_quad, GL_STATIC_DRAW);
    // Set mesh attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    
    // Load and create a texture
    GLuint texture = loadTexture("snow2Black.png");
    GLuint background_texture = loadTexture("paris.jpg");

    
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (rand() % (1000) == 0) {
            snowflakeNum++;
            snowflakes.push_back(SnowFlake(0.5, glm::vec3(rand() % (WIDTH), HEIGHT + 10, 0),
                glm::vec3(0.0, rand() % 7 * -1, 0.0), glm::vec3(0.0, -0.5, 0.0), (double)rand() / (double)RAND_MAX * 10 + 32, 10.0f + rand() % 40));
        }
        
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        
        // Render
        // Clear the colorbuffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        for (int i = 0; i < snowflakeNum; ++i)
            snowflakes[i].process(deltaTime*3, glm::vec3(rand()%(WIDTH), HEIGHT + 10, 0)); // apply gravity and update speed, position
        
        // Draw
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glm::mat4 projection = glm::ortho(0.0f, GLfloat(WIDTH), 0.0f, GLfloat(HEIGHT), -1.0f, 100.0f);
        particleShader.Use();
        GLint projLoc = glGetUniformLocation(particleShader.Program, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));



        glBindVertexArray(backgroundVAO);
        GLint offsetLoc = glGetUniformLocation(particleShader.Program, "offset");
        glUniform2f(offsetLoc, 0.0f, -100.0f);
        GLint colorLoc = glGetUniformLocation(particleShader.Program, "color");
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        GLfloat scaleLoc = glGetUniformLocation(particleShader.Program, "scale");
        glUniform1f(scaleLoc, 800.0f);
        glBindTexture(GL_TEXTURE_2D, background_texture);
        glBindVertexArray(backgroundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);


        for (int i = 0; i < snowflakeNum; ++i)
        {
            int j = 0;
            GLint offsetLoc = glGetUniformLocation(particleShader.Program, "offset");
            glUniform2f(offsetLoc, snowflakes[i].position[0], snowflakes[i].position[1]);
            GLint colorLoc = glGetUniformLocation(particleShader.Program, "color");
            glUniform4f(colorLoc, snowflakes[i].color.r, snowflakes[i].color.g, snowflakes[i].color.b,
                        snowflakes[i].color.a);
            GLfloat scaleLoc = glGetUniformLocation(particleShader.Program, "scale");
            glUniform1f(scaleLoc, snowflakes[i].scale);
            glBindTexture(GL_TEXTURE_2D, texture);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
        
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
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
}

GLuint loadTexture(const char * path, GLboolean alpha)
{
    //Generate texture ID and load texture data
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height;
    unsigned char* image = SOIL_load_image(path, &width, &height, 0, alpha ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);
    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);    // Use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes value from next repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return textureID;
}

