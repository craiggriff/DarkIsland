#pragma once

#include "Common\StepTimer.h"
#include "GameUIControl.h"
//#include "Textures.h"
#include "../DirectXTK/Inc/DDSTextureLoader.h"

#include "Lights.h"
#include "Physics.h"
#include "Camera.h"
//#include "Input.h"
#include "Audio.h"
#include "Textures.h"

#include "FXAA.h"

#include "MoveLookController.h"
//#include "freecamera.h"

//#include "Fire.h"

#include "orthowindowclass.h"
#include "deferredbuffersclass.h"

namespace Game
{
	typedef struct
	{
	public:
		int index;
		char group[50];
		char filename[50];
		int type;
		int physics_shape;
		float mass;
		float restitution;
		float friction;
		float rotx;
		float roty;
		float rotz;
		float scale;
		char sound_filename[50];
	} db_model_info;

	typedef struct
	{
	public:
		int index;
		char filename[51];
		char material_name[51];
		char type[4]; // t (texture) or n (normal)
		int tex_slot;
		int alpha;
		float intensity; // normal intensity
		float specular_level; // specular level
		Platform::String^ s_filename;
	} db_texture_info;

	class AllResources
	{
	public:
		AllResources(const std::shared_ptr<DX::DeviceResources>& deviceResources, Game::IGameUIControl^ UIControl, DX::StepTimer* pp_timer);

		void ReleaseDeviceDependentResources();

		void CreateDatabase();

		void SetModelShader();
		void SetPointShader();
		void SetInsPointShader();
		void SetPointLitShader();

		void SetShinyShader();
		void SetMeshShader();
		void SetSkinShader();
		void SetSkyShader();
		void SetGroundShader();
		void SetWaterShader();
		void SetDefWaterShader();
		void SetFireShader();
		void SetDepthShader();
		void SetDepthAlphaShader();
		void SetSkinDepthShader();
		void SetSkyplaneShader();
		void SetDeferredShader();
		void SetDefGroundShader();
		void SetDefSkinShader();
		void SetDefPointShader();

		void SetGlassShader();
		void SetEmmitShader();

		void SetLightShader();
		void SetGlowShader();
		void SetGlowMapShader();

		void SetHblurShader();
		void SetVblurShader();

		void SetLighting();
		void SaveLighting();

		void SetDrawDistance(float dist);

		XMFLOAT4X4 GetMatrix(btTransform* p_body);
		XMFLOAT4X4 GetMatrix(btRigidBody* p_body);

		unsigned int col_r, col_g, col_b;

		void LoadDatabaseModelInfo(std::vector<db_model_info>* minfo, char* where_str);
		void LoadDatabaseTextureInfo(std::vector<db_texture_info>* tinfo, char* where_str);

		void AllResources::RenderAlphaMode(int alpha_mode);
		const wchar_t* GetWC(const char *c);
		size_t to_narrow(const wchar_t * src, char * dest, size_t dest_len);
		char* to_narrow_str(const wchar_t* src);

		Platform::String^ StringFromAscIIChars(char* chars);
		Platform::String^ stringToPlatformString(std::wstring inputString);

		void UpdateMaterialBuffer(MaterialBufferType *_bufferData);

		void UpdateSkyplaneBuffer(SkyBufferType *_bufferData);

		task<ID3D11ShaderResourceView*> GetOrCreateTextureAsync(const std::wstring& textureName);
		ID3D11ShaderResourceView* CreateTextureFromDDSInMemory(const BYTE* ddsData, size_t ddsDataSize);

		void LoadDatabaseAllTextureFilenames();

		void CreateQuadIndexBuffer();
		void SetQuadIndexBuffer();

		void LoadLevelInfo(int level);
		void SaveLevelInfo(int level);

		XMFLOAT4X4* ConstantModelBuffer();

		task<void> CreateDeviceDependentResources();
		void FinalizeCreateDeviceResources();

		void LoadDatabaseLevelInfo(int level);
		void SaveDatabaseLevelInfo();

		// One big index buffer for all particle systems
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_quadindexBuffer;
		std::vector<unsigned short> m_quadindices;
		int m_quadindicesCount;

		bool bLoadingComplete;

		bool bAudioInitialized;

		float view_distance;

		int camera_mode;

		bool bFreeCamera;
		bool bContentFolder;
		bool bDevelopment;
		int current_level;

		int total_terrrain_x_points;
		int total_terrrain_y_points;

		int delete_radius;

		MoveLookController^ p_controller;

		FILE* pFile;

		DX::StepTimer* p_Timer;

		task<void> shaders_loading;

		task<void> AllResources::LoadShadersAsync();
		void AllResources::LoadShadersSync();

		void SetupAlphaBlendingStates();

		void Update(DX::StepTimer const& timer);
		void SetCull(bool bEnable);
		void SetDepthStencil(int state);

		void EnableBlendingColor();
		void EnableBlendingTexAlph();
		void EnableBlendingGlassAlph();
		void DisableBlending();
		void EnableBlendingSkyplane();

		float player_x, player_y, player_z;

		void SetplayerPos(float x, float y, float z) {
			player_x = x;
			player_y = y;
			player_z = z;
		};

		char local_file_folder[140];

		LevelInfo m_LevelInfo;

		//Textures* m_Textures;	// All the textures
		std::vector<db_model_info> model_info;

		FXAA* m_FXAA;

		Lights* m_Lights;

		Camera* m_Camera;

		Textures* m_Textures;

		//FreeCamera* m_FreeCam;

		//PadInput^ m_PadInput;

		Physics m_Physics;

		Audio  m_audio;

		OrthoWindowClass* m_FullScreenWindow;
		OrthoWindowClass* m_DownsampleWindow;

		DeferredBuffersClass* m_DeferredBuffers;
		DeferredBuffersClass* m_RDeferredBuffers; // refract

		Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_samplerLinear;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_samplerBilinear;

		XMFLOAT4X4 MakeMatrix(float x, float y, float z, float yaw, float pitch, float roll, float scale);

		void UpdateNoiseBuffer(NoiseBufferType* noiseData);
		void UpdateDistortionBuffer(DistortionBufferType* distortionData);

		void UpdateScreenSizeConstants(const ScreenSizeBufferType& data) const;

		ID3D11Buffer* GetMaterialConstants() const { return m_materialConstants.Get(); }
		ID3D11Buffer* GetLightConstants() const { return m_lightConstants.Get(); }
		ID3D11Buffer* GetObjectConstants() const { return m_objectConstants.Get(); }
		ID3D11Buffer* GetMiscConstants() const { return m_miscConstants.Get(); }

		ID3D11Buffer*               g_pcbConstants = NULL;
		ID3D11Buffer*               g_pcbFXAA = NULL;

		Game::IGameUIControl^ m_uiControl;

		ID3D11BlendState* m_alphaEnableBlendingColor;
		ID3D11BlendState* m_alphaDisableBlendingState;
		ID3D11BlendState* m_alphaEnableBlendingTexAlph;
		ID3D11BlendState* m_alphaEnableBlendingSkyPlane;
		ID3D11BlendState* m_alphaEnableBlendingGlass;

		ID3D11DepthStencilState* m_depthDisabledStencilState;
		ID3D11DepthStencilState* m_glowStencilState;

		ID3D11RasterizerState * g_pRasterState_cull;
		ID3D11RasterizerState * g_pRasterState_nocull;

		std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textureResources;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_model_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_model_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_model_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_point_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_point_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_point_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_inspoint_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_inspoint_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inspoint_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pointlit_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pointlit_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pointlit_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_defpoint_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_defpoint_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_defpoint_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_sky_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_sky_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_sky_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_shinemodel_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_shinemodel_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_shinemodel_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_glass_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_glass_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_glass_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_emmit_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_emmit_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_emmit_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_ground_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ground_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_ground_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_water_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_water_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_water_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_mesh_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_mesh_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_mesh_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_defwater_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_defwater_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_defwater_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skin_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_skin_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skin_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_defskin_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_defskin_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_defskin_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_fire_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_fire_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_fire_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_depth_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_depth_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_depth_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_depthalpha_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_depthalpha_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_depthalpha_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skindepth_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_skindepth_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skindepth_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skyplane_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_skyplane_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skyplane_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_deferred_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_deferred_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_deferred_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_defground_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_defground_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_defground_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_light_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_light_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_light_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_glow_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_glow_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_glow_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_glowmap_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_glowmap_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_glowmap_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_hblur_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_hblur_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_hblur_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vblur_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_vblur_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_vblur_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_fxaa_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_fxaa_pixelShader;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_materialConstants;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightConstants;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_objectConstants;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_miscConstants;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_noiseBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_distortionBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_skyplaneBuffer;

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_materialBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_VmaterialBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_VscreensizeBuffer;
	};
}
