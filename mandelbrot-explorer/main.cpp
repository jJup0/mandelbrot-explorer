#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int windowWidth = 1200;
int windowHeight = 800;

int max_iterations = 300;

GLuint shaderProgram;
GLuint vertexShader;
double zoom;
double pan_x;
double pan_y;
bool leftMousePressed = false;
double lastX, lastY;

bool use_double_precision = false;
bool auto_switch_double_precision = true;

float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

void reset_zoom_and_pan() {
    zoom = 0.5;
    pan_x = 0.0;
    pan_y = 0.0;
}

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

void update_all_shader_parameters() {
    glUseProgram(shaderProgram);
    if (use_double_precision) {
        GLint zoomDoubleLocation = glGetUniformLocation(shaderProgram, "zoom_double");
        GLint panDoubleLocation = glGetUniformLocation(shaderProgram, "pan_double");
        glUniform1d(zoomDoubleLocation, zoom);
        glUniform2d(panDoubleLocation, pan_x, pan_y);
    } else {
        GLint zoomLocation = glGetUniformLocation(shaderProgram, "zoom");
        GLint panLocation = glGetUniformLocation(shaderProgram, "pan");
        glUniform1f(zoomLocation, zoom);
        glUniform2f(panLocation, pan_x, pan_y);
    }

    GLint maxIterLocation = glGetUniformLocation(shaderProgram, "max_iterations");
    GLint aspectRatioLocation = glGetUniformLocation(shaderProgram, "aspectRatio");
    glUniform1i(maxIterLocation, max_iterations);
    glUniform1f(aspectRatioLocation, aspectRatio);
}

void change_max_iterations(bool increase) {
    if (increase) {
        max_iterations += max_iterations / 5;
        max_iterations = std::min(max_iterations, 1000);
    } else {
        max_iterations = static_cast<int>(static_cast<float>(max_iterations) * 0.8);
        max_iterations = std::max(max_iterations, 10);
    }
    printf("max iterations= %d\n", max_iterations);
    update_all_shader_parameters();
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Update viewport size
    glViewport(0, 0, width, height);

    // Update window size and aspect ratio
    windowWidth = width;
    windowHeight = height;
    aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    // Update the aspectRatio uniform in the shader
    update_all_shader_parameters();
}

void toggle_double_precision() {
    use_double_precision = !use_double_precision;
    std::string fragmentShaderSource;
    if (use_double_precision) {
        fragmentShaderSource = readShaderFile("shaders/fragmentShader_doubles.glsl");
    } else {
        fragmentShaderSource = readShaderFile("shaders/fragmentShader.glsl");
    }

    GLuint newFragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link the new shader program
    GLuint newShaderProgram = glCreateProgram();
    glAttachShader(newShaderProgram, vertexShader);
    glAttachShader(newShaderProgram, newFragmentShader);
    glLinkProgram(newShaderProgram);

    // Cleanup the old shader program
    glDeleteProgram(shaderProgram);

    // Use the new shader program
    glUseProgram(newShaderProgram);

    // Update the current shader program
    shaderProgram = newShaderProgram;

    // Cleanup the old fragment shader
    glDeleteShader(newFragmentShader);

    update_all_shader_parameters();
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Change zoom
    float viewport_width_before = 2.0f / zoom;

    float factor = std::pow(1.2f, static_cast<float>(yoffset));

    zoom *= factor;
    if (auto_switch_double_precision) {
        bool udp = zoom > 10'000;
        if (udp != use_double_precision) {
            printf("Double precision = %s\n", udp ? "true" : "false");
            toggle_double_precision();
        }
    }

    float viewport_width_after = 2.0f / zoom;
    float view_port_delta = viewport_width_before - viewport_width_after;

    // Get the mouse position in screen coordinates
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    float ndcX = (2.0f * mouseX) / windowWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / windowHeight;

    pan_x += ndcX * view_port_delta / 2.0f;
    pan_y += ndcY * view_port_delta / (2.0f * aspectRatio);

    update_all_shader_parameters();
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
        update_all_shader_parameters();

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

    printf("PAN (%.2f, %.2f), zoom: %.2f\n", pan_x, pan_y, zoom);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_R:
                reset_zoom_and_pan();
                // Update the shader with the new zoom level and pan
                update_all_shader_parameters();
                printf("Resetting zoom and pan");
                break;
            case GLFW_KEY_A:
                auto_switch_double_precision = !auto_switch_double_precision;
                printf("Auto double precision set to %s", auto_switch_double_precision ? "true" : "false");
                break;
            case GLFW_KEY_SPACE:
                print_loc_info(window);
                break;
            case GLFW_KEY_X:
                toggle_double_precision();
                printf("Double precision set to %s", use_double_precision ? "true" : "false");
                break;
            case GLFW_KEY_UP:
                change_max_iterations(true);
                break;
            case GLFW_KEY_DOWN:
                change_max_iterations(false);
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
    std::string vertexShaderSource = readShaderFile("shaders/vertexShader.glsl");
    std::string fragmentShaderSource = readShaderFile("shaders/fragmentShader.glsl");
    std::string fragmentShaderSourceDouble = readShaderFile("shaders/fragmentShader_doubles.glsl");

    if (vertexShaderSource.empty() || fragmentShaderSource.empty() || fragmentShaderSourceDouble.empty()) {
        std::cerr << "Failed to load shader sources" << std::endl;
        return -1;
    }

    // Compile shaders
    vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link shader program

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Cleanup shaders
    glDeleteShader(fragmentShader);

    // Create Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind VAO and VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // set vertices to be a flat square to act as a canvas
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
    glfwSetScrollCallback(window, scrollCallback);
    // Set up mouse button callback
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    // Set up cursor position callback
    glfwSetCursorPosCallback(window, cursorPosCallback);

    reset_zoom_and_pan();
    update_all_shader_parameters();

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

    // Cleanup shaders
    glDeleteShader(vertexShader);

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}
