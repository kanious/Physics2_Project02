#ifndef _BGOBJECT_H_
#define _BGOBJECT_H_

#include "GameObject.h"
#include "EngineStruct.h"

namespace Engine
{
	class CMesh;
	class iRigidBody;
	class CInputDevice;
}

// Static Object class
class BGObject : public Engine::CGameObject
{
private:
	Engine::CMesh*					m_pMesh;
	Engine::CInputDevice*			m_pInputDevice;
	Engine::iRigidBody*				m_pRigidBody;


private:
	explicit BGObject();
	virtual ~BGObject();

public:
	Engine::iRigidBody* GetRigidBody()	{ return m_pRigidBody; }
	void SetRigidBody(Engine::iRigidBody* pBody);
	void SetTransperancy();
	void AddForceToRigidBody(glm::vec3 vPos);
	virtual glm::vec3 GetPosition();
private:
	void KeyCheck(const _float& dt);

public:
	virtual void Update(const _float& dt);
	virtual void Render();
private:
	virtual void Destroy();
	RESULT Ready(_uint sTag, _uint lTag, _uint oTag, Engine::CLayer* pLayer, std::string meshID,
		glm::vec3 vPos, glm::vec3 vRot, glm::vec3 vScale);
public:
	static BGObject* Create(_uint sTag, _uint lTag, _uint oTag, Engine::CLayer* pLayer, std::string meshID,
		glm::vec3 vPos, glm::vec3 vRot, glm::vec3 vScale);
};

#endif //_BGOBJECT_H_