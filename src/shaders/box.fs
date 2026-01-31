#version 330 core

in vec2 fTexCoord;
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;
uniform sampler2D tex5;
uniform sampler2D tex6;
uniform sampler2D tex7;
uniform sampler2D tex8;
uniform bool hasTexture = false;
out vec4 color;

void main(){
    color = vec4(1.0f);
}
