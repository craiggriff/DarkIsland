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

// GameObject:
// This is the class representing a generic object in the 3D world of the game.  There are
// specific sub-classes that have specific geometric shapes.  This class contains all the
// properties of objects that are common.
//
// During the game physics calculations the IsTouching method will be called to determine
// the object's proximity to a point.  It is expected the sub-classes will replace this method.
// The Render method will be called during rendering to include the object in the generation of
// the scene.

#include "MeshObject.h"
#include "SoundEffect.h"
#include "Animate.h"
#include "Material.h"
#include "../AllResources.h"

namespace Game
{
	ref class GameObject
	{
	internal:
		GameObject();

		// Expect the IsTouching method to be overloaded by subclasses.
		virtual bool IsTouching(
			XMFLOAT3 /* point */,
			float /* radius */,
			_Out_ XMFLOAT3 *contact,
			_Out_ XMFLOAT3 *normal
		)
		{
			*contact = XMFLOAT3(0.0f, 0.0f, 0.0f);
			*normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
			return false;
		};

		void Render(
			AllResources &graphics,
			_In_ ID3D11Buffer *primitiveConstantBuffer
		);

		void Active(bool active);
		bool Active();
		void Target(bool target);
		bool Target();
		void Hit(bool hit);
		bool Hit();
		void OnGround(bool ground);
		bool OnGround();
		void TargetId(int targetId);
		int  TargetId();
		void HitTime(float t);
		float HitTime();

		void     AnimatePosition(_In_opt_ Animate^ animate);
		Animate^ AnimatePosition();

		void         HitSound(_In_ SoundEffect^ hitSound);
		SoundEffect^ HitSound();

		void PlaySound(float impactSpeed, XMFLOAT3 eyePoint);

		void Mesh(_In_ MeshObject^ mesh);

		void NormalMaterial(_In_ Material^ material);
		Material^ NormalMaterial();
		void HitMaterial(_In_ Material^ material);
		Material^ HitMaterial();

		void Position(XMFLOAT3 position);
		void Position(XMVECTOR position);
		void Velocity(XMFLOAT3 velocity);
		void Velocity(XMVECTOR velocity);
		XMMATRIX ModelMatrix();
		XMFLOAT3 Position();
		XMVECTOR VectorPosition();
		XMVECTOR VectorVelocity();
		XMFLOAT3 Velocity();

	protected private:
		virtual void UpdatePosition() {};
		// Object Data
		bool                m_active;
		bool                m_target;
		int                 m_targetId;
		bool                m_hit;
		bool                m_ground;

		XMFLOAT3   m_position;
		XMFLOAT3   m_velocity;
		XMFLOAT4X4 m_modelMatrix;

		Material^           m_normalMaterial;
		Material^           m_hitMaterial;

		XMFLOAT3   m_defaultXAxis;
		XMFLOAT3   m_defaultYAxis;
		XMFLOAT3   m_defaultZAxis;

		float               m_hitTime;

		Animate^            m_animatePosition;
		MeshObject^         m_mesh;

		SoundEffect^        m_hitSound;
	};

	__forceinline void GameObject::Active(bool active)
	{
		m_active = active;
	}

	__forceinline bool GameObject::Active()
	{
		return m_active;
	}

	__forceinline void GameObject::Target(bool target)
	{
		m_target = target;
	}

	__forceinline bool GameObject::Target()
	{
		return m_target;
	}

	__forceinline void GameObject::Hit(bool hit)
	{
		m_hit = hit;
	}

	__forceinline bool GameObject::Hit()
	{
		return m_hit;
	}

	__forceinline void GameObject::OnGround(bool ground)
	{
		m_ground = ground;
	}

	__forceinline bool GameObject::OnGround()
	{
		return m_ground;
	}

	__forceinline void GameObject::TargetId(int targetId)
	{
		m_targetId = targetId;
	}

	__forceinline int GameObject::TargetId()
	{
		return m_targetId;
	}

	__forceinline void GameObject::HitTime(float t)
	{
		m_hitTime = t;
	}

	__forceinline float GameObject::HitTime()
	{
		return m_hitTime;
	}

	__forceinline void GameObject::Position(XMFLOAT3 position)
	{
		m_position = position;
		// Update any internal states that are dependent on the position.
		// UpdatePosition is a virtual function that is specific to the derived class.
		UpdatePosition();
	}

	__forceinline void GameObject::Position(XMVECTOR position)
	{
		XMStoreFloat3(&m_position, position);
		// Update any internal states that are dependent on the position.
		// UpdatePosition is a virtual function that is specific to the derived class.
		UpdatePosition();
	}

	__forceinline XMFLOAT3 GameObject::Position()
	{
		return m_position;
	}

	__forceinline XMVECTOR GameObject::VectorPosition()
	{
		return XMLoadFloat3(&m_position);
	}

	__forceinline void GameObject::Velocity(XMFLOAT3 velocity)
	{
		m_velocity = velocity;
	}

	__forceinline void GameObject::Velocity(XMVECTOR velocity)
	{
		XMStoreFloat3(&m_velocity, velocity);
	}

	__forceinline XMFLOAT3 GameObject::Velocity()
	{
		return m_velocity;
	}

	__forceinline XMVECTOR GameObject::VectorVelocity()
	{
		return XMLoadFloat3(&m_velocity);
	}

	__forceinline void GameObject::AnimatePosition(_In_opt_ Animate^ animate)
	{
		m_animatePosition = animate;
	}

	__forceinline Animate^ GameObject::AnimatePosition()
	{
		return m_animatePosition;
	}

	__forceinline void GameObject::NormalMaterial(_In_ Material^ material)
	{
		m_normalMaterial = material;
	}

	__forceinline Material^ GameObject::NormalMaterial()
	{
		return m_normalMaterial;
	}

	__forceinline void GameObject::HitMaterial(_In_ Material^ material)
	{
		m_hitMaterial = material;
	}

	__forceinline Material^ GameObject::HitMaterial()
	{
		return m_hitMaterial;
	}

	__forceinline void GameObject::Mesh(_In_ MeshObject^ mesh)
	{
		m_mesh = mesh;
	}

	__forceinline void GameObject::HitSound(_In_ SoundEffect^ hitSound)
	{
		m_hitSound = hitSound;
	}

	__forceinline SoundEffect^ GameObject::HitSound()
	{
		return m_hitSound;
	}

	__forceinline XMMATRIX GameObject::ModelMatrix()
	{
		return XMLoadFloat4x4(&m_modelMatrix);
	}
}