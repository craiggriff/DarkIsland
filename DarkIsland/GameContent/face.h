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

// Face:
// This class is a specialization of GameObject that represents a parallelogram primitive.
// The face is defined by three points.  It is positioned at 'origin'.  The four corners
// of the face are defined at 'origin', 'p1', 'p2' and 'p1' + ('p2' - 'origin').

#include "GameObject.h"
namespace Game
{

	ref class Face : public GameObject
	{
	internal:
		Face();
		Face(
			XMFLOAT3 origin,
			XMFLOAT3 p1,
			XMFLOAT3 p2
		);

		void SetPlane(
			XMFLOAT3 origin,
			XMFLOAT3 p1,
			XMFLOAT3 p2
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
		void UpdateMatrix();

		XMFLOAT3   m_widthVector;
		XMFLOAT3   m_heightVector;
		XMFLOAT3   m_normal;
		XMFLOAT3   m_point[4];
		float               m_width;
		float               m_height;
		XMFLOAT4X4 m_rotationMatrix;
	};
}
