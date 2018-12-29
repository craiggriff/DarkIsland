#include "pch.h"
#include "AllResources.h"
#include "DefParticle.h"

#include "..\Common\DirectXHelper.h"

#include "DefGame.h"
#include "DefShader.h"

#include "ppltasks_extra.h"

#include <sqlite3.h>
#include <locale>
#include <codecvt>
#include <string>
#include <process.h>

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

using namespace Game;

template <class T> inline LONG SafeRelease(T*& pUnk) { ULONG lr = 0; if (pUnk != nullptr) { lr = pUnk->Release(); pUnk = nullptr; } return lr; }

static int get_static_models_callback(void *l_info, int columns, char **data, char **azColName) {
	int i;
	db_model_info all_info;

	std::vector<db_model_info>* vNames = static_cast<std::vector<db_model_info>*>(l_info);

	for (i = 0; i < columns; i++)
	{
		switch (i)
		{
		case 0:all_info.index = atoi(data[i]); break;
		case 1:strcpy_s(all_info.group, 40, data[i]); break;
		case 2:strcpy_s(all_info.filename, 40, data[i]); strcat_s(all_info.filename, ".cmo"); break;
		case 3:all_info.type = atoi(data[i]); break;
		case 4:all_info.physics_shape = atoi(data[i]); break;
		case 5:all_info.mass = atof(data[i]); break;
		case 6:all_info.restitution = atof(data[i]); break;
		case 7:all_info.friction = atof(data[i]); break;
		case 8:all_info.rotx = atof(data[i]); break;
		case 9:all_info.roty = atof(data[i]); break;
		case 10:all_info.rotz = atof(data[i]); break;
		case 11:all_info.scale = atof(data[i]); break;
		case 12:strcpy_s(all_info.sound_filename, 40, data[i]); break;
		}
	}

	vNames->push_back(all_info);

	return 0;
}

static int get_static_textures_callback(void *l_info, int columns, char **data, char **azColName) {
	int i;
	db_texture_info all_info;

	std::vector<db_texture_info>* vNames = static_cast<std::vector<db_texture_info>*>(l_info);

	for (i = 0; i < columns; i++)
	{
		switch (i)
		{
		case 0:all_info.index = atoi(data[i]); break;
		case 1:all_info.index = atoi(data[i]); break;
		case 2:strcpy_s(all_info.filename, 40, "Assets\\Compiled\\"); strcat_s(all_info.filename, data[i]); strcat_s(all_info.filename, ".dds"); break;
		case 3:strcpy_s(all_info.material_name, 40, data[i]); break;
		case 4:strcpy_s(all_info.type, 4, data[i]); break;
		case 5:all_info.tex_slot = atoi(data[i]); break;
		case 6:all_info.alpha = atoi(data[i]); break;
		case 7:all_info.intensity = atof(data[i]); break;
		case 8:all_info.specular_level = atof(data[i]); break;
		}
	}

	vNames->push_back(all_info);

	return 0;
}

using namespace Windows::Foundation;

using namespace Windows::System::Threading;

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	int i;
	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

XMFLOAT4X4* AllResources::ConstantModelBuffer()
{
	return &m_Camera->m_constantBufferData.model;
}

void AllResources::RenderAlphaMode(int alpha_mode)
{
	switch (alpha_mode)
	{
	case 0:
		DisableBlending();
		SetCull(true);
		SetDepthStencil(1);
		break;
	case 1:
		EnableBlendingColor();
		SetDepthStencil(0);
		SetCull(true);
		break;
	case 2:
		EnableBlendingTexAlph();
		//m_Res->EnableBlendingGlassAlph();
		SetDepthStencil(0);
		SetCull(false);
		break;
	case 3:
		EnableBlendingSkyplane();
		//m_Res->EnableBlendingGlassAlph();
		SetDepthStencil(0);
		SetCull(false);
		break;
	}
}

AllResources::AllResources(const std::shared_ptr<DX::DeviceResources>& deviceResources, Game::IGameUIControl^ UIControl, DX::StepTimer* pp_timer) :
	m_uiControl(UIControl),
	m_deviceResources(deviceResources)
{
	bLoadingComplete = false;

	p_Timer = pp_timer;

	int i = 0;

	//m_Textures = new Textures(m_deviceResources, true);

	m_Camera = new Camera(m_deviceResources);

	//m_Camera->SetViewParams(dsssss
	//	XMFLOAT3(0.0f, 0.7f, 0.0f),            // Eye point in world coordinates.
	//	XMFLOAT3(0.0f, 0.7f, 0.0f),     // Look at point in world coordinates.
	//	XMFLOAT3(0.0f, 1.0f, 0.0f)      // The Up vector for the camera.
	//);

	m_FXAA = new FXAA(m_deviceResources);

	m_Lights = new Lights(m_deviceResources, m_Camera);

	m_Textures = new Textures(m_deviceResources);

	//m_FreeCam = new FreeCamera(m_Camera, m_PadInput);

	m_FullScreenWindow = new OrthoWindowClass;
	m_DownsampleWindow = new OrthoWindowClass;

	m_DeferredBuffers = new DeferredBuffersClass;

	m_RDeferredBuffers = new DeferredBuffersClass;

	bAudioInitialized = false;

	int bDevMode = DEV_MODE;

	view_distance = 100.0f;

	if (bDevMode == 1)
	{
		//		m_Camera->SetFarDist(600.0f);
		bContentFolder = false;
		bDevelopment = true;
		bFreeCamera = true;
	}
	else
	{
		//		m_Camera->SetFarDist(MAX_VIEW_DISTANCE);
		bContentFolder = true;
		bDevelopment = false;
		bFreeCamera = false;
	}

	delete_radius = 1;

	//Windows::System::Launcher::LaunchFolderAsync(Windows::Storage::ApplicationData::Current->LocalFolder);

	if (ENABLE_EDIT_MODE == true)
	{
		size_t bytes_conv;
		Windows::Storage::StorageFolder^ folder = Windows::Storage::ApplicationData::Current->LocalFolder;
		wcstombs_s(&bytes_conv, local_file_folder, folder->Path->Data(), sizeof(local_file_folder));

		char local_data[500];
		char remote_data[500];

		sprintf_s(remote_data, "%s\\game-data.db", local_file_folder);
		sprintf_s(local_data, "Assets\\game-data.db");

		CopyFile2(GetWC(local_data), GetWC(remote_data), false);

		wchar_t info_folder[140];
		swprintf_s(info_folder, L"%s\\LevelBinary", Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data());
		CreateDirectory(info_folder, NULL);

		wchar_t file[500];
		swprintf_s(file, L"copy Assets\\LevelBinary\\*.* %s\\LevelBinary\\*.*", Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data());

		char system_str[500];
		int dest_len = 400;

		if (true)
		{
			to_narrow(file, system_str, dest_len);

			system(system_str);
		}

		LoadDatabaseModelInfo(&model_info, "[Index] > 0");

		std::vector<db_model_info> unique_model_types;

		for (db_model_info mi : model_info)
		{
			bool bFound = false;
			for (db_model_info mt : unique_model_types)
			{
				if (mt.type == mi.type)
				{
					bFound = true;
				}
			}
			if (bFound == false)
			{
				unique_model_types.push_back(mi);
			}
		}

		m_uiControl->SetEditFilterItems();
	}

	//WinExec("your cmd command", SW_HIDE);
	//system(system_str);

	/*
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(file, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			sprintf_s(remote_data, "%s\\LevelBinary\\%s.bmp", local_file_folder, FindFileData.cFileName);
			sprintf_s(local_data, "Assets\\LevelBinary\\%s.bmp", FindFileData.cFileName);

			CopyFile2(GetWC(local_data), GetWC(remote_data), false);

			//wcout << FindFileData.cFileName << "\n";
			i++;
		} while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
	*/

	m_audio.Initialize();
	m_audio.CreateResources();

	/*
	m_audio.Initialize();
	m_audio.CreateResources();
	m_audio.SetTrack(2);
	m_audio.SetMusicVolume(1.0f);
	m_audio.Start();

	m_audio.SetSoundEffectVolume(SFXThunder, 0.6f);
	m_audio.SetSoundEffectPitch(SFXThunder, 3.0f);

	//m_audio.SetRoomSize(10.0f, 10.0f);
	m_audio.PlaySoundEffect(SFXThunder);
	*/

	//
	//m_LightmapTextureArray[0] = m_Textures->LoadTexture("headlamp_lm").Get();
	//m_LightmapTextureArray[1] = m_Textures->LoadTexture("mushroomred").Get();

	col_r = 128;
	col_g = 128;
	col_b = 128;

	//CreateDatabase();
	//LoadDatabaseLevelInfo(1);
}

void AllResources::SetDrawDistance(float dist)
{
	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	view_distance = dist;
	m_Camera->view_distance = dist;

	m_Camera->SetProjParams(
		XM_PI / 2,
		outputSize.Width / outputSize.Height,
		0.01f,
		dist
	);
	m_Camera->close_projection = m_Camera->m_projectionMatrix;
}

XMFLOAT4X4 AllResources::GetMatrix(btTransform* p_body)
{
	XMFLOAT4X4 m_Matrix;

	btTransform trans;

	//trans = p_body->getWorldTransform();

	btQuaternion q_rotation = p_body->getRotation();

	XMVECTOR data;
	XMVECTORF32 floatingVector = { q_rotation.getX(), q_rotation.getY(), q_rotation.getZ(), q_rotation.getW() };
	data = floatingVector;

	auto rotationMatrix = XMMatrixRotationQuaternion(data);

	auto translationMatrix = XMMatrixTranslation(p_body->getOrigin().getX(), p_body->getOrigin().getY(), p_body->getOrigin().getZ());

	XMStoreFloat4x4(&m_Matrix, XMMatrixTranspose(rotationMatrix * translationMatrix));

	return m_Matrix;
}

XMFLOAT4X4 AllResources::GetMatrix(btRigidBody* p_body)
{
	XMFLOAT4X4 m_Matrix;

	btTransform trans;

	trans = p_body->getWorldTransform();

	btQuaternion q_rotation = trans.getRotation();

	XMVECTOR data;
	XMVECTORF32 floatingVector = { q_rotation.getX(), q_rotation.getY(), q_rotation.getZ(), q_rotation.getW() };
	data = floatingVector;

	auto rotationMatrix = XMMatrixRotationQuaternion(data);

	auto translationMatrix = XMMatrixTranslation(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());

	XMStoreFloat4x4(&m_Matrix, XMMatrixTranspose(rotationMatrix * translationMatrix));

	return m_Matrix;
}

size_t AllResources::to_narrow(const wchar_t * src, char * dest, size_t dest_len)
{
	size_t i;
	wchar_t code;

	i = 0;

	while (src[i] != '\0' && i < (dest_len - 1)) {
		code = src[i];
		if (code < 128)
			dest[i] = char(code);
		else {
			dest[i] = '?';
			if (code >= 0xD800 && code <= 0xD8FF)
				// lead surrogate, skip the next code unit, which is the trail
				i++;
		}
		i++;
	}

	dest[i] = '\0';

	return i - 1;
}

void AllResources::LoadDatabaseModelInfo(std::vector<db_model_info>* minfo, char* where_str)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql[4000];

	char info_filename[180];

	//sprintf_s(info_filename, "%s\\game-data.db", local_file_folder);
	sprintf_s(info_filename, "Assets\\game-data.db", local_file_folder);

	rc = sqlite3_open_v2(info_filename, &db, SQLITE_OPEN_READONLY, NULL);

	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else {
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf_s(sql, 4000, "SELECT * from [STATIC_MODELS] where %s", where_str);
	rc = sqlite3_exec(db, sql, get_static_models_callback, static_cast<void*>(minfo), &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Table created successfully\n");
	}

	sqlite3_close(db);
}

void AllResources::LoadDatabaseAllTextureFilenames()
{
	int i;
	std::vector<db_texture_info> texture_info;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql[4000];

	char info_filename[180];

	sprintf_s(info_filename, "%s\\game-data.db", local_file_folder);

	sprintf_s(info_filename, "Assets\\game-data.db", local_file_folder);

	rc = sqlite3_open_v2(info_filename, &db, SQLITE_OPEN_READONLY, NULL);

	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else {
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf_s(sql, 4000, "SELECT * from STATIC_TEXTURES where [model_index] > -1");

	rc = sqlite3_exec(db, sql, get_static_textures_callback, static_cast<void*>(&texture_info), &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Table created successfully\n");
	}

	sqlite3_close(db);

	for (i = 0; i < texture_info.size(); i++)
	{
		m_Textures->AddTextureFilename(texture_info[i].filename);
	}
}

void AllResources::LoadDatabaseTextureInfo(std::vector<db_texture_info>* tinfo, char* where_str)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql[4000];

	char info_filename[180];

	//sprintf_s(info_filename, "%s\\game-data.db", local_file_folder);
	sprintf_s(info_filename, "Assets\\game-data.db", local_file_folder);

	rc = sqlite3_open_v2(info_filename, &db, SQLITE_OPEN_READONLY, NULL);

	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else {
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf_s(sql, 4000, "SELECT * from STATIC_TEXTURES where %s", where_str);

	rc = sqlite3_exec(db, sql, get_static_textures_callback, static_cast<void*>(tinfo), &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Table created successfully\n");
	}

	sqlite3_close(db);
}

char* AllResources::to_narrow_str(const wchar_t* src)
{
	static char dest[1000];

	int name_len = 0;
	to_narrow(src, dest, name_len);

	return dest;
}

const wchar_t* AllResources::GetWC(const char *c)
{
	size_t convertedChars = 0;
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	//mbstowcs(wc, c, cSize);
	mbstowcs_s(&convertedChars, wc, cSize, c, _TRUNCATE);

	return wc;
}

task<ID3D11ShaderResourceView*> AllResources::GetOrCreateTextureAsync(const std::wstring& textureName)
{
	auto iter = m_textureResources.find(textureName);
	if (iter != m_textureResources.end())
	{
		return extras::create_value_task(iter->second.Get());
	}
	else
	{
		std::vector<BYTE> ddsBuffer;
		return DX::ReadDataAsync(textureName).then([this, textureName](const std::vector<byte>& ddsBuffer) -> ID3D11ShaderResourceView*
		{
			if (ddsBuffer.size() > 0)
			{
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> result(this->CreateTextureFromDDSInMemory(&ddsBuffer[0], ddsBuffer.size()));
				if (result == nullptr)
				{
					//throw std::exception("Texture could not be created");
					return nullptr;
				}
				else
				{
					m_textureResources[textureName] = result;

					return result.Get();
				}
			}
			else
			{
				return nullptr;
			}
		});
	}
}

ID3D11ShaderResourceView* AllResources::CreateTextureFromDDSInMemory(const BYTE* ddsData, size_t ddsDataSize)
{
	ID3D11ShaderResourceView* textureView = nullptr;

	if (ddsData != nullptr && ddsDataSize > 0)
	{
		HRESULT hr = CreateDDSTextureFromMemory(m_deviceResources->GetD3DDevice(), ddsData, ddsDataSize, nullptr, &textureView);

		if (FAILED(hr))
		{
			SafeRelease(textureView);
		}
		else
		{
			return textureView;
		}
	}

	return nullptr;
}

Platform::String^ AllResources::stringToPlatformString(std::wstring inputString) {
	return ref new Platform::String(inputString.c_str());
}

Platform::String^ AllResources::StringFromAscIIChars(char* chars)
{
	size_t newsize = strlen(chars) + 1;
	wchar_t * wcstring = new wchar_t[newsize];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring, newsize, chars, _TRUNCATE);
	Platform::String^ str = ref new Platform::String(wcstring);
	delete[] wcstring;
	return str;
}

static int get_callback(void *l_info, int columns, char **data, char **azColName) {
	int i;
	LevelInfo all_info;

	std::vector<LevelInfo>* vNames = static_cast<std::vector<LevelInfo>*>(l_info);

	for (i = 0; i < columns; i++)
	{
		switch (i)
		{
		case 0:all_info.db_rec_id = atoi(data[i]); break;
		case 1:strcpy_s(all_info.terrain_tex1, 40, data[i]); break;
		case 2:strcpy_s(all_info.terrain_tex2, 40, data[i]); break;
		case 3:strcpy_s(all_info.terrain_tex3, 40, data[i]); break;
		case 4:strcpy_s(all_info.terrain_tex4, 40, data[i]); break;
		case 5:strcpy_s(all_info.terrain_tex5, 40, data[i]); break;
		case 6:strcpy_s(all_info.terrain_tex6, 40, data[i]); break;
		case 7:strcpy_s(all_info.terrain_tex7, 40, data[i]); break;
		case 8:strcpy_s(all_info.terrain_tex8, 40, data[i]); break;
		case 9:break;
		case 10:break;
		case 11:strcpy_s(all_info.terrain_normal1, 40, data[i]); break;
		case 12:strcpy_s(all_info.terrain_normal2, 40, data[i]); break;
		case 13:strcpy_s(all_info.terrain_normal3, 40, data[i]); break;
		case 14:strcpy_s(all_info.terrain_normal4, 40, data[i]); break;
		case 15:strcpy_s(all_info.terrain_normal5, 40, data[i]); break;
		case 16:strcpy_s(all_info.terrain_normal6, 40, data[i]); break;
		case 17:strcpy_s(all_info.terrain_normal7, 40, data[i]); break;
		case 18:strcpy_s(all_info.terrain_normal8, 40, data[i]); break;
		case 19:break;
		case 20:break;
		case 21:all_info.ground_steepness_blend = atof(data[i]); break;
		case 22:strcpy_s(all_info.skybox, 40, data[i]); break;
		case 23:strcpy_s(all_info.music_track, 40, data[i]); break;

		case 24:all_info.ambient_col.x = atof(data[i]); break;
		case 25:all_info.ambient_col.y = atof(data[i]); break;
		case 26:all_info.ambient_col.z = atof(data[i]); break;
		case 27:all_info.ambient_col.w = atof(data[i]); break;

		case 28:all_info.diffuse_col.x = atof(data[i]); break;
		case 29:all_info.diffuse_col.y = atof(data[i]); break;
		case 30:all_info.diffuse_col.z = atof(data[i]); break;
		case 31:all_info.diffuse_col.w = atof(data[i]); break;

		case 32:all_info.specular_col.x = atof(data[i]); break;
		case 33:all_info.specular_col.y = atof(data[i]); break;
		case 34:all_info.specular_col.z = atof(data[i]); break;
		case 35:all_info.specular_col.w = atof(data[i]); break;

		case 36:all_info.fog_col.x = atof(data[i]); break;
		case 37:all_info.fog_col.y = atof(data[i]); break;
		case 38:all_info.fog_col.z = atof(data[i]); break;
		case 39:all_info.fog_col.w = atof(data[i]); break;

		case 40:all_info.dust_col.x = atof(data[i]); break;
		case 41:all_info.dust_col.y = atof(data[i]); break;
		case 42:all_info.dust_col.z = atof(data[i]); break;
		case 43:all_info.dust_col.w = atof(data[i]); break;

		case 44:all_info.diff_dir.x = atof(data[i]); break;
		case 45:all_info.diff_dir.y = atof(data[i]); break;
		case 46:all_info.diff_dir.z = atof(data[i]); break;

		case 47:all_info.specular_power = atof(data[i]); break;
		case 48:all_info.ground_friction = atof(data[i]); break;
		case 49:all_info.ground_restitution = atof(data[i]); break;

		case 50:all_info.player_start_x = atof(data[i]); break;
		case 51:all_info.player_start_z = atof(data[i]); break;
		case 52:all_info.player_start_angle = atof(data[i]); break;

		case 53:all_info.bRain = atoi(data[i]); break;
		case 54:all_info.bSnow = atoi(data[i]); break;
		case 55:all_info.bFog = atoi(data[i]); break;

		case 56:all_info.wind.x = atof(data[i]); break;
		case 57:all_info.wind.y = atof(data[i]); break;
		case 58:all_info.wind.z = atof(data[i]); break;
		case 59:all_info.wind.w = atof(data[i]); break;

		case 60:all_info.level = atoi(data[i]); break;
		case 61:all_info.spot_distance = data[i] == nullptr ? SPOT_LIGHT_RADIUS : atof(data[i]); break;
		case 62:all_info.point_distance = data[i] == nullptr ? POINT_LIGHT_RADIUS : atof(data[i]); break;
		case 63:all_info.sky_brightness = atof(data[i]); break;
		case 64:all_info.sky_ambient = atof(data[i]); break;
		case 65:all_info.sky_diffuse = atof(data[i]); break;

		case 66:all_info.start_sound = atoi(data[i]); break;
		case 67:all_info.start_music = atoi(data[i]); break;
		case 68:all_info.player_start_y = atof(data[i]); break;
			//case 61:all_info.spot_distance = atof(data[i]); break;
			//case 62:all_info.point_distance = atof(data[i]); break;
		}

		//if (!strcmp(azColName[i], "GND1"))
		//	strcpy_s(all_info.terrain_tex1, 40, data[i]);
	}

	vNames->push_back(all_info);

	return 0;
}

void AllResources::SaveDatabaseLevelInfo()
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql[4000];

	char info_filename[180];

	sprintf_s(info_filename, "%s\\game-data.db", local_file_folder);
	//sprintf_s(info_filename, "G:\\Desktop\\SolidGame\\Assets\\game-data.db", local_file_folder);

	rc = sqlite3_open(info_filename, &db);

	if (rc != SQLITE_OK) {
		//exit(1);
		//CreateDatabase();
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		//return(0);
	}
	else {
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf_s(sql, 4000, "UPDATE LEVELINFO set "
		"GND1='%s',GND2='%s',GND3='%s',GND4='%s',GND5='%s',GND6='%s',GND7='%s',GND8='%s',GNDN1='%s',GNDN2='%s',GNDN3='%s',GNDN4='%s',GNDN5='%s',GNDN6='%s',GNDN7='%s',GNDN8='%s', "
		"STEEPNESS=%f,SKYBOX='%s',MUSICTRACK='%s',"
		"AMBIENTX=%f,AMBIENTY=%f,AMBIENTZ=%f,AMBIENTW=%f,"
		"DIFFUSEX=%f,DIFFUSEY=%f,DIFFUSEZ=%f,DIFFUSEW=%f,"
		"SPECULARX=%f,SPECULARY=%f,SPECULARZ=%f,SPECULARW=%f,"
		"FOGX=%f,FOGY=%f,FOGZ=%f,FOGW=%f,"
		"DUSTX=%f,DUSTY=%f,DUSTZ=%f,DUSTW=%f,"
		"DIFFDIRX=%f,DIFFDIRY=%f,DIFFDIRZ=%f,"
		"SPECULARPOW=%f,GROUNDFRICTION=%f,GROUNDRESTITUTION=%f,"
		"STARTX=%f,STARTY=%f,STARTZ=%f,STARTANGLE=%f,"
		"ISRAIN=%d,ISSNOW=%d,ISFOG=%d,WINDX=%f,WINDY=%f,WINDZ=%f,WINDW=%f, SPOT_DISTANCE=%f,POINT_DISTANCE=%f,SKY_BRIGHTNESS=%f,SKY_AMBIENT=%f,SKY_DIFFUSE=%f where LEVELNUM=%d ;",

		m_LevelInfo.terrain_tex1, m_LevelInfo.terrain_tex2, m_LevelInfo.terrain_tex3, m_LevelInfo.terrain_tex4, m_LevelInfo.terrain_tex5, m_LevelInfo.terrain_tex6, m_LevelInfo.terrain_tex7, m_LevelInfo.terrain_tex8,

		m_LevelInfo.terrain_normal1, m_LevelInfo.terrain_normal2, m_LevelInfo.terrain_normal3, m_LevelInfo.terrain_normal4, m_LevelInfo.terrain_normal5, m_LevelInfo.terrain_normal6, m_LevelInfo.terrain_normal7, m_LevelInfo.terrain_normal8,

		m_LevelInfo.ground_steepness_blend, m_LevelInfo.skybox, m_LevelInfo.music_track,

		m_LevelInfo.ambient_col.x, m_LevelInfo.ambient_col.y, m_LevelInfo.ambient_col.z, m_LevelInfo.ambient_col.w,
		m_LevelInfo.diffuse_col.x, m_LevelInfo.diffuse_col.y, m_LevelInfo.diffuse_col.z, m_LevelInfo.diffuse_col.w,
		m_LevelInfo.specular_col.x, m_LevelInfo.specular_col.y, m_LevelInfo.specular_col.z, m_LevelInfo.specular_col.w,
		m_LevelInfo.fog_col.x, m_LevelInfo.fog_col.y, m_LevelInfo.fog_col.z, m_LevelInfo.fog_col.w,
		m_LevelInfo.dust_col.x, m_LevelInfo.dust_col.y, m_LevelInfo.dust_col.z, m_LevelInfo.dust_col.w,

		m_LevelInfo.diff_dir.x, m_LevelInfo.diff_dir.y, m_LevelInfo.diff_dir.z,

		m_LevelInfo.specular_power, m_LevelInfo.ground_friction, m_LevelInfo.ground_restitution,

		m_LevelInfo.player_start_x, m_LevelInfo.player_start_y, m_LevelInfo.player_start_z, m_LevelInfo.player_start_angle,

		m_LevelInfo.bRain, m_LevelInfo.bSnow, m_LevelInfo.bFog,

		m_LevelInfo.wind.x, m_LevelInfo.wind.y, m_LevelInfo.wind.z, m_LevelInfo.wind.w,

		m_LevelInfo.spot_distance,
		m_LevelInfo.point_distance,

		m_Lights->sky_brightness,
		m_Lights->sky_ambient,
		m_Lights->sky_diffuse,
		m_LevelInfo.level

	);

	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Table created successfully\n");
	}

	sqlite3_close(db);
}

void AllResources::LoadDatabaseLevelInfo(int level)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql[4000];

	char info_filename[180];

	if (false)// DEV_MODE == 1)
	{
		sprintf_s(info_filename, "%s\\game-data.db", local_file_folder);
	}
	else
	{
		sprintf_s(info_filename, "Assets\\game-data.db", local_file_folder);
	}

	//sprintf_s(info_filename, "c:\\comm\\game-data.db");

	rc = sqlite3_open_v2(info_filename, &db, SQLITE_OPEN_READONLY, NULL);
	//rc = sqlite3_open(info_filename, &db);

	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else {
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf_s(sql, 4000, "SELECT * from LEVELINFO where LEVELNUM=%d", level);

	std::vector<LevelInfo> l_info;

	rc = sqlite3_exec(db, sql, get_callback, static_cast<void*>(&l_info), &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Table created successfully\n");
	}

	int numb = l_info.size();

	sqlite3_close(db);

	if (numb > 0)
	{
		m_LevelInfo = l_info.at(0);
	}

	if (m_LevelInfo.bSnow > 0)
		m_LevelInfo.bSnow = 1;
	if (m_LevelInfo.bRain > 0)
		m_LevelInfo.bRain = 1;
	if (m_LevelInfo.bFog > 0)
		m_LevelInfo.bFog = 1;

	m_LevelInfo.flag_angle = atan2f(m_LevelInfo.wind.x, m_LevelInfo.wind.z) - M_PI * 0.27f;

	SetLighting();
}

void AllResources::SetLighting()
{
	m_Lights->m_lightBufferData.ambientColor = m_LevelInfo.ambient_col;
	m_Lights->m_lightBufferData.diffuseColor = m_LevelInfo.diffuse_col;
	m_Lights->m_lightBufferData.specularColor = m_LevelInfo.specular_col;
	m_Lights->m_lightBufferData.fogColor = m_LevelInfo.fog_col;

	btVector3 vec_temp = btVector3(m_LevelInfo.diff_dir.x, m_LevelInfo.diff_dir.y, m_LevelInfo.diff_dir.z);
	vec_temp.normalize();
	m_Lights->m_lightBufferData.lightDirection = XMFLOAT3(vec_temp.getX(), vec_temp.getY(), vec_temp.getZ());

	m_Lights->m_lightBufferData.specularPower = m_LevelInfo.specular_power;

	m_Lights->sky_brightness = m_LevelInfo.sky_brightness;
	m_Lights->sky_ambient = m_LevelInfo.sky_ambient;
	m_Lights->sky_diffuse = m_LevelInfo.sky_diffuse;

	m_uiControl->SetCols(m_LevelInfo.ambient_col.x, m_LevelInfo.ambient_col.y, m_LevelInfo.ambient_col.z,
		m_LevelInfo.diffuse_col.x, m_LevelInfo.diffuse_col.y, m_LevelInfo.diffuse_col.z,
		m_LevelInfo.diff_dir.x, m_LevelInfo.diff_dir.y, m_LevelInfo.diff_dir.z,
		m_LevelInfo.specular_col.x, m_LevelInfo.specular_col.y, m_LevelInfo.specular_col.z,
		m_LevelInfo.specular_power, m_LevelInfo.bSnow, m_LevelInfo.bRain, m_LevelInfo.bFog, m_LevelInfo.sky_ambient, m_LevelInfo.sky_diffuse, m_LevelInfo.sky_brightness);
}

void AllResources::SaveLighting()
{
	m_LevelInfo.ambient_col = m_Lights->m_lightBufferData.ambientColor;
	m_LevelInfo.diffuse_col = m_Lights->m_lightBufferData.diffuseColor;
	m_LevelInfo.specular_col = m_Lights->m_lightBufferData.specularColor;
	m_LevelInfo.fog_col = m_Lights->m_lightBufferData.fogColor;

	m_LevelInfo.diff_dir = m_Lights->m_lightBufferData.lightDirection;

	m_LevelInfo.specular_power = m_Lights->m_lightBufferData.specularPower;
}

void AllResources::CreateDatabase()
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql[4000];

	char info_filename[180];

	//sprintf_s(info_filename, "Assets\\game-data.db", local_file_folder);
	sprintf_s(info_filename, "%s\\game-data.db", local_file_folder);

	rc = sqlite3_open(info_filename, &db);

	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		//return(0);
	}
	else {
		fprintf(stderr, "Opened database successfully\n");
	}

	for (int i = 0; i < 10; i++)
	{
		//LoadLevelInfo(i + 1);

		/* Create SQL statement */
		//sql = "INSERT INTO LEVELINFO (ID,GND1,GND2,GND3,GND4,GND5,GND6,GND7,GND8,GND9,GND10,GNDN1,GNDN2,GNDN3,GNDN4,GNDN5,GNDN6,GNDN7,GNDN8,GNDN9,GNDN10,STEEPNESS,SKYBOX,MUSICTRACK,AMBIENTX,AMBIENTY,AMBIENTZ,AMBIENTW,DIFFUSEX,DIFFUSEY,DIFFUSEZ,DIFFUSEW,SPECULARX,SPECULARY,SPECULARZ,SPECULARW,FOGX,FOGY,FOGZ,FOGW,DUSTX,DUSTY,DUSTZ,DUSTW,DIFFDIRX,DIFFDIRY,DIFFDIRZ,SPECULARPOW,GROUNDFRICTION,GROUNDRESTITUTION,STARTX,STARTZ,STARTANGLE,ISRAIN,ISSNOW,ISFOG,WINDX,WINDY,WINDZ,WINDW) "  \
				//	"VALUES (1, 'Paul', 32, 'California', 20000.00 );";
		sprintf_s(sql, 4000, "INSERT INTO LEVELINFO (LEVELNUM, GND1,GND2,GND3,GND4,GND5,GND6,GND7,GND8,GNDN1,GNDN2,GNDN3,GNDN4,GNDN5,GNDN6,GNDN7,GNDN8, "
			"STEEPNESS,SKYBOX,MUSICTRACK,"
			"AMBIENTX,AMBIENTY,AMBIENTZ,AMBIENTW,"
			"DIFFUSEX,DIFFUSEY,DIFFUSEZ,DIFFUSEW,"
			"SPECULARX,SPECULARY,SPECULARZ,SPECULARW,"
			"FOGX,FOGY,FOGZ,FOGW,"
			"DUSTX,DUSTY,DUSTZ,DUSTW,"
			"DIFFDIRX,DIFFDIRY,DIFFDIRZ,"
			"SPECULARPOW,GROUNDFRICTION,GROUNDRESTITUTION,"
			"STARTX,STARTZ,STARTANGLE,"
			"ISRAIN,ISSNOW,ISFOG,WINDX,WINDY,WINDZ,WINDW) "
			"VALUES ( %d,"
			"'%s', '%s', '%s', '%s',"
			"'%s', '%s', '%s', '%s',"
			"'%s', '%s', '%s', '%s',"
			"'%s', '%s', '%s', '%s',"
			"%f,'%s','%s',"
			"%f,%f,%f,%f,"
			"%f,%f,%f,%f,"
			"%f,%f,%f,%f,"
			"%f,%f,%f,%f,"
			"%f,%f,%f,%f,"
			"%f,%f,%f,"
			"%f,%f,%f,"
			"%f,%f,%f,"
			"%d,%d,%d,"
			"%f,%f,%f,%f,%f,%f,%f);",
			i + 1,
			m_LevelInfo.terrain_tex1, m_LevelInfo.terrain_tex2, m_LevelInfo.terrain_tex3, m_LevelInfo.terrain_tex4, m_LevelInfo.terrain_tex5, m_LevelInfo.terrain_tex6, m_LevelInfo.terrain_tex7, m_LevelInfo.terrain_tex8,

			m_LevelInfo.terrain_normal1, m_LevelInfo.terrain_normal2, m_LevelInfo.terrain_normal3, m_LevelInfo.terrain_normal4, m_LevelInfo.terrain_normal5, m_LevelInfo.terrain_normal6, m_LevelInfo.terrain_normal7, m_LevelInfo.terrain_normal8,

			m_LevelInfo.ground_steepness_blend, m_LevelInfo.skybox, m_LevelInfo.music_track,

			m_LevelInfo.ambient_col.x, m_LevelInfo.ambient_col.y, m_LevelInfo.ambient_col.z, m_LevelInfo.ambient_col.w,
			m_LevelInfo.diffuse_col.x, m_LevelInfo.diffuse_col.y, m_LevelInfo.diffuse_col.z, m_LevelInfo.diffuse_col.w,
			m_LevelInfo.specular_col.x, m_LevelInfo.specular_col.y, m_LevelInfo.specular_col.z, m_LevelInfo.specular_col.w,
			m_LevelInfo.fog_col.x, m_LevelInfo.fog_col.y, m_LevelInfo.fog_col.z, m_LevelInfo.fog_col.w,
			m_LevelInfo.dust_col.x, m_LevelInfo.dust_col.y, m_LevelInfo.dust_col.z, m_LevelInfo.dust_col.w,

			m_LevelInfo.diff_dir.x, m_LevelInfo.diff_dir.y, m_LevelInfo.diff_dir.z,

			m_LevelInfo.specular_power, m_LevelInfo.ground_friction, m_LevelInfo.ground_restitution,

			m_LevelInfo.player_start_x, m_LevelInfo.player_start_z, m_LevelInfo.player_start_angle,

			m_LevelInfo.bRain, m_LevelInfo.bSnow, m_LevelInfo.bFog,

			m_LevelInfo.wind.x, m_LevelInfo.wind.y, m_LevelInfo.wind.z, m_LevelInfo.wind.w,
			m_LevelInfo.sky_brightness, m_LevelInfo.sky_ambient, m_LevelInfo.sky_diffuse
		);

		rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		else {
			fprintf(stdout, "Table created successfully\n");
		}
	}

	sqlite3_close(db);
}

void AllResources::SetQuadIndexBuffer()
{
	m_deviceResources->GetD3DDeviceContext()->IASetIndexBuffer(m_quadindexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
}

void AllResources::CreateQuadIndexBuffer()
{
	m_quadindicesCount = MAX_ANY_PARTICLES * 6;
	// Create all indices
	int index = 0;
	//m_quadindices.clear();

	m_quadindices.resize(m_quadindicesCount);
	for (int i = 0; i < MAX_ANY_PARTICLES * 6; i += 6)
	{
		m_quadindices[i] = index;
		m_quadindices[i + 1] = index + 1;
		m_quadindices[i + 2] = index + 2;
		m_quadindices[i + 3] = index;
		m_quadindices[i + 4] = index + 3;
		m_quadindices[i + 5] = index + 1;

		/*
		m_quadindices.push_back(index);
		m_quadindices.push_back(index + 1);
		m_quadindices.push_back(index + 2);
		m_quadindices.push_back(index);
		m_quadindices.push_back(index + 3);
		m_quadindices.push_back(index + 1);
		*/
		index += 4;
	}

	D3D11_SUBRESOURCE_DATA bufferData = { m_quadindices.data(), 0, 0 };
	UINT bytes = sizeof(unsigned short) * m_quadindicesCount;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_INDEX_BUFFER);
	m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_quadindexBuffer);
}

void AllResources::SaveLevelInfo(int level)
{
	char info_filename[40];

	current_level = level;

	sprintf_s(info_filename, "%s\\LevelInfo\\%d.txt", local_file_folder, level);

	//sprintf(info_filename, "Assets\\LevelInfo\\%d.txt", level);

	fopen_s(&pFile, info_filename, "w");

	fprintf(pFile, "%s\n", m_LevelInfo.terrain_tex1);
	fprintf(pFile, "%s\n", m_LevelInfo.terrain_tex2);
	fprintf(pFile, "%s\n", m_LevelInfo.terrain_tex3);
	fprintf(pFile, "%s\n", m_LevelInfo.terrain_normal1);
	fprintf(pFile, "%s\n", m_LevelInfo.terrain_normal2);
	fprintf(pFile, "%s\n", m_LevelInfo.terrain_normal3);

	fprintf(pFile, "%f\n", m_LevelInfo.ground_steepness_blend);

	fprintf(pFile, "%s\n", m_LevelInfo.skybox);
	fprintf(pFile, "%f %f %f %f\n", m_LevelInfo.ambient_col.x
		, m_LevelInfo.ambient_col.y
		, m_LevelInfo.ambient_col.z
		, m_LevelInfo.ambient_col.w);
	fprintf(pFile, "%f %f %f %f\n", m_LevelInfo.diffuse_col.x
		, m_LevelInfo.diffuse_col.y
		, m_LevelInfo.diffuse_col.z
		, m_LevelInfo.diffuse_col.w);
	fprintf(pFile, "%f %f %f %f\n", m_LevelInfo.specular_col.x
		, m_LevelInfo.specular_col.y
		, m_LevelInfo.specular_col.z
		, m_LevelInfo.specular_col.w);
	fprintf(pFile, "%f %f %f %f\n", m_LevelInfo.fog_col.x
		, m_LevelInfo.fog_col.y
		, m_LevelInfo.fog_col.z
		, m_LevelInfo.fog_col.w);

	fprintf(pFile, "%f %f %f %f\n", m_LevelInfo.dust_col.x
		, m_LevelInfo.dust_col.y
		, m_LevelInfo.dust_col.z
		, m_LevelInfo.dust_col.w);

	fprintf(pFile, "%f %f %f\n", m_LevelInfo.diff_dir.x
		, m_LevelInfo.diff_dir.y
		, m_LevelInfo.diff_dir.z);

	fprintf(pFile, "%f\n", m_LevelInfo.specular_power);

	fprintf(pFile, "%f\n", m_LevelInfo.ground_friction);
	fprintf(pFile, "%f\n", m_LevelInfo.ground_restitution);

	fprintf(pFile, "%f\n", m_LevelInfo.player_start_x);
	fprintf(pFile, "%f\n", m_LevelInfo.player_start_z);
	fprintf(pFile, "%f\n", m_LevelInfo.player_start_angle);

	fprintf(pFile, "%d\n", m_LevelInfo.bRain);
	fprintf(pFile, "%d\n", m_LevelInfo.bSnow);

	fprintf(pFile, "%f %f %f\n", m_LevelInfo.wind.x
		, m_LevelInfo.wind.z
		, m_LevelInfo.wind.y);

	fclose(pFile);
}

void AllResources::LoadLevelInfo(int level)
{
	char info_filename[40];

	current_level = level;

	sprintf_s(info_filename, "Assets\\LevelInfo\\%d.txt", level);

	//FILE* pFile;

	fopen_s(&pFile, info_filename, "rt");

	//rewind(pFile);

	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_tex1, sizeof(m_LevelInfo.terrain_tex1));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_tex2, sizeof(m_LevelInfo.terrain_tex2));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_tex3, sizeof(m_LevelInfo.terrain_tex3));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_tex4, sizeof(m_LevelInfo.terrain_tex4));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_tex5, sizeof(m_LevelInfo.terrain_tex5));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_tex6, sizeof(m_LevelInfo.terrain_tex6));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_tex7, sizeof(m_LevelInfo.terrain_tex7));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_tex8, sizeof(m_LevelInfo.terrain_tex8));

	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_normal1, sizeof(m_LevelInfo.terrain_normal1));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_normal2, sizeof(m_LevelInfo.terrain_normal2));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_normal3, sizeof(m_LevelInfo.terrain_normal3));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_normal4, sizeof(m_LevelInfo.terrain_normal4));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_normal5, sizeof(m_LevelInfo.terrain_normal5));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_normal6, sizeof(m_LevelInfo.terrain_normal6));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_normal7, sizeof(m_LevelInfo.terrain_normal7));
	fscanf_s(pFile, "%s\n", m_LevelInfo.terrain_normal8, sizeof(m_LevelInfo.terrain_normal8));

	fscanf_s(pFile, "%f\n", &m_LevelInfo.ground_steepness_blend);

	fscanf_s(pFile, "%s\n", m_LevelInfo.skybox, sizeof(m_LevelInfo.skybox));

	fscanf_s(pFile, "%s\n", m_LevelInfo.music_track, sizeof(m_LevelInfo.music_track));

	fscanf_s(pFile, "%f %f %f %f\n", &m_LevelInfo.ambient_col.x

		, &m_LevelInfo.ambient_col.y

		, &m_LevelInfo.ambient_col.z

		, &m_LevelInfo.ambient_col.w
	);
	fscanf_s(pFile, "%f %f %f %f\n", &m_LevelInfo.diffuse_col.x
		, &m_LevelInfo.diffuse_col.y
		, &m_LevelInfo.diffuse_col.z
		, &m_LevelInfo.diffuse_col.w);
	fscanf_s(pFile, "%f %f %f %f\n", &m_LevelInfo.specular_col.x
		, &m_LevelInfo.specular_col.y
		, &m_LevelInfo.specular_col.z
		, &m_LevelInfo.specular_col.w);
	fscanf_s(pFile, "%f %f %f %f\n", &m_LevelInfo.fog_col.x
		, &m_LevelInfo.fog_col.y
		, &m_LevelInfo.fog_col.z
		, &m_LevelInfo.fog_col.w);
	fscanf_s(pFile, "%f %f %f %f\n", &m_LevelInfo.dust_col.x
		, &m_LevelInfo.dust_col.y
		, &m_LevelInfo.dust_col.z
		, &m_LevelInfo.dust_col.w);

	fscanf_s(pFile, "%f %f %f\n", &m_LevelInfo.diff_dir.x
		, &m_LevelInfo.diff_dir.y
		, &m_LevelInfo.diff_dir.z);

	fscanf_s(pFile, "%f\n", &m_LevelInfo.specular_power);

	fscanf_s(pFile, "%f\n", &m_LevelInfo.ground_friction);
	fscanf_s(pFile, "%f\n", &m_LevelInfo.ground_restitution);

	fscanf_s(pFile, "%f\n", &m_LevelInfo.player_start_x);
	fscanf_s(pFile, "%f\n", &m_LevelInfo.player_start_z);
	fscanf_s(pFile, "%f\n", &m_LevelInfo.player_start_angle);

	fscanf_s(pFile, "%d\n", &m_LevelInfo.bRain);
	fscanf_s(pFile, "%d\n", &m_LevelInfo.bSnow);
	fscanf_s(pFile, "%d\n", &m_LevelInfo.bFog);

	fscanf_s(pFile, "%f %f %f %f\n", &m_LevelInfo.wind.x
		, &m_LevelInfo.wind.z
		, &m_LevelInfo.wind.y
		, &m_LevelInfo.wind.w);

	fclose(pFile);

	m_LevelInfo.flag_angle = atan2f(m_LevelInfo.wind.x, m_LevelInfo.wind.z) - M_PI * 0.27f;

	m_Lights->m_lightBufferData.ambientColor = m_LevelInfo.ambient_col;
	m_Lights->m_lightBufferData.diffuseColor = m_LevelInfo.diffuse_col;
	m_Lights->m_lightBufferData.specularColor = m_LevelInfo.specular_col;
	m_Lights->m_lightBufferData.fogColor = m_LevelInfo.fog_col;

	btVector3 vec_temp = btVector3(m_LevelInfo.diff_dir.x, m_LevelInfo.diff_dir.y, m_LevelInfo.diff_dir.z);
	vec_temp.normalize();
	m_Lights->m_lightBufferData.lightDirection = XMFLOAT3(vec_temp.getX(), vec_temp.getY(), vec_temp.getZ());

	m_Lights->m_lightBufferData.specularPower = m_LevelInfo.specular_power;

	//	m_audio.SetTrack(m_LevelInfo.music_track);
		//m_Lights->UpdateConstantBuffer();

	//	m_uiControl->SetLevel(level);
}
void AllResources::EnableBlendingSkyplane()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };;
	m_deviceResources->GetD3DDeviceContext()->OMSetBlendState(m_alphaEnableBlendingSkyPlane, blendFactor, 0xffffffff);
}

void AllResources::EnableBlendingTexAlph()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };;
	m_deviceResources->GetD3DDeviceContext()->OMSetBlendState(m_alphaEnableBlendingTexAlph, blendFactor, 0xffffffff);
}

void AllResources::EnableBlendingColor()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };;
	m_deviceResources->GetD3DDeviceContext()->OMSetBlendState(m_alphaEnableBlendingColor, blendFactor, 0xffffffff);
}

void AllResources::SetDepthStencil(int state)
{
	switch (state)
	{
	case 0:m_deviceResources->GetD3DDeviceContext()->OMSetDepthStencilState(m_depthDisabledStencilState, 0); break;
	case 1:m_deviceResources->GetD3DDeviceContext()->OMSetDepthStencilState(NULL, 0); break;
	case 2:m_deviceResources->GetD3DDeviceContext()->OMSetDepthStencilState(m_glowStencilState, 0); break;
	}
	/*|
	if (bEnabled == true)
	{
		m_deviceResources->GetD3DDeviceContext()->OMSetDepthStencilState(NULL, 0);
	}
	else
	{
		m_deviceResources->GetD3DDeviceContext()->OMSetDepthStencilState(m_depthDisabledStencilState, 0);
	}*/
}

void AllResources::EnableBlendingGlassAlph()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_deviceResources->GetD3DDeviceContext()->OMSetBlendState(m_alphaEnableBlendingGlass, blendFactor, 0xffffffff);
}
void AllResources::DisableBlending()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_deviceResources->GetD3DDeviceContext()->OMSetBlendState(m_alphaDisableBlendingState, blendFactor, 0xffffffff);
}

void AllResources::SetCull(bool bEnable)
{
	if (bEnable == true)
	{
		m_deviceResources->GetD3DDeviceContext()->RSSetState(g_pRasterState_cull);
	}
	else
	{
		m_deviceResources->GetD3DDeviceContext()->RSSetState(g_pRasterState_nocull);
	}
}

// make a matrix utility
XMFLOAT4X4 AllResources::MakeMatrix(float x, float y, float z, float yaw, float pitch, float roll, float scale)
{
	XMFLOAT4X4 matr;

	// create a flat model matrix
	auto rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	auto translationMatrix = XMMatrixTranslation(x, y, z);

	XMMATRIX scaleMatrix = XMMatrixScaling(scale, scale, scale);
	XMMATRIX scaleRotationMatrix = XMMatrixMultiply(scaleMatrix, rotationMatrix);
	XMMATRIX modelMatrix = XMMatrixMultiplyTranspose(scaleRotationMatrix, translationMatrix);

	XMStoreFloat4x4(&matr, modelMatrix);

	return matr;
}

void AllResources::FinalizeCreateDeviceResources()
{
	auto d3dContext = m_deviceResources->GetD3DDeviceContext();

	m_Lights->FinalizeLightTexturesLoading();

	d3dContext->RSSetState(g_pRasterState_cull);
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d3dContext->VSSetConstantBuffers(
		0,
		1,
		m_Camera->m_constantBuffer.GetAddressOf()
	);

	ID3D11SamplerState * state = 0;

	D3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	desc.MaxAnisotropy = 1;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	//desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; // bilinear
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // trilinear
												   //desc.Filter = D3D11_FILTER_ANISOTROPIC; // anisotropic
												   //desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

	m_deviceResources->GetD3DDevice()->CreateSamplerState(&desc, &state);
	d3dContext->PSSetSamplers(0, 1, &state);

	// create a sampler
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//desc.Filter = D3D11_FILTER_ANISOTROPIC;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MaxAnisotropy = 1;
	//desc.MaxAnisotropy = 16;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	//state = 0;
	m_deviceResources->GetD3DDevice()->CreateSamplerState(&desc, &state);
	d3dContext->PSSetSamplers(1, 1, &state);

	d3dContext->PSSetConstantBuffers(0, 1, m_Lights->m_lightBuffer.GetAddressOf());
	d3dContext->PSSetConstantBuffers(4, 1, m_materialBuffer.GetAddressOf());

	d3dContext->VSSetConstantBuffers(5, 1, m_Lights->m_VlightBuffer.GetAddressOf());
	d3dContext->PSSetConstantBuffers(7, 1, m_distortionBuffer.GetAddressOf());

	d3dContext->VSSetConstantBuffers(4, 1, m_VmaterialBuffer.GetAddressOf());
	d3dContext->VSSetConstantBuffers(10, 1, m_VscreensizeBuffer.GetAddressOf());
	d3dContext->VSSetConstantBuffers(6, 1, m_noiseBuffer.GetAddressOf());
	d3dContext->PSSetConstantBuffers(8, 1, m_skyplaneBuffer.GetAddressOf());

	//m_audio.Initialize();
	//m_audio.CreateResources();
	//m_audio.SetTrack(1);
	//m_audio.SetMusicVolume(0.7f);

	bAudioInitialized = true;
}

task<void> AllResources::CreateDeviceDependentResources()
{
	return create_task([this]() {
		auto d3dDevice = m_deviceResources->GetD3DDevice();

		// so mesh triangles always show, don't cull
		CD3D11_RASTERIZER_DESC rasterizerState(D3D11_DEFAULT);
		rasterizerState.CullMode = D3D11_CULL_NONE;
		rasterizerState.MultisampleEnable = false;
		rasterizerState.AntialiasedLineEnable = true;
		//rasterizerState.
		d3dDevice->CreateRasterizerState(&rasterizerState, &g_pRasterState_nocull);

		rasterizerState.CullMode = D3D11_CULL_BACK;

		d3dDevice->CreateRasterizerState(&rasterizerState, &g_pRasterState_cull);

		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));

		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = FLT_MAX;
		DX::ThrowIfFailed(
			d3dDevice->CreateSamplerState(&sampDesc, &m_samplerLinear)
		);

		ZeroMemory(&sampDesc, sizeof(sampDesc));

		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.MipLODBias = 0.0f;
		sampDesc.MaxAnisotropy = 1;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		DX::ThrowIfFailed(
			d3dDevice->CreateSamplerState(&sampDesc, &m_samplerBilinear)
		);

		//float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		//m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), color);

		//bAudioInitialized = true;
		// Make the constant buffer here

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_Camera->m_constantBuffer
			)
		);

		/*
		CD3D11_BUFFER_DESC cameraBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
		d3dDevice->CreateBuffer(
		&cameraBufferDesc,
		nullptr,
		&m_Camera->m_cameraBuffer
		)
		);
		*/
		CD3D11_BUFFER_DESC lightBufferDesc(sizeof(LightBufferType), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(
				&lightBufferDesc,
				nullptr,
				&m_Lights->m_lightBuffer
			)
		);

		CD3D11_BUFFER_DESC materialBufferDesc(sizeof(MaterialBufferType), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(
				&materialBufferDesc,
				nullptr,
				&m_materialBuffer
			)
		);

		CD3D11_BUFFER_DESC distortionBufferDesc(sizeof(DistortionBufferType), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(
				&distortionBufferDesc,
				nullptr,
				&m_distortionBuffer
			)
		);

		// vertex constant buffers

		CD3D11_BUFFER_DESC VlightBufferDesc(sizeof(LightBufferType), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(
				&VlightBufferDesc,
				nullptr,
				&m_Lights->m_VlightBuffer
			)
		);

		CD3D11_BUFFER_DESC VmaterialBufferDesc(sizeof(MaterialBufferType), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(
				&VmaterialBufferDesc,
				nullptr,
				&m_VmaterialBuffer
			)
		);

		CD3D11_BUFFER_DESC ScreenSizeBufferDesc(sizeof(ScreenSizeBufferType), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(
				&ScreenSizeBufferDesc,
				nullptr,
				&m_VscreensizeBuffer
			)
		);

		CD3D11_BUFFER_DESC VnoiseBufferDesc(sizeof(NoiseBufferType), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(
				&VnoiseBufferDesc,
				nullptr,
				&m_noiseBuffer
			)
		);

		CD3D11_BUFFER_DESC skyplaneBufferDesc(sizeof(SkyBufferType), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(
				&skyplaneBufferDesc,
				nullptr,
				&m_skyplaneBuffer
			)
		);

		// Create constant buffer
		D3D11_BUFFER_DESC cbDesc;
		ZeroMemory(&cbDesc, sizeof(cbDesc));
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		//cbDesc.ByteWidth = sizeof(CB_CONSTANTS);
		//pd3dDevice->CreateBuffer(&cbDesc, NULL, &g_pcbConstants));

		cbDesc.ByteWidth = sizeof(CB_FXAA);
		d3dDevice->CreateBuffer(&cbDesc, NULL, &g_pcbFXAA);

		/*
		MaterialBufferType buffer;
		buffer.specularLvl = 1.0f;
		buffer.specularPow = 20.0f;
		m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_materialBuffer.Get(), 0, nullptr, &buffer, 0, 0);
		*/

		/*
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		bufferDesc.ByteWidth = sizeof(MaterialConstants);
		d3dDevice->CreateBuffer(&bufferDesc, nullptr, &m_materialConstants);

		bufferDesc.ByteWidth = sizeof(LightConstants);
		d3dDevice->CreateBuffer(&bufferDesc, nullptr, &m_lightConstants);

		bufferDesc.ByteWidth = sizeof(ObjectConstants);
		d3dDevice->CreateBuffer(&bufferDesc, nullptr, &m_objectConstants);

		bufferDesc.ByteWidth = sizeof(MiscConstants);
		d3dDevice->CreateBuffer(&bufferDesc, nullptr, &m_miscConstants);
		*/
		//m_deviceResources->GetD3DDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

		SetupAlphaBlendingStates();

		CreateQuadIndexBuffer();
		//shaders_loading = LoadShaders();
		//int pls = sizeof(PointLightType);

		m_Lights->CreateStructuredBuffer(d3dDevice, MAX_POINT_LIGHTS, true, false, m_Lights->m_pointLightBuffer.GetAddressOf(), m_Lights->m_pointLightView.GetAddressOf(),
			m_Lights->m_pointLightAccessView.GetAddressOf());

		m_Lights->CreateStructuredBuffer(d3dDevice, MAX_SPOT_LIGHTS, true, false, m_Lights->m_spotLightBuffer.GetAddressOf(), m_Lights->m_spotLightView.GetAddressOf(),
			m_Lights->m_spotLightAccessView.GetAddressOf());

		//if(ta.is_done()==true)
		bLoadingComplete = true;
	});
}

void AllResources::UpdateNoiseBuffer(NoiseBufferType* noiseData)
{
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_noiseBuffer.Get(), 0, NULL, noiseData, 0, 0);
}

void AllResources::UpdateDistortionBuffer(DistortionBufferType* distortionData)
{
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_distortionBuffer.Get(), 0, NULL, distortionData, 0, 0);
}

void AllResources::UpdateScreenSizeConstants(const ScreenSizeBufferType& data) const
{
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_VscreensizeBuffer.Get(), 0, nullptr, &data, 0, 0);
}

void AllResources::UpdateMaterialBuffer(MaterialBufferType *_bufferData)
{
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_materialBuffer.Get(), 0, nullptr, _bufferData, 0, 0);
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_VmaterialBuffer.Get(), 0, nullptr, _bufferData, 0, 0);
}

void AllResources::UpdateSkyplaneBuffer(SkyBufferType *_bufferData)
{
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_skyplaneBuffer.Get(), 0, nullptr, _bufferData, 0, 0);
}

void AllResources::Update(DX::StepTimer const& timer)
{
	//m_Physics.Update(timer.GetElapsedSeconds(), timer.GetTotalSeconds());

	//m_Camera->buildWorldFrustumPlanes();

	if (bFreeCamera == true)
	{
		//m_FreeCam->Update(timer);
	}

	//m_PadInput->Update();
}

void AllResources::ReleaseDeviceDependentResources()
{
	bLoadingComplete = false;
}

void AllResources::SetModelShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_model_vertexShader.Get(),
		nullptr,
		0
	);

	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_model_inputLayout.Get());

	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_model_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetShinyShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_shinemodel_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_shinemodel_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_shinemodel_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetGroundShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_ground_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_ground_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_ground_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetDefGroundShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_defground_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_defground_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_defground_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetGlassShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_glass_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_glass_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_glass_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetWaterShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_water_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_water_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_water_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetDefWaterShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_defwater_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_defwater_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_defwater_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetMeshShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_mesh_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_mesh_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_mesh_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetSkinShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_skin_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_skin_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_skin_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetDefSkinShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_defskin_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_defskin_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_defskin_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetFireShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_fire_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_fire_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_fire_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetDepthShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_depth_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_depth_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_depth_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetDepthAlphaShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_depthalpha_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_depthalpha_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_depthalpha_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetSkinDepthShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_skindepth_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_skindepth_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_skindepth_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetSkyShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_sky_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_sky_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_sky_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetSkyplaneShader()
{
	//m_shinemodel_vertexShader.
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_skyplane_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_skyplane_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_skyplane_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetPointShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_point_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_point_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_point_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetInsPointShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_inspoint_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_inspoint_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_inspoint_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetPointLitShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_pointlit_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_pointlit_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_pointlit_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetEmmitShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_emmit_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_emmit_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_emmit_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetDefPointShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_defpoint_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_defpoint_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_defpoint_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetDeferredShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_deferred_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_deferred_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_deferred_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetLightShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_light_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_light_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_light_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetGlowShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_glow_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_glow_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_glow_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetGlowMapShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_glowmap_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_glowmap_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_glowmap_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetHblurShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_hblur_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_hblur_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_hblur_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetVblurShader()
{
	m_deviceResources->GetD3DDeviceContext()->VSSetShader(
		m_vblur_vertexShader.Get(),
		nullptr,
		0
	);
	m_deviceResources->GetD3DDeviceContext()->IASetInputLayout(m_vblur_inputLayout.Get());
	m_deviceResources->GetD3DDeviceContext()->PSSetShader(
		m_vblur_pixelShader.Get(),
		nullptr,
		0
	);
}

void AllResources::SetupAlphaBlendingStates()
{
	bool result;

	D3D11_BLEND_DESC blendStateDescription;
	// Clear the blend state description.
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	result = m_deviceResources->GetD3DDevice()->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingColor);

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = m_deviceResources->GetD3DDevice()->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingTexAlph);

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	// dest.a = src.a * (1 - dest.a) + dest.a
	//       == src.a + dest.a - src.a * dest.a
	//       == 1 - (1 - src.a) * (1 - dest.a) ==> preserves alpha to target (optional)

	result = m_deviceResources->GetD3DDevice()->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingGlass);

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MAX;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;

	// dest.a = src.a * (1 - dest.a) + dest.a
	//       == src.a + dest.a - src.a * dest.a
	//       == 1 - (1 - src.a) * (1 - dest.a) ==> preserves alpha to target (optional)

	result = m_deviceResources->GetD3DDevice()->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingSkyPlane);
	// Modify the description to create an alpha disabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;

	// Create the blend state using the description.
	result = m_deviceResources->GetD3DDevice()->CreateBlendState(&blendStateDescription, &m_alphaDisableBlendingState);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	//depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//depthStencilDesc.StencilEnable = false;
	//depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;
	//depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;
	//depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	//depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;

	m_deviceResources->GetD3DDevice()->CreateDepthStencilState(&depthStencilDesc, &m_glowStencilState);

	// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is
	// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
	depthStencilDesc.DepthEnable = true;
	//depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_LESS;

	m_deviceResources->GetD3DDevice()->CreateDepthStencilState(&depthStencilDesc, &m_depthDisabledStencilState);
}

void AllResources::LoadShadersSync()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	static D3D11_INPUT_ELEMENT_DESC vertexDescC[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		/*{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },*/
	};
	//loader->LoadShader("ShinyVertexShader.cso", vertexDescC, ARRAYSIZE(vertexDescC), &m_shinemodel_vertexShader, &m_shinemodel_inputLayout);
	//loader->LoadShader("ShinyPixelShader.cso", &m_shinemodel_pixelShader);

	loader->LoadShader("DefVertexShader.cso", vertexDescC, ARRAYSIZE(vertexDescC), &m_deferred_vertexShader, &m_deferred_inputLayout);
	loader->LoadShader("DefPixelShader.cso", &m_deferred_pixelShader);

	//loader->LoadShader("MeshVertexShader.cso", vertexDescC, ARRAYSIZE(vertexDescC), &m_mesh_vertexShader, &m_mesh_inputLayout);
	//loader->LoadShader("MeshPixelShader.cso", &m_mesh_pixelShader);

	loader->LoadShader("GlassVertexShader.cso", vertexDescC, ARRAYSIZE(vertexDescC), &m_glass_vertexShader, &m_glass_inputLayout);
	loader->LoadShader("GlassPixelShader.cso", &m_glass_pixelShader);

	loader->LoadShader("EmitVertexShader.cso", vertexDescC, ARRAYSIZE(vertexDescC), &m_emmit_vertexShader, &m_emmit_inputLayout);
	loader->LoadShader("EmitPixelShader.cso", &m_emmit_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescGround[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXBLEND", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDB", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDC", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDD", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDE", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDF", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDG", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDH", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // color
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	//loader->LoadShader("GroundVertexShader.cso", vertexDescGround, ARRAYSIZE(vertexDescGround), &m_ground_vertexShader, &m_ground_inputLayout);
	//loader->LoadShader("GroundPixelShader.cso", &m_ground_pixelShader);

	loader->LoadShader("DefGroundVertexShader.cso", vertexDescGround, ARRAYSIZE(vertexDescGround), &m_defground_vertexShader, &m_defground_inputLayout);
	loader->LoadShader("DefGroundPixelShader.cso", &m_defground_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescWater[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXBLEND", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDB", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // color
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//loader->LoadShader("WaterVertexShader.cso", vertexDescWater, ARRAYSIZE(vertexDescWater), &m_water_vertexShader, &m_water_inputLayout);
	//loader->LoadShader("WaterPixelShader.cso", &m_water_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescDefWater[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	loader->LoadShader("DefWaterVertexShader.cso", vertexDescDefWater, ARRAYSIZE(vertexDescDefWater), &m_defwater_vertexShader, &m_defwater_inputLayout);
	loader->LoadShader("DefWaterPixelShader.cso", &m_defwater_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescB[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, /*0*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, /*12*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXNUM", 0, DXGI_FORMAT_R32_UINT, 0, /*12*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PADDING", 0, DXGI_FORMAT_R32G32_FLOAT, 0, /*0*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	//loader->LoadShader("PointVertexShader.cso", vertexDescB, ARRAYSIZE(vertexDescB), &m_point_vertexShader, &m_point_inputLayout);
	//loader->LoadShader("PointPixelShader.cso", &m_point_pixelShader);

	loader->LoadShader("PointLitVertexShader.cso", vertexDescB, ARRAYSIZE(vertexDescB), &m_pointlit_vertexShader, &m_pointlit_inputLayout);
	loader->LoadShader("PointLitPixelShader.cso", &m_pointlit_pixelShader);

	loader->LoadShader("DefPointVertexShader.cso", vertexDescB, ARRAYSIZE(vertexDescB), &m_defpoint_vertexShader, &m_defpoint_inputLayout);
	loader->LoadShader("DefPointPixelShader.cso", &m_defpoint_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescA[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 /*D3D11_APPEND_ALIGNED_ELEMENT*/, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, /*12*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PADDING", 0, DXGI_FORMAT_R32G32_FLOAT, 0, /*0*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITIONI", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0 /*D3D11_APPEND_ALIGNED_ELEMENT*/, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLORI", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PADDINGI", 0, DXGI_FORMAT_R32_FLOAT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	loader->LoadShader("InsPointVertexShader.cso", vertexDescA, ARRAYSIZE(vertexDescA), &m_inspoint_vertexShader, &m_inspoint_inputLayout);
	loader->LoadShader("InsPointPixelShader.cso", &m_inspoint_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescE[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	//loader->LoadShader("SkinVertexShader.cso", vertexDescE, ARRAYSIZE(vertexDescE), &m_skin_vertexShader, &m_skin_inputLayout);
	//loader->LoadShader("SkinPixelShader.cso", &m_skin_pixelShader);

	loader->LoadShader("DefSkinVertexShader.cso", vertexDescE, ARRAYSIZE(vertexDescE), &m_defskin_vertexShader, &m_defskin_inputLayout);
	loader->LoadShader("DefSkinPixelShader.cso", &m_defskin_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescF[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	loader->LoadShader("SkiVertexShader.cso", vertexDescF, ARRAYSIZE(vertexDescF), &m_sky_vertexShader, &m_sky_inputLayout);
	loader->LoadShader("SkiPixelShader.cso", &m_sky_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescG[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	loader->LoadShader("FireVertexShader.cso", vertexDescG, ARRAYSIZE(vertexDescG), &m_fire_vertexShader, &m_fire_inputLayout);
	loader->LoadShader("FirePixelShader.cso", &m_fire_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescH[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//loader->LoadShader("DepthVertexShader.cso", vertexDescH, ARRAYSIZE(vertexDescH), &m_depth_vertexShader, &m_depth_inputLayout);
	//loader->LoadShader("DepthPixelShader.cso", &m_depth_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescHb[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//loader->LoadShader("DepthAlphaVertexShader.cso", vertexDescHb, ARRAYSIZE(vertexDescHb), &m_depthalpha_vertexShader, &m_depthalpha_inputLayout);
	//loader->LoadShader("DepthAlphaPixelShader.cso", &m_depthalpha_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescI[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//loader->LoadShader("SkinDepthVertexShader.cso", vertexDescI, ARRAYSIZE(vertexDescI), &m_skindepth_vertexShader, &m_skindepth_inputLayout);
	//loader->LoadShader("SkinDepthPixelShader.cso", &m_skindepth_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescJ[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ALPHA", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//loader->LoadShader("SkyPlaneVertexShader.cso", vertexDescJ, ARRAYSIZE(vertexDescJ), &m_skyplane_vertexShader, &m_skyplane_inputLayout);
	//loader->LoadShader("SkyPlanePixelShader.cso", &m_skyplane_pixelShader);

	static D3D11_INPUT_ELEMENT_DESC vertexDescL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	loader->LoadShader("LightVertexShader.cso", vertexDescL, ARRAYSIZE(vertexDescL), &m_light_vertexShader, &m_light_inputLayout);
	loader->LoadShader("LightPixelShader.cso", &m_light_pixelShader);

	loader->LoadShader("GlowVertexShader.cso", vertexDescL, ARRAYSIZE(vertexDescL), &m_glow_vertexShader, &m_glow_inputLayout);
	loader->LoadShader("GlowPixelShader.cso", &m_glow_pixelShader);

	loader->LoadShader("GlowMapVertexShader.cso", vertexDescL, ARRAYSIZE(vertexDescL), &m_glowmap_vertexShader, &m_glowmap_inputLayout);
	loader->LoadShader("GlowMapPixelShader.cso", &m_glowmap_pixelShader);

	loader->LoadShader("BlurHVertexShader.cso", vertexDescL, ARRAYSIZE(vertexDescL), &m_hblur_vertexShader, &m_hblur_inputLayout);
	loader->LoadShader("BlurHPixelShader.cso", &m_hblur_pixelShader);

	loader->LoadShader("BlurVVertexShader.cso", vertexDescL, ARRAYSIZE(vertexDescL), &m_vblur_vertexShader, &m_vblur_inputLayout);
	loader->LoadShader("BlurVPixelShader.cso", &m_vblur_pixelShader);

	loader->LoadShaderAsync("FXAAVertexShader.cso", nullptr, 0, &m_fxaa_vertexShader, nullptr);
	loader->LoadShaderAsync("FXAAPixelShader.cso", &m_fxaa_pixelShader);
}

task<void> AllResources::LoadShadersAsync()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	static D3D11_INPUT_ELEMENT_DESC vertexDescC[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		/*{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },*/
	};
	//tasks.push_back(loader->LoadShaderAsync("ShinyVertexShader.cso", vertexDescC, ARRAYSIZE(vertexDescC), &m_shinemodel_vertexShader, &m_shinemodel_inputLayout));
	//tasks.push_back(loader->LoadShaderAsync("ShinyPixelShader.cso", &m_shinemodel_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("DefVertexShader.cso", vertexDescC, ARRAYSIZE(vertexDescC), &m_deferred_vertexShader, &m_deferred_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("DefPixelShader.cso", &m_deferred_pixelShader));

	//tasks.push_back(loader->LoadShaderAsync("MeshVertexShader.cso", vertexDescC, ARRAYSIZE(vertexDescC), &m_mesh_vertexShader, &m_mesh_inputLayout));
	//tasks.push_back(loader->LoadShaderAsync("MeshPixelShader.cso", &m_mesh_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("GlassVertexShader.cso", vertexDescC, ARRAYSIZE(vertexDescC), &m_glass_vertexShader, &m_glass_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("GlassPixelShader.cso", &m_glass_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("EmitVertexShader.cso", vertexDescC, ARRAYSIZE(vertexDescC), &m_emmit_vertexShader, &m_emmit_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("EmitPixelShader.cso", &m_emmit_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescGround[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXBLEND", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDB", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDC", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDD", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDE", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDF", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDG", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDH", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // color
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	//tasks.push_back(loader->LoadShaderAsync("GroundVertexShader.cso", vertexDescGround, ARRAYSIZE(vertexDescGround), &m_ground_vertexShader, &m_ground_inputLayout));
	//tasks.push_back(loader->LoadShaderAsync("GroundPixelShader.cso", &m_ground_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("DefGroundVertexShader.cso", vertexDescGround, ARRAYSIZE(vertexDescGround), &m_defground_vertexShader, &m_defground_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("DefGroundPixelShader.cso", &m_defground_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescWater[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXBLEND", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "TEXBLENDB", 0, DXGI_FORMAT_R32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // two texture blend
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // color
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//tasks.push_back(loader->LoadShaderAsync("WaterVertexShader.cso", vertexDescWater, ARRAYSIZE(vertexDescWater), &m_water_vertexShader, &m_water_inputLayout));
	//tasks.push_back(loader->LoadShaderAsync("WaterPixelShader.cso", &m_water_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescDefWater[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	tasks.push_back(loader->LoadShaderAsync("DefWaterDefVertexShader.cso", vertexDescDefWater, ARRAYSIZE(vertexDescDefWater), &m_defwater_vertexShader, &m_defwater_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("DefWaterDefPixelShader.cso", &m_defwater_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescB[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, /*0*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, /*12*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PADDING", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, /*0*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	//tasks.push_back(loader->LoadShaderAsync("PointVertexShader.cso", vertexDescB, ARRAYSIZE(vertexDescB), &m_point_vertexShader, &m_point_inputLayout));
	//tasks.push_back(loader->LoadShaderAsync("PointPixelShader.cso", &m_point_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("PointLitVertexShader.cso", vertexDescB, ARRAYSIZE(vertexDescB), &m_pointlit_vertexShader, &m_pointlit_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("PointLitPixelShader.cso", &m_pointlit_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("DefPointVertexShader.cso", vertexDescB, ARRAYSIZE(vertexDescB), &m_defpoint_vertexShader, &m_defpoint_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("DefPointPixelShader.cso", &m_defpoint_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescA[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 /*D3D11_APPEND_ALIGNED_ELEMENT*/, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, /*12*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PADDING", 0, DXGI_FORMAT_R32G32_FLOAT, 0, /*0*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITIONI", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0 /*D3D11_APPEND_ALIGNED_ELEMENT*/, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLORI", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PADDINGI", 0, DXGI_FORMAT_R32_FLOAT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	tasks.push_back(loader->LoadShaderAsync("InsPointVertexShader.cso", vertexDescA, ARRAYSIZE(vertexDescA), &m_inspoint_vertexShader, &m_inspoint_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("InsPointPixelShader.cso", &m_inspoint_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescE[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	//tasks.push_back(loader->LoadShaderAsync("SkinVertexShader.cso", vertexDescE, ARRAYSIZE(vertexDescE), &m_skin_vertexShader, &m_skin_inputLayout));
	//tasks.push_back(loader->LoadShaderAsync("SkinPixelShader.cso", &m_skin_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("DefSkinVertexShader.cso", vertexDescE, ARRAYSIZE(vertexDescE), &m_defskin_vertexShader, &m_defskin_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("DefSkinPixelShader.cso", &m_defskin_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescF[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	tasks.push_back(loader->LoadShaderAsync("SkiVertexShader.cso", vertexDescF, ARRAYSIZE(vertexDescF), &m_sky_vertexShader, &m_sky_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("SkiPixelShader.cso", &m_sky_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescG[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*20*/D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	tasks.push_back(loader->LoadShaderAsync("FireVertexShader.cso", vertexDescG, ARRAYSIZE(vertexDescG), &m_fire_vertexShader, &m_fire_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("FirePixelShader.cso", &m_fire_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescH[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//tasks.push_back(loader->LoadShaderAsync("DepthVertexShader.cso", vertexDescH, ARRAYSIZE(vertexDescH), &m_depth_vertexShader, &m_depth_inputLayout));
	//tasks.push_back(loader->LoadShaderAsync("DepthPixelShader.cso", &m_depth_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescHb[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//tasks.push_back(loader->LoadShaderAsync("DepthAlphaVertexShader.cso", vertexDescHb, ARRAYSIZE(vertexDescHb), &m_depthalpha_vertexShader, &m_depthalpha_inputLayout));
	//tasks.push_back(loader->LoadShaderAsync("DepthAlphaPixelShader.cso", &m_depthalpha_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescI[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//tasks.push_back(loader->LoadShaderAsync("SkinDepthVertexShader.cso", vertexDescI, ARRAYSIZE(vertexDescI), &m_skindepth_vertexShader, &m_skindepth_inputLayout));
	//tasks.push_back(loader->LoadShaderAsync("SkinDepthPixelShader.cso", &m_skindepth_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescJ[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ALPHA", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//tasks.push_back(loader->LoadShaderAsync("SkyPlaneVertexShader.cso", vertexDescJ, ARRAYSIZE(vertexDescJ), &m_skyplane_vertexShader, &m_skyplane_inputLayout));
	//tasks.push_back(loader->LoadShaderAsync("SkyPlanePixelShader.cso", &m_skyplane_pixelShader));

	static D3D11_INPUT_ELEMENT_DESC vertexDescL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	tasks.push_back(loader->LoadShaderAsync("LightVertexShader.cso", vertexDescL, ARRAYSIZE(vertexDescL), &m_light_vertexShader, &m_light_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("LightPixelShader.cso", &m_light_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("GlowVertexShader.cso", vertexDescL, ARRAYSIZE(vertexDescL), &m_glow_vertexShader, &m_glow_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("GlowPixelShader.cso", &m_glow_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("GlowMapVertexShader.cso", vertexDescL, ARRAYSIZE(vertexDescL), &m_glowmap_vertexShader, &m_glowmap_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("GlowMapPixelShader.cso", &m_glowmap_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("BlurHVertexShader.cso", vertexDescL, ARRAYSIZE(vertexDescL), &m_hblur_vertexShader, &m_hblur_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("BlurHPixelShader.cso", &m_hblur_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("BlurVVertexShader.cso", vertexDescL, ARRAYSIZE(vertexDescL), &m_vblur_vertexShader, &m_vblur_inputLayout));
	tasks.push_back(loader->LoadShaderAsync("BlurVPixelShader.cso", &m_vblur_pixelShader));

	tasks.push_back(loader->LoadShaderAsync("FXAAVertexShader.cso", nullptr, 0, &m_fxaa_vertexShader, nullptr));
	tasks.push_back(loader->LoadShaderAsync("FXAAPixelShader.cso", &m_fxaa_pixelShader));

	return when_all(tasks.begin(), tasks.end());
}