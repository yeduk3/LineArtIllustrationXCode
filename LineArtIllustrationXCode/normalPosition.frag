#version 410 core

subroutine vec4 renderPassType();
subroutine uniform renderPassType renderPass;

in vec3 normal;
in vec3 worldPosition;

//layout (location = 0) out vec4 outNormal;
//layout (location = 1) out vec4 outPosition;
//layout (location = 6) out vec4 outEdge;

out vec4 fragColor;

uniform vec2 inverseSize;
uniform sampler2D positionTex;
uniform float edgeThreshold;

// pass #1 - out Normal


subroutine(renderPassType)
vec4 pass1() {
//    vec3 ranged = normalize(normal);
//    outNormal = vec4(ranged, 1);
//    outPosition = vec4(worldPosition, 1);
    
    vec3 ranged = normalize(normal);
    return vec4(ranged, 1);
    
}

// pass #2 - out Position

subroutine(renderPassType)
vec4 pass2() {
    return vec4(worldPosition, 1);
}

// lighting

uniform vec3 lightPosition;
uniform vec3 eyePosition;
uniform vec3 lightColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;

// pass #3 - phong shading
subroutine(renderPassType)
vec4 pass3() {
    vec3 l = lightPosition - worldPosition.xyz;
    vec3 L = normalize(l);
    vec3 N = normalize(normal);
    vec3 R = 2 * dot(L, N) * N - L;
    vec3 V = normalize(eyePosition - worldPosition.xyz);
    vec3 I = lightColor / dot(l, l);

    vec3 ambient = diffuseColor * vec3(0.02);
    vec3 diffuse = I * diffuseColor * max(dot(L, N), 0);
    vec3 specular = I * specularColor * pow(max(dot(R, V), 0), shininess);

    vec3 color = ambient + diffuse + specular;

    return vec4(pow(color, vec3(1 / 2.2)), 1);
}


// pass #4 - Edge detection



float luma(vec3 color) {
  return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

subroutine(renderPassType)
vec4 pass4() {
    float dx = inverseSize.x;
    float dy = inverseSize.y;
    vec2  texCoords = gl_FragCoord.xy * inverseSize;
    float s00 = luma(texture(positionTex, texCoords + vec2(-dx, dy)).rgb);
    float s10 = luma(texture(positionTex, texCoords + vec2(-dx, 0.0)).rgb);
    float s20 = luma(texture(positionTex, texCoords + vec2(-dx, -dy)).rgb);
    float s01 = luma(texture(positionTex, texCoords + vec2(0.0, dy)).rgb);
    float s21 = luma(texture(positionTex, texCoords + vec2(0.0, -dy)).rgb);
    float s02 = luma(texture(positionTex, texCoords + vec2(dx, dy)).rgb);
    float s12 = luma(texture(positionTex, texCoords + vec2(dx, 0.0)).rgb);
    float s22 = luma(texture(positionTex, texCoords + vec2(dx, -dy)).rgb);
    
    // sobel filter
    float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
    float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);
    float dist = pow(sx, 2) + pow(sy, 2);

    return vec4(int(dist > edgeThreshold));
//    outEdge = texture(positionTex, texCoords);
}

void main()
{
    fragColor = renderPass();
}
