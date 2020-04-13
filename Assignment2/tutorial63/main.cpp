// CREATED BY FEDOR IVACHEV, 2019280373
// USING TUTORIAL 6 CODE



// Std. Includes
#include <string>
#include <map>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "shader.h"
#include "camera.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL.h>

# define M_PI           3.14159265358979323846  /* pi */

// Properties
GLuint WIDTH = 800, HEIGHT = 600;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
GLuint loadTexture(const char* path, GLboolean alpha = false);

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


class star
{
public:
    GLfloat angle;
    glm::vec4 color;
    GLfloat a;
    GLfloat b;
    GLfloat x;
    GLfloat y;
    GLfloat distance;
};


// put stars in the array
const int num = 100;
star stars[num];

// distance to the stars
GLfloat zoom = -15.0f;
// initial angle
GLfloat init_a = 0.5;
GLfloat init_b = 0.5;
GLint iter = 0;
GLfloat max_dist = 5.0;
GLboolean first_loop = true;

// The MAIN function, from here we start our application and run our Game loop
int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Assignment2", nullptr, nullptr); // Windowed
    glfwMakeContextCurrent(window);
    
    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Initialize GLAD to setup the OpenGL Function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);


    
    // Setup and compile our shaders
    Shader shader("main.vert.glsl", "main.frag.glsl");
    
    // Set the object data (buffers, vertex attributes)
    GLfloat starVertices[] = {
        // Positions          // Texture Coords
        0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  0.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  1.0f
    };

    
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(starVertices), &starVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);
    
    // Load textures
    GLuint starTexture = loadTexture("earth.jpg", true);
    

    for (int loop = 0; loop < num; loop++) {
        stars[loop].angle = M_PI / 10 * loop;
        stars[loop].color = glm::vec4(0.65 + loop % 2 / 4.0,  0.1 + (loop % 5) / 5.0, 0.29 + (loop % 2) / 2.0, 1.0f);
        stars[loop].a = init_a * (1 + M_PI / 10) / (1 + M_PI / 10 * (loop));
        stars[loop].b = init_b * (1 + M_PI / 10) / (1 + M_PI / 10 * (loop));
        stars[loop].x = (init_a + init_b * stars[loop].angle) * cos(stars[loop].angle);
        stars[loop].y = (init_a + init_b * stars[loop].angle) * sin(stars[loop].angle);
    }
    max_dist = glm::sqrt(stars[num - 1].x * stars[num - 1].x + stars[num - 1].y * stars[num - 1].y);
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Check and call events
        glfwPollEvents();
        Do_Movement();
        
        // Clear the colorbuffer
        glClearColor(0, 0, 0, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        // Draw objects
        shader.Use();
        glm::mat4 model(1);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        for (int loop = 0; loop < num; loop++) {
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, starTexture);
            model = glm::mat4(1);
            stars[loop].distance = glm::sqrt(stars[loop].x * stars[loop].x + stars[loop].y * stars[loop].y);

            stars[loop].x *= 1 + M_PI / 1000 / stars[loop].distance;
            stars[loop].y *= 1 + M_PI / 1000 / stars[loop].distance;

            stars[loop].distance = glm::sqrt(stars[loop].x * stars[loop].x + stars[loop].y * stars[loop].y);


            model = glm::translate(model, glm::vec3(stars[loop].x, stars[loop].y, zoom + loop * 0.001));
            if (stars[loop].distance >= max_dist - 0.0001) {
                stars[loop].angle = M_PI / 10 * (iter + 0.05);
                stars[loop].a = init_a * (1 + M_PI / 10) / (1 + M_PI / 10 * (iter));
                stars[loop].b = init_b * (1 + M_PI / 10) / (1 + M_PI / 10 * (iter));
                stars[loop].x = (stars[loop].a + stars[loop].b * stars[loop].angle) * sin(stars[loop].angle);
                stars[loop].y = (stars[loop].a + stars[loop].b * stars[loop].angle) * cos(stars[loop].angle);
                iter = (iter + 1) % num;
            }
            stars[loop].color.w = first_loop ? 0 :  1 - stars[loop].distance / max_dist;
            if (first_loop) {
                if (iter > num * 0.9) {
                    first_loop = false;
                }
            }
            GLint color_location = glGetUniformLocation(shader.Program, "my_color");
            glUniform4f(color_location, stars[loop].color.x, stars[loop].color.y, stars[loop].color.z, stars[loop].color.w);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 12);
        }
        glBindVertexArray(0);
        // Swap the buffers
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}

// This function loads a texture from file. Note: texture loading functions like these are usually
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio).
// For learning purposes we'll just define it as a utility function.

GLuint loadTexture(const char* path, GLboolean alpha)
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

// Moves/alters the camera positions based on user input
void Do_Movement()
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


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;
    
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

