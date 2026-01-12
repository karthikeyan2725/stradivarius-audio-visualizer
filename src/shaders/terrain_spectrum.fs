#version 330 core

in vec3 p;
uniform int offset;
out vec4 color;

void main(){
    if(offset == 0) color = vec4(1.0f, 0.9f, 0.0f, 1.0f);
    else color = vec4(0.3f, 0.3f, 0.3f, 1.0f - (offset/30.0f));
};
