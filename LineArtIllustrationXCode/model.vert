#version 410 core

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;

out vec3 worldNormal;
out vec3 worldPosition;
out vec3 viewPosition;
out vec3 viewNormal;

void main()
{
    vec4 worldPos = modelMat * vec4(in_Position, 1);
    vec4 viewPos = viewMat * worldPos;
    gl_Position = projMat * viewPos;

    worldPosition = worldPos.xyz / worldPos.w;
    worldNormal = in_Normal;
    viewPosition = viewPos.xyz / viewPos.w;
    viewNormal = (viewMat * vec4(worldNormal, 0)).xyz;
}
