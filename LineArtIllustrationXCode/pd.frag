
#version 330 core

out vec4 out_Color;

in vec2 texCoords;

uniform sampler2D normalTexture;
uniform sampler2D positionTexture;

// const float OFFSET = 0.00002;
uniform float OFFSET;
// const float CLOSETOZERO = 0.0001;
uniform float CLOSETOZERO;

// Good Value Cand //
// TEAPOT
// OFFSET = 0.00001;
// CLOSETOZERO = 0.0003; or 0.0005;

uniform bool enableCaseTest;

void main()
{
    // init
    // out_Color = texture(positionTexture, texCoords);
    vec3 np = texture(normalTexture, texCoords).xyz;
    vec3 pp = texture(positionTexture, texCoords).xyz;

    vec2 biasedTexCoords = vec2(texCoords);
    if (biasedTexCoords.x + OFFSET >= 1.0)
        biasedTexCoords.x -= OFFSET;
    else
        biasedTexCoords.x += OFFSET;
    if (biasedTexCoords.y + OFFSET >= 1.0)
        biasedTexCoords.y -= OFFSET;
    else
        biasedTexCoords.y += OFFSET;

    vec3 n1 = texture(normalTexture, vec2(biasedTexCoords.x, texCoords.y)).xyz;
    vec3 p1 = texture(positionTexture, vec2(biasedTexCoords.x, texCoords.y)).xyz;
    vec3 n2 = texture(normalTexture, vec2(texCoords.x, biasedTexCoords.y)).xyz;
    vec3 p2 = texture(positionTexture, vec2(texCoords.x, biasedTexCoords.y)).xyz;

    // create ray
    vec3 tempvec = normalize(p1 - pp);
    vec3 localAxis2 = cross(np, tempvec);
    vec3 localAxis1 = cross(np, localAxis2);

    mat4 localbasis = mat4(vec4(localAxis1, 0), vec4(localAxis2, 0), vec4(np, 0), vec4(0, 0, 0, 1));
    localbasis = transpose(localbasis);
    mat4 localorigin = mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(-pp.x, -pp.y, 0, 1));
    mat4 localframe = localbasis * localorigin;

    p1 = (localframe * vec4(p1, 1)).xyz;
    p2 = (localframe * vec4(p2, 1)).xyz;
    n1 = (localframe * vec4(n1, 0)).xyz;
    n2 = (localframe * vec4(n2, 0)).xyz;

    vec4 rayp = vec4(0);
    vec2 ratio = vec2(n1.x / n1.z, n1.y / n1.z);
    vec4 ray1 = vec4(ratio.x, ratio.y, p1.x - (p1.z * ratio.x), p1.y - (p1.z * ratio.y));
    ratio = vec2(n2.x / n2.z, n2.y / n2.z);
    vec4 ray2 = vec4(ratio.x, ratio.y, p2.x - (p2.z * ratio.x), p2.y - (p2.z * ratio.y));

    // Handling Degeneracies
    float A = ray1.x * ray2.y - ray2.x * ray1.y;
    float B = ray1.x * ray2.w + ray2.y * ray1.z - ray2.x * ray1.w - ray1.y * ray2.z;
    float C = ray1.z * ray2.w - ray2.z * ray1.w;
    float D = B * B - 4 * A * C;

    vec3 maxPD, minPD;
    int umbilic = 0;

    vec3 caseTest = vec3(0);
    if (abs(A) < CLOSETOZERO && abs(B) < 2 * CLOSETOZERO)
    {
        // Locally planar cases
        caseTest.r = 1.0;

        vec3 xAxis = vec3(1, 0, 0);
        vec3 yAxis = vec3(0, 1, 0);
        vec3 zAxis = vec3(0, 0, 1);
        // not parallel test
        if (pow(dot(np, xAxis), 2) != dot(np, np))
        {
            maxPD = cross(xAxis, np);
        }
        else if (pow(dot(np, yAxis), 2) != dot(np, np))
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

        float k1 = 2 * A / (-B + sqrt(D));
        float k2 = 2 * A / (-B - sqrt(D));
        // choose greater curvature. since parabolic curvatures have only one 0 curvature and other not.
        float kk = abs(k1) < abs(k2) ? k1 : k2;

        // dont know what is max
        minPD = normalize(vec3(ray1.z * kk + ray1.x, ray1.w * kk + ray1.y, 0));
        maxPD = cross(np, maxPD);
    }
    else if (D < CLOSETOZERO * CLOSETOZERO)
    {
        // Umbilic points
        // mark fourth color as 1.
        // out onto new framebuffer (texture)
        // and read once again to fill this points.
        umbilic = 1;
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

    maxPD = (inverse(localbasis) * vec4(maxPD, 1)).xyz;
    minPD = (inverse(localbasis) * vec4(minPD, 1)).xyz;

    // if umbilic, 4-th color is 1, otherwise 0.
    if (enableCaseTest)
        out_Color = vec4(caseTest, umbilic);
    else
        out_Color = vec4(minPD, umbilic);
    // out_Color = vec4(maxPD, umbilic);
}