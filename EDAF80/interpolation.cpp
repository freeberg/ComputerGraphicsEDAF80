#include "interpolation.hpp"


glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{

	glm::vec4 linearV = glm::vec4(1.0f, x, 0.0f, 0.0f);
	glm::mat4 linearM = glm::mat4(0.0f);
	linearM[0] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	linearM[1] = glm::vec4(-1.0f, 1.0f, 0.0f, 0.0f);
	glm::mat4 posM = glm::mat4(0.0f);
	posM[0] = glm::vec4(p0, 0.0f);
	posM[1] = glm::vec4(p1, 0.0f);
	posM = glm::transpose(posM);
	linearM = glm::transpose(linearM);
	glm::vec3 newPos = (linearV * linearM * posM);

	return newPos;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
	glm::vec4 cubicV = glm::vec4(1.0f, x, x*x, x*x*x);
	glm::mat4 cubicM = glm::mat4(0.0f);
	cubicM[0] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	cubicM[1] = glm::vec4(-t, 0.0f, t, 0.0f);
	cubicM[2] = glm::vec4(2*t, t-3.0f, 3.0f - 2*t, -t);
	cubicM[3] = glm::vec4(-t, 2-t, t-2, t);
	glm::mat4 posM = glm::mat4(0.0f);
	posM[0] = glm::vec4(p0, 0.0f);
	posM[1] = glm::vec4(p1, 0.0f);
	posM[2] = glm::vec4(p2, 0.0f);
	posM[3] = glm::vec4(p3, 0.0f);
	posM = glm::transpose(posM);
	cubicM = glm::transpose(cubicM);
	glm::vec3 newPos = (cubicV * cubicM * posM);

	return newPos;
}
