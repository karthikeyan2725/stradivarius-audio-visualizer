#version 330 core

in vec3 p;
uniform int offset;
out vec4 color;

void main(){
    if(offset == 0) color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    else color = vec4(1.0f, 0.1f, 0.2f, 1.0f);
};
