#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// #define GLM_ENABLE_EXPERIMENTAL // TODO: Remove at end
// #include <glm/ext.hpp>

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
    int sampleRate;
    float *vertexData;
    int frameSize;
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
            "color = vec4(0.1f, 1.0f, 0.4f, 1.0f);\n"
        "}\0";
public:
    Waveform1(std::vector<std::vector<double>> audioSignal, int sampleRate){
        this->audioSignal = audioSignal;
        this->sampleRate = sampleRate;
        frameSize = sampleRate * 10;
        numVertices = frameSize * 3;
        vertexDataSize = numVertices * sizeof(float);
        vertexData = new float[numVertices]; // TODO: Make Double
        for(int i = 0; i < numVertices; i+=3){
            vertexData[i] = ((float)i/3)/frameSize - 0.5f;
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

    void draw(int currentMillisecond){
        for(int i = 0; i < numVertices; i+=3){
            vertexData[i+1] = audioSignal[0][(int)(i/3.0f) + (currentMillisecond/1000.0f) * sampleRate] / 2;
        }
        offset++;

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);      
        glUseProgram(shaderProgram);
        glDrawArrays(GL_LINE_STRIP, 0, frameSize);
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
        "void main(){\n"
            // "gl_Position = vec4(point, 1.0f);\n"
            "gl_Position = vec4((point.y + 0.5f) * cos(point.x * 6.2f), (point.y + 0.5f)* sin(point.x * 6.2f), point.z, 1.0);\n"
        "}\0"; // TODO: Load from file 
    const char* fragmentShaderSrc = 
        "#version 330 core\n"
        "out vec4 color;\n"
        "void main(){\n"
            "color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\0";
    public:
        Waveform2(std::vector<std::vector<double>> audioSignal, int sampleRate){
            this->audioSignal = audioSignal;
            this->sampleRate = sampleRate;
            frameSize = 1024 / 2;
            frame = new fftw_complex[frameSize];
            frequencyBands = new fftw_complex[frameSize];
            plan = fftw_plan_dft_1d(frameSize, frame, frequencyBands, FFTW_FORWARD,  FFTW_MEASURE);   
            startIndex = (20.0f * frameSize) / sampleRate ;
            size = ((20000.0f * frameSize)/sampleRate) - startIndex;
            size /= 2; // TODO: Not properly sizing between 20-20kHz
            std::cout << "size" << size << std::endl;

            numVertices = size * 3;
            frameDataSize = numVertices * sizeof(double);
            frameData = new double[numVertices]; 
            for(int i = 0; i < numVertices; i+=3){
                frameData[i] = ((double)i/3)/size; // TODO: Remove if not using circle - 0.5f;
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

            double a0 = 0.5;
            double a1 = 1 - a0;
            double w;
            for(int i = 0; i < frameSize; i++){
                w = a0 - a1 * cos((2*M_PI*i)/frameSize);
                frame[i][0] *= w;
            }

            fftw_execute(plan);
            int j = startIndex;
          
            int nObs = 10;
            for(int i = 0; i < numVertices; i+=3){ // TODO: Handle out of bound
                double real = frequencyBands[j][0];
                double complex = frequencyBands[j][1];
                double curr = sqrt(real * real + complex * complex) / (frameSize/64); 
                // curr = log(curr) / log(10);
                curr /= frameSize/128;
                int sum = 0;
                int n = 0;
                for(int x = (i-3)/3; x >= 0 && x >= (i-3-nObs*3)/3; x -= 3){
                    sum += frameData[x+1];
                    n++;
                }
                // frameData[i+1] = (sum + curr) / (n+1);
                double limit = 0.3f;
                if(curr <= limit) frameData[i+1] = curr; //
                else frameData[i+1] = limit;
                j++;
            }

            glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(double), (void*)0);
            glEnableVertexAttribArray(0);
            // glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, frameDataSize, frameData, GL_STATIC_DRAW);
            glUseProgram(shaderProgram);
            glPointSize(3.0f); // TODO: Circle feature
            glDrawArrays(GL_POINTS, 0, size);
        }

        ~Waveform2(){
            delete frame;
            delete frequencyBands;
            delete frameData;
            fftw_destroy_plan(plan);
            fftw_cleanup();

        }
};

class Waveform3 {
    std::vector<std::vector<double>> audioSignal;
    int frameSize;
    fftw_complex *frame, *frequencyBands; 
    fftw_plan plan;
    int startIndex;
    int size;
    double **frameData; // TODO: Careful when merge
    int sampleRate;
    unsigned int frameDataSize; // TODO: Careful when merge
    int num;
    unsigned int numVertices;
    unsigned int offset = 0;
    unsigned int vao;
    unsigned int *vbo;
    unsigned int shaderProgram;
    const char* vertexShaderSrc = 
        "#version 330 core\n" 
        "layout (location = 0) in vec3 point;\n"
        "uniform mat4 view;\n"
        "uniform int offset;\n"
        "out vec3 p;\n"
        "void main(){\n"
            "gl_Position = view * vec4(point.x, point.y, offset * 0.01f, 1.0);\n"
            "p = point;\n"
        "}\0"; // TODO: Load from file 
    const char* fragmentShaderSrc = 
        "#version 330 core\n"
        "out vec4 color;\n"
        "uniform int offset;\n"
        "in vec3 p;\n"
        "void main(){\n"
            "color = vec4(1.0f, 0.1f, 0.2f, 1.0f);\n"
        "}\0";
    public:
        Waveform3(std::vector<std::vector<double>> audioSignal, int sampleRate){
            this->audioSignal = audioSignal;
            this->sampleRate = sampleRate;
            frameSize = 256;
            frame = new fftw_complex[frameSize];
            frequencyBands = new fftw_complex[frameSize];
            plan = fftw_plan_dft_1d(frameSize, frame, frequencyBands, FFTW_FORWARD,  FFTW_MEASURE);   
            startIndex = (20.0f * frameSize) / sampleRate ;
            size = ((20000.0f * frameSize)/sampleRate) - startIndex;
            std::cout << "size" << size << std::endl;

            numVertices = size * 3;
            frameDataSize = numVertices * sizeof(double);
            num = 30;
            frameData = new double*[num]; 
            for(int i = 0; i < num; i++){
                frameData[i] = new double[numVertices];
            }

            for(int j = 0;j < num; j++){
                for(int i = 0; i < numVertices; i+=3){
                    frameData[j][i] = ((double)i/3)/size - 0.5f;
                    frameData[j][i+1] = 0.0f;
                    frameData[j][i+2] = 0.1f * j;
                }
            }
            
            // glGenVertexArrays(1, &vao);
            // glBindVertexArray(vao);
            
            vbo = new unsigned int[num];
            glGenBuffers(num, vbo);
            // for(int i = 0; i < 10; i++){
            //     glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
            //     glBufferData(GL_ARRAY_BUFFER, frameDataSize, frameData[i], GL_DYNAMIC_DRAW);
            //     glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(double), (void*)0);
            //     glEnableVertexAttribArray(0);
            // }

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

            glUseProgram(shaderProgram);
            glm::mat4 view = glm::mat4(1.0f);
            view = glm::scale(view, glm::vec3(2.0f, 2.0f, 0.5f));
            view = glm::rotate(view, 30.0f * (3.14f / 180), glm::vec3(1.0f, 0.0f, 0.0f));
            view = glm::rotate(view, -140.0f * (3.14f / 180), glm::vec3(0.0f, 1.0f, 0.0f));
            view = glm::translate(view, glm::vec3(-0.0f, 0.0f, 0.0f));
            // std::cout << glm::to_string(view) << std::endl;
            int viewUniformLoc = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewUniformLoc, 1, GL_FALSE, glm::value_ptr(view));
        }

        void draw(int currentMillisecond, bool swap){
            if(swap){
                double *f = frameData[num-1];
                for(int i = num-1; i > 0; i--){
                    frameData[i] = frameData[i-1];
                }
                frameData[0] = f;
                
                for(int i = 0; i < frameSize; i++){
                    frame[i][0] = audioSignal[0][(currentMillisecond/1000.0f) * sampleRate + i];
                    frame[i][1] = 0;
                }

                double a0 = 0.5f; // TODO: Move to windowing function code.
                double a1 = 1 - a0;
                double w;
                for(int i = 0; i < frameSize; i++){
                    w = a0 - a1 * cos((2*M_PI*i)/frameSize);
                    frame[i][0] *= w;
                }

                fftw_execute(plan);
                int j = startIndex;
                for(int i = 0; i < numVertices; i+=3){ // TODO: Handle out of bound
                    double real = frequencyBands[j][0];
                    double complex = frequencyBands[j][1];
                    frameData[0][i+1] = sqrt(real * real + complex * complex) / (frameSize/4); 

                    double limit = 0.2f;
                    if(frameData[0][i+1] > limit) frameData[0][i+1] = limit;
                    j++;
                }
            }

            glEnable(GL_BLEND); 
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
            for(int i = 0; i < num; i++){
                // glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, vbo[i]); // TODO: Update all
                glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(double), (void*)0);
                glUseProgram(shaderProgram);
                glEnableVertexAttribArray(0);
                int offsetUniformLoc = glGetUniformLocation(shaderProgram, "offset");
                glUniform1i(offsetUniformLoc, i);
                if(swap){
                    glBufferData(GL_ARRAY_BUFFER, frameDataSize, frameData[i], GL_DYNAMIC_DRAW);
                }
                glPointSize(1.0f);
                glDrawArrays(GL_POINTS, 0, size);
            }
        }

        ~Waveform3(){
            delete frame;
            delete frequencyBands;
            for(int i = 0; i < 0; i++){
                delete frameData[i]; // TODO: Check this
            }
            delete frameData;
            fftw_destroy_plan(plan);
            fftw_cleanup();
        }
};

class Waveform4 {
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
        "uniform mat4 view;\n"
        "void main(){\n"
            "gl_Position = vec4(point.x, point.y, point.z, 1.0);\n"
        "}\0"; // TODO: Load from file 
    const char* fragmentShaderSrc = 
        "#version 330 core\n"
        "out vec4 color;\n"
        "void main(){\n"
            "color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\0";
    public:
        Waveform4(std::vector<std::vector<double>> audioSignal, int sampleRate){
            this->audioSignal = audioSignal;
            this->sampleRate = sampleRate;
            frameSize = 1024 * 2;
            frame = new fftw_complex[frameSize];
            frequencyBands = new fftw_complex[frameSize];
            plan = fftw_plan_dft_1d(frameSize, frame, frequencyBands, FFTW_FORWARD,  FFTW_MEASURE);   
            startIndex = (20.0f * frameSize) / sampleRate ;
            size = ((20000.0f * frameSize)/sampleRate) - startIndex;
            std::cout << "size" << size << std::endl;

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

            glm::mat4 view = glm::mat4(1.0f);
            // std::cout << glm::to_string(view) << std::endl;
            // view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
            int viewUniformLoc = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewUniformLoc, 1, GL_FALSE, glm::value_ptr(view));
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

        ~Waveform4(){
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

    GLFWwindow* window = glfwCreateWindow(500, 500, APPNAME, NULL, NULL);
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
    Waveform1 waveform1(audioFile.samples, sampleRate);
    Waveform2 waveform2(audioFile.samples, sampleRate);
    Waveform3 waveform3(audioFile.samples, sampleRate);

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

    int choice = 1;
    
    glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
    int c = 1;
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        
        ma_int64 currentMillisecond = ma_engine_get_time_in_milliseconds(&engine);
        switch(choice){
            case 1: waveform1.draw(currentMillisecond); break;
            case 2: waveform2.draw(currentMillisecond); break;
            case 3: 
                if(c == 5){
                    waveform3.draw(currentMillisecond, true);
                    c = 0;
                } else {
                    waveform3.draw(currentMillisecond, false);
                    c++; 
                }
                break;
            default: break;
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS){
            choice = 1;
        } if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS){
            choice = 2;
        } if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS){
            choice = 3;
        }
    }

    ma_engine_uninit(&engine);
    ma_sound_uninit(&sound);
    glfwTerminate();

    return 0;
}
 