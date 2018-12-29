#pragma once

#include "GameConstants.h"
#include "GameUIConstants.h"
#include "Audio.h"
#include "Camera.h"

#include "GameTimer.h"
#include "MoveLookController.h"
#include "PersistentState.h"
#include "Player.h"

#include "GameRenderer.h"

//--------------------------------------------------------------------------------------

enum class GameState
{
	Waiting,
	Active,
	LevelComplete,
	TimeExpired,
	GameComplete,
};

typedef struct
{
	Platform::String^ tag;
	int totalHits;
	int totalShots;
	int levelCompleted;
} HighScoreEntry;

typedef std::vector<HighScoreEntry> HighScoreEntries;

typedef struct
{
	bool isTrial;
	bool autoFire;
	bool backgroundAvailable[GameConstants::MaxBackgroundTextures];
} GameConfig;

//--------------------------------------------------------------------------------------

namespace Game
{
	class GameRenderer;

	ref class Simple3DGame
	{
	internal:
		Simple3DGame(MoveLookController^ p_controller);

		void Initialize(
			_In_ MoveLookController^ controller,
			_In_ GameRenderer* renderer
		);

		void LoadGame();
		task<void> LoadLevelAsync();
		void FinalizeLoadLevel();
		void StartLevel();
		void PauseGame();
		void ContinueGame();
		GameState RunGame();
		void SetCurrentLevelToSavedState();

		void OnSuspending();
		void OnResuming();

		bool IsActivePlay() { return m_timer->Active(); }
		bool IsTrial() { return m_gameConfig.isTrial; }
		int LevelCompleted() { return m_currentLevel; };
		int TotalShots() { return m_totalShots; };
		int TotalHits(int t_hits = -1) { if (t_hits > -1) m_totalHits = t_hits; return m_totalHits; };
		float BonusTime() { return m_levelBonusTime; };
		bool GameActive() { return m_gameActive; };
		bool LevelActive() { return m_levelActive; };
		HighScoreEntry HighScore() { return m_topScore; };
		//GameLevel^ CurrentLevel() { return m_level[m_currentLevel]; };
		float TimeRemaining() { return m_levelTimeRemaining; };
		//Camera* GameCamera() { return m_renderer->m_Res->m_Camera; };

		Player^ GamePlayer() {
			return m_player;
		}

		void UpdateGameConfig(Windows::ApplicationModel::Store::LicenseInformation^ licenseInformation);
		void SetBackground(uint32 background);
		void CycleBackground();
		uint32 GetCurrentlevel() {
			return m_currentLevel;
		}

		MoveLookController^ GetController() { return m_controller; }

	private:
		void LoadState();
		void SaveState();
		void SaveHighScore();
		void LoadHighScore();
		void InitializeAmmo();
		void UpdateDynamics();
		void InitializeGameConfig();

		XMFLOAT3 soft_velocity;
		float soft_acceleration;

		MoveLookController^                         m_controller;
		GameRenderer*                               m_renderer;

		//Camera*                                   m_camera;

		//Audio*                                    m_audioController;

		//std::vector<Sphere^>                        m_ammo;
		//uint32                                      m_ammoCount;
		//uint32                                      m_ammoNext;

		HighScoreEntry                              m_topScore;
		PersistentState^                            m_savedState;

		bool										m_gameModeKeyDown;
		bool										m_gameSaveLevelKeyDown;

		GameTimer^                                  m_timer;
		bool                                        m_gameActive;
		bool                                        m_levelActive;
		int                                         m_totalHits;
		int                                         m_totalShots;
		float                                       m_levelDuration;
		float                                       m_levelBonusTime;
		float                                       m_levelTimeRemaining;
		//std::vector<GameLevel^>                     m_level;
		uint32                                      m_levelCount;
		uint32                                      m_currentLevel;

		//btRigidBody*								rig_body;

		GameConfig                                  m_gameConfig;
		uint32                                      m_activeBackground;

		Player^                                     m_player;

		XMFLOAT3                           m_minBound;
		XMFLOAT3                           m_maxBound;
	};
}