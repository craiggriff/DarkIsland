#pragma once

#include "AllResources.h"
#include "Level.h"
#include "Statics.h"
#include "Stuff.h"
#include "MoveLookController.h"

#include "CameraPath.h"

namespace Game
{
	typedef struct
	{
	public:

		char group[40];
		int type;
		int index;
		int group_index;
		int group_index_start;
	} item_pointer;

	class LevelEdit
	{
	public:
		LevelEdit(AllResources* p_Resources, MoveLookController^ p_controller, Level* pp_Level, Statics* pp_Statics, Stuff* pp_Stuff);
		~LevelEdit();

		void Update(float timeDelta, float timeTotal);
		void Render();
		void UpdateCurrentItemPointer(int pointer_delta);

		void LeftMouse();
		void MiddleMouse();
		void RightMouse();

		void SaveLevel();
		void SetModelNumbers();

		void SetLevel(int level) { game_level = level; }
		void SetStaticsHeightToTerrrain();
		void UpdatePhysics();
		void ClearPaths();

		void MakePath();

		task<void> LoadModels();
		task<void> LoadTextures();

		float camera_height;

		CameraPath* GetCameraPath() { return m_CameraPath; };

		int model_pointer_id;
		int max_model_pointer_id;
	private:
		bool bColKeyDown;

		bool bClearPaths;

		bool bMakePath;
		bool bLoopPath;
		bool bDipPath;

		bool bCamLookPosition;
		bool bSnapGridEdit;

		int total_item_groups;
		int current_item_group;

		MoveLookController^                         m_controller;

		CameraPath* m_CameraPath;

		std::vector<item_pointer> ptr_statics;

		float stretch_width;
		float fixed_height;

		int design_spacing;

		int game_level;

		int yaw_angle_snap;
		float yaw_angle;

		int area_to_clear;

		CG_POINT_LIGHT m_point_light;

		bool bLeftClicked;
		bool bRightClicked;
		bool bMiddleClicked;

		int ground_texture_index;

		float t_height;
		bool bTerrainHit = false;
		XMFLOAT3 hit_point;
		XMFLOAT3 last_hit_point;

		MeshModel* m_Marker;
		MeshModel* m_MarkerPath;

		AllResources* m_Res;
		Level* p_Level;
		Statics* p_Statics;
		Stuff* p_Stuff;
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		DirectX::XMFLOAT3 pos; // cursor pos
		DirectX::XMFLOAT3 last_pos; // cursor pos
	};
};
