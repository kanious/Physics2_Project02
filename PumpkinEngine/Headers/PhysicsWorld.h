#ifndef _PHYSICSWORLD_H_
#define _PHYSICSWORLD_H_

#include "btBulletDynamicsCommon.h"

#include <functional>
#include "iPhysicsWorld.h"
#include "CollisionHandler.h"

NAMESPACE_BEGIN(Engine)

class CRigidBody;
class CCollisionHandler;
class ENGINE_API CPhysicsWorld : public iPhysicsWorld
{
	class OverlappingPairCallback : public btOverlappingPairCallback
	{
	public:
		virtual btBroadphasePair* addOverlappingPair(
			btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) override
		{
			return nullptr;
		}

		virtual void* removeOverlappingPair(btBroadphaseProxy* proxy0,
			btBroadphaseProxy* proxy1, btDispatcher* dispatcher) override
		{
			return nullptr;
		}

		virtual void removeOverlappingPairsContainingProxy(
			btBroadphaseProxy* proxy0, btDispatcher* dispatcher) override
		{
		}
	};

private:
	glm::vec3								m_vGravity;
	std::vector<CRigidBody*>				m_vecRigidBodies;
	CCollisionHandler*						m_pColHandler;

	std::function<void(void)>				m_collisionCallback;

	// Bullet variables
	btDefaultCollisionConfiguration*		m_CollisionConfiguration;
	btCollisionDispatcher*					m_Dispatcher;
	btBroadphaseInterface*					m_Broadphase;
	btSequentialImpulseConstraintSolver*	m_Solver;
	btDiscreteDynamicsWorld*				m_DynamicWorld;
	OverlappingPairCallback*				m_OverlappingPairCallback;

	btIDebugDraw*							m_DebugDrawer;

private:
	explicit CPhysicsWorld();
	virtual ~CPhysicsWorld();
	virtual void Destroy();

public:
	virtual void Update(const _float& dt);

public:
	virtual void SetGravity(const glm::vec3& gravity);
	virtual void AddBody(iRigidBody* body);
	virtual void RemoveBody(iRigidBody* body);
	virtual void ResetAllRigidBodies();
	virtual void ApplyRandomForce();
	virtual void ApplyDirForce(const glm::vec3& linearDir, const glm::vec3& angularDir);

private:
	RESULT Ready(std::function<void(void)> callback);
public:
	static CPhysicsWorld* Create(std::function<void(void)> callback);
};

NAMESPACE_END

#endif //_PHYSICSWORLD_H_