#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture1;
uniform vec4 my_color;

void main()
{
    color = my_color * texture(texture1, TexCoords);
}
