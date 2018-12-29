//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "MoveLookController.h"
#include "GameMain.h"
#include "GameUIConstants.h"
#include "DefGame.h"

using namespace GameControl;

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Store;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::System::Threading;

using namespace Game;

GameMain::GameMain(const std::shared_ptr<DX::DeviceResources>& deviceResources, Game::IGameUIControl^ UIControl) :
	m_deviceResources(deviceResources),
	m_uiControl(UIControl)
{
	m_deviceResources->RegisterDeviceNotify(this);

	m_controller = ref new MoveLookController(Window::Current->CoreWindow, m_deviceResources->GetSwapChainPanel()->Dispatcher);

	m_controller->AutoFire(true);

	m_game = ref new Simple3DGame(m_controller);

	m_renderer = new GameRenderer(m_deviceResources, m_game, m_uiControl, &m_timer);

	bPaused = false;
	// messes up the timing when true
	//m_timer.SetFixedTimeStep(false);
	//m_timer.SetTargetElapsedSeconds(1.0 / 60);

	auto bounds = m_deviceResources->GetLogicalSize();

	m_controller->SetMoveRect(
		XMFLOAT2(0.0f, bounds.Height - GameConstants::TouchRectangleSize),
		XMFLOAT2(GameConstants::TouchRectangleSize, bounds.Height)
	);
	m_controller->SetFireRect(
		XMFLOAT2(bounds.Width - GameConstants::TouchRectangleSize, bounds.Height - GameConstants::TouchRectangleSize),
		XMFLOAT2(bounds.Width, bounds.Height)
	);

	SetGameInfoOverlay(GameInfoOverlayState::Loading);
	m_uiControl->SetAction(GameInfoOverlayCommand::None);
	m_uiControl->ShowGameInfoOverlay();
	//m_uiControl->
	m_renderer->CreateDeviceDependentResources();

	m_game->Initialize(m_controller, m_renderer);
	m_renderer->m_Res->LoadShadersSync();
	//m_renderer->m_Res->LoadShadersAsync().wait();
	create_task([this]()
	{
		// Asynchronously initialize the game class and load the renderer device resources.
		// By doing all this asynchronously, the game gets to its main loop more quickly
		// and in parallel all the necessary resources are loaded on other threads.

		return m_renderer->CreateGameDeviceResourcesAsync(m_game);
		//return m_renderer->m_Res->LoadShadersAsync().then([this]() {return m_renderer->CreateGameDeviceResourcesAsync(m_game); });
	}).then([this]()
	{
		m_renderer->bLoadingComplete = true;
		// The finalize code needs to run in the same thread context
		// as the m_renderer object was created because the D3D device context
		// can ONLY be accessed on a single thread.
		{
			critical_section::scoped_lock lock(m_criticalSection);
			m_renderer->FinalizeCreateGameDeviceResources();
		}

		InitializeGameState();

		if (m_updateState == UpdateEngineState::WaitingForResources)
		{
			m_renderer->m_levelResourcesLoaded = false;
			return m_game->LoadLevelAsync().then([this]()
			{
				// The m_game object may need to deal with D3D device context work so
				// again the finalize code needs to run in the same thread
				// context as the m_renderer object was created because the D3D
				// device context can ONLY be accessed on a single thread.
				critical_section::scoped_lock lock(m_criticalSection);
				m_game->FinalizeLoadLevel();
				//PressComplete();
				m_pressComplete = true;

				SetPause(false);
				m_game->SetCurrentLevelToSavedState();
				m_updateState = UpdateEngineState::ResourcesLoaded;
				//m_updateState = UpdateEngineState::PlayLevel;
			}, task_continuation_context::use_current());
		}
		else
		{
			// The game is not in the middle of a level so there aren't any level
			// resources to load.  Creating a no-op task so that in both cases
			// the same continuation logic is used.
			return create_task([]() {});
		}
	}, task_continuation_context::use_current()).then([this]()
	{
		// Since Game loading is an async task, the app visual state
		// may be too small or not have focus.  Put the state machine
		// into the correct state to reflect these cases.

		if (m_deviceResources->GetLogicalSize().Width < GameConstants::MinPlayableWidth)
		{
			m_updateStateNext = m_updateState;
			m_updateState = UpdateEngineState::TooSmall;
			m_controller->Active(false);
			m_uiControl->HideGameInfoOverlay();
			m_uiControl->ShowTooSmall();
		}
		else if (!m_haveFocus)
		{
			m_updateStateNext = m_updateState;
			m_updateState = UpdateEngineState::Deactivated;
			m_controller->Active(false);
			m_uiControl->SetAction(GameInfoOverlayCommand::None);
		}

		if (m_renderLoopWorker == nullptr || m_renderLoopWorker->Status != AsyncStatus::Started)
		{
			StartRenderLoop();
		}
	}, task_continuation_context::use_current());
}

//--------------------------------------------------------------------------------------

GameMain::~GameMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

//--------------------------------------------------------------------------------------

void GameMain::StartRenderLoop()
{
	// If the animation render loop is already running then do not start another thread.
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// Create a task that will be run on a background thread.
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		float timeDelta = 0.0f;// = m_timer.GetElapsedSeconds();
		float timeTotal = 0.0f;// = m_timer.GetTotalSeconds();
		// Calculate the updated frame and render once per vertical blanking interval.
		while (action->Status == AsyncStatus::Started)
		{
			critical_section::scoped_lock lock(m_criticalSection);

			std::vector<task<void>> tasks;

			m_timer.Tick([&]()
			{
				timeDelta = (m_timer.GetTotalSeconds() - timeTotal);// m_timer.GetElapsedSeconds();
				timeTotal = m_timer.GetTotalSeconds();
			});

			//if (bPaused == false)
			//{
			tasks.push_back(m_renderer->Update(timeDelta, timeTotal).then([this] {
				return m_renderer->Render(); }));
			//}
#ifdef PHYSICS_BACKGROUND_UPDATE
			if (bPaused == false)
			{
				tasks.push_back(m_renderer->m_Res->m_Physics.Update(timeDelta, timeTotal));
			}
#endif

			when_all(tasks.begin(), tasks.end()).then([this, timeDelta, timeTotal]
			{
				//m_timer.Tick([&]()
				//{
				//	timeDelta = (m_timer.GetTotalSeconds() - timeTotal);// m_timer.GetElapsedSeconds();
				//	timeTotal = m_timer.GetTotalSeconds();
				//});

				UpdatePhysics(timeDelta, timeTotal);

				m_deviceResources->Present();
			}).wait();

			if (!m_haveFocus || (m_updateState == UpdateEngineState::TooSmall))
			{
				// The app is in an inactive state so stop rendering
				// This optimizes for power and allows the framework to become more queiecent
				break;
			}
		}
	});

	// Run task on a dedicated high priority background thread.
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void GameMain::UpdatePhysics(float timeDelta, float timeTotal)
{
	m_controller->Update();

	if (m_controller->KeyState(Windows::System::VirtualKey::N, true) == true)
	{
		PressComplete();
		//m_main->SetPause(true);
		GetMoveLook()->SetCompleteLevel(true);
	}

	switch (m_updateState)
	{
	case UpdateEngineState::ResourcesLoaded:
		switch (m_pressResult)
		{
		case PressResultState::LoadGame:
			SetGameInfoOverlay(GameInfoOverlayState::GameStats);
			break;

		case PressResultState::PlayLevel:
			SetGameInfoOverlay(GameInfoOverlayState::LevelStart);
			break;

		case PressResultState::ContinueLevel:
			SetGameInfoOverlay(GameInfoOverlayState::Pause);
			break;
		}
		m_updateState = UpdateEngineState::WaitingForPress;
		m_uiControl->SetAction(GameInfoOverlayCommand::TapToContinue);
		m_controller->WaitForPress();
		m_uiControl->ShowGameInfoOverlay();
		break;

	case UpdateEngineState::WaitingForPress:
		if (m_controller->IsPressComplete() || m_pressComplete)
		{
			m_pressComplete = false;
			SetPause(false);
			switch (m_pressResult)
			{
			case PressResultState::LoadGame:
				m_updateState = UpdateEngineState::WaitingForResources;
				m_pressResult = PressResultState::PlayLevel;
				m_controller->Active(false);
				m_game->LoadGame();
				m_uiControl->SetAction(GameInfoOverlayCommand::PleaseWait);
				SetGameInfoOverlay(GameInfoOverlayState::LevelStart);
				m_uiControl->ShowGameInfoOverlay();
				m_renderer->m_levelResourcesLoaded = false;
				m_game->LoadLevelAsync().then([this]()
				{
					critical_section::scoped_lock lock(m_criticalSection);
					m_game->FinalizeLoadLevel();
					m_updateState = UpdateEngineState::ResourcesLoaded;
					//m_pressComplete = true;
				}, task_continuation_context::use_current());
				break;

			case PressResultState::PlayLevel:
				m_updateState = UpdateEngineState::Dynamics;
				m_uiControl->HideGameInfoOverlay();
				m_controller->Active(true);
				//SetPause(false);
				m_game->StartLevel();
				break;

			case PressResultState::ContinueLevel:
				m_updateState = UpdateEngineState::Dynamics;
				m_uiControl->HideGameInfoOverlay();
				m_controller->Active(true);
				m_game->ContinueGame();
				break;
			}
		}
		break;

	case UpdateEngineState::Dynamics:
		if (m_controller->IsPauseRequested() || m_pauseRequested)
		{
			//if (m_renderer->camera_mode == 0)
			//{
			m_pauseRequested = false;

			m_game->PauseGame();
			SetPause(true);
			SetGameInfoOverlay(GameInfoOverlayState::Pause);
			m_uiControl->SetAction(GameInfoOverlayCommand::TapToContinue);
			m_updateState = UpdateEngineState::WaitingForPress;
			m_pressResult = PressResultState::ContinueLevel;
			m_uiControl->ShowGameInfoOverlay();

			//}
		}
		else
		{
			GameState runState = m_game->RunGame();
			switch (runState)
			{
			case GameState::TimeExpired:
				m_uiControl->SetAction(GameInfoOverlayCommand::TapToContinue);
				SetGameInfoOverlay(GameInfoOverlayState::GameOverExpired);
				m_uiControl->ShowGameInfoOverlay();
				m_updateState = UpdateEngineState::WaitingForPress;
				m_pressResult = PressResultState::LoadGame;
				break;

			case GameState::LevelComplete:
				m_uiControl->SetAction(GameInfoOverlayCommand::PleaseWait);
				SetGameInfoOverlay(GameInfoOverlayState::LevelStart);
				m_uiControl->ShowGameInfoOverlay();
				m_updateState = UpdateEngineState::WaitingForResources;
				m_pressResult = PressResultState::PlayLevel;

				m_renderer->m_levelResourcesLoaded = false;

				m_renderer->UpdateRemovePhysics();

				m_game->LoadLevelAsync().then([this]()
				{
					critical_section::scoped_lock lock(m_criticalSection);
					m_game->FinalizeLoadLevel();
					m_updateState = UpdateEngineState::ResourcesLoaded;
					//PressComplete();
				}, task_continuation_context::use_current());
				break;

			case GameState::GameComplete:
				m_uiControl->SetAction(GameInfoOverlayCommand::TapToContinue);
				SetGameInfoOverlay(GameInfoOverlayState::GameOverCompleted);
				m_uiControl->ShowGameInfoOverlay();
				m_updateState = UpdateEngineState::WaitingForPress;
				m_pressResult = PressResultState::LoadGame;
				break;
			}
		}

		if (m_updateState == UpdateEngineState::WaitingForPress)
		{
			// Transitioning state, so enable waiting for the press event
			m_controller->WaitForPress();
		}
		if (m_updateState == UpdateEngineState::WaitingForResources)
		{
			// Transitioning state, so shut down the input controller until resources are loaded
			m_controller->Active(false);
		}
		break;
	}

	m_renderer->UpdatePhysics(timeDelta, timeTotal);
}

//--------------------------------------------------------------------------------------

void GameMain::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
}

//--------------------------------------------------------------------------------------

void GameMain::InitializeGameState()
{
	// Set up the initial state machine for handling Game playing state.
	if (m_game->GameActive() && m_game->LevelActive())
	{
		// The last time the game terminated it was in the middle
		// of a level.
		// We are waiting for the user to continue the game.
		m_updateState = UpdateEngineState::WaitingForResources;
		m_pressResult = PressResultState::ContinueLevel;
		SetGameInfoOverlay(GameInfoOverlayState::Pause);
		m_uiControl->SetAction(GameInfoOverlayCommand::PleaseWait);
	}
	else if (!m_game->GameActive() && (m_game->HighScore().totalHits > 0))
	{
		// The last time the game terminated the game had been completed.
		// Show the high score.
		// We are waiting for the user to acknowledge the high score and start a new game.
		// The level resources for the first level will be loaded later.
		m_updateState = UpdateEngineState::WaitingForPress;
		m_pressResult = PressResultState::LoadGame;
		SetGameInfoOverlay(GameInfoOverlayState::GameStats);
		m_controller->WaitForPress();
		m_uiControl->SetAction(GameInfoOverlayCommand::TapToContinue);
	}
	else
	{
		// This is either the first time the game has run or
		// the last time the game terminated the level was completed.
		// We are waiting for the user to begin the next level.43 hS
		m_updateState = UpdateEngineState::WaitingForResources;
		m_pressResult = PressResultState::PlayLevel;
		// SetGameInfoOverlay(GameInfoOverlayState::LevelStart);
		m_uiControl->SetAction(GameInfoOverlayCommand::PleaseWait);
	}
	m_uiControl->ShowGameInfoOverlay();
}

//--------------------------------------------------------------------------------------
/*
task<void> GameMain::Update(float timeDelta, float timeTotal)
{
	return create_task([this, timeDelta, timeTotal] {
	}).then([this,timeDelta,timeTotal]
	{
		return m_renderer->Update(timeDelta, timeTotal);
	});
}
*/

//--------------------------------------------------------------------------------------

void GameMain::CreateWindowSizeDependentResources()
{
	UpdateLayoutState();
	m_renderer->CreateWindowSizeDependentResources();

	// The location of the Control regions may have changed with the size change so update the controller.
	auto bounds = m_deviceResources->GetLogicalSize();

	m_controller->SetMoveRect(
		XMFLOAT2(0.0f, bounds.Height - GameConstants::TouchRectangleSize),
		XMFLOAT2(GameConstants::TouchRectangleSize, bounds.Height)
	);
	m_controller->SetFireRect(
		XMFLOAT2(bounds.Width - GameConstants::TouchRectangleSize, bounds.Height - GameConstants::TouchRectangleSize),
		XMFLOAT2(bounds.Width, bounds.Height)
	);

	if (m_renderLoopWorker == nullptr || m_renderLoopWorker->Status != AsyncStatus::Started)
	{
		StartRenderLoop();
	}
}

//--------------------------------------------------------------------------------------

void GameMain::OnDeviceLost()
{
	if (m_updateState == UpdateEngineState::Dynamics)
	{
		// Game is in the active game play state, Stop Game Timer and Pause play and save state
		m_game->PauseGame();
		SetPause(true);
	}
	StopRenderLoop();
	m_renderer->ReleaseDeviceDependentResources();
}

//--------------------------------------------------------------------------------------

void GameMain::OnDeviceRestored()
{
	m_renderer->CreateDeviceDependentResources();
	m_renderer->CreateWindowSizeDependentResources();

	m_uiControl->SetAction(GameInfoOverlayCommand::PleaseWait);
	SetGameInfoOverlay(GameInfoOverlayState::Loading);
	m_updateState = UpdateEngineState::WaitingForResources;

	if (m_renderLoopWorker == nullptr || m_renderLoopWorker->Status != AsyncStatus::Started)
	{
		StartRenderLoop();
	}

	create_task([this]()
	{
		return m_renderer->CreateGameDeviceResourcesAsync(m_game);
	}).then([this]()
	{
		// The finalize code needs to run in the same thread context
		// as the m_renderer object was created because the D3D device context
		// can ONLY be accessed on a single thread.
		{
			critical_section::scoped_lock lock(m_criticalSection);
			m_renderer->FinalizeCreateGameDeviceResources();
		}

		// Reset the main state machine based on the current game state now
		// that the device resources have been restored.
		InitializeGameState();

		if (m_updateState == UpdateEngineState::WaitingForResources)
		{
			// In the middle of a game so spin up the async task to load the level.
			return m_game->LoadLevelAsync().then([this]()
			{
				// The m_game object may need to deal with D3D device context work so
				// again the finalize code needs to run in the same thread
				// context as the m_renderer object was created because the D3D
				// device context can ONLY be accessed on a single thread.
				critical_section::scoped_lock lock(m_criticalSection);
				m_game->FinalizeLoadLevel();
				m_game->SetCurrentLevelToSavedState();
				m_updateState = UpdateEngineState::ResourcesLoaded;
			}, task_continuation_context::use_current());
		}
		else
		{
			// The game is not in the middle of a level so there aren't any level
			// resources to load.  Creating a no-op task so that in both cases
			// the same continuation logic is used.
			return create_task([]()
			{
			});
		}
	}, task_continuation_context::use_current()).then([this]()
	{
		// Since Device lost is an unexpected event, the app visual state
		// may be TooSmall or not have focus.  Put the state machine
		// into the correct state to reflect these cases.
		auto bounds = m_deviceResources->GetLogicalSize();
		if (bounds.Width < GameConstants::MinPlayableWidth)
		{
			m_updateStateNext = m_updateState;
			m_updateState = UpdateEngineState::TooSmall;
			m_controller->Active(false);
			m_uiControl->HideGameInfoOverlay();
			m_uiControl->ShowTooSmall();
		}
		else if (!m_haveFocus)
		{
			m_updateStateNext = m_updateState;
			m_updateState = UpdateEngineState::Deactivated;
			m_controller->Active(false);
			m_uiControl->SetAction(GameInfoOverlayCommand::None);
		}

		if (m_renderLoopWorker == nullptr || m_renderLoopWorker->Status != AsyncStatus::Started)
		{
			StartRenderLoop();
		}
	}, task_continuation_context::use_current());
}

//--------------------------------------------------------------------------------------

void GameMain::UpdateLayoutState()
{
	if (m_deviceResources->GetLogicalSize().Width < GameConstants::MinPlayableWidth)
	{
		switch (m_updateState)
		{
		case UpdateEngineState::Dynamics:
			// From Dynamic mode, when coming out of TooSmall layout rather than going directly back into game play
			// go to the paused state waiting for user input to continue
			m_updateStateNext = UpdateEngineState::WaitingForPress;
			m_pressResult = PressResultState::ContinueLevel;
			SetGameInfoOverlay(GameInfoOverlayState::Pause);
			m_uiControl->SetAction(GameInfoOverlayCommand::TapToContinue);
			m_game->PauseGame();
			SetPause(true);
			break;

		case UpdateEngineState::WaitingForResources:
			// There is outstanding async loading in progress.  This state change will be picked up after the
			// async task is completed.
			return;

		case UpdateEngineState::WaitingForPress:
			// Avoid corrupting the m_updateStateNext on a transition from TooSmall -> TooSmall.
			// Otherwise just cache the current state and return to it when leaving TooSmall layout

			m_updateStateNext = m_updateState;
			break;

		default:
			break;
		}

		m_updateState = UpdateEngineState::TooSmall;
		m_controller->Active(false);
		m_uiControl->HideGameInfoOverlay();
		m_uiControl->ShowTooSmall();
	}
	else
	{
		if (m_updateState == UpdateEngineState::TooSmall)
		{
			m_uiControl->HideTooSmall();
			m_uiControl->ShowGameInfoOverlay();

			if (m_haveFocus)
			{
				if (m_updateStateNext == UpdateEngineState::WaitingForPress)
				{
					m_uiControl->SetAction(GameInfoOverlayCommand::TapToContinue);
					m_controller->WaitForPress();
				}
				else if (m_updateStateNext == UpdateEngineState::WaitingForResources)
				{
					m_uiControl->SetAction(GameInfoOverlayCommand::PleaseWait);
				}

				m_updateState = m_updateStateNext;
			}
			else
			{
				m_updateState = UpdateEngineState::Deactivated;
				m_uiControl->SetAction(GameInfoOverlayCommand::None);
			}
		}
		else if (m_updateState == UpdateEngineState::Dynamics)
		{
			// In Dynamic mode, when a resize event happens, go to the paused state waiting for user input to continue.
			m_pressResult = PressResultState::ContinueLevel;
			SetGameInfoOverlay(GameInfoOverlayState::Pause);
			m_game->PauseGame();
			SetPause(true);
			if (m_haveFocus)
			{
				m_updateState = UpdateEngineState::WaitingForPress;
				m_uiControl->SetAction(GameInfoOverlayCommand::TapToContinue);
				m_controller->WaitForPress();
			}
			else
			{
				m_updateState = UpdateEngineState::Deactivated;
				m_uiControl->SetAction(GameInfoOverlayCommand::None);
			}
			m_uiControl->ShowGameInfoOverlay();
		}

		if (m_renderLoopWorker == nullptr || m_renderLoopWorker->Status != AsyncStatus::Started)
		{
			StartRenderLoop();
		}
	}
}

//--------------------------------------------------------------------------------------

void GameMain::ResetGame()
{
	m_updateState = UpdateEngineState::WaitingForResources;
	m_pressResult = PressResultState::PlayLevel;
	m_controller->Active(false);
	m_game->LoadGame();
	m_uiControl->SetAction(GameInfoOverlayCommand::PleaseWait);
	SetGameInfoOverlay(GameInfoOverlayState::LevelStart);
	m_uiControl->ShowGameInfoOverlay();
	m_renderer->m_levelResourcesLoaded = false;
	m_game->LoadLevelAsync().then([this]()
	{
		critical_section::scoped_lock lock(m_criticalSection);
		m_game->FinalizeLoadLevel();
		m_updateState = UpdateEngineState::ResourcesLoaded;
	}, task_continuation_context::use_current());
}

//--------------------------------------------------------------------------------------

void GameMain::SetGameInfoOverlay(GameInfoOverlayState state)
{
	m_gameInfoOverlayState = state;
	switch (state)
	{
	case GameInfoOverlayState::Loading:
		m_uiControl->SetGameLoading();
		break;
		//m_game->m_totalHits
	case GameInfoOverlayState::GameStats:
		m_uiControl->SetGameStats(
			1,
			m_game->HighScore().totalHits,
			m_game->HighScore().totalShots
		);
		break;

	case GameInfoOverlayState::LevelStart:
		m_uiControl->SetLevelStart(
			m_game->LevelCompleted(), "Collect",
			// m_game->CurrentLevel()->Objective(),
			/*m_game->CurrentLevel()->TimeLimit()*/ 2000.0f,
			m_game->BonusTime()
		);
		break;

	case GameInfoOverlayState::GameOverCompleted:
		m_uiControl->SetGameOver(
			true,
			m_game->LevelCompleted(),
			m_game->TotalHits(),
			m_game->TotalShots(),
			m_game->HighScore().totalHits
		);
		break;

	case GameInfoOverlayState::GameOverExpired:
		m_uiControl->SetGameOver(
			false,
			m_game->LevelCompleted(),
			m_game->TotalHits(),
			m_game->TotalShots(),
			m_game->HighScore().totalHits
		);
		break;

	case GameInfoOverlayState::Pause:
		m_uiControl->SetPause(
			m_game->LevelCompleted(),
			m_game->TotalHits(),
			m_game->TotalShots(),
			m_game->TimeRemaining()
		);
		break;
	}
}

//--------------------------------------------------------------------------------------

void GameMain::WindowActivationChanged(CoreWindowActivationState activationState)
{
	if (activationState == CoreWindowActivationState::Deactivated)
	{
		m_haveFocus = false;

		switch (m_updateState)
		{
		case UpdateEngineState::Dynamics:
			// From Dynamic mode, when coming out of Deactivated rather than going directly back into game play
			// go to the paused state waiting for user input to continue
			m_updateStateNext = UpdateEngineState::WaitingForPress;
			m_pressResult = PressResultState::ContinueLevel;
			SetGameInfoOverlay(GameInfoOverlayState::Pause);
			m_uiControl->ShowGameInfoOverlay();
			m_game->PauseGame();
			SetPause(true);
			m_updateState = UpdateEngineState::Deactivated;
			m_uiControl->SetAction(GameInfoOverlayCommand::None);
			break;

		case UpdateEngineState::WaitingForResources:
		case UpdateEngineState::WaitingForPress:
			m_updateStateNext = m_updateState;
			m_updateState = UpdateEngineState::Deactivated;
			m_uiControl->SetAction(GameInfoOverlayCommand::None);
			m_uiControl->ShowGameInfoOverlay();
			break;
		}
		// Don't have focus so shutdown input processing
		m_controller->Active(false);
	}
	else if (activationState == CoreWindowActivationState::CodeActivated
		|| activationState == CoreWindowActivationState::PointerActivated)
	{
		m_haveFocus = true;

		if (m_updateState == UpdateEngineState::Deactivated)
		{
			m_updateState = m_updateStateNext;

			if (m_updateState == UpdateEngineState::WaitingForPress)
			{
				m_uiControl->SetAction(GameInfoOverlayCommand::TapToContinue);
				m_controller->WaitForPress();
			}
			else if (m_updateStateNext == UpdateEngineState::WaitingForResources)
			{
				m_uiControl->SetAction(GameInfoOverlayCommand::PleaseWait);
			}

			if (m_renderLoopWorker == nullptr || m_renderLoopWorker->Status != AsyncStatus::Started)
			{
				StartRenderLoop();
			}
		}
	}
}

//--------------------------------------------------------------------------------------

void GameMain::Suspend()
{
	// Save application state.
	switch (m_updateState)
	{
	case UpdateEngineState::Dynamics:
		// Game is in the active game play state, Stop Game Timer and Pause play and save state
		m_uiControl->SetAction(GameInfoOverlayCommand::None);
		SetGameInfoOverlay(GameInfoOverlayState::Pause);
		m_updateStateNext = UpdateEngineState::WaitingForPress;
		m_pressResult = PressResultState::ContinueLevel;
		m_game->PauseGame();
		SetPause(true);
		break;

	case UpdateEngineState::WaitingForResources:
	case UpdateEngineState::WaitingForPress:
		m_updateStateNext = m_updateState;
		break;

	default:
		// any other state don't save as next state as they are trancient states and have already set m_updateStateNext
		break;
	}
	m_updateState = UpdateEngineState::Suspended;

	m_controller->Active(false);
	m_game->OnSuspending();
}

//--------------------------------------------------------------------------------------
void GameMain::SetPause(bool _bPause)
{
	//exit(0);
	bPaused = _bPause;
	m_renderer->SetPause(_bPause);
}

void GameMain::Resume()
{
	if (m_haveFocus)
	{
		m_updateState = m_updateStateNext;
	}
	else
	{
		m_updateState = UpdateEngineState::Deactivated;
	}

	if (m_updateState == UpdateEngineState::WaitingForPress)
	{
		m_uiControl->SetAction(GameInfoOverlayCommand::TapToContinue);
		m_controller->WaitForPress();
	}
	m_gameInfoOverlayState = GameInfoOverlayState::Loading;
	m_game->OnResuming();
	SetPause(false);
	m_uiControl->ShowGameInfoOverlay();
}

//--------------------------------------------------------------------------------------

void GameMain::SetBackground(unsigned int background)
{
	m_game->SetBackground(background);
}

//--------------------------------------------------------------------------------------

void GameMain::CycleBackground()
{
	/*
	m_game->CycleBackground();

	m_uiControl->SetAction(GameInfoOverlayCommand::PleaseWait);
	SetGameInfoOverlay(GameInfoOverlayState::Loading);
	m_updateState = UpdateEngineState::WaitingForResources;

	create_task([this]()
	{
		return m_renderer->LoadLevelResourcesAsync(m_game->GetCurrentlevel());
	}).then([this]()
	{
		// The finalize code needs to run in the same thread context
		// as the m_renderer object was created because the D3D device context
		// can ONLY be accessed on a single thread.
		critical_section::scoped_lock lock(m_criticalSection);
		m_renderer->FinalizeLoadLevelResources();
		m_updateState = UpdateEngineState::ResourcesLoaded;
	}, task_continuation_context::use_current());
	*/
}

//--------------------------------------------------------------------------------------

void GameMain::LicenseChanged(
	ListingInformation^ listing,
	LicenseInformation^ license
)
{
	m_listingInformation = listing;
	m_licenseInformation = license;

	m_game->UpdateGameConfig(m_licenseInformation);

	if (m_licenseInformation->IsActive)
	{
		if (m_licenseInformation->IsTrial)
		{
			m_renderer->Hud()->SetLicenseInfo("Trial Version");
		}
		else
		{
			m_renderer->Hud()->SetLicenseInfo("Full Version");
		}
	}
	else
	{
		m_renderer->Hud()->SetLicenseInfo("License Inactive - defaulting to Trial Version");
	}
}

//--------------------------------------------------------------------------------------