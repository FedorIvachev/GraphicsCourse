#version 410 core

out vec4 color;
uniform vec3 my_color; // A UNIFORM


void main()
{
    float r, g, b;
    r = (gl_PrimitiveID % 7) / 7.0f;
    g = (gl_PrimitiveID % 9) / 9.0f;
    b = (gl_PrimitiveID % 11) / 11.0f;
    color = vec4(r, g, b, 1.0f);
}