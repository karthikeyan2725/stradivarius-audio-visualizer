#version 330 core 

layout (location = 0) in vec2 point;
layout (location = 1) in vec2 inTexCoord;
out vec2 texCoord;

void main(){
    texCoord = inTexCoord;
    gl_Position = vec4(point.x, point.y, 0.0f, 1.0f);
}