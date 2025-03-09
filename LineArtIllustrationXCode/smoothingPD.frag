#version 410 core

//subroutine vec4 renderPassType();
//subroutine uniform renderPassType renderPass;

// variables

uniform sampler2D edgeTex;
uniform sampler2D pdTex;
uniform vec3 inverseSize;
uniform mat4 viewMat;

in vec2 texCoords;

out vec4 fragColor;

// kernel size in pixels. recommandation is 5~10.
const int KERNEL_SIZE = 10;
const vec2 direction[4] = vec2[4](vec2(0, 1), vec2(1, 0), vec2(0, -1), vec2(-1, 0));

// pass #1 -

uniform float edgeThreshold;

float luma(vec3 color) {
  return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

//subroutine(renderPassType)
vec4 pass1() {
    return vec4(1);
}

// pass #2 - Calc Smoothings

//subroutine(renderPassType)
vec4 pass2() {
    vec2 texel = vec2(gl_FragCoord.xy) * inverseSize.xy;
    
    vec4  pd     = texture(pdTex, texCoords);
    float depth  = texture(edgeTex, texCoords).r * KERNEL_SIZE;
    vec3  pixelX = vec3(gl_FragCoord.xy, depth);
    
    vec4 sum   = pd;
    int  count = 1;
    
    for (int j = 0; j < 4; j++) {
        for (int i = 1; i <= KERNEL_SIZE; i++) {
            vec2  neiTexel = texCoords + (direction[j] * i) * inverseSize.xy;
            float depthY   = texture(edgeTex, neiTexel).r * KERNEL_SIZE;
            vec3  pixelY   = vec3(gl_FragCoord.xy + (direction[j] * i), depthY);
            vec4  npd      = texture(pdTex, neiTexel);
            
            float signFunc   = dot(pd, npd) > 0 ? 1 : -1;
            float dist       = distance(pixelX, pixelY);
            bool  continuity = dist < KERNEL_SIZE;
            float sWeight    = continuity ? 1 - dist / float(KERNEL_SIZE) : 0.0;  // after edge detection
            float dWeight    = abs(dot(pd, npd));
            
            sum += signFunc * sWeight * dWeight * npd;
            
            if(!continuity)
                break;
//            else
//                count++; 
        }
    }
//    sum /= float(count);
    
    return normalize(sum);
}

void main() {
//    fragColor = renderPass();
    fragColor = pass2();
}
