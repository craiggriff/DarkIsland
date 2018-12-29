#include "pch.h"
#include "Fire.h"

#include "DefParticle.h"

using namespace Game;

// std::sort with inlining
struct fire_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist < b.dist;
	}
};

void Fire::Initialize(Level* pp_Level, bool _bInstanced)
{
	p_Level = pp_Level;

	m_maxParticles = MAX_FIRE_PARTICLES;

	fire = new fire_t[m_maxParticles];

	p_index.clear();

	bUpdate = true;

	current_particle_que = 0;

	m_particleSize = 1.0f;

	current_index = 0;

	current_point = 0;

	total_collected = 0;

	noise_position = 0.0f;

	rain_lightness = 1.0f;

	InitializeBuffers(_bInstanced);

	wind_pos = 0.0f;

	m_Res->m_audio.CreateSoundEffect(L"Assets/Sounds/fireloop.mp3", &m_FireLoop, true);

	m_Res->m_audio.SetSoundEffectVolume(&m_FireLoop, 0.0f);
	m_Res->m_audio.SetSoundEffectPitch(&m_FireLoop, 1.0f);

	m_Res->m_audio.PlaySoundEffect(&m_FireLoop);

	pn = new PerlinNoise(237);

	//CreateDDSTextureFromFile((ID3D11Device *)m_deviceResources->GetD3DDevice(), L"grass-blades.cmo", nullptr, &m_Texture, MAXSIZE_T);
	//m_Texture = m_Res->m_Textures->LoadTexture("fire01");
	//m_Texture2 = m_Res->m_Textures->LoadTexture("noise01");
	//m_Texture3 = m_Res->m_Textures->LoadTexture("alpha1");

	num_of_textures = 3;

	for (int i = 0; i < m_maxParticles; i++)
	{
		fire[i].bActive = 0;
	}
}

void Fire::SetFireVolume(float vol)
{
	if (vol > fire_volume)
		fire_volume = vol;

	m_Res->m_audio.SetSoundEffectVolume(&m_FireLoop, fire_volume);
}

void Fire::ResetFireVolume()
{
	fire_volume = 0.0f;
	m_Res->m_audio.SetSoundEffectVolume(&m_FireLoop, fire_volume);
}

task<void> Fire::LoadTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\fire01.dds", nullptr, m_Texture[0].GetAddressOf()));
	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\noise01.dds", nullptr, m_Texture[1].GetAddressOf()));
	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\alpha1.dds", nullptr, m_Texture[2].GetAddressOf()));

	return when_all(tasks.begin(), tasks.end());// .then([this]
}

void Fire::CreateOne(float x, float y, float z, float _topx, float _topy, float _topz, float _scale, XMFLOAT4 _col)
{
	int point_find = 0;

	do
	{
		point_find++;

		if (fire[current_point].bActive == 0)
		{
			fire[current_point].color = _col;
			fire[current_point].bActive = 1;
			fire[current_point].vel_y = 0.0f;
			fire[current_point].x = x;
			fire[current_point].y = y;
			fire[current_point].z = z;
			fire[current_point].v1 = 0.125f;// *(float)(rand() % 8);
			fire[current_point].v2 = fire[current_point].v1 + 0.125f;
			fire[current_point].life = 100.0f;
			fire[current_point].stopped = 0;
			fire[current_point].scale = _scale;
			fire[current_point].topx = _topx;
			fire[current_point].topy = _topy;
			fire[current_point].topz = _topz;
			fire[current_point].time_offset = (float)(rand() % 20)*0.9f;
			break;
		}
		else
		{
			current_point++;
			if (current_point > m_maxParticles - 2)
				current_point = 0;
		}
	} while (point_find < m_maxParticles - 2);
	//snow[current_point].v1 = 0.125f*(float)num;
	//snow[current_point].v2 = snow[current_point].v1 + 0.125f;

	current_point++;
	if (current_point > m_maxParticles - 2)
		current_point = 0;
}

Fire::~Fire(void)
{
}

void Fire::Reset()
{
	for (int k = 0; k < m_maxParticles; k++)
	{
		fire[k].bActive = 0;
	}

	current_point = 0;
}

float Fire::SetFire(float x, float y, float z, float _scale, float timeTotal, float timeDelta, XMFLOAT4 _col)
{
	x_mom = m_Res->m_LevelInfo.wind.x;// (timeTotal*m_Res->m_LevelInfo.wind.x)*0.4f;
	z_mom = m_Res->m_LevelInfo.wind.z; // (timeTotal*m_Res->m_LevelInfo.wind.z)*0.4f;

	unsigned int seed = 237;
	float pnscale = m_Res->m_LevelInfo.wind.w*0.4f;
	float wave_height = m_Res->m_LevelInfo.wind.y*2.0f;
	PerlinNoise pn(seed);

	if (m_Res->m_Camera->CheckPlanes(x, y, z, 2.0f) == true)
	{
		float add_val = (pn.noise(((float)(x)+x_movement)*pnscale, ((float)(z)+z_movement)*pnscale, change));
		float x_pos2 = x - (add_val*_scale);
		float z_pos2 = z - (add_val*_scale);
		//y = p_Level->GetTerrainHeight(x, z);
		if (y > 1.0f)
		{
			CreateOne(x, y, z, x_pos2, y + 3.0f*_scale, z_pos2, _scale, _col);
		}
		return add_val;
	}
	return 0.0f;
}

task<void> Fire::Update(float timeDelta, float timeTotal)
{
	x_mom = m_Res->m_LevelInfo.wind.x;// (timeTotal*m_Res->m_LevelInfo.wind.x)*0.4f;
	z_mom = m_Res->m_LevelInfo.wind.z; // (timeTotal*m_Res->m_LevelInfo.wind.z)*0.4f;

	wind_pos += 0.1f;
	return create_task([this, timeDelta, timeTotal]
	{
		int i, j, k;
		//timeDelta *= 1.1f;

		DistortionBufferType db;
		db.distortion1 = XMFLOAT2(0.1f, 0.2f);
		db.distortion2 = XMFLOAT2(0.1f, 0.3f);
		db.distortion3 = XMFLOAT2(0.1f, 0.1f);
		db.distortionScale = 0.8f;
		db.distortionBias = 0.5f;
		m_Res->UpdateDistortionBuffer(&db);

		NoiseBufferType nb;
		nb.frameTime = timeTotal;
		nb.scrollSpeeds = XMFLOAT3(1.6f, 1.1f, 1.3f);
		nb.scales = XMFLOAT3(0.3f, 0.6f, 0.9f);
		nb.padding = 0.0f;
		m_Res->UpdateNoiseBuffer(&nb);

		unsigned int seed = 237;
		float pnscale = m_Res->m_LevelInfo.wind.w*0.4f;
		float wave_height = m_Res->m_LevelInfo.wind.y*2.0f;
		PerlinNoise pn(seed);

		float noise_z = 0.5f;

		x_movement = (timeTotal*x_mom)*1.7f;
		z_movement = (timeTotal*z_mom)*1.7f;

		change = timeTotal * 0.5f;

		current_index = 0;
		p_index.clear();
		part_index pi;
		for (i = 0; i < m_maxParticles; i++)
		{
			if (fire[i].bActive == 1)
			{
				fire[i].full_dist = m_Res->m_Camera->CheckPoint(fire[i].x, fire[i].y, fire[i].z, 1.0f);

				if (fire[i].full_dist > 0.0f && fire[i].full_dist < m_Res->view_distance)
				{
					pi.dist = fire[i].full_dist;
					pi.part_no = i;

					p_index.push_back(pi);

					current_index++;
				}
			}
		}

		if (current_index > 0)
		{
			std::sort(p_index.begin(), p_index.end(), fire_compare{});
		}
	}).then([this]
	{
		return CreateVerticies();
	});
}

void Fire::Render()
{
	Sparticle::Render();
}

void Fire::SetRainLigtness(float _rain_lightness)
{
	rain_lightness = _rain_lightness;
}

task<void> Fire::CreateVerticies()
{
	auto this_task = create_task([this]
	{
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
		float size = 0.1f;
		float sizeB = 0.0f;

		float xsize;
		float zsize;

		m_vertexCount = 0;
		m_indexCount = 0;

		if (true)
		{
			for (i = 0; i < current_index; i++)
			{
				xsize = m_Res->m_Camera->LookingTanX()*fire[p_index[i].part_no].scale;
				zsize = m_Res->m_Camera->LookingTanZ()*fire[p_index[i].part_no].scale;

				px = fire[p_index[i].part_no].x;
				py = fire[p_index[i].part_no].y;
				pz = fire[p_index[i].part_no].z;

				lpx = fire[p_index[i].part_no].topx;
				lpy = fire[p_index[i].part_no].topy;
				lpz = fire[p_index[i].part_no].topz;

				col.x = 1.0f;
				col.y = 1.0f;
				col.z = 1.0f;

				col.w = 1.0f;

				index = m_vertexCount;

				// Top left.
				m_vertices[m_vertexCount].pos = XMFLOAT3(lpx - (xsize), lpy + sizeB, lpz - (zsize));
				m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 0.0f);
				m_vertices[m_vertexCount].col = fire[p_index[i].part_no].color;
				m_vertexCount++;
				// Bottom right.
				m_vertices[m_vertexCount].pos = XMFLOAT3(px + (xsize), py - sizeB, pz + (zsize));
				m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 1.0f);
				m_vertices[m_vertexCount].col = fire[p_index[i].part_no].color;
				m_vertexCount++;
				// Bottom left.
				m_vertices[m_vertexCount].pos = XMFLOAT3(px - (xsize), py - sizeB, pz - (zsize));
				m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 1.0f);
				m_vertices[m_vertexCount].col = fire[p_index[i].part_no].color;
				m_vertexCount++;
				// Top right.
				m_vertices[m_vertexCount].pos = XMFLOAT3(lpx + (xsize), lpy + sizeB, lpz + (zsize));
				m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 0.0f);
				m_vertices[m_vertexCount].col = fire[p_index[i].part_no].color;
				m_vertexCount++;

				m_indexCount += 6;
			}
		}
	});

	return this_task;
}