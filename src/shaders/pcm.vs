#version 330 core 

layout (location = 0) in vec3 point;

void main(){
    gl_Position = vec4(point.x, point.y, point.z, 1.0);
}
