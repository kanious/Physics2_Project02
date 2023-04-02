#ifndef _CONVERSION_H_
#define _CONVERSION_H_

#include "EngineDefines.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "iShape.h"
#include "BoxShape.h"
#include "PlaneShape.h"
#include "SphereShape.h"
#include "CylinderShape.h"

#include "iRigidBody.h"
#include "RigidBody.h"

NAMESPACE_BEGIN(Engine)

static void CastBulletQuaternion(const glm::quat& in, btQuaternion* out)
{
	out->setX(in.x);
	out->setY(in.y);
	out->setZ(in.z);
	out->setW(in.w);
}

static void CastBulletVector3(const glm::vec3& in, btVector3* out)
{
	out->setX(in.x);
	out->setY(in.y);
	out->setZ(in.z);
}

static void CastBulletScalar(const _float in, btScalar* out)
{
	*out = in;
}

static void CastGLMQuat(const btQuaternion& in, glm::quat* out)
{
	out->x = in.x();
	out->y = in.y();
	out->z = in.z();
	out->w = in.w();
}

static void CastGLMVec3(const btVector3& in, glm::vec3* out)
{
	out->x = in.x();
	out->y = in.y();
	out->z = in.z();
}

static void CastGLMVec4(const btVector4& in, glm::vec4* out)
{
	out->x = in.x();
	out->y = in.y();
	out->z = in.z();
	out->w = in.w();
}

static btCollisionShape* CastBulletShape(iShape* shape)
{
	switch (shape->GetShapeType())
	{

	case eShapeType::Box:
	{
		CBoxShape* box = dynamic_cast<CBoxShape*>(shape);
		btVector3 halfExtents;
		CastBulletVector3(box->GetHalfExtents(), &halfExtents);
		btBoxShape* btBox = new btBoxShape(halfExtents);
		return btBox;
	}
	break;

	case eShapeType::Plane:
	{
		CPlaneShape* plane = dynamic_cast<CPlaneShape*>(shape);
		btVector3 normal;
		btScalar planeConstant;
		CastBulletVector3(plane->GetNormal(), &normal);
		CastBulletScalar(plane->GetDotProduct(), &planeConstant);
		btStaticPlaneShape* btPlane = new btStaticPlaneShape(normal, planeConstant);
		return btPlane;
	}
		break;

	case eShapeType::Ghost:
	{
		btEmptyShape* btEmpty = new btEmptyShape();
		return btEmpty;
	}
		break;

	case eShapeType::Sphere:
	{
		CSphereShape* sphere = dynamic_cast<CSphereShape*>(shape);
		btScalar radius;
		CastBulletScalar(sphere->GetRadius(), &radius);
		btSphereShape* btSphere = new btSphereShape(radius);
		return btSphere;
	}
		break;

	case eShapeType::Cylinder:
	{
		CCylinderShape* cylinder = dynamic_cast<CCylinderShape*>(shape);
		btVector3 halfExtents;
		CastBulletVector3(cylinder->GetHalfExtents(), &halfExtents);
		btCylinderShape* btCylinder = new btCylinderShape(halfExtents);
		return btCylinder;
	}
		break;
	}

	return nullptr;
}

static btRigidBody* CastBulletRigidBody(iRigidBody* body)
{
	return dynamic_cast<CRigidBody*>(body)->GetBulletBody();
}


NAMESPACE_END

#endif //_CONVERSION_H_