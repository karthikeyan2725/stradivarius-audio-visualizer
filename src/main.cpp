#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <audiofile/AudioFile.h>
#include <miniaudio.h>
#include <fftw3/fftw3.h>

#include <iostream>
#include <vector>
#include <math.h>

#define APPNAME "Stradivarius"
#define ONE_SECOND 1000
#define FPS 120

void resize_frame_buffer_on_window(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

class Waveform1 {
    std::vector<std::vector<double>> audioSignal;
    float *vertexData;
    unsigned int vertexDataSize;
    unsigned int numVertices;
    unsigned int offset = 0;
    unsigned int vao;
    unsigned int vbo;
    unsigned int shaderProgram;
    const char* vertexShaderSrc = 
        "#version 330 core\n" 
        "layout (location = 0) in vec3 point;\n"
        "void main(){\n"
            "gl_Position = vec4(point.x, point.y, point.z, 1.0);\n"
        "}\0"; // TODO: Load from file 
    const char* fragmentShaderSrc = 
        "#version 330 core\n"
        "out vec4 color;"
        "void main(){\n"
            "color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\0";
public:
    Waveform1(std::vector<std::vector<double>> audioSignal){
        this->audioSignal = audioSignal;
        numVertices = 16000 * 3;
        vertexDataSize = numVertices * sizeof(float);
        vertexData = new float[numVertices]; // TODO: Make Double
        for(int i = 0; i < numVertices; i+=3){
            vertexData[i] = ((float)i/3)/16000 - 0.5f;
            vertexData[i+1] = 0.0f;
            vertexData[i+2] = 0.0f;
        }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

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

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(shaderProgram, 1000, NULL, message);
            std::cout << "Failed to link shaders: " << message << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void draw(){
        for(int i = 0; i < numVertices; i+=3){
            vertexData[i+1] = audioSignal[0][i + (int)((float)16000 / FPS) * offset];
        }
        offset++;
        glBindBuffer(GL_ARRAY_BUFFER, vao);
        glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_STATIC_DRAW);
        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINE_STRIP, 0, 16000);
    }

    ~Waveform1(){
        delete vertexData;
    }
};

class Waveform2 {
    std::vector<std::vector<double>> audioSignal;
    int frameSize;
    fftw_complex *frame, *frequencyBands; 
    fftw_plan plan;
    int startIndex;
    int size;
    double *frameData; // TODO: Careful when merge
    int sampleRate;
    unsigned int frameDataSize; // TODO: Careful when merge
    unsigned int numVertices;
    unsigned int offset = 0;
    unsigned int vao;
    unsigned int vbo;
    unsigned int shaderProgram;
    const char* vertexShaderSrc = 
        "#version 330 core\n" 
        "layout (location = 0) in vec3 point;\n"
        "out vec3 oPoint;\n"
        "void main(){\n"
            "gl_Position = vec4(point.x, point.y, point.z, 1.0);\n"
            "oPoint = point;\n"
        "}\0"; // TODO: Load from file 
    const char* fragmentShaderSrc = 
        "#version 330 core\n"
        "out vec4 color;\n"
        "in vec3 oPoint;\n"
        "void main(){\n"
            "color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\0";
    public:
        Waveform2(std::vector<std::vector<double>> audioSignal, int sampleRate){
            this->audioSignal = audioSignal;
            this->sampleRate = sampleRate;
            frameSize = 1024 * 2;
            frame = new fftw_complex[frameSize];
            frequencyBands = new fftw_complex[frameSize];
            plan = fftw_plan_dft_1d(frameSize, frame, frequencyBands, FFTW_FORWARD,  FFTW_MEASURE);   
            startIndex = (20.0f * frameSize) / sampleRate ;
            size = ((20000.0f * frameSize)/sampleRate) - startIndex;

            numVertices = size * 3;
            frameDataSize = numVertices * sizeof(double);
            frameData = new double[numVertices]; 
            for(int i = 0; i < numVertices; i+=3){
                frameData[i] = ((double)i/3)/size - 0.5f;
                frameData[i+1] = 0.0f;
                frameData[i+2] = 0.0f;
            }
            
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, frameDataSize, frameData, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(double), (void*)0);
            glEnableVertexAttribArray(0);

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

            shaderProgram = glCreateProgram();
            glAttachShader(shaderProgram, vertexShader);
            glAttachShader(shaderProgram, fragmentShader);
            glLinkProgram(shaderProgram);
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
            if(!success){
                glGetProgramInfoLog(shaderProgram, 1000, NULL, message);
                std::cout << "Failed to link shaders: " << message << std::endl;
            }

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
        }

        void draw(int currentMillisecond){
            for(int i = 0; i < frameSize; i++){
                frame[i][0] = audioSignal[0][(currentMillisecond/1000.0f) * sampleRate + i];
                frame[i][1] = 0;
            }
            fftw_execute(plan);
            int j = startIndex;
            for(int i = 0; i < numVertices; i+=3){ // TODO: Handle out of bound
                double real = frequencyBands[j][0];
                double complex = frequencyBands[j][1];
                frameData[i+1] =  sqrt(real * real + complex * complex) / (frameSize/8); 
                j++;
            }

            glBindBuffer(GL_ARRAY_BUFFER, vao);
            glBufferData(GL_ARRAY_BUFFER, frameDataSize, frameData, GL_STATIC_DRAW);
            glUseProgram(shaderProgram);
            glBindVertexArray(vao);
            glDrawArrays(GL_LINE_STRIP, 0, size);
        }

        ~Waveform2(){
            delete frame;
            delete frequencyBands;
            delete frameData;
            fftw_destroy_plan(plan);
            fftw_cleanup();

        }
};

int main(){
    int glfwInitialized = glfwInit();
    if(glfwInitialized == GLFW_FALSE){
        std::cout << "Failed to Initalize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, APPNAME, NULL, NULL);
    if(window == NULL){
        std::cout << "Failed to create a GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initalize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    int frameBufferWidth, frameBufferHeight;
    glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
    glViewport(0, 0, frameBufferWidth, frameBufferHeight);

    glfwSetFramebufferSizeCallback(window, resize_frame_buffer_on_window);

    const char* songPath = "audio/escape.wav";
    AudioFile<double> audioFile;
    audioFile.load(songPath);
    audioFile.printSummary();
    int sampleRate = audioFile.getSampleRate();
    // Waveform1 waveform1(audioFile.samples);
    Waveform2 Waveform2(audioFile.samples, sampleRate);

    ma_result result;
    ma_engine engine;
    ma_sound sound;
    result = ma_engine_init(NULL, &engine);
    if(result != MA_SUCCESS){
        std::cout << "Failed to initialize Miniaudio Engine" << std::endl;
        glfwTerminate();
        return -1;
    } 
    result = ma_sound_init_from_file(&engine, songPath, MA_SOUND_FLAG_ASYNC, NULL, NULL, &sound);
    if(result != MA_SUCCESS){
        std::cout << "Failed to Load Audio" << std::endl;
        glfwTerminate();
        return -1;
    } 
    ma_sound_start(&sound);

    glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        
        ma_int64 currentMillisecond = ma_engine_get_time_in_milliseconds(&engine);
        // waveform1.draw();
        Waveform2.draw(currentMillisecond); 
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ma_engine_uninit(&engine);
    ma_sound_uninit(&sound);
    glfwTerminate();

    return 0;
}
 