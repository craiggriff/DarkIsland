#include "pch.h"

#include <iostream>
#include "Car.h"

using namespace Game;

//#include "CarSimulation.h"

//===============================================================================================

static int rightIndex = 0;
static int upIndex = 1;
static int forwardIndex = 2;

static btVector3 wheelDirectionCS0(0, -1, 0);
static btVector3 wheelAxleCS(-1, 0, 0);

//===============================================================================================

Car::Car(AllResources* p_Resources)
{
	m_Res = p_Resources;
	m_deviceResources = m_Res->m_deviceResources;

	m_bodyModel = new MeshModel(m_deviceResources, &p_Resources->m_Physics);
	m_wheelModel = new MeshModel(m_deviceResources, &p_Resources->m_Physics);
}

Car::~Car()
{
	Remove();
}

void Car::Remove()
{
	delete car_chassis;

	if (is_default_chassis_shape)
		delete car_chassis_shape;

	delete wheel_shape;
	delete vehicle_ray_caster;
	delete vehicle;
}

void Car::SetCarPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
	car_chassis->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll), btVector3(x, y, z)));
	car_chassis->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
}


void Car::setCarChassis()
{
	this->car_chassis_transform.setIdentity();
	this->car_chassis_transform.setOrigin(btVector3(0, 0.1f, 0));

	btCompoundShape* compound = new btCompoundShape();

	if (car_chassis_shape == nullptr)
		car_chassis_shape = new btBoxShape(btVector3(1.f, 0.75f, 2.f));

	compound->addChildShape(car_chassis_transform, car_chassis_shape);

	//car_chassis = simulation->localCreateRigidBody(car_mass, tr, compound);

	btVector3 localInertia(0, 0, 0);

	compound->calculateLocalInertia(car_mass, localInertia);

	btTransform tr;
	tr.setIdentity();
	tr.setOrigin(btVector3(0.0f, 0.0f, 0.0f));

#define USE_MOTIONSTATE 1
#ifdef  USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(tr);
	btRigidBody::btRigidBodyConstructionInfo cInfo(car_mass, myMotionState, compound, localInertia);
	btRigidBody* body = new btRigidBody(cInfo);
#else
	btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);
#endif

	m_Res->m_Physics.m_dynamicsWorld.get()->addRigidBody(body, COL_OBJECTS | COL_WALLS | COL_CHAR | COL_TERRAIN, COL_CARBODY | COL_RAY);
	car_chassis = body;

	car_chassis->setDamping(0.2, 0.2);
	car_chassis->setActivationState(DISABLE_DEACTIVATION); //never deactivate the vehicle

	// make solid wheels becuase car wheels are just raycast
	//wheel_shape = new btCylinderShapeX(btVector3(wheel_width, wheel_radius, wheel_radius));

	//cInfo.m_collisionShape = wheel_shape;

	//wheel[0] = new btRigidBody(cInfo);
	//wheel[1] = new btRigidBody(cInfo);
	//wheel[2] = new btRigidBody(cInfo);
	//wheel[3] = new btRigidBody(cInfo);

	//m_Res->m_Physics.m_dynamicsWorld.get()->addRigidBody(wheel[0], COL_OBJECTS | COL_WALLS | COL_CHAR , COL_WHEEL );
	//m_Res->m_Physics.m_dynamicsWorld.get()->addRigidBody(wheel[1], COL_OBJECTS | COL_WALLS | COL_CHAR , COL_WHEEL);
	//m_Res->m_Physics.m_dynamicsWorld.get()->addRigidBody(wheel[2], COL_OBJECTS | COL_WALLS | COL_CHAR , COL_WHEEL );
	//m_Res->m_Physics.m_dynamicsWorld.get()->addRigidBody(wheel[3], COL_OBJECTS | COL_WALLS | COL_CHAR , COL_WHEEL );

}

/*
void Car::setCarWheelForGraphics(GUIHelperInterface * m_guiHelper)
{
	//定义Car轮胎形状
	wheel_shape = new btCylinderShapeX(btVector3(wheel_width, wheel_radius, wheel_radius));
	m_guiHelper->createCollisionShapeGraphicsObject(wheel_shape);

	const float position[4] = { 0, 0, 0, 0 };
	const float quaternion[4] = { 0, 0, 0, 1 };
	const float color[4] = { 0, 1, 0, 1 };
	const float scaling[4] = { 1, 1, 1, 1 };

	int wheelGraphicsIndex = wheel_shape->getUserIndex();
	for (int i = 0; i < 4; i++)
		wheel_instances[i] = m_guiHelper->registerGraphicsInstance(wheelGraphicsIndex, position, quaternion, color, scaling);
}
*/

void Car::setCarWheelForSimulation()
{
	btVector3 aabb_min, aabb_max;
	car_chassis->getAabb(aabb_min, aabb_max);

	//car_chassis->setFlags

	float x_half_width = (aabb_max[0] - aabb_min[0]) / 2;
	float z_half_width = (aabb_max[2] - aabb_min[2]) / 2;

	bool isFrontWheel = true;

	//front right wheel
	btVector3 front_right_wheel_pos(front_wheel_x_offset, wheel_height, front_wheel_z_offset);
	if (front_right_wheel_pos[0] == 0.f) front_right_wheel_pos[0] = x_half_width - 0.3*wheel_width;
	if (front_right_wheel_pos[2] == 0.f) front_right_wheel_pos[2] = z_half_width - wheel_radius;

	vehicle->addWheel(front_right_wheel_pos, wheelDirectionCS0, wheelAxleCS, suspension_rest_length, wheel_radius, vehicle_tuning, isFrontWheel);

	//front left wheel
	btVector3 front_left_wheel_pos(-front_wheel_x_offset, wheel_height, front_wheel_z_offset);
	if (front_left_wheel_pos[0] == 0.f) front_left_wheel_pos[0] = -(x_half_width - 0.3*wheel_width);
	if (front_left_wheel_pos[2] == 0.f) front_left_wheel_pos[2] = z_half_width - wheel_radius;
	vehicle->addWheel(front_left_wheel_pos, wheelDirectionCS0, wheelAxleCS, suspension_rest_length, wheel_radius, vehicle_tuning, isFrontWheel);

	isFrontWheel = false;

	//back right wheel
	btVector3 back_right_wheel_pos(back_wheel_x_offset, wheel_height, -back_wheel_z_offset);
	if (back_right_wheel_pos[0] == 0.f) back_right_wheel_pos[0] = x_half_width - 0.3*wheel_width;
	if (back_right_wheel_pos[2] == 0.f) back_right_wheel_pos[2] = -(z_half_width - wheel_radius);
	vehicle->addWheel(back_right_wheel_pos, wheelDirectionCS0, wheelAxleCS, suspension_rest_length, wheel_radius, vehicle_tuning, isFrontWheel);

	//back left wheel
	btVector3 back_left_wheel_pos(-back_wheel_x_offset, wheel_height, -back_wheel_z_offset);
	if (back_left_wheel_pos[0] == 0.f) back_left_wheel_pos[0] = -(x_half_width - 0.3*wheel_width);
	if (back_left_wheel_pos[2] == 0.f)back_left_wheel_pos[2] = -(z_half_width - wheel_radius);
	vehicle->addWheel(back_left_wheel_pos, wheelDirectionCS0, wheelAxleCS, suspension_rest_length, wheel_radius, vehicle_tuning, isFrontWheel);

	//vehicle->
	for (int i = 0; i < vehicle->getNumWheels(); i++)
	{
		btWheelInfo & wheel = vehicle->getWheelInfo(i);
		wheel.m_suspensionStiffness = suspension_stiffness;
		wheel.m_wheelsDampingRelaxation = suspension_damping;
		wheel.m_wheelsDampingCompression = suspension_compression;
		wheel.m_frictionSlip = wheel_friction;
		wheel.m_rollInfluence = roll_influence;
		//wheel.

		//wheel.m_wheelsRadius = 1.0f;
		//wheel.m_raycastInfo.m_suspensionLength = 10.0f;
	}
}

void Car::MakePhysics()
{

	//btDiscreteDynamicsWorld * m_dynamicsWorld = simulation->getDynamicsWorld();
	//GUIHelperInterface * m_guiHelper = simulation->getGuiHelper();

	this->setCarChassis();
	//this->setCarWheelForGraphics(m_guiHelper);

	//创建Car
	vehicle_ray_caster = new FilterableVehicleRaycaster(m_Res->m_Physics.m_dynamicsWorld.get());

	vehicle_ray_caster->setCollisionFilterGroup(COL_OBJECTS | COL_WALLS | COL_CHAR | COL_TERRAIN);
	vehicle_ray_caster->setCollisionFilterMask(COL_CARBODY | COL_RAY);

	vehicle = new btRaycastVehicle(vehicle_tuning, car_chassis, vehicle_ray_caster);
	vehicle->setCoordinateSystem(rightIndex, upIndex, forwardIndex); //choose coordinate system

	m_Res->m_Physics.m_dynamicsWorld.get()->addVehicle(vehicle);

	this->setCarWheelForSimulation();

	//m_Res->m_Physics.cr
	// Create wheel shapes
	//wheel_shape = new btCylinderShapeX(btVector3(wheel_width, wheel_radius, wheel_radius));
	//m_guiHelper->createCollisionShapeGraphicsObject(wheel_shape);

	//add to CarSimulation
	//simulation->addCar(this);
}

task<void> Car::LoadModels()
{
	std::vector<task<void>> tasks;

	tasks.push_back(m_bodyModel->LoadModel(m_Res, L"buggy.cmo", PHY_CONVEXHULL, 0.007f));

	tasks.push_back(m_wheelModel->LoadModel(m_Res, L"wheel2.cmo", PHY_NOTHING, 0.0038f));


	return when_all(begin(tasks), end(tasks));
}

task<void> Car::LoadTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(L"Assets\\Compiled\\carbody_red.dds"), nullptr, m_bodyModel->GetMaterialTexture(L"matbody", 0, 0)));
	tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(L"Assets\\Compiled\\carbody_blue.dds"), nullptr, m_bodyModel->GetMaterialTexture(L"matspoiler", 0, 0)));

	tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(L"Assets\\Compiled\\carbody_s.dds"), nullptr, m_bodyModel->GetMaterialEmmit(L"matbody", 1.0f)));

	tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(L"Assets\\Compiled\\wheel2.dds"), nullptr, m_wheelModel->GetMaterialTexture(L"matwheel", 0, 0)));

	return when_all(begin(tasks), end(tasks));
}


void Car::Update(float timeDelta, float timeTotal)
{
	KeyboardUpdate(timeDelta, timeTotal);

	int wheelIndex = 2;
	vehicle->applyEngineForce(engine_force, wheelIndex);
	vehicle->setBrake(breaking_force, wheelIndex);

	wheelIndex = 3;
	vehicle->applyEngineForce(engine_force, wheelIndex);
	vehicle->setBrake(breaking_force, wheelIndex);

	wheelIndex = 0;
	vehicle->setSteeringValue(vehicle_steering, wheelIndex);

	wheelIndex = 1;
	vehicle->setSteeringValue(vehicle_steering, wheelIndex);

	//for (int i = 0; i < 4; i++)
	//{
	//	wheel[i]->setWorldTransform(vehicle->getWheelInfo(i).m_worldTransform);
	//}
}

/*
void Car::updateWheelTransformForRender(CarSimulation * simulation)
{
	GUIHelperInterface * m_guiHelper = simulation->getGuiHelper();

	for (int i = 0; i < vehicle->getNumWheels(); i++)
	{
		//synchronize the wheels with the (interpolated) chassis world transform
		vehicle->updateWheelTransform(i, true);

		CommonRenderInterface* renderer = m_guiHelper->getRenderInterface();
		if (renderer)
		{

			btTransform tr = vehicle->getWheelInfo(i).m_worldTransform;
			btVector3 pos = tr.getOrigin();
			btQuaternion orn = tr.getRotation();
			renderer->writeSingleInstanceTransformToCPU(pos, orn, wheel_instances[i]);
		}
	}
}
*/

std::vector<btTransform> Car::getCarTransform() const
{
	std::vector<btTransform> trans;

	// chassis transform
	btTransform chassis_tran;
	this->car_chassis->getMotionState()->getWorldTransform(chassis_tran);
	trans.push_back(chassis_tran);

	//wheel transforms
	for (int i = 0; i < vehicle->getNumWheels(); i++)
	{
		//synchronize the wheels with the (interpolated) chassis world transform
		vehicle->updateWheelTransform(i, true);
		btTransform tr = vehicle->getWheelInfo(i).m_worldTransform;
		trans.push_back(tr);
	}

	return trans;
}

void Car::setFrontWheelXOffset(float x_offset)
{
	this->front_wheel_x_offset = x_offset;
}

void Car::setFrontWheelZOffset(float z_offset)
{
	this->front_wheel_z_offset = z_offset;
}

void Car::setBackWheelXOffset(float x_offset)
{
	this->back_wheel_x_offset = x_offset;
}

void Car::setBackWheelZOffset(float z_offset)
{
	this->back_wheel_z_offset = z_offset;
}

void Car::setCarChassisTransform(const btTransform & chassis_transform)
{
	car_chassis_transform = chassis_transform;
}

void Car::setCarChassisBoxShape(const btVector3 & half_extents)
{
	if (is_default_chassis_shape)
		delete car_chassis_shape;

	is_default_chassis_shape = false;
	car_chassis_shape = new btBoxShape(half_extents);
}

void Car::setCarChassisCustomShape(btCollisionShape * shape)
{
	if (is_default_chassis_shape)
		delete car_chassis_shape;

	is_default_chassis_shape = false;
	car_chassis_shape = shape;
}

void Car::KeyboardUpdate(float timeDelta, float timeTotal)
{
	if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::W) == true)
	{
		engine_force = max_engine_force;
		breaking_force = 0.f;
	}
	else
	{
		if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::S) == true)
		{
			engine_force = -max_engine_force;
			breaking_force = 0.f;
		}
		else
		{
			engine_force = 0.f;
			breaking_force = default_breaking_force;
		}
	}

	if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::A) == true)
	{
		if (vehicle_steering < 0.0f)
		{
			vehicle_steering += (steering_increment * 2.0f) * timeDelta;
		}
		else
		{
			vehicle_steering += (steering_increment * 2.0f) * timeDelta;
		}

		if (vehicle_steering > steering_clamp)
			vehicle_steering = steering_clamp;
	}

	if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::D) == true)
	{
		if (vehicle_steering > 0.0f)
		{
			vehicle_steering -= (steering_increment * 2.0f) * timeDelta;
		}
		else
		{
			vehicle_steering -= (steering_increment * 2.0f) * timeDelta;
		}

		if (vehicle_steering < -steering_clamp)
			vehicle_steering = -steering_clamp;
	}

	if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::A) == false && (m_Res->p_controller->KeyState(Windows::System::VirtualKey::D) == false))
	{
		if (vehicle_steering > 0)
		{
			vehicle_steering -= steering_increment * timeDelta;
		}
		else
		{
			vehicle_steering += steering_increment * timeDelta;
		}

		if (vehicle_steering < 0.01f && vehicle_steering > -0.01f)
		{
			vehicle_steering = 0.0f;
		}
	}

	/*
	bool handled = false;

	if (state)
	{
		if (is_shift_pressed == false)
		{
			switch (key)
			{
			case B3G_LEFT_ARROW:
			{
				handled = true;
				vehicle_steering += steering_increment;
				if (vehicle_steering > steering_clamp)
					vehicle_steering = steering_clamp;

				break;
			}
			case B3G_RIGHT_ARROW:
			{
				handled = true;
				vehicle_steering -= steering_increment;
				if (vehicle_steering < -steering_clamp)
					vehicle_steering = -steering_clamp;

				break;
			}
			case B3G_UP_ARROW:
			{
				handled = true;
				engine_force = max_engine_force;
				breaking_force = 0.f;
				break;
			}
			case B3G_DOWN_ARROW:
			{
				handled = true;
				engine_force = -max_engine_force;
				breaking_force = 0.f;
				break;
			}

			default:
				break;
			}
		}
	}
	else
	{
		switch (key)
		{
		case B3G_UP_ARROW:
		{
			engine_force = 0.f;
			breaking_force = default_breaking_force;
			handled = true;
			break;
		}
		case B3G_DOWN_ARROW:
		{
			engine_force = 0.f;
			breaking_force = default_breaking_force;
			handled = true;
			break;
		}
		case B3G_LEFT_ARROW:
		case B3G_RIGHT_ARROW:
		{
			handled = true;
			break;
		}
		default:

			break;
		}
	}

	*/

	//return handled;
}


void Car::Render(int alpha_mode)
{
	int i;
	//btTransform car_transform = body_transform;
	car_chassis->getMotionState()->getWorldTransform(body_transform);

	if (alpha_mode == 3)
	{
		if (true) //(m_Res->m_Lights->CheckPointPlanes(point_plane, car_transform.getOrigin().getX(), car_transform.getOrigin().getY(), car_transform.getOrigin().getZ(), m_bodyModel[0]->Extents().Radius))
		{


			m_Res->m_Camera->m_constantBufferData.model = m_Res->GetMatrix(&body_transform);
			m_Res->m_Camera->UpdateConstantBuffer();

			for (Mesh* m : m_bodyModel->m_mesh)
			{
				m->Render(*m_Res, true);
			}
		}
	}
	else
	{
		if (true) //(m_Res->m_Lights->CheckPointPlanes(point_plane, car_transform.getOrigin().getX(), car_transform.getOrigin().getY(), car_transform.getOrigin().getZ(), m_bodyModel[0]->Extents().Radius))
		{
			m_Res->m_Camera->m_constantBufferData.model = m_Res->GetMatrix(&body_transform);
			m_Res->m_Camera->UpdateConstantBuffer();

			for (Mesh* m : m_bodyModel->m_mesh)
			{
				m->Render(*m_Res);
			}
		}

		for (i = 0; i < 4; i++)
		{
			//btTransform wheel_transform = wheel[i]->getWorldTransform();

			if (true)//(m_Res->m_Lights->CheckPointPlanes(point_plane, wheel_transform.getOrigin().getX(), wheel_transform.getOrigin().getY(), wheel_transform.getOrigin().getZ(), m_wheelModel[0]->Extents().Radius))
			{
				wheel_transform[i] = vehicle->getWheelInfo(i).m_worldTransform;

				m_Res->m_Camera->m_constantBufferData.model = m_Res->GetMatrix(&wheel_transform[i]);

				//XMStoreFloat4x4(m_Res->ConstantModelBuffer(),
				//	XMLoadFloat4x4(m_Res->ConstantModelBuffer()) *
				//	XMMatrixRotationRollPitchYaw(M_PI, 0.0f, 0.0f));

				if (i == 0 || i == 2) // turn wheel round
				{
					XMStoreFloat4x4(m_Res->ConstantModelBuffer(),
						XMLoadFloat4x4(m_Res->ConstantModelBuffer()) *
						XMMatrixRotationRollPitchYaw(0.0f, 0.0f, M_PI));
				}

				m_Res->m_Camera->UpdateConstantBuffer();

				for (Mesh* m : m_wheelModel->m_mesh)
				{
					m->Render(*m_Res);
				}
			}
		}
	}
}

