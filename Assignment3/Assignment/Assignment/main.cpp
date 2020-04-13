#include <iostream>
#include <cmath>
#include <map>
#include <string>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

// Other includes
#include "shader.h"
#include "camera.h"

// Other Libs
#include <SOIL.h>

# define M_PI           3.14159265358979323846  /* pi */



// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
void change_speed();
bool loadOBJ(const char* path, std::vector <glm::vec3 >& out_vertices,
    std::vector < glm::vec2 >& out_uvs,
    std::vector < glm::vec3 >& out_normals);
GLuint loadTexture(const char* path, GLboolean alpha);
void RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
int glhProjectf(double objx, double objy, double objz, double* modelview, double* projection, int* viewport, double* windowCoordinatex, double* windowCoordinatey, double* windowCoordinatez);


class planet
{
public:
    GLfloat angle;
    GLfloat distance;
    GLuint texture;
    GLfloat scale;
    double x;
    double y;
    std::string name;
};

planet planets[8];

// Window dimensions
GLuint WIDTH = 800, HEIGHT = 600;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
GLuint text_VAO, text_VBO;

// Camera
Camera camera(glm::vec3(0.0f, 10.0f, 10.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
GLint mode;

// Deltatime
GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
GLfloat lastFrame = 0.0f;      // Time of last frame

// Distance
GLfloat distance = 1.0f;
GLfloat speed = 0.0003;




// The MAIN function, from here we start the application and run the game loop
int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Solar system", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Initialize GLAD to setup the OpenGL Function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set OpenGL options
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compile and setup the text shader
    Shader text_shader("text.vert.glsl", "text.frag.glsl");
    glm::mat4 text_projection = glm::ortho(0.0f, static_cast<GLfloat>(WIDTH), 0.0f, static_cast<GLfloat>(HEIGHT));
    text_shader.Use();
    glUniformMatrix4fv(glGetUniformLocation(text_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(text_projection));


    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    
    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "C:/Windows/Fonts/Arial.ttf", 0, &face)) { // only absolute path works
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return 0;
    }
    
    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);
    
    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            GLuint(face->glyph->advance.x)
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }


    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &text_VAO);
    glGenBuffers(1, &text_VBO);
    glBindVertexArray(text_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, text_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);




    glEnable(GL_DEPTH_TEST);


    // Build and compile our shader program
    Shader ourShader("texture.vert.glsl", "texture.frag.glsl");



    // load obj file

    std::vector< glm::vec3 > vertices1;
    std::vector< glm::vec2 > uvs;
    std::vector< glm::vec3 > normals;
    bool res = loadOBJ("sphere.obj", vertices1, uvs, normals);

    std::vector<GLfloat> vertices;
    for (unsigned int i = 0; i < vertices1.size(); i++) {

        vertices.push_back(vertices1[i].x);
        vertices.push_back(vertices1[i].y);
        vertices.push_back(vertices1[i].z);
        vertices.push_back(uvs[i].x);
        vertices.push_back(uvs[i].y);

    }

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0); // Unbind VAO


        // Load textures
    planets[2].texture = loadTexture("Texture/earth.jpg", true);
    planets[4].texture = loadTexture("Texture/jupiter.jpg", true);
    planets[3].texture = loadTexture("Texture/mars.jpg", true);
    planets[0].texture = loadTexture("Texture/mercury.jpg", true);
    GLuint moonTexture = loadTexture("Texture/moon.jpg", true);
    planets[7].texture = loadTexture("Texture/neptune.jpg", true);
    planets[5].texture = loadTexture("Texture/saturn.jpg", true);
    GLuint sunTexture = loadTexture("Texture/sun.jpg", true);
    planets[6].texture = loadTexture("Texture/uranus.jpg", true);
    planets[1].texture = loadTexture("Texture/venus.jpg", true);

    // defining initial values of planets

    for (int i = 0; i < 8; i++) {
        planets[i].distance = distance;
        planets[i].angle = 0;
        distance += 0.5f * log(i + 4);
    }

    planets[0].scale = 0.38f;
    planets[1].scale = 0.94f;
    planets[2].scale = 1.0f;
    planets[3].scale = 0.53f;
    planets[4].scale = 2.0f;
    planets[5].scale = 1.6f;
    planets[6].scale = 1.2f;
    planets[7].scale = 1.3f;

    planets[0].name = "mercury";
    planets[1].name = "venus";
    planets[2].name = "earth";
    planets[3].name = "mars";
    planets[4].name = "jupiter";
    planets[5].name = "saturn";
    planets[6].name = "uranus";
    planets[7].name = "neptune";


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
        change_speed();

        // Render
        // Clear the colorbuffer
        glClearColor(0, 0, 0, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderText(text_shader, "Use W, A, S, D and your mouse to move camera, use left/right arrow keys to change speed", 25.0f, 25.0f, 0.3f, glm::vec3(0.5, 0.5f, 0.8f));
        std::string speed_text = "Current Speed : " + std::to_string(speed);
        RenderText(text_shader, speed_text, 25.0f, 10.0f, 0.3f, glm::vec3(0.5, 0.5f, 0.8f));

        for (int i = 0; i < 8; i++) {
            RenderText(text_shader, planets[i].name, planets[i].x, planets[i].y, 0.3f, glm::vec3(0.5, 0.5f, 0.8f));
        }




        // Activate shader
        
        ourShader.Use();
        // Camera/View transformation
        glm::mat4 view(1);
        view = camera.GetViewMatrix();
        // Projection
        glm::mat4 projection(1);
        projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        // Get the uniform locations
        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
        GLfloat scaleLoc = glGetUniformLocation(ourShader.Program, "scale");
        // Pass the matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        for (int i = 0; i < 8; i++) {
            glUniform1f(scaleLoc, planets[i].scale);

            planets[i].angle = planets[i].angle > 2 * M_PI ? planets[i].angle + M_PI * speed / log(i * i / 2.5 + 2) - 2 * M_PI : planets[i].angle + M_PI * speed / log(i * i / 2.5 + 2);
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, planets[i].texture);
            glm::mat4 model_vertex(1);
            model_vertex = glm::translate(model_vertex, 
                glm::vec3(planets[i].distance * sin(planets[i].angle), 0.0f, planets[i].distance * cos(planets[i].angle)));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_vertex));
            glm::mat4 coords = view * model_vertex;

            double zval;


            double modelMatrix[16];

            modelMatrix[0] =  coords[0][0];
            modelMatrix[1] =  coords[0][1];
            modelMatrix[2] =  coords[0][2];
            modelMatrix[3] =  coords[0][3];
            modelMatrix[4] =  coords[1][0];
            modelMatrix[5] =  coords[1][1];
            modelMatrix[6] =  coords[1][2];
            modelMatrix[7] =  coords[1][3];
            modelMatrix[8] =  coords[2][0];
            modelMatrix[9] =  coords[2][1];
            modelMatrix[10] = coords[2][2];
            modelMatrix[11] = coords[2][3];
            modelMatrix[12] = coords[3][0];
            modelMatrix[13] = coords[3][1];
            modelMatrix[14] = coords[3][2];
            modelMatrix[15] = coords[3][3];

            double projMatrix[16];

            projMatrix[0] =  projection[0][0];
            projMatrix[1] =  projection[0][1];
            projMatrix[2] =  projection[0][2];
            projMatrix[3] =  projection[0][3];
            projMatrix[4] =  projection[1][0];
            projMatrix[5] =  projection[1][1];
            projMatrix[6] =  projection[1][2];
            projMatrix[7] =  projection[1][3];
            projMatrix[8] =  projection[2][0];
            projMatrix[9] =  projection[2][1];
            projMatrix[10] = projection[2][2];
            projMatrix[11] = projection[2][3];
            projMatrix[12] = projection[3][0];
            projMatrix[13] = projection[3][1];
            projMatrix[14] = projection[3][2];
            projMatrix[15] = projection[3][3];

            GLint viewport[4];

            //glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
            //glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
            glGetIntegerv(GL_VIEWPORT, viewport);


            glhProjectf(-0.5f, -0.5f, 0.0f, modelMatrix, projMatrix, viewport, &planets[i].x, &planets[i].y, &zval);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
            

        }

        // Render sun
        glUniform1f(scaleLoc, 4.0f);
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        glm::mat4 model_vertex(1);
        model_vertex = glm::translate(model_vertex,
            glm::vec3(0.0f, 0.0f, 0.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_vertex));

        glDrawArrays(GL_TRIANGLES, 0, vertices.size());


        // Render moon
        glUniform1f(scaleLoc, 0.3f);
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, moonTexture);
        model_vertex = glm::translate(model_vertex,
            glm::vec3(planets[2].distance * sin(planets[2].angle) + planets[2].distance / 5 * cos(planets[2].angle * 28), 0.0f, planets[2].distance * cos(planets[2].angle) + planets[2].distance / 5 * sin(planets[2].angle * 28)));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_vertex));

        glDrawArrays(GL_TRIANGLES, 0, vertices.size());


        
        

        
        glBindVertexArray(0);

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
    GLfloat cameraSpeed = 5.0f * deltaTime;
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void change_speed()
{
    if (keys[GLFW_KEY_RIGHT])
        speed += 0.0001;
    if (keys[GLFW_KEY_LEFT])
        speed -= 0.0001;
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
    GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// loading obj file, code based on http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
bool loadOBJ(const char* path, std::vector <glm::vec3 >& out_vertices,
    std::vector < glm::vec2 >& out_uvs,
    std::vector < glm::vec3 >& out_normals) {
    std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    std::vector< glm::vec3 > temp_vertices;
    std::vector< glm::vec2 > temp_uvs;
    std::vector< glm::vec3 > temp_normals;

    FILE* file = fopen(path, "r");

    while (1) {

        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            vertex.x /= 100;
            vertex.y /= 100;
            vertex.z /= 100;
            temp_vertices.push_back(vertex);

        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9) {
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }
    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        unsigned int vertexIndex = vertexIndices[i];
        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        out_vertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < uvIndices.size(); i++) {
        unsigned int uvIndex = uvIndices[i];
        glm::vec2 vertex = temp_uvs[uvIndex - 1];
        out_uvs.push_back(vertex);
    }
    for (unsigned int i = 0; i < normalIndices.size(); i++) {
        unsigned int normalIndex = normalIndices[i];
        glm::vec3 vertex = temp_normals[normalIndex - 1];
        out_normals.push_back(vertex);
    }
    return true;
}

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

void RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state
    shader.Use();
    glUniform3f(glGetUniformLocation(shader.Program, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(text_VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 1.0 },
            { xpos,     ypos,       0.0, 0.0 },
            { xpos + w, ypos,       1.0, 0.0 },

            { xpos,     ypos + h,   0.0, 1.0 },
            { xpos + w, ypos,       1.0, 0.0 },
            { xpos + w, ypos + h,   1.0, 1.0 }
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, text_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// from :: https://www.khronos.org/opengl/wiki/GluProject_and_gluUnProject_code
int glhProjectf(double objx, double objy, double objz, double* modelview, double* projection, int* viewport, double *windowCoordinatex, double* windowCoordinatey, double* windowCoordinatez)
{
    // Transformation vectors
    double fTempo[8];
    // Modelview transform
    fTempo[0] = modelview[0] * objx + modelview[4] * objy + modelview[8] * objz + modelview[12]; // w is always 1
    fTempo[1] = modelview[1] * objx + modelview[5] * objy + modelview[9] * objz + modelview[13];
    fTempo[2] = modelview[2] * objx + modelview[6] * objy + modelview[10] * objz + modelview[14];
    fTempo[3] = modelview[3] * objx + modelview[7] * objy + modelview[11] * objz + modelview[15];
    // Projection transform, the final row of projection matrix is always [0 0 -1 0]
    // so we optimize for that.
    fTempo[4] = projection[0] * fTempo[0] + projection[4] * fTempo[1] + projection[8] * fTempo[2] + projection[12] * fTempo[3];
    fTempo[5] = projection[1] * fTempo[0] + projection[5] * fTempo[1] + projection[9] * fTempo[2] + projection[13] * fTempo[3];
    fTempo[6] = projection[2] * fTempo[0] + projection[6] * fTempo[1] + projection[10] * fTempo[2] + projection[14] * fTempo[3];
    fTempo[7] = -fTempo[2];
    // The result normalizes between -1 and 1
    if (fTempo[7] == 0.0) // The w value
        return 0;
    fTempo[7] = 1.0 / fTempo[7];
    // Perspective division
    fTempo[4] *= fTempo[7];
    fTempo[5] *= fTempo[7];
    fTempo[6] *= fTempo[7];
    // Window coordinates
    // Map x, y to range 0-1
    *windowCoordinatex = (fTempo[4] * 0.5 + 0.5) * viewport[2] + viewport[0];
    *windowCoordinatey = (fTempo[5] * 0.5 + 0.5) * viewport[3] + viewport[1];
    // This is only correct when glDepthRange(0.0, 1.0)
    *windowCoordinatez = (1.0 + fTempo[6]) * 0.5;	// Between 0 and 1
    return 1;
}