#pragma once
#include "../DirectXTK/Inc/DDSTextureLoader.h"
#include "DeviceResources.h"

namespace Game
{
	typedef struct
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cpTexture;
		char filename[100];
	} texture_t;

	class Textures
	{
	public:
		Textures(std::shared_ptr<DX::DeviceResources> pm_deviceResources);
		~Textures();

		int current;
		bool bHighRez;

		const wchar_t* GetWC(const char *c);

		void AddTextureFilename(char* filename);

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Textures::GetTexturePointer(char* filename);

		concurrency::task<void> LoadTextures();

		std::vector<texture_t> textures;

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
	};
}
