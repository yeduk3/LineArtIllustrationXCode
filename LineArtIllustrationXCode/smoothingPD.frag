#version 410 core

subroutine vec4 renderPassType();
subroutine uniform renderPassType renderPass;

// variables

uniform sampler2D positionTex;
uniform sampler2D pdTex;
uniform vec3 inverseSize;
uniform mat4 viewMat;

in vec2 texCoords;

out vec4 fragColor;

// kernel size in pixels. recommandation is 5~10.
const int KERNEL_SIZE = 5;
const vec2 direction[4] = vec2[4](vec2(0, 1), vec2(1, 0), vec2(0, -1), vec2(-1, 0));

// pass #1 - Write Depth to Texels

subroutine(renderPassType) vec4 pass1() {
    vec4 pd = texture(pdTex, texel.xy);
}

// pass #2 - Calc Smoothings

subroutine(renderPassType) vec4 pass2() {
//    vec2 texel = vec2(gl_FragCoord.xy) * inverseSize.xy;
    vec4 pd = texture(pdTex, texCoords);

    float depth = (viewMat * texture(positionTex, texCoords)).z;
    vec3 pixelX = vec3(gl_FragCoord.xy, depth);

    vec4 sum = vec4(0);

    for (int j = 0; j < 4; j++) {
        for (int i = 1; i <= KERNEL_SIZE; i++) {
            vec2 neiTexel = texCoords + (direction[j] * i) * inverseSize;
            float depthY = (viewMat * texture(positionTex, neiTexel)).z;
            vec3 pixelY = vec3(gl_FragCoord.xy + (direction[j] * i), depthY)
            vec4 npd = texture(pdTex, neiTexel);

            float signFunc = dot(pd, npd) > 0 ? 1 : -1;
            float dist = distance(pixelX, pixelY);
            float sWeight = dist < KERNEL_SIZE  ? 1 - dist / KERNEL_SIZE : 0;  // after edge detection
            float dWeight = abs(dot(pd, npd));

            sum += signFunc * sWeight * dWeight * npd;
        }
    }
}

void main() { fragColor = renderPass(); }
