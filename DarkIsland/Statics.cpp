
#include "pch.h"
#include "Statics.h"
#include "DefPhysics.h"
#include <sqlite3.h>

using namespace Game;

#define MAX_STATICS 10000

// std::sort with inlining
struct statics_light_compare {
	// the compiler will automatically inline this
	bool operator()(const CG_POINT_LIGHT &a, const CG_POINT_LIGHT &b) {
		return a.dist < b.dist;
	}
};

// std::sort with inlining
struct statics_compare {
	// the compiler will automatically inline this
	bool operator()(const static_tt* a, const static_tt* b) {
		return a->cam_dist < b->cam_dist;
	}
};

void Statics::Reset()
{
	int i;
	for (i = 0; i < m_static.size(); i++)
	{
		static_tt s = m_static[i];

		s.center.clear();
		s.bMeshVisible.clear();
		s.stuff.bActive = 0;
		//if (s.m_Body != nullptr)
		//	s.m_Body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);

		m_static[i] = s;
	}

	m_static.clear();
	pm_static.clear();
}

Statics::Statics(AllResources* p_Resources, Level* pp_Level)
{
	int i;
	m_Res = p_Resources;
	p_Level = pp_Level;

	m_deviceResources = p_Resources->m_deviceResources;

	/*
	for (i = 0; i < 50; i++)
	{
		m_static_model[i] = new static_model_t(m_deviceResources, &p_Resources->m_Physics);
	}
*/
	bRandomness = false;

	total_static_models = 30;
}

concurrency::task<void> Statics::LoadModels()
{
	m_Res->LoadDatabaseModelInfo(&model_info, "[Index] < 1000");

	std::vector<concurrency::task<void>> tasks;

	for (int i = 0; i < model_info.size(); i++)
	{
		m_static_model[model_info[i].index] = new static_model_t(m_deviceResources, &m_Res->m_Physics);

		tasks.push_back(m_static_model[model_info[i].index]->LoadModel(m_Res, m_Res->GetWC(model_info[i].filename), model_info[i].physics_shape, model_info[i].scale));
	}
	total_static_models = 0;

	return when_all(begin(tasks), end(tasks)).then([this]()
	{
		for (int i = 0; i < model_info.size(); i++)
		{
			m_static_model[model_info[i].index]->info.mrf = XMFLOAT3(model_info[i].mass, model_info[i].restitution, model_info[i].friction);
			m_static_model[model_info[i].index]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
			m_static_model[model_info[i].index]->info.mask = (COL_TERRAIN | COL_RAY); // Statics are terrain

			m_static_model[model_info[i].index]->type = model_info[i].type;

			strcpy_s(m_static_model[model_info[i].index]->group, model_info[i].group);
			strcpy_s(m_static_model[model_info[i].index]->filename, model_info[i].filename);
			//m_static_model[model_info[i].index]->light_radius =

			m_static_model[model_info[i].index]->info.dir.x = model_info[i].rotx;
			m_static_model[model_info[i].index]->info.dir.y = model_info[i].roty;
			m_static_model[model_info[i].index]->info.dir.z = model_info[i].rotz;

			m_static_model[model_info[i].index]->CalcOverallRadius();

			//total_static_models++;

			if (model_info[i].index > total_static_models)
				total_static_models = model_info[i].index;
		}
	});
}

concurrency::task<void> Statics::LinkTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	m_Res->LoadDatabaseTextureInfo(&texture_info, "[model_index] < 1000");

	std::vector<task<void>> tasks;
	//m_Res->m_Textures->GetTexturePointer()
	for (int i = 0; i < texture_info.size(); i++)
	{
		texture_info[i].s_filename = ref new Platform::String(m_Res->GetWC(texture_info[i].filename));

		if (!strcmp(texture_info[i].type, "t"))
		{
			m_static_model[texture_info[i].index]->SetMaterialTexture(m_Res->m_Textures->GetTexturePointer(texture_info[i].filename), m_Res->GetWC(texture_info[i].material_name), texture_info[i].tex_slot, texture_info[i].alpha, texture_info[i].specular_level);
			//tasks.push_back(loader->LoadTextureAsync(texture_info[i].s_filename, nullptr, m_static_model[texture_info[i].index]->GetMaterialTexture(m_Res->GetWC(texture_info[i].material_name), texture_info[i].tex_slot, texture_info[i].alpha)));
		}
		if (!strcmp(texture_info[i].type, "n"))
		{
			m_static_model[texture_info[i].index]->SetMaterialNormal(m_Res->m_Textures->GetTexturePointer(texture_info[i].filename), m_Res->GetWC(texture_info[i].material_name), texture_info[i].intensity);
			//tasks.push_back(loader->LoadTextureAsync(texture_info[i].s_filename, nullptr, m_static_model[texture_info[i].index]->GetMaterialNormal(m_Res->GetWC(texture_info[i].material_name), texture_info[i].intensity)));
		}
		if (!strcmp(texture_info[i].type, "e"))
		{
			m_static_model[texture_info[i].index]->SetMaterialEmmit(m_Res->m_Textures->GetTexturePointer(texture_info[i].filename), m_Res->GetWC(texture_info[i].material_name), texture_info[i].intensity);
			//tasks.push_back(loader->LoadTextureAsync(texture_info[i].s_filename, nullptr, m_static_model[texture_info[i].index]->GetMaterialEmmit(m_Res->GetWC(texture_info[i].material_name), texture_info[i].intensity)));
		}
	}

	return when_all(tasks.begin(), tasks.end());
}

concurrency::task<void> Statics::LoadBinary(int level)
{
	return concurrency::create_task([this, level]()
	{
		int i, total;
		static_tt stat;

		Reset();
		//return true;

		char info_filename[140];

		if (m_Res->bContentFolder == false)
		{
			sprintf_s(info_filename, "%s\\LevelBinary\\Statics%d.bmp", m_Res->local_file_folder, level);
		}
		else
		{
			sprintf_s(info_filename, "Assets\\LevelBinary\\Statics%d.bmp", level);
		}

		FILE * pFile;

		fopen_s(&pFile, info_filename, "rb");
		if (pFile != NULL)
		{
			fread_s(&total, sizeof(int), sizeof(int), 1, pFile);
			for (i = 0; i < total; i++)
			{
				fread_s(&stat.stuff, sizeof(static_t), sizeof(static_t), 1, pFile);
				if (stat.stuff.colour.x == 0.0f &&
					stat.stuff.colour.y == 0.0f &&
					stat.stuff.colour.z == 0.0f &&
					stat.stuff.colour.w == 0.0f)
				{
					stat.stuff.colour = XMFLOAT4(float(rand() % 100)*0.01f, float(rand() % 100)*0.01f, float(rand() % 100)*0.01f, 1.0f);
				}
				stat.m_Body.clear();
				m_static.push_back(stat);
			}
			fclose(pFile);

			MakeCenters();

			return;
		}
		else
		{
			return;
		}
	});
}

bool Statics::SaveBinary(int level)
{
	int i;
	char info_filename[140];
	FILE * pFile;

	sprintf_s(info_filename, "%s\\Statics%d.bmp", m_Res->local_file_folder, level);

	fopen_s(&pFile, info_filename, "wb");
	if (pFile != NULL)
	{
		int total = 0;
		for (static_tt s : m_static)
		{
			if (s.stuff.bActive == 1)
			{
				total++;
			}
		}
		fwrite(&total, sizeof(int), 1, pFile);
		for (static_tt s : m_static)
		{
			if (s.stuff.bActive == 1)
			{
				s.stuff.y = s.stuff.model_matrix._24;

				fwrite(&s.stuff, sizeof(static_t), 1, pFile);
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

void Statics::MakeCenters()
{
	for (int i = 0; i < m_static.size(); i++)
	{
		static_tt s = m_static[i];
		XMFLOAT3 center;
		XMVECTOR rec_res;

		s.bMeshVisible.resize(m_static_model[s.stuff.model_index]->m_mesh.size());

		s.center.clear();
		for (Mesh* m : m_static_model[s.stuff.model_index]->m_mesh)
		{
			rec_res = XMVector3Rotate(XMLoadFloat3(
				&XMFLOAT3(m->Extents().CenterX,
					m->Extents().CenterY,
					m->Extents().CenterZ)), XMQuaternionRotationRollPitchYaw(s.stuff.rot_y, s.stuff.rot_x, s.stuff.rot_z));

			XMStoreFloat3(&center, rec_res);

			center.x += s.stuff.x;
			center.y += s.stuff.y;
			center.z += s.stuff.z;

			s.center.push_back(center);
		}

		if (s.stuff.model_index == 3)
		{
			XMFLOAT3 light_pos = XMFLOAT3(0.0f, 4.3f, 0.0f);

			rec_res = XMVector3Rotate(XMLoadFloat3(
				&XMFLOAT3(light_pos.x,
					light_pos.y,
					light_pos.z)), XMQuaternionRotationRollPitchYaw(s.stuff.rot_y, s.stuff.rot_x, s.stuff.rot_z));

			XMStoreFloat3(&s.light_position, rec_res);

			s.light_position.x += s.stuff.x;
			s.light_position.y += s.stuff.y;
			s.light_position.z += s.stuff.z;
		}

		if (s.stuff.model_index == 4) // fire
		{
			XMFLOAT3 light_pos = XMFLOAT3(0.0f, 1.0f, 0.0f);

			rec_res = XMVector3Rotate(XMLoadFloat3(
				&XMFLOAT3(light_pos.x,
					light_pos.y,
					light_pos.z)), XMQuaternionRotationRollPitchYaw(s.stuff.rot_y, s.stuff.rot_x, s.stuff.rot_z));

			XMStoreFloat3(&s.light_position, rec_res);

			s.light_position.x += s.stuff.x;
			s.light_position.y += s.stuff.y;
			s.light_position.z += s.stuff.z;
		}

		if (s.stuff.model_index == 30)
		{
			for (Mesh* m : m_static_model[s.stuff.model_index]->m_mesh)
			{
				if (m->GetName()->compare(L"LIGHT_POS") == 0)
				{
					XMFLOAT3 light_pos = XMFLOAT3(m->Extents().CenterX, m->Extents().CenterY, m->Extents().CenterZ);

					rec_res = XMVector3Rotate(XMLoadFloat3(
						&XMFLOAT3(light_pos.x,
							light_pos.y,
							light_pos.z)), XMQuaternionRotationRollPitchYaw(s.stuff.rot_y, s.stuff.rot_x, s.stuff.rot_z));

					XMStoreFloat3(&s.light_position, rec_res);

					s.light_position.x += s.stuff.x;
					s.light_position.y += s.stuff.y;
					s.light_position.z += s.stuff.z;
					s.light_angle = 0.0f;
					//s.light_direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
				}
			}
		}
		m_static[i] = s;
	}
}

void Statics::FinalizeCreateDeviceResources()
{
	m_skinnedMeshRenderer.Initialize(m_Res->m_deviceResources->GetD3DDevice(), m_Res->m_materialBuffer);
}

void Statics::MakePhysics()
{
	int i;

	for (i = 0; i < m_static.size(); i++)
	{
		static_tt s = m_static[i];
		//s.m_Body = nullptr;
		if (s.stuff.bActive == 1 && s.m_Body.size() == 0)
		{
			s.stuff.MakeMatrix();

			s.info = m_static_model[s.stuff.model_index]->info;
			//s.info.mrf = XMFLOAT3(1.0f, 1.0f, 1.0f);
			s.info.pos = XMFLOAT3(s.stuff.x, s.stuff.y, s.stuff.z);
			s.info.dir = XMFLOAT3(s.stuff.rot_x, s.stuff.rot_y, s.stuff.rot_z);

			s.m_Body.clear();

			switch (m_static_model[s.stuff.model_index]->physics_shape)
			{
			case PHY_NOTHING:break;
			case PHY_BOX:s.m_Body.push_back(m_static_model[s.stuff.model_index]->MakePhysicsBoxFromMeshModel(&s.info, 1.0f)); break;
			case PHY_CYLINDER:s.m_Body.push_back(m_static_model[s.stuff.model_index]->MakePhysicsCylinderFromMeshModel(&s.info, 1.0f)); break;
			case PHY_SPHERE:s.m_Body.push_back(m_static_model[s.stuff.model_index]->MakePhysicsSphereFromMeshModel(&s.info, 1.0f)); break;
			case PHY_ELLIPSEOID:s.m_Body.push_back(m_static_model[s.stuff.model_index]->MakePhysicsEllipseoidFromMeshModel(&s.info, 1.0f)); break;
			case PHY_CONVEXHULL:s.m_Body.push_back(m_static_model[s.stuff.model_index]->MakePhysicsConvexHullFromMeshModel(&s.info, 1.0f)); break;
			}

			for (Mesh* m : m_static_model[s.stuff.model_index]->m_mesh)
			{
				if (m->GetName()->compare(L"PHY_BOX") == 0 || m->GetName()->compare(L"PHY_BOX_R") == 0)
				{
					s.m_Body.push_back(m->MakePhysicsBoxFromMesh(&s.info, 1.0f, &m_Res->m_Physics));
				}
				if (m->GetName()->compare(L"PHY_CYLINDER") == 0 || m->GetName()->compare(L"PHY_CYLINDER_R") == 0)
				{
					s.m_Body.push_back(m->MakePhysicsCylinderFromMesh(&s.info, 1.0f, &m_Res->m_Physics));
				}
				if (m->GetName()->compare(L"PHY_SPHERE") == 0 || m->GetName()->compare(L"PHY_SPHERE_R") == 0)
				{
					s.m_Body.push_back(m->MakePhysicsSphereFromMesh(&s.info, 1.0f, &m_Res->m_Physics));
				}
				if (m->GetName()->compare(L"PHY_ELLIPSEOID") == 0 || m->GetName()->compare(L"PHY_ELLIPSEOID_R") == 0)
				{
					s.m_Body.push_back(m->MakePhysicsEllipseoidFromMesh(&s.info, 1.0f, &m_Res->m_Physics));
				}
				if (m->GetName()->compare(L"PHY_CONVEXHULL") == 0 || m->GetName()->compare(L"PHY_CONVEXHULL_R") == 0)
				{
					s.m_Body.push_back(m->MakePhysicsConvexHullFromMesh(&s.info, 1.0f, &m_Res->m_Physics));
				}
			}
		}
		//s.bIsPhysics = false;
		if (s.m_Body.size() > 0)
		{
			s.m_InitialTransform = s.m_Body[0]->getWorldTransform();
			s.bIsPhysics = true;
			s.set_physics_active = false;
			s.set_physics_inactive = false;
			s.bPhysical = false;

			for (btRigidBody* b : s.m_Body)
			{
				b->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);
			}
		}

		m_static[i] = s;
	}
}

void Statics::CreateOne(int model_index, float x, float y, float z, float rx, float ry, float rz, float hft)
{
	int i;
	bool bFound = false;

	//return;
	static_tt new_s;

	new_s.stuff.bActive = 1;
	//new_s.stuff.type = type;
	new_s.stuff.x = x;
	new_s.stuff.z = z;
	new_s.stuff.y = y;
	new_s.stuff.height_from_terrain = hft;
	new_s.stuff.rot_x = rx;
	new_s.stuff.rot_y = ry;
	new_s.stuff.rot_z = rz;
	new_s.stuff.scale = 1.0f;
	new_s.stuff.model_index = model_index;
	new_s.stuff.type = m_static_model[model_index]->type;

	new_s.stuff.colour = XMFLOAT4(m_Res->col_r / 255.0f, m_Res->col_g / 255.0f, m_Res->col_b / 255.0f, 1.0f);
	new_s.stuff.MakeMatrix();
	new_s.m_Body.clear();

	m_static.push_back(new_s);
	MakeCenters();
	MakePhysics();
}

void Statics::Clear(float x, float z)
{
	int i;
	for (i = 0; i < m_static.size(); i++)
	{
		static_tt s = m_static[i];

		float del_radius = 0.5f;

		if (m_Res->delete_radius > 1)
		{
			del_radius += 1.0f * (float)m_Res->delete_radius - 1;
		}

		if (s.stuff.x > x - del_radius && s.stuff.x < x + del_radius &&
			s.stuff.z > z - del_radius && s.stuff.z < z + del_radius)
		{
			s.stuff.bActive = 0;
			for (btRigidBody* b : s.m_Body)
			{
				b->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);
			}
		}
		m_static[i] = s;
	}
}

void Statics::MakeAllPhysical()
{
	int i;
	for (i = 0; i < m_static.size(); i++)
	{
		static_tt s = m_static[i];
		if (s.bIsPhysics == true)
		{
			for (btRigidBody* b : s.m_Body)
			{
				b->setCollisionFlags(!btCollisionObject::CF_STATIC_OBJECT | !btCollisionObject::CF_NO_CONTACT_RESPONSE);
			}
		}
	}
}

void Statics::UpdatePhysics()
{
	int i;
	for (i = 0; i < m_static.size(); i++)
	{
		static_tt s = m_static[i];
		if (s.bIsPhysics == true)
		{
			if (s.set_physics_inactive == true)
			{
				for (btRigidBody* b : s.m_Body)
				{
					b->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);
				}
				s.set_physics_inactive = false;
				s.bPhysical = false;
			}
			if (s.set_physics_active == true)
			{
				for (btRigidBody* b : s.m_Body)
				{
					b->setCollisionFlags(!btCollisionObject::CF_STATIC_OBJECT | !btCollisionObject::CF_NO_CONTACT_RESPONSE);
				}
				s.set_physics_active = false;
				s.bPhysical = true;
			}
		}
		m_static[i] = s;
	}
}

Concurrency::task<void> Statics::Update(float timeDelta, float timeTotal)
{
	return concurrency::create_task([this, timeDelta, timeTotal]
	{
		if (true)
		{
			m_point_lights.clear();
			m_spot_lights.clear();

			m_Fire->Reset();

			unsigned int seed = 237;
			float pnscale = m_Res->m_LevelInfo.wind.w*1.04f;

			PerlinNoise pn(seed);

			int i;

			pm_static.clear();

			float light_dist;

			m_Fire->ResetFireVolume();

			for (i = 0; i < m_static.size(); i++)
			{
				static_tt s = m_static[i];

				if (s.stuff.bActive == 1)
				{
					for (Mesh* m : m_static_model[s.stuff.model_index]->m_mesh)
					{
						if (m->GetName()->compare(L"LIGHT_POINT") == 0)
						{
							XMFLOAT3 light_pos = XMFLOAT3(m->Extents().CenterX, m->Extents().CenterY, m->Extents().CenterZ);

							XMVECTOR rec_res = XMVector3Rotate(XMLoadFloat3(
								&XMFLOAT3(light_pos.x,
									light_pos.y,
									light_pos.z)), XMQuaternionRotationRollPitchYaw(s.stuff.rot_y, s.stuff.rot_x, s.stuff.rot_z));

							XMStoreFloat3(&s.light_position, rec_res);

							s.light_position.x += s.stuff.x;
							s.light_position.y += s.stuff.y;
							s.light_position.z += s.stuff.z;

							//pnscale = 5.5f;
							//float add_val = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*8.0f)*0.1f;
							float add_valr = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*8.0f);// *0.01f;
							float add_valg = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*5.0f);// *0.01f;
							float add_valb = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*1.0f);// *0.01f;
							add_valr = 0.5f + ((add_valr * add_valg * add_valb)*0.2f);

							light_dist = m_Res->m_Camera->CheckPoint(s.light_position.x, s.light_position.y, s.light_position.z, POINT_LIGHT_RADIUS);
							if (light_dist > 0.0f)// && light_dist<LIGHT_RADIUS)
							{
								float add_val = 0.0f;// (pn.noise(timeTotal + static_item.x*pnscale, timeTotal + static_item.z*pnscale, timeTotal*2.0f)*0.1f) - 0.05f;
								float add_val2 = 0.0f;// (pn.noise(timeTotal + 2.0f + static_item.x*pnscale, timeTotal + 2.0f + static_item.z*pnscale, timeTotal*2.0f)*0.1f) - 0.05f;

								CG_POINT_LIGHT fire_point_light;
								fire_point_light.ambient = XMFLOAT4((s.stuff.colour.x + add_valr)*LIGHT_POINT_MULTIPLIER, (s.stuff.colour.y + add_valg) *LIGHT_POINT_MULTIPLIER, (s.stuff.colour.z + add_valb)*LIGHT_POINT_MULTIPLIER, 0.5f); ;
								fire_point_light.diffuse = XMFLOAT4((s.stuff.colour.x * add_valr)*LIGHT_POINT_MULTIPLIER, (s.stuff.colour.y * add_valr) *LIGHT_POINT_MULTIPLIER, (s.stuff.colour.z * add_valr)*LIGHT_POINT_MULTIPLIER, 1.0f); ;
								fire_point_light.specular = XMFLOAT4(s.stuff.colour.x*LIGHT_POINT_MULTIPLIER, s.stuff.colour.y*LIGHT_POINT_MULTIPLIER, s.stuff.colour.z*LIGHT_POINT_MULTIPLIER, 0.5f); ;

								fire_point_light.pos = XMFLOAT3(s.light_position.x, s.light_position.y, s.light_position.z);

								fire_point_light._specular_power = 20.0f;
								fire_point_light.bCastShadows = false;
								fire_point_light.radius = POINT_LIGHT_RADIUS;
								fire_point_light.fire_scale = 0.3f;
								fire_point_light.dist = light_dist;

								m_point_lights.push_back(fire_point_light);

								m_Fire->SetFire(s.light_position.x, s.light_position.y, s.light_position.z, 0.3f, timeTotal, timeDelta, fire_point_light.diffuse);
							}
						}
					}

					if (s.stuff.model_index == 3)
					{
						light_dist = m_Res->m_Camera->CheckPoint(s.light_position.x, s.light_position.y, s.light_position.z, POINT_LIGHT_RADIUS);
						if (light_dist > 0.0f)// && light_dist<LIGHT_RADIUS)
						{
							float add_val = 0.0f;// (pn.noise(timeTotal + static_item.x*pnscale, timeTotal + static_item.z*pnscale, timeTotal*2.0f)*0.1f) - 0.05f;
							float add_val2 = 0.0f;// (pn.noise(timeTotal + 2.0f + static_item.x*pnscale, timeTotal + 2.0f + static_item.z*pnscale, timeTotal*2.0f)*0.1f) - 0.05f;

							float add_valr = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*8.0f);// *0.01f;
							float add_valg = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*5.0f);// *0.01f;
							float add_valb = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*1.0f);// *0.01f;
							add_valr = 0.5f + ((add_valr * add_valg * add_valb)*0.2f);

							CG_POINT_LIGHT fire_point_light;
							fire_point_light.ambient = XMFLOAT4(s.stuff.colour.x*LIGHT_POINT_MULTIPLIER, s.stuff.colour.y*LIGHT_POINT_MULTIPLIER, s.stuff.colour.z*LIGHT_POINT_MULTIPLIER, 0.5f); ;
							fire_point_light.diffuse = XMFLOAT4((s.stuff.colour.x * add_valr)*LIGHT_POINT_MULTIPLIER, (s.stuff.colour.y * add_valr) *LIGHT_POINT_MULTIPLIER, (s.stuff.colour.z * add_valr)*LIGHT_POINT_MULTIPLIER, 1.0f); ;
							fire_point_light.specular = XMFLOAT4(s.stuff.colour.x*LIGHT_POINT_MULTIPLIER, s.stuff.colour.y*LIGHT_POINT_MULTIPLIER, s.stuff.colour.z*LIGHT_POINT_MULTIPLIER, 0.5f); ;
							fire_point_light.pos = XMFLOAT3(s.light_position.x, s.light_position.y, s.light_position.z);
							fire_point_light._specular_power = 0.0f;
							fire_point_light.bCastShadows = false;
							fire_point_light.radius = POINT_LIGHT_RADIUS;
							fire_point_light.fire_scale = 0.3f;
							fire_point_light.dist = light_dist;

							m_point_lights.push_back(fire_point_light);

							m_Fire->SetFire(s.light_position.x, s.light_position.y, s.light_position.z, 0.3f, timeTotal, timeDelta, fire_point_light.diffuse);
						}
					}

					if (s.stuff.model_index == 4)
					{
						light_dist = m_Res->m_Camera->CheckPoint(s.light_position.x, s.light_position.y, s.light_position.z, POINT_LIGHT_RADIUS);
						if (light_dist > 0.0f)
						{
							float add_val = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*8.0f)*0.1f;
							float add_valr = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*4.0f)*0.1f;
							float add_valg = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*2.0f)*0.1f;
							float add_valb = pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*0.5f)*0.1f;

							CG_POINT_LIGHT fire_point_light;
							fire_point_light.ambient = XMFLOAT4((0.2f + add_val + add_valr)*LIGHT_POINT_MULTIPLIER, (0.1f + add_val + add_valg)*LIGHT_POINT_MULTIPLIER, (0.1f + add_val + add_valb)*LIGHT_POINT_MULTIPLIER, 1.0f);
							fire_point_light.diffuse = XMFLOAT4((0.3f + add_val + add_valr)*LIGHT_POINT_MULTIPLIER, (0.2f + add_val + add_valg)*LIGHT_POINT_MULTIPLIER, (0.2f + add_val + add_valb)*LIGHT_POINT_MULTIPLIER, 1.0f);
							fire_point_light.specular = XMFLOAT4((0.2f + add_val + add_valr)*LIGHT_POINT_MULTIPLIER, (0.1f + add_val + add_valg)*LIGHT_POINT_MULTIPLIER, (0.1f + add_val + add_valb)*LIGHT_POINT_MULTIPLIER, 0.9998f);
							fire_point_light.pos = XMFLOAT3(s.light_position.x, s.light_position.y, s.light_position.z);
							//fire_point_light.radius = m_static_model[static_item.model_index]->light_radius;
							fire_point_light._specular_power = 0.0f;
							fire_point_light.bCastShadows = false;
							fire_point_light.radius = POINT_LIGHT_RADIUS;
							fire_point_light.fire_scale = 0.8f;
							fire_point_light.dist = light_dist;

							m_point_lights.push_back(fire_point_light);

							m_Fire->SetFire(s.stuff.x, s.stuff.y, s.stuff.z, 0.8f, timeTotal, timeDelta, fire_point_light.diffuse);

							float atten = (1.0f - (light_dist / MAX_SFX_DISTANCE)) *(1.0 / (1.0 + 0.1*light_dist + 0.01*light_dist*light_dist));

							m_Fire->SetFireVolume(atten);
						}
					}

					if (s.stuff.model_index == 30)
					{
						s.light_angle += 0.3f * timeDelta;
						if (s.light_angle > M_PI*2.0f)
						{
							s.light_angle -= M_PI * 2.0f;
						}

						light_dist = m_Res->m_Camera->CheckPoint(s.light_position.x, s.light_position.y, s.light_position.z, 150.0f);
						if (true)//(light_dist>0.0f)
						{
							float add_val = 1.0f;// pn.noise(timeTotal + s.stuff.x*pnscale, timeTotal + s.stuff.z*pnscale, timeTotal*8.0f)*0.3f;

							CG_SPOT_LIGHT lighthouse_light;
							lighthouse_light.ambient = XMFLOAT4((0.5f + add_val)*LIGHT_SPOT_MULTIPLIER, (0.1f + add_val)*LIGHT_SPOT_MULTIPLIER, (0.1f + add_val)*LIGHT_SPOT_MULTIPLIER, 1.0f);
							lighthouse_light.diffuse = XMFLOAT4((0.2f + add_val)*LIGHT_SPOT_MULTIPLIER, (0.15f + add_val)*LIGHT_SPOT_MULTIPLIER, (0.15f + add_val)*LIGHT_SPOT_MULTIPLIER, 1.0f);
							lighthouse_light.specular = XMFLOAT4((0.2f + add_val)*LIGHT_SPOT_MULTIPLIER, (0.1f + add_val)*LIGHT_SPOT_MULTIPLIER, (0.1f + add_val)*LIGHT_SPOT_MULTIPLIER, 0.9998f);
							lighthouse_light.pos = XMFLOAT3(s.light_position.x, s.light_position.y, s.light_position.z);
							//lighthouse_light.radius = m_static_model[static_item.model_index]->light_radius;
							lighthouse_light._specular_power = 0.0f;
							lighthouse_light.bCastShadows = false;

							lighthouse_light.radius = 150.0f;
							//lighthouse_light.pos = XMFLOAT3(s.stuff., m_Res->m_Camera->Eye().y, m_Res->m_Camera->Eye().z);
							//m_Res->m_Camera->LookDirection();
							//spot_light.dir = XMFLOAT3(-m_Res->m_Camera->LookingDir().x, -m_Res->m_Camera->LookingDir().y, -m_Res->m_Camera->LookingDir().z);

							lighthouse_light.up = XMFLOAT3(0.0f, 1.0f, 0.0f);
							lighthouse_light.spot = 7.0f;

							lighthouse_light._specular_power = 20.0f;
							lighthouse_light.lightmap = 1;
							//m_Res->m_Lights->AddSpot(spot_light);

							btVector3 light_rot = btVector3(1.0f, 0.0f, 0.0f);
							light_rot = light_rot.rotate(btVector3(0.0f, 1.0f, 0.0f), s.light_angle);

							lighthouse_light.dir = XMFLOAT3(light_rot.getX(), light_rot.getY(), light_rot.getZ());

							m_spot_lights.push_back(lighthouse_light);
						}
					}

					if (m_Res->m_Camera->Within3DManhattanEyeDistance(s.center[0].x,
						s.center[0].y,
						s.center[0].z,
						m_Res->view_distance) > 0.0f)
					{
						//float full_dist = m_Res->m_Camera->CheckPoint(s.info.pos.x + m_static_model[s.stuff.model_index]->OverallCenterX,
						//	s.info.pos.y + m_static_model[s.stuff.model_index]->OverallCenterY,
						//	s.info.pos.z + m_static_model[s.stuff.model_index]->OverallCenterZ, m_static_model[s.stuff.model_index]->OverallRadius) + 1000.0f;

						float full_dist = m_Res->m_Camera->CheckPoint(s.stuff.x + m_static_model[s.stuff.model_index]->OverallCenterX,
							s.stuff.y + m_static_model[s.stuff.model_index]->OverallCenterY,
							s.stuff.z + m_static_model[s.stuff.model_index]->OverallCenterZ, m_static_model[s.stuff.model_index]->OverallRadius);

						//float full_dist = m_Res->m_Camera->CheckPoint(s.center[0].x,
						//	s.center[0].y,
						//	s.center[0].z, m_static_model[s.stuff.model_index]->m_mesh[0]->Extents().Radius);

						s.cam_dist = full_dist;
						//full_dist = 0.0f;
						if (full_dist > 0.0f)
						{
							pm_static.push_back(&m_static[i]);
							s.bVisible = true;

							for (int i = 0; i < s.center.size(); i++)
							{
								if (/*m_Res->m_Camera->Within3DManhattanEyeDistance(s.center[i].x,
									s.center[i].y,
									s.center[i].z, m_static_model[s.stuff.model_index]->m_mesh[i]->Extents().Radius) > 0.0f

									&&
									*/
									m_Res->m_Camera->CheckPoint(s.center[i].x,
										s.center[i].y,
										s.center[i].z, m_static_model[s.stuff.model_index]->m_mesh[i]->Extents().Radius) > 0.0f)
								{
									s.bMeshVisible[i] = true;
								}
								else
								{
									s.bMeshVisible[i] = false;
								}
							}
						}
						else
						{
							s.bVisible = false;
							//for (int i = 0; i < s.center.size(); i++)
							//{
							//	s.bMeshVisible[i] = false;
							//}
						}

						if (s.stuff.type == 1)
						{
							if (false)// (m_Res->m_Camera->DistanceEst(
								//m_Res->player_x, m_Res->player_y, m_Res->player_z,
								//s.stuff.x, s.stuff.y, s.stuff.z) < 1.0f)
							{
								//s.stuff.bActive = 0;
								//m_Steam->CreateOne(s.x, s.y, s.z, 0.0f, 0.1f, 0.0f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 2);

								//m_Res->m_audio.PlaySoundEffect(SFXPop);
							}

							s.stuff.rot_x += 1.5f * timeDelta;
							if (s.stuff.rot_x > M_PI*2.0f)
							{
								s.stuff.rot_x -= M_PI * 2.0f;
							}
							s.stuff.MakeMatrix();
						}
					}
					else
					{
					}
				}

				if (s.bIsPhysics == 1)
				{
					if (s.bPhysical == true)
					{
						if (m_Res->m_Camera->Within3DManhattanEyeDistance(s.center[0].x,
							s.center[0].y,
							s.center[0].z,
							m_Res->view_distance * 0.5) < 0.0f)
						{
							s.set_physics_inactive = true;
							s.bPhysical = false;
						}
					}
					else
					{
						if (m_Res->m_Camera->Within3DManhattanEyeDistance(s.center[0].x,
							s.center[0].y,
							s.center[0].z,
							m_Res->view_distance * 0.5) > 0.0f)
						{
							s.set_physics_active = true;
							s.bPhysical = true;
						}
					}
				}

				m_static[i] = s;
			}

			std::sort(pm_static.begin(), pm_static.end(), statics_compare{});

			std::sort(m_point_lights.begin(), m_point_lights.end(), statics_light_compare{});

			for (i = 0; i < m_point_lights.size(); i++)
			{
				m_Res->m_Lights->AddPoint(m_point_lights.at(i));
				//m_Fire->SetFire(m_point_lights.at(i).pos.x, m_point_lights.at(i).pos.y, m_point_lights.at(i).pos.z, m_point_lights.at(i).fire_scale, timeTotal, timeDelta, m_point_lights.at(i).diffuse);
			}

			for (i = 0; i < m_spot_lights.size(); i++)
			{
				m_Res->m_Lights->AddSpot(m_spot_lights.at(i));
				//m_Fire->SetFire(m_point_lights.at(i).pos.x, m_point_lights.at(i).pos.y, m_point_lights.at(i).pos.z, m_point_lights.at(i).fire_scale, timeTotal, timeDelta, m_point_lights.at(i).diffuse);
			}
		}
	});
}

void Statics::Render(int alpha_mode, bool _bdef)
{
	int i;

	for (static_tt* s : pm_static)
	{
		m_Res->m_Camera->m_constantBufferData.model = s->stuff.model_matrix;
		m_Res->m_Camera->UpdateConstantBuffer();

		i = 0;
		for (Mesh* m : m_static_model[s->stuff.model_index]->m_mesh)
		{
			if (s->bMeshVisible[i] == true)
			{
				if (alpha_mode == 3)
				{
					m->Render(*m_Res, true);
				}
				else
				{
					if (m->alpha_mode == alpha_mode)
					{
						m->Render(*m_Res);
					}
				}
			}

			i++;
		}
	}
}

void Statics::RenderDepth(int alpha_mode, int point_plane)
{
	int i;
	bool bRender;
}