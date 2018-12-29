#include "pch.h"
#include "grass.h"
#include "DefParticle.h"

using namespace Game;

// std::sort with inlining
struct grass_compare {
	// the compiler will automatically inline this
	bool operator()(const grass_t* a, const grass_t* b) {
		return a->cam_dist < b->cam_dist;
	}
};

unsigned long Grass::localrand()
{
	rand_state = rand_state * 46781267 + 9788973;  // scramble the number
	return rand_state;
}

void Grass::Reset()
{
	for (int i = 0; i < m_maxParticles; i++)
	{
		grass[i].bActive = 0;
	}
}

void Grass::Initialize(Level* pp_Level, bool _bInstanced)
{
	p_Level = pp_Level;

	m_maxParticles = MAX_GRASS_PARTICLES;

	grass = new grass_t[m_maxParticles];

	bUpdate = true;

	rp_grass.clear();

	current_particle_que = 0;

	m_particleSize = 1.0f;

	current_point = 0;

	InitializeBuffers(_bInstanced);

	wind_pos = 0.0f;

	pn = new PerlinNoise(237);

	//CreateDDSTextureFromFile((ID3D11Device *)m_deviceResources->GetD3DDevice(), L"grass-blades.cmo", nullptr, &m_Texture, MAXSIZE_T);
	//m_Texture = m_Res->m_Textures->LoadTexture("grasssoft2");
	for (int i = 0; i < m_maxParticles; i++)
	{
		grass[i].bActive = 0;
	}
}

task<void> Grass::LoadTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	/*
	tasks.push_back(loader->LoadTextureAsync("grass1.dds", nullptr, m_Texture[0].GetAddressOf()));
	tasks.push_back(loader->LoadTextureAsync("grass2.dds", nullptr, m_Texture[1].GetAddressOf()));
	tasks.push_back(loader->LoadTextureAsync("grass3.dds", nullptr, m_Texture[2].GetAddressOf()));
	tasks.push_back(loader->LoadTextureAsync("grass4.dds", nullptr, m_Texture[3].GetAddressOf()));
	*/
	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\grass1_small.dds", nullptr, m_Texture[0].GetAddressOf()));
	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\grass2_small.dds", nullptr, m_Texture[1].GetAddressOf()));
	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\grass3_small.dds", nullptr, m_Texture[2].GetAddressOf()));
	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\grass4_small.dds", nullptr, m_Texture[3].GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("noise01.dds", nullptr, m_Texture.GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("alpha1.dds", nullptr, m_Texture.GetAddressOf()));

	return when_all(tasks.begin(), tasks.end());// .then([this]
}

void Grass::CreateOne(int tex_num, float x, float y, float z, float _topx, float _topy, float _topz, float _scale, float _angle, int _flaketype, float _full_dist)
{
	int point_find = 0;

	if (current_point < (m_maxParticles / 2) - 2)
	{
		grass[current_point].tex_num = tex_num;
		grass[current_point].bActive = 1;
		grass[current_point].vel_y = 0.0f;
		grass[current_point].x = x;
		grass[current_point].y = y;
		grass[current_point].z = z;
		grass[current_point].v1 = 0.125f;// *(float)(rand() % 8);
		grass[current_point].v2 = grass[current_point].v1 + 0.125f;
		grass[current_point].life = 100.0f;
		grass[current_point].stopped = 0;
		grass[current_point].scale = _scale;
		grass[current_point].topx = _topx;
		grass[current_point].topy = _topy;
		grass[current_point].topz = _topz;
		grass[current_point].flaketype = _flaketype;
		grass[current_point].angle = _angle;
		grass[current_point].cam_dist = _full_dist;

		current_point++;
	}
}

Grass::~Grass(void)
{
}

task<void> Grass::Update(float timeDelta, float timeTotal)
{
	x_mom = m_Res->m_LevelInfo.wind.x;// (timeTotal*m_Res->m_LevelInfo.wind.x)*0.4f;
	z_mom = m_Res->m_LevelInfo.wind.z; // (timeTotal*m_Res->m_LevelInfo.wind.z)*0.4f;

	wind_pos += 0.1f;
	return create_task([this, timeDelta, timeTotal]
	{
		int i, j, k;

		unsigned int seed = 237;
		float pnscale = m_Res->m_LevelInfo.wind.w*0.4f;
		float wave_height = m_Res->m_LevelInfo.wind.y*2.0f;
		PerlinNoise pn(seed);

		float noise_z = 0.5f;

		float change, x_movement, z_movement;

		x_movement = (timeTotal*x_mom)*1.7f;
		z_movement = (timeTotal*z_mom)*1.7f;

		change = timeTotal * 0.005f;

		current_point = 0;

		for (Terrain* t : p_Level->rm_Terrain)

			//for (k = 0; k < p_Level->current_index; k++)
		{
			if (t->cam_dist < 70.0f && t->cam_dist > 0.0f)
			{
				srand(100);

				for (i = 0; i < 8; i++)
				{
					for (j = 0; j < 8; j++)
					{
						if (t->tex_blend[5][i][j] > 0.0f)
						{
							int numb = int(t->tex_blend[5][i][j] * 12.0f);
							for (int z = 0; z < numb; z++)
							{
								float y_pos = t->height_map[i][j];

								float x_pos = t->pos_x - 8.0f + 1.0f + (float)i*2.0f;
								float z_pos = t->pos_z - 8.0f + 1.0f + (float)j*2.0f;

								x_pos += (rand() % 20)*0.1f;
								z_pos += ((rand() % 20)*0.1f);
								angle = (rand() % 20)*0.1f;
								int flaketype = rand() % 8;
								int tex_num = rand() % 4;
								float full_dist = m_Res->m_Camera->CheckPoint(x_pos, y_pos, z_pos, 2.0f);
								//float full_dist = -m_Res->m_Camera->EyePlaneDepth(x_pos, y_pos, z_pos); //m_Res->m_Camera->CheckPoint(x_pos, y_pos, z_pos, 2.0f);

								if (full_dist > -1.0f)
								{
									if (full_dist < 60.0f)
									{
										float dist_scale = 1.0f;

										if (full_dist > 40.0f)
											dist_scale = (60.0f - full_dist)*0.05f;

										float add_val = (pn.noise(((float)(x_pos)+x_movement)*pnscale, ((float)(z_pos)+z_movement)*pnscale, change));
										float add_val2 = (pn.noise(((float)(z_pos)+z_movement)*pnscale*0.3f, ((float)(x_pos)+x_movement)*pnscale, change));
										float add_val3 = (pn.noise(((float)(x_pos)+x_movement)*pnscale, ((float)(z_pos)+z_movement)*pnscale*3.3f, change));
										float x_pos2 = x_pos + (add_val3*add_val2*add_val*t->tex_blend[5][i][j]);
										float z_pos2 = z_pos + (add_val3*add_val2*add_val*t->tex_blend[5][i][j]);
										y_pos = p_Level->GetTerrainHeight(x_pos, z_pos);
										if (y_pos > 1.0f)
										{
											CreateOne(tex_num, x_pos, y_pos, z_pos, x_pos2, y_pos, z_pos2, t->tex_blend[5][i][j] * dist_scale, angle, flaketype, full_dist);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		rp_grass.clear();
		for (i = 0; i < current_point; i++)
		{
			rp_grass.push_back(&grass[i]);
		}

		if (rp_grass.size() > 0)
		{
			std::sort(rp_grass.begin(), rp_grass.end(), grass_compare{});
		}
	}).then([this]
	{
		return CreateVerticies();
	});
}

void Grass::Render()
{
	//UpdateVertecies(m_Res->m_deviceResources->GetD3DDeviceContext());
	MaterialBufferType mat_buffer;

	mat_buffer.Specular.w = 0.0f;
	m_Res->UpdateMaterialBuffer(&mat_buffer);

	Sparticle::Render();
}

task<void> Grass::CreateVerticies()
{
	auto this_task = create_task([this]
	{
		XMVECTOR dif = XMLoadFloat4(&m_Res->m_LevelInfo.diffuse_col);
		XMVECTOR amb = XMLoadFloat4(&m_Res->m_LevelInfo.ambient_col);

		int index, i, j;

		index = 0;
		int total_active = 0;
		float px;
		float py;
		float pz;

		float lpx;
		float lpy;
		float lpz;

		XMFLOAT4 col;
		float size = 0.03f;
		float sizeB = 0.0f;

		float scale = 0.5f;
		float scaley = 0.8f;

		float xsize;
		float zsize;

		m_vertexCount = 0;
		m_indexCount = 0;

		if (true)
		{
			for (grass_t* g : rp_grass)
				//for (i = 0; i < current_index; i++)
			{
				int tex = rand() % 4;

				xsize = (1.0f*scale);
				zsize = ((-1.0f + g->angle)*scale);

				px = g->x;
				py = g->y;
				pz = g->z;

				lpx = g->topx;
				lpy = py + scaley * g->scale;
				lpz = g->topz;

				col = m_Res->m_Lights->m_lightBufferData.ambientColor;
				col.x += m_Res->m_Lights->m_lightBufferData.diffuseColor.x *1.0f;
				col.y += m_Res->m_Lights->m_lightBufferData.diffuseColor.y *1.0f;
				col.z += m_Res->m_Lights->m_lightBufferData.diffuseColor.z *1.0f;

				col.w = 1.0f;

				if (col.w > 0.0f)
				{
					col.w = 1.0f;

					index = m_vertexCount;

					// Top left.
					m_vertices[m_vertexCount].pos = XMFLOAT3(lpx - (xsize), lpy + sizeB, lpz - (zsize));
					m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 0.0f);
					m_vertices[m_vertexCount].col = col;
					m_vertices[m_vertexCount].tex_num = g->tex_num;
					m_vertexCount++;
					// Bottom right.
					m_vertices[m_vertexCount].pos = XMFLOAT3(px + (xsize), py - sizeB, pz + (zsize));
					m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 1.0f);
					m_vertices[m_vertexCount].col = col;
					m_vertices[m_vertexCount].tex_num = g->tex_num;
					m_vertexCount++;
					// Bottom left.
					m_vertices[m_vertexCount].pos = XMFLOAT3(px - (xsize), py - sizeB, pz - (zsize));
					m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 1.0f);
					m_vertices[m_vertexCount].col = col;
					m_vertices[m_vertexCount].tex_num = g->tex_num;
					m_vertexCount++;

					// Top right.
					m_vertices[m_vertexCount].pos = XMFLOAT3(lpx + (xsize), lpy + sizeB, lpz + (zsize));
					m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 0.0f);
					m_vertices[m_vertexCount].col = col;
					m_vertices[m_vertexCount].tex_num = g->tex_num;
					m_vertexCount++;

					m_indexCount += 6;

					xsize = ((1.0f - g->angle)*scale);
					zsize = (1.0f*scale);

					px = g->x;
					py = g->y;
					pz = g->z;

					lpx = g->topx;
					lpy = py + scaley * g->scale;
					lpz = g->topz;

					index = m_vertexCount;

					// Top left.
					m_vertices[m_vertexCount].pos = XMFLOAT3(lpx - (xsize), lpy + sizeB, lpz - (zsize));
					m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 0.0f);
					m_vertices[m_vertexCount].col = col;
					m_vertices[m_vertexCount].tex_num = g->tex_num;
					m_vertexCount++;
					// Bottom right.
					m_vertices[m_vertexCount].pos = XMFLOAT3(px + (xsize), py - sizeB, pz + (zsize));
					m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 1.0f);
					m_vertices[m_vertexCount].col = col;
					m_vertices[m_vertexCount].tex_num = g->tex_num;
					m_vertexCount++;
					// Bottom left.
					m_vertices[m_vertexCount].pos = XMFLOAT3(px - (xsize), py - sizeB, pz - (zsize));
					m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 1.0f);
					m_vertices[m_vertexCount].col = col;
					m_vertices[m_vertexCount].tex_num = g->tex_num;
					m_vertexCount++;

					// Top right.
					m_vertices[m_vertexCount].pos = XMFLOAT3(lpx + (xsize), lpy + sizeB, lpz + (zsize));
					m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 0.0f);
					m_vertices[m_vertexCount].col = col;
					m_vertices[m_vertexCount].tex_num = g->tex_num;
					m_vertexCount++;

					m_indexCount += 6;
				}
			}
		}
	});

	return this_task;
}