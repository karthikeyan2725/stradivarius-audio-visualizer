#pragma once
#include <vector>
#include <PcmWaveform.h>
#include <Bloom.h>
#include <AudioFFT.h>

class Scene1 {
    std::vector<std::vector<double>> audioSignal;
    unsigned int sampleRate;
    unsigned int frameSize;
    unsigned int numVertices;
    float *vertexData;
    PcmWaveform *pcmWaveform;
public:
    Scene1(std::vector<std::vector<double>> audioSignal, int sampleRate){
        this->audioSignal = audioSignal;
        this->sampleRate = sampleRate;

        // Configurations
        const float xSpace = 0.5f;
        frameSize = sampleRate * 10;

        numVertices = frameSize * 3;
        vertexData = new float[numVertices]; 
        for(int i = 0; i < numVertices; i+=3){
            vertexData[i] = (i/3.0f)/frameSize - xSpace; 
            vertexData[i+1] = 0.0f;
            vertexData[i+2] = 0.0f;
        }

        pcmWaveform = new PcmWaveform(frameSize, "./src/shaders/pcm.vs", "./src/shaders/pcm.fs");
    }

    void draw(int currentMillisecond){
        for(int i = 0; i < numVertices; i+=3){
            vertexData[i+1] = audioSignal[0][(int)(i/3.0f) + (currentMillisecond/1000.0f) * sampleRate];
        }
        pcmWaveform->draw(vertexData);
    }

    ~Scene1(){
        delete vertexData;
        delete pcmWaveform;
    }
};
