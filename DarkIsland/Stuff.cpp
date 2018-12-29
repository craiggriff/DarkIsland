#include "pch.h"
#include "Stuff.h"

#include "DefPhysics.h"
#include "DefShader.h"

using namespace Game;

#define MAX_STUFF 10000

// std::sort with inlining
struct stuff_compare {
	// the compiler will automatically inline this
	bool operator()(const stuff_tt* a, const stuff_tt* b) {
		return a->cam_dist < b->cam_dist;
	}
};

Stuff::Stuff(AllResources* p_Resources)
{
	int i;
	m_Res = p_Resources;

	m_deviceResources = p_Resources->m_deviceResources;

	max_num = 200;

	total_stuff_models = 10;

	m_stuff.clear();

	/*
	for (i = 0; i < 10; i++)
	{
		m_stuff_model[i] = new stuff_model_t(m_deviceResources, &p_Resources->m_Physics);
	}
*/
	cur_total = 0;

	cur_phy = 0;

	current_selected_index = 0;

	bLoadingComplete = false;
}

void Stuff::Reset()
{
	cur_total = 0;
	cur_phy = 0;

	current_selected_index = 0;

	int i;
	for (i = 0; i < m_stuff.size(); i++)
	{
		stuff_tt s = m_stuff[i];

		s.stuff.bActive = 0;
		s.bPhysical = false;

		if (s.m_Body != nullptr)
			s.m_Body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);

		m_stuff[i] = s;
	}

	m_stuff.clear();
	rp_stuff.clear();
}

int Stuff::IsStuffModel(btRigidBody* rb)
{
	int res = -1;
	for (int i = 0; i < m_stuff.size(); i++)
	{
		if (m_stuff[i].m_Body == rb)
			return i;
	}

	return res;
}

void Stuff::PlaySounds()
{
	bool index_played[1000];

	for (int i = 0; i < total_stuff_models; i++) // ytrue to only play sound once
	{
		index_played[i] = false;
	}

	for (stuffplaysound_t s : play_sounds)
	{
		if (m_stuff_model[s.model_index]->bHasSounds == true && index_played[s.model_index] == false)
		{
			index_played[s.model_index] = true;
			int rnd = rand() % 30;
			m_Res->m_audio.PlaySoundEffect(&m_stuff_model[s.model_index]->m_soundEffects[rnd]);
			m_Res->m_audio.SetSoundEffectVolume(&m_stuff_model[s.model_index]->m_soundEffects[rnd], s.impulse);
			//m_stuff_model[s.model_index]->m_soundEffects[rnd].m_soundEffectSourceVoice->

			// how to 3d sound
			//https://msdn.microsoft.com/en-us/library/windows/desktop/ee415798(v=vs.85).aspx
		}
	}
	play_sounds.clear();
}

//void Stuff::ModelImpact(btRigidBody* rb,float strength)

task<void> Stuff::LoadModels()
{
	m_Res->LoadDatabaseModelInfo(&model_info, "[Index] > 999");

	std::vector<concurrency::task<void>> tasks;
	//model_info.resize(model_info.size() - 1);
	for (int i = 0; i < model_info.size(); i++)
	{
		model_info[i].index -= 1000;
		m_stuff_model[model_info[i].index] = new stuff_model_t(m_deviceResources, &m_Res->m_Physics);
		tasks.push_back(m_stuff_model[model_info[i].index]->LoadModel(m_Res, m_Res->GetWC(model_info[i].filename), model_info[i].physics_shape, model_info[i].scale));
		//if (!strcmp(model_info[i].filename, "eye.cmo"))
		//{
		//	model_info[i].index = 1;
		//}
	}
	total_stuff_models = 0;

	return when_all(begin(tasks), end(tasks)).then([this]()
	{
		for (int i = 0; i < model_info.size(); i++)
		{
			m_stuff_model[model_info[i].index]->info.mrf = XMFLOAT3(model_info[i].mass, model_info[i].restitution, model_info[i].friction);
			m_stuff_model[model_info[i].index]->info.group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
			m_stuff_model[model_info[i].index]->info.mask = (COL_OBJECTS | COL_RAY);

			m_stuff_model[model_info[i].index]->type = model_info[i].type;

			//m_static_model[model_info[i].index]->light_radius =
			strcpy_s(m_stuff_model[model_info[i].index]->group, model_info[i].group);
			strcpy_s(m_stuff_model[model_info[i].index]->filename, model_info[i].filename);

			m_stuff_model[model_info[i].index]->info.dir.x = model_info[i].rotx;
			m_stuff_model[model_info[i].index]->info.dir.y = model_info[i].roty;
			m_stuff_model[model_info[i].index]->info.dir.z = model_info[i].rotz;

			m_stuff_model[model_info[i].index]->CalcOverallRadius();

			if (model_info[i].index > total_stuff_models)
				total_stuff_models = model_info[i].index;

			// load sound effects
			if (!strcmp(model_info[i].sound_filename, " "))
			{
				m_stuff_model[model_info[i].index]->bHasSounds = false;
			}
			else
			{
				m_stuff_model[model_info[i].index]->bHasSounds = true;
				char sfname[30];

				size_t convertedChars = 0;

				for (int j = 0; j < 10; j++)
				{
					sprintf_s(sfname, "Assets/Sounds/%s%d.mp3", model_info[i].sound_filename, j + 1);

					const size_t cSize = strlen(sfname) + 1;
					wchar_t* wc = new wchar_t[cSize];
					//mbstowcs(wc, c, cSize);
					mbstowcs_s(&convertedChars, wc, cSize, sfname, _TRUNCATE);

					m_Res->m_audio.CreateSoundEffect(wc, &m_stuff_model[model_info[i].index]->m_soundEffects[j]);

					m_Res->m_audio.SetSoundEffectVolume(&m_stuff_model[model_info[i].index]->m_soundEffects[j], 0.3f);
					m_Res->m_audio.SetSoundEffectPitch(&m_stuff_model[model_info[i].index]->m_soundEffects[j], 1.0f);
				}
				for (int j = 0; j < 10; j++)
				{
					sprintf_s(sfname, "Assets/Sounds/%s%d.mp3", model_info[i].sound_filename, j + 1);

					const size_t cSize = strlen(sfname) + 1;
					wchar_t* wc = new wchar_t[cSize];
					//mbstowcs(wc, c, cSize);
					mbstowcs_s(&convertedChars, wc, cSize, sfname, _TRUNCATE);

					m_Res->m_audio.CreateSoundEffect(wc, &m_stuff_model[model_info[i].index]->m_soundEffects[j + 10]);

					m_Res->m_audio.SetSoundEffectVolume(&m_stuff_model[model_info[i].index]->m_soundEffects[j + 10], 0.3f);
					m_Res->m_audio.SetSoundEffectPitch(&m_stuff_model[model_info[i].index]->m_soundEffects[j + 10], 0.8f);
				}
				for (int j = 0; j < 10; j++)
				{
					sprintf_s(sfname, "Assets/Sounds/%s%d.mp3", model_info[i].sound_filename, j + 1);

					const size_t cSize = strlen(sfname) + 1;
					wchar_t* wc = new wchar_t[cSize];
					//mbstowcs(wc, c, cSize);
					mbstowcs_s(&convertedChars, wc, cSize, sfname, _TRUNCATE);

					m_Res->m_audio.CreateSoundEffect(wc, &m_stuff_model[model_info[i].index]->m_soundEffects[j + 20]);

					m_Res->m_audio.SetSoundEffectVolume(&m_stuff_model[model_info[i].index]->m_soundEffects[j + 20], 0.3f);
					m_Res->m_audio.SetSoundEffectPitch(&m_stuff_model[model_info[i].index]->m_soundEffects[j + 20], 1.2f);
				}
			}
		}
	});
}

task<void> Stuff::LinkTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	m_Res->LoadDatabaseTextureInfo(&texture_info, "[model_index] > 999");

	std::vector<task<void>> tasks;

	for (int i = 0; i < texture_info.size(); i++)
	{
		texture_info[i].s_filename = ref new Platform::String(m_Res->GetWC(texture_info[i].filename));
		texture_info[i].index -= 1000;
		if (!strcmp(texture_info[i].type, "t"))
		{
			m_stuff_model[texture_info[i].index]->SetMaterialTexture(m_Res->m_Textures->GetTexturePointer(texture_info[i].filename), m_Res->GetWC(texture_info[i].material_name), texture_info[i].tex_slot, texture_info[i].alpha, texture_info[i].specular_level);
		}
		if (!strcmp(texture_info[i].type, "n"))
		{
			m_stuff_model[texture_info[i].index]->SetMaterialNormal(m_Res->m_Textures->GetTexturePointer(texture_info[i].filename), m_Res->GetWC(texture_info[i].material_name), texture_info[i].intensity);
		}
		if (!strcmp(texture_info[i].type, "e"))
		{
			m_stuff_model[texture_info[i].index]->SetMaterialEmmit(m_Res->m_Textures->GetTexturePointer(texture_info[i].filename), m_Res->GetWC(texture_info[i].material_name), texture_info[i].intensity);
		}
	}

	return when_all(tasks.begin(), tasks.end());
}

task<void> Stuff::LoadBinary(int level)
{
	return create_task([this, level]()
	{
		int i;

		cur_phy = 0;

		char info_filename[140];

		m_stuff.clear();

		stuff_tt temp_stuff;

		if (m_Res->bContentFolder == false)
		{
			sprintf_s(info_filename, "%s\\LevelBinary\\Stuff%d.bmp", m_Res->local_file_folder, level);
		}
		else
		{
			sprintf_s(info_filename, "Assets\\LevelBinary\\Stuff%d.bmp", level);
		}

		FILE * pFile;

		fopen_s(&pFile, info_filename, "rb");
		if (pFile != NULL)
		{
			fread_s(&cur_total, sizeof(int), sizeof(int), 1, pFile);
			for (i = 0; i < cur_total; i++)
			{
				fread_s(&temp_stuff.stuff, sizeof(stuff_t), sizeof(stuff_t), 1, pFile);
				if (temp_stuff.stuff.bActive == 1)
				{
					temp_stuff.bPhysical = false;
					temp_stuff.m_Body = nullptr;
					m_stuff.push_back(temp_stuff);
				}
			}
			fclose(pFile);
		}
		else
		{
			cur_total = 0;
		}
	});
}

void Stuff::CreateOne(int model_index, float x, float y, float z, float rx, float ry, float rz, float hft)
{
	int i;
	bool bFound = false;
	/*
	for (stuff_tt s : m_stuff)
	{
		if (s.stuff.info.pos.x == x &&
			s.stuff.info.pos.z == z)
		{
			bFound = true;
		}
	}
	*/
	if (true)//(bFound == false)
	{
		stuff_tt new_s;

		new_s.stuff.bActive = 1;
		//new_s.stuff.type = type;
		new_s.stuff.info.pos.x = x;
		new_s.stuff.info.pos.z = z;
		new_s.stuff.info.pos.y = y;
		new_s.stuff.info.dir.x = rx;
		new_s.stuff.info.dir.y = ry;
		new_s.stuff.info.dir.z = rz;
		new_s.stuff.model_index = model_index;
		new_s.stuff.height_from_terrain = hft;
		//new_s.stuff.type = m_static_model[model_index]->type;
		new_s.stuff.colour = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		//new_s.stuff.MakeMatrix();
		new_s.m_Body = nullptr;
		m_stuff.push_back(new_s);
		MakePhysics();
		//MakeCenters();
	}
}

bool Stuff::SaveBinary(int level)
{
	int i;
	char info_filename[140];
	FILE * pFile;

	sprintf_s(info_filename, "%s\\Stuff%d.bmp", m_Res->local_file_folder, level);

	fopen_s(&pFile, info_filename, "wb");
	if (pFile != NULL)
	{
		int total = 0;
		for (stuff_tt s : m_stuff)
		{
			if (s.stuff.bActive == 1)
			{
				total++;
			}
		}
		fwrite(&total, sizeof(int), 1, pFile);
		for (stuff_tt s : m_stuff)
		{
			if (s.stuff.bActive == 1)
			{
				fwrite(&s.stuff, sizeof(stuff_t), 1, pFile);
			}
		}
		fclose(pFile);
	}
	else
	{
		return false;
	}

	return true;
}

void Stuff::MakePhysics()
{
	int i;

	for (i = 0; i < m_stuff.size(); i++)
	{
		stuff_tt s = m_stuff[i];
		if (s.stuff.bActive == 1 && s.m_Body == nullptr)
		{
			s.stuff.info.group = m_stuff_model[s.stuff.model_index]->info.group;
			s.stuff.info.mask = m_stuff_model[s.stuff.model_index]->info.mask;
			s.stuff.info.mrf = m_stuff_model[s.stuff.model_index]->info.mrf;

			switch (m_stuff_model[s.stuff.model_index]->physics_shape)
			{
			case PHY_NOTHING:break;
			case PHY_BOX:s.m_Body = m_stuff_model[s.stuff.model_index]->MakePhysicsBoxFromMeshModel(&s.stuff.info, 1.0f); break;
			case PHY_CYLINDER:s.m_Body = m_stuff_model[s.stuff.model_index]->MakePhysicsCylinderFromMeshModel(&s.stuff.info, 1.0f); break;
			case PHY_SPHERE:s.m_Body = m_stuff_model[s.stuff.model_index]->MakePhysicsCylinderFromMeshModel(&s.stuff.info, 1.0f); break;
			case PHY_ELLIPSEOID:s.m_Body = m_stuff_model[s.stuff.model_index]->MakePhysicsEllipseoidFromMeshModel(&s.stuff.info, 1.0f); break;
			case PHY_CONVEXHULL:s.m_Body = m_stuff_model[s.stuff.model_index]->MakePhysicsConvexHullFromMeshModel(&s.stuff.info, 1.0f); break;
			}
		}

		s.bIsPhysics = false;
		if (s.m_Body != nullptr)
		{
			s.bIsPhysics = true;
			s.m_InitialTransform = s.m_Body->getWorldTransform();

			s.bPhysical = false;
			s.set_physics_inactive = false;
			s.set_physics_active = false;
			s.m_Body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		}
		m_stuff[i] = s;
	}
}

void Stuff::Clear(float x, float z)
{
	int i;
	for (i = 0; i < m_stuff.size(); i++)
	{
		stuff_tt s = m_stuff[i];

		float del_radius = 0.5f;

		if (m_Res->delete_radius > 1)
		{
			del_radius += 1.0f * (float)m_Res->delete_radius - 1;
		}

		if (s.stuff.info.pos.x > x - del_radius && s.stuff.info.pos.x < x + del_radius &&
			s.stuff.info.pos.z > z - del_radius && s.stuff.info.pos.z < z + del_radius)
		{
			s.stuff.bActive = 0;
			s.m_Body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);

			m_Res->m_Physics.m_dynamicsWorld->removeCollisionObject(s.m_Body);
		}
		m_stuff[i] = s;
	}
}

void Stuff::ResetAllPositions()
{
	int i;
	for (i = 0; i < m_stuff.size(); i++)
	{
		stuff_tt s = m_stuff[i];

		s.m_Body->setWorldTransform(s.m_InitialTransform);
		s.m_Body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
		s.m_Body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
		s.m_Body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		s.m_Body->setActivationState(ISLAND_SLEEPING);
		s.set_physics_inactive = false;
		s.bPhysical = false;

		s.set_physics_inactive = false;
		s.set_physics_active = false;

		s.stuff.model_matrix = m_Res->GetMatrix(s.m_Body);
		s.m_CurrentTransform = s.m_Body->getWorldTransform();

		m_stuff[i] = s;
	}
}

void Stuff::UpdatePhysics(bool make_active)
{
	int i;

	for (i = 0; i < m_stuff.size(); i++)
	{
		stuff_tt s = m_stuff[i];
		if (s.set_physics_inactive == true)
		{
			s.m_Body->setWorldTransform(s.m_InitialTransform);
			s.m_Body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
			s.m_Body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
			s.m_Body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);
			s.m_Body->setActivationState(ISLAND_SLEEPING);
			s.set_physics_inactive = false;
			s.bPhysical = false;
		}

		if (s.set_physics_active == true)
		{
			if (make_active == true && s.stuff.bActive == 1)
			{
				s.m_Body->setCollisionFlags(!btCollisionObject::CF_STATIC_OBJECT | !btCollisionObject::CF_NO_CONTACT_RESPONSE);
				s.m_Body->setActivationState(ISLAND_SLEEPING);
				s.set_physics_active = false;
			}

			s.bPhysical = true;
		}

		if (s.bPhysical == true)
		{
			s.stuff.model_matrix = m_Res->GetMatrix(s.m_Body);
			s.m_CurrentTransform = s.m_Body->getWorldTransform();
		}

		//m_stuff_model[model_info[i].index]->info.group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		//m_stuff_model[model_info[i].index]->info.mask = (COL_OBJECTS | COL_RAY);

		btBroadphaseProxy* proxy = s.m_Body->getBroadphaseProxy();
		if (proxy != nullptr)
		{
			if (s.m_Body->getActivationState() == ISLAND_SLEEPING)
			{
				proxy->m_collisionFilterGroup = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
				proxy->m_collisionFilterMask = (COL_TERRAIN | COL_RAY);
			}
			else
			{
				proxy->m_collisionFilterGroup = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
				proxy->m_collisionFilterMask = (COL_OBJECTS | COL_RAY);
			}
		}
		// Could change collision flags if island sleeping
		//btBroadphaseProxy* proxy = body->getBroadphaseProxy();
		//proxy->m_collisionFilterGroup = group;
		//proxy->m_collisionFilterMask = collidesWithMask;

		m_stuff[i] = s;
	}
}

task<void> Stuff::Update()
{
	return create_task([this]
	{
		int i;

		rp_stuff.clear();

		for (i = 0; i < m_stuff.size(); i++)
		{
			stuff_tt s = m_stuff[i];

			if (s.m_Body != nullptr && s.stuff.bActive == 1)
			{
				if (m_Res->m_Camera->Within3DManhattanEyeDistance(s.stuff.info.pos.x,
					s.stuff.info.pos.y,
					s.stuff.info.pos.z,
					m_Res->view_distance) > 0.0f)
				{
					float full_dist = m_Res->m_Camera->CheckPoint(s.m_CurrentTransform.getOrigin().getX(),
						s.m_CurrentTransform.getOrigin().getY(),
						s.m_CurrentTransform.getOrigin().getZ(), m_stuff_model[s.stuff.model_index]->m_mesh[0]->Extents().Radius);

					s.cam_dist = full_dist;

					if (s.bPhysical == true)
					{
						/*
						if (m_Res->m_Camera->Within3DManhattanEyeDistance(s.stuff.info.pos.x,
							s.stuff.info.pos.y,
							s.stuff.info.pos.z,
							full_dist > 0.0f ? MAX_VIEW_DISTANCE : MAX_VIEW_DISTANCE) < 0.0f)
						{
							// if also out of camera view
							if (m_Res->m_Camera->CheckPoint(s.m_CurrentTransform.getOrigin().getX(),
								s.m_CurrentTransform.getOrigin().getY(),
								s.m_CurrentTransform.getOrigin().getZ(), m_stuff_model[s.stuff.model_index]->OverallRadius) < 0.0f)
							{
								s.set_physics_inactive = true;
							}
						}*/
					}
					else
					{
						s.set_physics_active = true;
						/*
						if (m_Res->m_Camera->Within3DManhattanEyeDistance(s.stuff.info.pos.x,
							s.stuff.info.pos.y,
							s.stuff.info.pos.z,
							full_dist > 0.0f ? MAX_VIEW_DISTANCE : MAX_VIEW_DISTANCE) > 0.0f)
						{
						}*/
					}

					if (s.bPhysical == true)
					{
						if (full_dist > 0.0f)
						{
							rp_stuff.push_back(&m_stuff[i]);
							s.bVisible = true;
						}
						else
						{
							s.bVisible = false;
						}
					}
					else
					{
						s.bVisible = false;
					}
				}
				else
				{
					s.set_physics_active = false;
					s.set_physics_inactive = true;
					s.bVisible = false;
					s.bPhysical = false;
				}
			}
			m_stuff[i] = s;
		}

		std::sort(rp_stuff.begin(), rp_stuff.end(), stuff_compare{});
	});
}

void Stuff::Render(int alpha_mode)
{
	int i;

	for (stuff_tt* s : rp_stuff)
	{
		m_Res->m_Camera->m_constantBufferData.model = s->stuff.model_matrix;
		m_Res->m_Camera->UpdateConstantBuffer();
		for (Mesh* m : m_stuff_model[s->stuff.model_index]->m_mesh)
		{
			if (alpha_mode == 3)
			{
				m->Render(*m_Res, true);
			}
			else
			{
				m->Render(*m_Res);
			}
		}
	}
}

void Stuff::RenderDepth(int alpha_mode, int point_plane)
{
	int i;
	bool bRender;

	if (bLoadingComplete == false)
		return;

	for (stuff_tt* s : rp_stuff)
	{
		m_Res->m_Camera->m_constantBufferData.model = s->stuff.model_matrix;
		m_Res->m_Camera->UpdateConstantBuffer();

		if (alpha_mode == 3)
		{
			m_stuff_model[s->stuff.model_index]->m_mesh[0]->Render(*m_Res, true);
		}
		else
		{
			m_stuff_model[s->stuff.model_index]->m_mesh[0]->Render(*m_Res);
		}
	}
}