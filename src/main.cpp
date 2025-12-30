#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<cstdlib>

void resize_frame_buffer_on_window(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

float generateRandomFloat(int min, int max){
    int decimal = min + (rand() % (int)(max - min));
    float fractional = ((float)rand() / (float) RAND_MAX);
    return (float)(decimal + fractional);
}

int main(){
    int glfwInitialized = glfwInit();
    if(glfwInitialized == GLFW_FALSE){
        std::cout << "Failed to Initalize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Stradivarius", NULL, NULL);
    if(window == NULL){
        std::cout << "Failed to create a GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initalize GLAD" << std::endl;
        return -1;
    }

    int frameBufferWidth, frameBufferHeight;
    glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
    glViewport(0, 0, frameBufferWidth, frameBufferHeight);

    glfwSetFramebufferSizeCallback(window, resize_frame_buffer_on_window);

    std::cout << "Strad has started..." << std::endl;

    float signal[300] = {0};
    for(int i = 0; i < 300; i+=3){
        signal[i] = (i/3 * 0.01f) + (-0.5f);
        signal[i+1] = 0.25f;
        signal[i+2] = 0.0f;
    }

    unsigned int signalVao;
    glGenVertexArrays(1, &signalVao);
    glBindVertexArray(signalVao);

    unsigned int signalVbo;
    glGenBuffers(1, &signalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, signalVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(signal), signal, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    const char* vertexShaderSrc = 
        "#version 330 core\n" 
        "layout (location = 0) in vec3 point;\n"
        "void main(){\n"
           "gl_Position = vec4(point.x, point.y, point.z, 1.0);\n"
        "}\0";
    const char* fragmentShaderSrc = 
        "#version 330 core\n"
        "out vec4 color;"
        "void main(){\n"
            "color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\0";

    int success;
    char message[1000];
  
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 1000, NULL, message);
        std::cout << "Failed to compile Vertex Shader: " << message << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 1000, NULL, message);
        std::cout << "Failed to compile Fragment Shader: " << message << std::endl;
    }
    
    unsigned int shader = glCreateProgram();
    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shader, 1000, NULL, message);
        std::cout << "Failed to link shaders: " << message << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    fflush(0);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    while(!glfwWindowShouldClose(window)){
        for(int i = 0; i < 300; i+=3){
            signal[i+1] = generateRandomFloat(0, 1);
            std::cout << signal[i+1] << std::endl;
        }
        glBindBuffer(GL_ARRAY_BUFFER, signalVao);
        glBufferData(GL_ARRAY_BUFFER, sizeof(signal), signal, GL_STATIC_DRAW);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);
        glBindVertexArray(signalVao);
        glDrawArrays(GL_LINE_STRIP, 0, 100);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
