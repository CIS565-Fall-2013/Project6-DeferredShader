#include "ShaderCommon.glsl"
#include "LightingCommon.glsl"

// Textures
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

in vec2 vo_f2TexCoord;
out vec4 out_f4Colour;

const float occlusion_strength = 1.5f;
void main() 
{
    vec3 f3Normal = SampleFragmentNormal(u_Normaltex, vo_f2TexCoord);
    vec3 f3Position = SampleTexture(u_Positiontex, vo_f2TexCoord);
    vec3 f3Colour = SampleTexture(u_Colortex, vo_f2TexCoord);

    vec3 f3FinalColour = vec3(0.0f);

	float fDistToLight = length(uf4Light.xyz - f3Position);
	float fDecay = max(1.0f - (fDistToLight / uf4Light.w), 0.0f);
	float fClampedDotPdt = clamp(dot(normalize(f3Normal), (uf4Light.xyz - f3Position)/fDistToLight), 0.0f, 1.0f);

	if (ubToonOn)
	{
		if (fClampedDotPdt == 1.0f)
			fClampedDotPdt = 1.0f;
		else if (fClampedDotPdt >= 0.8f)
			fClampedDotPdt = 0.8f;
		else if (fClampedDotPdt >= 0.6f)
			fClampedDotPdt = 0.6f;
		else if (fClampedDotPdt >= 0.4f)
			fClampedDotPdt = 0.4f;
		else if (fClampedDotPdt >= 0.2f)
			fClampedDotPdt = 0.2f;
		else
			fClampedDotPdt = 0.0f;
	}
	f3FinalColour = (f3Colour * uf3LightCol * ufLightIl * fClampedDotPdt) * fDecay;
    
    out_f4Colour = vec4(f3FinalColour, 1.0);		// Because light and normal are both in view space.
}