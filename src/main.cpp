#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>

void resize_frame_buffer_on_window(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

int main(){
    int glfwInitialized = glfwInit();
    if(glfwInitialized == GLFW_NOT_INITIALIZED){
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

    glfwSetFramebufferSizeCallback(window, resize_frame_buffer_on_window);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
