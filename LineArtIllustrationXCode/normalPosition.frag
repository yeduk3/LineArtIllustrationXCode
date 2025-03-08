#version 410 core

in vec3 normal;
in vec3 worldPosition;

layout (location = 0) out vec4 outNormal;
layout (location = 1) out vec4 outPosition;

void main()
{
    // vec3 ranged = (normalize(normal) + vec3(1))/vec3(2);
    vec3 ranged = normalize(normal);
    outNormal = vec4(ranged, 1);
    outPosition = vec4(worldPosition, 1);
}