#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/glm.hpp>

#include <learnopengl/filesystem.h>

#include <iostream>
#include <chrono>
#include <ctime>

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
void calculateBallCollisions();
void renderMenu(SpriteRenderer *sprite);
void updateLevel();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Status
enum GameStatus {menu, pointing, shooting };
GameStatus status = menu;
int hoopCount = 0;

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
float ballDiameter = (SCR_WIDTH * 1/14);
float ballPosX;
float ballPosY;

// Hole status
float holeDiameter = (SCR_WIDTH * 1/3);
float holePosX = (SCR_WIDTH * 1/2) - holeDiameter/2;
float holePosY = (SCR_HEIGHT * 1/5) - holeDiameter/2;

// Menu Status
std::string menuStatus = "menu_start";
std::chrono::time_point<std::chrono::system_clock> startTime;

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
    ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/arrow1.png").c_str(), GL_TRUE, "arrow");
		ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/burntball.png").c_str(), GL_TRUE, "ball");
    //menu
    ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/menu_start.jpg").c_str(), GL_TRUE, "menu_start");
    ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/menu_help.jpg").c_str(), GL_TRUE, "menu_help");
    ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/menu_exit.jpg").c_str(), GL_TRUE, "menu_exit");
    ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/menu_help_instructions.jpg").c_str(), GL_TRUE, "menu_help_instructions");
    ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/menu_start_3.jpg").c_str(), GL_TRUE, "menu_start_3");
    ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/menu_start_2.jpg").c_str(), GL_TRUE, "menu_start_2");
    ResourceManager::LoadTexture(FileSystem::getPath("resources/textures/menu_start_1.jpg").c_str(), GL_TRUE, "menu_start_1");
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

        if (status == menu){
          //draw menu
          renderMenu(arrow);
        } else {
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
                            glm::vec2(holeDiameter,holeDiameter),
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

  					tex = ResourceManager::GetTexture("ball");
  					arrow->DrawSprite(tex,
  														glm::vec2(ballPosX, ballPosY),
  														glm::vec2(ballDiameter, ballDiameter),
  														ballRot,
  														glm::vec3(1.0f, 1.0f, 1.0f));
            calculateBallCollisions();
          }

          if(hoopCount == 2){
            updateLevel();
          }

          arrowRot += arrowRotInc;
          ballPos += ballPosInc;
        }

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
    *x = arrowPosX - (ballDiameter/2) + (arrowWidth/2)   + (arrowLength/2 + ballPos) * glm::sin(ballRot);
    *y = arrowPosY - (ballDiameter/2) + (arrowLength/2)  - (arrowLength/2 + ballPos) * glm::cos(ballRot);
}

// Calculate all
// ---------------------------------------------------------------------------------------------
void calculateBallCollisions(){
  /* collision with the windows border */
  if( (ballPosX < 0) || (ballPosX  + ballDiameter > SCR_WIDTH) ||
      (ballPosY < 0) || (ballPosY  + ballDiameter > SCR_HEIGHT) ){
    status = pointing;
    // printf("ballPosX: %f\n", ballPosX);
    // printf("ballPosY: %f\n", ballPosY);
  }

  /* collision with the hole */
  float holeRadius = holeDiameter/2;
  float holeCenterX = holePosX + holeRadius;
  float holeCenterY = holePosY + holeRadius;

  float ballRadius = ballDiameter/2;
  float ballCenterX = ballPosX + ballRadius;
  float ballCenterY = ballPosY + ballRadius;

  // http://www.gamasutra.com/view/feature/3015/pool_hall_lessons_fast_accurate_.php
  if((glm::pow(ballCenterX - holeCenterX,2) + glm::pow(ballCenterY - holeCenterY,2)) <= glm::pow(ballRadius - holeRadius,2)){
    hoopCount++;
    status = pointing;
  }

}

void renderMenu(SpriteRenderer *sprite){
  Texture2D tex = ResourceManager::GetTexture(menuStatus);
  sprite->DrawSprite(tex,
                    glm::vec2(0.0f,0.0f),
                    glm::vec2(SCR_WIDTH, SCR_HEIGHT),
                    0.0f,
                    glm::vec3(1.0f, 1.0f, 1.0f));
}

void processInput(GLFWwindow* window){
  switch (status) {
    case menu:{
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        int elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds> (now-startTime).count();
        if (Keys[GLFW_KEY_UP] && !KeysProcessed[GLFW_KEY_UP]){
          if (menuStatus == "menu_start"){
            menuStatus = "menu_exit";
          } else if (menuStatus == "menu_help"){
            menuStatus = "menu_start";
          } else if (menuStatus == "menu_exit"){
            menuStatus = "menu_help";
          }
          KeysProcessed[GLFW_KEY_UP] = GL_TRUE;
        } else if (Keys[GLFW_KEY_DOWN] && !KeysProcessed[GLFW_KEY_DOWN]){
          if (menuStatus == "menu_start"){
            menuStatus = "menu_help";
          } else if (menuStatus == "menu_help"){
            menuStatus = "menu_exit";
          } else if (menuStatus == "menu_exit"){
            menuStatus = "menu_start";
          }
          KeysProcessed[GLFW_KEY_DOWN] = GL_TRUE;
        } else if (Keys[GLFW_KEY_ENTER] && !KeysProcessed[GLFW_KEY_ENTER]){
          if (menuStatus == "menu_start"){
            menuStatus = "menu_start_3";
            startTime = std::chrono::system_clock::now();
          } else if (menuStatus == "menu_help"){
            menuStatus = "menu_help_instructions";
          } else if (menuStatus == "menu_exit"){
            glfwSetWindowShouldClose(window, GL_TRUE);
          }
          KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
        } else if (menuStatus == "menu_start_3" && elapsedTime > 700){
          startTime = std::chrono::system_clock::now();
          menuStatus = "menu_start_2";
        } else if (menuStatus == "menu_start_2" && elapsedTime > 700){
          startTime = std::chrono::system_clock::now();
          menuStatus = "menu_start_1";
        } else if (menuStatus == "menu_start_1" && elapsedTime > 700){
          status = pointing;
        }
      }
      break;
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

void updateLevel(){
  holeDiameter = holeDiameter*2/3;
  holePosX = (SCR_WIDTH * 1/2) - holeDiameter/2;
  holePosY = (SCR_HEIGHT * 1/5) - holeDiameter/2;
  hoopCount = 0;
}
