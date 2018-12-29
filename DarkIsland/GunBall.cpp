#include "pch.h"
#include "GunBall.h"

//#include "Stuff.h"

using namespace Game;

GunBall::GunBall(AllResources* p_Resources, Stuff* pp_Stuff)
{
	m_Res = p_Resources;
	p_Stuff = pp_Stuff;

	m_newballs.clear();

	m_deviceResources = p_Resources->m_deviceResources;

	m_meshModel = new MeshModel(m_deviceResources, &p_Resources->m_Physics);

	model_type = 0;
}

void GunBall::UpdatePhysics()
{
	for (ball_t b : m_newballs)
	{
		ObjInfo info;

		info.dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		info.pos = XMFLOAT3(b.pos.x, b.pos.y, b.pos.z);
		info.group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		info.mask = (COL_OBJECTS | COL_RAY);
		info.mrf = XMFLOAT3(0.3f, 0.5f, 0.5f);

		if (p_Stuff->m_stuff_model[b.model_type]->physics_shape == PHY_BOX)
		{
			btRigidBody* body = p_Stuff->m_stuff_model[model_type]->MakePhysicsBoxFromMeshModel(&info, 1.0f);
			body->setLinearVelocity(btVector3(b.vel.x, b.vel.y, b.vel.z));
			body->setActivationState(DISABLE_DEACTIVATION);
			body->item_id = b.model_type;
			m_Body.push_back(body);
		}
		if (p_Stuff->m_stuff_model[b.model_type]->physics_shape == PHY_CYLINDER)
		{
			btRigidBody* body = p_Stuff->m_stuff_model[model_type]->MakePhysicsCylinderFromMeshModel(&info, 1.0f);
			body->setLinearVelocity(btVector3(b.vel.x, b.vel.y, b.vel.z));
			body->setActivationState(DISABLE_DEACTIVATION);
			body->item_id = b.model_type;
			m_Body.push_back(body);
		}
		if (p_Stuff->m_stuff_model[b.model_type]->physics_shape == PHY_SPHERE)
		{
			btRigidBody* body = p_Stuff->m_stuff_model[model_type]->MakePhysicsCylinderFromMeshModel(&info, 1.0f);
			body->setLinearVelocity(btVector3(b.vel.x, b.vel.y, b.vel.z));
			body->setActivationState(DISABLE_DEACTIVATION);
			body->item_id = b.model_type;
			m_Body.push_back(body);
		}
		if (p_Stuff->m_stuff_model[b.model_type]->physics_shape == PHY_ELLIPSEOID)
		{
			btRigidBody* body = p_Stuff->m_stuff_model[model_type]->MakePhysicsEllipseoidFromMeshModel(&info, 1.0f);
			body->setLinearVelocity(btVector3(b.vel.x, b.vel.y, b.vel.z));
			body->setActivationState(DISABLE_DEACTIVATION);
			body->item_id = b.model_type;
			m_Body.push_back(body);
		}
		if (p_Stuff->m_stuff_model[b.model_type]->physics_shape == PHY_CONVEXHULL)
		{
			btRigidBody* body = p_Stuff->m_stuff_model[model_type]->MakePhysicsConvexHullFromMeshModel(&info, 1.0f);
			body->setLinearVelocity(btVector3(b.vel.x, b.vel.y, b.vel.z));
			body->setActivationState(DISABLE_DEACTIVATION);
			body->item_id = b.model_type;
			m_Body.push_back(body);
		}

		//btRigidBody* body = p_Stuff->m_stuff_model[model_type]->MakePhysicsBoxFromMeshModel(&info, 1.0f);
	}

	m_newballs.clear();
}

void GunBall::CreateOne(XMFLOAT3 pos, XMFLOAT3 vel)
{
	ball_t bt;
	bt.pos = pos;
	bt.vel = vel;
	bt.model_type = model_type;

	m_newballs.push_back(bt);
}

bool GunBall::IsGunBall(btRigidBody* rb)
{
	for (btRigidBody* b : m_Body)
	{
		if (rb == b)
			return true;
	}
	return false;
}

void GunBall::Render()
{
	for (btRigidBody* b : m_Body)
	{
		*m_Res->ConstantModelBuffer() = m_Res->GetMatrix(b);
		m_Res->m_Camera->UpdateConstantBuffer();

		for (Mesh* m : p_Stuff->m_stuff_model[b->item_id]->m_mesh)
		{
			m->Render(*m_Res);
		}
	}
}