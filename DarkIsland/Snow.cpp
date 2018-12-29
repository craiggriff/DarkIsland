#include "pch.h"
#include "Snow.h"
#include "DefParticle.h"

using namespace Game;

// std::sort with inlining
struct snow_compare {
	// the compiler will automatically inline this
	bool operator()(const snow_t* a, const snow_t* b) {
		return a->cam_dist > b->cam_dist;
	}
};

void Snow::Initialize(Level* pp_Level, bool _bInstanced)
{
	p_Level = pp_Level;

	m_maxParticles = MAX_SNOW_PARTICLES;

	snow.resize(m_maxParticles);

	bUpdate = true;

	current_particle_que = 0;

	m_particleSize = 1.0f;

	current_point = 0;

	total_collected = 0;

	rand_state = 0;  // <- state of the RNG (input and output)

	InitializeBuffers(_bInstanced);

	m_indexCount = 0;

	pn = new PerlinNoise(237);

	for (int i = 0; i < m_maxParticles; i++)
	{
		snow[i].bActive = 0;
	}
}

task<void> Snow::LoadTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\snow1.dds", nullptr, m_Texture[0].GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("perturb001.dds", nullptr, m_PerturbTexture.GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("noise01.dds", nullptr, m_Texture.GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("alpha1.dds", nullptr, m_Texture.GetAddressOf()));

	return when_all(tasks.begin(), tasks.end());// .then([this]
}

void Snow::CreateOne(float x, float y, float z, float size, float weight)
{
	XMFLOAT4 col = XMFLOAT4(1.0f*size, 1.0f*size, 1.0f*size, 1.0f*size);

	if (snow[current_point].bActive == 0) //&& z < p_Level->front_back_walls && z > -p_Level->front_back_walls)
	{
		//float weight = (((float(rand() % 100) * 0.1f))*0.3f) + 0.3f;
		snow[current_point].bActive = 1;
		snow[current_point].vel_y = -2.8f*(weight + 1.0f);
		snow[current_point].vel_x = weight * 0.6f; // 0.0f; //((localrand() % 50) / 150.0f)*0.2f;
		snow[current_point].vel_z = weight * 0.6f;
		snow[current_point].x = x;
		snow[current_point].y = y;
		snow[current_point].z = z;
		snow[current_point].v1 = 0.125f*(float)(rand() % 6);
		snow[current_point].v2 = snow[current_point].v1 + 0.0625f;
		snow[current_point].life = 10.0f;
		snow[current_point].stopped = 0;
		snow[current_point].angle = 0.0f;
		snow[current_point].angle_mom = 0.0f;// (((rand() % 50) / 50.0f) - 0.5f)*0.1f;

		current_point++;
		//break;
	}
	if (current_point > m_maxParticles - 2)
		current_point = 0;
}

unsigned Snow::localrand()
{
	rand_state = rand_state * 46781267 + 9788973;  // scramble the number
	return rand_state;
}

void Snow::Reset()
{
	for (int i = 0; i < m_maxParticles; i++)
	{
		snow[i].bActive = 0;
	}
	for (int i = 0; i < 120; i++)
	{
		Update(0.0f, 1.1f);
	}
}

Snow::~Snow(void)
{
}

task<void> Snow::Update(float timeDelta, float timeTotal)
{
	return create_task([this, timeDelta, timeTotal]
	{
		int i;

		rp_snow.clear();

		XMFLOAT3 cam_pos = m_Res->m_Camera->Eye();

		cam_pos.x += m_Res->m_Camera->LookingX()*15.0f;
		cam_pos.z += m_Res->m_Camera->LookingZ()*15.0f;

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
			if (snow[i].bActive == 1)
			{
				float diff_x = -m_Res->m_LevelInfo.wind.x;// *timeDelta*0.2f;
				float diff_y = -snow[i].vel_y*timeDelta*8.0f;
				float diff_z = -m_Res->m_LevelInfo.wind.z;// *timeDelta*0.2f;

				float speed1 = ((pn.noise((snow[i].x)*pnscale*2.0f, (timeTotal + snow[i].y)*0.3f, (snow[i].z)*pnscale*2.0f)));
				float speed2 = ((pn.noise((snow[i].x)*pnscale*3.0f, (timeTotal + snow[i].y)*0.5, (snow[i].z)*pnscale*3.0f)));
				float speed3 = ((pn.noise((snow[i].x)*pnscale, (timeTotal + snow[i].y)*0.7f, (snow[i].z)*pnscale)));

				float speed = (speed1*(speed2*4.0f)*speed3)*wave_height;

				diff_x = diff_x * speed;
				diff_z = diff_z * speed;

				snow[i].x += (snow[i].vel_x*diff_x)*timeDelta*0.6f;
				snow[i].y += snow[i].vel_y*timeDelta*0.4f;
				snow[i].z += (snow[i].vel_z*diff_z)*timeDelta*0.6f;

				//snow[i].y += -snow[i].vel_y;
				//snow[i].x += snow[i].vel_x;
				//snow[i].z += snow[i].vel_z;

				//if (snow[i].y <= 0.0f)
				//{
				//	snow[i].y += 20.0f;
				//}

				if (snow[i].y - cam_pos.y > WEATHER_AREA / 4)
				{
					snow[i].y -= ((WEATHER_AREA / 2));
					//snow[i].bVisible = true;
				}

				if (snow[i].y - cam_pos.y < -WEATHER_AREA / 4)
				{
					snow[i].y += ((WEATHER_AREA / 2));
					//snow[i].bVisible = true;
				}

				if (snow[i].x - cam_pos.x > WEATHER_AREA / 2)
				{
					snow[i].x -= (WEATHER_AREA);
				}

				if (snow[i].x - cam_pos.x < -WEATHER_AREA / 2)
				{
					snow[i].x += (WEATHER_AREA);
				}

				if (snow[i].z - cam_pos.z > WEATHER_AREA / 2)
				{
					snow[i].z -= (WEATHER_AREA);
				}

				if (snow[i].z - cam_pos.z < -WEATHER_AREA / 2)
				{
					snow[i].z += (WEATHER_AREA);
				}
			}
		}

		int total_particles = 0;

		part_index pi;
		for (i = 0; i < m_maxParticles; i++)
		{
			if (snow[i].bActive == 1 /*&& snow[i].bVisible == 1*/)
			{
				total_particles++;

				if (snow[i].y > 1.0f)
				{
					//snow[i].full_dist = -m_Res->m_Camera->EyePlaneDepth(snow[i].x, snow[i].y, snow[i].z);
					snow[i].cam_dist = m_Res->m_Camera->CheckPoint(snow[i].x, snow[i].y, snow[i].z, 0.1f);

					if (snow[i].cam_dist > 0.0f)
					{
						float t_height = snow[i].y - p_Level->GetRoofHeight(snow[i].x, snow[i].z);
						if (t_height > 0.0f)
						{
							rp_snow.push_back(&snow[i]);
						}
						else
						{
							//snow[i].bVisible = false;
						}
					}
				}
			}
		}

		if (total_particles + 1 < MAX_SNOW_PARTICLES)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<int> dist(0, 400);
			std::uniform_int_distribution<int> col_dist(0, 1000);
			std::uniform_int_distribution<int> weight(0, 1000);
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
					CreateOne(randx, randy, randz, col_dist(gen)*0.001f, weight(gen)*0.001f);
				}
			}
		}

		if (rp_snow.size() > 0)
		{
			std::sort(rp_snow.begin(), rp_snow.end(), snow_compare{});

			//qsort(p_index, current_index, sizeof(part_index), SnowSortCB);
		}
	}).then([this]
	{
		return CreateVerticies();
	});

	//CreateVerticies();
}

void Snow::Render()
{
	//UpdateVertecies(m_Res->m_deviceResources->GetD3DDeviceContext());

	Sparticle::Render();
}

task<void> Snow::CreateVerticies()
{
	auto this_task = create_task([this]
	{
		int index, i, j;

		XMFLOAT4 col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		float size = 0.01f;
		float snow_edge = 0.1f;

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

			for (snow_t* s : rp_snow)
			{
				if (s->z > p_Level->front_back_walls || s->z < -p_Level->front_back_walls)
				{
				}
				else
				{
					ParticleInstance pinsatnce;

					pinsatnce.color.x = 0.2f;
					pinsatnce.color.y = 0.2f;
					pinsatnce.color.z = 0.2f;
					pinsatnce.color.w = 0.8f;// ((pinsatnce.color.x + pinsatnce.color.y + pinsatnce.color.z)*0.6f);

					pinsatnce.position = XMFLOAT3(s->x, s->y, s->z);
					/*
					GetLightAtPosition(pinsatnce.position, pinsatnce.color);

					pinsatnce.color.x = 0.2f + pinsatnce.color.x;
					pinsatnce.color.y = 0.2f + pinsatnce.color.y;
					pinsatnce.color.z = 0.2f + pinsatnce.color.z;

					pinsatnce.color.w = ((pinsatnce.color.x + pinsatnce.color.y + pinsatnce.color.z)*0.6f);
					*/
					m_instances.push_back(pinsatnce);
				}
			}
	});

	return this_task;
}