#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_s.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void calculateBallPosition(float *x, float *y);

// settings
const unsigned int SCR_WIDTH = 1800;
const unsigned int SCR_HEIGHT = 1800;

// Status
enum GameStatus { pointing, shooting };
GameStatus status = pointing;

// Arrow status
float arrowRot = 0.0f;
float arrowRotInc = 0.02f;

// Ball status
float ballPos;
float ballPosInc=0.01f;
float ballAngle;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Space Hole", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("5.2.transform.vs", "5.2.transform.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions (x,y,z)   // texture coords
         1.0f,  1.0f, 0.0f,    1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,    1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,    0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,    0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // load and create a texture
    // -------------------------
    unsigned int texture;
    // texture
    // ---------
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = SOIL_load_image(FileSystem::getPath("resources/textures/arrow1.png").c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    SOIL_free_image_data(data);

    // enable transparency
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use();
    ourShader.setInt("tex", 0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        if(arrowRot > glm::half_pi<float>() || arrowRot < -glm::half_pi<float>()){
          arrowRotInc = -arrowRotInc;
        }
        //printf("arrowRot: %f\n", arrowRot);

        // arrow
        // ---------------
        glm::mat4 transform;


        transform = glm::translate(transform, glm::vec3(0.0f, -0.75f, 0.0f));

        // rotation NOT at the centro of mass
        //transform = glm::translate(transform, glm::vec3(0.0f, -0.1f, 0.0f));
        transform = glm::rotate(transform, arrowRot, glm::vec3(0.0f, 0.0f, 1.0f));
        //transform = glm::translate(transform, glm::vec3(0.0f, 0.1f, 0.0f));
        //
        transform = glm::scale(transform, glm::vec3(0.05f, 0.5f, 1.0f));

        // get their uniform location and set matrix (using glm::value_ptr)
        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        // with the uniform matrix set, draw the first container
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        if (status==shooting){
          //printf("shooting\n");

          transform = glm::mat4();

          transform = glm::translate(transform, glm::vec3(0.0f, -0.75f, 0.0f));
          transform = glm::translate(transform, glm::vec3(0.5f*glm::sin(-ballAngle), 0.5f*glm::cos(ballAngle), 0.0f));
          transform = glm::rotate(transform, ballAngle, glm::vec3(0.0f, 0.0f, 1.0f));
          transform = glm::translate(transform, glm::vec3(0.0f,ballPos, 0.0f));
          transform = glm::scale(transform, glm::vec3(0.05f, 0.03f, 0.05f));

          // get their uniform location and set matrix (using glm::value_ptr)
          unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
          glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

          float x;
          float y;

          calculateBallPosition(&x, &y);
          if( (x < -1) || (x > 1) ||
              (y < -1) || (y > 1)){
            status = pointing;
            printf("x: %f\n", x);
            printf("y: %f\n", y);
          }
          glBindVertexArray(VAO);
          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        /* se la palla esce dallo schermo allora aggiorna lo stato a pointing*/

        arrowRot += arrowRotInc;
        ballPos += ballPosInc;

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      if (status == pointing){
          status = shooting;
          ballPos = 0.0f;
          ballAngle = arrowRot;

          // printf( "-1.0f*glm::sin(ballAngle): %f\n", -1.0f*glm::sin(ballAngle));
          // printf( "1.0f*glm::cos(ballAngle): %f\n", 1.0f*glm::cos(ballAngle));
      }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// Calculate all
// ---------------------------------------------------------------------------------------------
void calculateBallPosition(float *x, float *y)
{
    *x = (0.5f + ballPos) * glm::sin(-ballAngle);
    *y = -0.75f + (0.5f + ballPos) * glm::cos(ballAngle);
}
