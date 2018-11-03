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

#include <irrklang/irrKlang.h>
using namespace irrklang;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void processInput(GLFWwindow* window);
void calculateBallPosition(float *x, float *y);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Status
enum GameStatus { pointing, shooting };
GameStatus status = pointing;

// Objects status

// Arrow
float arrowRot = 0.0f;
float arrowRotInc = 0.02f;
float arrowLength = (SCR_HEIGHT * 1/4);
float arrowWidth = (SCR_WIDTH * 1/10);
float arrowPosX = (SCR_WIDTH * 1/2) - arrowWidth/2;
float arrowPosY = (SCR_HEIGHT * 5/6) - arrowLength/2;

// Ball status
float ballPos;
float ballPosInc = 5.0f;
float ballRot;
float ballDiameter = (SCR_WIDTH * 1/20);
float ballPosX;
float ballPosY;

// Hole status
float holeSize = (SCR_WIDTH * 1/3);
float holePosX = (SCR_WIDTH * 1/2) - holeSize/2;
float holePosY = (SCR_HEIGHT * 1/6) - holeSize/2;
// Input status
GLboolean Keys[1024];
GLboolean KeysProcessed[1024];

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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
		glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // start the sound engine with default parameters
    ISoundEngine* engine = createIrrKlangDevice();

    if (!engine){
       return 0; // error starting up the engine
    }

    engine->play2D(FileSystem::getPath("resources/sounds/puzzle-bobble.mp3").c_str(), true);

		// GL configuration
		// enable transparency
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile our shader programs
    // ------------------------------------
		ResourceManager::LoadShader("arrow.vs", "arrow.fs", nullptr, "arrow");

		// create arrow sprite
		Shader ourShader = ResourceManager::GetShader("arrow");
		SpriteRenderer *arrow = new SpriteRenderer(ourShader);

		glm::mat4 projection = glm::ortho(0.0f,
																			static_cast<GLfloat>(SCR_WIDTH),
         															static_cast<GLfloat>(SCR_HEIGHT),
																			0.0f, -1.0f, 1.0f);
		ResourceManager::GetShader("arrow").Use().SetInteger("image", 0);
		ResourceManager::GetShader("arrow").SetMatrix4("projection", projection);

		// load and create a texture
    // -------------------------
		ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/banana.png").c_str(), GL_TRUE, "arrow");
		ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/apple.png").c_str(), GL_TRUE, "awesome");
    ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/space.jpg").c_str(), GL_FALSE, "space");
    ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/space-hole.png").c_str(), GL_TRUE, "hole");

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

        // draw background
        Texture2D tex = ResourceManager::GetTexture("space");
        arrow->DrawSprite(tex,
                          glm::vec2(0.0f,0.0f),
                          glm::vec2(SCR_WIDTH, SCR_HEIGHT),
                          0.0f,
                          glm::vec3(1.0f, 1.0f, 1.0f));

        // draw Hole
        // TODO scale hole for increasing diffulties
        tex = ResourceManager::GetTexture("hole");
        arrow->DrawSprite(tex,
                          glm::vec2(holePosX, holePosY),
                          glm::vec2(holeSize,holeSize),
                          0.0f,
                          glm::vec3(1.0f, 1.0f, 1.0f));

        // bind textures on corresponding texture units
				tex = ResourceManager::GetTexture("arrow");
				tex.Bind();

        if(arrowRot > glm::half_pi<float>() || arrowRot < -glm::half_pi<float>()){
          arrowRotInc = -arrowRotInc;
        }

				arrow->DrawSprite(tex,
													glm::vec2(arrowPosX, arrowPosY),
													glm::vec2(arrowWidth, arrowLength),
													arrowRot,
													glm::vec3(1.0f, 1.0f, 1.0f),
                          0.5f);

        if (status==shooting){
          calculateBallPosition(&ballPosX, &ballPosY);

					tex = ResourceManager::GetTexture("awesome");
					arrow->DrawSprite(tex,
														glm::vec2(ballPosX, ballPosY),
														glm::vec2(ballDiameter*2, ballDiameter*2),
														ballRot,
														glm::vec3(1.0f, 1.0f, 1.0f));
        }

				/* se la palla esce dallo schermo allora aggiorna lo stato a pointing */
				if( (ballPosX < 0) || (ballPosX > SCR_WIDTH) ||
						(ballPosY < 0) || (ballPosY > SCR_HEIGHT) ){
					status = pointing;
					// printf("ballPosX: %f\n", ballPosX);
					// printf("ballPosY: %f\n", ballPosY);
				}

        arrowRot += arrowRotInc;
        ballPos += ballPosInc;

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    engine->drop();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  // When a user presses the escape key, we set the WindowShouldClose property to true, closing the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			Keys[key] = GL_TRUE;
		else if (action == GLFW_RELEASE)
		{
			Keys[key] = GL_FALSE;
			KeysProcessed[key] = GL_FALSE;
		}
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	std::cout << "xpos: " << xpos << std::endl;
	std::cout << "ypos: " << ypos << std::endl;
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
    *x = arrowPosX - (ballDiameter) + (arrowWidth/2)   + (arrowLength/2 + ballPos) * glm::sin(ballRot);
    *y = arrowPosY - (ballDiameter) + (arrowLength/2)  - (arrowLength/2 + ballPos) * glm::cos(ballRot);
}
void processInput(GLFWwindow* window){
  switch (status) {
    case pointing:{
      if (Keys[GLFW_KEY_SPACE] && !KeysProcessed[GLFW_KEY_SPACE]){
        status = shooting;
        ballPos = 0.0f;
        ballRot = arrowRot;
      }
      break;
    }
    case shooting:{
      break;
    }
  }
}
