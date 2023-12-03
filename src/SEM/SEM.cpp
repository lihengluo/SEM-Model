#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(0.0f, 0.0f, -5.0f);

// 球面顶点数据
std::vector<float> sphereVertices;
std::vector<int> sphereIndices;
const int Y_SEGMENTS = 50;
const int X_SEGMENTS = 50;
const float Radio = 2.0;
const GLfloat  PI = 3.14159265358979323846f;
const float lengthInv = 1.0f / Radio; // 球的半径

// 生成球的顶点和纹理顶点
void generateBallVerticles(std::vector<float>& sphereVertices) {
  for (int y = 0; y <= Y_SEGMENTS; y++)
  {
    for (int x = 0; x <= X_SEGMENTS; x++)
    {
      float xSegment = (float)x / (float)X_SEGMENTS;
      float ySegment = (float)y / (float)Y_SEGMENTS;
      float xPos = std::cos(xSegment * Radio * PI) * std::sin(ySegment * PI);
      float yPos = std::cos(ySegment * PI);
      float zPos = std::sin(xSegment * Radio * PI) * std::sin(ySegment * PI);
      // 球的顶点
      sphereVertices.push_back(xPos);
      sphereVertices.push_back(yPos);
      sphereVertices.push_back(zPos);

      // normal
      sphereVertices.push_back(xPos*lengthInv);
      sphereVertices.push_back(yPos*lengthInv);
      sphereVertices.push_back(zPos*lengthInv);
      // 纹理顶点，映射到经纬
      sphereVertices.push_back(xSegment);
      sphereVertices.push_back(ySegment);
    }
  }
}

// 生成球的顶点索引
void generateBallIndices(std::vector<int>& sphereIndices) {
  for (int i = 0; i < Y_SEGMENTS; i++)
  {
    for (int j = 0; j < X_SEGMENTS; j++)
    {
      sphereIndices.push_back(i * (X_SEGMENTS + 1) + j);
      sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j);
      sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);

      sphereIndices.push_back(i * (X_SEGMENTS + 1) + j);
      sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
      sphereIndices.push_back(i * (X_SEGMENTS + 1) + j + 1);
    }
  }
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SEM-Model-U202115578-LihengLuo", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("SEM.vs", "SEM.fs");
    Shader lightCubeShader("light.vs", "light.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    generateBallVerticles(sphereVertices);
    generateBallIndices(sphereIndices);



    unsigned int VBO, VAO,VEO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VEO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0],GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VEO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);

    int stride = 8 * sizeof(float);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //unbind the buffer and vertexw
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // load textures (we now use a utility function to keep the code more organized)
    // -----------------------------------------------------------------------------
    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/final/sun.jpg").c_str());
    unsigned int diffuseMap2 = loadTexture(FileSystem::getPath("resources/textures/final/earth.jpg").c_str());
    unsigned int texture3 = loadTexture(FileSystem::getPath("resources/textures/final/moon.jpg").c_str());

    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("light.position", lightPos);
        lightingShader.setVec3("viewPos", camera.Position);

        // light properties
        lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        lightingShader.setFloat("material.shininess", 64.0f);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

      // render boxes
      glBindVertexArray(VAO);
      // bind diffuse map
      lightingShader.setInt("material.diffuse", 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, diffuseMap);
      // bind textures on corresponding texture units
      // calculate the model matrix for each object and pass it to shader before drawing
      glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
      model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));
      float angle = 20.0f * static_cast<float>(glfwGetTime());
      model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, -1.0f, 0.0f));
      lightingShader.setMat4("model", model);

      glDrawElements(GL_TRIANGLES,                    // primitive type
                     X_SEGMENTS * Y_SEGMENTS * 6,          // # of indices
                     GL_UNSIGNED_INT,                 // data type
                     (void*)0);                       // offset to indices

      // draw the earth
      lightingShader.setInt("material.diffuse", 1);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, diffuseMap2);
      model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
      // earth rotation around sun
      float angle_earth = 2.5f * static_cast<float>(glfwGetTime());
      // self rotation of earth
      float angle_self = 200.0f * static_cast<float>(glfwGetTime());
      model = glm::translate(model, glm::vec3(std::cos(angle_earth) * 3.0f, 0.0f, std::sin(angle_earth) * 2.0f-5.0f));
      model = glm::rotate(model, glm::radians(-22.5f), glm::vec3(0.0f, 0.0f, 1.0f));
      model = glm::rotate(model, glm::radians(angle_self), glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
      lightingShader.setMat4("model", model);
      glDrawElements(GL_TRIANGLES,                    // primitive type
                     X_SEGMENTS * Y_SEGMENTS * 6,          // # of indices
                     GL_UNSIGNED_INT,                 // data type
                     (void*)0);                       // offset to indices


      // draw the moon
      lightingShader.setInt("material.diffuse", 2);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, texture3);
      model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
      // moon rotation around earth
      float angle_moon = 5.0f * static_cast<float>(glfwGetTime());
      // moon self rotation
      float angle_self_moon = 100.0f * static_cast<float>(glfwGetTime());
      // get the position of the earth now time
      model = glm::translate(model, glm::vec3(std::cos(angle_earth) * 3.0f, 0.0f, std::sin(angle_earth) * 2.0f - 5.0f));
      // get self rotation angle
      model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      // public rotation around the earth
      model = glm::translate(model, glm::vec3(std::cos(angle_moon) * 0.5f, 0.0f, std::sin(angle_moon) * 0.5f));
      // self rotation of moon
      model = glm::rotate(model, glm::radians(angle_self_moon), glm::vec3(0.0f, 1.0f, 0.0f));
      // scale the moon
      model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
      lightingShader.setMat4("model", model);
      glDrawElements(GL_TRIANGLES,                    // primitive type
                     X_SEGMENTS * Y_SEGMENTS * 6,          // # of indices
                     GL_UNSIGNED_INT,                 // data type
                     (void*)0);                       // offset to indices




        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
