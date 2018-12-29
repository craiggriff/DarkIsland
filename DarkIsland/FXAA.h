#pragma once

#include "DeviceResources.h"

namespace Game
{
	class FXAA
	{
	public:
		FXAA(std::shared_ptr<DX::DeviceResources> pm_deviceResources);

		void FxaaIntegrateResource();

		void CreateDeviceDependentResources();

		void Shutdown();

		// extra FXAA resource
		ID3D11Texture2D*			g_pProxyTexture = NULL;
		ID3D11ShaderResourceView*	g_pProxyTextureSRV = NULL;
		ID3D11Texture2D*			g_pCopyResolveTexture = NULL;
		ID3D11ShaderResourceView*	g_pCopyResolveTextureSRV = NULL;
		ID3D11RenderTargetView*     g_pProxyTextureRTV = NULL;

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
	};
}