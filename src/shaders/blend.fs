#version 330 core

in vec2 fTexCoord;
uniform sampler2D sTexture1;
uniform sampler2D sTexture2;
out vec4 color;

void main(){
    const float gamma = 2.2;
    vec3 normal = texture(sTexture1, fTexCoord).rgb;
    vec3 blur = texture(sTexture2, fTexCoord).rgb;
    vec3 c = (normal + blur).rgb;
    color = vec4(pow(c, vec3(1.0 / gamma)), 1.0f);
    // color = vec4(c, 1.0f);
}