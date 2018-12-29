//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "GameObject.h"
#include "level.h"
#include "btMyMotionState.h"
namespace Game
{
	ref class Player : public GameObject
	{
	internal:
		Player(AllResources* p_Resources, Level* p_Level);
		Player(XMFLOAT3 pos, float radius);

		void Position(XMFLOAT3 position);
		void Position(XMVECTOR position);
		void Radius(float radius);
		float Radius();

		virtual bool IsTouching(
			XMFLOAT3 point,
			float radius,
			_Out_ XMFLOAT3 *contact,
			_Out_ XMFLOAT3 *normal
		) override;

		float player_eye_height;

		bool bDidCollide;

		XMFLOAT3   m_last_position;
		XMFLOAT3   m_last_velocity;

		void ResetVelocity() { m_Body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f)); m_last_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f); }

		btRigidBody* m_Body;

		Level* m_Level;

		btVector3 p_lastvelocity;

		btRigidBody*  MakePhysicsCylinderExtents(ObjInfo* ob_info, float extent1, float extent2, float extent3, float _scale);
		btRigidBody*  Player::MakePhysicsSphereFromExtent(ObjInfo* ob_info, float extent, float _scale);

		void UpdatePhysics(float timeFrame);
		void MakePhysics();
		bool IsPlayer(btRigidBody* rb);

		void Jump();
	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Res;

		btMyMotionState* m_motion;

		float vel_y;
		bool bContact;

		float m_radius;
	};

	__forceinline void Player::Position(XMFLOAT3 position)
	{
		m_position = position;
		if (false)//(m_Body != nullptr)
		{
			btTransform tran = m_Body->getWorldTransform();
			tran.setOrigin(btVector3(m_position.x, m_position.y, m_position.z));
			m_Body->setWorldTransform(tran);
		}
		//Update();
	}

	__forceinline void Player::Position(XMVECTOR position)
	{
		XMStoreFloat3(&m_position, position);
		//Update();
	}

	__forceinline void Player::Radius(float radius)
	{
		m_radius = radius;
		//Update();
	}

	__forceinline float Player::Radius()
	{
		return m_radius;
	}
}