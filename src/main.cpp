#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <audiofile/AudioFile.h>
#include <miniaudio.h>
#include <stb_image.h>

#include <Scene1.h>
#include <Scene2.h>
#include <Scene3.h>
#include <Scene4.h>

#include <Texture.h>
#include <FrameBuffer.h>
#include <Mesh.h>
#include <Model.h>

#define APPNAME "Stradivarius"

bool handleSizeChange = false;
void resize_frame_buffer_on_window(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
    handleSizeChange = true;
}

int main(int argc, char** argv){
    const char* songPath;
    const char* modelPath;
    if(argc < 3) {
        std::cout << "Please specify all necessary paths (song, model)" << std::endl;
        return -1;
    }

    songPath = argv[1];
    std::cout << "Playing : " << songPath << std::endl;

    modelPath = argv[2];

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
    // ma_sound_start(&sound);
    
    AudioFile<double> audioFile;
    audioFile.load(songPath);
    audioFile.printSummary();
    int sampleRate = audioFile.getSampleRate();

    Scene1 scene1(audioFile.samples, sampleRate);
    Scene2 scene2(audioFile.samples, sampleRate);
    Scene3 scene3(audioFile.samples, sampleRate);
    Scene4 scene4;

    // Single Vao for all
    unsigned int vao;
    glGenVertexArrays(1, &vao); // TODO: Move to each waveforms
    glBindVertexArray(vao);
    
    std::shared_ptr<Texture> bgTexture = Texture::load("images/night.jpg", GL_LINEAR, GL_CLAMP_TO_EDGE);
    bgTexture->bind(3);
    
    std::shared_ptr<Texture> dudvTexture = Texture::load("images/dudv_map.png", GL_LINEAR, GL_REPEAT);
    dudvTexture->bind(4);
    
    float quadVertex[24] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    
    unsigned int quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertex), quadVertex, GL_STATIC_DRAW);
    
    Shader quadShader("src/shaders/bg.vs", "src/shaders/bg.fs");
    
    FrameBuffer waterFrameBuffer(frameBufferWidth, frameBufferHeight, GL_LINEAR, GL_CLAMP_TO_EDGE);
    
    Shader waterShader("src/shaders/water.vs", "src/shaders/water.fs");
    glm::mat4 view = glm::mat4(1.0f); 
    view = glm::scale(view, glm::vec3(1.0f));
    view = glm::translate(view, glm::vec3(0.0f, -0.5f, 0.0f));
    view = glm::rotate(view, 180.0f * (3.14f / 180), glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::rotate(view, 180.0f * (3.14f / 180), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, 50.0f * (3.14f / 180), glm::vec3(1.0f, 0.0f, 0.0f));
    waterShader.use();
    waterShader.setUniformMatrix4fv("view", view);
    
    Bloom bloom(frameBufferWidth, frameBufferHeight);

    float lastSecond = 0;
    float waterOffset = 0.0f;
    int choice = 1;
    bool isBloom = true;

    glEnable(GL_DEPTH_CLAMP); // No clipping of objects
    glEnable(GL_DEPTH_TEST);

    // // Testing Mesh class
    // std::vector<Vertex> vertices = {
    //     Vertex{glm::vec3(0.5f,  0.5f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f)},   
    //     Vertex{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f)},   
    //     Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f)},  
    //     Vertex{glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f)}    
    // };

    // std::vector<unsigned int> indices;
    // indices.push_back(0);
    // indices.push_back(1);
    // indices.push_back(3);
    // indices.push_back(1);
    // indices.push_back(2);
    // indices.push_back(3);

    // std::vector<std::shared_ptr<Texture>> textures;
    // std::shared_ptr<Texture> pianoTexture = Texture::load("images/piano.jpg", GL_LINEAR, GL_CLAMP_TO_EDGE);
    // textures.push_back(pianoTexture);
    
    // Shader tunnelShader("src/shaders/tunnel.vs", "src/shaders/tunnel.fs");
    // Mesh mesh(vertices, indices, textures);
    
    // Shader boxShader("src/shaders/box.vs", "src/shaders/box.fs");

    // LightBox
    // std::vector<Vertex> boxVertices;
    // std::vector<unsigned int> boxIndices;

    // // Bottom
    // boxVertices.push_back({glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // A 0
    // boxVertices.push_back({glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // B 1
    // boxVertices.push_back({glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // C 2
    // boxVertices.push_back({glm::vec3(-0.5, -0.5f, 0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // D 3
    // // ABC
    // boxIndices.push_back(0);
    // boxIndices.push_back(1);
    // boxIndices.push_back(2);
    // // CDA
    // boxIndices.push_back(2);
    // boxIndices.push_back(3);
    // boxIndices.push_back(0);

    // // Top
    // boxVertices.push_back({glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // G 4
    // boxVertices.push_back({glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // F 5
    // boxVertices.push_back({glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // E 6
    // boxVertices.push_back({glm::vec3(-0.5, 0.5f, 0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // H 7
    // // EFG
    // boxIndices.push_back(6);
    // boxIndices.push_back(5);
    // boxIndices.push_back(4);
    // // GHE
    // boxIndices.push_back(4);
    // boxIndices.push_back(7);
    // boxIndices.push_back(6);

    // // LEFT
    // // GFA
    // boxIndices.push_back(4);
    // boxIndices.push_back(5);
    // boxIndices.push_back(0);
    // // FBA
    // boxIndices.push_back(5);
    // boxIndices.push_back(1);
    // boxIndices.push_back(0);

    // // RIGHT
    // // EHD
    // boxIndices.push_back(6);
    // boxIndices.push_back(7);
    // boxIndices.push_back(3);
    // // DCE
    // boxIndices.push_back(3);
    // boxIndices.push_back(2);
    // boxIndices.push_back(6);

    // // FRONT
    // // ADH
    // boxIndices.push_back(0);
    // boxIndices.push_back(3);
    // boxIndices.push_back(7);
    // // HGA
    // boxIndices.push_back(7);
    // boxIndices.push_back(4);
    // boxIndices.push_back(0);

    // // BACK
    // // FEB
    // boxIndices.push_back(5);
    // boxIndices.push_back(6);
    // boxIndices.push_back(1);
    // // BEC
    // boxIndices.push_back(1);
    // boxIndices.push_back(6);
    // boxIndices.push_back(2);

    // Right 
    // boxVertices.push_back({glm::vec3()});

    // std::vector<std::shared_ptr<Texture>> boxTextures;

    // Mesh boxMesh(boxVertices, boxIndices, textures);

    // Model tunnel(modelPath); 
    // Model tunnel("models/backpack/backpack.obj");
    
    ma_sound_start(&sound);
    
    float rotateX = 90.0f;
    float rotateY = 0.0f;
    float tX = 0.0f;
    float tY = 0.0f;
    float tZ = -0.3f;
    float scaleBy =  0.003f;// 0.0030912; // prev 0.0001
    float scaleCube = 0.5f;
    float fov = 90.0f;
    int numTunnel = 20;
    float offsetAmount = 0.3f;
    float bZ = 2.0f;
    while(!glfwWindowShouldClose(window)){
        if(handleSizeChange){
            glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
            waterFrameBuffer.updateSize(frameBufferWidth, frameBufferHeight);
            bloom.updateSize(frameBufferWidth, frameBufferHeight);
            handleSizeChange = false;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        if(isBloom) bloom.start();
        else waterFrameBuffer.bind();

        // Background
        if(choice != 4){
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            quadShader.use();
            quadShader.setUniform1i("sTexture", 3);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glBindVertexArray(vao); // TODO: Add VAO to all
        ma_int64 currentMillisecond = ma_engine_get_time_in_milliseconds(&engine);
        float currentSecond = currentMillisecond / 1000.0f;
        switch(choice){
            case 1: scene1.draw(currentMillisecond); break;
            case 2: scene2.draw(currentMillisecond); break;
            case 3: 
                if(currentSecond >= lastSecond + 0.05f) {
                    scene3.draw(currentMillisecond, true);
                    lastSecond = currentSecond;
                } else {
                    scene3.draw(currentMillisecond, false);
                }
                break;
            case 4: {
                float boxRotation = ma_engine_get_time_in_milliseconds(&engine) / 10.0f;
                float tunnelRotation = ma_engine_get_time_in_milliseconds(&engine) / 100.0f;
                tZ -= 0.0001f;
                scene4.draw(bZ, tZ, (float)frameBufferWidth / frameBufferHeight, boxRotation, -tunnelRotation);
                glBindVertexArray(vao); // TODO: Add VAO to all
            }
            default: break;
        }

        if(isBloom) bloom.end(&waterFrameBuffer);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render Bloomed texture
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, waterFrameBuffer.getColorAttachment());
        quadShader.use();
        quadShader.setUniform1i("sTexture", 5);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // render Water Ripple
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, waterFrameBuffer.getColorAttachment());
        waterShader.use();
        waterShader.setUniform1i("dudvTexture", 4);
        waterShader.setUniform1i("reflection", 5);
        waterShader.setUniform1f("offset", waterOffset);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // // Render waveform again
        // switch(choice){
        //     case 1: scene1.draw(currentMillisecond); break;
        //     case 2: scene2.draw(currentMillisecond); break;
        //     case 3: 
        //     if(currentSecond >= lastSecond + 0.05f) {
        //         scene3.draw(currentMillisecond, true);
        //         lastSecond = currentSecond;
        //     } else {
        //         scene3.draw(currentMillisecond, false);
        //     }
        //     break;
        //     case 4: break;
        //     default: break;
        // }

        // Testing mesh 
        // mesh.draw(&tunnelShader);
        // glBindVertexArray(vao); // TODO: Add VAO to all

        // Loading Model        
        // tunnelShader.use();

        // glm::mat4 proj = glm::perspective(glm::radians(fov), (float)frameBufferWidth/(float)frameBufferHeight, 0.1f, 10000.0f);
        // tunnelShader.setUniformMatrix4fv("projection", proj);

        // std::cout << "-" << std::endl;
        // for(int i = 0; i < numTunnel; i++){
        //     float currentOffset = offsetAmount * i;

        //     glm::mat4 modelInstanced = glm::mat4(1.0f); 
        //     modelInstanced = glm::translate(modelInstanced, glm::vec3(0, 0, -(currentOffset + tZ)));
        //     modelInstanced = glm::rotate(modelInstanced, glm::radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f));
        //     modelInstanced = glm::rotate(modelInstanced, glm::radians(rotateY), glm::vec3(0.0f, 1.0f, 0.0f));
        //     modelInstanced = glm::scale(modelInstanced, glm::vec3(scaleBy));
        //     std::string uniformString = "modelInstanced[" + std::to_string(i) + "]";
        //     tunnelShader.setUniformMatrix4fv(uniformString.c_str(), modelInstanced);
        // }
        
        // glm::mat4 scaleMat = glm::mat4(1.0f);
        // tunnelShader.setUniform3fv("lightColor", glm::vec3(0.5, 0.0, 0.0));
        // tunnelShader.setUniform3fv("lightPos", glm::vec3(0.0, 0.0, -(bZ+0.5f)));
        // tunnel.draw(&tunnelShader, numTunnel);
        // glBindVertexArray(vao); // TODO: Add VAO to all

        // ma_int64 currentMillisecond = ma_engine_get_time_in_milliseconds(&engine);
        // float currentSecond = currentMillisecond / 1000.0f;
        // float rotateCos = cos(currentSecond);
        // glm::mat4 boxModel = glm::mat4(1.0f); 
        // boxModel = glm::translate(boxModel, glm::vec3(0.0f, 0.0f, -bZ));
        // boxModel = glm::rotate(boxModel, glm::radians(20.0f), glm::vec3(1.0f, rotateCos, 0));
        // boxModel = glm::scale(boxModel, glm::vec3(scaleCube));
        // boxShader.use();
        // boxShader.setUniformMatrix4fv("projection", proj);
        // boxShader.setUniformMatrix4fv("model", boxModel);
        // boxMesh.draw(&boxShader);
        // glBindVertexArray(vao); // TODO: Add VAO to all


        waterOffset += 0.001f;
        waterOffset = fmodf(waterOffset, 1);

        if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) choice = 1;
        if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) choice = 2;
        if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) choice = 3;
        if(glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) choice = 4;
        if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) isBloom = true;
        if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) isBloom = false;

        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) rotateX += 1.0f;
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) rotateX -= 1.0f;
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rotateY += 1.0f;
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) rotateY -= 1.0f;
        if(glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
            scaleBy *= 1.1f;
            std::cout << "Scale Increase to : " << scaleBy << std::endl;
        }
        if(glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS){
            scaleBy *= 0.9f;
            std::cout << "Scale Decrease to : " << scaleBy << std::endl;
        }
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) tX += 1.0f;
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) tX -= 1.0f;
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) tY += 1.0f;
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) tY -= 1.0f;
        if(glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS){
            fov -= 1.0f;
            std::cout << "Fov Decrease to: " << fov << std::endl;
        }   
        if(glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS){ 
            fov += 1.0f;
            std::cout << "Fov Increase to:" << fov << std::endl;
        } 
        if(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS){ 
            numTunnel++;
            std::cout << "Num Of tunnel Increase to:" << numTunnel << std::endl;
        } 
        if(glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS){ 
            offsetAmount += 0.001f;
            std::cout << "Offset Increase to:" << offsetAmount << std::endl;
        } 
        if(glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS){ 
            offsetAmount -= 0.001f;
            std::cout << "Offset Decrease to:" << offsetAmount << std::endl;
        }
        if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){
            tZ += 0.0001f;
            std::cout << "tZ = " << tZ << std::endl;
        }
        if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){
            tZ -= 0.0001f;
            std::cout << "tZ = " << tZ << std::endl;
        }
        if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS){
            bZ += 0.1f;
            std::cout << "bZ = " << bZ << std::endl;
        }
        if(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS){
            bZ -= 0.1f;
            std::cout << "bZ = " << bZ << std::endl;
        }

        // if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) scene3.rotateBy(0.1f, 0.0f);
        // if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) scene3.rotateBy(-0.1f, 0.0f);
        // if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) scene3.rotateBy(0.0f, -0.05f);
        // if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) scene3.rotateBy(0.0f, 0.05f);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    ma_engine_uninit(&engine);
    ma_sound_uninit(&sound);
    glfwTerminate();

    return 0;
}
 