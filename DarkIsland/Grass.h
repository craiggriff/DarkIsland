#pragma once
#include <vector>

//#include "GameHelpers.h"
//#include "Constants.h"

#include "../DirectXTK/Inc/DDSTextureLoader.h"

#include "Physics.h"

#include "Level.h"

//#include "Splash.h"

#include "Sparticle.h"

typedef struct
{
	int bActive;
	bool bInView;

	float x, y, z;
	float topx, topy, topz;
	float last_x, last_y, last_z;

	float life;
	float v1, v2; // texture offsey
	float vel_x, vel_y, vel_z;
	float nx, ny, nz;
	float angle, angle_mom;
	float floor_height;

	float scale;

	int tex_num;
	int flaketype;
	int stopped;
	float t_height;

	float cam_dist; // full distance from camera
} grass_t;

namespace Game
{
	class Grass : Sparticle
	{
	public:
		Grass(AllResources* p_Resources) : Sparticle(p_Resources) { };
		~Grass(void);

		std::vector<grass_t* > rp_grass;

		grass_t* grass;

		//Splash* p_Splash;

		void Reset();

		int total_pills;
		int total_collected;

		bool bUpdate;

		float wind_pos;

		float timeTotal;

		float x_mom, z_mom;

		PerlinNoise* pn;

		XMFLOAT3 cam_last_pos;

		int current_point;

		Level* p_Level;

		int current_particle_que;

		unsigned long rand_state;

		unsigned long localrand();

		float angle;
		float m_particleSize;

		task<void> LoadTextures();

		void Initialize(Level* pp_Level, bool _bInstanced = false);

		void UpdateVertexBuffers() { UpdateVertecies(m_Res->m_deviceResources->GetD3DDeviceContext()); };

		void Render();

		task<void> Grass::CreateVerticies();

		task<void> Update(float timeDelta, float timeTotal);

		void CreateOne(int tex_num, float x, float y, float z, float _topx, float _topy, float _topz, float _scale, float _angle, int _flaketype, float _full_dist);
	};
}