#include "ShaderCommon.glsl"
#include "LightingCommon.glsl"

// Textures
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

in vec2 fs_Texcoord;
out vec4 out_f4Colour;

const float occlusion_strength = 1.5f;
void main() 
{
    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth);

    vec3 normal = SampleTexture(u_Normaltex, fs_Texcoord);
    vec3 position = SampleTexture(u_Positiontex, fs_Texcoord);
    vec3 color = SampleTexture(u_Colortex, fs_Texcoord);
    vec3 light = uf4Light.xyz;
    float lightRadius = uf4Light.w;
    out_f4Colour = vec4(0, 0, 0, 1.0);

    if(uiDisplayType == DISPLAY_LIGHTS)
    {
        //Put some code here to visualize the fragment associated with this point light
		out_f4Colour = vec4 (uf3LightCol, 1.0);
    }
    else
    {
		float distLight = length (light-position);
		float decay = max(1 - (distLight / lightRadius), 0);
		float clampedDotPdt = clamp (dot (normalize(normal), (light-position)/distLight), 0.0, 1.0);

		if (ubToonOn)
		{
			if (clampedDotPdt == 1.0)
				clampedDotPdt = 1.0;
			else if (clampedDotPdt >= 0.8)
				clampedDotPdt = 0.8;
			else if (clampedDotPdt >= 0.6)
				clampedDotPdt = 0.6;
			else if (clampedDotPdt >= 0.4)
				clampedDotPdt = 0.4;
			else if (clampedDotPdt >= 0.2)
				clampedDotPdt = 0.2;
			else
				clampedDotPdt = 0.0;
		}
		vec3 finalColour = (color * uf3LightCol * ufLightIl * clampedDotPdt) * decay;
		out_f4Colour = vec4(finalColour, 1.0);		// Because light and normal are both in view space.
    }
}