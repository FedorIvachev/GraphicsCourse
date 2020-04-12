#version 410 core

out vec4 color;
uniform vec4 my_color; // A UNIFORM


void main()
{
    color = my_color;
}