#include "pch.h"
#include "../Headers/PhysicsWorld.h"
#include "../Headers/iRigidBody.h"
#include "../Headers/RigidBody.h"
#include "../Headers/CollisionHandler.h"
#include "../Headers/iShape.h"
#include "../Headers/EngineFunction.h"
#include "../Headers/Conversion.h"

USING(Engine)
USING(std)
USING(glm)

CPhysicsWorld::CPhysicsWorld()
	: m_vGravity(vec3(0.f)), m_collisionCallback(nullptr)
{
	m_vecRigidBodies.clear();

	m_CollisionConfiguration = new btDefaultCollisionConfiguration();
	m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);
	m_Broadphase = new btDbvtBroadphase();
	m_Solver = new btSequentialImpulseConstraintSolver();
	m_DynamicWorld = new btDiscreteDynamicsWorld(m_Dispatcher, m_Broadphase, m_Solver, m_CollisionConfiguration);

	//m_DebugDrawer = new DebugDrawer(7);
	//m_DynamicWorld->setDebugDrawer(m_DebugDrawer);
	//m_DynamicWorld->getDebugDrawer()->setDebugMode(1);
	
	m_OverlappingPairCallback = new OverlappingPairCallback();
	m_DynamicWorld->getPairCache()->setInternalGhostPairCallback(m_OverlappingPairCallback);
}

CPhysicsWorld::~CPhysicsWorld()
{
}

void CPhysicsWorld::Destroy()
{
	for (int i = m_DynamicWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = m_DynamicWorld->getCollisionObjectArray()[i];
		m_DynamicWorld->removeCollisionObject(obj);
		delete obj;
	}

	for (int i = 0; i < m_vecRigidBodies.size(); ++i)
		SafeDestroy(m_vecRigidBodies[i]);

	SafeDestroy(m_pColHandler);

	delete m_DynamicWorld;
	delete m_CollisionConfiguration;
	delete m_Dispatcher;
	delete m_Broadphase;
	delete m_Solver;
	delete m_OverlappingPairCallback;
}

void CPhysicsWorld::Update(const _float& dt)
{
	m_DynamicWorld->stepSimulation(dt);

	//for (int i = 0; i < m_vecRigidBodies.size(); ++i)
	//	m_vecRigidBodies[i]->Update(dt);

	//for (int i = 0; i < m_vecRigidBodies.size(); ++i)
	//{
	//	m_vecRigidBodies[i]->SetGravityAcceleration(m_vGravity);
	//	m_vecRigidBodies[i]->UpdateAcceleration();
	//}

	//for (int i = 0; i < m_vecRigidBodies.size(); ++i)
	//{
	//	//m_vecRigidBodies[i]->VerletStep3(dt);
	//	m_vecRigidBodies[i]->ApplyDamping(dt / 2.f);
	//}

	//for (int i = 0; i < m_vecRigidBodies.size(); ++i)
	//	m_vecRigidBodies[i]->VerletStep1(dt);

	//// Collision
	//vector<CCollisionHandler::sColPair> vecPairs;
	//m_pColHandler->Collide(dt, m_vecRigidBodies, vecPairs);

	//for (int i = 0; i < vecPairs.size(); ++i)
	//{
	//	CCollisionHandler::sColPair pair = vecPairs[i];
	//	if (eShapeType::Plane == pair.pBodyA->GetShape()->GetShapeType() ||
	//		eShapeType::Plane == pair.pBodyB->GetShape()->GetShapeType())
	//		continue;

	//	if (nullptr != m_collisionCallback)
	//		m_collisionCallback();
	//}

	//for (int i = 0; i < m_vecRigidBodies.size(); ++i)
	//{
	//	m_vecRigidBodies[i]->VerletStep2(dt);
	//	m_vecRigidBodies[i]->ApplyDamping(dt / 2.f);
	//	m_vecRigidBodies[i]->KillForces();
	//}
}

void CPhysicsWorld::SetGravity(const vec3& gravity)
{
	btVector3 btGravity;
	CastBulletVector3(gravity, &btGravity);
	m_vGravity = gravity;
}

void CPhysicsWorld::AddBody(iRigidBody* body)
{
	btRigidBody* bulletBody = CastBulletRigidBody(body);
	m_DynamicWorld->addRigidBody(bulletBody);

	//if (nullptr == body)
	//	return;

	CRigidBody* rigidBody = dynamic_cast<CRigidBody*>(body);

	for (int i = 0; i < m_vecRigidBodies.size(); ++i)
	{
		if (rigidBody == m_vecRigidBodies[i])
			return;
	}

	m_vecRigidBodies.push_back(rigidBody);
}

void CPhysicsWorld::RemoveBody(iRigidBody* body)
{
	btRigidBody* bulletBody = CastBulletRigidBody(body);
	m_DynamicWorld->removeRigidBody(bulletBody);
	delete bulletBody;

	CRigidBody* rigidBody = dynamic_cast<CRigidBody*>(body);
	vector<CRigidBody*>::iterator iter;
	for (iter = m_vecRigidBodies.begin(); iter != m_vecRigidBodies.end(); ++iter)
	{
		if (rigidBody == (*iter))
		{
			SafeDestroy(*iter);
			m_vecRigidBodies.erase(iter);
			return;
		}
	}
}

void CPhysicsWorld::ResetAllRigidBodies()
{
	for (int i = 0; i < m_vecRigidBodies.size(); ++i)
	{
		m_vecRigidBodies[i]->ResetAll();
	}
}

void CPhysicsWorld::ApplyRandomForce()
{
	for (int i = 0; i < m_vecRigidBodies.size(); ++i)
	{
		_float randX = GetRandNum(-500, 500);
		_float randZ = GetRandNum(-500, 500);
		_float mass = m_vecRigidBodies[i]->GetMass();
		m_vecRigidBodies[i]->ApplyForce(vec3(randX * mass, 0.f, randZ * mass));
	}
}

void CPhysicsWorld::ApplyDirForce(const vec3& linearDir, const vec3& angularDir)
{
	for (int i = 0; i < m_vecRigidBodies.size(); ++i)
	{
		_float mass = m_vecRigidBodies[i]->GetMass();

		if (0.f == mass)
			continue;
		
		vec3 vPos = m_vecRigidBodies[i]->GetPosition();
		vPos.y = 50.f;
		m_vecRigidBodies[i]->SetPosition(vPos);
		m_vecRigidBodies[i]->ApplyImpulse(linearDir);

		m_vecRigidBodies[i]->ResetQ();
	}
}

RESULT CPhysicsWorld::Ready(function<void(void)> callback)
{
	m_pColHandler = CCollisionHandler::Create();

	m_collisionCallback = callback;

	return PK_NOERROR;
}

CPhysicsWorld* CPhysicsWorld::Create(function<void(void)> callback)
{
	CPhysicsWorld* pInstance = new CPhysicsWorld();
	if (PK_NOERROR != pInstance->Ready(callback))
	{
		pInstance->Destroy();
		pInstance = nullptr;
	}

	return pInstance;
}
