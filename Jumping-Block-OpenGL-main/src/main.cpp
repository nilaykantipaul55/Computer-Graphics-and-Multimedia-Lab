
#include "glad.h"
#include "glfw3.h"

#include <iostream>
#include <string>
#include <algorithm>

// --------------------------------------------------
// WINDOW SETTINGS
// --------------------------------------------------

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// --------------------------------------------------
// SHADERS
// --------------------------------------------------

const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core

out vec4 FragColor;

uniform vec4 objectColor;

void main()
{
    FragColor = objectColor;
}
)";

// --------------------------------------------------
// GAME VARIABLES
// --------------------------------------------------

float groundY = -0.72f;

float playerX = -0.65f;
float playerY = groundY + 0.06f;

float playerWidth = 0.12f;
float playerHeight = 0.12f;

float velocityY = 0.0f;

float gravity = 4.8f;
float jumpForce = 1.65f;

bool isJumping = false;
bool gameOver = false;

float obstacleX = 1.1f;
float obstacleWidth = 0.10f;
float obstacleHeight = 0.13f;

float obstacleSpeed = 0.55f;

int score = 0;

// --------------------------------------------------
// RECTANGLE DRAWING FUNCTION
// --------------------------------------------------

void drawRectangle(
    unsigned int VAO,
    unsigned int VBO,
    unsigned int shaderProgram,
    float x,
    float y,
    float width,
    float height,
    float r,
    float g,
    float b
)
{
    float vertices[] = {

        // First triangle
        x - width / 2, y - height / 2, 0.0f,
        x + width / 2, y - height / 2, 0.0f,
        x + width / 2, y + height / 2, 0.0f,

        // Second triangle
        x - width / 2, y - height / 2, 0.0f,
        x + width / 2, y + height / 2, 0.0f,
        x - width / 2, y + height / 2, 0.0f
    };

    glUseProgram(shaderProgram);

    // Update rectangle position
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(vertices),
        vertices
    );

    // Set object color
    int colorLocation = glGetUniformLocation(
        shaderProgram,
        "objectColor"
    );

    glUniform4f(
        colorLocation,
        r, g, b, 1.0f
    );

    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// --------------------------------------------------
// COLLISION DETECTION
// --------------------------------------------------

bool checkCollision()
{
    float playerLeft = playerX - playerWidth / 2;
    float playerRight = playerX + playerWidth / 2;

    float playerBottom = playerY - playerHeight / 2;
    float playerTop = playerY + playerHeight / 2;

    float obstacleLeft = obstacleX - obstacleWidth / 2;
    float obstacleRight = obstacleX + obstacleWidth / 2;

    float obstacleBottom = groundY;
    float obstacleTop = groundY + obstacleHeight;

    if (
        playerRight > obstacleLeft &&
        playerLeft < obstacleRight &&
        playerTop > obstacleBottom &&
        playerBottom < obstacleTop
    )
    {
        return true;
    }

    return false;
}

// --------------------------------------------------
// RESET GAME
// --------------------------------------------------

void resetGame()
{
    playerY = groundY + playerHeight / 2;

    velocityY = 0.0f;

    isJumping = false;
    gameOver = false;

    obstacleX = 1.1f;

    obstacleSpeed = 0.55f;

    score = 0;
}

// --------------------------------------------------
// INPUT HANDLING
// --------------------------------------------------

void processInput(GLFWwindow* window)
{
    static bool spaceWasPressed = false;
    static bool upWasPressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    bool spacePressed =
        glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    bool upPressed =
        glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;

    bool jumpPressed =
        (spacePressed && !spaceWasPressed) ||
        (upPressed && !upWasPressed);

    if (!gameOver && jumpPressed && !isJumping)
    {
        velocityY = jumpForce;
        isJumping = true;
    }

    spaceWasPressed = spacePressed;
    upWasPressed = upPressed;

    // Restart game
    if (
        gameOver &&
        glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS
    )
    {
        resetGame();
    }
}

// --------------------------------------------------
// MAIN FUNCTION
// --------------------------------------------------

int main()
{
    // Initialize GLFW
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH,
        SCR_HEIGHT,
        "Jumping Block Game",
        NULL,
        NULL
    );

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window!" << std::endl;

        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(
        window,
        framebuffer_size_callback
    );

    // Initialize GLAD
    if (!gladLoadGLLoader(
        (GLADloadproc)glfwGetProcAddress
    ))
    {
        std::cout << "Failed to initialize GLAD!" << std::endl;

        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // --------------------------------------------------
    // COMPILE SHADERS
    // --------------------------------------------------

    unsigned int vertexShader;
    unsigned int fragmentShader;

    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexShaderSource,
        NULL
    );

    glCompileShader(vertexShader);

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentShaderSource,
        NULL
    );

    glCompileShader(fragmentShader);

    // Create shader program
    unsigned int shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --------------------------------------------------
    // CREATE VAO AND VBO
    // --------------------------------------------------

    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Reserve memory for 6 vertices
    glBufferData(
        GL_ARRAY_BUFFER,
        18 * sizeof(float),
        NULL,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    // --------------------------------------------------
    // GAME LOOP
    // --------------------------------------------------

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        // Calculate frame time
        float currentFrame = (float)glfwGetTime();

        float deltaTime = currentFrame - lastFrame;

        lastFrame = currentFrame;

        // Avoid large physics jumps after window pauses
        deltaTime = std::min(deltaTime, 0.05f);

        // Handle keyboard input
        processInput(window);

        // --------------------------------------------------
        // UPDATE GAME PHYSICS
        // --------------------------------------------------

        if (!gameOver)
        {
            // Player jumping and gravity
            velocityY -= gravity * deltaTime;

            playerY += velocityY * deltaTime;

            // Ground collision
            float playerGround = groundY + playerHeight / 2;

            if (playerY <= playerGround)
            {
                playerY = playerGround;

                velocityY = 0.0f;

                isJumping = false;
            }

            // Move obstacle
            obstacleX -= obstacleSpeed * deltaTime;

            // Obstacle passes player
            if (obstacleX < -1.1f)
            {
                obstacleX = 1.1f;

                score++;

                // Increase difficulty
                obstacleSpeed += 0.04f;
            }

            // Check collision
            if (checkCollision())
            {
                gameOver = true;
            }
        }

        // Update window title with score
        std::string title = "Jumping Block Game | Score: "
                            + std::to_string(score);

        if (gameOver)
        {
            title += " | GAME OVER - Press R to Restart";
        }

        glfwSetWindowTitle(window, title.c_str());

        // --------------------------------------------------
        // RENDERING
        // --------------------------------------------------

        glClearColor(
            0.08f,
            0.10f,
            0.16f,
            1.0f
        );

        glClear(GL_COLOR_BUFFER_BIT);

        // Draw ground
        drawRectangle(
            VAO, VBO, shaderProgram,
            0.0f, groundY - 0.04f,
            2.0f, 0.08f,
            0.0f, 0.8f, 0.3f
        );

        // Draw player block
        drawRectangle(
            VAO, VBO, shaderProgram,
            playerX, playerY,
            playerWidth, playerHeight,
            0.1f, 0.4f, 1.0f
        );

        // Draw obstacle
        drawRectangle(
            VAO, VBO, shaderProgram,
            obstacleX,
            groundY + obstacleHeight / 2,
            obstacleWidth,
            obstacleHeight,
            1.0f, 0.15f, 0.15f
        );

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll events
        glfwPollEvents();
    }

    // --------------------------------------------------
    // CLEANUP
    // --------------------------------------------------

    glDeleteVertexArrays(1, &VAO);

    glDeleteBuffers(1, &VBO);

    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}

// --------------------------------------------------
// WINDOW RESIZE CALLBACK
// --------------------------------------------------

void framebuffer_size_callback(
    GLFWwindow* window,
    int width,
    int height
)
{
    glViewport(0, 0, width, height);
}
