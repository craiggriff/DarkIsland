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

// Cylinder:
// This class is a specialization of GameObject that represents a cylinder primitive.
// The cylinder is defined by a vector starting at 'position' and oriented along the
// 'direction' vector.  The length of the cylinder is just the length of the 'direction'
// vector.

#include "GameObject.h"
namespace Game
{
	ref class Cylinder : public GameObject
	{
	internal:
		Cylinder();
		Cylinder(
			XMFLOAT3 position,
			float radius,
			XMFLOAT3 direction
		);

		virtual bool IsTouching(
			XMFLOAT3 point,
			float radius,
			_Out_ XMFLOAT3 *contact,
			_Out_ XMFLOAT3 *normal
		) override;

	protected:
		virtual void UpdatePosition() override;

	private:
		void Initialize(
			XMFLOAT3 position,
			float radius,
			XMFLOAT3 direction
		);

		XMFLOAT3   m_axis;
		float               m_length;
		float               m_radius;
		XMFLOAT4X4 m_rotationMatrix;
	};
}