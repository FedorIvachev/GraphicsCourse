#version 330 core

in vec2 TexCoords;
in vec2 TexCoords2;
in float cl;

out vec4 color;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
    vec4 texcolor = texture(texture0, TexCoords);
    if (cl < 0.5)
        discard;
    else 
        color = clamp(texture(texture1, TexCoords2) + texcolor - vec4(0.5, 0.5, 0.5, 1.0), 0.0, 1.0);
}
