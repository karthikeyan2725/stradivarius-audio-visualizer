#version 330 core

in vec4 gl_FragCoord;
uniform int maxIterations = 50;
uniform vec2 constant;
out vec4 color;

void main(){
    int iteration = 0;
    float absVal, real, img;
    vec2 zn = gl_FragCoord.xy / 500;
    
    float i = 0;
    while(i < maxIterations){
        absVal = zn.x * zn.x + zn.y * zn.y;
        if(absVal > 4.0f) break;

        real = zn.x * zn.x - zn.y * zn.y;
        img = 2 * zn.x * zn.y;

        zn = vec2(real, img) + constant;

        i++;
    }   

    float brightness = (i / maxIterations);
    color = vec4(vec3(brightness), 1.0f);
}