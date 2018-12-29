#include "pch.h"

#include "Textures.h"

using namespace Game;
using namespace DirectX;
using namespace DX;

Textures::Textures(std::shared_ptr<DX::DeviceResources> pm_deviceResources)
{
	m_deviceResources = pm_deviceResources;
}

Textures::~Textures()
{
	// Release all textures
}

inline bool exists_test1(char* name) {
	FILE *file;
	fopen_s(&file, name, "r");

	if (file) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

void Textures::AddTextureFilename(char* filename)
{
	int i;
	bool bFound;

	char full_filename[60];
	wchar_t w_full_filename[60];

	for (i = 0; i < textures.size(); i++)
	{
		if (!strcmp(textures[i].filename, filename))
		{
			bFound = true;
			return;
		}
	}

	texture_t new_tex;// = new texture_t();

	new_tex.cpTexture;// = new Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>();

	strcpy_s(new_tex.filename, filename);

	textures.push_back(new_tex);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Textures::GetTexturePointer(char* filename)
{
	int i;
	bool bFound;

	char full_filename[60];
	wchar_t w_full_filename[60];

	for (i = 0; i < textures.size(); i++)
	{
		if (!strcmp(textures[i].filename, filename))
		{
			bFound = true;
			return textures[i].cpTexture;
		}
	}

	//size_t bytes_conv;
	//mbstowcs_s(&bytes_conv, w_full_filename, full_filename, 60);

	//CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), w_full_filename, nullptr, &textures[current].m_Texture, MAXSIZE_T);

	//textures[current].m_Texture.CopyTo()

	texture_t new_tex;// = new texture_t();

	new_tex.cpTexture;// = new Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>();

	strcpy_s(new_tex.filename, filename);

	textures.push_back(new_tex);

	return new_tex.cpTexture;
}

const wchar_t* Textures::GetWC(const char *c)
{
	size_t convertedChars = 0;
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	//mbstowcs(wc, c, cSize);
	mbstowcs_s(&convertedChars, wc, cSize, c, _TRUNCATE);

	return wc;
}

concurrency::task<void> Textures::LoadTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	for (int i = 0; i < textures.size(); i++)
	{
		texture_t tex = textures[i];

		loader->LoadTexture(ref new Platform::String(GetWC(tex.filename)), nullptr, tex.cpTexture.GetAddressOf());

		//tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(GetWC(tex.filename)), nullptr, tex.cpTexture.GetAddressOf()));

		textures[i] = tex;
	}

	return when_all(tasks.begin(), tasks.end());
}