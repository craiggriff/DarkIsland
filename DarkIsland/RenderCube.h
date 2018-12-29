#pragma once
#include "DeviceResources.h"
////////////////////////////////////////////////////////////////////////////////
// Class name: RenderTextureClass
////////////////////////////////////////////////////////////////////////////////
namespace Game
{
	class  RenderCube
	{
	public:
		RenderCube();
		~RenderCube();

		int tex_width, tex_height;

		bool Initialize(std::shared_ptr<DX::DeviceResources> pm_deviceResources, int size_width, int size_height, DXGI_FORMAT tex_format);
		void Shutdown();

		void ClearTargets();

		XMMATRIX RenderCubeSide(int _side, XMFLOAT4 target_col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

		void SetRenderPosition(float x, float y, float z) {
			render_position.x = x;
			render_position.y = y;
			render_position.z = z;
		}

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShaderResourceView();
		XMFLOAT3 render_position;

		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Members for the point light depth shadows map.
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_pointshadowMapRenderTargetView[6];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_pointshadowMapShaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_pointshadowMapDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_pointshadowMapDepthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_pointshadowMapTexture;

		Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_sampler;                    // cube texture sampler
	private:
	};
}