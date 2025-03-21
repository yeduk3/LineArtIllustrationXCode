#version 410 core

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;

out vec3 normal;
out vec3 worldPosition;

uniform bool isView;

void main()
{
    vec4 worldPos = modelMat * vec4(in_Position, 1);
    gl_Position = projMat * viewMat * worldPos;

//    if(isView) {
//        worldPosition = (viewMat * vec4(worldPos.xyz, 1)).xyz;
//        normal = (viewMat * vec4(in_Normal, 0)).xyz;
//        
//    } else {
//        worldPosition = worldPos.xyz;
//        normal = in_Normal;
//        
//    }
    
    worldPosition = worldPos.xyz;
    normal = in_Normal;
}
