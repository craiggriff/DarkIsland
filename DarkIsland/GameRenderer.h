#pragma once

#include "AllStructures.h"
#include "DeviceResources.h"
#include "GameHud.h"
#include "Simple3DGame.h"
#include "AllResources.h"

#include "Level.h"
#include "LevelEdit.h"

#include "rendertextureclass.h"

#include "Sky.h"
#include "skyplaneclass.h"

#include "mesh.h"
#include "Fire.h"
#include "Snow.h"
#include "Rain.h"
#include "Fog.h"

#include "Grass.h"
#include "Statics.h"
#include "Stuff.h"

#include "buggy.h"
#include "bert.h"

#include "Car.h"

#include "GunBall.h"

#include "PanCamera.h"

namespace Game
{
	ref class Simple3DGame;
	ref class GameHud;

	class GameRenderer
	{
	public:
		AllResources * GetResources() { return m_Res; };

		GameRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources, Simple3DGame^ p_game, Game::IGameUIControl^ UIControl, DX::StepTimer* pp_timer);

		void Initialize();

		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();

		void SwitchTorch();

		void RenderWaterTexture();

		void RenderSkyCubeMap();
		void RenderGlowTexture();
		void Render();
		void UpdateCurrentItemPointer(int pointer_delta);

		bool bCarMode = false;

		void LevelLoading() {
			m_levelResourcesLoaded = false;
			//m_Res->m_Physics.RemoveAllObjects(0);
		};

		task<void> CreateGameDeviceResourcesAsync(_In_ Simple3DGame^ game);
		void FinalizeCreateGameDeviceResources();
		task<void> LoadLevelResourcesAsync(int levelnum);
		void FinalizeLoadLevelResources();
		void ChangeSkybox(char* sky_filename);
		void ChangeWeather(bool _bSnow, bool _bRain, bool _bFog);
		void ChangePauseFly(bool _bPauseFly) {
			bPauseFly = _bPauseFly;
		};
		void ChangeEditAsPlayer(bool _bEditAsPlayer) {
			bEditAsPlayer = _bEditAsPlayer;
		};

		Windows::Foundation::Size outputSize;

		void UpdateRemovePhysics();

		void SetPause(bool _bPause);

		task<void> ProcessCollisions();

		task<void> Update(float timeDelta, float timeTotal);
		//task<void> UpdatePhysics(DX::StepTimer const& timer);
		task<void> UpdateNonPhysics(float timeDelta, float timeTotal);
		void UpdatePhysics(float timeDelta, float timeTotal);

		GameHud^ Hud() { return m_gameHud; }

		bool bFXAAEnabled = true;
		int model_pointer_id;

		int render_delay;
		bool bLoadingComplete;

		int player_mode;

		float camera_height;

		bool bPaused;
		bool bPauseFly;
		bool bEditAsPlayer;

		bool bTorchOn;

		bool bDefGrass;// Deffered Grass ( faster but less nice than alpha blended )

		PanCamera* m_PanCamera;

		Level*	m_Level;
		LevelEdit* m_LevelEdit;

		Sky* m_Sky;
		SkyPlaneClass* m_Skyplane;

		Statics* m_Statics;
		Stuff* m_Stuff;

		Fire* m_Fire;
		Snow* m_Snow;
		Rain* m_Rain;
		Fog* m_Fog;

		Grass* m_Grass;

		GunBall* m_GunBall;

		Car* m_Car;

		Buggy* m_Buggy;
		Bert* m_Bert;

		std::vector<Mesh*> m_Marker;

		bool bMusicStarted;

#if defined(_DEBUG)
		void ReportLiveDeviceObjects();
#endif
		AllResources* m_Res;

		RenderTextureClass* m_DeferredTexture;
		RenderTextureClass* m_FXAATexture;
		bool                                                m_levelResourcesLoaded;
		bool                                                m_initialized;
		bool                                                m_gameResourcesLoaded;
	private:

		ScreenSizeBufferType screen_size_buffer;

		RenderTextureClass* m_waterTexture;

		RenderTextureClass* m_GlowTexture;
		RenderTextureClass* m_GlowTextureDownsample;
		RenderTextureClass* m_GlowTextureDownsample2;
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources>                m_deviceResources;

		GameHud^                                            m_gameHud;
		Simple3DGame^                                       m_game;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_sphereTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_cylinderTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_ceilingTexture[GameConstants::MaxBackgroundTextures];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_floorTexture[GameConstants::MaxBackgroundTextures];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_wallsTexture[GameConstants::MaxBackgroundTextures];

		// Constant Buffers
		Microsoft::WRL::ComPtr<ID3D11Buffer>                m_constantBufferNeverChanges;
		Microsoft::WRL::ComPtr<ID3D11Buffer>                m_constantBufferChangeOnResize;
		Microsoft::WRL::ComPtr<ID3D11Buffer>                m_constantBufferChangesEveryFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer>                m_constantBufferChangesEveryPrim;

		Microsoft::WRL::ComPtr<ID3D11VertexShader>          m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>          m_vertexShaderFlat;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>           m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>           m_pixelShaderFlat;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>           m_vertexLayout;
	};
}