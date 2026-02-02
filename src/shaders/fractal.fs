#version 330 core

in vec2 fTexCoord;
uniform sampler2D sTexture;
out vec4 color;

void main(){
    color = texture(sTexture, fTexCoord);
}