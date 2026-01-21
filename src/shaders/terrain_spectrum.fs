#version 330 core

in vec3 p;
uniform int offset;
out vec4 color;

void main(){
    if(offset == 0) color = vec4(1.0f, 0.9f, 0.0f, 1.0f);
    else {
        color = vec4(0.1f, 0.1f, 0.1f, 0.1f) + vec4(0.5f, 0.5f, 0.5f, min(0.7f, p.y * 10));
        color = vec4(color.rgb, (color.a - (offset/30.0f)));
    }
};
