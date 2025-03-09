#version 410 core

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

uniform float edgeThreshold;

vec4 pass2() {
    vec4  pd     = texture(pdTex, texCoords);
    float depthX  = texture(edgeTex, texCoords).r * KERNEL_SIZE;
    vec3  pixelX = vec3(gl_FragCoord.xy, depthX);
    
    vec4 sum   = pd;
    int  count = 1;
    
    for (int j = 0; j < 4; j++) {
        for (int i = 1; i <= KERNEL_SIZE; i++) {
            vec3  pixelY   = vec3(gl_FragCoord.xy + (direction[j] * i), 0);
            vec2  neiTexel = pixelY.xy * inverseSize.xy;
            float depthY   = texture(edgeTex, neiTexel).r * KERNEL_SIZE;
            pixelY.z       = depthY;
            
            vec4  npd        = texture(pdTex, neiTexel);
            float dist       = distance(pixelX, pixelY);
            bool  continuity = dist < KERNEL_SIZE;
            float sWeight    = continuity ? 1.0 - dist / float(KERNEL_SIZE) : 0.0;
            float dWeight    = dot(pd, npd);
            
            sum += (sWeight * dWeight * npd) * (1.0 - i / float(KERNEL_SIZE));
            
            if(!continuity) {
                break;
            }
//            else
//                count++;
        }
    }
//    sum /= float(count);
    
    return sum;
}

void main() {
    fragColor = pass2();
}
