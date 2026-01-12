#version 330 core 

#define M_PI 3.1415f

layout (location = 0) in vec3 point;
uniform int total_points = 128;
uniform float min_width = 0.5f;
out vec3 p;

void main(){
    float width = min_width + point.y;
    float theta = 2 * M_PI * (point.x/total_points);
    gl_Position = vec4(width * cos(theta), width * sin(theta), point.z, 1.0);
};