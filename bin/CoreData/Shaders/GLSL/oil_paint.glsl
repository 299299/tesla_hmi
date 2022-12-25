#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vScreenPos;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

void PS()
{
    const int _Radius = 2;      //0~10
    vec3 mean[4] = vec3[](
        vec3(0),
        vec3(0),
        vec3(0),
        vec3(0)
    );

    vec3 sigma[4] = vec3[](
        vec3(0),
        vec3(0),
        vec3(0),
        vec3(0)
    );

    vec2 start[4] = vec2[](
        vec2(-_Radius, -_Radius), 
        vec2(-_Radius, 0), 
        vec2(0, -_Radius), 
        vec2(0, 0)
    );
    vec2 pos;
    vec3 col;
    for (int k = 0; k < 4; k++) {
        for(int i = 0; i <= _Radius; i++) {
            for(int j = 0; j <= _Radius; j++) {
                pos = vec2(i, j) + start[k];
                col = texture2D(sDiffMap, vScreenPos + pos * cGBufferInvSize).rgb;
                mean[k] += col;
                sigma[k] += col * col;
            }
        }
    }

    float sigma2;
 
    float n = pow(_Radius + 1, 2);
    vec4 color = texture2D(sDiffMap, vScreenPos);
    float min_ = 1;

    for (int l = 0; l < 4; l++) {
        mean[l] /= n;
        sigma[l] = abs(sigma[l] / n - mean[l] * mean[l]);
        sigma2 = sigma[l].r + sigma[l].g + sigma[l].b;

        if (sigma2 < min_) {
            min_ = sigma2;
            color.rgb = mean[l].rgb;
        }
    }
    gl_FragColor = color;
}