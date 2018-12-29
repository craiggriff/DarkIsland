#pragma once
#include "AllResources.h"
#include "DefParticle.h"

namespace Game
{
	class Sparticle
	{
	public:
		Sparticle(AllResources* p_Resources);
		~Sparticle(void);

		int m_maxParticles;

		bool bInstanced;

		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_point_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_point_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_point_inputLayout;

		int num_of_textures;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Texture[4];
		//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Texture2;
		//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Texture3;

		AllResources* m_Res;

		VertexPositionTexCol* m_vertices;

		std::vector<ParticleInstance> m_instances;

		int m_vertexCount, m_indexCount;

		ID3D11Buffer *m_vertexBuffer;
		ID3D11Buffer *m_instanceBuffer;

		bool InitializeBuffers(bool _bInstanced);
		bool UpdateVertecies(ID3D11DeviceContext* deviceContext);
		bool UpdateInstances(ID3D11DeviceContext* deviceContext);
		void Render();
		void GetLightAtPosition(XMFLOAT3 &pos, XMFLOAT4 &light);
	};
}
