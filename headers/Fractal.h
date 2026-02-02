#pragma once
#include <glm/glm.hpp>
#include <iostream>

class Fractal{
public:
    static glm::vec2 computeNext(glm::vec2 current, glm::vec2 constant){
        float real = current.x * current.x - current.y * current.y;
        float img = 2 * current.x * current.y;

        return glm::vec2(real, img) + constant;
    }    

    static float abs(glm::vec2 c){
        return c.x * c.x + c.y * c.y;
    }

    static int getIterationCount(glm::vec2 z0, glm::vec2 constant){
        int iteration = 0;
        int maxIteration = 50;
        glm::vec2 zn = z0;
        while(iteration < maxIteration && Fractal::abs(zn) <= 4.0f){
            zn = computeNext(zn, constant);
            // std::cout << zn.x << "+" << zn.y << "i" << ", abs = " << Fractal::abs(zn) << std::endl;
            iteration++;
        }
        return iteration;
    }
};
