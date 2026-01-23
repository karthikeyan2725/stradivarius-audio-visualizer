#version 330 core

in vec2 fTexCoord;
uniform sampler2D tex0;
out vec4 color;

void main(){
    color = texture(tex0, fTexCoord);
}
