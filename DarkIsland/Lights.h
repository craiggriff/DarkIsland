#pragma once

#include "DefShader.h"
#include "AllStructures.h"
#include "camera.h"
#include "DeviceResources.h"
#include "RenderCube.h"
#include "RenderCubeArray.h"
#include "RenderTextureArray.h"

namespace Game
{
	class Lights
	{
	public:
		Lights::Lights(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Camera* p_Camera);

		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		Camera* m_Camera;

		void SetLights1();

		HRESULT Lights::CreateStructuredBuffer(
			ID3D11Device*               pd3dDevice,
			const UINT                  iNumElements,
			const bool                  isCpuWritable,
			const bool                  isGpuWritable,
			ID3D11Buffer**              ppBuffer,
			ID3D11ShaderResourceView**  ppSRV,
			ID3D11UnorderedAccessView** ppUAV,
			const PointLightType*                    pInitialData = NULL);

		bool UpdatePointLightBuffer(ID3D11DeviceContext* deviceContext);
		bool UpdateSpotLightBuffer(ID3D11DeviceContext* deviceContext);

		int current_point_shadow_index;
		int current_spot_shadow_index;
		int current_shadow_cube_index;

		void FinalizeLightTexturesLoading();

		SpotShadow spot_shadow[MAX_SPOT_SHADOWS];

		ID3D11ShaderResourceView* m_LightmapTextureArray[6];

		RenderCubeArray* m_RenderCubeArray;

		RenderTextureArray* m_SpotTextureArray;

		//RenderTextureClass* m_RenderTexture[10];

		XMFLOAT4X4 spot_projection;

		XMFLOAT4X4 point_projection;

		XMFLOAT4	mFrustumPlanes[6]; 	// [0] = near, [1] = far, [2] = left, [3] = right, [4] = top, [5] = bottom

		XMFLOAT3 m_eye;

		task<void> LoadTextures();

		void ResetLights() { current_point_shadow_index = 0; current_spot_shadow_index = 0; m_lightBufferData.numPointLights = 0; m_lightBufferData.numSpotLights = 0; m_pointLights.clear(); m_spotLights.clear(); };

		XMFLOAT4X4 pointView;
		XMFLOAT4X4 spotView;

		void Lights::CreateSpotProjection(
			_In_ float minimumFieldOfView,  // the minimum horizontal or vertical field of view, in degrees
			_In_ float aspectRatio,         // the aspect ratio of the projection (width / height)
			_In_ float nearPlane,           // depth to map to 0
			_In_ float farPlane             // depth to map to 1
		);

		void CreatePointProjection(
			_In_ float minimumFieldOfView,  // the minimum horizontal or vertical field of view, in degrees
			_In_ float aspectRatio,         // the aspect ratio of the projection (width / height)
			_In_ float nearPlane,           // depth to map to 0
			_In_ float farPlane             // depth to map to 1
		);

		void AddPoint(CG_POINT_LIGHT &point_data);
		void AddSpot(CG_SPOT_LIGHT &spot_data);

		void BuildSpotFrustumPlanes();
		float DistanceEst(float x, float y, float z, float x1, float y1, float z1);
		float CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius, float dist, float* full_dist);
		float CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius, float* full_dist);
		bool CheckPlanes(float x, float y, float z, float radius);

		void buildPointFrustumPlanes();
		void setPointFrustumPlanePosition(float x, float y, float z);
		void setPointFrustumPlanePosition(int _cube_index);
		bool CheckPointPlanes(int _direction, float x, float y, float z, float radius);

		float Within3DManhattanDistance(float x, float y, float z, float x1, float y1, float z1, float distance);
		float PointCubeWithin3DManhattanDistance(float x, float y, float z, float distance);

		void SetFog(XMFLOAT4 col);

		void UpdateConstantBuffer();
		void ClearSpotCubeDepth();
		void RenderPointDepthCubSide(int _side, int _cube_index);

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_lightBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_VlightBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_pointLightBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pointLightView;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_pointLightAccessView;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_spotLightBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_spotLightView;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_spotLightAccessView;

		XMFLOAT4	mPointFrustumPlanes[6][6]; 	// [0] = near, [1] = far, [2] = left, [3] = right, [4] = top, [5] = bottom
		XMFLOAT4	mTransPointFrustumPlanes[6][6]; 	// [0] = near, [1] = far, [2] = left, [3] = right, [4] = top, [5] = bottom

		float sky_brightness;
		float sky_ambient;
		float sky_diffuse;
		float spare3;

		LightBufferType m_lightBufferData;
		std::vector<PointLightType> m_pointLights;

		std::vector<PointLightType> m_spotLights;

		//VLightBufferType m_VlightBufferData;
	};
}
