#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <audiofile/AudioFile.h>
#include <miniaudio.h>
#include <fftw3/fftw3.h>

#include <iostream>
#include <vector>
#include <math.h>

#include <CircularWaveform.h>
#include <PcmWaveform.h>
#include <TerrainWaveform.h>
#include <Bloom.h>

#include <AudioFFT.h>

#define APPNAME "Stradivarius"

void resize_frame_buffer_on_window(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height); // TODO: handle resize properly
}

class Scene1 {
    std::vector<std::vector<double>> audioSignal;
    int sampleRate;

    int frameSize;
    unsigned int numVertices;
    float *vertexData;

    PcmWaveform *pcmWaveform;
public:
    Scene1(std::vector<std::vector<double>> audioSignal, int sampleRate){
        this->audioSignal = audioSignal;
        this->sampleRate = sampleRate;

        frameSize = sampleRate * 10;
        numVertices = frameSize * 3;
        vertexData = new float[numVertices]; 
        for(int i = 0; i < numVertices; i+=3){
            vertexData[i] = ((float)i/3)/frameSize - 0.5f;
            vertexData[i+1] = 0.0f;
            vertexData[i+2] = 0.0f;
        }

        pcmWaveform = new PcmWaveform(frameSize, "./src/shaders/pcm.vs", "./src/shaders/pcm.fs");
    }

    void draw(int currentMillisecond){
        for(int i = 0; i < numVertices; i+=3){
            vertexData[i+1] = audioSignal[0][(int)(i/3.0f) + (currentMillisecond/1000.0f) * sampleRate] / 2;
        }
        pcmWaveform->draw(vertexData);
    }

    ~Scene1(){
        delete vertexData;
        delete pcmWaveform;
    }
};

class Scene2 {
    std::vector<std::vector<double>> audioSignal;
    int sampleRate;

    // FFT
    int frameSize;
    fftw_complex *frame, *frequencyBands; 
    fftw_plan plan;
    int startIndex;
    int size;
    float *frameData; // TODO: Careful when merge

    unsigned int numVertices;
    CircularWaveform *circularWaveform;
    Bloom *bloom;
    public:
        Scene2(std::vector<std::vector<double>> audioSignal, int sampleRate, int width, int height){
            this->audioSignal = audioSignal;
            this->sampleRate = sampleRate;
            frameSize = 1024 / 2;
            frame = new fftw_complex[frameSize];
            frequencyBands = new fftw_complex[frameSize];
            plan = fftw_plan_dft_1d(frameSize, frame, frequencyBands, FFTW_FORWARD,  FFTW_MEASURE);   
            startIndex = (20.0f * frameSize) / sampleRate;
            size = ((20000.0f * frameSize)/sampleRate) - startIndex;
            size /= 2; // TODO: Not properly sizing between 20-20kHz

            numVertices = size * 3;
            frameData = new float[numVertices]; 
            for(int i = 0; i < numVertices; i+=3){
                frameData[i] = ((double)i/3)/size; // TODO: Remove if not using circle - 0.5f;
                frameData[i+1] = 0.0f;
                frameData[i+2] = 0.0f;
            }
            
            circularWaveform = new CircularWaveform(size, "src/shaders/circular_spectrum.vs", "src/shaders/circular_spectrum.fs");
            bloom = new Bloom(width, height);
        }

        void draw(int currentMillisecond){
            for(int i = 0; i < frameSize; i++){
                frame[i][0] = audioSignal[0][(currentMillisecond/1000.0f) * sampleRate + i];
                frame[i][1] = 0;
            }

            double a0 = 0.5; // TODO: window Function
            double a1 = 1 - a0;
            double w;
            for(int i = 0; i < frameSize; i++){
                w = a0 - a1 * cos((2*M_PI*i)/frameSize);
                frame[i][0] *= w;
            }

            fftw_execute(plan);
            int j = startIndex;
          
            int nObs = 0;
            for(int i = 0; i < numVertices; i+=3){ // TODO: Handle out of bound
                double real = frequencyBands[j][0];
                double complex = frequencyBands[j][1];
                double curr = sqrt(real * real + complex * complex) / (frameSize/64); 
                curr /= frameSize/128;
                int sum = 0;
                int n = 0;
                for(int x = (i-3)/3; x >= 0 && x >= (i-3-nObs*3)/3; x -= 3){
                    sum += frameData[x+1];
                    n++;
                }
                double limit = 0.3f;
                if(curr <= limit) frameData[i+1] = curr; //
                else frameData[i+1] = limit;
                j++;
            }

            bloom->start();

            circularWaveform->draw(frameData);

            bloom->end();
        }

        ~Scene2(){
            delete frame;
            delete frequencyBands;
            delete frameData;
            // glDeleteFramebuffers(2, fbo);
            // delete fbo;
            // glDeleteTextures(2, texture);
            // delete texture;
            // delete brightShader;
            delete circularWaveform;
            fftw_destroy_plan(plan);
            fftw_cleanup();
        }
};

class Scene3 {
    std::vector<std::vector<double>> audioSignal;
    int sampleRate;
    int frameSize;
    int clippedSize;
    float **frameData; // TODO: Careful when merge
    int numRows;
    TerrainWaveform *terrainWaveform;
    Bloom *bloom;
    AudioFFT *audioFFT; 
public:
    Scene3(std::vector<std::vector<double>> audioSignal, int sampleRate, int width, int height){
        this->audioSignal = audioSignal;
        this->sampleRate = sampleRate;
        frameSize = 256;
        audioFFT = new AudioFFT(frameSize);
        clippedSize = audioFFT->getClippedSize(20, 20000, sampleRate);
        int numVertices = 3 * clippedSize;
        numRows = 30;

        frameData = new float*[numRows]; 
        for(int row = 0; row < numRows; row++){
            frameData[row] = new float[numVertices];
        }

        for(int row = 0; row < numRows; row++){
            for(int i = 0; i < numVertices; i+=3){
                frameData[row][i] = ((double)i/3)/clippedSize - 0.5f;
                frameData[row][i+1] = 0.0f;
                frameData[row][i+2] = 0.1f * row;
            }
        }
        
        terrainWaveform = new TerrainWaveform(clippedSize, numRows, "src/shaders/terrain_spectrum.vs", "src/shaders/terrain_spectrum.fs");
        bloom = new Bloom(width, height);   
    }

    void draw(int currentMillisecond, bool swap){

        int numVertices = 3 * clippedSize;

        float *f = frameData[numRows-1];
        for(int i = numRows-1; i > 0; i--){
            frameData[i] = frameData[i-1];
        }
        frameData[0] = f;

        float *audioFrame = new float[frameSize];
        for(int i = 0; i < frameSize; i++){
            audioFrame[i] = (float)audioSignal[0][(currentMillisecond/1000.0f) * sampleRate + i];
        }
        float *spectrum = new float[clippedSize];
        audioFFT->fftMagnitudesClipped(audioFrame, spectrum, 20, 20000, sampleRate);

        int j = 0;
        for(int i = 0; i < numVertices; i+=3){
            frameData[0][i+1] = spectrum[j++];
        }

        bloom->start();

        terrainWaveform->draw(frameData);

        bloom->end();
    }

    ~Scene3(){ // TODO: Check if all destroyed
        for(int i = 0; i < 0; i++){
            delete[] frameData[i]; 
        }
        delete[] frameData;
        delete terrainWaveform;
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

    const char* songPath = "audio/riff.wav";
    
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
    
    AudioFile<double> audioFile;
    audioFile.load(songPath);
    audioFile.printSummary();
    int sampleRate = audioFile.getSampleRate();

    Scene1 scene1(audioFile.samples, sampleRate);
    Scene2 scene2(audioFile.samples, sampleRate, frameBufferWidth, frameBufferHeight);
    Scene3 scene3(audioFile.samples, sampleRate, frameBufferWidth, frameBufferHeight);

    int choice = 1;

    unsigned int vao;
    glGenVertexArrays(1, &vao); // TODO: Move to each waveforms
    glBindVertexArray(vao);
    
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
        
        ma_int64 currentMillisecond = ma_engine_get_time_in_milliseconds(&engine);
        switch(choice){
            case 1: scene1.draw(currentMillisecond); break;
            case 2: scene2.draw(currentMillisecond); break;
            case 3: scene3.draw(currentMillisecond, true); break;
            default: break;
        }
        
        if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) choice = 1;
        if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) choice = 2;
        if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) choice = 3;
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    ma_engine_uninit(&engine);
    ma_sound_uninit(&sound);
    glfwTerminate();

    return 0;
}
 