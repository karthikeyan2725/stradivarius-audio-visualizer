#pragma once
#include <fftw3/fftw3.h>
#include <math.h>

class AudioFFT{
    unsigned int size;
    fftw_complex *input;
    fftw_complex *output;
    fftw_plan plan;
    const int REAL = 0;
    const int COMPLEX = 1;
public:
    AudioFFT(unsigned int size){
        this->size = size;
        input = new fftw_complex[size];
        output = new fftw_complex[size];
        plan = fftw_plan_dft_1d(size, input, output, FFTW_FORWARD,  FFTW_MEASURE);   
    }

    ~AudioFFT(){
        delete[] input;
        delete[] output;
    }

    void fftMagnitudesClipped(const float *signal, float *target, int startFrequency, int endFrequency, int sampleRate){
        for(int i = 0; i < size; i++){
            input[i][REAL] = signal[i];
            input[i][COMPLEX] = 0;
        }

        applyHannWindow(input);

        fftw_execute(plan);
        
        int output_index = getIndex(startFrequency, sampleRate);
        int clippedSize = getClippedSize(startFrequency, endFrequency, sampleRate);
        for(int i = 0; i < clippedSize; i++){
            target[i] = magnitude(output[output_index+i]);
        }
    }

    float magnitude(const fftw_complex u){
        return sqrt(u[REAL] * u[REAL] + u[COMPLEX] * u[COMPLEX]);
    }

    void applyHannWindow(fftw_complex *input){
        double a0 = 0.5f; 
        double a1 = 1 - a0;
        double w;
        for(int i = 0; i < size; i++){
            w = a0 - a1 * cos((2*M_PI*i)/size);
            input[i][REAL] *= w;
        }
    }

    int getClippedSize(int startFrequency, int endFrequency, int sampleRate){
        return getIndex(endFrequency, sampleRate) - getIndex(startFrequency, sampleRate);
    }

    int getIndex(int frequency, int sampleRate){ // TODO : Check for logic issues
        return ceil((frequency * size) / sampleRate);
    }
};