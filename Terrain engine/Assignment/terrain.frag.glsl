#version 330 core

in vec2 TexCoords;
in vec2 TexCoords2;

out vec4 color;

uniform sampler2D texture0;
uniform sampler2D texture1;


void main()
{
    color = clamp(texture(texture1, TexCoords2) + texture(texture0, TexCoords) - vec4(0.5, 0.5, 0.5, 1.0), 0.0, 1.0);
}