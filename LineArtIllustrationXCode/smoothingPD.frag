#version 410 core

subroutine vec4 renderPassType();
subroutine uniform renderPassType renderPass;

// variables

uniform sampler2D edgeTex;
uniform sampler2D pdTex;
uniform vec2 inverseSize;
uniform mat4 viewMat;

in vec2 texCoords;

out vec4 fragColor;

// kernel size in pixels. recommandation is 5~10.
const int KERNEL_SIZE = 10;
const vec2 direction[4] = vec2[4](vec2(0, 1), vec2(1, 0), vec2(0, -1), vec2(-1, 0));

// Quantization  level recommandation is 12~24.
const int Q_LEVEL = 12;


uniform sampler2D tam0;
uniform sampler2D tam1;
uniform sampler2D tam2;
uniform sampler2D tam3;
uniform sampler2D tam4;
uniform sampler2D tam5;
uniform sampler2D phong;

subroutine(renderPassType)
vec4 pass1() {
    vec4  pd     = texture(pdTex, texCoords);
    float depthX  = texture(edgeTex, texCoords).r;
    vec3  pixelX = vec3(gl_FragCoord.xy, depthX);
    
    vec4 sum   = pd;
    
    for (int j = 0; j < 4; j++) {
        for (int i = 1; i <= KERNEL_SIZE; i++) {
            vec3  pixelY   = vec3(gl_FragCoord.xy + (direction[j] * i), 0);
            vec2  neiTexel = pixelY.xy * inverseSize.xy;
            float depthY   = texture(edgeTex, neiTexel).r * KERNEL_SIZE;
            pixelY.z       = depthY;
            
            float dist       = distance(pixelX, pixelY);
            bool  continuity = dist < KERNEL_SIZE;
            
            if(!continuity)
                break;
            
            vec4  npd        = texture(pdTex, neiTexel);
            float sWeight    = 1.0 - dist / float(KERNEL_SIZE);
            float dWeight    = dot(pd, npd);
            
            sum += (sWeight * dWeight * npd);
        }
    }
    
    return normalize(sum);
}



subroutine(renderPassType)
vec4 pass2() {
    // Discretize Angle & Blending
    
    float angle = acos(texture(pdTex, texCoords).x);
    float qd = 3.141592 / float(Q_LEVEL);
    
    // -1 <= qlLevel < qxLevel < qrLevel <= Q_LEVEL
    //            0 <= qxLevel < Q_LEVEL
    int qxLevel = min(int(angle / qd), Q_LEVEL - 1);
    int qlLevel = qxLevel - 1 < 0 ? Q_LEVEL - 1 : qxLevel - 1;
    int qrLevel = (qxLevel + 1) % Q_LEVEL;
    
    float lDiff = angle - qxLevel * qd;
    float rDiff = qd - lDiff;
//    return vec4(angle / 3.141592);
    
    // Using TAM
    
    float tone = color.x; // 0 dark, 1 light

    vec2 texCoord;
    texCoord.y = asin(N.y) / 3.141592 * k;
    texCoord.x = atan(N.x, N.z) / 3.141592 * k;

    vec3 color1, color2;

    color1.r = texture(tam0, texCoord).r;
    color1.g = texture(tam1, texCoord).g;
    color1.b = texture(tam2, texCoord).b;
    color2.r = texture(tam3, texCoord).r;
    color2.g = texture(tam4, texCoord).g;
    color2.b = texture(tam5, texCoord).b;

    float tone2 = 1 - tone;

    int i;
    for (i = 1; i < 7; i++)
    {
        if (tone2 < t[i])
            break;
    }
    float right = (tone2 - t[i - 1]) * 6; // right
    float left = 1.0 - right;
    float ratioArray[7] = float[7](0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    ratioArray[i - 1] = left;
    ratioArray[i] = right;

    vec3 ratio1 = vec3(ratioArray[1], ratioArray[2], ratioArray[3]);
    vec3 ratio2 = vec3(ratioArray[4], ratioArray[5], ratioArray[6]);

    vec3 blended = 1 - vec3(dot(1 - color1, ratio1) + dot(1 - color2, ratio2));

    
    return texture(tam0, texCoords);
}

void main() {
    fragColor = renderPass();
}
