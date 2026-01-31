#version 330 core 

layout (location = 0) in vec3 point;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 aNormal;
uniform mat4 modelInstanced[10];
uniform mat4 projection;
out vec3 fragPos;
out vec2 fTexCoord;
out vec3 normal;

void main(){
    gl_Position = projection * modelInstanced[gl_InstanceID] * vec4(point, 1.0f);
    fTexCoord = texCoord;
    normal = mat3(transpose(inverse(modelInstanced[gl_InstanceID]))) * aNormal;
    fragPos =  (modelInstanced[gl_InstanceID] * vec4(point, 1.0f)).rgb;
}
