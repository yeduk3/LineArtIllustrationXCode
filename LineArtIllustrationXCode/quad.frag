#version 410 core

uniform sampler2D tex;

in vec2 texCoords;

out vec4 fragColor;

void main()
{
    fragColor = texture(tex, texCoords);
}