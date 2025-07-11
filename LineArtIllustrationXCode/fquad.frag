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
        smoothedPDColor1 = weightSum > 0 ? vec4(normalize(sum),1) : vec4(pd,1);
    else
        smoothedPDColor2 = weightSum > 0 ? vec4(normalize(sum),1) : vec4(pd,1);
}

// Quantization  level recommandation is 12~24.
uniform int QUANTIZATION_LEVEL = 24;

uniform sampler2D tam0;
uniform sampler2D tam1;
uniform sampler2D texCoordTexture;
#define PI 3.1415926535897932384626433832795

// https://github.com/hughsk/glsl-hsv2rgb/blob/master/index.glsl
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

mat2 texRotateMat(float angle) {
    float co = cos(angle);
    float si = sin(angle);
    return mat2(vec2(co, si), vec2(-si, co));
}

vec3 quantizedAndBlendedColor(int Q_LEVEL, float angle) {
    // split pi into same-width Q_LEVEL bins.
    float qd = PI / float(Q_LEVEL);
    
    // -1 <= qlLevel < qxLevel < qrLevel <= Q_LEVEL
    //            0 <= qxLevel < Q_LEVEL
    int qxLevel = min(int(angle / qd), Q_LEVEL - 1);
    int qlLevel = qxLevel - 1 < 0 ? Q_LEVEL - 1 : qxLevel - 1;
    int qrLevel = (qxLevel + 1) % Q_LEVEL;
    
    float lDiff = angle - qxLevel * qd;
    float rDiff = qd - lDiff;
    float dilAngle = qlLevel * qd;
    float dixAngle = qxLevel * qd;
    float dirAngle = qrLevel * qd;
    
    // mix weight l, x, r
    float wl = 1/(dilAngle - angle);
    float wx = 1/(angle - dixAngle);
    float wr = 1/(angle - dirAngle);
    float wsum = wl+wx+wr;
    
    // rotated texture l, x, r
//    vec2 tc = texture(texCoordTexture, texCoord).xy;
    vec2 tc  = texCoord.xy;
    vec2 rtl = texRotateMat(-dilAngle) * tc*5;
    vec2 rtx = texRotateMat(-dixAngle) * tc*5;
    vec2 rtr = texRotateMat(-dirAngle) * tc*5;
    
    // stroke l, x, r
    float stl = wl/wsum;
    float stx = wx/wsum;
    float str = wr/wsum;
    
    float tone = 1-texture(phongTexture, texCoord).x;
    int tlevel = int(floor(tone * 6));
//    strokeMappedColor = vec4(vec3(max(floor(tone*6),0)/6), 1);
//    return;
    if(tlevel < 1) {
        stl *= 1;
        stx *= 1;
        str *= texture(tam0, rtl).r;
    } else if(tlevel == 1) {
        stl *= 1;
        stx *= texture(tam0, rtx).r;
        str *= texture(tam0, rtr).g;
    } else if(tlevel == 2) {
        stl *= texture(tam0, rtl).r;
        stx *= texture(tam0, rtx).g;
        str *= texture(tam0, rtr).b;
    } else if(tlevel == 3) {
        stl *= texture(tam0, rtl).g;
        stx *= texture(tam0, rtx).b;
        str *= texture(tam1, rtr).r;
    } else if(tlevel == 4) {
        stl *= texture(tam0, rtl).b;
        stx *= texture(tam1, rtx).r;
        str *= texture(tam1, rtr).g;
    } else if(tlevel == 5) {
        stl *= texture(tam1, rtl).r;
        stx *= texture(tam1, rtx).g;
        str *= texture(tam1, rtr).b;
    } else {
        stl *= texture(tam1, rtl).g;
        stx *= texture(tam1, rtx).b;
        str *= 0;
    }
    
    return vec3(stl+stx+str);
}

uniform mat4 viewMat;
uniform mat4 projMat;

//#define ANGLE_VISUALIZE

subroutine(renderPassType)
void strokeMapping() {
    // Discretize Angle & Blending
    
    vec3 pd  = texture(pdTexture,texCoord).xyz;
    vec3 pos = texture(positionTexture,texCoord).xyz;
    vec4 vpd  = projMat * viewMat * vec4(pos+pd, 1);
    vec4 vpos = projMat * viewMat * vec4(pos, 1);
    pd  = vpd.xyz / vpd.w;
    pos = vpos.xyz / vpos.w;
    pd = (pd - pos);
    
    float angle = pd.x == 0 ? PI/2 : atan(pd.y, pd.x); // [-pi, pi]
//    angle = angle * sign(angle); // [0, pi]
    if(angle < 0) angle = PI+angle;
    
    
#ifdef ANGLE_VISUALIZE
    float normAngle = (angle) / (PI);
    strokeMappedColor = vec4(hsv2rgb(vec3(normAngle, 1, 1)), 1);
    return;
#endif
    
    vec3 q1 = quantizedAndBlendedColor(QUANTIZATION_LEVEL, angle);
    vec3 q2 = quantizedAndBlendedColor(QUANTIZATION_LEVEL/2, angle);
    
    strokeMappedColor = vec4((q1+q2)/2, 1);
    return;
}

subroutine(renderPassType)
void result() {
    fragColor = texture(tex, texCoord);
}

void main() {
    renderPass();
}
