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
uniform int KERNEL_SIZE = 5;
uniform vec2 direction[4] = vec2[4](vec2(0, 1), vec2(1, 0), vec2(0, -1), vec2(-1, 0));

// Quantization  level recommandation is 12~24.
uniform int Q_LEVEL = 12;


uniform sampler2D tam0;
uniform sampler2D tam1;
uniform sampler2D phong;

subroutine(renderPassType)
vec4 pass1() {
    vec4  pd     = texture(pdTex, texCoords);
    float depthX  = texture(edgeTex, texCoords).r;
    vec3  pixelX = vec3(gl_FragCoord.xy, depthX);
    
    vec4 sum   = pd;
    
//    for (int j = 0; j < 4; j++) {
//        for (int i = 1; i <= KERNEL_SIZE; i++) {
//            vec3  pixelY   = vec3(gl_FragCoord.xy + (direction[j] * i), 0);
//            vec2  neiTexel = pixelY.xy * inverseSize.xy;
//            pixelY.z       = texture(edgeTex, neiTexel).r * KERNEL_SIZE;
//            
//            float dist       = distance(pixelX, pixelY);
//            bool  continuity = dist < KERNEL_SIZE;
//            
//            if(!continuity)
//                break;
//            
//            vec4  npd        = texture(pdTex, neiTexel);
////            float sWeight    = 1.0 - dist / float(KERNEL_SIZE);
////            float dWeight    = dot(pd, npd);
//            
//            sum += (1.0 - dist / float(KERNEL_SIZE) * dot(pd, npd) * npd);
//        }
//    }
    
    return normalize(sum);
}


uniform float t[7] = float[7](0.0, float(1) / 6, float(2) / 6, float(3) / 6, float(4) / 6, float(5) / 6, 1.0);

subroutine(renderPassType)
vec4 pass2() {
    // Discretize Angle & Blending
    
    // 0 < angel < pi
    float angle = acos(texture(pdTex, texCoords).x);
    // split pi into same-width Q_LEVEL bins.
    float qd = 3.141592 / float(Q_LEVEL);
    
    // -1 <= qlLevel < qxLevel < qrLevel <= Q_LEVEL
    //            0 <= qxLevel < Q_LEVEL
    int qxLevel = min(int(angle / qd), Q_LEVEL - 1);
    int qlLevel = qxLevel - 1 < 0 ? Q_LEVEL - 1 : qxLevel - 1;
    int qrLevel = (qxLevel + 1) % Q_LEVEL;
    
    float lDiff = angle - qxLevel * qd;
    float rDiff = qd - lDiff;
    float dixAngle = qxLevel * qd;
    float dilAngle = qlLevel * qd;
    float dirAngle = qrLevel * qd;
    float blendedAngle = (dixAngle * qd + dilAngle * lDiff + dirAngle * rDiff) / (2 * qd);
//    return vec4(angle / 3.141592); // Original Angle
//    return vec4(float(qxLevel) / Q_LEVEL); // Quantized Angle
//    return vec4(blendedAngle / 3.141952);
    
    
    // Using TAM
    
    float tone = texture(phong, texCoords).x; // 0 dark, 1 light

//    vec2 texCoord;
//    texCoord.y = asin(N.y) / 3.141592 * k;
//    texCoord.x = atan(N.x, N.z) / 3.141592 * k;

    vec3 color1, color2;
    
    float co = cos(blendedAngle);
    float si = sin(blendedAngle);
    mat2 rot = mat2(vec2(co, si), vec2(-si, co));
    vec2 rotatedTexCoords = rot * texCoords * 5;

    color1 = texture(tam0, rotatedTexCoords).rgb;
    color2 = texture(tam1, rotatedTexCoords).rgb;

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

    
    return vec4(blended, 1);
}

void main() {
    fragColor = renderPass();
}
