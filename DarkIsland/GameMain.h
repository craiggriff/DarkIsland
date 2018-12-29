#pragma once

#include "Common\StepTimer.h"
#include "DeviceResources.h"
#include "MoveLookController.h"
#include "GameRenderer.h"
#include "Simple3DGame.h"
#include "GameUIControl.h"

namespace GameControl
{
	private enum class UpdateEngineState
	{
		WaitingForResources,
		ResourcesLoaded,
		WaitingForPress,
		Dynamics,
		TooSmall,
		Suspended,
		Deactivated,
	};

	private enum class PressResultState
	{
		LoadGame,
		PlayLevel,
		ContinueLevel,
	};

	private enum class GameInfoOverlayState
	{
		Loading,
		GameStats,
		GameOverExpired,
		GameOverCompleted,
		LevelStart,
		Pause,
	};
};
namespace Game
{
	class GameMain : public DX::IDeviceNotify
	{
	public:
		GameMain(const std::shared_ptr<DX::DeviceResources>& deviceResources, Game::IGameUIControl^ UIControl);
		~GameMain();
		void CreateWindowSizeDependentResources();
		void StartRenderLoop();
		void StopRenderLoop();
		void Suspend();
		void Resume();
		critical_section& GetCriticalSection() { return m_criticalSection; }

		bool bPaused;
		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		void SetPause(bool _bPause);

		void PauseRequested() { if (m_updateState == GameControl::UpdateEngineState::Dynamics) m_pauseRequested = true; };
		void PressComplete() { if (m_updateState == GameControl::UpdateEngineState::WaitingForPress) m_pressComplete = true; };
		void ResetGame();
		void SetBackground(unsigned int background);
		void CycleBackground();
		void LicenseChanged(
			Windows::ApplicationModel::Store::ListingInformation^ listing,
			Windows::ApplicationModel::Store::LicenseInformation^ license
		);

		void WindowActivationChanged(Windows::UI::Core::CoreWindowActivationState activationState);
		GameRenderer* GetRenderer() { return m_renderer; };
		Simple3DGame^ GetGame() { return m_game; };
		MoveLookController^ GetMoveLook() { return m_controller; };
	private:
		void SetGameInfoOverlay(GameControl::GameInfoOverlayState state);
		void InitializeGameState();
		void UpdateLayoutState();
		//task<void> Update(float timeDelta, float timeTotal);
		void UpdatePhysics(float timeDelta, float timeTotal);

	private:
		bool                                                m_pauseRequested;
		bool                                                m_pressComplete;
		bool                                                m_haveFocus;

		std::shared_ptr<DX::DeviceResources>                m_deviceResources;

		DX::StepTimer m_timer;

		Game::IGameUIControl^                   m_uiControl;

		MoveLookController^                                 m_controller;
		GameRenderer*                                       m_renderer;
		Simple3DGame^                                       m_game;

		GameControl::UpdateEngineState                      m_updateState;
		GameControl::UpdateEngineState                      m_updateStateNext;
		GameControl::PressResultState                       m_pressResult;
		GameControl::GameInfoOverlayState                   m_gameInfoOverlayState;
		Windows::ApplicationModel::Store::LicenseInformation^ m_licenseInformation;
		Windows::ApplicationModel::Store::ListingInformation^ m_listingInformation;
#ifdef USE_STORE_SIMULATOR
		PersistentState^                                    m_licenseState;
		bool                                                m_isTrial;
#endif
		critical_section                       m_criticalSection;
		Windows::Foundation::IAsyncAction^                  m_renderLoopWorker;
	};
}