#include "ShaderCommon.glsl"

// Textures
uniform sampler2D u_Posttex;
uniform sampler2D u_GlowMask;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;
uniform sampler2D u_normalTex;
uniform sampler2D u_positionTex;
uniform sampler2D u_depthTex;

in vec2 vo_f2TexCoord;
out vec4 out_f4Colour;

const float occlusion_strength = 1.5f;
const mat3 m3Gaussian = mat3(vec3 (1,2,1), 
							 vec3 (2,4,2),
							 vec3 (1,2,1)) / 16.0f;
const float m5Gaussian[] = {    
                                1/273.0f, 4/273.0f, 7/273.0f, 4/273.0f, 1/273.0f,
							    4/273.0f, 16/273.0f, 26/273.0f, 16/273.0f, 4/273.0f,
							    7/273.0f, 26/273.0f, 41/273.0f, 26/273.0f, 7/273.0f,
							    4/273.0f, 16/273.0f, 26/273.0f, 16/273.0f, 4/273.0f,
							    1/273.0f, 4/273.0f, 7/273.0f, 4/273.0f, 1/273.0f
                           };
								
void main() 
{
    vec3 f3Colour = SampleTexture(u_Posttex, vo_f2TexCoord);
	if (ubBloomOn)
	{
		if (SampleTexture(u_GlowMask, vo_f2TexCoord).r)
		{
			vec3 f3BloomColour = vec3(0);
			for (int i = -1; i < 2; ++i)
			{
				int j = -1;
				f3BloomColour += (SampleTexture(u_GlowMask, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m3Gaussian[i+1].x);
				++j;
				f3BloomColour += (SampleTexture(u_GlowMask, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m3Gaussian[i+1].y);
				++j;
				f3BloomColour += (SampleTexture(u_GlowMask, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m3Gaussian[i+1].z);
				++j;
			}  
			f3Colour += f3Colour * f3BloomColour;  
		}
	}

	if (ubToonOn)
	{
		float fDotPdt = dot(SampleTexture(u_normalTex, vo_f2TexCoord).xyz, -(SampleTexture(u_positionTex, vo_f2TexCoord).xyz));
		if (fDotPdt < 0.1)
			f3Colour = vec3(0.0, 0.0, 0.0);
	}

	if (ubDOFOn)
	{
		float fDepth = SampleTexture(u_depthTex, vo_f2TexCoord).x;
		fDepth = linearizeDepth(fDepth);

		float fFocalLen = SampleTexture(u_depthTex, vec2(ufMouseTexX, ufMouseTexY)).x;
		fFocalLen = linearizeDepth(fFocalLen);

		float fLenQuant = 0.01f;
		fDepth = abs(fFocalLen - fDepth); 
		fDepth /= fLenQuant;

		vec3 f3BloomColour = vec3(0);
		if (fDepth >= 3.0f)
		{
            // 5x5 kernel - Maximum strength DOF
			for (int i = -2; i < 3; ++i)
			{
				int j = -2;
				f3BloomColour += (SampleTexture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m5Gaussian[(j+2)*5+ (i+2)]);
				++j;
				f3BloomColour += (SampleTexture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m5Gaussian[(j+2)*5+ (i+2)]);
				++j;
				f3BloomColour += (SampleTexture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m5Gaussian[(j+2)*5+ (i+2)]);
				++j;
				f3BloomColour += (SampleTexture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m5Gaussian[(j+2)*5+ (i+2)]);
				++j;
				f3BloomColour += (SampleTexture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m5Gaussian[(j+2)*5+ (i+2)]);
				++j;
			}

			if (!ubDOFDebug)
				f3Colour = f3BloomColour;
			else
				f3Colour = vec3(1.0f, 0.0f, 0.0f);
		}
		else if (fDepth >= 2.0f)
		{
            // 3x3 kernel - Medium strength DOF
			for (int i = -1; i < 2; ++i)
			{
				int j = -1;
				f3BloomColour += (SampleTexture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m3Gaussian[i+1].x);
				++j;
				f3BloomColour += (SampleTexture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m3Gaussian[i+1].y);
				++j;
				f3BloomColour += (SampleTexture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * m3Gaussian[i+1].z);
				++j;
			}
			if (!ubDOFDebug)
				f3Colour = f3BloomColour;
			else
				f3Colour = vec3(0.0f, 1.0f, 0.0f);
		}
		else if (fDepth >= 1.0f)
		{
            // 2x2 kernel - Minimum strength DOF
			for (int i = 0; i < 2; ++ i)
			{			
				for (int j = 0; j < 2; ++ j)
				{
					f3BloomColour += SampleTexture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz;
				}
			}
			f3BloomColour /= 4.0f;
			
			if (!ubDOFDebug)
				f3Colour = f3BloomColour;
			else
				f3Colour = vec3(0.0f, 0.0f, 1.0f);
		}
	}

    out_f4Colour = vec4(f3Colour, 1.0f);
}

