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
uniform highp float cEdgeThreshold;
uniform vec4 cEdgeColor;

vec3 normal_from_depth(float depth, highp vec2 texcoords) {
    vec2 offset1 = vec2(0.0, 1.0) * cGBufferInvSize;
    vec2 offset2 = vec2(1.0, 0.0) * cGBufferInvSize;
    
    float depth1 = DecodeDepth(texture2D(sEmissiveMap, texcoords + offset1).rgb);
    float depth2 = DecodeDepth(texture2D(sEmissiveMap, texcoords + offset2).rgb);
    
    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);
    
    highp vec3 normal = cross(p1, p2);
    normal.z = -normal.z;
    
    return normalize(normal);
}

vec4 GetPixelValue(highp vec2 uv) {
    float depth = DecodeDepth(texture2D(sEmissiveMap, uv).rgb);
    return vec4(normal_from_depth(depth, uv), depth);
}

void PS()
{
    vec4 col = texture2D(sDiffMap, vScreenPos);
    vec4 orValue = GetPixelValue(vScreenPos);
    vec4 sampledValue = vec4(0);
    sampledValue += GetPixelValue(vScreenPos + vec2(-1, -1) * cGBufferInvSize);
    sampledValue += GetPixelValue(vScreenPos + vec2(-1, 0) * cGBufferInvSize);
    sampledValue += GetPixelValue(vScreenPos + vec2(-1, 1) * cGBufferInvSize);
    sampledValue += GetPixelValue(vScreenPos + vec2(0, -1) * cGBufferInvSize);
    sampledValue += GetPixelValue(vScreenPos + vec2(0, 1) * cGBufferInvSize);
    sampledValue += GetPixelValue(vScreenPos + vec2(1, -1) * cGBufferInvSize);
    sampledValue += GetPixelValue(vScreenPos + vec2(1, 0) * cGBufferInvSize);
    sampledValue += GetPixelValue(vScreenPos + vec2(1, 1) * cGBufferInvSize);
    sampledValue /= 8;

    gl_FragColor = mix(col, cEdgeColor, step(cEdgeThreshold * cEdgeThreshold, dot(orValue - sampledValue, orValue - sampledValue)));
}
#endif
