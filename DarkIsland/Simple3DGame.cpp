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
#include "GameRenderer.h"

#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "Level4.h"
#include "Level5.h"
#include "Level6.h"
#include "Animate.h"
#include "Sphere.h"
#include "Cylinder.h"
#include "Face.h"
#include "MediaReader.h"
#include "DefGame.h"

using namespace Windows::ApplicationModel::Store;
using namespace Windows::Storage;
using namespace Windows::UI::Core;

using namespace Game;

//----------------------------------------------------------------------

Simple3DGame::Simple3DGame(MoveLookController^ p_controller) :
	//m_ammoCount(0),
	//m_ammoNext(0),
	m_gameActive(false),
	m_levelActive(false),
	m_totalHits(0),
	m_totalShots(0),
	m_levelBonusTime(0.0),
	m_levelTimeRemaining(0.0),
	m_levelCount(0),
	m_currentLevel(START_LEVEL),
	m_activeBackground(0),
	m_controller(p_controller)
{
	m_topScore.totalHits = 0;
	m_topScore.totalShots = 0;
	m_topScore.levelCompleted = 0;
}

//----------------------------------------------------------------------

void Simple3DGame::Initialize(
	_In_ MoveLookController^ controller,
	_In_ GameRenderer* renderer
)
{
	// This method is expected to be called as an asynchronous task.
	// Care should be taken to not call rendering methods on the
	// m_renderer as this would result in the D3D Context being
	// used in multiple threads, which is not allowed.

	m_controller = controller;
	m_renderer = renderer;

	//m_audioController = ref new Audio;
	//m_audioController->CreateDeviceIndependentResources();

	//m_audioController->MusicEngine()
	InitializeGameConfig();

	m_savedState = ref new PersistentState();
	m_savedState->Initialize(ApplicationData::Current->LocalSettings->Values, "Game");

	m_timer = ref new GameTimer();

	// Create a sphere primitive to represent the player.
	// The sphere will be used to handle collisions and constrain the player in the world.
	// It is not rendered so it is not added to the list of render objects.
	// It is added to the object list so it will be included in intersection calculations.
	m_player = ref new Player(m_renderer->m_Res, m_renderer->m_Level);
	//m_player->MakePhysics();

	//m_objects.push_back(m_player);
	m_player->Active(true);

	//m_renderer->m_Res->m_Camera = new Camera(renderer->m_Res->m_deviceResources);

	m_controller->Pitch(m_renderer->m_Res->m_Camera->Pitch());
	m_controller->Yaw(m_renderer->m_Res->m_Camera->Yaw());

	// Min and max Bound are defining the world space of the game.
	// All camera motion and dynamics are confined to this space.
	m_minBound = XMFLOAT3(-400.0f, -300.0f, -600.0f);
	m_maxBound = XMFLOAT3(400.0f, 300.0f, 600.0f);

	MediaReader^ mediaReader = ref new MediaReader;
	//auto targetHitSound = mediaReader->LoadMedia("Assets\\hit.wav");

	// Instantiate a set of spheres to be used as ammunition for the game
	// and set the material properties of the spheres.
	//auto ammoHitSound = mediaReader->LoadMedia("Assets\\bounce.wav");

	m_gameModeKeyDown = false;
	m_gameSaveLevelKeyDown = false;
	/*
	for (int a = 0; a < GameConstants::MaxAmmo; a++)
	{
		m_ammo[a] = ref new Sphere;
		m_ammo[a]->Radius(GameConstants::AmmoRadius);
		m_ammo[a]->HitSound(ref new SoundEffect());
		m_ammo[a]->HitSound()->Initialize(
			m_audioController->SoundEffectEngine(),
			mediaReader->GetOutputWaveFormatEx(),
			ammoHitSound
			);
		m_ammo[a]->Active(false);
		m_renderObjects.push_back(m_ammo[a]);
	}
	*/

	soft_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	soft_acceleration = 0.0f;
	//}
	m_levelCount = 10;// static_cast<uint32>(m_level.size());

	// Load the top score from disk if it exists.
	LoadHighScore();

	// Load the currentScore for saved state if it exists.
	//LoadState();

	m_controller->Active(false);
}

//----------------------------------------------------------------------

void Simple3DGame::LoadGame()
{
	/**/
	//m_player->Position(XMFLOAT3 (0.0f, -1.3f, 4.0f));

	//m_renderer->m_Res->m_Camera->SetViewParams(
	//    m_player->Position(),            // Eye point in world coordinates.
	//    XMFLOAT3 (0.0f, 0.7f, 0.0f),     // Look at point in world coordinates.
	//    XMFLOAT3 (0.0f, 1.0f, 0.0f)      // The Up vector for the camera.
	//    );

	//m_controller->Pitch(m_renderer->m_Res->m_Camera->Pitch());
	//m_controller->Yaw(m_renderer->m_Res->m_Camera->Yaw());
	m_currentLevel = START_LEVEL;
	m_levelTimeRemaining = 0.0f;
	m_levelBonusTime = m_levelTimeRemaining;
	m_levelDuration = /*m_level[m_currentLevel]->TimeLimit()*/ 200.0f + m_levelBonusTime;
	//InitializeAmmo();
	m_totalHits = 0;
	m_totalShots = 0;
	m_gameActive = false;
	m_levelActive = false;
	m_timer->Reset();
}

//----------------------------------------------------------------------

task<void> Simple3DGame::LoadLevelAsync()
{
	m_renderer->m_levelResourcesLoaded = false;
	//m_renderer->UpdateRemovePhysics();

	// Initialize the level and spin up the async loading of the rendering
	// resources for the level.
	// This will run in a separate thread, so for Direct3D 11, only Device
	// methods are allowed.  Any DeviceContext method calls need to be
	// done in FinalizeLoadLevel.

	//m_player->Position(XML)
	//wait(GameConstants::LevelLoadingDelay);
	//m_level[m_currentLevel]->Initialize(m_objects);
	m_levelDuration = /*m_level[m_currentLevel]->TimeLimit()*/ 2000.0f + m_levelBonusTime;

	//m_audioController->

	return m_renderer->LoadLevelResourcesAsync(m_currentLevel);
}

//----------------------------------------------------------------------

void Simple3DGame::FinalizeLoadLevel()
{
	// This method is called on the main thread, so Direct3D 11 DeviceContext
	// method calls are allowable here.

	// Finalize the Level loading.
	m_renderer->FinalizeLoadLevelResources();

	m_player->Position(XMFLOAT3(m_renderer->m_Res->m_LevelInfo.player_start_x, m_renderer->m_Res->m_LevelInfo.player_start_y, m_renderer->m_Res->m_LevelInfo.player_start_z));
	m_player->MakePhysics();
	m_player->ResetVelocity();

	m_controller->SetYaw(m_renderer->m_Res->m_LevelInfo.player_start_angle);

	XMFLOAT3 cam_pos = XMFLOAT3(m_player->Position().x, m_player->Position().y + m_player->player_eye_height, m_player->Position().z);
	m_renderer->m_Res->m_Camera->Eye(cam_pos);

	m_renderer->m_Res->m_Camera->LookDirection(m_controller->LookDirection());
	m_renderer->m_Res->m_Camera->UpDirection(m_controller->UpDirection());
	m_renderer->m_Res->m_Camera->UpdateConstantBuffer();
	//UpdateDynamics();
}

//----------------------------------------------------------------------

void Simple3DGame::StartLevel()
{
	m_timer->Reset();
	m_timer->Start();
	if (m_currentLevel == START_LEVEL)
	{
		m_gameActive = true;
	}
	m_gameActive = true;
	//m_renderer->SetPause(true);
	m_levelActive = true;
	m_controller->Active(true);
}

//----------------------------------------------------------------------

void Simple3DGame::PauseGame()
{
	//bPaused = true;
	m_timer->Stop();
	SaveState();
}

//----------------------------------------------------------------------

void Simple3DGame::ContinueGame()
{
	m_timer->Start();
	m_controller->Active(true);
}

//----------------------------------------------------------------------

GameState Simple3DGame::RunGame()
{
	// This method is called to execute a single time interval for active game play.
	// It returns the resulting state of game play after the interval has been executed.

	m_timer->Update();

	//m_timer->

	m_levelTimeRemaining = m_levelDuration - m_timer->PlayingTime();

	if (m_levelTimeRemaining <= 0.0f)
	{
		// Time expired, so the game is over.
		m_levelTimeRemaining = 0.0f;
		InitializeAmmo();
		m_timer->Reset();
		m_gameActive = false;
		m_levelActive = false;
		SaveState();

		if (m_totalHits > m_topScore.totalHits)
		{
			// There is a new high score so save it.
			m_topScore.totalHits = m_totalHits;
			m_topScore.totalShots = m_totalShots;
			m_topScore.levelCompleted = m_currentLevel;

			SaveHighScore();
		}
		return GameState::TimeExpired;
	}
	else
	{
		UpdateDynamics();

		if (m_renderer->m_Res->camera_mode == 1 && m_renderer->bEditAsPlayer == false)
		{
			m_player->Position(XMFLOAT3(m_player->Position().x, m_renderer->camera_height, m_player->Position().z));
		}
		m_renderer->m_Res->m_Camera->Eye(XMFLOAT3(m_player->Position().x, m_player->Position().y + m_player->player_eye_height, m_player->Position().z));
		// Update the Camera with the player position updates from the dynamics calculations.

		m_renderer->m_Res->m_Camera->LookDirection(m_controller->LookDirection());
		m_renderer->m_Res->m_Camera->UpDirection(m_controller->UpDirection());
		//m_renderer->m_Res->m_Camera->buildWorldFrustumPlanes();
		m_renderer->m_LevelEdit->UpdateCurrentItemPointer(m_controller->GetWheelDelta());
		//m_totalHits = m_renderer->m_Res->m_Physics.m_rigidBodies.size();
		//m_main
		//m_renderer->m_Res->
		if (m_controller->CompleteLevel())// (m_level[m_currentLevel]->Update(m_timer->PlayingTime(), m_timer->DeltaTime(), m_levelTimeRemaining, m_objects))
		{
			m_controller->SetCompleteLevel(false);
			// The level has been completed.
			m_renderer->LevelLoading();
			m_levelActive = false;
			//InitializeAmmo();

			if (m_currentLevel < m_levelCount - 1)
			{
				// More levels to go so increment the level number.
				// Actual level loading will occur in the LoadLevelAsync / FinalizeLoadLevel
				// methods.
				m_timer->Reset();
				m_currentLevel++;
				m_levelBonusTime = m_levelTimeRemaining;
				SaveState();
				return GameState::LevelComplete;
			}
			else
			{
				// All levels have been completed.
				m_timer->Reset();
				m_gameActive = false;
				m_levelActive = false;
				SaveState();

				if (m_totalHits > m_topScore.totalHits)
				{
					// There is a new high score so save it.
					m_topScore.totalHits = m_totalHits;
					m_topScore.totalShots = m_totalShots;
					m_topScore.levelCompleted = m_currentLevel;

					SaveHighScore();
				}
				return GameState::GameComplete;
			}
		}
	}
	return GameState::Active;
}

//----------------------------------------------------------------------

void Simple3DGame::OnSuspending()
{
	// m_audioController->SuspendAudio();
}

//----------------------------------------------------------------------

void Simple3DGame::OnResuming()
{
	//m_audioController->ResumeAudio();
}

//----------------------------------------------------------------------

void Simple3DGame::UpdateDynamics()
{
	float timeTotal = m_timer->PlayingTime();
	float timeFrame = m_timer->DeltaTime();
	bool fire = m_controller->IsFiring();
	bool jump = m_controller->IsJumping();
	bool grab = m_controller->IsGrabbing();
	bool choose = m_controller->IsRightClick();

	if (m_controller->IsSwitching() == true)
		m_renderer->SwitchTorch();

	// Do collision detection of the player with the bounding world.
	XMFLOAT3 position = m_player->Position();
	XMFLOAT3 velocity = m_player->Velocity();
	//position.y += 3.0f;

	if (m_renderer->m_Res->camera_mode == 1)
	{
		if (fire == true)
		{
			m_renderer->m_LevelEdit->LeftMouse();
		}
		if (grab == true)
		{
			m_renderer->m_LevelEdit->MiddleMouse();
		}
		if (choose == true)
		{
			m_renderer->m_LevelEdit->RightMouse();
		}
	}

	if (grab == true && m_renderer->m_Res->camera_mode == 0)
	{
		float from_dir = 1.0f;
		float to_dir = m_renderer->m_Res->view_distance;
		btCollisionWorld::ClosestRayResultCallback RayCallbackB(btVector3(position.x + m_controller->LookDirection().x * from_dir, position.y + m_controller->LookDirection().y * from_dir, position.z + m_controller->LookDirection().z * from_dir), btVector3(position.x + m_controller->LookDirection().x * to_dir, position.y + m_controller->LookDirection().y * to_dir, position.z + m_controller->LookDirection().z * to_dir));
		//RayCallbackB.m_flags &= !btTriangleRaycastCallback::kF_FilterBackfaces;
		m_renderer->m_Res->m_Physics.m_dynamicsWorld->rayTest(btVector3(position.x + m_controller->LookDirection().x * from_dir, position.y + m_controller->LookDirection().y * from_dir, position.z + m_controller->LookDirection().z * from_dir), btVector3(position.x + m_controller->LookDirection().x * to_dir, position.y + m_controller->LookDirection().y * to_dir, position.z + m_controller->LookDirection().z * to_dir), RayCallbackB);
		if (RayCallbackB.hasHit())
		{
			//PhysicsData* pPhysicsData = reinterpret_cast<PhysicsData*>(RayCallbackB.m_collisionObject->getUserPointer());
			//btRigidBody* pBody = btRigidBody::upcast(RayCallbackB.m_collisionObject);

			btVector3 pos = RayCallbackB.m_hitPointWorld;

			//const btCollisionObject* col_object = RayCallbackB.m_collisionObject;

			btRigidBody* rig_body = (btRigidBody*)btRigidBody::upcast(RayCallbackB.m_collisionObject);

			if (false)
			{
				rig_body->activate(true);
				rig_body->applyForce(btVector3(m_controller->LookDirection().x, m_controller->LookDirection().y, m_controller->LookDirection().z), pos);
			}

			if (true)
			{
				rig_body->activate(true);
				btTransform tran = rig_body->getWorldTransform();
				//tran.setOrigin(btVector3(position.x + m_controller->LookDirection().x*2.0f, position.y + m_controller->LookDirection().y*2.0f, position.z + m_controller->LookDirection().z*2.0f));

				btVector3 vec_to = btVector3(position.x + m_controller->LookDirection().x*2.0f, position.y + m_player->player_eye_height + m_controller->LookDirection().y*2.0f, position.z + m_controller->LookDirection().z*2.0f);
				btVector3 vec_at = rig_body->getWorldTransform().getOrigin();
				btVector3 vec_dif = vec_to - vec_at;

				if (vec_dif.length() > 8.0f)
				{
					vec_dif = vec_dif.normalize()*5.0f;
					//vec_dif*00.3f;
				}

				if (vec_dif.length() > 1.0f)
				{
					//vec_dif.normalize();
					//vec_dif*=0.3f;
				}

				rig_body->setLinearVelocity(vec_dif*8.0f);

				if (true)
				{
					btQuaternion ang_to = btQuaternion(0.0f, 0.0f, 0.0f);
					btQuaternion ang_at = rig_body->getWorldTransform().getRotation();
					btVector3 ang_dif = vec_to - vec_at;

					btVector3 ang_vel = btVector3(-(ang_at.getX()*2.1f), -(ang_at.getY()*2.1f), -(ang_at.getZ()*2.1f));
					//if(ang_dif)
					rig_body->setAngularVelocity(ang_vel);
				}

				//rig_body->setWorldTransform(tran);
			}
			//rig_body->setLinearVelocity(btVector3(0.0f,3.0f,0.0f));
			//col_object->
			/*float dist = pos.getY() - model_matrix._24;
			if (dist < 0.3f)
			{
				bert_pos.y = pos.getY() - 0.3f;
				bert_mom.y = 0.0f;
			}
			*/
		}
	}

	//m_controller->AutoFire(true);
	
	if (m_controller->Velocity().x != 0.0f || m_controller->Velocity().z != 0.0f)
	{
		if (soft_acceleration < 1.0f)
			soft_acceleration += 0.2f;
		soft_velocity.y = m_controller->Velocity().y;
		soft_velocity.x = m_controller->Velocity().x * soft_acceleration;
		soft_velocity.z = m_controller->Velocity().z * soft_acceleration;
	}
	else
	{
		soft_acceleration = 0.0f;
		//if (soft_acceleration > 0.0f)
		//				soft_acceleration -= 0.01f;
		soft_velocity.x *= 0.5f;
		soft_velocity.z *= 0.5f;
	}

	soft_velocity.x *= 0.5f;
	soft_velocity.z *= 0.5f;

	/*x
	Buggy* p_Buggy = m_renderer->m_Buggy;
	
	if (m_controller->KeyState(Windows::System::VirtualKey::W) == true)
	{
		if (p_Buggy->carspeed < -0.5f)
			p_Buggy->Breaking();
		else
			p_Buggy->SetMotors(p_Buggy->cvalues.max_speed, p_Buggy->cvalues.torque);
	}
	*/



	/*
	if (m_controller->KeyState(Windows::System::VirtualKey::S) == true)
	{
		if (p_Buggy->carspeed > 0.5f)
			p_Buggy->Breaking();
		else
			p_Buggy->SetMotors((-p_Buggy->cvalues.max_speed*0.5f), -p_Buggy->cvalues.torque*0.5f);
	}
	*/
	/*
	if (m_Res->m_PadInput->LeftTrigger() > 0.0f)
	{
	if (carspeed > 0.5f)
	Breaking();
	else
	SetMotors((-cvalues.max_speed*0.3f)*m_Res->m_PadInput->LeftTrigger(), -cvalues.torque*0.3f);
	}
	*/
	if (m_controller->KeyState(Windows::System::VirtualKey::Number1) == true && ENABLE_EDIT_MODE == false)
	{
		m_renderer->m_GunBall->model_type = 0;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::Number2) == true && ENABLE_EDIT_MODE == false)
	{
		m_renderer->m_GunBall->model_type = 1;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::Number3) == true && ENABLE_EDIT_MODE == false)
	{
		m_renderer->m_GunBall->model_type = 2;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::Number4) == true && ENABLE_EDIT_MODE == false)
	{
		m_renderer->m_GunBall->model_type = 3;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::Number5) == true && ENABLE_EDIT_MODE == false)
	{
		m_renderer->m_GunBall->model_type = 4;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::Number6) == true && ENABLE_EDIT_MODE == false)
	{
		m_renderer->m_GunBall->model_type = 5;
	}

	//if (m_controller->KeyState(Windows::System::VirtualKey::F12) == true)
	//{
	//	exit(0);
	//}

	if (m_controller->KeyState(Windows::System::VirtualKey::E) == true)
	{
		m_renderer->camera_height += 0.3f;
		m_renderer->m_LevelEdit->camera_height = m_renderer->camera_height;
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Q) == true)
	{
		m_renderer->camera_height -= 0.3f;
		m_renderer->m_LevelEdit->camera_height = m_renderer->camera_height;
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Number3) == true)
	{
		if (ENABLE_EDIT_MODE)
		{
			if (m_gameSaveLevelKeyDown == false)
			{
				m_renderer->m_LevelEdit->SaveLevel();

				m_gameSaveLevelKeyDown = true;
			}
		}
	}
	else
	{
		m_gameSaveLevelKeyDown = false;
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Number2) == true)
	{
		if (m_gameModeKeyDown == false)
		{
			if (m_renderer->m_Res->camera_mode == 1)
			{
				m_renderer->m_Res->camera_mode = 0;
				m_renderer->m_Res->m_uiControl->SetControlsVisible(false);
				m_player->m_Body->setCollisionFlags(!btCollisionObject::CF_NO_CONTACT_RESPONSE);
				m_player->m_Body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
				btTransform tran = m_player->m_Body->getWorldTransform();
				tran.setOrigin(btVector3(m_renderer->m_Res->m_Camera->Eye().x, m_renderer->m_Res->m_Camera->Eye().y, m_renderer->m_Res->m_Camera->Eye().z));
				m_player->m_Body->setWorldTransform(tran);
				m_player->Velocity(XMFLOAT3(0.0f, 0.0f, 0.0f));
				m_player->Position(XMFLOAT3(m_renderer->m_Res->m_Camera->Eye().x, m_renderer->m_Res->m_Camera->Eye().y - m_player->player_eye_height, m_renderer->m_Res->m_Camera->Eye().z));
				m_controller->SetEditMode(false);
			}
			else
			{
				if (ENABLE_EDIT_MODE)
				{
					m_renderer->camera_height = m_player->Position().y;
					m_renderer->m_Stuff->ResetAllPositions();
					m_renderer->m_Res->camera_mode = 1;
					m_renderer->m_Res->m_uiControl->SetControlsVisible(true);
					m_controller->SetEditMode(true);
					m_player->m_Body->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
				}
			}

			m_gameModeKeyDown = true;
		}
	}
	else
	{
		m_gameModeKeyDown = false;
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Number7) == true)
	{
		m_renderer->player_mode = 0;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::Number8) == true)
	{
		m_renderer->player_mode = 1;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::Number9) == true)
	{
		m_renderer->player_mode = 2;
	}

	// car controls
	/*
	if (m_controller->KeyState(Windows::System::VirtualKey::A) == true)// || m_Res->m_PadInput->MoveCommandX()<-0.2f || bSBLeft == true)
	{
		if (p_Buggy->front_wheel_angle > -p_Buggy->cvalues.max_wheel_turn)
		{
			p_Buggy->SetFrontWheelAngle(p_Buggy->front_wheel_angle - ((p_Buggy->cvalues.max_wheel_turn*3.05f)* m_timer->DeltaTime()));
		}
	}
	else
	{
		if (m_controller->KeyState(Windows::System::VirtualKey::D) == true)// || m_Res->m_PadInput->MoveCommandX()>0.2f || bSBRight == true)
		{
			if (p_Buggy->front_wheel_angle < p_Buggy->cvalues.max_wheel_turn)
			{
				p_Buggy->SetFrontWheelAngle(p_Buggy->front_wheel_angle + ((p_Buggy->cvalues.max_wheel_turn*3.05f)* m_timer->DeltaTime()));
			}
		}
		else
		{
			if (p_Buggy->front_wheel_angle > 0.0f)
			{
				p_Buggy->front_wheel_angle -= 1.03f * m_timer->DeltaTime();
				if (p_Buggy->front_wheel_angle < 0.0f)
					p_Buggy->front_wheel_angle = 0.0f;
			}
			if (p_Buggy->front_wheel_angle < 0.0f)
			{
				p_Buggy->front_wheel_angle += 1.03f * m_timer->DeltaTime();
				if (p_Buggy->front_wheel_angle > 0.0f)
					p_Buggy->front_wheel_angle = 0.0f;
			}

			p_Buggy->SetFrontWheelAngle(p_Buggy->front_wheel_angle);
		}
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::R) == true)// || m_Res->m_PadInput->ButtonYwas() == true) && respot_delay <= 0.0f)
	{
		p_Buggy->respot_delay = 3.0f;
		if (m_renderer->m_Level->GetTerrainHeight(p_Buggy->player_x, p_Buggy->player_z) < 1.0)
		{
			for (int i = 0; i < 200; i++)
			{
				if (m_renderer->m_Level->GetTerrainHeight(m_renderer->m_Buggy->player_x - (float)i, m_renderer->m_Buggy->player_z)>1.0f)
				{
					i = i + 8;
					m_renderer->m_Buggy->SetCarPosition(m_renderer->m_Buggy->player_x - (float)i, m_renderer->m_Buggy->p_Level->GetTerrainHeight(m_renderer->m_Buggy->player_x - (float)i, m_renderer->m_Buggy->player_z) + 5.0f, m_renderer->m_Buggy->player_z, atan2f(m_renderer->m_Buggy->car_point.getX(), m_renderer->m_Buggy->car_point.getZ()), 0.0f, 0.0f);
					break;
				}
				if (m_renderer->m_Level->GetTerrainHeight(p_Buggy->player_x + (float)i, p_Buggy->player_z)>1.0f)
				{
					i = i + 8;
					p_Buggy->SetCarPosition(p_Buggy->player_x + (float)i, m_renderer->m_Level->GetTerrainHeight(p_Buggy->player_x + (float)i, p_Buggy->player_z) + 5.0f, p_Buggy->player_z, atan2f(p_Buggy->car_point.getX(), p_Buggy->car_point.getZ()), 0.0f, 0.0f);
					break;
				}
				if (m_renderer->m_Level->GetTerrainHeight(p_Buggy->player_x, p_Buggy->player_z - (float)i)>1.0f)
				{
					i = i + 8;
					p_Buggy->SetCarPosition(p_Buggy->player_x, m_renderer->m_Level->GetTerrainHeight(p_Buggy->player_x, p_Buggy->player_z - (float)i) + 5.0f, p_Buggy->player_z - (float)i, atan2f(p_Buggy->car_point.getX(), p_Buggy->car_point.getZ()), 0.0f, 0.0f);
					break;
				}
				if (m_renderer->m_Level->GetTerrainHeight(p_Buggy->player_x, p_Buggy->player_z + (float)i)>1.0f)
				{
					i = i + 8;
					p_Buggy->SetCarPosition(p_Buggy->player_x, m_renderer->m_Level->GetTerrainHeight(p_Buggy->player_x, p_Buggy->player_z + (float)i) + 5.0f, p_Buggy->player_z + (float)i, atan2f(p_Buggy->car_point.getX(), p_Buggy->car_point.getZ()), 0.0f, 0.0f);
					break;
				}
			}
		}
		else
		{
			p_Buggy->SetCarPosition(p_Buggy->player_x, m_renderer->m_Level->GetTerrainHeight(p_Buggy->player_x, p_Buggy->player_z) + 5.0f, p_Buggy->player_z, atan2f(p_Buggy->car_point.getX(), p_Buggy->car_point.getZ()), 0.0f, 0.0f);
		}
	}
	else
	{
		p_Buggy->respot_delay -= m_timer->DeltaTime();
	}
	*/

	Bert* p_Bert = m_renderer->m_Bert;
	p_Bert->bKeyDown = false;
	p_Bert->bAttack = false;

	/*
	if (m_controller->KeyState(Windows::System::VirtualKey::Space) == true);// || m_Res->m_PadInput->ButtonX() == true || bTouchAttack == true)
	{
		if (p_Bert->bAttack == false)
		{
			p_Bert->m_skinnedMeshRenderer.ResetAnimTimer(3);
			p_Bert->bAttack = true;
			p_Bert->attack_timer = 0.0f;
		}
	}
	*/
	p_Bert->x_motion = 0.0f;
	p_Bert->z_motion = 0.0f;

	if (m_controller->KeyState(Windows::System::VirtualKey::W) == true)// && p_Bert->bAttack == false)
	{
		p_Bert->z_motion += 10.0f;
		p_Bert->bKeyDown = true;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::S) == true)// && p_Bert->bAttack == false)
	{
		p_Bert->z_motion -= 10.0f;
		p_Bert->bKeyDown = true;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::A) == true)// && p_Bert->bAttack == false)
	{
		if (p_Bert->x_motion_to < 0.0f)
			p_Bert->x_motion_to = 0.0f;
		p_Bert->x_motion_to += 0.3f;
		if (p_Bert->x_motion_to > 3.0f)
			p_Bert->x_motion_to = 3.0f;
		p_Bert->x_motion += p_Bert->x_motion_to;
		//x_motion += 5.0f;
		p_Bert->bKeyDown = true;
	}

	//m_Res->m_PadwddInput->
	if (m_controller->KeyState(Windows::System::VirtualKey::D) == true)// && p_Bert->bAttack == false)
	{
		if (p_Bert->x_motion_to > 0.0f)
			p_Bert->x_motion_to = 0.0f;
		p_Bert->x_motion_to -= 0.3f;
		if (p_Bert->x_motion_to < -3.0f)
			p_Bert->x_motion_to = -3.0f;
		p_Bert->x_motion += p_Bert->x_motion_to;
		//x_motion -= 5.0f;
		p_Bert->bKeyDown = true;
	}
	p_Bert->y_motion = 0.0f;

	p_Bert->x_motion_to *= 0.1f;

	if (m_controller->KeyState(Windows::System::VirtualKey::R) == true)
	{
		if (p_Bert->view_pos_y < 10.0f)
		{
			p_Bert->view_pos_y += 0.1f;
		}
		p_Bert->bKeyDown = true;
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::F) == true)
	{
		if (p_Bert->view_pos_y > 0.0f)
		{
			p_Bert->view_pos_y -= 0.1f;
		}
		p_Bert->bKeyDown = true;
	}
	m_player->player_eye_height = 1.0f;
	if (false)//(m_controller->KeyState(Windows::System::VirtualKey::C, false) == true)
	{
		m_player->player_eye_height = 1.0f;
	}
	else
	{
		m_player->player_eye_height = 2.0f;
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Shift) == true)// || m_controller->ButtonA() == true || p_Bert->touch_jump == true)
	{
		if (p_Bert->bJump == false && p_Bert->bWorldContact == true)
		{
			p_Bert->bJump = true;
			p_Bert->touch_jump = false;
			p_Bert->y_motion = 10.0f;
			p_Bert->bert_mom.y = 0.2f;
			//m_Res->m_audio.PlaySoundEffect(SFXJump);
			p_Bert->m_skinnedMeshRenderer.ResetAnimTimer(2);
		}
	}
	else
	{
		p_Bert->bJump = false;
	}

	/*
	if ((m_controller->MoveCommandX() != 0.0f || m_Res->m_PadInput->MoveCommandY() != 0.0f) && bAttack == false)
	{
		x_motion = -m_Res->m_PadInput->MoveCommandX();
		z_motion = m_Res->m_PadInput->MoveCommandY();
		bKeyDown = true;
	}
	*/

	// Time has not expired, so run one frame of game play.
	m_player->Velocity(soft_velocity);
	m_renderer->m_Res->m_Camera->LookDirection(m_controller->LookDirection());

	// If the elapsed time is too long, we slice up the time and handle physics over several
	// smaller time steps to avoid missing collisions.
	float timeLeft = timeFrame;
	float elapsedFrameTime;

	if (jump == true)
	{
		m_player->Jump();
	}

	if (fire == true && m_renderer->m_Res->camera_mode == 0)
	{
		m_renderer->m_GunBall->CreateOne(XMFLOAT3(position.x + (m_controller->LookDirection().x*3.0f), position.y + m_player->player_eye_height + (m_controller->LookDirection().y*3.0f), position.z + (m_controller->LookDirection().z*3.0f)), XMFLOAT3((m_controller->LookDirection().x*5.0f) + velocity.x, (m_controller->LookDirection().y*5.0f) + velocity.y, (m_controller->LookDirection().z*5.0f) + velocity.z));
	}
}

//----------------------------------------------------------------------

void Simple3DGame::SaveState()
{
	// Save basic state of the game.
	m_savedState->SaveBool(":GameActive", m_gameActive);
	m_savedState->SaveBool(":LevelActive", m_levelActive);
	m_savedState->SaveInt32(":LevelCompleted", m_currentLevel);
	m_savedState->SaveInt32(":TotalShots", m_totalShots);
	m_savedState->SaveInt32(":TotalHits", m_totalHits);
	m_savedState->SaveSingle(":BonusRoundTime", m_levelBonusTime);
	m_savedState->SaveXMFLOAT3(":PlayerPosition", m_player->Position());
	m_savedState->SaveXMFLOAT3(":PlayerLookDirection", m_controller->LookDirection());

	if (m_levelActive)
	{
		// The game is currently in the middle of a level, so save the extended state of
		// the game.
		m_savedState->SaveSingle(":LevelDuration", m_levelDuration);
		m_savedState->SaveSingle(":LevelPlayingTime", m_timer->PlayingTime());

		//m_savedState->SaveInt32(":AmmoCount", m_ammoCount);
		//m_savedState->SaveInt32(":AmmoNext", m_ammoNext);

		const int bufferLength = 16;
		char16 str[bufferLength];

		/*
		for (uint32 i = 0; i < m_ammoCount; i++)
		{
			int len = swprintf_s(str, bufferLength, L"%d", i);
			Platform::String^ string = ref new Platform::String(str, len);

			m_savedState->SaveBool(Platform::String::Concat(":AmmoActive", string), m_ammo[i]->Active());
			m_savedState->SaveXMFLOAT3(Platform::String::Concat(":AmmoPosition", string), m_ammo[i]->Position());
			m_savedState->SaveXMFLOAT3(Platform::String::Concat(":AmmoVelocity", string), m_ammo[i]->Velocity());
		}
		*/

		//m_level[m_currentLevel]->SaveState(m_savedState);
	}
}

//----------------------------------------------------------------------

void Simple3DGame::LoadState()
{
	return;

	m_gameActive = m_savedState->LoadBool(":GameActive", m_gameActive);
	m_levelActive = m_savedState->LoadBool(":LevelActive", m_levelActive);

	if (m_gameActive)
	{
		// Loading from the last known state means the game wasn't finished when it was last played,
		// so set the current level.

		m_totalShots = m_savedState->LoadInt32(":TotalShots", 0);
		m_totalHits = m_savedState->LoadInt32(":TotalHits", 0);
		m_currentLevel = START_LEVEL;// m_savedState->LoadInt32(":LevelCompleted", 0);
		m_levelBonusTime = m_savedState->LoadSingle(":BonusRoundTime", 0.0f);

		m_levelTimeRemaining = m_levelBonusTime;

		// Reload the current player position and set both the camera and the controller
		// with the current Look Direction.
	   /* m_player->Position(
			m_savedState->LoadXMFLOAT3(":PlayerPosition", XMFLOAT3(0.0f, 0.0f, 0.0f))
			);*/

			/*
			m_renderer->m_Res->m_Camera->Eye(m_player->Position());
			m_renderer->m_Res->m_Camera->LookDirection(
				m_savedState->LoadXMFLOAT3(":PlayerLookDirection", XMFLOAT3(0.0f, 0.0f, 1.0f))
				);
			m_controller->Pitch(m_renderer->m_Res->m_Camera->Pitch());
			m_controller->Yaw(m_renderer->m_Res->m_Camera->Yaw());
			*/
	}
	else
	{
		// The game was not being played when it was last saved, so initialize to the beginning.
		m_currentLevel = START_LEVEL;
		m_levelBonusTime = 0;
	}

	if (m_currentLevel >= m_levelCount)
	{
		// The current level is not valid so, reset to a known state and abandon the current game.
		m_currentLevel = START_LEVEL;
		m_levelBonusTime = 0;
		m_gameActive = false;
		m_levelActive = false;
	}
}

//----------------------------------------------------------------------

void Simple3DGame::SetCurrentLevelToSavedState()
{
	if (m_gameActive)
	{
		if (m_levelActive)
		{
			// Middle of a level so restart where left off.
			m_levelDuration = m_savedState->LoadSingle(":LevelDuration", 0.0f);

			m_timer->Reset();
			m_timer->PlayingTime(m_savedState->LoadSingle(":LevelPlayingTime", 0.0f));

			//m_ammoCount = m_savedState->LoadInt32(":AmmoCount", 0);

			//m_ammoNext = m_savedState->LoadInt32(":AmmoNext", 0);

			const int bufferLength = 16;
			char16 str[bufferLength];

			/*
			for (uint32 i = 0; i < m_ammoCount; i++)
			{
				int len = swprintf_s(str, bufferLength, L"%d", i);
				Platform::String^ string = ref new Platform::String(str, len);

				m_ammo[i]->Active(
					m_savedState->LoadBool(
						Platform::String::Concat(":AmmoActive", string),
						m_ammo[i]->Active()
						)
					);
				if (m_ammo[i]->Active())
				{
					m_ammo[i]->OnGround(false);
				}

				m_ammo[i]->Position(
					m_savedState->LoadXMFLOAT3(
						Platform::String::Concat(":AmmoPosition", string),
						m_ammo[i]->Position()
						)
					);

				m_ammo[i]->Velocity(
					m_savedState->LoadXMFLOAT3(
						Platform::String::Concat(":AmmoVelocity", string),
						m_ammo[i]->Velocity()
						)
					);
			}
			*/
			m_levelTimeRemaining = 2000.0f + m_levelBonusTime - m_timer->PlayingTime();
		}
	}
}

//----------------------------------------------------------------------

void Simple3DGame::SaveHighScore()
{
	m_savedState->SaveInt32(":HighScore:LevelCompleted", m_topScore.levelCompleted);
	m_savedState->SaveInt32(":HighScore:TotalShots", m_topScore.totalShots);
	m_savedState->SaveInt32(":HighScore:TotalHits", m_topScore.totalHits);
}

//----------------------------------------------------------------------

void Simple3DGame::LoadHighScore()
{
	m_topScore.levelCompleted = m_savedState->LoadInt32(":HighScore:LevelCompleted", 0);
	m_topScore.totalShots = m_savedState->LoadInt32(":HighScore:TotalShots", 0);
	m_topScore.totalHits = m_savedState->LoadInt32(":HighScore:TotalHits", 0);
}

//----------------------------------------------------------------------

void Simple3DGame::InitializeAmmo()
{
	// m_ammoCount = 0;
	//// m_ammoNext = 0;
	// for (uint32 i = 0; i < GameConstants::MaxAmmo; i++)
	// {
	//     m_ammo[i]->Active(false);
	// }
}

//----------------------------------------------------------------------

void Simple3DGame::InitializeGameConfig()
{
	m_gameConfig.isTrial = false;
	m_gameConfig.autoFire = false;
	m_controller->AutoFire(false);
	m_activeBackground = 0;
	m_gameConfig.backgroundAvailable[0] = true;
	for (int i = 1; i < GameConstants::MaxBackgroundTextures; i++)
	{
		m_gameConfig.backgroundAvailable[i] = false;
	}
}

//--------------------------------------------------------------------------------------

void Simple3DGame::UpdateGameConfig(LicenseInformation^ licenseInformation)
{
	if (false)//(licenseInformation->IsActive)
	{
		m_gameConfig.isTrial = licenseInformation->IsTrial;
		if (!m_gameConfig.isTrial && licenseInformation->ProductLicenses->Lookup("AutoFire")->IsActive)
		{
			m_gameConfig.autoFire = true;
			m_controller->AutoFire(true);
		}
		else
		{
			m_gameConfig.autoFire = false;
			m_controller->AutoFire(false);
		}
		if (!m_gameConfig.isTrial && licenseInformation->ProductLicenses->Lookup("NightBackground")->IsActive)
		{
			m_gameConfig.backgroundAvailable[1] = true;
		}
		else
		{
			m_gameConfig.backgroundAvailable[1] = false;
		}
		if (!m_gameConfig.isTrial && licenseInformation->ProductLicenses->Lookup("DayBackground")->IsActive)
		{
			m_gameConfig.backgroundAvailable[2] = true;
		}
		else
		{
			m_gameConfig.backgroundAvailable[2] = false;
		}
	}
	else
	{
		// If no active license then default back to trial version.
		InitializeGameConfig();
	}

	if (m_gameConfig.isTrial)
	{
		//if (m_level.size() > 2)
		//{
	   //     m_level.erase(m_level.begin() + 2, m_level.end());
	   // }
	}
	else
	{
		//if (m_level.size() == 2)
		//{
		//    m_level.push_back(ref new Level3);
		//    m_level.push_back(ref new Level4);
		//    m_level.push_back(ref new Level5);
		//    m_level.push_back(ref new Level6);
		//}
	}
	m_levelCount = 10;// static_cast<uint32>(m_level.size());
}

//--------------------------------------------------------------------------------------

void Simple3DGame::SetBackground(uint32 background)
{
	if (background < GameConstants::MaxBackgroundTextures)
	{
		if (m_gameConfig.backgroundAvailable[background])
		{
			m_activeBackground = background;
			//m_renderer->SetBackground(background);
		}
	}
}

//--------------------------------------------------------------------------------------

void Simple3DGame::CycleBackground()
{
	// There may be gaps in the background textures list that are available for use because the
	// user has not purchased all them in order or at all.
	// Cycle through the list looking for the next one that is valid.  This may require wrapping
	// back to the beginning.

	unsigned int newBackground = m_activeBackground;

	while (newBackground < GameConstants::MaxBackgroundTextures)
	{
		newBackground++;
		if (newBackground >= GameConstants::MaxBackgroundTextures)
		{
			newBackground = 0;
			break;
		}
		else if (m_gameConfig.backgroundAvailable[newBackground])
		{
			break;
		}
	}

	if (newBackground != m_activeBackground)
	{
		m_activeBackground = newBackground;
		//m_renderer->SetBackground(newBackground);
	}
}
//--------------------------------------------------------------------------------------