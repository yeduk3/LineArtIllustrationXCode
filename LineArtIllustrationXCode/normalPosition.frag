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


// pass #3 - Edge detection



float luma(vec3 color) {
  return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

subroutine(renderPassType)
vec4 pass3() {
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

    // outPosition.w is 1 if it is edge, otherwise 0.
    return vec4(int(dist > edgeThreshold));
//    outEdge = texture(positionTex, texCoords);
}

void main()
{
    fragColor = renderPass();
}
