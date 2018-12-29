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

#include "pch.h"
#include "Camera.h"
#include "StereoProjection.h"

using namespace Game;

#undef min // Use __min instead.
#undef max // Use __max instead.

//--------------------------------------------------------------------------------------

Camera::Camera(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Setup the view matrix.
	SetViewParams(
		XMFLOAT3(0.0f, 0.0f, 0.0f),   // Default eye position.
		XMFLOAT3(0.0f, 0.0f, 1.0f),   // Default look at position.
		XMFLOAT3(0.0f, 1.0f, 0.0f)    // Default up vector.
	);

	// Setup the projection matrix.
	//SetProjParams(XM_PI / 4, 1.0f, 1.0f, 1000.0f);
	SetProjParams(XM_PI / 2, 1.0f, 0.01f, 100.0f);

	view_distance = 100.0f;
}

void Camera::SetSkyProjection()
{
	m_constantBufferData.projection = sky_projection;
}

void Camera::SetCloseProjection()
{
	m_constantBufferData.projection = close_projection;
}

void Camera::CreateWindowSizeDependentResources()
{
	outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	SetProjParams(
		XM_PI / 2,
		outputSize.Width / outputSize.Height,
		0.01f,
		view_distance
	);

	close_projection = m_projectionMatrix;

	SetProjParams(
		XM_PI / 2,
		outputSize.Width / outputSize.Height,
		0.01f,
		SKY_SIZE + (SKY_SIZE*2.0f)
	);

	sky_projection = m_projectionMatrix;

	// Initialize the world matrix to the identity matrix.
	XMStoreFloat4x4(&m_orthBufferData.model, XMMatrixIdentity());

	XMStoreFloat4x4(&m_orthBufferData.projection, XMMatrixOrthographicRH(outputSize.Width, outputSize.Height, 0.1f, 100.0f));

	XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	XMStoreFloat4x4(&m_orthBufferData.view, XMMatrixLookAtRH(XMLoadFloat3(&pos), XMLoadFloat3(&look), XMLoadFloat3(&up)));
}

void Camera::UpdateConstantBuffer(XMFLOAT4X4 &proj_matrix, XMFLOAT4X4 &view_matrix)
{
	m_constantBufferData.view = view_matrix;
	m_constantBufferData.projection = proj_matrix;

	XMStoreFloat4x4(&m_constantBufferData.mvp, XMLoadFloat4x4(&m_constantBufferData.projection) * XMLoadFloat4x4(&m_constantBufferData.view) *XMLoadFloat4x4(&m_constantBufferData.model));

	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0);
}

void Camera::UpdateConstantBuffer()
{
	XMStoreFloat4x4(&m_constantBufferData.mvp, XMLoadFloat4x4(&m_constantBufferData.projection) * XMLoadFloat4x4(&m_constantBufferData.view) *XMLoadFloat4x4(&m_constantBufferData.model));

	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0);
}

void Camera::UpdateConstantBufferOrth()
{
	XMStoreFloat4x4(&m_orthBufferData.mvp, XMLoadFloat4x4(&m_orthBufferData.projection) * XMLoadFloat4x4(&m_orthBufferData.view) *XMLoadFloat4x4(&m_orthBufferData.model));

	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &m_orthBufferData, sizeof(m_orthBufferData), sizeof(m_orthBufferData));
}

void Camera::buildWorldFrustumPlanes()
{
	XMMATRIX identityMatrix = XMMatrixIdentity();
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&m_constantBufferData.projection);
	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_constantBufferData.view);

	projectionMatrix = XMMatrixInverse(nullptr, projectionMatrix);

	viewMatrix = viewMatrix;// *XMLoadFloat4x4(&m_deviceResources->GetOrientationTransform3D());
	viewMatrix = XMMatrixInverse(nullptr, viewMatrix);

	XMMATRIX VP = XMMatrixMultiply(viewMatrix, projectionMatrix)*XMMatrixTranspose(XMLoadFloat4x4(&m_deviceResources->GetOrientationTransform3D()));;

	VP = XMMatrixInverse(nullptr, VP);
	XMFLOAT4X4 VPvalues;
	XMStoreFloat4x4(&VPvalues, VP);

	XMVECTOR col0 = XMVectorSet(VPvalues(0, 0), VPvalues(0, 1), VPvalues(0, 2), VPvalues(0, 3));
	XMVECTOR col1 = XMVectorSet(VPvalues(1, 0), VPvalues(1, 1), VPvalues(1, 2), VPvalues(1, 3));
	XMVECTOR col2 = XMVectorSet(VPvalues(2, 0), VPvalues(2, 1), VPvalues(2, 2), VPvalues(2, 3));
	XMVECTOR col3 = XMVectorSet(VPvalues(3, 0), VPvalues(3, 1), VPvalues(3, 2), VPvalues(3, 3));

	// Planes face inward.
	XMStoreFloat4(&mFrustumPlanes[0], -(col2 + col1)); // near
	XMStoreFloat4(&mFrustumPlanes[1], -(col2 - col1)); // far
	XMStoreFloat4(&mFrustumPlanes[2], -(col3 + col0)); // left
	XMStoreFloat4(&mFrustumPlanes[3], -(col3 - col0)); // right
	XMStoreFloat4(&mFrustumPlanes[4], -(col3 + col1)); // top
	XMStoreFloat4(&mFrustumPlanes[5], -(col3 - col1)); // bottom

	for (int i = 0; i < 6; i++)
		XMStoreFloat4(&mFrustumPlanes[i], XMPlaneNormalize(XMLoadFloat4(&mFrustumPlanes[i])));
}

float Camera::DistanceFromEye(float x, float y, float z)
{
	static float distance = sqrt((m_eye.x - x) * (m_eye.x - x) +
		(m_eye.y - y) * (m_eye.y - y) +
		(m_eye.z - z) * (m_eye.z - z));

	return distance;
}

float Camera::EyePlaneDepth(float x, float y, float z)
{
	XMVECTOR rel_pos = XMLoadFloat3(&XMFLOAT3(x - m_eye.x, y - m_eye.y, z - m_eye.z));
	float f_dist;
	XMStoreFloat(&f_dist, XMVector3Dot(XMLoadFloat4(&mFrustumPlanes[0]), rel_pos));
	return f_dist;
}

bool Camera::CheckPlanes(float x, float y, float z, float radius)
{
	int i;
	XMVECTOR dist;
	XMVECTOR rel_pos = XMLoadFloat3(&XMFLOAT3(x - m_eye.x, y - m_eye.y, z - m_eye.z));
	float f_dist;

	for (i = 0; i < 6; i++) // 2 to 4 only does left to right
	{
		//if (i == 1)
		//	continue;
		//XMVector3Dot
		XMStoreFloat(&f_dist, XMVector3Dot(XMLoadFloat4(&mFrustumPlanes[i]), rel_pos));
		plane_dist[i] = f_dist;

		if (f_dist > radius)
		{
			return false;
		}
	}
	return true;
}

float Camera::CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius)
{
	if (Within3DManhattanDistance(x, y, z, m_eye.x, m_eye.y, m_eye.z, view_distance + radius) < 0.0f)
		return -1.0f;

	if (CheckPlanes(x, y, z, radius) == true)
	{
		return DistanceEst(m_eye.x, m_eye.y, m_eye.z, x, y, z);
	}
	else
	{
		return -1.0f;
	}
}

float Camera::DistanceEst(float x, float y, float z, float x1, float y1, float z1)
{
	/*
	if (abs(x1 - x) > 30.0f)
	return 999.9f;

	if (abs(y1 - y) > 30.0f)
	return 999.9f;

	if (abs(z1 - z) > 30.0f)
	return 999.9f;
	*/

	float dist1 = (x1 - x) * (x1 - x) +
		(y1 - y) * (y1 - y) +
		(z1 - z) * (z1 - z);

	XMStoreFloat(&dist1, XMVectorSqrtEst(XMLoadFloat(&dist1)));

	return dist1;
}

float Camera::Within3DManhattanEyeDistance(float x, float y, float z, float distance)
{
	float dx = abs(x - m_eye.x);
	if (dx > distance) return -1.0f; // too far in x direction

	float dy = abs(y - m_eye.y);
	if (dy > distance) return -1.0f; // too far in z direction

	float dz = abs(z - m_eye.z);
	if (dz > distance) return -1.0f; // too far in y direction

	return 1.0f; // we're within the cube
}

float Camera::Within3DManhattanDistance(float x, float y, float z, float x1, float y1, float z1, float distance)
{
	float dx = abs(x - x1);
	if (dx > distance) return -1.0f; // too far in x direction

	float dz = abs(z - z1);
	if (dz > distance) return -1.0f; // too far in z direction

	float dy = abs(y - y1);
	if (dy > distance) return -1.0f; // too far in y direction

	return 1.0f; // we're within the cube
}

float Camera::Distance(float x, float y, float z, float x1, float y1, float z1)
{
	float dist1 = (x1 - x) * (x1 - x) +
		(y1 - y) * (y1 - y) +
		(z1 - z) * (z1 - z);

	XMStoreFloat(&dist1, XMVectorSqrt(XMLoadFloat(&dist1)));

	return dist1;
}

float Camera::CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius, float dist, float* full_dist)
{
	if (Within3DManhattanDistance(x, y, z, m_eye.x, m_eye.y, m_eye.z, dist) < 0.0f)
		return -1.0f;

	float dist_b = DistanceEst(m_eye.x, m_eye.y, m_eye.z, x, y, z);

	if (full_dist != nullptr)
	{
		*full_dist = dist_b;
	}
	if (CheckPlanes(x, y, z, radius) == true && dist_b < dist)
	{
		return dist_b;
	}
	else
	{
		return -1.0f;
	}
}

void Camera::UpDirection(_In_ XMFLOAT3 upDirection)
{
	m_up = upDirection;

	SetViewParams(m_eye, m_lookAt, m_up);
}

//--------------------------------------------------------------------------------------

void Camera::LookDirection(_In_ XMFLOAT3 lookDirection)
{
	XMFLOAT3 lookAt;
	lookAt.x = m_eye.x + lookDirection.x;
	lookAt.y = m_eye.y + lookDirection.y;
	lookAt.z = m_eye.z + lookDirection.z;

	SetViewParams(m_eye, lookAt, m_up);
}

void Camera::LookAtY(float look_height)
{
	m_lookAt.y += look_height;

	SetViewParams(m_eye, m_lookAt, m_up);
}

//--------------------------------------------------------------------------------------

void Camera::Eye(_In_ XMFLOAT3 eye)
{
	SetViewParams(eye, m_lookAt, m_up);
}

//--------------------------------------------------------------------------------------

void Camera::SetViewParams(
	_In_ XMFLOAT3 eye,
	_In_ XMFLOAT3 lookAt,
	_In_ XMFLOAT3 up
)
{
	m_eye.x = eye.x;
	m_eye.y = eye.y;
	m_eye.z = eye.z;
	m_lookAt = lookAt;
	m_up = up;

	// Calculate the view matrix.
	XMMATRIX view = XMMatrixLookAtRH(
		XMLoadFloat3(&m_eye),
		XMLoadFloat3(&m_lookAt),
		XMLoadFloat3(&m_up)
	);

	XMVECTOR det;
	XMMATRIX inverseView = XMMatrixInverse(&det, view);
	XMStoreFloat4x4(&m_viewMatrix, view);
	XMStoreFloat4x4(&m_inverseView, inverseView);

	// store it transposed ( need to figure out why )
	XMMATRIX transposeView = XMMatrixTranspose(view);
	XMStoreFloat4x4(&m_constantBufferData.view, transposeView);

	//XMStoreFloat4x4(&m_constantBufferData.view, transpoviewseView);

	// The axis basis vectors and camera position are stored inside the
	// position matrix in the 4 rows of the camera's world matrix.
	// To figure out the yaw/pitch of the camera, we just need the Z basis vector.
	XMFLOAT3 zBasis;
	XMStoreFloat3(&zBasis, inverseView.r[2]);

	m_cameraYawAngle = atan2f(zBasis.x, zBasis.z);

	float len = sqrtf(zBasis.z * zBasis.z + zBasis.x * zBasis.x);
	m_cameraPitchAngle = atan2f(zBasis.y, len);

	btVector3 vec_looking = btVector3(m_lookAt.x, m_lookAt.y, m_lookAt.z) - btVector3(m_eye.x, m_eye.y, m_eye.z);
	//vec_looking.setY(0.0f);
	vec_looking.normalize();
	btVector3 vec_looking_tan = vec_looking.rotate(btVector3(0.0f, 1.0f, 0.0f), -3.14159*0.5f);

	view_dir = XMFLOAT3(vec_looking.getX(), vec_looking.getY(), vec_looking.getZ());
	view_tan = XMFLOAT3(vec_looking_tan.getX(), vec_looking_tan.getY(), vec_looking_tan.getZ());
}

//--------------------------------------------------------------------------------------

void Camera::SetProjParams(
	_In_ float fieldOfView,
	_In_ float aspectRatio,
	_In_ float nearPlane,
	_In_ float farPlane
)
{
	// Set attributes for the projection matrix.
	m_fieldOfView = fieldOfView;
	m_aspectRatio = aspectRatio;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
	XMStoreFloat4x4(
		&m_projectionMatrix,
		XMMatrixPerspectiveFovRH(
			m_fieldOfView,
			m_aspectRatio,
			m_nearPlane,
			m_farPlane
		)
	);

	auto orientation = m_deviceResources->GetOrientationTransform3D();

	XMStoreFloat4x4(
		&m_projectionMatrix,
		XMMatrixMultiply(
			XMMatrixTranspose(Projection()),
			XMMatrixTranspose(XMLoadFloat4x4(&orientation))
		)
	);

	m_constantBufferData.projection = m_projectionMatrix;

	STEREO_PARAMETERS* stereoParams = nullptr;
	// Update the projection matrix.
	XMStoreFloat4x4(
		&m_projectionMatrixLeft,
		MatrixStereoProjectionFovLH(
			stereoParams,
			STEREO_CHANNEL::LEFT,
			m_fieldOfView,
			m_aspectRatio,
			m_nearPlane,
			m_farPlane,
			STEREO_MODE::NORMAL
		)
	);

	XMStoreFloat4x4(
		&m_projectionMatrixRight,
		MatrixStereoProjectionFovLH(
			stereoParams,
			STEREO_CHANNEL::RIGHT,
			m_fieldOfView,
			m_aspectRatio,
			m_nearPlane,
			m_farPlane,
			STEREO_MODE::NORMAL
		)
	);
}

//--------------------------------------------------------------------------------------

XMMATRIX Camera::View()
{
	return XMLoadFloat4x4(&m_constantBufferData.view);
}
XMMATRIX Camera::Projection()
{
	return XMLoadFloat4x4(&m_projectionMatrix);
}
XMMATRIX Camera::LeftEyeProjection()
{
	return XMLoadFloat4x4(&m_projectionMatrixLeft);
}
XMMATRIX Camera::RightEyeProjection()
{
	return XMLoadFloat4x4(&m_projectionMatrixRight);
}
XMMATRIX Camera::World()
{
	return XMLoadFloat4x4(&m_inverseView);
}
XMFLOAT3 Camera::Eye()
{
	return m_eye;
}
XMFLOAT3 Camera::LookAt()
{
	return m_lookAt;
}

void Camera::LookAt(XMFLOAT3 la)
{
	m_lookAt = la;
}

float Camera::NearClipPlane()
{
	return m_nearPlane;
}
float Camera::FarClipPlane()
{
	return m_farPlane;
}
float Camera::Pitch()
{
	return m_cameraPitchAngle;
}
float Camera::Yaw()
{
	return m_cameraYawAngle;
}
float Camera::LookingTanX()
{
	return view_tan.x;
}
float Camera::LookingTanZ()
{
	return view_tan.z;
}
XMFLOAT3 Camera::LookingDir()
{
	return view_dir;
}
float Camera::LookingX()
{
	return view_dir.x;
}
float Camera::LookingY()
{
	return view_dir.y;
}
float Camera::LookingZ()
{
	return view_dir.z;
}
float Camera::PositionX()
{
	return m_eye.x;
}

float Camera::PositionZ()
{
	return m_eye.z;
}

//--------------------------------------------------------------------------------------