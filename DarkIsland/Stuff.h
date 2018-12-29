#pragma once

#include "AllResources.h"

#include "animation.h"

#include "meshmodel.h"

namespace Game
{
	class stuff_model_t : public MeshModel
	{
	public:
		stuff_model_t(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Physics* phys) : MeshModel(pm_deviceResources, phys) {}

		std::vector<float> m_time; // for animations
		int collision_range;
		int type;
		float light_radius;

		char group[40];
		char filename[40];
		SoundEffectData             m_soundEffects[30];
		bool bHasSounds;

		ObjInfo info;
		bool bHasAnim;
	};

	typedef struct
	{
	public:
		int bActive;
		float scale;
		float height_from_terrain; // used for terrain changes
		XMFLOAT4X4 model_matrix;
		int model_index;
		ObjInfo info;
		int _padd;
		XMFLOAT4 colour;
		XMFLOAT4 spare[50];
	} stuff_t;

	typedef struct
	{
		stuff_t stuff;

		bool bPhysical;
		bool bVisible;
		bool bIsPhysics;
		bool set_physics_active;
		bool set_physics_inactive;
		float cam_dist;
		btRigidBody* m_Body;
		btTransform m_InitialTransform;
		btTransform m_CurrentTransform;
	}stuff_tt;

	class Stuff
	{
	public:
		Stuff(AllResources* p_Resources);
		~Stuff();

		int current_selected_index;
		int total_stuff_models;

		task<void> LoadModels();

		task<void> LinkTextures();

		task<void> LoadBinary(int level);
		bool SaveBinary(int level);
		void MakePhysics();

		int IsStuffModel(btRigidBody* rb);
		void PlaySounds();

		void Reset();

		stuff_model_t* m_stuff_model[20];

		std::vector<db_model_info> model_info;
		std::vector<db_texture_info> texture_info;

		std::vector<stuffplaysound_t> play_sounds;

		std::vector<stuff_tt> m_stuff;
		std::vector<stuff_tt*> rp_stuff; // stuff pointers

		bool bLoadingComplete;

		int cur_phy;

		int max_num;
		int cur_total;

		float found_y;
		int index_found;

		float stack_pointer_y;

		task<void> Update();
		void UpdatePhysics(bool make_active = true);
		void ResetAllPositions();
		void Clear(float x, float z);
		void CreateOne(int model_index, float x, float y, float z, float rx, float ry, float rz, float hft);

		void Render(int alpha_mode);
		void RenderDepth(int alpha_mode, int point_plane = -1);
	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Res;
	};
}