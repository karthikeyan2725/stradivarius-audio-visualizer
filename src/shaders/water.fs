#version 330 core

in vec2 fTexCoord;
uniform sampler2D dudvTexture;
uniform sampler2D reflection;
uniform float offset;
out vec4 color;

void main(){
    // float gamma = 2.2f;
    // vec3 c = texture(reflection, fTexCoord).rgb;
    // color = vec4(pow(c, vec3(gamma)), 1.0f);
    vec2 distortion1 = texture(dudvTexture, fTexCoord * 3 + offset).rg * 0.05f;
    // vec2 distortion2 = texture(dudvTexture, fTexCoord * 3 + vec2(offset, -offset * 0.5f)).rg * 0.05f;
    vec2 distortion = distortion1;
    vec4 reflectionColor = texture(reflection, fTexCoord + distortion);
    vec4 refractionColor = vec4(vec3(0.0f), 1.0f);
    color = mix(reflectionColor, refractionColor, 0.5f);
}