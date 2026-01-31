#version 330 core

in vec3 fragPos;
in vec2 fTexCoord;
in vec3 normal;
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
uniform vec3 lightColor;
uniform vec3 lightPos;

float constant = 1.0f;
float linear = 0.09f;
float quadratic = 0.032f;

out vec4 color;

void main(){
    
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diffuse = max(dot(norm, lightDir), 0.0f);
    vec3 diffuseColor = diffuse * lightColor;
    diffuseColor *= attenuation;

    float ambientStrength = 0.4f;
    vec3 ambientColor = ambientStrength * lightColor;
    ambientColor *= attenuation;

    vec3 result = (ambientColor + diffuseColor) * texture(tex0, fTexCoord).rgb;
    if(hasTexture) color = vec4(result, 1.0f);
    else discard;
}
