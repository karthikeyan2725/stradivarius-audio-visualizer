#pragma once
#include <vector>
#include <TerrainWaveform.h>
#include <AudioFFT.h>

class Scene3 {
    std::vector<std::vector<double>> audioSignal;
    int sampleRate;
    int frameSize;
    int clippedSize;
    float **frameData; 
    int numRows;
    TerrainWaveform *terrainWaveform;
    AudioFFT *audioFFT; 
public:
    Scene3(std::vector<std::vector<double>> audioSignal, int sampleRate, int width, int height){
        this->audioSignal = audioSignal;
        this->sampleRate = sampleRate;

        // Configurations
        frameSize = 256;
        numRows = 30;
        const float xSpace = 0.5f;
        const float zSpace = 0.1f;
        
        audioFFT = new AudioFFT(frameSize);
        clippedSize = audioFFT->getClippedSize(20, 20000, sampleRate); // TODO: Not properly sizing between 20-20kHz
        const int numVertices = 3 * clippedSize;
        frameData = new float*[numRows]; 
        for(int row = 0; row < numRows; row++){
            frameData[row] = new float[numVertices];
        }

        for(int row = 0; row < numRows; row++){
            for(int i = 0; i < numVertices; i+=3){
                frameData[row][i] = ((double)i/3)/clippedSize - xSpace;
                frameData[row][i+1] = 0.0f;
                frameData[row][i+2] = zSpace * row;
            }
        }
        
        terrainWaveform = new TerrainWaveform(clippedSize, numRows, "src/shaders/terrain_spectrum.vs", "src/shaders/terrain_spectrum.fs");   
    }

    void draw(int currentMillisecond){

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
        
        const int numVertices = 3 * clippedSize;
        int j = 0;
        for(int i = 0; i < numVertices; i+=3){
            frameData[0][i+1] = spectrum[j++] / (frameSize/4);
            if(frameData[0][i+1] > 0.2f) frameData[0][i+1] = 0.2f;
        }

        terrainWaveform->draw(frameData);

        delete[] audioFrame;
        delete[] spectrum;
    }

    ~Scene3(){ 
        delete audioFFT;
        for(int row = 0; row < numRows; row++){
            delete[] frameData[row]; 
        }
        delete[] frameData;
        delete terrainWaveform;
    }
};
