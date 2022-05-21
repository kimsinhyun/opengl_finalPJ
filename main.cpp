// 33_ModelLoading
//     Mouse: left mouse dragging - arcball rotation
//            wheel - zooming
//     Keyboards:  r - reset camera and object position
//                 a - toggle camera/object rotations for arcball
//                 arrow left, right, up, down: panning object position
// Std. Includes
#include <string>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>
// GLEW
//#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include <shader.h>
#include <arcball.h>
#include <Model.h>
#include <Mesh.h>
#include <text.h>

//sound headers
//#include <mmsystem.h>
#include <windows.h>
#pragma comment(lib,"winmm.lib") 

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
#define M_PI 3.141592
#endif

using namespace std;

// Globals 
unsigned int SCR_WIDTH = 1800;
unsigned int SCR_HEIGHT = 900;

bool die = false;
GLFWwindow* mainWindow = NULL;
glm::mat4 projection;
Shader* shader;
Shader* background;

//pacman     
enum Direction {
    FRONT, BACK, LEFT, RIGHT
};
Direction currentDirection = FRONT;
Direction nextDirection = FRONT;
GLfloat currentAngles[5] = {0.0f, 180.0f, 90.0f, -90.0f, 0.0f};
Direction currentDirections[5] = { FRONT , BACK, RIGHT, LEFT, FRONT};
Direction nextDirections[5] = { FRONT , BACK, RIGHT, LEFT, FRONT };


// For model
Model* pacman;
Model* ghost;
Model* grid;
Model* space;
Model* yellow;

// for arcball
float arcballSpeed = 0.2f;
static Arcball camArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
static Arcball modelArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
bool arcballCamRot = true;


// for camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraOrigPos(0.0f, 0.0f, 50.0f);

// Ghost
int ghost_object_num = 5;
glm::vec3 ghostmoving1(0.0f, -0.5f, 0.0f);
glm::vec3 ghostmoving2(4.0f, -0.5f, 3.0f);
glm::vec3 ghostmoving3(3.0f, -0.5f, 1.0f);
glm::vec3 ghostmoving4(8.0f, -0.5f, -4.0f);
glm::vec3 ghostmoving5(1.0f, -0.5f, -7.0f);



vector<glm::vec3> yellows;
int yellows_num = 200;

//for time
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;
float cameraSpeed = 50.0f;
float ghost_last_moving_time = 0.0f;
float ghost_last_moving_times[5] = {};
//for mouse
bool validCursor = false;
float yaw = -90.0f;    // yaw is initialized to -90.0 degrees since a yaw of 0.0 results
                         // in a direction vector pointing to the right
                         // so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
float fov = 45.0f;

glm::mat4 model(1.0);

// text shader
Shader* textShader = NULL;
Text* text = NULL;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
GLFWwindow* glAllInit();
void render();
void render2();
GLfloat determineAngle();
GLfloat temp_determineAngle(Direction* temp_nextDirection, Direction* temp_currentDirection, int idx);
Direction generateRandomDirection();
double calculateDistance(int camera_x, int camera_z, int ghost_x, int ghost_z);


int main()
{
    mainWindow = glAllInit();
    srand(time(NULL));
    // Create shader program object
    textShader = new Shader("res/shaders/text.vs", "res/shaders/text.frag");
    shader = new Shader("res/shaders/modelLoading.vs", "res/shaders/modelLoading.frag");
    background = new Shader("res/shaders/backgroundLoading.vs", "res/shaders/backgroundLoading.frag");

   
    // Load models
    space = new Model((GLchar*)"res/models/space/space.obj");
    pacman = new Model((GLchar*)"res/models/pacman/QWER.obj");
    ghost = new Model((GLchar*)"res/models/ghost/ghost.obj");
    grid = new Model((GLchar*)"res/models/grid/grid.obj");
    yellow = new Model((GLchar*)"res/models/yellow/yellow.obj");
    
    for (int i = 0; i < yellows_num; i++) {
        yellows.push_back(glm::vec3(rand() % 160 - 80, 0, rand() % 160 - 80));
    }
    for (int i = 0; i < yellows.size(); ++i) {
        std::cout << yellows[i].x << std::endl;
    }

    while (!glfwWindowShouldClose(mainWindow) )
    {
        glfwPollEvents();
        if (!die) render();
        if (die) render2();
        
    }

    glfwTerminate();
    return 0;
}

void render()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();
    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    shader->setMat4("projection", projection);

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    //std::cout << "cameraPos: " << cameraPos.x << " ; "  << cameraPos.z << std::endl;
    //view = view * camArcBall.createRotationMatrix();
    shader->setMat4("view", view);

    glm::mat4 model(1.0);
    //===================draw ghosts===================
    //ghost objects
    for (int idx = 0; idx < ghost_object_num; idx++) {
        model = glm::mat4(1.0);
        if (currentFrame - ghost_last_moving_times[idx] >= 5.0f) {
            cout << currentFrame << endl;
            nextDirections[idx] = generateRandomDirection();
            ghost_last_moving_times[idx] = currentFrame;
        }
        glm::vec3 temp_ghostmoving = (idx == 0) ? ghostmoving1 : (idx == 1) ? ghostmoving2 : (idx == 2) ? ghostmoving3 : (idx == 3) ? ghostmoving4 : ghostmoving5;
        //std::cout << "distance : " << calculateDistance(cameraPos.x, cameraPos.z, temp_ghostmoving.x, temp_ghostmoving.z) << std::endl;
        model = glm::translate(model, temp_ghostmoving);
        float testtest = temp_determineAngle(nextDirections, currentDirections, idx);
        model = glm::rotate(model, glm::radians(currentAngles[idx] += temp_determineAngle(nextDirections,currentDirections, idx)), glm::vec3(0.0f, 1.0f, 0.0f));
        currentDirections[idx] = nextDirections[idx];
        if (currentDirections[idx] == LEFT) {
            if((temp_ghostmoving[0]<=80.0) & (temp_ghostmoving[0] >= -80.0))
                temp_ghostmoving[0] -= 0.03;
        }
        else if (currentDirections[idx] == RIGHT) {
            if ((temp_ghostmoving[0] <= 80.0) & (temp_ghostmoving[0] >= -80.0))
                temp_ghostmoving[0] += 0.03;
        }
        else if (currentDirections[idx] == FRONT) {
            if ((temp_ghostmoving[2] <= 80.0) & (temp_ghostmoving[2] >= -80.0))
                temp_ghostmoving[2] += 0.03;
        }
        else if (currentDirections[idx] == BACK) {
            if ((temp_ghostmoving[2] <= 80.0) & (temp_ghostmoving[2] >= -80.0))
                temp_ghostmoving[2] -= 0.03;
        }
        //충돌 감지
        if (calculateDistance(cameraPos.x, cameraPos.z, temp_ghostmoving.x, temp_ghostmoving.z) <= 2) {
            die = true;
            bool dead_sound = PlaySound("res/sound/pacman_death.wav", NULL, SND_ASYNC);

        }

        if (idx == 0)
            ghostmoving1 = temp_ghostmoving;
        if (idx == 1)
            ghostmoving2 = temp_ghostmoving;
        if (idx == 2)
            ghostmoving3 = temp_ghostmoving;
        if (idx == 3)
            ghostmoving4 = temp_ghostmoving;
        if (idx == 4)
            ghostmoving5 = temp_ghostmoving;
        shader->setMat4("model", model);
        ghost->Draw(shader);
    }

    //===================draw yellow balls ==========
    for (int i = 0; i < yellows.size(); i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, yellows[i]);
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        shader->setMat4("model", model);
        yellow->Draw(shader);
        if (calculateDistance(cameraPos.x, cameraPos.z, yellows[i].x, yellows[i].z) <= 2) {
            yellows.erase(yellows.begin() + i);
            yellows_num --;
            std::cout << "eat!" << std::endl;
            bool coin_sound = PlaySound("res/sound/pacman_chomp.wav", NULL, SND_ASYNC);
        }
    }


    //===================draw grid===================
    model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
    model = glm::scale(model, glm::vec3(80.0f, 80.0f, 80.0f));
    shader->setMat4("model", model);
    
    grid->Draw(shader);

    ////===================draw space background===================
    shader->use();
    model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(0.0f, -35.5f, -80.0f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(2.5f, 3.0f, 0.0f));
    shader->setMat4("model", model);
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shader->setMat4("view", view);
    space->Draw(shader);

    // Drawing texts
    glDisable(GL_DEPTH_TEST);
    std::string ghost_info = std::string("Left Ghost num: ") + to_string(ghost_object_num);
    std::string yellow_ball_info = std::string("Left Bell num: ") + to_string(yellows_num);
    text = new Text((char*)"fonts/arial.ttf", textShader, SCR_WIDTH, SCR_HEIGHT);
    text->RenderText(ghost_info, 1600.0f, 850.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
    text->RenderText(yellow_ball_info, 1600.0f, 750.0f, 0.5f, glm::vec3(1.0, 1.0f, 0.0f));
    glEnable(GL_DEPTH_TEST);
    glfwSwapBuffers(mainWindow);
}
void render2()
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Drawing texts
    glDisable(GL_DEPTH_TEST);
    text = new Text((char*)"fonts/arial.ttf", textShader, SCR_WIDTH, SCR_HEIGHT);
    text->RenderText("YOU DIE (press R to restart game)", 750.0f, 450.0f, 0.5f, glm::vec3(1.0, 1.0f, 0.0f));
    glEnable(GL_DEPTH_TEST);
    glfwSwapBuffers(mainWindow);
}


GLFWwindow* glAllInit()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Loading", nullptr, nullptr);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        exit(-1);
    }

    // Define the viewport dimensions
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // OpenGL options
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return(window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    projection = glm::perspective(glm::radians(45.0f),
        (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    shader->use();
    shader->setMat4("projection", projection);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        if (die) {
            // initialize all camera related parameters
            cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
            validCursor = false;
            yaw = -90.0f;
            pitch = 0.0f;
            lastX = SCR_WIDTH / 2.0;
            lastY = SCR_HEIGHT / 2.0;
            fov = 45.0f;
            glm::vec3 front;
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraFront = glm::normalize(front);

            die = false;
        }
        
    }
    else {
        float cameraDelta = cameraSpeed * deltaTime;

        if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
            cameraPos += cameraDelta * cameraFront;
        if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
            cameraPos -= cameraDelta * cameraFront;
        if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraDelta;
        if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraDelta;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (!validCursor) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastX = xpos;
            lastY = ypos;
            validCursor = true;
        }
        else validCursor = false;
    }
}


void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!validCursor) return;

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.2f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    //front.y = sin(glm::radians(pitch));
    front.y = sin(glm::radians(0.0f));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}


GLfloat determineAngle() {
    switch (nextDirection) {
    case FRONT:
        if (currentDirection == BACK)
            return 180.0f;
        if (currentDirection == RIGHT)
            return -90.0f;
        if (currentDirection == LEFT)
            return 90.0f;
        else
            return 0.0f;
    case BACK:
        if (currentDirection == FRONT)
            return 180.0f;
        if (currentDirection == RIGHT)
            return 90.0f;
        if (currentDirection == LEFT)
            return -90.0f;
        else
            return 0.0f;
    case RIGHT:
        if (currentDirection == FRONT)
            return 90.0f;
        if (currentDirection == BACK)
            return -90.0f;
        if (currentDirection == LEFT)
            return 180.0f;
        else
            return 0.0f;
    case LEFT:
        if (currentDirection == FRONT)
            return -90.0f;
        if (currentDirection == BACK)
            return 90.0f;
        if (currentDirection == RIGHT)
            return 180.0f;
        else
            return 0.0f;
    }
    return 0.0f;
}

GLfloat temp_determineAngle(Direction* temp_nextDirection, Direction* temp_currentDirection,int idx) {
    switch (temp_nextDirection[idx]) {
    case FRONT:
        if (temp_currentDirection[idx] == BACK) {
            return 180.0f;
        }
        if (temp_currentDirection[idx] == RIGHT) {
            return -90.0f;
        }
        if (temp_currentDirection[idx] == LEFT) {
            return 90.0f;
        }
        else
            return 0.0f;
    case BACK:
        if (temp_currentDirection[idx] == FRONT) {
            return 180.0f;
        }
        if (temp_currentDirection[idx] == RIGHT) {
            return 90.0f;
        }
        if (temp_currentDirection[idx] == LEFT) {
            return -90.0f;
        }
        else
            return 0.0f;
    case RIGHT:
        if (temp_currentDirection[idx] == FRONT) {
            return 90.0f;
        }
        if (temp_currentDirection[idx] == BACK) {
            return -90.0f;
        }
        if (temp_currentDirection[idx] == LEFT) {
            return 180.0f;
        }
        else
            return 0.0f;
    case LEFT:
        if (temp_currentDirection[idx] == FRONT) {
            return -90.0f;
        }
        if (temp_currentDirection[idx] == BACK) {
            return 90.0f;
        }
        if (temp_currentDirection[idx] == RIGHT) {
            return 180.0f;
        }
        else
            return 0.0f;
    }
    return 0.0f;
}

Direction generateRandomDirection() {
    int randomDirection = rand() % 4;
    switch (randomDirection) {
    case 0:
        return FRONT;
    case 1:
        return BACK;
    case 2:
        return LEFT;
    case 3:
        return RIGHT;
    }
}

double calculateDistance(int camera_x, int camera_z, int ghost_x, int ghost_z)
{
    return  sqrt(pow(camera_x - ghost_x, 2) + pow(camera_z - ghost_z, 2));
}