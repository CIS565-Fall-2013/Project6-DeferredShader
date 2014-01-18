#include <math.h>
#include "light.h"

Light::Light(vec3 pos, float str) :
m_position(pos),
m_strength(str),
m_rate(0),
m_sampleOffset(0)
{
}

Light::Light(vec3 pos, float str, float rate, float offset) :
m_position(pos),
m_strength(str),
m_rate(rate),
m_sampleOffset(offset)
{
}


float Light::sampleStrength(float time)
{
	if (m_rate == 0)
		return m_strength;

	float coeff = sin(1.0 * time / (m_rate) + m_sampleOffset);

	if (coeff < 0)
		coeff = 0;

	return coeff * m_strength;
}