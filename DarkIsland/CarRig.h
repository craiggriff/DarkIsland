#pragma once

#include "level.h"
#include "meshmodel.h"

namespace Game
{
	typedef struct
	{
		std::wstring body_fname;
		std::wstring wheel_fname;

		std::wstring body_tex0;
		std::wstring body_tex1;
		std::wstring body_tex2;

		std::wstring body_norm0;
		std::wstring body_norm1;
		std::wstring body_norm2;

		std::wstring body_light0;
		std::wstring body_light1;
		std::wstring body_light2;

		std::wstring body_matname0;
		std::wstring body_matname1;
		std::wstring body_matname2;

		std::wstring wheel_tex0;
		std::wstring wheel_tex1;
		std::wstring wheel_tex2;

		std::wstring wheel_matname0;
		std::wstring wheel_matname1;
		std::wstring wheel_matname2;

		std::wstring wheel_nor0;
		std::wstring wheel_nor1;
		std::wstring wheel_nor2;

		MaterialBufferType body_material[3];
		MaterialBufferType wheel_material[3];

		int body_alpha[3];
		int wheel_alpha[3];

		float car_scale;  // overall scale

		float body_scale;
		float wheel_scale;
		float wheel_rig_scale_to_model;

		float wheel_x_off;
		float wheel_y_off;
		float wheel_z_off;

		float wheel_mass;
		float body_mass;

		float front_friction;
		float back_friction;

		bool enable_antistropic_friction;

		float wheel_angular_damping;
		float wheel_linear_damping;

		float antistropic_friction_x;
		float antistropic_friction_y;
		float antistropic_friction_z;

		float max_speed;
		float torque;

		float max_wheel_turn;

		float bPoint2Point;

		float sus_stiffness;
		float sus_damping;
		float sus_travel;

		float body_floor_distance;
	} car_values;

	class CarRig
	{
	public:
		CarRig(AllResources* p_Resources);
		~CarRig(void);

		Physics* m_phys;
		int points;

		car_values cvalues;

		task<void> LoadModels();
		task<void> LoadTextures();

		void Initialize(Level* pp_Level);
		void MakePhysics();
		void RemoveContraints();

		void Update(float timeDelta, float timeTotal);

		void SetSteerPosition(float _pos);

		void SetWheelActivation(bool bActive);

		void Render(int alpha_mode = 0);

		void RenderDepth(int alpha_mode, int point_plane);

		bool bPlayerActive;

		Level* p_Level;

		float wheel_x_off, wheel_y_off, wheel_z_off;

		float front_wheel_angle;

		float steer_position;

		ObjInfo car_info;

		btRigidBody* m_FloorPlane;

		btTransform wheel_transform[4];
		btTransform body_transform;

		int load_count;

		btRigidBody* carbody;
		btRigidBody* wheel[4];

		MeshModel* m_bodyModel;
		MeshModel* m_wheelModel;

		btVector3 wheel_offset[4];

		btVector3 player_pos;

		//btTransform frontLeftframeInA; // wheel turning and transforms
		//btTransform frontLeftframeInB;

		//btTransform frontRightframeInA;
		//btTransform frontRightframeInB;

		btTransform frameInA[4];
		btTransform frameInB[4];

		void SetMotors(float targetVelocity, float maxMotorImpulse);

		void SetFrontWheelAngle(float _angle);
		void SetRearWheelAngle(float _angle);

		void Breaking();

		bool bShooting;
		bool bShoot;
		bool bAB; // afterburner

		float shooting_delay;
		float shoot_delay_time;

		float carspeed;

		bool bIsHolding;
		bool bReleaseForce;

		void SetUp(bool _bVal) { bSBUp = _bVal; }
		void SetDown(bool _bVal) { bSBDown = _bVal; }
		void SetLeft(bool _bVal) { bSBLeft = _bVal; }
		void SetRight(bool _bVal) { bSBRight = _bVal; }

		bool bSBUp; // screen buttons
		bool bSBDown; // screen buttons
		bool bSBLeft; // screen buttons
		bool bSBRight; // screen buttons

		float release_force;

		float break_value;

		float respot_delay;

		int rockets;
		bool bInCarView;

		btPoint2PointConstraint* hold_constraint;

		// Sets wheel position
		void SetPosition(float x, float y, float z, float yaw, float pitch, float roll);
		void SetCarPosition(float x, float y, float z, float yaw, float pitch, float roll);

		void SetWheelTurn(int turn_to);

		int Wheelposition; // -1,0,1

		ObjInfo local_info;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Texture2;

		int b_show_rockets;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Texture_rocket;

		btVector3 car_point;
		//XMMATRIX point_martix;
		XMFLOAT3X3 point_martix;
		btVector3 x_offset;
		btVector3 y_offset; // light
		btVector3 x_offset1;
		btVector3 x_offset2;
		btVector3 x_frontoffset1;
		btVector3 x_frontoffset2;
		btVector3 down_offset; // floor plane

		float player_x, player_y, player_z;
		btQuaternion rot;
		btMatrix3x3 rot_mat;
		btMatrix3x3 rot_mat_front;
		bool bPhysicsMade;

		float wheel_turn;

		btGeneric6DofSpringConstraint* sus[4];

		btVector3* GetDirection();
		btVector3* GetUp();

		btVector3 dir_position;
		btVector3 dir_last_osition;

		btVector3 last_floor_norm;

		btVector3* GetPosition();

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Texture;

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Res;
	};
}