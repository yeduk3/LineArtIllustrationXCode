#version 330 core

out vec4 out_Color;

in vec2 texCoords;

uniform sampler2D pdTexture;

const float OFFSET = 0.04;
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

void main()
{
    vec4 pd = texture(pdTexture, texCoords);

    // if umbilic,
    if (pd.w == 1)
    {
        if (enableCaseTest)
            pd = vec4(1, 1, 1, 1);
        else
        {
            // find 4 neighboring, not umbilic, points
            vec2 neighbor;
            vec2 direction[4] = vec2[4](vec2(0, OFFSET), vec2(OFFSET, 0), vec2(0, -OFFSET * 10), vec2(-OFFSET, 0));

            vec4 sum = vec4(0);
            int count = 0;
            for (int i = 0; i < 4; i++)
            {
                neighbor = texCoords;
                vec4 nTex = vec4(1);
                int distByOffset = 0;
                while (texInRange(neighbor + direction[i]))
                {
                    neighbor += direction[i];
                    nTex = texture(pdTexture, neighbor);
                    distByOffset++;

                    // found!
                    if (nTex.a != 1 /* && abs(dot(nTex.xyz, vec3(1))) > 0.2*/)
                        break;
                }

                // if found,
                if (nTex.a != 1)
                {
                    sum += nTex * distByOffset;
                    count += distByOffset;
                    break;
                }
            }

            sum /= float(count);

            pd = sum;
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