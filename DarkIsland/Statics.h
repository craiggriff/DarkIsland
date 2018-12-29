#pragma once

#include "fire.h"

#include "AllResources.h"
//#include "mesh.h"
#include "animation.h"

#include "meshmodel.h"

namespace Game
{
	class static_model_t : public MeshModel
	{
	public:
		static_model_t(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Physics* phys) : MeshModel(pm_deviceResources, phys) {}
		std::vector<float> m_time; // for animations

		char group[40];
		char filename[40];
		int type;
		float light_radius;

		ObjInfo info;
		bool bHasAnim;
	};

	typedef struct
	{
	public:
		int bActive;
		float scale;
		float x, y, z;
		float rot_x, rot_y, rot_z;
		float height_from_terrain; // used for terrain changes
		XMFLOAT4X4 model_matrix;
		int model_index;
		float flag_pos;
		float angle;
		bool bHasAnim;
		int type;
		float dist;
		XMFLOAT4 colour;

		XMFLOAT4 spare[50];

		void MakeMatrix()
		{
			XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rot_y, rot_x, rot_z);
			XMMATRIX translationMatrix = XMMatrixTranslation(x, y, z);

			XMMATRIX scaleMatrix = XMMatrixScaling(scale, scale, scale);
			XMMATRIX scaleRotationMatrix = XMMatrixMultiply(scaleMatrix, rotationMatrix);
			XMMATRIX modelMatrix = XMMatrixMultiplyTranspose(scaleRotationMatrix, translationMatrix);

			XMStoreFloat4x4(&model_matrix, modelMatrix);
		}
	} static_t;

	typedef struct
	{
		static_t stuff;

		ObjInfo info;

		std::vector<XMFLOAT3> center; // 8 for example. can be increased
		std::vector<bool> bMeshVisible;

		XMFLOAT3 light_position;
		//XMFLOAT3 light_direction;
		float light_angle; // for lighthouse

		bool bVisible;
		bool bIsPhysics;
		bool bPhysical;

		bool set_physics_active;
		bool set_physics_inactive;
		float cam_dist;

		std::vector<btRigidBody*> m_Body;

		btTransform m_InitialTransform;
	}static_tt;

	typedef struct
	{
	public:
		int bActive;
		float scale;
		float x, y, z;
		float rot_x, rot_y, rot_z;
		float height_from_terrain; // used for terrain changes
		XMFLOAT4X4 model_matrix;
		int model_index;
		float flag_pos;
		float angle;
		bool bHasAnim;
		int type;
		float dist;

		void MakeMatrix()
		{
			XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rot_y, rot_x, rot_z);
			XMMATRIX translationMatrix = XMMatrixTranslation(x, y, z);

			XMMATRIX scaleMatrix = XMMatrixScaling(scale, scale, scale);
			XMMATRIX scaleRotationMatrix = XMMatrixMultiply(scaleMatrix, rotationMatrix);
			XMMATRIX modelMatrix = XMMatrixMultiplyTranspose(scaleRotationMatrix, translationMatrix);

			XMStoreFloat4x4(&model_matrix, modelMatrix);
		}
	} static_t_old;

	class Statics
	{
	public:
		Statics(AllResources* p_Resources, Level* pp_Level);
		~Statics();

		int total_static_models;

		bool bRandomness;

		bool bLoadingComplete;

		void SetRandomness(bool _rand) { bRandomness = _rand; }

		void MakeCenters();
		void MakePhysics();
		void MakeAllPhysical();

		void UpdatePhysics();
		void Clear(float x, float z);
		void CreateOne(int model_index, float x, float y, float z, float rx, float ry, float rz, float hft);

		task<void> LoadModels();
		task<void> LinkTextures();

		task<void> LoadBinary(int level);

		bool SaveBinary(int level);

		void Initialize(Fire* p_Fire) {
			m_Fire = p_Fire;
		};

		std::vector<db_model_info> model_info;
		std::vector<db_texture_info> texture_info;

		static_model_t* m_static_model[150];

		Level* p_Level;

		void Reset();

		bool bMakePhysicsWhenLoaded;

		int current_index;
		int current_index2;

		std::vector<static_tt> m_static;
		std::vector<static_tt*> pm_static;

		std::vector<CG_POINT_LIGHT> m_point_lights;
		std::vector<CG_SPOT_LIGHT> m_spot_lights;

		Fire* m_Fire;

		SkinnedMeshRenderer m_skinnedMeshRenderer; // for animated trees

		task<void> Statics::Update(float timeDelta, float timeTotal);

		void Render(int alpha_mode, bool _bdef = false);
		void Statics::RenderDepth(int alpha_mode, int point_plane = -1);

		void FinalizeCreateDeviceResources();

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Res;
	};
}