#version 330 core

in vec3 p;
uniform int offset;
out vec4 color;

void main(){
    color = vec4(1.0f, 0.1f, 0.2f, 1.0f);
};
