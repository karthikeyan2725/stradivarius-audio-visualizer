#version 330 core 

layout (location = 0) in vec3 point;
layout (location = 1) in vec2 texCoord;
uniform mat4 model;
uniform mat4 projection;
out vec2 fTexCoord;

void main(){
    gl_Position = projection * model * vec4(point, 1.0f);
    fTexCoord = texCoord;
}
