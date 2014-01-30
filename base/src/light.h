#include <glm/glm.hpp>

using glm::vec3;

class Light
{
public:
	Light(vec3 position, float strength);
	Light(vec3 position, float strength, float rate, float offset=0);
	float sampleStrength(float time);
	vec3 getPosition() { return m_position; }
	float getStrength() { return m_strength; }

private:
	vec3 m_position;
	float m_strength;
	float m_rate; // rate at which the strength of the light fluctuates
	float m_sampleOffset;
};