#version 330 core

in vec2 fTexCoord;
uniform sampler2D sTexture;
out vec4 color;

void main(){
    float gamma = 2.2f;
    vec3 c = texture(sTexture, fTexCoord).rgb;
    color = vec4(pow(c, vec3(gamma)), 1.0f);
}