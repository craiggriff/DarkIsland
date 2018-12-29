#include "pch.h"
#include "MeshModel.h"

using namespace Game;

MeshModel::MeshModel(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Physics* phys)
{
	m_deviceResources = pm_deviceResources;
	p_Phys = phys;

	OverallRadius = 0.0f;
	OverallMaxX = 0.0f;
	OverallMaxY = 0.0f;
	OverallMaxZ = 0.0f;
	OverallCenterX = 0.0f;
	OverallCenterY = 0.0f;
	OverallCenterZ = 0.0f;
	OverallMinX = 0.0f;
	OverallMinY = 0.0f;
	OverallMinZ = 0.0f;
}

// this is not used
void MeshModel::MakePhysicsMeshes(ObjInfo* ob_info)
{
	for (Mesh* m : m_mesh)
	{
		if (m->GetName()->compare(L"PHY_BOX") == 0 || m->GetName()->compare(L"PHY_BOX_R") == 0)
		{
			m->MakePhysicsBoxFromMesh(ob_info, 1.0f, p_Phys);
		}
		if (m->GetName()->compare(L"PHY_CYLINDER") == 0 || m->GetName()->compare(L"PHY_CYLINDER_R") == 0)
		{
			m->MakePhysicsCylinderFromMesh(ob_info, 1.0f, p_Phys);
		}
		if (m->GetName()->compare(L"PHY_CONVEXHULL") == 0 || m->GetName()->compare(L"PHY_CONVEXHULL_R") == 0)
		{
			m->MakePhysicsConvexHullFromMesh(ob_info, 1.0f, p_Phys);
		}
	}
}

task<void> MeshModel::LoadModel(
	AllResources* p_Res,
	const std::wstring& meshFilename,
	int phys_shape,
	float _scale)
{
	m_filename = L"Assets\\Compiled\\" + meshFilename;

	return Mesh::LoadFromFileAsync(
		*p_Res,
		m_filename,
		L"",
		L"",
		m_mesh,
		_scale).then([this, phys_shape]()
	{
		physics_shape = phys_shape;

		CalcOverallRadius();
	});;
}

void MeshModel::CalcOverallRadius()
{
	for (Mesh* m : m_mesh)
	{
		for (Mesh::MeshPoint p : m->Points())
		{
			if (p.point.x > OverallMaxX)
				OverallMaxX = p.point.x;
			if (p.point.y > OverallMaxY)
				OverallMaxY = p.point.y;
			if (p.point.z > OverallMaxZ)
				OverallMaxZ = p.point.z;

			if (p.point.x < OverallMinX)
				OverallMinX = p.point.x;
			if (p.point.y < OverallMinY)
				OverallMinY = p.point.y;
			if (p.point.z < OverallMinZ)
				OverallMinZ = p.point.z;
		}
	}

	OverallCenterX = ((OverallMaxX - OverallMinX) / 2.0f) + OverallMinX;
	OverallCenterY = ((OverallMaxY - OverallMinY) / 2.0f) + OverallMinY;
	OverallCenterZ = ((OverallMaxZ - OverallMinZ) / 2.0f) + OverallMinZ;

	OverallMaxX = 0.0f;
	OverallMaxY = 0.0f;
	OverallMaxZ = 0.0f;

	OverallMinX = 0.0f;
	OverallMinY = 0.0f;
	OverallMinZ = 0.0f;

	for (Mesh* m : m_mesh)
	{
		for (Mesh::MeshPoint p : m->Points())
		{
			const float dist1 = (p.point.x - OverallCenterX) * (p.point.x - OverallCenterX) +
				(p.point.y - OverallCenterY) * (p.point.y - OverallCenterY) +
				(p.point.z - OverallCenterZ) * (p.point.z - OverallCenterZ);
			float dist2 = 0.0f;

			XMStoreFloat(&dist2, XMVectorSqrt(XMLoadFloat(&dist1)));

			if (dist2 > OverallRadius)
				OverallRadius = dist2;

			if (p.point.x - OverallCenterX > OverallMaxX)
				OverallMaxX = p.point.x - OverallCenterX;
			if (p.point.y - OverallCenterY > OverallMaxY)
				OverallMaxY = p.point.y - OverallCenterY;
			if (p.point.z - OverallCenterZ > OverallMaxZ)
				OverallMaxZ = p.point.z - OverallCenterZ;

			if (p.point.x - OverallCenterX < OverallMinX)
				OverallMinX = p.point.x - OverallCenterX;
			if (p.point.y - OverallCenterY < OverallMinY)
				OverallMinY = p.point.y - OverallCenterY;
			if (p.point.z - OverallCenterZ < OverallMinZ)
				OverallMinZ = p.point.z - OverallCenterZ;
		}
	}

	if (physics_shape == PHY_CONVEXHULL)
	{
		ComputeOverallConvexHull();
	}

	for (Mesh* m : m_mesh)
	{
		if (m->GetName()->compare(L"PHY_CONVEXHULL") == 0 || m->GetName()->compare(L"PHY_CONVEXHULL_R") == 0)
		{
			m->ComputeConvexHull();
		}
	}
}

void MeshModel::ComputeOverallConvexHull()
{
	std::vector<float> coords;

	hull_comp = new btConvexHullComputer;

	float point = 0.0f;

	for (Mesh* m : m_mesh)
	{
		for (int i = 0; i < m->GetPoints().size(); i++)
		{
			coords.push_back(m->GetPoints()[i].point.x - OverallCenterX);
			coords.push_back(m->GetPoints()[i].point.y - OverallCenterY);
			coords.push_back(m->GetPoints()[i].point.z - OverallCenterZ);
		}
	}

	hull_comp->compute((float *)coords.data(), sizeof(float) * 3, coords.size() / 3, 0.0f, 0.0f);
}

btRigidBody*  MeshModel::MakePhysicsConvexHullFromMeshModel(ObjInfo* ob_info, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	auto fallShape = new btConvexHullShape();

	btVector3 vec;
	for (int i = 0; i < hull_comp->vertices.size(); i++)
	{
		vec.setX(hull_comp->vertices[i].getX());
		vec.setY(hull_comp->vertices[i].getY());
		vec.setZ(hull_comp->vertices[i].getZ());

		fallShape->addPoint(vec);
	}

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x + OverallCenterX, ob_info->pos.y + OverallCenterY, ob_info->pos.z + OverallCenterZ)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshModel::MakePhysicsCompoundBoxFromMeshModel(ObjInfo* ob_info, float x, float y, float z, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = OverallMaxX * _scale;
	extent2 = OverallMaxY * _scale;
	extent3 = OverallMaxZ * _scale;

	extent2 = abs(extent2) *0.6f;

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x + OverallCenterX, ob_info->pos.y + OverallCenterY, ob_info->pos.z + OverallCenterZ)));

	btCollisionShape* boxShape = new btBoxShape(btVector3(extent1, extent2, extent3));
	btCompoundShape* compound = new btCompoundShape();
	btTransform localTrans;
	localTrans.setIdentity();
	localTrans.setOrigin(btVector3(x, y, z));
	compound->addChildShape(localTrans, boxShape);

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	compound->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(compound, fallMotionState, fallInertia, ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshModel::MakePhysicsBoxFromMeshModel(ObjInfo* ob_info, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = OverallMaxX * _scale;
	extent2 = OverallMaxY * _scale;
	extent3 = OverallMaxZ * _scale;

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x + OverallCenterX, ob_info->pos.y + OverallCenterY, ob_info->pos.z + OverallCenterZ)));

	auto fallShape = new btBoxShape(btBoxShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshModel::MakePhysicsCylinderFromMeshModel(ObjInfo* ob_info, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = OverallMaxX * _scale;
	extent2 = OverallMaxY * _scale;
	extent3 = OverallMaxZ * _scale;

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x + OverallCenterX, ob_info->pos.y + OverallCenterY, ob_info->pos.z + OverallCenterZ)));

	auto fallShape = new btCylinderShape(btCylinderShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshModel::MakePhysicsSphereFromMeshModel(ObjInfo* ob_info, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = OverallMaxX * _scale;
	extent2 = OverallMaxY * _scale;
	extent3 = OverallMaxZ * _scale;

	if (extent2 > extent1)
		extent1 = extent2;

	if (extent3 > extent1)
		extent1 = extent3;

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x + OverallCenterX, ob_info->pos.y + OverallCenterY, ob_info->pos.z + OverallCenterZ)));

	auto fallShape = new btSphereShape(btSphereShape(extent1));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshModel::MakePhysicsEllipseoidFromMeshModel(ObjInfo* ob_info, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = OverallMaxX * _scale;
	extent2 = OverallMaxY * _scale;
	extent3 = OverallMaxZ * _scale;

	float uniform_value = 1.0f;

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x + OverallCenterX, ob_info->pos.y + OverallCenterY, ob_info->pos.z + OverallCenterZ)));

	auto fallShape = new btMultiSphereShape(&btVector3(0.0f, 0.0f, 0.0f), &uniform_value, 1);

	fallShape->setLocalScaling(btVector3(extent1, extent2, extent3));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

ID3D11ShaderResourceView** MeshModel::GetMaterialTexture(std::wstring _name, int tex_slot, int _alpha_mode)
{
	bool bFound = false;

	if (_name == L"matspoiler")
	{
		//exit(0);
		bFound = false;
	}

	for (Mesh* m : m_mesh)
	{
		for (int i = 0; i < m->GetMaterials().size(); i++)
		{
			if (!wcscmp(_name.c_str(), m->GetMaterial(i)->Name.c_str()))
			{
				if (_name == L"matspoiler")
				{
					//exit(0);
					//bFound = false;
				}
				m->alpha_mode = _alpha_mode;
				return m->GetMaterial(i)->Textures[tex_slot].GetAddressOf();
			}
		}
	}

	return nullptr;
}

ID3D11ShaderResourceView** MeshModel::GetMaterialNormal(std::wstring _name, float _intensity)
{
	bool bFound = false;
	for (Mesh* m : m_mesh)
	{
		for (int i = 0; i < m->GetMaterials().size(); i++)
		{
			if (!wcscmp(_name.c_str(), m->GetMaterial(i)->Name.c_str()))
			{
				m->GetMaterial(i)->normal_height = _intensity;
				return m->GetMaterial(i)->Normal.GetAddressOf();
			}
		}
	}

	return nullptr;
}

ID3D11ShaderResourceView** MeshModel::GetMaterialEmmit(std::wstring _name, float _intensity)
{
	bool bFound = false;
	for (Mesh* m : m_mesh)
	{
		for (int i = 0; i < m->GetMaterials().size(); i++)
		{
			if (!wcscmp(_name.c_str(), m->GetMaterial(i)->Name.c_str()))
			{
				m->GetMaterial(i)->emmit_brightness = _intensity;
				return m->GetMaterial(i)->Emmit.GetAddressOf();
			}
		}
	}

	return nullptr;
}

void MeshModel::SetMaterialTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cpTex, std::wstring _name, int tex_slot, int _alpha_mode, int specular_power)
{
	for (Mesh* m : m_mesh)
	{
		for (int i = 0; i < m->GetMaterials().size(); i++)
		{
			if (!wcscmp(_name.c_str(), m->GetMaterial(i)->Name.c_str()))
			{
				m->GetMaterial(i)->Specular[3] = specular_power;
				m->alpha_mode = _alpha_mode;
				m->GetMaterial(i)->Textures[tex_slot] = cpTex;
			}
		}
	}

	return;
}

void MeshModel::SetMaterialNormal(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cpTex, std::wstring _name, float _intensity)
{
	for (Mesh* m : m_mesh)
	{
		for (int i = 0; i < m->GetMaterials().size(); i++)
		{
			if (!wcscmp(_name.c_str(), m->GetMaterial(i)->Name.c_str()))
			{
				m->GetMaterial(i)->normal_height = _intensity;
				m->GetMaterial(i)->Normal = cpTex;
			}
		}
	}

	return;
}

void MeshModel::SetMaterialEmmit(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cpTex, std::wstring _name, float _intensity)
{
	for (Mesh* m : m_mesh)
	{
		for (int i = 0; i < m->GetMaterials().size(); i++)
		{
			if (!wcscmp(_name.c_str(), m->GetMaterial(i)->Name.c_str()))
			{
				m->GetMaterial(i)->emmit_brightness = _intensity;
				m->GetMaterial(i)->Emmit = cpTex;
			}
		}
	}

	return;
}