#pragma once
#include "Mesh.h"
#include "animation.h"
#include "level.h"
#include "MeshModel.h"
#include "btMyMotionState.h"

namespace Game
{
	typedef struct
	{
		std::wstring material_name;
		std::wstring texture_fname;
		std::wstring texture_type; // t n e
	} char_tex;

	typedef struct
	{
		std::wstring model_fname;

		std::vector<char_tex> textures;

		std::wstring model_idle;
		std::wstring model_run;
		std::wstring model_jump;
		std::wstring model_attack;

		float char_scale;
		float mesh_scale;

		float off_x;
		float off_y;
		float off_z;

		float rot_x;
		float rot_y;
		float rot_z;

		MaterialBufferType model_material;

		float max_walk_speed;
	} char_values;

	class AnimChar
	{
	public:
		AnimChar::AnimChar(AllResources* p_Resources, Level* pp_Level);
		Level* p_Level;

		char_values cvalues;

		bool bJump;
		bool bAttack;

		float attack_timer;
		float attack_anim;

		float x_motion_to;

		bool bPlayerActive;

		bool bKeyDown;

		float char_angle;
		float char_angle_to;

		float walk_speed;

		float x_motion;
		float z_motion;
		float y_motion;

		float jump_anim;

		float model_pos_y;

		float view_pos_y;

		float touch_move_x, touch_move_y;

		void SetTouchMove(float x, float y) { touch_move_x = x; touch_move_y = y; }

		btRigidBody*  MakePhysicsCylinderExtents(ObjInfo* ob_info, float extent1, float extent2, float extent3, float _scale);

		task<void> LoadModels();
		task<void> LoadTextures();

		bool bWorldContact;

		bool touch_jump;
		void DoTouchJump() { touch_jump = true; };

		float touch_rotate;
		void SetTouchRotate(float _val) { touch_rotate = -_val; }

		bool bTouchAttack;
		void SetTouchAttack(bool _bAttack) {
			bTouchAttack = _bAttack;
		};

		MeshModel* m_meshModels;
		SkinnedMeshRenderer m_skinnedMeshRenderer;

		std::vector<float> m_time;

		float view_angle;
		float view_angle_to;
		float view_angle_temp;
		// character based stuff

		//std::vector<Mesh*> m_cube;

		btRigidBody* m_bert_motion;

		btMyMotionState* m_motion;

		XMFLOAT4X4 model_matrix;

		XMFLOAT3 bert_pos;
		XMFLOAT3 bert_pos_last;
		XMFLOAT3 bert_mom;

		btHingeConstraint* bert_hinge;
		btGeneric6DofSpringConstraint* bert_dof;

		void UpdatePhysics(float timeDelta, float timeTotal);
		void Render(int alpha_mode);
		void RenderDepth(int alpha_mode, int point_plane = -1);
		btVector3* GetPosition();
		void SetPosition(float x, float y, float z, float yaw = 0.0f, float pitch = 0.0f, float roll = 0.0f);

		void FinalizeCreateDeviceResources();

		void MakePhysics();

	private:
		AllResources * m_Res;
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
	};
};
