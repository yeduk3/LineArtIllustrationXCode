#version 410 core

in vec3 normal;
in vec3 worldPosition;

uniform vec3 lightPosition;
uniform vec3 eyePosition;
uniform vec3 lightColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;

out vec4 out_Color;

void main() {

    vec3 l = lightPosition - worldPosition;
    vec3 L = normalize(l);
    vec3 N = normalize(normal);
    vec3 R = 2 * dot(L, N) * N - L;
    vec3 V = normalize(eyePosition - worldPosition);
    vec3 I = lightColor / dot(l, l);

    vec3 H = (L+V)/2;

    vec3 ambient = diffuseColor * vec3(0.02);
    vec3 diffuse = I * diffuseColor * max(dot(L, N), 0);
    vec3 specular = I * specularColor * pow(max(dot(R, V), 0), shininess);

    vec3 color = ambient + diffuse + specular;

    color = pow(color, vec3(1/2.2f));
    
    out_Color = vec4(color, 1);
}