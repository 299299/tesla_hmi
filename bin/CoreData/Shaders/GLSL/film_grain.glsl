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
    const float amount = 0.1;
    const float toRadians = 3.14 / 180;
    vec4 color = texture2D(sDiffMap, vScreenPos);

    float randomIntensity =
        fract( 10000 * sin(
                ( vScreenPos.x / cGBufferInvSize.x
                + vScreenPos.y / cGBufferInvSize.y
                * cElapsedTimePS) * toRadians)
        );

    randomIntensity *= amount;
    color.rgb += randomIntensity;
    gl_FragColor = color;
}
