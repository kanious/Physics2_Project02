#include "pch.h"
#include "../Headers/RigidBody.h"
#include "../Headers/RigidBodyDesc.h"
#include "../Headers/iShape.h"
#include "../Headers/Transform.h"
#include "../Headers/OpenGLDefines.h"
#include "../Headers/Conversion.h"

USING(Engine)
USING(std)
USING(glm)

CRigidBody::CRigidBody()
	: m_pDesc(nullptr)
{
}

CRigidBody::~CRigidBody()
{
}

void CRigidBody::Destroy()
{
	if (nullptr != m_pDesc)
		delete m_pDesc;

	if (nullptr != m_pShape)
		SafeDestroy(m_pShape);

	if (nullptr != m_pBulletShape)
		delete m_pBulletShape;

	if (nullptr != m_pMotionState)
		delete m_pMotionState;
}

void CRigidBody::Update(const _float& dt)
{
}

void CRigidBody::SetGravityAcceleration(const vec3& gravity)
{
	m_vGravity = gravity;
}

void CRigidBody::UpdateAcceleration()
{
	if (m_bIsStatic)
		return;

	m_vLinearAcceleration = m_vForce * m_fInvMass + m_vGravity;
	m_vAngularAcceleration = m_vTorque * m_fInvMass * m_fInvMass;
}

void CRigidBody::VerletStep1(const _float& dt)
{
	if (m_bIsStatic)
		return;

	m_vPreviousPosition = m_vPosition;
	m_vPosition += (m_vLinearVelocity + m_vLinearAcceleration * (dt/* * 0.5f*/)) * dt;

	if (-10.f > m_vPosition.y)
		m_vPosition.y = 5.f;

	vec3 axis = m_vAngularVelocity + m_vAngularAcceleration * dt;
	_float angle = length(axis);
	axis = normalize(axis);
	if (angle != 0.f)
	{
		quat rot = angleAxis(angle, axis);
		m_qRotation *= rot;
	}
}

void CRigidBody::VerletStep2(const _float& dt)
{
	if (m_bIsStatic) 
		return;

	m_vLinearVelocity += m_vLinearAcceleration * (dt * 0.5f);
	m_vAngularVelocity += m_vAngularAcceleration * (dt * 0.5f);
}

void CRigidBody::VerletStep3(const _float& dt)
{
	VerletStep2(dt);
}

void CRigidBody::KillForces()
{
	m_vForce = vec3(0.f);
	m_vTorque = vec3(0.f);
}

void CRigidBody::ApplyDamping(_float dt)
{
	m_vLinearVelocity *= pow(1.f - m_fLinearDamping, dt);
	m_vAngularVelocity *= m_fAngularDamping;// pow(1.f - m_fAngularDamping, dt);

	if (0.01f > length(m_vLinearVelocity))
		m_vLinearVelocity = vec3(0.f);
	if (0.01f > length(m_vAngularVelocity))
		m_vAngularVelocity = vec3(0.f);
}

void CRigidBody::ResetQ()
{
	m_qRotation = quat(1.f, 0.f, 0.f, 0.f);
}

vec3 CRigidBody::GetPosition()
{
	//return m_vPosition;
	vec3 pos;
	CastGLMVec3(m_bulletBody->getCenterOfMassPosition(), &pos);
	return pos;
}

void CRigidBody::SetPosition(const vec3& position)
{
	//m_vPosition = position;
}

quat CRigidBody::GetRotation()
{
	//return m_qRotation;
	quat rot;
	CastGLMQuat(m_bulletBody->getOrientation(), &rot);
	return rot;
}

void CRigidBody::SetRotation(const quat& rotation)
{
	//m_qRotation = rotation;
}

void CRigidBody::ApplyForce(const vec3& force)
{
	//m_vForce += force;
	btVector3 btForce;
	CastBulletVector3(force, &btForce);
	m_bulletBody->applyCentralForce(btForce);
	//m_bulletBody->activate(true);
}

void CRigidBody::ApplyForceAtPoint(const vec3& force, const vec3& relativePoint)
{
	//ApplyForce(force);
	//ApplyTorque(cross(relativePoint, force));
	btVector3 btForce;
	btVector3 btForceAtPoint;

	CastBulletVector3(force, &btForce);
	CastBulletVector3(relativePoint, &btForceAtPoint);

	m_bulletBody->applyForce(btForce, btForceAtPoint);
}

void CRigidBody::ApplyImpulse(const vec3& impulse)
{
	//m_vLinearVelocity += impulse * m_fInvMass * m_fInvMass;
	btVector3 btImpluse;
	CastBulletVector3(impulse, &btImpluse);
	m_bulletBody->applyCentralImpulse(btImpluse);
}

void CRigidBody::ApplyImpulseAtPoint(const vec3& impulse, const vec3& relativePoint)
{
	//ApplyTorqueImpulse(cross(relativePoint, impulse));
	btVector3 btImpulse;
	btVector3 btImpulseAtPoint;

	CastBulletVector3(impulse, &btImpulse);
	CastBulletVector3(relativePoint, &btImpulseAtPoint);

	m_bulletBody->applyImpulse(btImpulse, btImpulseAtPoint);
}

void CRigidBody::ApplyTorque(const vec3& torque)
{
	//m_vTorque += torque;
	btVector3 btTorque;
	CastBulletVector3(torque, &btTorque);
	m_bulletBody->applyTorque(btTorque);
}

void CRigidBody::ApplyTorqueImpulse(const glm::vec3& torqueImpulse)
{
	//m_vAngularVelocity += torqueImpulse;
	btVector3 btTorqueImpulse;
	CastBulletVector3(torqueImpulse, &btTorqueImpulse);
	m_bulletBody->applyTorqueImpulse(btTorqueImpulse);
}

_float CRigidBody::GetMass()
{
	_float mass = m_bulletBody->getMass();
	return mass;
}

void CRigidBody::ResetAll()
{
	SetRigidBodyDesc(*m_pDesc);
}

RESULT CRigidBody::Ready(const CRigidBodyDesc& desc, iShape* shape)
{
	m_pDesc = new CRigidBodyDesc(desc);
	m_pShape = shape;

	SetRigidBodyDesc(desc);

	return PK_NOERROR;
}

void CRigidBody::SetRigidBodyDesc(const CRigidBodyDesc& desc)
{
	/*if (nullptr != m_bulletBody)
	{
		delete m_bulletBody;
		m_bulletBody = nullptr;
	}*/

	btQuaternion rotation;
	btVector3 position;

	CastBulletQuaternion(desc.rotation, &rotation);
	CastBulletVector3(desc.position, &position);

	if (nullptr == m_bulletBody)
	{
		m_pMotionState = new btDefaultMotionState(btTransform(rotation, position));
		btVector3 inertia(0, 0, 0);
		m_pBulletShape = CastBulletShape(m_pShape);
		if (0.f != desc.mass)
			m_pBulletShape->calculateLocalInertia(desc.mass, inertia);
		btRigidBody::btRigidBodyConstructionInfo bodyCI(desc.mass, m_pMotionState, m_pBulletShape, inertia);
		m_bulletBody = new btRigidBody(bodyCI);
	}
	else
	{
		btTransform transform;
		transform.setOrigin(position);
		transform.setRotation(rotation);
		m_bulletBody->setWorldTransform(transform);
		m_bulletBody->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
		m_bulletBody->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
		m_bulletBody->clearForces();

		btMotionState* state = m_bulletBody->getMotionState();
		state->setWorldTransform(transform);
	}

	if (desc.isStatic)
		m_bulletBody->setActivationState(WANTS_DEACTIVATION);
	else
		m_bulletBody->setActivationState(DISABLE_DEACTIVATION);
	
	btVector3 btAngularFactor;
	btVector3 btLinearFactor;

	CastBulletVector3(desc.angularFactor, &btAngularFactor);
	CastBulletVector3(desc.linearFactor, &btLinearFactor);

	m_bulletBody->setAngularFactor(btAngularFactor);
	m_bulletBody->setLinearFactor(btLinearFactor);


	/*m_bIsStatic = desc.isStatic;
	m_bIsGround = desc.isGround;

	if (m_bIsStatic || desc.mass <= 0.f)
	{
		m_fMass = 0.f;
		m_fInvMass = 0.f;
		m_bIsStatic = true;
	}
	else
	{
		m_fMass = desc.mass;
		m_fInvMass = 1.f / m_fMass;
	}

	m_fRestitution = desc.restitution;
	m_fFriction = desc.friction;
	m_fLinearDamping = desc.linearDamping;
	m_fAngularDamping = desc.angularDamping;

	m_vPosition = desc.position;
	m_vLinearVelocity = desc.linearVelocity;
	m_vLinearFactor = desc.linearFactor;
	m_vAngularVelocity = desc.angularVelocity;
	m_vAngularFactor = desc.angularFactor;

	m_qRotation = desc.rotation;*/
}

CRigidBody* CRigidBody::Create(const CRigidBodyDesc& desc, iShape* shape)
{
	CRigidBody* pInstance = new CRigidBody();
	if (PK_NOERROR != pInstance->Ready(desc, shape))
	{
		pInstance->Destroy();
		pInstance = nullptr;
	}

	return pInstance;
}
