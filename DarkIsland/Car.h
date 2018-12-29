#pragma once

#include <vector>
//#include "btBulletDynamicsCommon.h"
//#include "btBulletCollisionCommon.h"
#include "../Bullet/src/btBulletDynamicsCommon.h"

#include "AllResources.h"
#include "meshmodel.h"
//class GUIHelperInterface;
//class CarSimulation;

#include "FilterableVehicleRaycaster.h"

namespace Game
{

	class Car
	{
	public:
		Car(AllResources* p_Resources);
		virtual ~Car();
		void Remove();

		Car(const Car &) = delete;
		Car & operator = (const Car &) = delete;

		task<void> LoadModels();
		task<void> LoadTextures();

		virtual void MakePhysics();
		void KeyboardUpdate(float timeDelta, float timeTotal);

		void Update(float timeDelta, float timeTotal);
		void Render(int alpha_mode);
		//void updateWheelTransformForRender(CarSimulation * simulation);
		std::vector<btTransform> getCarTransform() const;

		void setFrontWheelXOffset(float x_offset);
		void setFrontWheelZOffset(float z_offset);
		void setBackWheelXOffset(float x_offset);
		void setBackWheelZOffset(float z_offset);

		void setCarChassisTransform(const btTransform & chassis_transform);
		void setCarChassisBoxShape(const btVector3 & half_extents);
		void setCarChassisCustomShape(btCollisionShape * shape);

		void SetCarPosition(float x, float y, float z, float yaw, float pitch, float roll);
	protected:
		void setCarChassis();
		//void setCarWheelForGraphics(GUIHelperInterface * m_guiHelper);
		void setCarWheelForSimulation();

	protected:
		btRigidBody * car_chassis = nullptr;
		btRigidBody * wheel[4] = { nullptr,nullptr,nullptr,nullptr };

		btCollisionShape * car_chassis_shape = nullptr;
		bool is_default_chassis_shape = true;

		MeshModel* m_bodyModel;
		MeshModel* m_wheelModel;

		btTransform wheel_transform[4];
		btTransform body_transform;

		btTransform car_chassis_transform; //default: <0, 1, 0>

		btCollisionShape * wheel_shape = nullptr;

		int wheel_instances[4] = { -1, -1, -1, -1 };

		float front_wheel_x_offset = 0.f; //default: x_half_width_of_chassis - (0.3*wheel_width)
		float front_wheel_z_offset = 0.f; //default: z_half_width_of_chassis - wheel_radius
		float back_wheel_x_offset = 0.f;  //default: x_half_width_of_chassis - (0.3*wheel_width)
		float back_wheel_z_offset = 0.f;  //default: z_half_width_of_chassis - wheel_radius

		btRaycastVehicle::btVehicleTuning vehicle_tuning;
		FilterableVehicleRaycaster * vehicle_ray_caster = nullptr;
		btRaycastVehicle * vehicle = nullptr;

		float car_mass = 200.f;

		float max_motor_impulse = 800.f;
		float engine_force = 0.f;
		float max_engine_force = 400.f;

		float default_breaking_force = 10.f;
		float breaking_force = 100.f;
		float max_breaking_force = 100.f;

		float vehicle_steering = 0.f;
		float steering_increment = 0.8f;
		float steering_clamp = 0.4f;

		float wheel_radius = 0.8f;
		float wheel_width = 0.7f;
		float wheel_height = 0.5f;
		float wheel_friction = 10;

		float suspension_stiffness = 20.f;
		float suspension_damping = 2.3f;
		float suspension_compression = 4.4f;
		float suspension_rest_length = 0.6f;

		float roll_influence = 0.1f;
	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Res;
	};
}