#version 450
in vec2 coord;
out vec4 FragColor;

uniform float zoom;
uniform vec2 pan;
uniform float aspectRatio; 

void main()
{
    vec2 z = (vec2(coord.x, coord.y * (1.0 / aspectRatio))) / zoom + pan;
    vec2 c = z;

    // random special marks
    if (int(z.y * 100) == 0){
        if (int (z.x * 100) == 0){
            FragColor = vec4(1.0, 0.3, 0.3, 1.0);
            return;
        }
        if (int(z.x * 100) == -100){
            FragColor = vec4(0.1, 1.0, 0.3, 1.0);
            return;
        }
        
    }

    int iterations = 1000;
    int i;

    for (i = 0; i < iterations; i++)
    {
        float xtemp = z.x * z.x - z.y * z.y + c.x;
        z.y = 2.0 * z.x * z.y + c.y;
        z.x = xtemp;

        if (z.x * z.x + z.y * z.y > 4.0)
            break;
    }

    float color = float(i) / float(iterations);
    FragColor = vec4(color, color, color, 1.0);
}
