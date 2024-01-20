#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#define println(message)                   \
    do {                                   \
        std::cout << message << std::endl; \
    } while (false)

#define print(message)        \
    do {                      \
        std::cout << message; \
    } while (false)

std::string readShaderFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }

    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

GLuint compileShader(GLenum shaderType, const std::string& source) {
    GLuint shader = glCreateShader(shaderType);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    // Check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

int windowWidth = 1200;
int windowHeight = 800;

GLuint shaderProgram;
double zoom = 1.0;
double pan_x = 0.0; 
double pan_y = 0.0;
bool leftMousePressed = false;
double lastX, lastY;

float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

void updateShaderParameters() {
    GLint zoomLocation = glGetUniformLocation(shaderProgram, "zoom");
    GLint panLocation = glGetUniformLocation(shaderProgram, "pan");
    GLint aspectRatioLocation = glGetUniformLocation(shaderProgram, "aspectRatio");

    glUseProgram(shaderProgram);
    glUniform1f(zoomLocation, zoom);
    glUniform2f(panLocation, pan_x, pan_y);
    glUniform1f(aspectRatioLocation, aspectRatio);

    // println("pan: (" << pan_x << ", " << pan_y << ") || Zoom: " << zoom);
    // float pan_x = pan_x
    // println("top left: (" << pan_x << ", " << pan_y << ")");
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Update viewport size
    glViewport(0, 0, width, height);

    // Update window size and aspect ratio
    windowWidth = width;
    windowHeight = height;
    aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    // Update the aspectRatio uniform in the shader
    updateShaderParameters();
}

void scrollCallback4(GLFWwindow* window, double xoffset, double yoffset) {
    // Change zoom
    float viewport_width_before = 2.0f / zoom;

    float factor = std::pow(1.2f, static_cast<float>(yoffset));
    zoom *= factor;

    float viewport_width_after = 2.0f / zoom;
    float view_port_delta = viewport_width_before - viewport_width_after;

    // Get the mouse position in screen coordinates
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    float ndcX = (2.0f * mouseX) / windowWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / windowHeight;

    pan_x += ndcX * view_port_delta / 2.0f;
    pan_y += ndcY * view_port_delta / (2.0f * aspectRatio);

    updateShaderParameters();
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftMousePressed = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            leftMousePressed = false;
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (leftMousePressed) {
        // Calculate the difference in mouse position
        double deltaX = xpos - lastX;
        double deltaY = ypos - lastY;

        // Update pan based on the difference
        pan_x -= 2 * static_cast<double>(deltaX) / (windowWidth * zoom);
        pan_y += 2 * static_cast<double>(deltaY) / (windowHeight * aspectRatio * zoom);

        // Update the shader with the new zoom level and pan
        updateShaderParameters();

        // Update last mouse position
        lastX = xpos;
        lastY = ypos;
    }
}

void print_loc_info(GLFWwindow* window) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    float ndcX = (2.0f * mouseX) / windowWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / windowHeight;

    float screenTopLeftX = (-1.0f) / zoom + pan_x;
    float screenTopLeftY = (1.0f) / zoom + pan_y;

    print("MOUSE: (" << mouseX << ", " << mouseY << ")  ");
    print("NDC: (" << ndcX << ", " << ndcY << ")  ");
    println("Top left coords: (" << screenTopLeftX << ", " << screenTopLeftY << ")");
    printf("PAN (%.2f, %.2f), zoom: %.2f\n", pan_x, pan_y, zoom);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_R:
                zoom = 1.0f;
                pan_x = 0.0f;
                pan_y = 0.0f;

                // Update the shader with the new zoom level and pan
                updateShaderParameters();
                break;
            case GLFW_KEY_SPACE:
                print_loc_info(window);
                break;
        }
    }
}

void updateFPSCounter(GLFWwindow* window) {
    static double lastTime = glfwGetTime();
    static int frameCount = 0;
    static int fps = 0;

    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime;

    frameCount++;
    if (delta >= 1.0) {
        fps = static_cast<int>(frameCount / delta);
        frameCount = 0;
        lastTime = currentTime;

        // Print FPS to console
        // std::cout << "FPS: " << fps << std::endl;
    }
    // Display FPS on the window title bar
    std::ostringstream title;
    title << "Mandelbrot Explorer - FPS: " << fps;
    glfwSetWindowTitle(window, title.str().c_str());
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Mandelbrot Set", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Load shaders from external files
    std::string vertexShaderSource = readShaderFile("vertexShader.glsl");
    std::string fragmentShaderSource = readShaderFile("fragmentShader.glsl");

    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        std::cerr << "Failed to load shader sources" << std::endl;
        return -1;
    }

    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link shader program

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Cleanup shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind VAO and VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Set vertex data
    float vertices[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // frame resize callback
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Set up key callback
    glfwSetKeyCallback(window, keyCallback);

    // Set up scroll callback
    glfwSetScrollCallback(window, scrollCallback4);

    // Set up mouse button callback
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // Set up cursor position callback
    glfwSetCursorPosCallback(window, cursorPosCallback);

    updateShaderParameters();
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        updateFPSCounter(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}
