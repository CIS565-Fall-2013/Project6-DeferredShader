#include "ShaderCommon.glsl"
#include "LightingCommon.glsl"

// Textures
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;
uniform sampler2D u_GlowMask;

in vec2 fs_Texcoord;
out vec4 out_Color;

const float occlusion_strength = 1.5f;
void main() 
{
    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth);

    vec3 normal = SampleTexture(u_Normaltex, fs_Texcoord);
    vec3 position = SampleTexture(u_Positiontex, fs_Texcoord);
    vec3 color = SampleTexture(u_Colortex, fs_Texcoord);
    vec3 light = uf4Light.xyz;
	vec3 glowMask = SampleTexture(u_GlowMask, fs_Texcoord).rrr;
    float lightRadius = uf4Light.w;

    switch (uiDisplayType) 
    {
        case DISPLAY_DEPTH:
            out_Color = vec4(vec3(lin_depth), 1.0f);
            break;
        case DISPLAY_NORMAL:
            out_Color = vec4(abs(normal), 1.0f);
            break;
        case DISPLAY_POSITION:
            out_Color = vec4(abs(position) / ufFar, 1.0f);
            break;
        case DISPLAY_COLOR:
            out_Color = vec4(color, 1.0);
            break;
		case DISPLAY_GLOWMASK:
			out_Color = vec4(glowMask, 1.0);
        case DISPLAY_LIGHTS:
        case DISPLAY_TOTAL:
            break;
    }	
}

