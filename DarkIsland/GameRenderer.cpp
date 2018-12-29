#include "pch.h"
#include "GameRenderer.h"
#include "ConstantBuffers.h"
#include "TargetTexture.h"

#include "CylinderMesh.h"
#include "FaceMesh.h"
#include "SphereMesh.h"
#include "WorldMesh.h"
#include "Face.h"
#include "Sphere.h"
#include "Cylinder.h"
#include "windows.ui.xaml.media.dxinterop.h"
#include "DefGame.h"

using namespace Game;

GameRenderer::GameRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources, Simple3DGame^ p_game, Game::IGameUIControl^ UIControl, DX::StepTimer* pp_timer) :
	m_deviceResources(deviceResources),
	m_initialized(false),
	m_gameResourcesLoaded(false),
	bLoadingComplete(false),
	m_levelResourcesLoaded(false),
	m_game(p_game)
{
	m_gameHud = ref new GameHud(deviceResources, "", "", "");
	m_Res = new AllResources(deviceResources, UIControl, pp_timer);
	m_Res->p_controller = p_game->GetController();

	bPaused = false;
	bPauseFly = false;
	bEditAsPlayer = false;

	bDefGrass = false; // true is faster
	
	bTorchOn = true;
}

void GameRenderer::SetPause(bool _bPause)
{
	bPaused = _bPause;
	if (bPaused == true)
		m_LevelEdit->GetCameraPath()->Reset();
}

void GameRenderer::UpdateCurrentItemPointer(int pointer_delta)
{
	model_pointer_id += pointer_delta;

	if (model_pointer_id < 0)
		model_pointer_id = 0;

	if (model_pointer_id > m_LevelEdit->max_model_pointer_id - 1)
		model_pointer_id = m_LevelEdit->max_model_pointer_id;

	//m_GunBall->model_type = model_pointer_id;
}

void GameRenderer::Initialize()
{
	m_Sky = new Sky(m_Res);
	m_Fire = new Fire(m_Res);
	m_Snow = new Snow(m_Res);
	m_Rain = new Rain(m_Res);
	m_Fog = new Fog(m_Res);
	m_Level = new Level(m_Res);
	m_Grass = new Grass(m_Res);
	m_Stuff = new Stuff(m_Res);
	m_Statics = new Statics(m_Res, m_Level);
	m_Car = new Car(m_Res);
	m_Buggy = new Buggy(m_Res);
	m_Buggy->Initialize(m_Level, START_BUGGY);
	m_GunBall = new GunBall(m_Res, m_Stuff);
	m_PanCamera = new PanCamera(m_Res->m_Camera, m_Level);
	m_LevelEdit = new LevelEdit(m_Res, m_game->GetController(), m_Level, m_Statics, m_Stuff);

	m_Bert = new Bert(m_Res, m_Level);
	m_Bert->Initialize(START_CHAR);

	render_delay = 0;

	player_mode = 0;

	m_FXAATexture = new RenderTextureClass;

	m_DeferredTexture = new RenderTextureClass;
	m_waterTexture = new RenderTextureClass;
	m_GlowTexture = new RenderTextureClass;
	m_GlowTextureDownsample = new RenderTextureClass;
	m_GlowTextureDownsample2 = new RenderTextureClass;

	m_Res->camera_mode = 0;
	camera_height = 12.0f;

	bMusicStarted = false;

	m_Level->InitializeTerrainVariables(32, 32, 0.1f);
	m_Level->InitializeGeneratedTerrain(2, 0.04f, 0.0f);

	m_Res->total_terrrain_x_points = m_Level->total_x_points;
	m_Res->total_terrrain_y_points = m_Level->total_y_points;

	m_Fire->Initialize(m_Level);
	m_Snow->Initialize(m_Level, true);
	m_Rain->Initialize(m_Level);
	m_Fog->Initialize(m_Level, true);
	m_Grass->Initialize(m_Level);
	m_Statics->Initialize(m_Fire);
	m_Sky->Initialize(SKY_SIZE, 0.0f);
}

void GameRenderer::ReleaseDeviceDependentResources()
{
	m_game = nullptr;
	m_gameHud->ReleaseDeviceDependentResources();
}

void GameRenderer::CreateDeviceDependentResources()
{
	m_Res->CreateDeviceDependentResources();
	Initialize();
	m_gameHud->CreateDeviceDependentResources();
}

void GameRenderer::CreateWindowSizeDependentResources()
{
	auto d3dDevice = m_deviceResources->GetD3DDevice();

	outputSize = m_deviceResources->GetOutputSize();

	if (screen_size_buffer.height_size == outputSize.Height / GLOW_SCALE &&
		screen_size_buffer.width_size == outputSize.Width / GLOW_SCALE)
		return;

	screen_size_buffer.height_size = outputSize.Height / GLOW_SCALE;
	screen_size_buffer.width_size = outputSize.Width / GLOW_SCALE;

	if (screen_size_buffer.height_size == 0.0f || screen_size_buffer.width_size == 0.0f)
		return;

	m_gameHud->CreateWindowSizeDependentResources();

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	m_deviceResources->GetD3DDeviceContext()->Map(m_Res->g_pcbFXAA, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CB_FXAA* pFXAA = (CB_FXAA*)MappedResource.pData;
	float frameWidth = outputSize.Width;
	float frameHeight = outputSize.Height;
	pFXAA->m_fxaa = XMFLOAT4(1.0f / frameWidth, 1.0f / frameHeight, 0.0f, 0.0f);
	m_deviceResources->GetD3DDeviceContext()->Unmap(m_Res->g_pcbFXAA, 0);

	//d3dContext->VSSetConstantBuffers(1, 1, &m_Res->g_pcbFXAA);
	m_deviceResources->GetD3DDeviceContext()->PSSetConstantBuffers(10, 1, &m_Res->g_pcbFXAA);

	//m_Res->m_FXAA->Shutdown();
	//m_Res->m_FXAA->FxaaIntegrateResource();

	m_DeferredTexture->Shutdown();
	m_DeferredTexture->Initialize(d3dDevice, (UINT)outputSize.Width, (UINT)outputSize.Height, DXGI_FORMAT_B8G8R8A8_UNORM);

	m_FXAATexture->Shutdown();
	m_FXAATexture->Initialize(d3dDevice, (UINT)outputSize.Width, (UINT)outputSize.Height, DXGI_FORMAT_B8G8R8A8_UNORM);

	m_Res->m_FullScreenWindow->Shutdown();
	m_Res->m_FullScreenWindow->Initialize(d3dDevice, (UINT)outputSize.Width, (UINT)outputSize.Height);

	m_Res->m_DownsampleWindow->Shutdown();
	m_Res->m_DownsampleWindow->Initialize(d3dDevice, (UINT)outputSize.Width / GLOW_SCALE, (UINT)outputSize.Height / GLOW_SCALE);

	m_waterTexture->Shutdown();
	m_waterTexture->Initialize(d3dDevice, (UINT)outputSize.Width, (UINT)outputSize.Height, DXGI_FORMAT_B8G8R8A8_UNORM);

	m_GlowTexture->Shutdown();
	m_GlowTexture->Initialize(d3dDevice, (UINT)outputSize.Width, (UINT)outputSize.Height, DXGI_FORMAT_B8G8R8A8_UNORM, GLOW_MIPS);
	m_GlowTextureDownsample->Shutdown();
	m_GlowTextureDownsample->Initialize(d3dDevice, (UINT)outputSize.Width / GLOW_SCALE, (UINT)outputSize.Height / GLOW_SCALE, DXGI_FORMAT_B8G8R8A8_UNORM);
	m_GlowTextureDownsample2->Shutdown();
	m_GlowTextureDownsample2->Initialize(d3dDevice, (UINT)outputSize.Width / GLOW_SCALE, (UINT)outputSize.Height / GLOW_SCALE, DXGI_FORMAT_B8G8R8A8_UNORM);

	m_Res->m_DeferredBuffers->Shutdown();
	m_Res->m_DeferredBuffers->Initialize(d3dDevice, (UINT)outputSize.Width, (UINT)outputSize.Height, 400.0f, 0.1f);

	m_Res->m_RDeferredBuffers->Shutdown();
	m_Res->m_RDeferredBuffers->Initialize(d3dDevice, (UINT)outputSize.Width, (UINT)outputSize.Height, 400.0f, 0.1f);

	m_Res->m_Camera->CreateWindowSizeDependentResources();

	m_Res->UpdateScreenSizeConstants(screen_size_buffer);

	m_Res->m_Lights->m_lightBufferData.screen_width = outputSize.Width;
	m_Res->m_Lights->m_lightBufferData.screen_height = outputSize.Height;
	m_Res->m_Lights->m_lightBufferData.texel_width = 1.0f / outputSize.Width;
	m_Res->m_Lights->m_lightBufferData.texel_height = 1.0f / outputSize.Height;
	//m_Res->m_Lights->m_lightBufferData.texel_width = 1.0f / screen_size_buffer.width_size;
	//m_Res->m_Lights->m_lightBufferData.texel_height = 1.0f / screen_size_buffer.height_size;
}

void GameRenderer::ChangeSkybox(char* sky_filename)
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	if (m_Sky->bLoaded == true)
	{
		m_Sky->ReleaseTexture();
	}

	loader->LoadTexture("Assets\\Compiled\\" + m_Res->StringFromAscIIChars(sky_filename) + ".dds", nullptr, m_Sky->m_Texture.GetAddressOf());

	strcpy_s(m_Res->m_LevelInfo.skybox, sky_filename);

	m_Sky->bEnvironmentCreated = false;
}

void GameRenderer::ChangeWeather(bool _bSnow, bool _bRain, bool _bFog)
{
	if (_bSnow == true)
		m_Res->m_LevelInfo.bSnow = 1;
	else
		m_Res->m_LevelInfo.bSnow = 0;
	if (_bRain == true)
		m_Res->m_LevelInfo.bRain = 1;
	else
		m_Res->m_LevelInfo.bRain = 0;
	if (_bFog == true)
		m_Res->m_LevelInfo.bFog = 1;
	else
		m_Res->m_LevelInfo.bFog = 0;
}

task<void> GameRenderer::LoadLevelResourcesAsync(int levelnum)
{
	m_levelResourcesLoaded = false;
	m_Sky->bEnvironmentCreated = false;

	m_Res->LoadDatabaseLevelInfo(levelnum);

	if (m_Sky->bLoaded == true)
	{
		m_Sky->ReleaseTexture();
	}
	m_Sky->bLoaded = true;

	m_Level->LoadBinary(levelnum);

	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	m_LevelEdit->SetLevel(levelnum);

	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\" + m_Res->StringFromAscIIChars(m_Res->m_LevelInfo.skybox) + ".dds", nullptr, m_Sky->m_Texture.GetAddressOf()));
	tasks.push_back(m_Level->LoadLevelTextures());
	tasks.push_back(m_Statics->LoadBinary(levelnum));
	tasks.push_back(m_Stuff->LoadBinary(levelnum));
	tasks.push_back(m_LevelEdit->GetCameraPath()->LoadBinary(levelnum));

	tasks.push_back(m_Level->UpdateTerrain(true));

	return when_all(tasks.begin(), tasks.end());// .then([this] {});
}

void GameRenderer::FinalizeLoadLevelResources()
{
	m_Level->UpdateVertexBuffers(true);

	m_Level->MakePhysics();

	m_Stuff->MakePhysics();

	m_Statics->MakePhysics();

	m_Statics->MakeAllPhysical();
	m_Level->CalculateRoofHeightmap();

	//m_LevelEdit->SetStaticsHeightToTerrrain();
#ifdef RENDER_BUGGY
	m_Buggy->MakePhysics();

	m_Buggy->SetCarPosition(m_Res->m_LevelInfo.player_start_x, m_Level->GetTerrainHeight(m_Res->m_LevelInfo.player_start_z, m_Res->m_LevelInfo.player_start_x) + 2.0f, m_Res->m_LevelInfo.player_start_z, 0.0f, 0.0f, 0.0f);
#endif

#ifdef RENDER_CAR
		m_Car->MakePhysics();
		m_Car->SetCarPosition(m_Res->m_LevelInfo.player_start_x+4.0f, m_Level->GetTerrainHeight(m_Res->m_LevelInfo.player_start_z, m_Res->m_LevelInfo.player_start_x) + 2.0f, m_Res->m_LevelInfo.player_start_z, 0.0f, 0.0f, 0.0f);
#endif

#ifdef RENDER_MIKEY
	m_Bert->MakePhysics();

	m_Bert->SetPosition(m_Res->m_LevelInfo.player_start_x + 3.0f, m_Res->m_LevelInfo.player_start_y + 10.0f, m_Res->m_LevelInfo.player_start_z);
#endif

	m_Res->m_audio.SetTrack(m_Res->m_LevelInfo.start_music);
	m_Res->m_audio.SetMusicVolume(1.0f);
	if (START_MUSIC == 1)
	{
		m_Res->m_audio.Start();
		m_Res->m_audio.SetSoundEffectVolume((SoundEvent)m_Res->m_LevelInfo.start_sound, 0.2f);
		m_Res->m_audio.SetSoundEffectPitch((SoundEvent)m_Res->m_LevelInfo.start_sound, 1.0f);
		m_Res->m_audio.PlaySoundEffect((SoundEvent)m_Res->m_LevelInfo.start_sound);
	}

	m_levelResourcesLoaded = true;
}

void GameRenderer::UpdateRemovePhysics()
{
	
#ifdef RENDER_BUGGY
	m_Buggy->RemoveContraints();
#endif
#ifdef RENDER_CAR
	//m_Car->Remove();
#endif
	m_Res->m_Physics.ClearPhysicsObjects();
	//m_Res->m_Physics.RemoveAllObjects(0);
	m_GunBall->m_Body.clear();
	//m_Stuff->m_stuff.clear();
}

task<void> GameRenderer::CreateGameDeviceResourcesAsync(_In_ Simple3DGame^ game)
{
	m_game = game;

	auto d3dDevice = m_deviceResources->GetD3DDevice();

	m_Res->LoadDatabaseAllTextureFilenames();

	BasicLoader^ loader = ref new BasicLoader(d3dDevice);

	std::vector<task<void>> tasks;

	tasks.push_back(m_Res->m_Lights->LoadTextures());

	tasks.push_back(m_Res->m_Textures->LoadTextures());

	tasks.push_back(m_Res->m_Lights->LoadTextures());

	tasks.push_back(m_Fire->LoadTextures());
	tasks.push_back(m_Level->LoadWaterTextures());
	tasks.push_back(m_Rain->LoadTextures());
	//tasks.push_back(m_Skyplane->LoadTextures());
	tasks.push_back(m_Snow->LoadTextures());
	tasks.push_back(m_Fog->LoadTextures());
	tasks.push_back(m_Grass->LoadTextures());
	
	//m_GunBall->LoadModel().then([this] { m_GunBall->LoadTexture(); });
	
	//tasks.push_back(m_Res->LoadShaders().then([this] {  }));
	tasks.push_back(m_Statics->LoadModels());// .then([this] {}));

	tasks.push_back(m_LevelEdit->LoadModels().then([this] { m_LevelEdit->LoadTextures(); }));

	tasks.push_back(m_Stuff->LoadModels());// .then([this] { m_Stuff->LoadTextures(); }));
	
#ifdef RENDER_BUGGY
	tasks.push_back(m_Buggy->LoadModels().then([this] { m_Buggy->LoadTextures(); }));
#endif
#ifdef RENDER_CAR
	tasks.push_back(m_Car->LoadModels().then([this] { m_Car->LoadTextures(); }));

#endif
	tasks.push_back(m_Bert->LoadModels().then([this] { m_Bert->LoadTextures(); }));

	return when_all(tasks.begin(), tasks.end()).then([this] {m_Statics->LinkTextures(); m_Stuff->LinkTextures(); });//
}

void GameRenderer::FinalizeCreateGameDeviceResources()
{
	m_Res->FinalizeCreateDeviceResources();
	m_Bert->FinalizeCreateDeviceResources();
	m_Statics->FinalizeCreateDeviceResources();
	m_LevelEdit->SetModelNumbers();
	m_gameResourcesLoaded = true;
}

task<void> GameRenderer::ProcessCollisions()
{
	return create_task([this]
	{
		if (true)
		{
			int numManifolds = m_Res->m_Physics.m_dynamicsWorld->getDispatcher()->getNumManifolds();
			for (int i = 0; i < numManifolds; i++)
			{
				btPersistentManifold* contactManifold = m_Res->m_Physics.m_dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
				//contactManifold->getObjectType()
				btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
				btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

				btRigidBody* bodyA = btRigidBody::upcast(obA);
				btRigidBody* bodyB = btRigidBody::upcast(obB);

				if (bodyA && bodyA->getMotionState())
				{
					//SimpleMotion3* ms = SimpleMotion3(bodyA->getMotionState());
					//bodyA->getMotionState()->
					//delete bodyA->getMotionState();
					//bodyA = nullptr;
				}
				if (bodyB && bodyB->getMotionState())
				{
					//delete bodyA->getMotionState();
					//bodyA = nullptr;
				}

				int stuff_indexA = m_Stuff->IsStuffModel(bodyA);
				int stuff_indexB = m_Stuff->IsStuffModel(bodyB);
				if ((stuff_indexA > 0 || stuff_indexB > 0) &&
					(bodyA->getLinearVelocity().length() > 0.1f ||
						bodyB->getLinearVelocity().length() > 0.1f)) //(stuff_index > -1)
				{
					int numContacts = contactManifold->getNumContacts();
					for (int j = 0; j < numContacts; j++)
					{
						btManifoldPoint& pt = contactManifold->getContactPoint(j);
						if (pt.getDistance() < 0.f)
						{
							if (pt.getAppliedImpulse() > 0.05f)
							{
								const btVector3& ptA = pt.getPositionWorldOnA();
								const btVector3& ptB = pt.getPositionWorldOnB();
								const btVector3& normalOnB = pt.m_normalWorldOnB;

								btVector3 tra = bodyA->getWorldTransform().getOrigin();

								float dist = m_Res->m_Camera->DistanceFromEye(tra.getX(), tra.getY(), tra.getZ());

								if (dist < MAX_SFX_DISTANCE)
								{
									stuffplaysound_t sound;

									sound.impulse = pt.getAppliedImpulse() * pt.getAppliedImpulse() * 50.5f;
									if (sound.impulse > 1.0f)
										sound.impulse = 1.0f;

									float atten = (1.0f - (dist / MAX_SFX_DISTANCE)) *(1.0 / (1.0 + 0.1*dist + 0.01*dist*dist));
									sound.impulse *= atten;

									if (stuff_indexA > -1 && stuff_indexA < m_Stuff->total_stuff_models)
									{
										sound.model_index = stuff_indexA;
										m_Stuff->play_sounds.push_back(sound);
									}
									if (stuff_indexB > -1 && stuff_indexB < m_Stuff->total_stuff_models)
									{
										sound.model_index = stuff_indexB;
										m_Stuff->play_sounds.push_back(sound);
									}
								}
							}
						}
					}
				}

				if (m_GunBall->IsGunBall(bodyA) || m_GunBall->IsGunBall(bodyB) &&
					(bodyA->getLinearVelocity().length() > 0.1f ||
						bodyB->getLinearVelocity().length() > 0.1f))
				{
					int numContacts = contactManifold->getNumContacts();
					for (int j = 0; j < numContacts; j++)
					{
						btManifoldPoint& pt = contactManifold->getContactPoint(j);
						if (pt.getDistance() < 0.f)
						{
							if (pt.getAppliedImpulse() > 0.2f && pt.getLifeTime() < 2)
							{
								const btVector3& ptA = pt.getPositionWorldOnA();
								const btVector3& ptB = pt.getPositionWorldOnB();
								const btVector3& normalOnB = pt.m_normalWorldOnB;

								btVector3 tra = bodyA->getWorldTransform().getOrigin();
								btVector3 vea = bodyA->getLinearVelocity();

								float dist = m_Res->m_Camera->DistanceFromEye(tra.getX(), tra.getY(), tra.getZ());

								if (dist < MAX_SFX_DISTANCE)
								{
									stuffplaysound_t sound;
									sound.model_index = m_GunBall->model_type;

									sound.impulse = pt.getAppliedImpulse() * 0.5f;
									if (sound.impulse > 1.0f)
										sound.impulse = 1.0f;

									float atten = (1.0f - (dist / MAX_SFX_DISTANCE)) * (1.0 / (1.0 + 0.1*dist + 0.01*dist*dist));
									sound.impulse *= atten;

									sound.Emitter.Position = XMFLOAT3(tra.getX(), tra.getY(), tra.getZ());
									sound.Emitter.Velocity = XMFLOAT3(vea.getX(), vea.getY(), vea.getZ());

									sound.Listener.Position = m_Res->m_Camera->Eye();

									m_Stuff->play_sounds.push_back(sound);
								}
							}
						}
					}
				}
			}
		}
	});
}

task<void> GameRenderer::UpdateNonPhysics(float timeDelta, float timeTotal)
{
	std::vector<task<void>> tasks;

	tasks.push_back(ProcessCollisions());

	if (m_Res->m_LevelInfo.bSnow == 1)
		tasks.push_back(m_Snow->Update(timeDelta, timeTotal));

	if (m_Res->m_LevelInfo.bRain == 1)
		tasks.push_back(m_Rain->Update(timeDelta, timeTotal));

	if (m_Res->m_LevelInfo.bFog == 1)
		tasks.push_back(m_Fog->Update(timeDelta, timeTotal));

	tasks.push_back(m_Level->Update(timeDelta, timeTotal).then([timeDelta, timeTotal, this]
	{
		std::vector<task<void>> tasks;

		tasks.push_back(m_Grass->Update(timeDelta, timeTotal));
		tasks.push_back(m_Statics->Update(timeDelta, timeTotal));
		tasks.push_back(m_Stuff->Update());

		return when_all(begin(tasks), end(tasks));
	}));

	tasks.push_back(m_Fire->Update(timeDelta, timeTotal));

	return when_all(begin(tasks), end(tasks));
}

void GameRenderer::SwitchTorch()
{
	if (bTorchOn == true)
	{
		bTorchOn = false;
		m_Res->m_audio.SetSoundEffectVolume(SFXTorchOff, 0.5f);
		m_Res->m_audio.SetSoundEffectPitch(SFXTorchOff, 1.0f);
		m_Res->m_audio.PlaySoundEffect(SFXTorchOff);
	}
	else
	{
		bTorchOn = true;
		m_Res->m_audio.SetSoundEffectVolume(SFXTorchOn, 0.5f);
		m_Res->m_audio.SetSoundEffectPitch(SFXTorchOn, 1.0f);
		m_Res->m_audio.PlaySoundEffect(SFXTorchOn);
	}
}

task<void> GameRenderer::Update(float timeDelta, float timeTotal)
{
	if (bLoadingComplete == false || m_gameResourcesLoaded == false || m_levelResourcesLoaded == false)
		return task<void>([] {return; });

	m_Res->m_Lights->ResetLights();

	if (m_Res->camera_mode == 1)
	{
		m_LevelEdit->Update(timeDelta, timeTotal);
	}
	else
	{
		if (m_game->GetController()->KeyState(Windows::System::VirtualKey::E, true) == true)
		{
			SwitchTorch();
		}
	}

	if (m_game->GetController()->KeyState(Windows::System::VirtualKey::C, true) == true)
	{
		if (bCarMode == false)
		{
			bCarMode = true;
			//m_Car->SetCarPosition(m_Buggy->GetPosition()->getX(), 10.0f, m_Buggy->GetPosition()->getZ(), 0.0f, 0.0f, 0.0f);
		}
		else
		{
			bCarMode = false;
		}
	}
#ifdef RENDER_BUGGY
	
	{
		m_Buggy->SetCarPosition(m_Buggy->GetPosition()->getX(), 10.0f, m_Buggy->GetPosition()->getZ(), 0.0f, 0.0f, 0.0f);
		m_Buggy->carbody->setAngularVelocity(btVector3(0.1f,0.1f,0.1f));
	}
#endif
#ifdef RENDER_CAR
	if(bCarMode==true)
	{
		if (m_game->GetController()->KeyState(Windows::System::VirtualKey::R, false) == true)
		{
			m_Car->SetCarPosition(m_Buggy->GetPosition()->getX(), 10.0f, m_Buggy->GetPosition()->getZ(), 0.0f, 0.0f, 0.0f);
			//m_Buggy->carbody->setAngularVelocity(btVector3(0.1f, 0.1f, 0.1f));
		}
	}
#endif
	if ((bPaused == true && bPauseFly == true) || (bPaused == true && m_Res->camera_mode == 0))
		m_LevelEdit->GetCameraPath()->Update(timeDelta, timeTotal);

	if (bPaused == true)
	{
	}
	else
	{
		m_Res->m_uiControl->SetNonFocus();
	}
#ifdef RENDER_MIKEY
	btVector3 bert_pos = btVector3(m_Bert->bert_pos.x, m_Bert->bert_pos.y, m_Bert->bert_pos.z);
#endif

	btVector3 bug_pos;
	btVector3 bug_dir;
	btVector3 bug_up;

#ifdef RENDER_BUGGY
	bug_pos = btVector3(m_Buggy->player_x, m_Buggy->player_y, m_Buggy->player_z);//   //*m_Buggy->GetPosition();
	bug_dir = *m_Buggy->GetDirection();
	bug_up = *m_Buggy->GetUp();
#endif
#ifdef RENDER_CAR
	if (bCarMode == true)
	{
		std::vector<btTransform> car_trans = m_Car->getCarTransform();
		bug_pos = car_trans[0].getOrigin();

		btMatrix3x3 rot_mat = btMatrix3x3(car_trans[0].getRotation());

		btVector3 car_point = btVector3(0.0f, 0.0f, 1.0f);
		car_point = rot_mat * car_point;

		bug_dir = car_point;
		bug_up = btVector3(0.0f, 1.0f, 0.0f);
	}
#endif
	/*
	float px = bug_pos.getX();
	float py = bug_pos.getY();
	float pz = bug_pos.getZ();

	float dx = bug_dir.getX();
	float dy = bug_dir.getY();
	float dz = bug_dir.getZ();

	float ux = bug_up.getX();
	float uy = bug_up.getY();
	float uz = bug_up.getZ();
	*/

	player_mode = 0;

#ifdef RENDER_BUGGY
	player_mode = 3;
#endif
#ifdef RENDER_CAR
	if (bCarMode == true)
	{
		player_mode = 3;
	}
#endif

	switch (player_mode)//(player_mode)
	{
	case 0:break;
		/*
		if (camera_mode == 0)
				{
				}
				else
				{
					float height_dif = camera_height - m_Res->m_Camera->Eye().y;
					m_Res->m_Camera->Eye(XMFLOAT3(m_Res->m_Camera->Eye().x, camera_height, m_Res->m_Camera->Eye().z));
					m_Res->m_Camera->LookAtY(height_dif);
				}
		break;
		*/
#ifdef RENDER_MIKEY
	case 2:m_PanCamera->SetEyeAt(bert_pos.getX() + (sin(m_Bert->view_angle)*5.0f), bert_pos.getY() + 2.0f + m_Bert->view_pos_y, bert_pos.getZ() - (cos(m_Bert->view_angle)*5.0f), bert_pos.getX(), bert_pos.getY() + 0.0f, bert_pos.getZ());
		m_PanCamera->Update(timeDelta); break;
#endif
		case 3:m_PanCamera->SetEyeAt(bug_pos.getX() - (bug_dir.getX()*3.0f), bug_pos.getY() + 3.0f, bug_pos.getZ() - (bug_dir.getZ()*3.0f), bug_pos.getX(), bug_pos.getY() + 2.0f, bug_pos.getZ());
			m_PanCamera->Update(timeDelta); break;
	}
	//m_PanCamera->SetEyeAt(bert_pos.getX() + (sin(m_Bert->view_angle)*5.0f), bert_pos.getY() + 2.0f + m_Bert->view_pos_y, bert_pos.getZ() - (cos(m_Bert->view_angle)*5.0f), bert_pos.getX(), bert_pos.getY() + 0.0f, bert_pos.getZ());

	//m_PanCamera->SetEyeAt(bug_pos.getX() - (bug_dir.getX()*3.0f), bug_pos.getY() + 3.0f, bug_pos.getZ() - (bug_dir.getZ()*3.0f), bug_pos.getX(), bug_pos.getY() + 2.0f, bug_pos.getZ());

	//m_PanCamera->Update(timer.GetElapsedSeconds());

	m_Res->m_Camera->buildWorldFrustumPlanes();

	if (bMusicStarted == false)
	{
		bMusicStarted = true;
		if (START_MUSIC == 1)
		{
			//m_Res->m_audio.Start();
		}
	}
	
	if (bTorchOn == true)
	{
		CG_SPOT_LIGHT spot_light;
		ZeroMemory(&spot_light, sizeof(CG_SPOT_LIGHT));
		spot_light.ambient = XMFLOAT4(0.1f * LIGHT_SPOT_MULTIPLIER, 0.1f * LIGHT_SPOT_MULTIPLIER, 0.1f * LIGHT_SPOT_MULTIPLIER, 1.0f);
		spot_light.diffuse = XMFLOAT4(0.35f * LIGHT_SPOT_MULTIPLIER, 0.3f * LIGHT_SPOT_MULTIPLIER, 0.3f * LIGHT_SPOT_MULTIPLIER, 1.0f);
		spot_light.specular = XMFLOAT4(0.1f * LIGHT_SPOT_MULTIPLIER, 0.1f * LIGHT_SPOT_MULTIPLIER, 0.1f * LIGHT_SPOT_MULTIPLIER, 1.0f);
		
		spot_light.radius = SPOT_LIGHT_RADIUS;
		spot_light.up = XMFLOAT3(m_Res->m_Camera->Up().x, m_Res->m_Camera->Up().y, m_Res->m_Camera->Up().z);
		spot_light.spot = 7.0f;
		spot_light._specular_power = 20.0f;
		spot_light.lightmap = 1;
#ifdef RENDER_CAR
		if (bCarMode == true)
		{
			spot_light.pos = XMFLOAT3(bug_pos.getX(), bug_pos.getY(), bug_pos.getZ());
			spot_light.dir = XMFLOAT3(bug_dir.getX(), bug_dir.getY(), bug_dir.getZ());
		}
#else
		spot_light.pos = XMFLOAT3(m_Res->m_Camera->Eye().x, m_Res->m_Camera->Eye().y, m_Res->m_Camera->Eye().z);
		spot_light.dir = XMFLOAT3(m_Res->m_Camera->LookingDir().x, m_Res->m_Camera->LookingDir().y, m_Res->m_Camera->LookingDir().z);
#endif
		spot_light.pos = XMFLOAT3(spot_light.pos.x + spot_light.dir.x, spot_light.pos.y + spot_light.dir.y, spot_light.pos.z + spot_light.dir.z);
		m_Res->m_Lights->AddSpot(spot_light);
	}

	std::vector<task<void>> tasks;

	tasks.push_back(create_task([this] { m_Res->m_audio.Render(); }));

	tasks.push_back(UpdateNonPhysics(timeDelta, timeTotal));

	return when_all(begin(tasks), end(tasks));
}

void GameRenderer::UpdatePhysics(float timeDelta, float timeTotal)
{
	if (bLoadingComplete == false || m_gameResourcesLoaded == false || m_levelResourcesLoaded == false)
		return;

	m_Stuff->PlaySounds();

	m_Level->UpdatePhysics();

	if (m_Res->camera_mode == 1)
		m_LevelEdit->UpdatePhysics();

	m_Statics->UpdatePhysics();

#ifdef RENDER_BUGGY
	m_Buggy->Update(timeDelta, timeTotal);
#endif
#ifdef RENDER_CAR
	if (bCarMode == true)
	{
		m_Car->Update(timeDelta, timeTotal);
	}
#endif

	if (m_Res->camera_mode == 1)
	{
		m_Stuff->UpdatePhysics(false);
	}
	else
	{
		m_Stuff->UpdatePhysics();
	}
#ifdef RENDER_MIKEY
	m_Bert->UpdatePhysics(timeDelta, timeTotal);
#endif

	m_GunBall->UpdatePhysics();

	m_game->GamePlayer()->UpdatePhysics(timeDelta);
#ifdef PHYSICS_SINGLETHREAD_UPDATE
	if (bPaused == false) // not working
	{
		m_Res->m_Physics.Update(timeDelta, timeTotal).wait();
	}
#endif

	//m_Rain->UpdateCameraPosition();
}

void GameRenderer::RenderWaterTexture()
{
	m_Res->SetDepthStencil(1);
	m_Res->m_RDeferredBuffers->SetRenderTargets(m_deviceResources->GetD3DDeviceContext(), m_deviceResources->GetDepthStencilView());
	m_Res->m_RDeferredBuffers->ClearRenderTargets(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);
	m_Res->m_Camera->SetCloseProjection();
	m_Res->m_Camera->UpdateConstantBuffer();
	m_Res->SetDefWaterShader();
	m_Level->RenderWater();
}

void GameRenderer::RenderGlowTexture()
{
	m_GlowTexture->SetRenderTarget(m_deviceResources->GetD3DDeviceContext(), m_deviceResources->GetDepthStencilView());

	m_GlowTexture->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);

	m_Res->m_Camera->SetCloseProjection();
	m_Res->m_Camera->UpdateConstantBuffer();

	m_Res->SetEmmitShader();

	m_Res->SetDepthStencil(2);
	m_Statics->Render(3, false);
	m_Stuff->Render(3);
	m_Res->SetGlassShader();
	m_Res->SetDepthStencil(0);
	m_Statics->Render(2, false);

	if (false)
	{
		m_Res->SetQuadIndexBuffer();
		m_Res->SetFireShader();
		m_Fire->Render();
	}

	m_deviceResources->GetD3DDeviceContext()->GenerateMips(m_GlowTexture->GetShaderResourceView().Get());
	m_Res->m_Camera->UpdateConstantBufferOrth();

	m_GlowTextureDownsample->SetRenderTarget(m_deviceResources->GetD3DDeviceContext());
	m_GlowTextureDownsample->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Res->RenderAlphaMode(0);
	m_Res->SetGlowShader();
	m_Res->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_GlowTexture->GetShaderResourceView().GetAddressOf());

	m_Res->m_FullScreenWindow->Render(m_Res->m_deviceResources->GetD3DDeviceContext());

	if (true)
		for (int i = 0; i < GLOW_BLUR_PASS; i++)
		{
			m_GlowTextureDownsample2->SetRenderTarget(m_deviceResources->GetD3DDeviceContext());
			m_GlowTextureDownsample2->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

			m_Res->SetVblurShader();
			m_Res->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_GlowTextureDownsample->GetShaderResourceView().GetAddressOf());
			m_Res->m_FullScreenWindow->Render(m_Res->m_deviceResources->GetD3DDeviceContext());

			m_GlowTextureDownsample->SetRenderTarget(m_deviceResources->GetD3DDeviceContext());
			m_GlowTextureDownsample->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

			m_Res->SetHblurShader();
			m_Res->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_GlowTextureDownsample2->GetShaderResourceView().GetAddressOf());
			m_Res->m_FullScreenWindow->Render(m_Res->m_deviceResources->GetD3DDeviceContext());
		}
}

void GameRenderer::RenderSkyCubeMap()
{
	int i;
	XMFLOAT4X4 view_temp;
	view_temp = m_Res->m_Camera->m_constantBufferData.view;

	if (true)
	{
		m_Res->SetSkyShader();
		m_Res->m_Camera->SetSkyProjection();

		if (m_Sky->bEnvironmentCreated == false)
		{
			for (i = 0; i < 6; ++i)
			{
				m_Sky->m_SkyCubeMap->SetRenderPosition(0.0f, 0.0f, 0.0f);

				XMStoreFloat4x4(&m_Res->m_Camera->m_constantBufferData.view, XMMatrixTranspose(m_Sky->m_SkyCubeMap->RenderCubeSide(i, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f))));

				if (true)
				{
					m_Res->RenderAlphaMode(0);
					m_Sky->RenderCenter();
				}
			}
			m_Sky->bEnvironmentCreated = true;
		}
	}
	m_Res->m_Camera->m_constantBufferData.view = view_temp;

	m_Res->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(5, 1, m_Sky->m_SkyCubeMap->GetShaderResourceView().GetAddressOf());
}

void GameRenderer::Render()
{
	if (bLoadingComplete == false || m_gameResourcesLoaded == false)
		return;

	bool stereoEnabled = m_deviceResources->GetStereoState();

	auto d3dContext = m_deviceResources->GetD3DDeviceContext();
	auto d2dContext = m_deviceResources->GetD2DDeviceContext();

	int renderingPasses = 1;
	if (stereoEnabled)
	{
		renderingPasses = 2;
	}

	for (int i = 0; i < renderingPasses; i++)
	{
		if (m_game != nullptr && m_gameResourcesLoaded && m_levelResourcesLoaded)
		{
			if (i > 0)
			{
				// Doing the Right Eye View.
				ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetViewRight() };
				d3dContext->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());
				d3dContext->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
				d2dContext->SetTarget(m_deviceResources->GetD2DTargetBitmapRight());
			}
			else
			{
				// Doing the Mono or Left Eye View.
				ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
				d3dContext->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());
				d3dContext->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
				d2dContext->SetTarget(m_deviceResources->GetD2DTargetBitmap());
			}

			if (true)
			{
			}

			if (true)
			{
				m_Res->m_Lights->UpdateConstantBuffer();

				RenderSkyCubeMap();

				m_Snow->UpdateVertexBuffers();
				m_Rain->UpdateVertexBuffers();
				m_Fire->UpdateVertexBuffers();
				m_Fog->UpdateVertexBuffers();
				m_Grass->UpdateVertexBuffers();

				d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				m_Res->RenderAlphaMode(0);

				d3dContext->PSSetSamplers(0, 1, m_Res->m_samplerLinear.GetAddressOf());

				if (true)
				{
					m_Res->SetDeferredShader();

					m_Res->m_DeferredBuffers->SetRenderTargets(d3dContext, m_deviceResources->GetDepthStencilView());
					m_Res->m_DeferredBuffers->ClearRenderTargets(d3dContext, 0.0f, 0.0f, 0.0f, 0.0f);
				}

				m_Res->m_Camera->SetCloseProjection();

				d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_Res->SetCull(true);
				m_Res->SetDepthStencil(1);

				XMStoreFloat4x4(&m_Res->m_Camera->m_constantBufferData.model, XMMatrixIdentity());

				m_Res->m_Camera->UpdateConstantBuffer();

				m_Res->SetDepthStencil(1);
				m_Res->SetCull(true);

				m_Res->SetDeferredShader();
				m_Statics->Render(0, true);
				m_Statics->Render(1, true);

				m_Stuff->Render(0);

				if (m_Res->camera_mode == 1 && bPaused == false)
					m_LevelEdit->Render();

#ifdef RENDER_BUGGY
				//m_Buggy->SetCarPosition(m_Res->m_LevelInfo.player_start_x, m_Level->GetTerrainHeight(m_Res->m_LevelInfo.player_start_z, m_Res->m_LevelInfo.player_start_x) + 2.0f, m_Res->m_LevelInfo.player_start_z, 0.0f, 0.0f, 0.0f);
				m_Buggy->Render();
#endif
#ifdef RENDER_CAR
				if (bCarMode == true)
				{
					m_Car->Render(0);
				}
#endif
				m_GunBall->Render();

#ifdef RENDER_MIKEY
				m_Res->SetDefSkinShader();
				m_Bert->Render(0);
#endif
				if (bDefGrass == true)
				{
					m_Res->SetDefPointShader();
					m_Res->SetQuadIndexBuffer();
					m_Res->RenderAlphaMode(0);
					m_Res->SetCull(false);
					m_Grass->Render();
				}

				m_Res->SetDefGroundShader();
				//RenderAlphaMode(0);
				m_Res->SetDepthStencil(1);
				m_Res->SetCull(true);
				m_Level->Render();

				m_Res->SetDepthStencil(0);
				m_Res->RenderAlphaMode(0);
				RenderGlowTexture();

				if (true)
				{
					m_DeferredTexture->SetRenderTarget(d3dContext, m_deviceResources->GetDepthStencilView());

					m_DeferredTexture->ClearRenderTarget(d3dContext, 0.0f, 0.0f, 0.0f, 0.0f);

					m_Res->m_Camera->UpdateConstantBufferOrth();
					m_Res->SetDepthStencil(0);

					m_Res->SetLightShader();
					d3dContext->PSSetShaderResources(0, 1, m_Res->m_DeferredBuffers->GetShaderResourceView(0).GetAddressOf());
					d3dContext->PSSetShaderResources(1, 1, m_Res->m_DeferredBuffers->GetShaderResourceView(1).GetAddressOf());
					d3dContext->PSSetShaderResources(2, 1, m_Res->m_DeferredBuffers->GetShaderResourceView(2).GetAddressOf());
					d3dContext->PSSetShaderResources(3, 1, m_Res->m_DeferredBuffers->GetShaderResourceView(3).GetAddressOf());

					m_Res->m_FullScreenWindow->Render(d3dContext);
				}

				RenderWaterTexture();

				m_DeferredTexture->SetRenderTarget(d3dContext, m_deviceResources->GetDepthStencilView());

				m_Res->RenderAlphaMode(0);
				m_Res->SetDepthStencil(0);
				m_Res->SetSkyShader();
				m_Res->m_Camera->SetSkyProjection();

				m_Sky->Render();
				m_Res->RenderAlphaMode(3);
				m_Res->SetCull(true);

				d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				m_Res->m_Camera->SetCloseProjection();

				m_Res->RenderAlphaMode(2);

				if (ENABLE_EDIT_MODE == true && m_game->GetController()->KeyState(Windows::System::VirtualKey::R, true) == true)
				{
					if (bFXAAEnabled == true)
					{
						bFXAAEnabled = false;
					}
					else
					{
						bFXAAEnabled = true;
					}
				}

				if (bFXAAEnabled == true)//(m_game->GetController()->KeyState(Windows::System::VirtualKey::R, false) == true) //(true)
				{
					//ID3D11RenderTargetView* pRTV = m_deviceResources->GetBackBufferRenderTargetView();
					//d3dContext->OMSetRenderTargets(1, &pRTV, 0);

					m_FXAATexture->SetRenderTarget(d3dContext, m_deviceResources->GetDepthStencilView());
					m_FXAATexture->ClearRenderTarget(d3dContext, 0.0f, 0.0f, 0.0f, 0.0f);
					m_Res->SetDepthStencil(0);

					d3dContext->PSSetSamplers(3, 1, m_Res->m_samplerBilinear.GetAddressOf());
					//m_deviceResources->RestoreViewportAndRenderTarget();
					//m_Res->m_Camera->UpdateConstantBufferOrth();

					d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
					d3dContext->VSSetShader(m_Res->m_fxaa_vertexShader.Get(), NULL, 0);
					d3dContext->PSSetShader(m_Res->m_fxaa_pixelShader.Get(), NULL, 0);
					if (false)//(DXUTGetDXGIBackBufferSurfaceDesc()->SampleDesc.Count > 1)
					{
						// resolve first
						//pd3dImmediateContext->ResolveSubresource(g_pCopyResolveTexture, 0, g_pProxyTexture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
						//pd3dImmediateContext->PSSetShaderResources(0, 1, &g_pCopyResolveTextureSRV);
						//pd3dImmediateContext->Draw(4, 0);
					}
					else
					{
						d3dContext->PSSetShaderResources(0, 1, m_DeferredTexture->GetShaderResourceView().GetAddressOf());
						d3dContext->Draw(4, 0);
					}
				}

				if (true)//(rand() % 2 == 1)
				{
					m_deviceResources->RestoreViewportAndRenderTarget();
					m_Res->m_Camera->UpdateConstantBufferOrth();
					m_Res->SetDepthStencil(0);
					m_Res->SetGlowMapShader();
					if (bFXAAEnabled == true)
					{
						d3dContext->PSSetShaderResources(0, 1, m_FXAATexture->GetShaderResourceView().GetAddressOf());
					}
					else
					{
						d3dContext->PSSetShaderResources(0, 1, m_DeferredTexture->GetShaderResourceView().GetAddressOf());
					}
					d3dContext->PSSetShaderResources(1, 1, m_GlowTextureDownsample->GetShaderResourceView().GetAddressOf());
					d3dContext->PSSetShaderResources(2, 1, m_GlowTexture->GetShaderResourceView().GetAddressOf());

					d3dContext->PSSetShaderResources(3, 1, m_Res->m_RDeferredBuffers->GetShaderResourceView(0).GetAddressOf());
					d3dContext->PSSetShaderResources(4, 1, m_Res->m_RDeferredBuffers->GetShaderResourceView(1).GetAddressOf());

					d3dContext->PSSetShaderResources(6, 1, m_Res->m_DeferredBuffers->GetShaderResourceView(2).GetAddressOf());

					d3dContext->PSSetShaderResources(7, 1, m_Res->m_RDeferredBuffers->GetShaderResourceView(2).GetAddressOf());

					m_Res->m_FullScreenWindow->Render(d3dContext);
				}

				if (bDefGrass == false)
				{
					//m_Res->SetDefPointShader();
					m_Res->SetPointLitShader();
					m_Res->SetQuadIndexBuffer();

					m_Res->RenderAlphaMode(1);
					m_Res->SetCull(false);
					m_Grass->Render();
				}

				m_Res->RenderAlphaMode(2);
				m_Res->SetCull(false);
				m_Res->SetQuadIndexBuffer();
				m_Res->SetFireShader();
				m_Fire->Render();

				m_Res->SetInsPointShader();
				m_Res->SetDepthStencil(0);

				m_Res->RenderAlphaMode(1);
				if (m_Res->m_LevelInfo.bSnow == 1)
					m_Snow->Render();

				if (m_Res->m_LevelInfo.bRain == 1 && m_Rain->bInstanced == true)
					m_Rain->Render();

				//if (m_Res->m_LevelInfo.bFog == 1)
				//	m_Fog->Render();

				if (m_Res->m_LevelInfo.bRain == 1 && m_Rain->bInstanced == false)
				{
					m_Res->SetQuadIndexBuffer();
					m_Res->SetPointLitShader();
					m_Rain->Render();
				}
			}

			if (false)//player_mode==0)
			{
				d2dContext->BeginDraw();
				d2dContext->SetTransform(m_deviceResources->GetOrientationTransform2D());
				if (m_game != nullptr && m_gameResourcesLoaded)
					m_gameHud->Render(m_game);
				HRESULT hr = d2dContext->EndDraw();
				if (hr != D2DERR_RECREATE_TARGET)
					DX::ThrowIfFailed(hr);
			}
		}
		else
		{
			const float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
			if (i > 0)
				d3dContext->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetViewRight(), ClearColor);
			else
				d3dContext->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), ClearColor);
		}
	}
}

#if defined(_DEBUG)
void GameRenderer::ReportLiveDeviceObjects()
{
	// If the debug layer isn't present on the system then this QI will fail.  In that case
	// the routine just exits.   This is a debugging aid to see the active D3D objects and
	// is not critical to functional operation of the sample.

	ComPtr<ID3D11Debug> debugLayer;
	ComPtr<ID3D11Device2>d3dDevice = m_deviceResources->GetD3DDevice();
	HRESULT hr = d3dDevice.As(&debugLayer);
	if (FAILED(hr))
	{
		return;
	}
	DX::ThrowIfFailed(
		debugLayer->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL)
	);
}
#endif