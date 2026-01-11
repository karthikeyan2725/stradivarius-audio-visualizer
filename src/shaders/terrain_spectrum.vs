#version 330 core 

layout (location = 0) in vec3 point;
uniform mat4 view;
uniform int offset = 0;
out vec3 p;

void main(){
    gl_Position = view * vec4(point.x, point.y, offset * 0.005f, 1.0);
    p = point;
}