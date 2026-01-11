#version 330 core 

layout (location = 0) in vec3 point;
out vec3 p;

void main(){
    gl_Position = vec4((point.y + 0.5f) * cos(point.x * 6.2f), (point.y + 0.5f)* sin(point.x * 6.2f), point.z, 1.0);
};