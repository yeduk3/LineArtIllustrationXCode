#version 410 core

layout (location = 0) in vec2 in_Position;
layout (location = 1) in vec2 in_TexCoord;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(in_Position, 0.0, 1.0);
    texCoord = in_TexCoord;
}
