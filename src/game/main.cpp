#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>

#include <iostream>

#include <common/ResourceManager.h>
std::map<std::string, Shader>    ResourceManager::Shaders;
std::map<std::string, Texture2D> ResourceManager::Textures;
#include <common/SpriteRenderer.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void calculateBallPosition(float *x, float *y);

// settings
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 900;

// Status
enum GameStatus { pointing, shooting };
GameStatus status = pointing;

// Objects status

// Arrow
float arrowRot = 0.0f;
float arrowRotInc = 0.02f;
float arrowLength = 200.0f;
float arrowWidth = 50.0f;
float arrrowPosX = 400.0f;
float arrrowPosY = 400.0f;

// Ball status
float ballPos;
float ballPosInc = 5.0f;
float ballRot;
float ballRadius = 50.0f;
float ballPosX;
float ballPosY;

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
		ResourceManager::LoadShader("arrow.vs", "arrow.fs", nullptr, "arrow");

		Shader ourShader = ResourceManager::GetShader("arrow");

		// create arrow sprite
		SpriteRenderer *arrow = new SpriteRenderer(ourShader);

		ResourceManager::GetShader("arrow").Use().SetInteger("tex", 0);
		glm::mat4 projection = glm::ortho(0.0f,
																			static_cast<GLfloat>(SCR_WIDTH),
         															static_cast<GLfloat>(SCR_HEIGHT),
																			0.0f, -1.0f, 1.0f);
		ResourceManager::GetShader("arrow").Use().SetInteger("image", 0);
		ResourceManager::GetShader("arrow").SetMatrix4("projection", projection);

		// load and create a texture
    // -------------------------
		ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/arrow1.png").c_str(), GL_TRUE, "arrow");

    // enable transparency
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
				ResourceManager::GetTexture("arrow").Bind();

        if(arrowRot > glm::half_pi<float>() || arrowRot < -glm::half_pi<float>()){
          arrowRotInc = -arrowRotInc;
        }

				Texture2D tex = ResourceManager::GetTexture("arrow");
				arrow->DrawSprite(tex,
													glm::vec2(arrrowPosX, arrrowPosY),
													glm::vec2(arrowWidth, arrowLength),
													arrowRot,
													glm::vec3(0.0f, 1.0f, 0.0f));

				Texture2D texArrow = ResourceManager::GetTexture("arrow");
				arrow->DrawSprite(texArrow,
													glm::vec2(arrrowPosX, arrrowPosY + arrowLength/2),
													glm::vec2(ballRadius, ballRadius),
													ballRot,
													glm::vec3(0.0f, 0.0f, 0.0f));


        if (status==shooting){
					printf("ballPosX: %f\n", ballPosX);
					printf("ballPosY: %f\n", ballPosY);

          calculateBallPosition(&ballPosX, &ballPosY);

					Texture2D tex = ResourceManager::GetTexture("arrow");
					arrow->DrawSprite(tex,
														glm::vec2(ballPosX, ballPosY),
														glm::vec2(ballRadius, ballRadius),
														ballRot,
														glm::vec3(1.0f, 0.0f, 0.0f));
        }

				/* se la palla esce dallo schermo allora aggiorna lo stato a pointing */
				if( (ballPosX < 0) || (ballPosX > SCR_WIDTH) ||
						(ballPosY < 0) || (ballPosY > SCR_HEIGHT)){
					status = pointing;
					printf("ballPosX: %f\n", ballPosX);
					printf("ballPosY: %f\n", ballPosY);
				}

        arrowRot += arrowRotInc;
        ballPos += ballPosInc;

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
          ballRot = arrowRot;
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
    *x = (arrrowPosX) + (arrowLength/2 + ballPos) * glm::sin(ballRot);
    *y = (arrrowPosY + arrowLength/2) - (arrowLength/2 + ballPos) * glm::cos(ballRot);
}
