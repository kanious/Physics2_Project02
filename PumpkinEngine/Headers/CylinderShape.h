#ifndef _CYLINDERSHAPE_H_
#define _CYLINDERSHAPE_H_

#include "iShape.h"
#include "glm\vec3.hpp"

NAMESPACE_BEGIN(Engine)

class ENGINE_API CCylinderShape : public iShape
{
private:
	glm::vec3		m_vHalfExtents;

private:
	explicit CCylinderShape();
	virtual ~CCylinderShape();
	virtual void Destroy();

public:
	glm::vec3& GetHalfExtents()	{ return m_vHalfExtents; }

private:
	RESULT Ready(eShapeType type, glm::vec3 half);
public:
	static CCylinderShape* Create(eShapeType type, glm::vec3 half);
};

NAMESPACE_END

#endif //_CYLINDERSHAPE_H_
