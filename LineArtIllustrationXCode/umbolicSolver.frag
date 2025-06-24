#version 330 core

out vec4 out_Color;

in vec2 texCoords;

uniform sampler2D pdTexture;

const float stepSize = 10;
uniform vec2 inverseSize;
uniform bool enableCaseTest;

bool texInRange(vec2 texCoord)
{
    if (texCoord.x < 0.0 || 1.0 < texCoord.x)
        return false;
    else if (texCoord.y < 0.0 || 1.0 < texCoord.y)
        return false;
    else
        return true;
}

uniform int MAXDIST = 10;
uniform ivec2 direction[4] = ivec2[4](ivec2(0, 1), ivec2(1, 0), ivec2(0, -1), ivec2(-1, 0));
void main()
{
    vec4 pd = texture(pdTexture, texCoords);
    
//    float dx = inverseSize.x * stepSize;
//    float dy = inverseSize.y * stepSize;
//    vec2 direction[4] = vec2[4](vec2(0, dy), vec2(dx, 0), vec2(0, -dy), vec2(-dx, 0));
    
    // if umbilic,
    if (pd.a == 0)
    {
        if (enableCaseTest)
            pd = vec4(1, 1, 1, 1);
        else
        {
            // find 4 neighboring, not umbilic, points

            vec4 sum = vec4(0);
            int count = 0;
            ivec2 neighbor;
            for (int i = 0; i < 4; i++)
            {
                neighbor = ivec2(gl_FragCoord.xy);
                vec4 nTex = vec4(0);
                int dist = 1;
                
                while (nTex.a == 0 && dist < MAXDIST)
                {
                    neighbor += direction[i];
                    nTex = texelFetch(pdTexture, neighbor, 0);
                }

                // if found,
                if (nTex.a == 1)
                {
                    sum += nTex * (MAXDIST-dist);
                    count += (MAXDIST-dist);
                }
            }

            sum /= float(count);

            pd = vec4(sum.xyz, 1);
        }
    }

    // assume that the all directions are set well done

    // project into screen surface

    // vec3 xAxis = vec3(1, 0, 0);
    // float cosTheta = dot(xAxis, vec3(pd.xy, 0)) / length(pd.xyz);
    // float angle = acos(cosTheta);

    // out_Color = vec4(angle / 3.141592, 0, 0, 1);
    out_Color = pd;
}
