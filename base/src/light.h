#include <glm/glm.hpp>

using glm::vec3;

class Light
{
public:
	Light(vec3 position, float strength);
	float sampleStrength(float time);
	vec3 getPosition() { return m_position; }
	float getStrength() { return m_strength; }

private:
	vec3 m_position;
	float m_strength;
};