#include "pch.h"
#include "FXAA.h"

using namespace Game;

FXAA::FXAA(std::shared_ptr<DX::DeviceResources> pm_deviceResources)
{
	m_deviceResources = pm_deviceResources;

	g_pProxyTexture = NULL;
	g_pProxyTextureSRV = NULL;
	g_pCopyResolveTexture = NULL;
	g_pCopyResolveTextureSRV = NULL;
	g_pProxyTextureRTV = NULL;
}

void FXAA::FxaaIntegrateResource()
{
	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();

	D3D11_TEXTURE2D_DESC desc;
	::ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	//desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	desc.Height = outputSize.Height;
	desc.Width = outputSize.Width;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MipLevels = 1;

	m_deviceResources->GetD3DDevice()->CreateTexture2D(&desc, 0, &g_pCopyResolveTexture);
	desc.SampleDesc.Count = 1;//pBackBufferSurfaceDesc->SampleDesc.Count;
	desc.SampleDesc.Quality = 0;//pBackBufferSurfaceDesc->SampleDesc.Quality;
	desc.MipLevels = 1;
	m_deviceResources->GetD3DDevice()->CreateTexture2D(&desc, 0, &g_pProxyTexture);
	m_deviceResources->GetD3DDevice()->CreateRenderTargetView(g_pProxyTexture, 0, &g_pProxyTextureRTV);
	m_deviceResources->GetD3DDevice()->CreateShaderResourceView(g_pProxyTexture, 0, &g_pProxyTextureSRV);
	m_deviceResources->GetD3DDevice()->CreateShaderResourceView(g_pCopyResolveTexture, 0, &g_pCopyResolveTextureSRV);
}

void FXAA::Shutdown()
{
	if (g_pProxyTexture != NULL)
	{
		g_pProxyTexture->Release();
		g_pProxyTexture = NULL;
	}
	if (g_pProxyTextureSRV != NULL)
	{
		g_pProxyTextureSRV->Release();
		g_pProxyTextureSRV = NULL;
	}
	if (g_pCopyResolveTexture != NULL)
	{
		g_pCopyResolveTexture->Release();
		g_pCopyResolveTexture = NULL;
	}
	if (g_pCopyResolveTextureSRV != NULL)
	{
		g_pCopyResolveTextureSRV->Release();
		g_pCopyResolveTextureSRV = NULL;
	}
	if (g_pProxyTextureRTV != NULL)
	{
		g_pProxyTextureRTV->Release();
		g_pProxyTextureRTV = NULL;
	}
}

void FXAA::CreateDeviceDependentResources()
{
}