#version 410 core

in vec2 texCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 sobelColor;
layout(location = 2) out vec4 pdColor;
layout(location = 3) out vec4 caseTestColor;
layout(location = 4) out vec4 smoothedPDColor1;
layout(location = 5) out vec4 smoothedPDColor2;
layout(location = 6) out vec4 strokeMappedColor;

uniform sampler2D tex;
uniform sampler2D phongTexture;
uniform sampler2D normalTexture;
uniform sampler2D positionTexture;


subroutine void renderPassType();
subroutine uniform renderPassType renderPass;


// Edge Detection
uniform vec2 inverseSize;
uniform float edgeThreshold = 0.1f;

float luma(vec3 color) {
  return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}
subroutine(renderPassType)
void sobelFilter() {
    ivec2 tc = ivec2(gl_FragCoord.xy);
    float s00 = luma(texelFetchOffset(phongTexture, tc, 0, ivec2(-1,  1)).rgb);
    float s10 = luma(texelFetchOffset(phongTexture, tc, 0, ivec2(-1,  0)).rgb);
    float s20 = luma(texelFetchOffset(phongTexture, tc, 0, ivec2(-1, -1)).rgb);
    float s01 = luma(texelFetchOffset(phongTexture, tc, 0, ivec2( 0,  1)).rgb);
    float s21 = luma(texelFetchOffset(phongTexture, tc, 0, ivec2( 0, -1)).rgb);
    float s02 = luma(texelFetchOffset(phongTexture, tc, 0, ivec2( 1,  1)).rgb);
    float s12 = luma(texelFetchOffset(phongTexture, tc, 0, ivec2( 1,  0)).rgb);
    float s22 = luma(texelFetchOffset(phongTexture, tc, 0, ivec2( 1, -1)).rgb);
    
    // sobel filter
    float sx = s00 + 2*s10 + s20 - (s02 + 2*s12 + s22);
    float sy = s00 + 2*s01 + s02 - (s20 + 2*s21 + s22);
    float dist = sx*sx + sy*sy;

    sobelColor = vec4(int(dist > edgeThreshold));
}

// Estimating Principal Direction
const float CLOSETOZERO = 0.0000001;

subroutine(renderPassType)
void estimatePD() {
    //
    // init
    //
    ivec2 tc = ivec2(gl_FragCoord.xy);
    vec3 np = texelFetch(normalTexture,   tc, 0).xyz;
    vec3 pp = texelFetch(positionTexture, tc, 0).xyz;
    vec3 n1 = texelFetchOffset(normalTexture,   tc, 0, ivec2(1, 0)).xyz;
    vec3 p1 = texelFetchOffset(positionTexture, tc, 0, ivec2(1, 0)).xyz;
    vec3 n2 = texelFetchOffset(normalTexture,   tc, 0, ivec2(0, 1)).xyz;
    vec3 p2 = texelFetchOffset(positionTexture, tc, 0, ivec2(0, 1)).xyz;
    
    //
    // Create Ray
    //
    vec3 tempvector = normalize(p1 - pp);
    vec3 localAxis2 = normalize(cross(np, tempvector));
    vec3 localAxis1 = cross(np, localAxis2);

    // Local Frame
    mat4 localbasis  = mat4(vec4(localAxis1, 0), vec4(localAxis2, 0), vec4(np, 0), vec4(0, 0, 0, 1));
    mat4 localorigin = mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(-pp.x, -pp.y, -pp.z, 1));
    mat4 localframe  = transpose(localbasis) * localorigin;

    p1 = (localframe * vec4(p1, 1)).xyz;
    p2 = (localframe * vec4(p2, 1)).xyz;
    n1 = normalize((localframe * vec4(n1, 0)).xyz);
    n2 = normalize((localframe * vec4(n2, 0)).xyz);

    vec4 rayp  = vec4(0);
    vec2 ratio = vec2(n1.x / n1.z, n1.y / n1.z);
    vec4 ray1  = vec4(ratio.x, ratio.y, p1.x - (p1.z * ratio.x), p1.y - (p1.z * ratio.y));
         ratio = vec2(n2.x / n2.z, n2.y / n2.z);
    vec4 ray2  = vec4(ratio.x, ratio.y, p2.x - (p2.z * ratio.x), p2.y - (p2.z * ratio.y));

    // Handling Degeneracies
    // A*la^2 + B*la + C = 0
    float A = ray1.x * ray2.y - ray2.x * ray1.y;
    float B = ray1.x * ray2.w + ray2.y * ray1.z - ray2.x * ray1.w - ray1.y * ray2.z;
    float C = ray1.z * ray2.w - ray2.z * ray1.w;
    float D = B * B - 4 * A * C;

    vec3 minPD = vec3(0);
    int umbilic = 1;

    vec3 caseTest = vec3(0);
    
    if (abs(A) < CLOSETOZERO && abs(B) < CLOSETOZERO) {
        // Locally planar cases
        caseTest.r = 1.0;

        vec3 maxPD = vec3(0);
        // not parallel test
        if (abs(np.x) < 1-CLOSETOZERO)
            maxPD = normalize(cross(vec3(1, 0, 0), np));
        else if (abs(np.y) < 1-CLOSETOZERO)
            maxPD = normalize(cross(vec3(0, 1, 0), np));
        else
            maxPD = normalize(cross(vec3(0, 0, 1), np));
        minPD = cross(maxPD, np);
    }
    else if (abs(A) < CLOSETOZERO) {
        // Locally near parabolic
        caseTest.g = 1.0;

        float kk = (2 * A) / (-B + sqrt(D));
        minPD = normalize(vec3(ray1.z * kk + ray1.x, ray1.w * kk + ray1.y, 0));
    }
    else if (D < CLOSETOZERO * CLOSETOZERO)
    {
        // Umbilic points
        caseTest = vec3(1);
        
        // mark fourth color as 1.
        // out onto new (texture)
        // and read once again to fill this points.
        umbilic = 0;
    }
    else
    {
        // General cases
        caseTest.b = 1.0;

        float k1 = (2 * A) / (-B + sqrt(D));
        float k2 = (2 * A) / (-B - sqrt(D));
        
        // make k1's curvature greater than k2's curvature
        float kk = abs(k1) < abs(k2) ? k2 : k1;
        minPD = vec3(ray1.z * kk + ray1.x, ray1.w * kk + ray1.y, 0);
    }

    minPD = normalize(minPD);
    
    float ma = max(minPD.x, max(minPD.y, minPD.z));
    float mi = min(minPD.x, min(minPD.y, minPD.z));
    if(ma < abs(mi)) minPD = -minPD;

    // if umbilic, 4-th color is 0, otherwise 1.
    caseTestColor = vec4(caseTest, umbilic);
    pdColor = vec4(minPD, umbilic);
}


uniform sampler2D pdTexture;
uniform sampler2D edgeTexture;
uniform int KERNEL_SIZE = 10;
uniform vec2 direction[4] = vec2[4](vec2(0, 1), vec2(1, 0), vec2(0, -1), vec2(-1, 0));
uniform int smoothTarget;

subroutine(renderPassType)
void smoothingPD() {
    vec3  pd     = texture(pdTexture, texCoord).xyz;
    float depthX  = texture(edgeTexture, texCoord).r * KERNEL_SIZE;
    vec3  fragX = vec3(gl_FragCoord.xy, depthX);
    
    vec3 sum   = pd;
    float weightSum = 0;
    
    for (int j = 0; j < 4; j++) {
        for (int i = 1; i <= KERNEL_SIZE; i++) {
            vec3  fragY    = fragX + vec3(direction[j] * i, 0);
            vec2  neiTexel = fragY.xy * inverseSize.xy;
            float depthY   = texture(edgeTexture, neiTexel).r * KERNEL_SIZE;
            fragY.z       = depthY;
            
            float dist       = distance(fragX, fragY);
            if(dist >= KERNEL_SIZE) break;
            
            vec3  npd        = texture(pdTexture, neiTexel).xyz;
            float sWeight    = 1.0 - dist / float(KERNEL_SIZE);
            float dWeight    = dot(pd, npd);
            float totalWeight = sWeight * dWeight;
            
            sum += (totalWeight * npd);
            weightSum += totalWeight;
        }
    }
    
    if(smoothTarget == 0)
        smoothedPDColor1 = weightSum > 0 ? vec4(normalize(sum/weightSum),1) : vec4(pd,1);
    else
        smoothedPDColor2 = weightSum > 0 ? vec4(normalize(sum/weightSum),1) : vec4(pd,1);
}

// Quantization  level recommandation is 12~24.
uniform int Q_LEVEL = 12;
uniform float t[7] = float[7](0.0, float(1) / 6, float(2) / 6, float(3) / 6, float(4) / 6, float(5) / 6, 1.0);

uniform sampler2D tam0;
uniform sampler2D tam1;

subroutine(renderPassType)
void strokeMapping() {
    // Discretize Angle & Blending
    
    // 0 < angel < pi
//    float angle = acos(texture(pdTex, texCoords).x);
    vec3 pd = texture(pdTexture,texCoord).xyz;
    float angle = atan(pd.y, pd.x);
//    return 
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
    
    float tone = texture(phongTexture, texCoord).x; // 0 dark, 1 light

//    vec2 texCoord;
//    texCoord.y = asin(N.y) / 3.141592 * k;
//    texCoord.x = atan(N.x, N.z) / 3.141592 * k;

    vec3 color1, color2;
    
    float co = cos(blendedAngle);
    float si = sin(blendedAngle);
    mat2 rot = mat2(vec2(co, si), vec2(-si, co));
    vec2 rotatedTexCoords = rot * texCoord * 5;

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

    strokeMappedColor = vec4(blended, 1);
//    strokeMappedColor = vec4(vec3(angle / 3.141592), 1);
}

subroutine(renderPassType)
void result() {
    fragColor = texture(tex, texCoord);
}

void main() {
    renderPass();
}
