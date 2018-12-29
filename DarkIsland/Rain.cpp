#include "pch.h"
#include "Rain.h"
#include "DefParticle.h"

using namespace Game;

// std::sort with inlining
struct rain_compare {
	// the compiler will automatically inline this
	bool operator()(const rain_t* a, const rain_t* b) {
		return a->cam_dist > b->cam_dist;
	}
};

void Rain::Initialize(Level* pp_Level, bool _bInstanced)
{
	p_Level = pp_Level;

	m_maxParticles = MAX_RAIN_PARTICLES;

	rain.resize(m_maxParticles);

	bUpdate = true;

	current_particle_que = 0;

	m_particleSize = 1.0f;

	current_point = 0;

	total_collected = 0;

	rand_state = 0;  // <- state of the RNG (input and output)

	InitializeBuffers(_bInstanced);

	bInstanced = _bInstanced;

	m_indexCount = 0;

	pn = new PerlinNoise(237);

	for (int i = 0; i < m_maxParticles; i++)
	{
		rain[i].bActive = 0;
	}
}

task<void> Rain::LoadTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\drop.dds", nullptr, m_Texture[0].GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("perturb001.dds", nullptr, m_PerturbTexture.GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("noise01.dds", nullptr, m_Texture.GetAddressOf()));
	//tasks.push_back(loader->LoadTextureAsync("alpha1.dds", nullptr, m_Texture.GetAddressOf()));

	return when_all(tasks.begin(), tasks.end());// .then([this]
}

void Rain::CreateOne(float x, float y, float z, float size, float weight)
{
	XMFLOAT4 col = XMFLOAT4(1.0f*size, 1.0f*size, 1.0f*size, 1.0f*size);

	if (rain[current_point].bActive == 0) //&& z < p_Level->front_back_walls && z > -p_Level->front_back_walls)
	{
		//float weight = (((float(rand() % 100) * 0.1f))*0.3f) + 0.3f;
		rain[current_point].bActive = 1;
		rain[current_point].vel_y = -18.8f*(weight + 1.0f);
		rain[current_point].vel_x = weight * 0.03f; // 0.0f; //((localrand() % 50) / 150.0f)*0.2f;
		rain[current_point].vel_z = weight * 0.03f;
		rain[current_point].x = x;
		rain[current_point].y = y;
		rain[current_point].z = z;
		rain[current_point].last_x = x;
		rain[current_point].last_y = y;
		rain[current_point].last_z = z;
		rain[current_point].col = col;
		rain[current_point].v1 = 0.125f*(float)(rand() % 6);
		rain[current_point].v2 = rain[current_point].v1 + 0.0625f;
		rain[current_point].life = 10.0f;
		rain[current_point].stopped = 0;
		rain[current_point].angle = 0.0f;
		rain[current_point].angle_mom = 0.0f;// (((rand() % 50) / 50.0f) - 0.5f)*0.1f;

		current_point++;
		//break;
	}
	if (current_point > m_maxParticles - 2)
		current_point = 0;
}

unsigned Rain::localrand()
{
	rand_state = rand_state * 46781267 + 9788973;  // scramble the number
	return rand_state;
}

void Rain::Reset()
{
	for (int i = 0; i < m_maxParticles; i++)
	{
		rain[i].bActive = 0;
	}
	for (int i = 0; i < 120; i++)
	{
		Update(0.0f, 1.1f);
	}
}

Rain::~Rain(void)
{
}

task<void> Rain::Update(float timeDelta, float timeTotal)
{
	return create_task([this, timeDelta, timeTotal]
	{
		int i;

		rp_rain.clear();

		XMFLOAT3 cam_pos = m_Res->m_Camera->Eye();

		cam_pos.x += m_Res->m_Camera->LookingX()*15.0f;
		cam_pos.z += m_Res->m_Camera->LookingZ()*15.0f;

		XMFLOAT4X4 p_mat;

		XMStoreFloat4x4(&p_mat, m_Res->m_Camera->Projection());

		XMMATRIX proj_mat = XMLoadFloat3x3(&XMFLOAT3X3(p_mat._11, p_mat._12, p_mat._13, p_mat._21, p_mat._22, p_mat._23, p_mat._31, p_mat._32, p_mat._33));

		XMFLOAT4X4 x_mat;

		XMStoreFloat4x4(&x_mat, m_Res->m_Camera->View());

		XMMATRIX vrot_mat = XMLoadFloat3x3(&XMFLOAT3X3(x_mat._11, x_mat._12, x_mat._13, x_mat._21, x_mat._22, x_mat._23, x_mat._31, x_mat._32, x_mat._33));

		matCamLast = matCamCurrent;

		matCamCurrent = vrot_mat;//  m_Res->m_Camera->View();

		vecCamLast = vecCamCurrent;

		vecCamCurrent = m_Res->m_Camera->Eye();

		/*
		m_Res->m_uiControl->SetInfoVal(0, m_Res->m_Camera->PositionX());
		m_Res->m_uiControl->SetInfoVal(1, cam_pos.y);
		m_Res->m_uiControl->SetInfoVal(2, cam_pos.z);

		m_Res->m_uiControl->SetInfoVal(3, m_Res->m_Camera->LookingX());
		m_Res->m_uiControl->SetInfoVal(4, m_Res->m_Camera->LookingY());
		m_Res->m_uiControl->SetInfoVal(5, m_Res->m_Camera->LookingZ());
		*/
		unsigned int seed = 237;
		float pnscale = m_Res->m_LevelInfo.wind.w*0.4f;
		float wave_height = m_Res->m_LevelInfo.wind.y*0.05f;
		PerlinNoise pn(seed);

		for (i = 0; i < m_maxParticles; i++)
		{
			if (rain[i].bActive == 1)
			{
				float diff_x = -m_Res->m_LevelInfo.wind.x*timeDelta*0.2f;
				//float diff_y = -snow[i].vel_y*timeDelta*8.0f;
				float diff_z = -m_Res->m_LevelInfo.wind.z*timeDelta*0.2f;

				float speed1 = ((pn.noise((rain[i].x)*pnscale*2.0f, (timeTotal + rain[i].y)*0.3f, (rain[i].z)*pnscale*2.0f)));
				float speed2 = ((pn.noise((rain[i].x)*pnscale*3.0f, (timeTotal + rain[i].y)*0.5, (rain[i].z)*pnscale*3.0f)));
				float speed3 = ((pn.noise((rain[i].x)*pnscale, (timeTotal + rain[i].y)*0.7f, (rain[i].z)*pnscale)));

				float speed = (speed1*(speed2*4.0f)*speed3)*wave_height;

				diff_x = diff_x * speed1;
				diff_z = diff_z * speed2;

				rain[i].x += (diff_x);// *timeDelta*20.6f;
				rain[i].y += rain[i].vel_y*timeDelta;
				rain[i].z += (diff_z);// *timeDelta*20.6f;

				if (rain[i].y - cam_pos.y > WEATHER_AREA / 4)
				{
					rain[i].y -= ((WEATHER_AREA / 2));
					rain[i].last_y -= ((WEATHER_AREA / 2));
					//snow[i].bVisible = true;
				}

				if (rain[i].y - cam_pos.y < -WEATHER_AREA / 4)
				{
					rain[i].y += ((WEATHER_AREA / 2));
					rain[i].last_y += ((WEATHER_AREA / 2));
					//snow[i].bVisible = true;
				}

				if (rain[i].x - cam_pos.x > WEATHER_AREA / 2)
				{
					rain[i].x -= (WEATHER_AREA);
					rain[i].last_x -= (WEATHER_AREA);
				}

				if (rain[i].x - cam_pos.x < -WEATHER_AREA / 2)
				{
					rain[i].x += (WEATHER_AREA);
					rain[i].last_x += (WEATHER_AREA);
				}

				if (rain[i].z - cam_pos.z > WEATHER_AREA / 2)
				{
					rain[i].z -= (WEATHER_AREA);
					rain[i].last_z -= (WEATHER_AREA);
				}

				if (rain[i].z - cam_pos.z < -WEATHER_AREA / 2)
				{
					rain[i].z += (WEATHER_AREA);
					rain[i].last_z += (WEATHER_AREA);
				}

				rain[i].last_x = rain[i].x;
				rain[i].last_y = rain[i].y;
				rain[i].last_z = rain[i].z;
			}
		}

		int total_particles = 0;

		part_index pi;
		for (i = 0; i < m_maxParticles; i++)
		{
			if (rain[i].bActive == 1 /*&& snow[i].bVisible == 1*/)
			{
				total_particles++;

				if (rain[i].y > 1.0f)
				{
					//snow[i].full_dist = -m_Res->m_Camera->EyePlaneDepth(snow[i].x, snow[i].y, snow[i].z);
					rain[i].cam_dist = m_Res->m_Camera->CheckPoint(rain[i].x, rain[i].y, rain[i].z, 0.1f);

					if (rain[i].cam_dist > 0.0f)
					{
						float t_height = rain[i].y - p_Level->GetRoofHeight(rain[i].x, rain[i].z);
						if (t_height > 0.0f)
						{
							/*
							XMVECTOR vec_pos = XMLoadFloat3(&XMFLOAT3(rain[i].x - vecCamLast.x, rain[i].y - vecCamLast.y, rain[i].z - vecCamLast.z));

							XMMATRIX mat_diff = XMMatrixTranspose(matCamLast * proj_mat) - XMMatrixTranspose(matCamCurrent * proj_mat);

							//XMVECTOR vec_pos_new1 = XMVector3Transform(vec_pos, XMMatrixInverse(nullptr,matCamLast));
							//XMVECTOR vec_pos_new2 = XMVector3Transform(vec_pos, XMMatrixInverse(nullptr, matCamCurrent));

							XMVECTOR vec_diff = XMVector3Transform(vec_pos, XMMatrixTranspose(mat_diff));

							//XMFLOAT3 fpos_new1;
							//XMFLOAT3 fpos_new2;

							XMFLOAT3 fpos_diff;

							//XMStoreFloat3(&fpos_new1, vec_pos_new1);
							//XMStoreFloat3(&fpos_new2, vec_pos_new2);

							XMStoreFloat3(&fpos_diff, vec_diff);

							rain[i].last_x += (-fpos_diff.x)+(vecCamCurrent.x - vecCamLast.x);
							rain[i].last_y += (-fpos_diff.y)+(vecCamCurrent.y - vecCamLast.y);
							rain[i].last_z += (-fpos_diff.z)+(vecCamCurrent.z - vecCamLast.z);
							*/

							//rain[i].last_x = rain[i].x+5.0f;
							//rain[i].last_y = rain[i].y+5.0f;
							//rain[i].last_z = rain[i].z;

							//if(rain[i].last_x != rain[i].x && rain[i].last_z != rain[i].z)
							rp_rain.push_back(&rain[i]);
						}
						else
						{
							//snow[i].bVisible = false;
						}
					}
				}
			}
		}

		if (total_particles + 1 < MAX_RAIN_PARTICLES)
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

		if (rp_rain.size() > 0)
		{
			std::sort(rp_rain.begin(), rp_rain.end(), rain_compare{});

			//qsort(p_index, current_index, sizeof(part_index), SnowSortCB);
		}
	}).then([this]
	{
		return CreateVerticies();
	});

	//CreateVerticies();
}

void Rain::Render()
{
	//UpdateVertecies(m_Res->m_deviceResources->GetD3DDeviceContext());

	Sparticle::Render();
}

task<void> Rain::CreateVerticies()
{
	auto this_task = create_task([this]
	{
		int index, i, j;

		XMFLOAT4 col = XMFLOAT4(1.0, 1.0f, 1.0f, 1.0f);

		if (bInstanced == true)
		{
			float sizex = 0.02f;
			float sizey = 0.2f;
			float snow_edge = 0.2f;

			XMVECTOR vtop_right = XMLoadFloat3(&XMFLOAT3(sizex, sizey, 0.0f));
			XMVECTOR vtop_left = XMLoadFloat3(&XMFLOAT3(-sizex, sizey, 0.0f));
			XMVECTOR vbottom_right = XMLoadFloat3(&XMFLOAT3(sizex, -sizey, 0.0f));
			XMVECTOR vbottom_left = XMLoadFloat3(&XMFLOAT3(-sizex, -sizey, 0.0f));

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

			for (rain_t* r : rp_rain)
			{
				if (r->z > p_Level->front_back_walls || r->z < -p_Level->front_back_walls)
				{
				}
				else
				{
					ParticleInstance pinsatnce;

					pinsatnce.color.x = 0.2f;
					pinsatnce.color.y = 0.2f;
					pinsatnce.color.z = 0.2f;
					pinsatnce.color.w = 0.8f;// ((pinsatnce.color.x + pinsatnce.color.y + pinsatnce.color.z)*0.6f);

					pinsatnce.position = XMFLOAT3(r->x, r->y, r->z);
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
		}
		else
		{
			float px;
			float py;
			float pz;

			float lpx;
			float lpy;
			float lpz;

			float size = 0.1f;
			float sizeB = 0.2f;

			float xsize = m_Res->m_Camera->LookingTanX()*0.01f;
			float zsize = m_Res->m_Camera->LookingTanZ()*0.01f;

			m_vertexCount = 0;
			m_indexCount = 0;

			for (rain_t* r : rp_rain)
			{
				px = r->x;
				py = r->y;
				pz = r->z;

				lpx = r->last_x;
				lpy = r->last_y;
				lpz = r->last_z;

				// Top left.
				m_vertices[m_vertexCount].pos = XMFLOAT3(lpx - (xsize), lpy + sizeB, lpz - (zsize));
				m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 0.0f);
				m_vertices[m_vertexCount].col = r->col;
				m_vertexCount++;
				// Bottom right.
				m_vertices[m_vertexCount].pos = XMFLOAT3(px + (xsize), py - sizeB, pz + (zsize));
				m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 1.0f);
				m_vertices[m_vertexCount].col = r->col;
				m_vertexCount++;
				// Bottom left.
				m_vertices[m_vertexCount].pos = XMFLOAT3(px - (xsize), py - sizeB, pz - (zsize));
				m_vertices[m_vertexCount].tex = XMFLOAT2(0.0f, 1.0f);
				m_vertices[m_vertexCount].col = r->col;
				m_vertexCount++;

				// Top right.
				m_vertices[m_vertexCount].pos = XMFLOAT3(lpx + (xsize), lpy + sizeB, lpz + (zsize));
				m_vertices[m_vertexCount].tex = XMFLOAT2(1.0f, 0.0f);
				m_vertices[m_vertexCount].col = r->col;
				m_vertexCount++;

				/*
				m_indices[m_indexCount++] = index;
				m_indices[m_indexCount++] = index + 1;
				m_indices[m_indexCount++] = index + 2;
				m_indices[m_indexCount++] = index;
				m_indices[m_indexCount++] = index + 3;
				m_indices[m_indexCount++] = index + 1;
				*/
				m_indexCount += 6;
			}
		}
	});

	return this_task;
}