#version 330 core

in vec4 gl_FragCoord;
uniform int maxIterations = 500;
uniform vec2 constant;
out vec4 color;

void main(){
    int iteration = 0;
    float absVal, real, img;
    vec2 zn = gl_FragCoord.xy / 900;
    
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
    vec3 c = vec3(0.0f);

    // TODO: this is Patch ups, rewrite
    if(i < 5){
        c.b += (i / 5.0f) * 0.3;
    }
    if(i < 10){
        c.b += (i / 10.0f) * 0.1;
    }
    if(i < 100){
        c.r += i / 100.0f; 
    }
    if (i < 200){
        c.g += i / 200.0f;
    } 
    if (i < 350){
        c.b += i / 350;
    }
    
    color = vec4(c, 1.0f);
}