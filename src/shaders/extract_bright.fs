#version 330 core

in vec2 texCoord;
uniform sampler2D sTexture;
out vec4 color;

void main(){
    vec4 c = texture(sTexture, texCoord);
    float brightness = (c.r + c.g + c.b)/3.0f;
    if(brightness > 20.0f) color = c;
    else color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}