#version 330 core 

layout (location = 0) in vec3 point;
layout (location = 1) in vec2 texCoord;
out vec2 fTexCoord;

void main(){
    gl_Position = vec4(point.x, point.y, point.z, 1.0f);
    fTexCoord = texCoord;
}
