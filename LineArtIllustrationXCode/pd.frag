
#version 330 core

out vec4 out_Color;

in vec2 texCoords;

uniform sampler2D normalTexture;
uniform sampler2D positionTexture;

// const float OFFSET = 0.00002;
//uniform float OFFSET;
uniform vec2 inverseSize;
// const float CLOSETOZERO = 0.0001;
uniform float CLOSETOZERO;

// Good Value Cand //
// CLOSETOZERO = 0.0003; or 0.0005;

uniform bool enableCaseTest;

const int k = 1;

void main()
{
    //
    // init
    //
    vec3 np = texture(normalTexture, texCoords).xyz;
    vec3 pp = texture(positionTexture, texCoords).xyz;
    
    float dx = inverseSize.x * k;
    float dy = inverseSize.y * k;

    vec2 biasedTexCoords = vec2(texCoords);
    if (biasedTexCoords.x + dx >= 1.0)
        biasedTexCoords.x -= dx;
    else
        biasedTexCoords.x += dx;
    if (biasedTexCoords.y + dy >= 1.0)
        biasedTexCoords.y -= dy;
    else
        biasedTexCoords.y += dy;

    vec3 n1 = texture(normalTexture, vec2(biasedTexCoords.x, texCoords.y)).xyz;
    vec3 p1 = texture(positionTexture, vec2(biasedTexCoords.x, texCoords.y)).xyz;
    vec3 n2 = texture(normalTexture, vec2(texCoords.x, biasedTexCoords.y)).xyz;
    vec3 p2 = texture(positionTexture, vec2(texCoords.x, biasedTexCoords.y)).xyz;

    //
    // Create Ray
    //
    vec3 tempvector = normalize(p1 - pp);
    vec3 localAxis2 = normalize(cross(np, tempvector));
    vec3 localAxis1 = cross(np, localAxis2);

    // Local Frame
    mat4 localbasis = mat4(vec4(localAxis1, 0), vec4(localAxis2, 0), vec4(np, 0), vec4(0, 0, 0, 1));
//    localbasis = transpose(localbasis); // 헷갈림...
    mat4 localorigin = mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(-pp.x, -pp.y, -pp.z, 1));
    mat4 localframe = localbasis * localorigin;

    p1 = (localframe * vec4(p1, 1)).xyz;
    p2 = (localframe * vec4(p2, 1)).xyz;
    n1 = (localframe * vec4(n1, 0)).xyz;
    n2 = (localframe * vec4(n2, 0)).xyz;

    vec4 rayp  = vec4(0);
    vec2 ratio = vec2(n1.x / n1.z, n1.y / n1.z);
    vec4 ray1  = vec4(ratio.x, ratio.y, p1.x - (p1.z * ratio.x), p1.y - (p1.z * ratio.y));
         ratio = vec2(n2.x / n2.z, n2.y / n2.z);
    vec4 ray2  = vec4(ratio.x, ratio.y, p2.x - (p2.z * ratio.x), p2.y - (p2.z * ratio.y));

    // Handling Degeneracies
    float A = ray1.z * ray2.w - ray2.z * ray1.w;
    float B = ray1.x * ray2.w + ray2.y * ray1.z - ray2.x * ray1.w - ray1.y * ray2.z;
    float C = ray1.x * ray2.y - ray2.x * ray1.y;
    float D = B * B - 4 * A * C;

    vec3 maxPD, minPD;
    int umbilic = 1;

    vec3 caseTest = vec3(0);
    
    if (abs(A) < CLOSETOZERO && abs(B) < 2 * CLOSETOZERO)
    {
        // Locally planar cases
        caseTest.r = 1.0;

        vec3 xAxis = vec3(1, 0, 0);
        vec3 yAxis = vec3(0, 1, 0);
        vec3 zAxis = vec3(0, 0, 1);
        // not parallel test
        if (abs(dot(np, xAxis)) != 1)
        {
            maxPD = cross(xAxis, np);
        }
        else if (abs(dot(np, yAxis)) != 1)
        {
            maxPD = cross(yAxis, np);
        }
        else
        {
            maxPD = cross(zAxis, np);
        }
        minPD = cross(maxPD, np);
    }
    else if (abs(A) < CLOSETOZERO)
    {
        // Locally near parabolic
        caseTest.g = 1.0;

        float k1 = (-B + sqrt(D)) / (2 * A);
        float k2 = (-B - sqrt(D)) / (2 * A);
        // choose greater curvature. since parabolic curvatures have only one 0 curvature and other not.
        float kk = abs(k1) < abs(k2) ? k1 : k2;

        minPD = normalize(vec3(ray1.z * kk + ray1.x, ray1.w * kk + ray1.y, 0));
        maxPD = cross(np, maxPD);
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

        float k1 = 2 * A / (-B + sqrt(D));
        float k2 = 2 * A / (-B - sqrt(D));
        // make k1's curvature greater than k2's curvature
        if (abs(k1) > abs(k2))
        {
            caseTest.r = 1.0;

            float tmp = k1;
            k1 = k2;
            k2 = tmp;
        }

        maxPD = normalize(vec3(ray1.z * k1 + ray1.x, ray1.w * k1 + ray1.y, 0));
        minPD = normalize(vec3(ray1.z * k2 + ray1.x, ray1.w * k2 + ray1.y, 0));
    }

    maxPD = (inverse(localbasis) * vec4(maxPD, 0)).xyz;
    minPD = (inverse(localbasis) * vec4(minPD, 0)).xyz;

    // if umbilic, 4-th color is 0, otherwise 1.
    if (enableCaseTest)
        out_Color = vec4(caseTest, umbilic);
    else
        out_Color = vec4(minPD, umbilic);
    // out_Color = vec4(maxPD, umbilic);
}
