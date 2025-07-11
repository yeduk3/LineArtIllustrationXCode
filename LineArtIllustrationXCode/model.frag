#version 410 core

in vec3 worldNormal;
in vec3 worldPosition;
in vec3 viewPosition;
in vec3 viewNormal;
in vec2 texCoord;

//out vec4 fragColor;
layout(location = 0) out vec4 positionColor;
layout(location = 1) out vec4 normalColor;
layout(location = 2) out vec4 phongColor;
layout(location = 3) out vec4 texCoordColor;

// lighting
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;
uniform mat4 viewMat;


void positionNormalPhongTex() {
    positionColor = vec4(worldPosition, 1);
    normalColor = vec4(normalize(worldNormal), 1);
    
    
    vec4 lp = viewMat * vec4(lightPosition, 1);
    vec3 l = lp.xyz/lp.w - viewPosition;
    vec3 L = normalize(l);
    vec3 N = normalize(viewNormal);
    vec3 R = 2 * dot(L, N) * N - L;
    vec3 I = lightColor / dot(l, l);

    vec3 ambient = diffuseColor * vec3(0.02);
    vec3 diffuse = I * diffuseColor * max(dot(L, N), 0);
    vec3 specular = I * specularColor * pow(max(R.z, 0), shininess);

    vec3 color = ambient + diffuse + specular;

    // Square the intensity to emphasize the ratio of brightness differences
    // Lee, 2006
    phongColor = vec4(pow(color, vec3(2 / 2.2)), 1);
    
    texCoordColor = vec4(texCoord, 0, 1);
}

void main()
{
    positionNormalPhongTex();
}
