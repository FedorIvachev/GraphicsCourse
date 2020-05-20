#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec2 texCoord2;


out vec2 TexCoords;
out vec2 TexCoords2;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float scale;

void main()
{
    gl_Position = projection * view * model * vec4(position * scale, 1.0f);
    TexCoords = vec2(texCoord.x, 1.0f - texCoord.y);
    TexCoords2 = vec2(texCoord2.x, 1.0f - texCoord2.y);

}