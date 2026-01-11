#version 330 core

in vec2 fTexCoord;
uniform sampler2D sTexture;
uniform bool horizontal = true;
uniform float weight[5] = float[] (0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);
out vec4 color;

void main(){
    vec2 tex_offset = 1.0f / textureSize(sTexture, 0);
    vec3 result = texture(sTexture, fTexCoord).rgb * weight[0];
    if(horizontal){
        for(int i = 1; i < 5; i++){
            result += texture(sTexture, fTexCoord + vec2(tex_offset.x * i, 0.0f)).rgb * weight[i];
            result += texture(sTexture, fTexCoord - vec2(tex_offset.x * i, 0.0f)).rgb * weight[i];
        }
    } else{
        for(int i = 1; i < 5; i++){
            result += texture(sTexture, fTexCoord + vec2( 0.0f, tex_offset.y * i)).rgb * weight[i];
            result += texture(sTexture, fTexCoord - vec2( 0.0f, tex_offset.y * i)).rgb * weight[i];
        }
    }

    color = vec4(result, 1.0f);
}