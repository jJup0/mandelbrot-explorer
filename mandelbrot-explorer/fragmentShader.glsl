#version 450
in vec2 coord;
out vec4 FragColor;

uniform float zoom;
uniform vec2 pan;
uniform float aspectRatio; 
uniform int max_iterations;

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

    for (i = 0; i < max_iterations; i++)
    {
        float xtemp = z.x * z.x - z.y * z.y + c.x;
        z.y = 2.0 * z.x * z.y + c.y;
        z.x = xtemp;

        if (z.x * z.x + z.y * z.y > 4.0)
            break;
    }


    if (i == max_iterations){
        float white_shade = 0.9;
        FragColor = vec4(white_shade, white_shade, white_shade, white_shade);
    } else {
        float col = float(i) / float(max_iterations);
        int cuttoff = min(max_iterations/2, 50);
        if (i < cuttoff){
            FragColor = vec4(hsv2rgb(vec3(col, 1.0, float(i) / cuttoff)), 1.0);
        } else {
            FragColor = vec4(hsv2rgb(vec3(col, 1.0, 1.0)), 1.0);
        }
    }
}
