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

#ifdef COMPILEPS
uniform float cPosterizationLevel;

void PS()
{
    vec4 texColor = texture2D(sDiffMap, vScreenPos);
    vec3 grey  = vec3((texColor.r + texColor.g + texColor.b) / 3.0);
    vec3 grey1 = grey;

    grey = floor(grey * cPosterizationLevel) / cPosterizationLevel;

    texColor.rgb += (grey - grey1);

    gl_FragColor = texColor;
}
#endif
