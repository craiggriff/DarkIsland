#include "pch.h"
#include "Player.h"
#include "defgame.h"

using namespace Game;
//----------------------------------------------------------------------

Player::Player(AllResources* p_Resources, Level* p_Level)
{
	m_Res = p_Resources;
	m_deviceResources = p_Resources->m_deviceResources;

	m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_radius = 1.0f;

	m_Level = p_Level;

	player_eye_height = 2.0f;

	bDidCollide = false;

	vel_y = 0.0f;
	bContact = false;
	//Update();
}

bool Player::IsPlayer(btRigidBody* rb)
{
	if (rb == m_Body)
		return true;
	else
		return false;
}

void Player::MakePhysics()
{
	ObjInfo bod_info = {
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(1.1f, 1.1f, 1.0f),
		0
		, (COL_WALLS | COL_CARBODY | COL_TERRAIN | COL_WHEEL | COL_OBJECTS)
		, (COL_CHAR) };

	m_Body = MakePhysicsSphereFromExtent(&bod_info, 1.0f, 0.5f);
	m_Body->setDamping(0.0f, 0.2f);
	m_Body->setActivationState(DISABLE_DEACTIVATION);

	btTransform tran = m_Body->getWorldTransform();
	tran.setOrigin(btVector3(m_position.x, m_position.y + player_eye_height, m_position.z));

	m_Body->setWorldTransform(tran);

	m_last_position = m_position;
}

btRigidBody*  Player::MakePhysicsSphereFromExtent(ObjInfo* ob_info, float extent, float _scale)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	if (extent2 > extent1)
		extent1 = extent2;

	if (extent3 > extent1)
		extent1 = extent3;

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z)));

	auto fallShape = new btSphereShape(btSphereShape(extent));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = m_Res->m_Physics.AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}


btRigidBody*  Player::MakePhysicsCylinderExtents(ObjInfo* ob_info, float extent1, float extent2, float extent3, float _scale)
{
	extent1 *= _scale;
	extent2 *= _scale;
	extent3 *= _scale;

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z)));

	auto fallShape = new btCylinderShape(btCylinderShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = m_Res->m_Physics.AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

//----------------------------------------------------------------------

Player::Player(
	XMFLOAT3 position,
	float radius
)
{
	m_position = position;
	m_radius = radius;
	//Update();
}

//----------------------------------------------------------------------

void Player::UpdatePhysics(float timeFrame)
{
	btVector3 p_velocity = m_Body->getLinearVelocity();
	p_velocity.setY(0.0f);

	btTransform tran = m_Body->getWorldTransform();

	float xdif = abs(m_position.x - tran.getOrigin().getX());
	float zdif = abs(m_position.z - tran.getOrigin().getZ());

	if (xdif > 0.000005f && xdif < 10.0f)
		m_position.x = tran.getOrigin().getX();

	if (zdif > 0.000005f && zdif < 10.0f)
		m_position.z = tran.getOrigin().getZ();

	m_position.y = tran.getOrigin().getY() - player_eye_height;

	vel_y += 0.1f *PHYSICS_STEP_SIMULATION * PHYSICS_GRAVITY * timeFrame;

	//m_position.y += vel_y;

	/*
	float ground_pos = m_Level->GetTerrainHeight(m_position.x, m_position.z) + 2.1f;
	if (m_position.y <= ground_pos)
	{
	m_position.y = ground_pos;
	vel_y = 0.0f;
	bContact = true;
	}
	*/
	//float gpos = m_Level->GetTerrainHeight(m_position.x, m_position.z) + 2.1f;
	//if (m_position.y < gpos)
	//	m_position.y = gpos;
	bContact = false;

	float ground_pos = m_Level->GetTerrainHeight(m_position.x, m_position.z) + 0.9f;
	if (m_position.y <= ground_pos)
	{
		m_position.y = ground_pos;
		bContact = true;
	}

	//else
	{
		float above_check = 0.9f;
		btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(m_position.x, m_position.y + above_check, m_position.z), btVector3(m_position.x, m_position.y - 8.2f, m_position.z));
		m_Res->m_Physics.m_dynamicsWorld->rayTest(btVector3(m_position.x, m_position.y + above_check, m_position.z), btVector3(m_position.x, m_position.y - 8.2f, m_position.z), RayCallback);
		if (RayCallback.hasHit())
		{
			btVector3 pos = RayCallback.m_hitPointWorld;
			float dist = m_position.y - pos.getY();

			if (dist > 0.9f)
			{
				bContact = false;
			}
			else
			{
				bContact = true;
				m_position.y += (0.9f - dist)*0.2f;// ((pos.getY() + 2.0f) - m_position.y) * (timeFrame*10.0f);
				vel_y = 0.0f;
				m_Body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
			}
		}
	}

	m_Body->setWorldTransform(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(m_position.x, m_position.y + player_eye_height, m_position.z)));

	m_Body->setLinearVelocity(btVector3(m_velocity.x, m_Body->getLinearVelocity().getY(), m_velocity.z));
	//m_Body->setLinearVelocity(btVector3(m_velocity.x, m_velocity.y, m_velocity.z));

	m_last_position = m_position;
	m_last_velocity = m_velocity;
}

void Player::Jump()
{
	if (bContact == true)
	{
		vel_y += 0.3f;
		m_Body->setLinearVelocity(btVector3(m_velocity.x, 4.3f, m_velocity.z));
		m_position.y += 0.1f;
		m_Body->setWorldTransform(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(m_position.x, m_position.y + player_eye_height, m_position.z)));
	}
}
//----------------------------------------------------------------------

bool Player::IsTouching(
	XMFLOAT3 point,
	float radius,
	_Out_ XMFLOAT3 *contact,
	_Out_ XMFLOAT3 *normal
)
{
	// Check collision between instances One and Two.
	// oneToTwo is the collision normal vector.
	XMVECTOR oneToTwo = XMLoadFloat3(&m_position) - XMLoadFloat3(&point);

	float distance = XMVectorGetX(XMVector3Length(oneToTwo));

	oneToTwo = XMVector3Normalize(oneToTwo);
	XMStoreFloat3(normal, oneToTwo);
	XMStoreFloat3(contact, oneToTwo * m_radius);

	if (distance < 0.0f)
	{
		distance *= -1.0f;
	}

	if (distance < (radius + m_radius))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//----------------------------------------------------------------------