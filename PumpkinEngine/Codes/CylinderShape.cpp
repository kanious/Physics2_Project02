#include "pch.h"
#include "../Headers/CylinderShape.h"

USING(Engine)
USING(std)
USING(glm)

CCylinderShape::CCylinderShape()
    : m_vHalfExtents(vec3(0.f))
{
}

CCylinderShape::~CCylinderShape()
{
}

void CCylinderShape::Destroy()
{
}

RESULT CCylinderShape::Ready(eShapeType type, vec3 half)
{
    m_shapeType = type;
    m_vHalfExtents = half;

    return PK_NOERROR;
}

CCylinderShape* CCylinderShape::Create(eShapeType type, vec3 half)
{
    CCylinderShape* pInstance = new CCylinderShape();
    if (PK_NOERROR != pInstance->Ready(type, half))
    {
        pInstance->Destroy();
        pInstance = nullptr;
    }

    return pInstance;
}
