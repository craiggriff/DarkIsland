////////////////////////////////////////////////////////////////////////////////
// Filename: skyplaneclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SKYPLANECLASS_H_
#define _SKYPLANECLASS_H_

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include "AllResources.h"

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
//#include "textureclass.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: SkyPlaneClass
////////////////////////////////////////////////////////////////////////////////

namespace Game
{
	class SkyPlaneClass
	{
	private:
		struct SkyPlaneType
		{
			float x, y, z;
			float tu, tv;
			float aa, ab; // alpha
		};

		struct VertexType
		{
			XMFLOAT3 position;
			XMFLOAT2 texture;
			XMFLOAT2 alpha;
		};

	public:
		SkyPlaneClass(AllResources* pm_Res);
		~SkyPlaneClass();

		bool Initialize();
		void Shutdown();
		void Render();
		void RenderCenter();
		void Update(float timeTotal, float timeDelta);

		int GetIndexCount();
		ID3D11ShaderResourceView* GetCloudTexture();
		ID3D11ShaderResourceView* GetPerturbTexture();

		float GetScale();
		float GetBrightness();

		task<void> LoadTextures();

	private:
		bool InitializeSkyPlane(int, float, float, float, int);
		void ShutdownSkyPlane();

		bool InitializeBuffers(ID3D11Device*, int);
		void ShutdownBuffers();
		void RenderBuffers(ID3D11DeviceContext*);

		void ReleaseTextures();

	private:
		SkyBufferType m_SkyplaneBufferData;

		SkyPlaneType* m_skyPlane;
		int m_vertexCount, m_indexCount;
		ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
		//TextureClass *m_CloudTexture, *m_PerturbTexture;
		float m_scale, m_brightness, m_translationx, m_translationz, m_translationspeedx, m_translationspeedz;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_CloudTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_PerturbTexture;

		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Res;

		float m_translationSpeed[4];
		float m_textureTranslation[4];
	};
}

#endif