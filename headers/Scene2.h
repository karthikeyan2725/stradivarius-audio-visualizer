#pragma once
#include <vector>
#include <CircularWaveform.h>
#include <AudioFFT.h>

class Scene2 {
    std::vector<std::vector<double>> audioSignal;
    int sampleRate;
    int frameSize;
    int clippedSize;
    float *frameData; 
    CircularWaveform *circularWaveform;
    AudioFFT *audioFFT;
    public:
        Scene2(std::vector<std::vector<double>> audioSignal, int sampleRate, int width, int height){
            this->audioSignal = audioSignal;
            this->sampleRate = sampleRate;

            // configurations
            frameSize = 1024 / 4;

            audioFFT = new AudioFFT(frameSize);
            clippedSize = audioFFT->getClippedSize(20, 20000, sampleRate); 

            int numVertices = clippedSize * 3;
            frameData = new float[numVertices]; 
            for(int i = 0; i < numVertices; i+=3){
                frameData[i] = ((double)i/3); 
                frameData[i+1] = 0.0f;
                frameData[i+2] = 0.0f;
            }
            
            circularWaveform = new CircularWaveform(clippedSize, "src/shaders/circular_spectrum.vs", "src/shaders/circular_spectrum.fs");
        }

        void draw(int currentMillisecond){

            float *audioFrame = new float[frameSize];
            for(int i = 0; i < frameSize; i++){
                audioFrame[i] = (float)audioSignal[0][(currentMillisecond/1000.0f) * sampleRate + i];
            }
            float *spectrum = new float[clippedSize];
            audioFFT->fftMagnitudesClipped(audioFrame, spectrum, 20, 20000, sampleRate);

            const int numVertices = 3 * clippedSize;
            int j = 0;
            for(int i = 0; i < numVertices; i+=3){
                frameData[i+1] = spectrum[j++] / (frameSize/8);
                if(frameData[i+1] > 0.2f) frameData[i+1] = 0.2f;
            }

            circularWaveform->draw(frameData);

            delete[] audioFrame;
            delete[] spectrum;
        }

        ~Scene2(){
            delete[] frameData;
            delete circularWaveform;
            delete audioFFT;
        }
};
