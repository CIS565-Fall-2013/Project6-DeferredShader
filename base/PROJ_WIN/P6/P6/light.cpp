#include <math.h>
#include "light.h"

Light::Light(vec3 pos, float str) :
m_position(pos),
m_strength(str)
{

}

float Light::sampleStrength(float time)
{
	return sin(time) * m_strength;
}