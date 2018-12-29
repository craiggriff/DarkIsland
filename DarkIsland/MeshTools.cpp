#include "pch.h"

#include "MeshTools.h"

#include "../Bullet/src/LinearMath/btConvexHullComputer.h"

using namespace Game;



void SimpleMotion2::getWorldTransform(btTransform &worldTransform) const
{
	worldTransform = m_initialTransform;
}

void SimpleMotion2::setWorldTransform(const btTransform &worldTransform)
{

}

SimpleMotion2::SimpleMotion2()
{

}

SimpleMotion2::~SimpleMotion2()
{


}


MeshTools::MeshTools(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Physics* phys)
{
	m_deviceResources = pm_deviceResources;
	p_Phys = phys;

	m_motion = new SimpleMotion2();
}


MeshTools::~MeshTools()
{


}
btRigidBody*  MeshTools::MakePhysicsNonColision()
{
	ObjInfo ob_info = {
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.01f, 0.1f, 0.01f),
		0
		, (COL_WHEELLIFT | COL_OBJECTS)
		, (COL_WHEEL) };


	m_motion->m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	m_motion->setWorldTransform(m_motion->m_initialTransform);

	//bt
	auto fallShape = new btSphereShape(btSphereShape(1.0f));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);

	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);

	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshTools::MakePhysicsEllipseoidFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = p_model->Extents().MaxX*_scale;
	extent2 = p_model->Extents().MaxY*_scale*0.6f;
	extent3 = p_model->Extents().MaxZ*_scale;

	float uniform_value = 1.0f;

	m_motion->m_initialTransform = btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z));

	m_motion->setWorldTransform(m_motion->m_initialTransform);

	auto fallShape = new btMultiSphereShape(&btVector3(0.0f, 0.0f, 0.0f), &uniform_value, 1);

	fallShape->setLocalScaling(btVector3(extent1, extent2, extent3));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshTools::MakePhysicsSphereFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale)
{

	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = p_model->Extents().MaxX*_scale;
	extent2 = p_model->Extents().MaxY*_scale;
	extent3 = p_model->Extents().MaxZ*_scale;

	if (extent2 > extent1)
		extent1 = extent2;

	if (extent3 > extent1)
		extent1 = extent3;


	m_motion->m_initialTransform = btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z));

	m_motion->setWorldTransform(m_motion->m_initialTransform);

	auto fallShape = new btSphereShape(btSphereShape(extent1));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshTools::MakePhysicsCylinderExtents(ObjInfo* ob_info, float extent1, float extent2, float extent3, float _scale)
{
	extent1 *= _scale;
	extent2 *= _scale;
	extent3 *= _scale;

	m_motion->m_initialTransform = btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z));

	m_motion->setWorldTransform(m_motion->m_initialTransform);

	auto fallShape = new btCylinderShape(btCylinderShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshTools::MakePhysicsCylinderFBX(Mesh* p_model, ObjInfo* ob_info, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = p_model->Extents().MaxX*_scale;
	extent2 = p_model->Extents().MaxY*_scale;
	extent3 = p_model->Extents().MaxZ*_scale;

	m_motion->m_initialTransform = btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z));

	m_motion->setWorldTransform(m_motion->m_initialTransform);

	auto fallShape = new btCylinderShape(btCylinderShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshTools::MakePhysicsSquareplane(ObjInfo* ob_info, float size, float y_offset)
{
	btVector3 A;
	btVector3 B;
	btVector3 C;
	btTriangleMesh* data = new btTriangleMesh();

	A = btVector3(-size, y_offset, size);
	B = btVector3(size, y_offset, size);
	C = btVector3(size, y_offset, -size);
	data->addTriangle(A, B, C, false);

	A = btVector3(size, y_offset, -size);
	B = btVector3(-size, y_offset, -size);
	C = btVector3(-size, y_offset, size);
	data->addTriangle(A, B, C, false);

	m_motion->m_initialTransform = btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z));

	m_motion->setWorldTransform(m_motion->m_initialTransform);

	auto fallShape = new btBvhTriangleMeshShape(data, true);

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshTools::MakePhysicsBoxExtents(ObjInfo* ob_info, float extent1, float extent2, float extent3, float _scale)
{
	m_motion->m_initialTransform = btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z));

	m_motion->setWorldTransform(m_motion->m_initialTransform);

	auto fallShape = new btBoxShape(btBoxShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}





btRigidBody*  MeshTools::MakePhysicsBoxFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = p_model->Extents().MaxX*_scale;
	extent2 = p_model->Extents().MaxY*_scale;
	extent3 = p_model->Extents().MaxZ*_scale;

	m_motion->m_initialTransform = btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z));

	m_motion->setWorldTransform(m_motion->m_initialTransform);

	auto fallShape = new btBoxShape(btBoxShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}


btRigidBody*  MeshTools::MakePhysicsConvexHullFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	btConvexHullComputer hull_comp;

	std::vector<float> coords;

	coords.clear();// resize(p_model->GetPoints().size() * 3);
	float point = 0.0f;
	for (int i = 0; i < p_model->GetPoints().size(); i++)
	{
		coords.push_back(p_model->GetPoints()[i].point.x);
		coords.push_back(p_model->GetPoints()[i].point.y);
		coords.push_back(p_model->GetPoints()[i].point.z);
	}

	
	//p_model->Points->

	hull_comp.compute((float *)coords.data(), sizeof(float)*3, p_model->GetPoints().size(), 0.0f, 0.0f);
	
	//hull_comp.compute((float *)&p_model->GetPoints().data(),)
	//p_model->Points

	//hull_comp.compute

	auto fallShape = new btConvexHullShape();
	//btConvexTriangleMeshShape
	//m_model->SubMeshes[0]
	btVector3 vec;
	for (int i = 0; i < hull_comp.vertices.size(); i++)
	{
		vec.setX(hull_comp.vertices[i].getX());
		vec.setY(hull_comp.vertices[i].getY());
		vec.setZ(hull_comp.vertices[i].getZ());

		fallShape->addPoint(vec);
	}

	m_motion->m_initialTransform = btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z));

	m_motion->setWorldTransform(m_motion->m_initialTransform);

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshTools::MakePhysicsCompoundBoxFromFBX(Mesh* p_model, ObjInfo* ob_info, float x, float y, float z, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = p_model->Extents().MaxX*_scale;
	extent2 = p_model->Extents().MaxY*_scale;
	extent3 = p_model->Extents().MaxZ*_scale;

	extent2 = abs(extent2) *0.6f;

	m_motion->m_initialTransform = btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z));

	m_motion->setWorldTransform(m_motion->m_initialTransform);

	btCollisionShape* boxShape = new btBoxShape(btVector3(extent1, extent2, extent3));
	//btCollisionShape* boxShape = new btSphereShape(2.0f);
	btCompoundShape* compound = new btCompoundShape();
	btTransform localTrans;
	localTrans.setIdentity();
	//localTrans effectively shifts the center of mass with respect to the chassis
	localTrans.setOrigin(btVector3(x, y, z));
	compound->addChildShape(localTrans, boxShape);

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	compound->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(compound, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

XMFLOAT4X4 MeshTools::GetMatrix(btTransform* p_body)
{
	XMFLOAT4X4 m_Matrix;

	btTransform trans;


	//trans = p_body->getWorldTransform();

	btQuaternion q_rotation = p_body->getRotation();

	XMVECTOR data;
	XMVECTORF32 floatingVector = { q_rotation.getX(), q_rotation.getY(), q_rotation.getZ(), q_rotation.getW() };
	data = floatingVector;

	auto rotationMatrix = XMMatrixRotationQuaternion(data);

	auto translationMatrix = XMMatrixTranslation(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());

	XMStoreFloat4x4(&m_Matrix, XMMatrixTranspose(rotationMatrix * translationMatrix));

	return m_Matrix;
}


XMFLOAT4X4 MeshTools::GetMatrix(btRigidBody* p_body)
{
	XMFLOAT4X4 m_Matrix;

	btTransform trans;


	trans = p_body->getWorldTransform();

	btQuaternion q_rotation = trans.getRotation();

	XMVECTOR data;
	XMVECTORF32 floatingVector = { q_rotation.getX(), q_rotation.getY(), q_rotation.getZ(), q_rotation.getW() };
	data = floatingVector;

	auto rotationMatrix = XMMatrixRotationQuaternion(data);

	auto translationMatrix = XMMatrixTranslation(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());

	XMStoreFloat4x4(&m_Matrix, XMMatrixTranspose(rotationMatrix * translationMatrix));

	return m_Matrix;
}

