#include "pch.h"
#include "Fog.h"
#include "DefParticle.h"

using namespace Game;

// std::sort with inlining
struct fog_compare {
	// the compiler will automatically inline this
	bool operator()(const fog_t* a, const fog_t* b) {
		return a->cam_dist > b->cam_dist;
	}
};

void Fog::Initialize(Level* pp_Level, bool _bInstanced)
{
	p_Level = pp_Level;

	m_maxParticles = MAX_FOG_PARTICLES;

	fog.resize(m_maxParticles);

	bUpdate = true;

	current_particle_que = 0;

	m_particleSize = 1.0f;

	current_point = 0;

	total_collected = 0;

	rand_state = 0;  // <- state of the RNG (input and output)

	InitializeBuffers(_bInstanced);

	m_indexCount = 0;

	pn = new PerlinNoise(237);

	//CreateDDSTextureFromFile((ID3D11Device *)m_deviceResources->GetD3DDevice(), L"Assets/Textures/snow.dds", nullptr, &m_Texture, MAXSIZE_T);

	//m_Texture = m_Res->m_Textures->LoadTexture("misc_smoke");

	for (int i = 0; i < m_maxParticles; i++)
	{
		fog[i].bActive = 0;
	}
}

task<void> Fog::LoadTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\misc_smoke.dds", nullptr, m_Texture[0].GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("perturb001.dds", nullptr, m_PerturbTexture.GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("noise01.dds", nullptr, m_Texture.GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("alpha1.dds", nullptr, m_Texture.GetAddressOf()));

	return when_all(tasks.begin(), tasks.end());// .then([this]
}

void Fog::CreateOne(float x, float y, float z)
{
	//int point_find = 0;
	//point_find = current_point;
	if (fog[current_point].bActive == 0) //&& z < p_Level->front_back_walls && z > -p_Level->front_back_walls)
	{
		fog[current_point].bActive = 1;
		fog[current_point].vel_y = -0.1f;//  (((rand() % 100) / 50.0f) + 1.0f)*0.1f;
		fog[current_point].vel_x = (((rand() % 100) / 50.0f) + 0.3f)*0.2f; // 0.0f; //((localrand() % 50) / 150.0f)*0.2f;
		fog[current_point].vel_z = (((rand() % 100) / 50.0f) + 0.3f)*0.2f;
		fog[current_point].x = x;
		fog[current_point].y = y;
		fog[current_point].z = z;
		fog[current_point].v1 = 0.125f*(float)(rand() % 6);
		fog[current_point].v2 = fog[current_point].v1 + 0.0625f;
		fog[current_point].life = 10.0f;
		fog[current_point].stopped = 0;
		fog[current_point].angle = 0.0f;
		fog[current_point].angle_mom = (((rand() % 50) / 50.0f) - 0.5f)*0.1f;

		current_point++;
		//break;
	}
	if (current_point > m_maxParticles - 2)
		current_point = 0;

	/*
	do
	{
	if (point_find>m_maxParticles - 2)
	point_find = 0;

	if (snow[point_find].bActive == 0) //&& z < p_Level->front_back_walls && z > -p_Level->front_back_walls)
	{
	snow[point_find].bActive = 1;
	snow[point_find].vel_y = (((rand() % 100) / 50.0f) + 1.0f)*0.2f;
	snow[point_find].vel_x = 0.0f; //((localrand() % 50) / 150.0f)*0.2f;
	snow[point_find].x = x;
	snow[point_find].y = y;
	snow[point_find].z = z;
	snow[point_find].v1 = 0.125f*(float)(rand() % 6);
	snow[point_find].v2 = snow[point_find].v1 + 0.0625f;
	snow[point_find].life = 10.0f;
	snow[point_find].stopped = 0;
	snow[point_find].angle = 0.0f;
	snow[point_find].angle_mom = (((rand() % 50) / 50.0f) - 0.5f)*0.1f;

	point_find++;
	break;
	}
	else
	{
	point_find++;
	}
	} while (point_find < m_maxParticles - 2 && point_find!=current_point);

	current_point = point_find;
	if (current_point>m_maxParticles - 2)
	current_point = 0;
	*/
}

unsigned Fog::localrand()
{
	rand_state = rand_state * 46781267 + 9788973;  // scramble the number
	return rand_state;
}

void Fog::Reset()
{
	for (int i = 0; i < m_maxParticles; i++)
	{
		fog[i].bActive = 0;
	}
	for (int i = 0; i < 120; i++)
	{
		Update(0.0f, 1.1f);
	}
}

Fog::~Fog(void)
{
}

task<void> Fog::Update(float timeDelta, float timeTotal)
{
	return create_task([this, timeDelta, timeTotal]
	{
		int i;

		rp_fog.clear();

		XMFLOAT3 cam_pos = m_Res->m_Camera->Eye();

		cam_pos.x += m_Res->m_Camera->LookingX()*15.0f;
		cam_pos.z += m_Res->m_Camera->LookingZ()*15.0f;

		float change, x_movement, z_movement;

		x_movement = (timeTotal*m_Res->m_LevelInfo.wind.x)*0.2f;
		z_movement = (timeTotal*m_Res->m_LevelInfo.wind.z)*0.2f;

		change = 0.0f;

		float pn_scale = 0.2f;

		/*
		m_Res->m_uiControl->SetInfoVal(0, m_Res->m_Camera->PositionX());
		m_Res->m_uiControl->SetInfoVal(1, cam_pos.y);
		m_Res->m_uiControl->SetInfoVal(2, cam_pos.z);

		m_Res->m_uiControl->SetInfoVal(3, m_Res->m_Camera->LookingX());
		m_Res->m_uiControl->SetInfoVal(4, m_Res->m_Camera->LookingY());
		m_Res->m_uiControl->SetInfoVal(5, m_Res->m_Camera->LookingZ());
		*/
		unsigned int seed = 237;
		float pnscale = m_Res->m_LevelInfo.wind.w*1.04f;
		float wave_height = m_Res->m_LevelInfo.wind.y*0.5f;
		PerlinNoise pn(seed);

		for (i = 0; i < m_maxParticles; i++)
		{
			if (fog[i].bActive == 1)
			{
				float diff_x = -m_Res->m_LevelInfo.wind.x;// *timeDelta*0.2f;
				float diff_y = -fog[i].vel_y*timeDelta*8.0f;
				float diff_z = -m_Res->m_LevelInfo.wind.z;// *timeDelta*0.2f;

				float speed1 = ((pn.noise((fog[i].x)*pnscale*2.0f, (timeTotal + fog[i].y)*0.3f, (fog[i].z)*pnscale*2.0f)));
				float speed2 = ((pn.noise((fog[i].x)*pnscale*3.0f, (timeTotal + fog[i].y)*0.5, (fog[i].z)*pnscale*3.0f)));
				float speed3 = ((pn.noise((fog[i].x)*pnscale, (timeTotal + fog[i].y)*0.7f, (fog[i].z)*pnscale)));

				float speed = (speed1*(speed2*4.0f)*speed3)*wave_height;

				diff_x = diff_x * speed;
				diff_z = diff_z * speed;

				fog[i].x += (fog[i].vel_x*diff_x)*timeDelta*0.6f;
				fog[i].y += fog[i].vel_y*timeDelta*0.4f;
				fog[i].z += (fog[i].vel_z*diff_z)*timeDelta*0.6f;

				//snow[i].y += -snow[i].vel_y;
				//snow[i].x += snow[i].vel_x;
				//snow[i].z += snow[i].vel_z;

				//if (snow[i].y <= 0.0f)
				//{
				//	snow[i].y += 20.0f;
				//}

				if (fog[i].y - cam_pos.y > WEATHER_AREA / 4)
				{
					fog[i].y -= ((WEATHER_AREA / 2));
					//snow[i].bVisible = true;
				}

				if (fog[i].y - cam_pos.y < -WEATHER_AREA / 4)
				{
					fog[i].y += ((WEATHER_AREA / 2));
					//snow[i].bVisible = true;
				}

				if (fog[i].x - cam_pos.x > WEATHER_AREA / 2)
				{
					fog[i].x -= (WEATHER_AREA);
				}

				if (fog[i].x - cam_pos.x < -WEATHER_AREA / 2)
				{
					fog[i].x += (WEATHER_AREA);
				}

				if (fog[i].z - cam_pos.z > WEATHER_AREA / 2)
				{
					fog[i].z -= (WEATHER_AREA);
				}

				if (fog[i].z - cam_pos.z < -WEATHER_AREA / 2)
				{
					fog[i].z += (WEATHER_AREA);
				}
			}
		}
		//p_Level->

		//float noise_scale = 0.01f;

		//float noises = pn->noise(noise_position, 0.0f, 0.8)*100.0f;
		//int num_noise = (int)pn->noise(noise_position, 0.0f, 0.8)*100.0f;

		//noise_position += 0.1f;
		//if (noise_position > 100.0f)
		//{
		//	noise_position = 0.0f;
		//}

		//std::no

		int total_particles = 0;

		part_index pi;
		for (i = 0; i < m_maxParticles; i++)
		{
			if (fog[i].bActive == 1)
			{
				total_particles++;

				if (fog[i].y > 1.0f)
				{
					//snow[i].full_dist = -m_Res->m_Camera->EyePlaneDepth(snow[i].x, snow[i].y, snow[i].z);
					fog[i].cam_dist = m_Res->m_Camera->CheckPoint(fog[i].x, fog[i].y, fog[i].z, 0.1f);

					if (fog[i].cam_dist > 0.0f)
					{
						float t_height = fog[i].y - p_Level->GetTerrainHeight(fog[i].x, fog[i].z);
						if (t_height > 0.0f)
						{
							rp_fog.push_back(&fog[i]);
						}
					}
				}
			}
		}

		if (total_particles + 1 < MAX_FOG_PARTICLES)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<int> dist(0, 400);

			for (i = 0; i < 100; i++)
			{
				float randx;
				float randz;
				float randy;

				float rx = (dist(gen) - 200)*0.1f;
				float rz = (dist(gen) - 200)*0.1f;
				float ry = (dist(gen) - 200)*0.05f;

				randz = cam_pos.z + rz;// +(m_Res->m_Camera->LookingZ()*15.0f);
				randx = cam_pos.x + rx;// +(m_Res->m_Camera->LookingX()*15.0f);
				randy = cam_pos.y + ry;// +(m_Res->m_Camera->LookingX()*15.0f);
				if (true)//((rand() % (int)noise)>39)
				{
					CreateOne(randx, randy, randz);
				}
			}
		}

		if (rp_fog.size() > 0)
		{
			std::sort(rp_fog.begin(), rp_fog.end(), fog_compare{});
		}
	}).then([this]
	{
		return CreateVerticies();
	});
}

void Fog::Render()
{
	//UpdateVertecies(m_Res->m_deviceResources->GetD3DDeviceContext());

	Sparticle::Render();
}

task<void> Fog::CreateVerticies()
{
	auto this_task = create_task([this]
	{
		int index, i, j;

		XMFLOAT4 col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		float size = 4.5f;
		float snow_edge = 0.8f;

		XMVECTOR vtop_right = XMLoadFloat3(&XMFLOAT3(size, size, 0.0f));
		XMVECTOR vtop_left = XMLoadFloat3(&XMFLOAT3(-size, size, 0.0f));
		XMVECTOR vbottom_right = XMLoadFloat3(&XMFLOAT3(size, -size, 0.0f));
		XMVECTOR vbottom_left = XMLoadFloat3(&XMFLOAT3(-size, -size, 0.0f));

		XMFLOAT3 rtop_right;
		XMFLOAT3 rtop_left;
		XMFLOAT3 rbottom_right;
		XMFLOAT3 rbottom_left;

		XMFLOAT4X4 x_mat;

		XMStoreFloat4x4(&x_mat, m_Res->m_Camera->View());

		XMMATRIX vrot_mat = XMLoadFloat3x3(&XMFLOAT3X3(x_mat._11, x_mat._12, x_mat._13, x_mat._21, x_mat._22, x_mat._23, x_mat._31, x_mat._32, x_mat._33));

		XMStoreFloat3(&rtop_right, XMVector3Transform(vtop_right, vrot_mat));
		XMStoreFloat3(&rtop_left, XMVector3Transform(vtop_left, vrot_mat));
		XMStoreFloat3(&rbottom_right, XMVector3Transform(vbottom_right, vrot_mat));
		XMStoreFloat3(&rbottom_left, XMVector3Transform(vbottom_left, vrot_mat));

		m_vertexCount = 0;

		// topleft
		m_vertices[m_vertexCount].pos = XMFLOAT3(rtop_left.x, rtop_left.y, rtop_left.z);
		m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 0.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;
		// Top right.
		m_vertices[m_vertexCount].pos = XMFLOAT3(rtop_right.x, rtop_right.y, rtop_right.z);
		m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 0.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;
		// Bottom right.
		m_vertices[m_vertexCount].pos = XMFLOAT3(rbottom_right.x, rbottom_right.y, rbottom_right.z);
		m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 1.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;
		// topleft
		m_vertices[m_vertexCount].pos = XMFLOAT3(rtop_left.x, rtop_left.y, rtop_left.z);
		m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 0.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;
		// Bottom right.
		m_vertices[m_vertexCount].pos = XMFLOAT3(rbottom_right.x, rbottom_right.y, rbottom_right.z);
		m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 1.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;
		// Bottom left.
		m_vertices[m_vertexCount].pos = XMFLOAT3(rbottom_left.x, rbottom_left.y, rbottom_left.z);
		m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 1.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;

		m_indexCount = 6;

		m_instances.clear();

		if (true)
			for (fog_t* s : rp_fog)
			{
				if (s->z > p_Level->front_back_walls || s->z < -p_Level->front_back_walls)
				{
				}
				else
				{
					ParticleInstance pinsatnce;

					pinsatnce.color.x = 0.0f;
					pinsatnce.color.y = 0.0f;
					pinsatnce.color.z = 0.0f;
					pinsatnce.color.w = 0.03f;

					pinsatnce.position = XMFLOAT3(s->x, s->y, s->z);

					m_instances.push_back(pinsatnce);
				}
			}
	});

	return this_task;
}