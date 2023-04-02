#include "SceneDungeon.h"
#include "Function.h"
#include "glm\vec3.hpp"
#include "InputDevice.h"
#include "DefaultCamera.h"
#include "JsonParser.h"
#include "Layer.h"
#include "ComponentMaster.h"
#include "Component.h"
#include "Shader.h"
#include "LightMaster.h"
#include "BGObject.h"
#include "Define.h"
#include "UIManager.h"
#include "Enums.h"
#include "SkyBox.h"
#include "Renderer.h"
#include "ObjectFactory.h"
#include "SoundMaster.h"

#include "PhysicsDefines.h"

#include <functional>
#include <sstream>
#include <atlconv.h>
 

USING(Engine)
USING(glm)
USING(std)

SceneDungeon::SceneDungeon()
	: m_pSkyBox(nullptr)
	, m_pDefaultCamera(nullptr), m_vCameraSavedPos(vec3(0.f)), m_vCameraSavedRot(vec3(0.f)), m_vCameraSavedTarget(vec3(0.f))
	, m_pCharacterLayer(nullptr), m_pPFactory(nullptr), m_pPWorld(nullptr), m_iTargetIndex(0)
{
	m_pInputDevice = CInputDevice::GetInstance(); m_pInputDevice->AddRefCnt();
	m_pUIManager = UIManager::GetInstance(); m_pUIManager->AddRefCnt();
	m_vecTargets.clear();
	m_vecSpheres.clear();

	m_ObjListFileName = "physicsMapObjects.json";
	m_LightListFileName = "lights.xml";
}

SceneDungeon::~SceneDungeon()
{
}

// Call instead of destructor to manage class internal data
void SceneDungeon::Destroy()
{
	SafeDestroy(m_pInputDevice);
	SafeDestroy(m_pUIManager);
	SafeDestroy(m_pPFactory);
	SafeDestroy(m_pPWorld);

	SafeDestroy(m_pSkyBox);

	m_vecTargets.clear();

	CScene::Destroy();
}

// Basic Update Function
void SceneDungeon::Update(const _float& dt)
{
	if (nullptr != m_pSkyBox)
		CRenderer::GetInstance()->AddRenderObj(m_pSkyBox);

	if (nullptr != m_pPWorld)
		m_pPWorld->Update(dt);

	KeyCheck();

	CLightMaster::GetInstance()->SetLightInfo();

	CScene::Update(dt);
}

// Basic Render Function
void SceneDungeon::Render()
{
	if (nullptr != m_pUIManager)
		m_pUIManager->RenderUI();
}

// Return current camera position
vec3 SceneDungeon::GetCameraPos()
{
	if (nullptr != m_pDefaultCamera)
		return m_pDefaultCamera->GetCameraEye();

	return vec3(0.f);
}

void SceneDungeon::CollisionCallback()
{
	CSoundMaster::GetInstance()->PlaySound("Ball");
}

string SceneDungeon::GetCurrentTargetName()
{
	if (0 == m_iTargetIndex)
		return "NULL";

	BGObject* target = m_vecTargets[m_iTargetIndex];
	return target->GetMeshName();
}

// Check User input
void SceneDungeon::KeyCheck()
{
	static _bool isF4Down = false;
	if (m_pInputDevice->IsKeyDown(GLFW_KEY_F4))
	{
		if (!isF4Down)
		{
			isF4Down = true;
			CLayer* bgLayer = GetLayer((_uint)LAYER_BACKGROUND_OBJECT);
			_float randX = GetRandNum(700, 800) * 0.01f;
			_float randY = GetRandNum(400, 600) * 0.01f;
			_float randZ = GetRandNum(-700, -800) * 0.01f;
			_float randSize = GetRandNum(80, 120) * 0.01f;
			_int randNum = GetRandNum(0, 7);
			string meshId = GetMeshIdByNumber(randNum);
			BGObject* newObj = AddNewObject(bgLayer, meshId, vec3(randX, randY, randZ), vec3(0.f), vec3(randSize), false, eShapeType::Sphere);
			m_vecSpheres.push_back(newObj);
		}
	}
	else
		isF4Down = false;

	static _bool isF3Down = false;
	if (m_pInputDevice->IsKeyDown(GLFW_KEY_F3))
	{
		if (!isF3Down)
		{
			isF3Down = true;

			if (nullptr != m_pPWorld)
				m_pPWorld->ApplyRandomForce();
		}
	}
	else
		isF3Down = false;

	static _bool isF2Down = false;
	if (m_pInputDevice->IsKeyDown(GLFW_KEY_F2))
	{
		if (!isF2Down)
		{
			isF2Down = true;

			if (nullptr != m_pPWorld)
			{
				m_pPWorld->ResetAllRigidBodies();

				for (int i = 0; i < m_vecSpheres.size(); ++i)
				{
					BGObject* pObj = m_vecSpheres[i];
					iRigidBody* body = pObj->GetRigidBody();
					m_pPWorld->RemoveBody(body);
					pObj->SetDead(true);
				}
				m_vecSpheres.clear();
			}
		}
	}
	else
		isF2Down = false;

	static _bool isF1Down = false;
	if (m_pInputDevice->IsKeyDown(GLFW_KEY_F1))
	{
		if (!isF1Down)
		{
			isF1Down = true;

			m_iTargetIndex = 0;
			m_pDefaultCamera->SetTargetObject(m_vecTargets[m_iTargetIndex]);
		}
	}
	else
		isF1Down = false;

	static _bool isTabDown = false;
	if (m_pInputDevice->IsKeyDown(GLFW_KEY_TAB))
	{
		if (!isTabDown)
		{
			isTabDown = true;
			
			++m_iTargetIndex;
			if (m_iTargetIndex >= m_vecTargets.size())
				m_iTargetIndex = 0;
			m_pDefaultCamera->SetTargetObject(m_vecTargets[m_iTargetIndex]);
		}
	}
	else
		isTabDown = false;
}

// Saves camera position
void SceneDungeon::SetDefaultCameraSavedPosition(vec3 vPos, vec3 vRot, vec3 target)
{
	m_vCameraSavedPos.x = vPos.x;
	m_vCameraSavedPos.y = vPos.y;
	m_vCameraSavedPos.z = vPos.z;

	m_vCameraSavedRot.x = vRot.x;
	m_vCameraSavedRot.y = vRot.y;
	m_vCameraSavedRot.z = vRot.z;

	m_vCameraSavedTarget.x = target.x;
	m_vCameraSavedTarget.y = target.y;
	m_vCameraSavedTarget.z = target.z;
}

// Reset camera position
void SceneDungeon::ResetDefaultCameraPos()
{
	if (nullptr != m_pDefaultCamera)
	{
		m_pDefaultCamera->SetCameraEye(m_vCameraSavedPos);
		m_pDefaultCamera->SetCameraRot(m_vCameraSavedRot);
		m_pDefaultCamera->SetCameraTarget(m_vCameraSavedTarget);
	}
}

// Initialize
RESULT SceneDungeon::Ready(string dataPath)
{
	m_DataPath = dataPath;

	// Physics
	m_pPFactory = CPhysicsFactory::Create();
	m_pPWorld = m_pPFactory->CreateWorld(bind(&SceneDungeon::CollisionCallback, this));
	if (nullptr != m_pPWorld)
		m_pPWorld->SetGravity(vec3(0.f, -9.81f * 10.f, 0.f));

	// Sound
	CSoundMaster::GetInstance()->SetVolume("Ball", 2.f);

	// GameObjects
	RESULT result = PK_NOERROR;
	result = ReadyLayerAndGameObject();
	if (PK_NOERROR != result)
		return result;

	// Light
	CComponent* shader = CComponentMaster::GetInstance()->FindComponent("DefaultShader");
	_uint shaderID = 0;
	if (nullptr != shader)
		shaderID = dynamic_cast<CShader*>(shader)->GetShaderProgram();
	CLightMaster::GetInstance()->SetShader(shaderID);
	CLightMaster::GetInstance()->LoadLights(m_DataPath, m_LightListFileName);

	// Set Camera info to Shader
	if (nullptr != m_pDefaultCamera)
		m_pDefaultCamera->AddShaderLocation("DefaultShader");

	// UI
	if (nullptr != m_pUIManager)
		m_pUIManager->Ready(this);

	if (nullptr == m_pSkyBox)
	{
		stringstream ss, ss2;
		ss << m_DataPath << "Texture\\SkyBox\\";

		vector<string> faces;
		ss2.str(""); ss2 << ss.str() << "right.jpg"; faces.push_back(ss2.str());
		ss2.str(""); ss2 << ss.str() << "left.jpg"; faces.push_back(ss2.str());
		ss2.str(""); ss2 << ss.str() << "top.jpg"; faces.push_back(ss2.str());
		ss2.str(""); ss2 << ss.str() << "bottom.jpg"; faces.push_back(ss2.str());
		ss2.str(""); ss2 << ss.str() << "front.jpg"; faces.push_back(ss2.str());
		ss2.str(""); ss2 << ss.str() << "back.jpg"; faces.push_back(ss2.str());

		CComponent* skyboxShader = CComponentMaster::GetInstance()->FindComponent("SkyBoxShader");
		m_pSkyBox = CSkyBox::Create(faces, dynamic_cast<CShader*>(skyboxShader));
	}
	return PK_NOERROR;
}

// Initialize GameObjects
RESULT SceneDungeon::ReadyLayerAndGameObject()
{
	//Create.Camera
	CLayer* pLayer = GetLayer((_uint)LAYER_INTERACTIVE_OBJECT);
	if (nullptr != pLayer)
	{
		vec3 vPos = vec3(0.f);
		vec3 vRot = vec3(0.f);
		vec3 vScale = vec3(1.f);

		m_pDefaultCamera = ObjectFactory::CreateCamera(
			(_uint)SCENE_3D, pLayer->GetTag(),
			(_uint)OBJ_CAMERA, pLayer,
			vPos, vRot, vScale, 0.6f, 0.1f, 1000.f);
	}

	//Create.BackgroundLayer 
	LoadObjects();

	return PK_NOERROR;
}

BGObject* SceneDungeon::AddNewObject(CLayer* pLayer, string meshId, vec3 vPos, vec3 vRot, vec3 vScale, _bool isStatic, eShapeType shapeType)
{
	BGObject* newObject = ObjectFactory::CreateBGObject(
		(_uint)SCENE_3D, pLayer->GetTag(), (_uint)OBJ_BACKGROUND, pLayer, meshId, vPos, vRot, vScale);

	CRigidBodyDesc newDesc;
	newDesc.isStatic = isStatic;
	if (isStatic)
		newDesc.mass = 0.f;
	else
	{
		if (eShapeType::Cylinder == shapeType)
			newDesc.mass = vScale.x * 0.3f;
		else
			newDesc.mass = vScale.x;
	}
	newDesc.position = vPos;
	quat qRot = quat(vec3(0.f));
	newDesc.rotation = qRot;
	newDesc.linearVelocity = vec3(0.f);
	iShape* shape = nullptr;
	if (eShapeType::Sphere == shapeType)
		shape = CSphereShape::Create(shapeType, vScale.x);
	else if (eShapeType::Box == shapeType)
		shape = CBoxShape::Create(shapeType, vScale / 2.f);
	else if (eShapeType::Cylinder == shapeType)
		shape = CCylinderShape::Create(shapeType, vec3(vScale.x / 4.f, vScale.y * 0.8f, vScale.z / 4.f));
	iRigidBody* rigidBody = m_pPFactory->CreateRigidBody(newDesc, shape);
	newObject->SetRigidBody(rigidBody);
	m_pPWorld->AddBody(rigidBody);

	return newObject;
}

// Load Objects from json file
void SceneDungeon::LoadObjects()
{
	CLayer* bgLayer = GetLayer((_uint)LAYER_BACKGROUND_OBJECT);
	bgLayer->RemoveAllGameObject();

	m_vecTargets.push_back(nullptr);

	// Target Sphere
	BGObject* targetObj = AddNewObject(bgLayer, "earth", vec3(0.f, 5.f, 0.f), vec3(0.f), vec3(1.f), false, eShapeType::Sphere);
	m_vecTargets.push_back(targetObj);
	m_iTargetIndex = 1;
	m_pDefaultCamera->SetTargetObject(targetObj);

	// Static Floor box
	AddNewObject(bgLayer, "box", vec3(0.f, -16.f, 0.f), vec3(0.f), vec3(32.f), true, eShapeType::Box);

	// Left up (non-static boxes)
	//AddNewObject(bgLayer, "venus", vec3(8.f, 5.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Sphere);
	AddNewObject(bgLayer, "box", vec3(8.f, 1.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(9.f, 1.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(10.f, 1.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(7.f, 1.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(6.f, 1.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);

	AddNewObject(bgLayer, "box", vec3(8.f, 2.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(9.f, 2.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(10.f, 2.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(7.f, 2.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(6.f, 2.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);

	AddNewObject(bgLayer, "box", vec3(8.f, 3.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(9.f, 3.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(10.f, 3.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(7.f, 3.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(6.f, 3.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);

	AddNewObject(bgLayer, "box", vec3(8.f, 4.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(9.f, 4.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(10.f, 4.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(7.f, 4.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(6.f, 4.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Box);

	// Left down (Spawn Sphere)
	//AddNewObject(bgLayer, "jupiter", vec3(8.f, 5.f, -8.f), vec3(0.f), vec3(1.f), false, eShapeType::Sphere);

	// Right up (Static boxes)
	//AddNewObject(bgLayer, "neptune", vec3(-8.f, 5.f, 8.f), vec3(0.f), vec3(1.f), false, eShapeType::Sphere);
	AddNewObject(bgLayer, "box", vec3(-8.f, 0.5f, 8.f), vec3(0.f), vec3(0.7f), true, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(-8.f, 0.5f, 11.f), vec3(0.f), vec3(0.7f), true, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(-8.f, 0.5f, 5.f), vec3(0.f), vec3(0.7f), true, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(-11.f, 0.5f, 8.f), vec3(0.f), vec3(0.7f), true, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(-5.f, 0.5f, 8.f), vec3(0.f), vec3(0.7f), true, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(-5.f, 0.5f, 11.f), vec3(0.f), vec3(0.7f), true, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(-5.f, 0.5f, 5.f), vec3(0.f), vec3(0.7f), true, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(-11.f, 0.5f, 5.f), vec3(0.f), vec3(0.7f), true, eShapeType::Box);
	AddNewObject(bgLayer, "box", vec3(-11.f, 0.5f, 11.f), vec3(0.f), vec3(0.7f), true, eShapeType::Box);

	// Right down (Cylinders)
	//AddNewObject(bgLayer, "moon", vec3(-8.f, 5.f, -8.f), vec3(0.f), vec3(1.f), false, eShapeType::Sphere);
	AddNewObject(bgLayer, "pin", vec3(-8.f, 2.f, -6.f), vec3(0.f), vec3(2.f), false, eShapeType::Cylinder);
	AddNewObject(bgLayer, "pin", vec3(-7.5f, 2.f, -7.f), vec3(0.f), vec3(2.f), false, eShapeType::Cylinder);
	AddNewObject(bgLayer, "pin", vec3(-8.5f, 2.f, -7.f), vec3(0.f), vec3(2.f), false, eShapeType::Cylinder);
	AddNewObject(bgLayer, "pin", vec3(-7.f, 2.f, -8.f), vec3(0.f), vec3(2.f), false, eShapeType::Cylinder);
	AddNewObject(bgLayer, "pin", vec3(-8.f, 2.f, -8.f), vec3(0.f), vec3(2.f), false, eShapeType::Cylinder);
	AddNewObject(bgLayer, "pin", vec3(-9.f, 2.f, -8.f), vec3(0.f), vec3(2.f), false, eShapeType::Cylinder);

	AddNewObject(bgLayer, "pin", vec3(-6.5f, 2.f, -9.f), vec3(0.f), vec3(2.f), false, eShapeType::Cylinder);
	AddNewObject(bgLayer, "pin", vec3(-7.5f, 2.f, -9.f), vec3(0.f), vec3(2.f), false, eShapeType::Cylinder);
	AddNewObject(bgLayer, "pin", vec3(-8.5f, 2.f, -9.f), vec3(0.f), vec3(2.f), false, eShapeType::Cylinder);
	AddNewObject(bgLayer, "pin", vec3(-9.5f, 2.f, -9.f), vec3(0.f), vec3(2.f), false, eShapeType::Cylinder);
}

string SceneDungeon::GetMeshIdByNumber(_int num)
{
	string meshId = "";

	switch (num)
	{
	case 0:		meshId = "moon";	break;
	case 1:		meshId = "jupiter";	break;
	case 2:		meshId = "mars";	break;
	case 3:		meshId = "mercury";	break;
	case 4:		meshId = "neptune";	break;
	case 5:		meshId = "pluto";	break;
	case 6:		meshId = "uranus";	break;
	case 7:		meshId = "venus";	break;
	}

	return meshId;
}

// Create an instance
SceneDungeon* SceneDungeon::Create(string dataPath)
{
	SceneDungeon* pInstance = new SceneDungeon();
	if (PK_NOERROR != pInstance->Ready(dataPath))
	{
		pInstance->Destroy();
		pInstance = nullptr;
	}

	return pInstance;
}
