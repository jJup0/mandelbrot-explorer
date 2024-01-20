#version 450
in vec2 coord;
out vec4 FragColor;

uniform float zoom;
uniform vec2 pan;
uniform float aspectRatio; 

float MAX_ITER = 100;


// All components are in the range [0…1]
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main()
{
    vec2 z = (vec2(coord.x, coord.y * (1.0 / aspectRatio))) / zoom + pan;
    vec2 c = z;

    int i;

    for (i = 0; i < MAX_ITER; i++)
    {
        float xtemp = z.x * z.x - z.y * z.y + c.x;
        z.y = 2.0 * z.x * z.y + c.y;
        z.x = xtemp;

        if (z.x * z.x + z.y * z.y > 4.0)
            break;
    }


    if (i == MAX_ITER){
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        // FragColor = getColor(MAX_ITER);
        float col = float(i) / float(MAX_ITER);
        
        // FragColor = vec4(col, col, col, 1.0);
        if (i < 10){
            FragColor = vec4(hsv2rgb(vec3(col, 1.0, float(i) / 10.0)), 1.0);
        } else {
            FragColor = vec4(hsv2rgb(vec3(col, 1.0, 1.0)), 1.0);
        }
    }
}
