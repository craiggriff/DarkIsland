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
#include "AllStructures.h"
#include "DeviceResources.h"
// Camera:
// This class defines the position, orientation and viewing frustum of a camera looking into
// a 3D world.  It will generate both the View matrix and Projection matrix.  It can also
// provide a pair of Projection matrices to be used when stereoscopic 3D is used.
namespace Game
{
	class Camera
	{
	public:
		Camera(const std::shared_ptr<DX::DeviceResources>& deviceResources);

		void CreateWindowSizeDependentResources();

		void SetSkyProjection();
		void SetCloseProjection();

		void SetViewParams(_In_ XMFLOAT3 eye, _In_ XMFLOAT3 lookAt, _In_ XMFLOAT3 up);
		void SetProjParams(_In_ float fieldOfView, _In_ float aspectRatio, _In_ float nearPlane, _In_ float farPlane);

		void UpDirection(_In_ XMFLOAT3 upDirection);
		void LookDirection(_In_ XMFLOAT3 lookDirection);
		void Eye(_In_ XMFLOAT3 position);
		void LookAtY(float look_height);

		void buildWorldFrustumPlanes();
		bool CheckPlanes(float x, float y, float z, float radius);

		float EyePlaneDepth(float x, float y, float z);

		float CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius, float dist, float* full_dist = nullptr);
		float CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius);
		float DistanceFromEye(float x, float y, float z);

		float Within3DManhattanDistance(float x, float y, float z, float x1, float y1, float z1, float distance);
		float Within3DManhattanEyeDistance(float x, float y, float z, float distance);

		float Distance(float x, float y, float z, float x1, float y1, float z1);
		float DistanceEst(float x, float y, float z, float x1, float y1, float z1);

		void LookAt(XMFLOAT3 la);

		void Up(XMFLOAT3 up) {
			m_up = up;
		}

		XMMATRIX View();
		XMMATRIX Projection();
		XMMATRIX LeftEyeProjection();
		XMMATRIX RightEyeProjection();
		XMMATRIX World();
		XMFLOAT3 Eye();
		XMFLOAT3 LookAt();
		XMFLOAT3 Up() {
			return m_up;
		}
		float NearClipPlane();
		float FarClipPlane();
		float Pitch();
		float Yaw();

		float view_distance;

		float LookingTanX();
		float LookingTanZ();
		float LookingX();
		float LookingY();
		float LookingZ();
		XMFLOAT3 LookingDir();
		float PositionX();
		float PositionZ();

		void UpdateConstantBuffer(XMFLOAT4X4 &proj_matrix, XMFLOAT4X4 &view_matrix);
		void UpdateConstantBuffer();
		void UpdateConstantBufferOrth();

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	 m_constantBufferData;
		ModelViewProjectionConstantBuffer	 m_orthBufferData; // for def rendering

		XMFLOAT4X4 close_projection;
		XMFLOAT4X4 sky_projection;
		XMFLOAT4X4 skycube_projection;
		XMFLOAT4X4 cube_projection;

		XMFLOAT4X4 m_viewMatrix;
		XMFLOAT4X4 m_projectionMatrix;
		XMFLOAT4X4 m_projectionMatrixLeft;
		XMFLOAT4X4 m_projectionMatrixRight;
	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		XMFLOAT3 view_dir;
		XMFLOAT3 view_tan;

		XMFLOAT4	mFrustumPlanes[6]; 	// [0] = near, [1] = far, [2] = left, [3] = right, [4] = top, [5] = bottom

		float plane_dist[6];

		Windows::Foundation::Size outputSize;

		XMFLOAT4X4 m_inverseView;

		XMFLOAT3 m_eye;
		XMFLOAT3 m_lookAt;
		XMFLOAT3 m_up;
		float             m_cameraYawAngle;
		float             m_cameraPitchAngle;

		float             m_fieldOfView;
		float             m_aspectRatio;
		float             m_nearPlane;
		float             m_farPlane;
	};
}