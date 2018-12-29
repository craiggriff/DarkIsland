#include "pch.h"

#include "CarRig.h"



using namespace Game;

//using namespace utility;

CarRig::CarRig(AllResources* p_Resources)
{
	points = 0;

	m_Res = p_Resources;
	m_deviceResources = m_Res->m_deviceResources;

	m_phys = &m_Res->m_Physics;

	dir_position = btVector3(0.0f, 0.0f, 0.0f);
	dir_last_osition = btVector3(0.0f, 0.0f, 0.0f);

	bIsHolding = false;
	bReleaseForce = false;

	bShooting = false;
	bShoot = false;

	Wheelposition = 0;

	b_show_rockets = 0;

	carspeed = 0.0f;

	break_value = 400.0f;

	rockets = 0;

	respot_delay = 0.0f;

	bPlayerActive = false;

	last_floor_norm = btVector3(0.0f, 1.0f, 0.0f);

	bPhysicsMade = false;

	load_count = 0;

	car_point = btVector3(0.1f, 0.1f, 1.0f);
	y_offset = btVector3(0.1f, 1.0f, 0.1f);

	bSBUp = false;
	bSBDown = false;
	bSBLeft = false;
	bSBRight = false;

	bInCarView = false;

	shoot_delay_time = 0.3f;
	bAB = false;

	m_bodyModel = new MeshModel(m_deviceResources, &p_Resources->m_Physics);
	m_wheelModel = new MeshModel(m_deviceResources, &p_Resources->m_Physics);
}

void CarRig::SetSteerPosition(float _pos)
{
	steer_position = _pos;
}

btVector3* CarRig::GetDirection()
{
	return &car_point;
}

btVector3* CarRig::GetUp()
{
	return &y_offset;
}

void CarRig::Update(float timeDelta, float timeTotal)
{
		int i;

		//return;

		float wheel_height[4];
		float highest = -100.0f;
		float lowest = 100.0f;
		float highest_space = -100.0f;
		float lowest_space = 100.0f;
		
		//OutputDebugString(L"update wheel");
		if(true)
		for (i = 0; i < 4; i++)
		{
			wheel_transform[i] = wheel[i]->getWorldTransform();

			btVector3 wheel_pos = wheel_transform[i].getOrigin();

			float wheel_height = p_Level->GetTerrainHeight(wheel_pos.getX(), wheel_pos.getZ());

			if (wheel_pos.getY() < wheel_height)
			{
				wheel_transform[i].getOrigin().setY(wheel_height);
				wheel[i]->setLinearVelocity(btVector3(wheel[i]->getLinearVelocity().getX(), 0.0f, wheel[i]->getLinearVelocity().getZ()));
			}
		}
		
		
		float spacing = 0.0f;

		body_transform = carbody->getWorldTransform();

		//carbody->MakeMatrix();

		player_pos = body_transform.getOrigin();

		player_x = body_transform.getOrigin().getX();
		player_y = body_transform.getOrigin().getY();
		player_z = body_transform.getOrigin().getZ();

		//return;

		rot_mat = btMatrix3x3(body_transform.getRotation());

		x_offset = btVector3(1.0f, 0.0f, 0.0f);
		x_offset = rot_mat * x_offset;

		y_offset = btVector3(0.0f, 0.2f, 0.0f);
		y_offset = rot_mat * y_offset;

		down_offset = btVector3(0.0f, -0.3f, 0.0f);
		down_offset = rot_mat * down_offset;

		rot_mat_front = btMatrix3x3(wheel[1]->getWorldTransform().getRotation());

		x_frontoffset1 = btVector3(0.0f, 1.0f, 0.0f);
		x_frontoffset1 = rot_mat_front * x_frontoffset1;

		rot_mat_front = btMatrix3x3(wheel[3]->getWorldTransform().getRotation());

		x_frontoffset2 = btVector3(0.0f, 1.0f, 0.0f);
		x_frontoffset2 = rot_mat_front * x_frontoffset2;
		
		if (false)
		{
			bool bHasHit = false;
			btTransform block_tran;
			//spacing = wheel[i]->m_rigidbody->getWorldTransform().getOrigin().getY();
			float wheel_x = body_transform.getOrigin().getX();// carbody->m_Matrix._14;// -(y_offset.getX()*2.0f);
			float wheel_y = body_transform.getOrigin().getY();//carbody->m_Matrix._24;// -(y_offset.getX()*2.0f);
			float wheel_z = body_transform.getOrigin().getZ();//carbody->m_Matrix._34;// -(y_offset.getZ()*2.0f);

			btCollisionWorld::ClosestRayResultCallback RayCallbackB(btVector3(wheel_x, wheel_y, wheel_z), btVector3(wheel_x - (y_offset.getX()*10.0f), wheel_y - (y_offset.getY()*10.0f), wheel_z - (y_offset.getZ()*10.0f)));
			m_Res->m_Physics.m_dynamicsWorld->rayTest(btVector3(wheel_x, wheel_y, wheel_z), btVector3(wheel_x - (y_offset.getX()*10.0f), wheel_y - (y_offset.getY()*10.0f), wheel_z - (y_offset.getZ()*10.0f)), RayCallbackB);
			if (RayCallbackB.hasHit())
			{
				bHasHit = true;

				const btBroadphaseProxy* proxy = RayCallbackB.m_collisionObject->getBroadphaseHandle();

				wheel_x = RayCallbackB.m_hitPointWorld.getX();
				wheel_z = RayCallbackB.m_hitPointWorld.getZ();

				float floor_height = p_Level->GetTerrainHeight(wheel_x, wheel_z);
				btVector3 floor_norm = p_Level->GetNormal(wheel_x, wheel_z);

				//btQuaternion norm_rot = btQuaternion(-(M_PI*1.5f),(M_PI*1.5f), (M_PI*0.5f));
				//btMatrix3x3 rot_mat = btMatrix3x3(norm_rot);

				//floor_norm = rot_mat * floor_norm;

				float norm_x = floor_norm.getX();
				float norm_y = floor_norm.getY();
				float norm_z = floor_norm.getZ();

				//floor_norm.setX(floor_norm.getX());
				//floor_norm.setY(floor_norm.getY());
				//floor_norm.setZ(floor_norm.getZ());

				floor_norm.normalize();

				float dot_prod = floor_norm.dot(y_offset);

				//m_Res->m_uiControl->SetLevel(dot_prod * 100);
				//btQuaternion norm_rot = btQuaternion((M_PI*0.5f), -(M_PI*0.5f), (M_PI*0.5f));
				// cool
				if (floor_norm.dot(y_offset) > 0.18f && player_pos.distance(RayCallbackB.m_hitPointWorld) < 2.0f && proxy->m_collisionFilterMask == (COL_TERRAIN | COL_RAY))//(wheel_y-3.0f<floor_height)
				{
					btQuaternion floor_rot = btQuaternion(floor_norm.getX(), floor_norm.getY(), floor_norm.getZ(), 0.0f);
					//floor_rot = norm_rot * floor_rot;
					//floor_rot.setY(-floor_rot.getY());

					block_tran.setOrigin(btVector3(wheel_x - floor_norm.getX()*0.95f, floor_height - (floor_norm.getY()*0.95f), wheel_z - floor_norm.getZ()*0.95f));

					block_tran.setRotation(floor_rot);

					//floorblock[i]->setWorldTransform(block_tran);
					m_FloorPlane->setWorldTransform(block_tran);
				}
				else
				{
					bHasHit = false;
				}
			}

			if (bHasHit == false)
			{
				btTransform block_tran;
				block_tran.setOrigin(btVector3(0.0f, p_Level->GetTerrainHeight(wheel_x, wheel_z) + 1.0f, 0.0f));

				btQuaternion floor_rot = btQuaternion(0.0f, 1.0f, 0.0f, 0.0f);
				block_tran.setRotation(floor_rot);

				m_FloorPlane->setWorldTransform(block_tran);

				for (i = 0; i < 4; i++)
				{
					btBroadphaseProxy* proxy = wheel[i]->getBroadphaseProxy();
					proxy->m_collisionFilterGroup = (COL_OBJECTS | COL_WALLS | COL_CHAR | COL_TERRAIN);
				}
				btBroadphaseProxy* proxy = carbody->getBroadphaseProxy();
				proxy->m_collisionFilterGroup = (COL_OBJECTS | COL_WALLS | COL_CHAR | COL_TERRAIN);
			}
			else
			{
				for (i = 0; i < 4; i++)
				{
					btBroadphaseProxy* proxy = wheel[i]->getBroadphaseProxy();
					proxy->m_collisionFilterGroup = (COL_WHEELLIFT | COL_OBJECTS | COL_WALLS | COL_CHAR);
				}
				btBroadphaseProxy* proxy = carbody->getBroadphaseProxy();
				proxy->m_collisionFilterGroup = (COL_WHEELLIFT | COL_OBJECTS | COL_WALLS | COL_CHAR);
			}
		}

		if (shooting_delay > 0)
		{
			shooting_delay -= timeDelta;
		}
		else
		{
			if (bShooting == true)
			{
				shooting_delay = shoot_delay_time;
				bShoot = true;
			}
		}

		btVector3 lin_vel = carbody->getLinearVelocity();

		car_point = btVector3(0.0f, 0.0f, 1.0f);
		car_point = rot_mat * car_point;

		carspeed = lin_vel.dot(car_point);

		for (int row = 0; row < 3; ++row)
			for (int column = 0; column < 3; ++column)
				point_martix.m[row][column] = rot_mat[column][row];

		for (int column = 0; column < 3; ++column)
			point_martix.m[3][column] = 0.0f;

		if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::A) == true)// || m_Resources->m_PadInput->MoveCommandX()<-0.2f || bSBLeft == true)
		{
			if (front_wheel_angle > -cvalues.max_wheel_turn)
			{
				SetFrontWheelAngle(front_wheel_angle - (cvalues.max_wheel_turn*0.05f));
			}
		}
		else
		{
			if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::D) == true)// || m_Resources->m_PadInput->MoveCommandX()>0.2f || bSBRight == true)
			{
				if (front_wheel_angle < cvalues.max_wheel_turn)
				{
					SetFrontWheelAngle(front_wheel_angle + (cvalues.max_wheel_turn*0.05f));
				}
			}
			else
			{

				if (front_wheel_angle > 0.0f)
				{
					front_wheel_angle -= 0.03f;
					if (front_wheel_angle < 0.0f)
						front_wheel_angle = 0.0f;
				}
				if (front_wheel_angle < 0.0f)
				{
					front_wheel_angle += 0.03f;
					if (front_wheel_angle > 0.0f)
						front_wheel_angle = 0.0f;
				}

				SetFrontWheelAngle(front_wheel_angle);
			}
		}

		if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::W) == true || bSBUp == true)
		{
			if (carspeed < -0.5f)
				Breaking();
			else
				SetMotors(cvalues.max_speed, cvalues.torque);
		}
		else
		{
			Breaking();
		}
		/*
		if (bPlayerActive == true)
		{
			if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::W) == true || bSBUp == true)
			{
				if (carspeed < -0.5f)
					Breaking();
				else
					SetMotors(cvalues.max_speed, cvalues.torque);
			}

			if (m_Res->p_controller->RightTrigger() > 0.0f)
			{
				if (carspeed < -0.5f)
					Breaking();
				else
					SetMotors(cvalues.max_speed*m_Res->p_controller->RightTrigger(), cvalues.torque);
			}

			if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::S) == true || bSBDown == true)
			{
				if (carspeed > 0.5f)
					Breaking();
				else
					SetMotors((-cvalues.max_speed*0.5f), -cvalues.torque*0.5f);
			}
			if (m_Res->p_controller->LeftTrigger() > 0.0f)
			{
				if (carspeed > 0.5f)
					Breaking();
				else
					SetMotors((-cvalues.max_speed*0.3f)*m_Res->p_controller->LeftTrigger(), -cvalues.torque*0.3f);
			}

			if (steer_position != 0.0f)
			{
				SetFrontWheelAngle(cvalues.max_wheel_turn*steer_position);
			}
			else
			{
				if (m_Res->p_controller->MoveCommandX() < -0.2f || m_Res->p_controller->MoveCommandX() > 0.2f)
				{
					SetFrontWheelAngle(cvalues.max_wheel_turn*m_Res->p_controller->MoveCommandX());

				}
				else
				{
					if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::A) == true)// || m_Resources->m_PadInput->MoveCommandX()<-0.2f || bSBLeft == true)
					{
						if (front_wheel_angle > -cvalues.max_wheel_turn)
						{
							SetFrontWheelAngle(front_wheel_angle - (cvalues.max_wheel_turn*0.05f));
						}
					}
					else
					{
						if (m_Res->p_controller->KeyState(Windows::System::VirtualKey::D) == true)// || m_Resources->m_PadInput->MoveCommandX()>0.2f || bSBRight == true)
						{
							if (front_wheel_angle < cvalues.max_wheel_turn)
							{
								SetFrontWheelAngle(front_wheel_angle + (cvalues.max_wheel_turn*0.05f));
							}
						}
						else
						{

							if (front_wheel_angle > 0.0f)
							{
								front_wheel_angle -= 0.03f;
								if (front_wheel_angle < 0.0f)
									front_wheel_angle = 0.0f;
							}
							if (front_wheel_angle < 0.0f)
							{
								front_wheel_angle += 0.03f;
								if (front_wheel_angle > 0.0f)
									front_wheel_angle = 0.0f;
							}

							SetFrontWheelAngle(front_wheel_angle);
						}
					}
				}
			}
			if ((m_Res->p_controller->KeyState(Windows::System::VirtualKey::R) == true || m_Res->p_controller->ButtonYwas() == true) && respot_delay <= 0.0f)
			{
				respot_delay = 3.0f;
				if (p_Level->GetTerrainHeight(player_x, player_z) < 1.0)
				{
					for (i = 0; i < 200; i++)
					{
						if (p_Level->GetTerrainHeight(player_x - (float)i, player_z)>1.0f)
						{
							i = i + 8;
							SetCarPosition(player_x - (float)i, p_Level->GetTerrainHeight(player_x - (float)i, player_z) + 5.0f, player_z, atan2f(car_point.getX(), car_point.getZ()), 0.0f, 0.0f);
							break;
						}
						if (p_Level->GetTerrainHeight(player_x + (float)i, player_z)>1.0f)
						{
							i = i + 8;
							SetCarPosition(player_x + (float)i, p_Level->GetTerrainHeight(player_x + (float)i, player_z) + 5.0f, player_z, atan2f(car_point.getX(), car_point.getZ()), 0.0f, 0.0f);
							break;
						}
						if (p_Level->GetTerrainHeight(player_x, player_z - (float)i)>1.0f)
						{
							i = i + 8;
							SetCarPosition(player_x, p_Level->GetTerrainHeight(player_x, player_z - (float)i) + 5.0f, player_z - (float)i, atan2f(car_point.getX(), car_point.getZ()), 0.0f, 0.0f);
							break;
						}
						if (p_Level->GetTerrainHeight(player_x, player_z + (float)i)>1.0f)
						{
							i = i + 8;
							SetCarPosition(player_x, p_Level->GetTerrainHeight(player_x, player_z + (float)i) + 5.0f, player_z + (float)i, atan2f(car_point.getX(), car_point.getZ()), 0.0f, 0.0f);
							break;
						}
					}
				}
				else
				{
					SetCarPosition(player_x, p_Level->GetTerrainHeight(player_x, player_z) + 5.0f, player_z, atan2f(car_point.getX(), car_point.getZ()), 0.0f, 0.0f);
				}
			}
			else
			{
				respot_delay -= timeDelta;
			}

		}
		else
		{
			Breaking();
		}
		*/
}

void Game::CarRig::SetWheelActivation(bool bActive)
{
	for (int i = 0; i < 4; i++)
	{
		if (bActive == true)
		{
			wheel[i]->setActivationState(DISABLE_DEACTIVATION);
		}
		else
		{
			wheel[i]->setActivationState(WANTS_DEACTIVATION);
		}
	}
}

btVector3* CarRig::GetPosition()
{
	return &player_pos;
}

void CarRig::Breaking()
{
	wheel[0]->setDamping(0.0f, break_value);
	wheel[1]->setDamping(0.0f, break_value);
	wheel[2]->setDamping(0.0f, break_value);
	wheel[3]->setDamping(0.0f, break_value);
}

void CarRig::SetMotors(float targetVelocity, float maxMotorImpulse)
{
	wheel[0]->setDamping(0.0f, cvalues.wheel_angular_damping);
	wheel[1]->setDamping(0.0f, cvalues.wheel_angular_damping);
	wheel[2]->setDamping(0.0f, cvalues.wheel_angular_damping);
	wheel[3]->setDamping(0.0f, cvalues.wheel_angular_damping);
	if (targetVelocity < 0.0f)
	{
		targetVelocity = -targetVelocity;
		maxMotorImpulse = -maxMotorImpulse;
		//if (wheel[0]->getAngularVelocity().length() < targetVelocity)
		wheel[0]->applyTorqueImpulse(-x_offset * maxMotorImpulse);

		//if (wheel[1]->getAngularVelocity().length() < targetVelocity)
		wheel[1]->applyTorqueImpulse(-x_frontoffset1 * maxMotorImpulse);

		//if (wheel[2]->getAngularVelocity().length() < targetVelocity)
		wheel[2]->applyTorqueImpulse(-x_offset * maxMotorImpulse);

		//if (wheel[3]->getAngularVelocity().length() < targetVelocity)
		wheel[3]->applyTorqueImpulse(-x_frontoffset2 * maxMotorImpulse);
	}
	else
	{
		//if (wheel[0]->getAngularVelocity().length() < targetVelocity)
		wheel[0]->applyTorqueImpulse(x_offset*maxMotorImpulse);

		//if (wheel[1]->getAngularVelocity().length() < targetVelocity)
		wheel[1]->applyTorqueImpulse(x_frontoffset1*maxMotorImpulse);

		//if (wheel[2]->getAngularVelocity().length() < targetVelocity)
		wheel[2]->applyTorqueImpulse(x_offset*maxMotorImpulse);

		//if (wheel[3]->getAngularVelocity().length() < targetVelocity)
		wheel[3]->applyTorqueImpulse(x_frontoffset2*maxMotorImpulse);
	}
}

task<void> CarRig::LoadModels()
{
	std::vector<task<void>> tasks;

	tasks.push_back(m_bodyModel->LoadModel(m_Res, cvalues.body_fname, PHY_CONVEXHULL, cvalues.body_scale*cvalues.car_scale));

	tasks.push_back(m_wheelModel->LoadModel(m_Res, cvalues.wheel_fname, PHY_NOTHING, cvalues.wheel_scale*cvalues.car_scale));

	/*
	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Res,
		L"Assets\\Compiled\\" + cvalues.body_fname,
		L"",
		L"",
		m_bodyModel->m_mesh,
		cvalues.body_scale*cvalues.car_scale));

	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Res,
		L"Assets\\Compiled\\" + cvalues.wheel_fname,
		L"",
		L"",
		m_wheelModel->m_mesh, cvalues.wheel_scale*cvalues.car_scale));
		*/
	return when_all(begin(tasks), end(tasks));
}

task<void> CarRig::LoadTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(cvalues.body_tex0.c_str()), nullptr, m_bodyModel->GetMaterialTexture(L"matbody", 0, 0)));
	tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(cvalues.body_tex1.c_str()), nullptr, m_bodyModel->GetMaterialTexture(L"matspoiler", 0, 0)));

	tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(cvalues.body_light0.c_str()), nullptr, m_bodyModel->GetMaterialEmmit(cvalues.body_matname0, 1.0f)));

	tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(cvalues.wheel_tex0.c_str()), nullptr, m_wheelModel->GetMaterialTexture(L"matwheel", 0, 0)));

	return when_all(begin(tasks), end(tasks));
}

void CarRig::Initialize(Level* pp_Level)
{
	float car_mass = 5.1f;

	p_Level = pp_Level;
}

void CarRig::RemoveContraints()
{
	//return;
	if (bPhysicsMade == false)
		return;

	carbody->removeConstraintRef(sus[0]);
	carbody->removeConstraintRef(sus[1]);
	carbody->removeConstraintRef(sus[2]);
	carbody->removeConstraintRef(sus[3]);

	wheel[0]->removeConstraintRef(sus[0]);
	wheel[1]->removeConstraintRef(sus[1]);
	wheel[2]->removeConstraintRef(sus[2]);
	wheel[3]->removeConstraintRef(sus[3]);
}

void CarRig::SetWheelTurn(int turn_to)
{
	float _turn = turn_to * 0.3f;

	SetFrontWheelAngle(_turn);
	//SetRearWheelAngle(-_turn);
}

void CarRig::SetFrontWheelAngle(float _angle)
{
	front_wheel_angle = _angle;

	btQuaternion rot_quat = btQuaternion((M_PI*0.5f) - _angle, 0.0f, 0.0f);

	frameInA[1].setRotation(rot_quat);
	sus[1]->setFrames(frameInA[1], frameInB[1]);

	frameInA[3].setRotation(rot_quat);
	sus[3]->setFrames(frameInA[3], frameInB[3]);
}

void CarRig::SetRearWheelAngle(float _angle)
{
	front_wheel_angle = _angle;

	btQuaternion rot_quat = btQuaternion((M_PI*0.5f) - _angle, 0.0f, 0.0f);

	frameInA[1].setRotation(rot_quat);
	sus[1]->setFrames(frameInA[1], frameInB[1]);

	frameInA[3].setRotation(rot_quat);
	sus[3]->setFrames(frameInA[3], frameInB[3]);
}

void CarRig::MakePhysics()
{
	int i;
	//exit(0);
	btQuaternion rot_quat;

	float softness = 1.0f;
	float bias = 1.0f;
	float relaxation = 1.0f;
	float wheelheight = -0.22f;

	float hit_fraction = 0.0f;

	float steer_ammount = 0.3f;
	float wheel_shine = 0.1f;

	wheel_x_off = cvalues.wheel_x_off *cvalues.car_scale * 100.0f;
	wheel_y_off = cvalues.wheel_y_off *cvalues.car_scale * 100.0f;
	wheel_z_off = cvalues.wheel_z_off *cvalues.car_scale * 100.0f;

	float lin_strength = 0.9f;
	float ang_strength = 0.5f;

	bool bEllipsoidWheels = false;

	ObjInfo dum_objInfo = {
		XMFLOAT3(0.0f, 5.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.1f, 1.3f),
		0
		, (COL_WHEEL)
		, (COL_WHEELLIFT) };

	btDefaultMotionState* floor = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 1.0f, 0.0f)));
	auto floorShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);//floor
	m_FloorPlane = m_phys->AddPhysicalObject(floorShape, floor, btVector3(0, 0, 0), &dum_objInfo);

	ObjInfo car_info = {
		XMFLOAT3(0.0f, 5.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.1f, 1.3f),
		0
		, (COL_OBJECTS | COL_WALLS | COL_CHAR | COL_TERRAIN)
		, (COL_CARBODY | COL_RAY) };

	//carbody = m_bodyModel->MakePhysicsCompoundBoxFromMeshModel(&car_info, 0.0f, cvalues.body_floor_distance, 0.0f, 1.8f);

	carbody = m_bodyModel->MakePhysicsConvexHullFromMeshModel(&car_info, 1.0f);
	carbody->setActivationState(DISABLE_DEACTIVATION);

	ObjInfo info;

	info = {
		XMFLOAT3(2.0f, 5.0f, -2.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(10.0f, 0.5f, cvalues.back_friction),
		0
		, (COL_WHEELLIFT | COL_OBJECTS | COL_WALLS | COL_CHAR | COL_TERRAIN)
		, (COL_WHEEL | COL_RAY) };

	for (i = 0; i < 4; i++)
	{
		switch (i)
		{
			/* REAR LEFT */
		case 0:info.mrf.z = cvalues.back_friction; wheel_offset[i] = btVector3(wheel_x_off, wheel_y_off, -wheel_z_off); break;
			/* FRONT LEFT */
		case 1:info.mrf.z = cvalues.front_friction; wheel_offset[i] = btVector3(wheel_x_off, wheel_y_off, wheel_z_off); break;
			/* REAR RIGHT */
		case 2:info.mrf.z = cvalues.back_friction; wheel_offset[i] = btVector3(-wheel_x_off, wheel_y_off, -wheel_z_off); break;
			/* FRONT RIGHT */
		case 3:info.mrf.z = cvalues.front_friction; wheel_offset[i] = btVector3(-wheel_x_off, wheel_y_off, wheel_z_off); break;
		}

		if (wheel[i] == nullptr)
		{
			if (bEllipsoidWheels == false)
			{
				wheel[i] = m_wheelModel->MakePhysicsSphereFromMeshModel(&info, 1.0f);
			}
			else
			{
				wheel[i] = m_wheelModel->MakePhysicsEllipseoidFromMeshModel(&info, 1.0f);
			}
		}
		wheel[i]->setActivationState(DISABLE_DEACTIVATION);
		//wheel[i]->setDamping(cvalues.wheel_linear_damping, cvalues.wheel_angular_damping);
		//wheel[i]->setHitFraction(hit_fraction);

		if (cvalues.enable_antistropic_friction == true)
		{
			wheel[i]->setAnisotropicFriction(
				btVector3(cvalues.antistropic_friction_x,
					cvalues.antistropic_friction_y,
					cvalues.antistropic_friction_z), 1);
		}

		btVector3 pivotInA = wheel_offset[i];
		btVector3 pivotInB = btVector3(0.0f, 0.0f, 0.0f);
		btVector3 axisInA = btVector3(-10.0f, 0.0f, 0.0f);
		btVector3 axisInB = btVector3(0.0f, 1.0f, 0.0f);

		frameInA[i] = btTransform::getIdentity();
		frameInA[i].setOrigin(pivotInA);
		frameInB[i] = btTransform::getIdentity();
		frameInB[i].setOrigin(pivotInB);

		rot_quat = btQuaternion(M_PI*0.5, 0.0f, 0.0f);
		frameInA[i].setRotation(rot_quat);

		sus[i] = new btGeneric6DofSpringConstraint(*carbody, *wheel[i],
			frameInA[i],
			frameInB[i], true);

		sus[i]->setLinearUpperLimit(btVector3(0, cvalues.sus_travel, 0));
		sus[i]->setLinearLowerLimit(btVector3(0, 0, 0));
		//sus[i]->setEquilibriumPoint(2, 0.0f);
		sus[i]->setAngularLowerLimit(btVector3(M_PI + M_PI * 0.5f, 0.0f, 1.0f));
		sus[i]->setAngularUpperLimit(btVector3(M_PI + M_PI * 0.5f, 0.0f, 0.0f));

		sus[i]->setDbgDrawSize(btScalar(5.f));

		
		//sus[i]->enableSpring(1, true);
		//sus[i]->setStiffness(1, cvalues.sus_stiffness);
		//sus[i]->setDamping(1, cvalues.sus_damping);
		/*
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, lin_strength, 0);
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, lin_strength, 1);
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, lin_strength, 2);
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, ang_strength, 3);
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, ang_strength, 4);
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, ang_strength, 5);
		*/
		m_phys->m_dynamicsWorld->addConstraint(sus[i], true);
		
	}
}

void CarRig::RenderDepth(int alpha_mode, int point_plane)
{
	int i;
	//btTransform car_transform = carbody->getWorldTransform();

	if (m_Res->m_Lights->CheckPointPlanes(point_plane, body_transform.getOrigin().getX(), body_transform.getOrigin().getY(), body_transform.getOrigin().getZ(), m_bodyModel->m_mesh[0]->Extents().Radius))
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

		if (m_Res->m_Lights->CheckPointPlanes(point_plane, wheel_transform[i].getOrigin().getX(), wheel_transform[i].getOrigin().getY(), wheel_transform[i].getOrigin().getZ(), m_wheelModel->m_mesh[0]->Extents().Radius))
		{
			m_Res->m_Camera->m_constantBufferData.model = m_Res->GetMatrix(&wheel_transform[i]);

			if (i > 1) // turn wheel round
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

void CarRig::Render(int alpha_mode)
{
	int i;
	//btTransform car_transform = body_transform;

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
			m_Res->m_Camera->m_constantBufferData.model =  m_Res->GetMatrix(&body_transform);
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
				m_Res->m_Camera->m_constantBufferData.model = m_Res->GetMatrix(&wheel_transform[i]);

				if (i > 1) // turn wheel round
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

void CarRig::SetPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
	wheel[0]->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll - M_PI * 0.5f), btVector3(x, y, z)));
	wheel[1]->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll - M_PI * 0.5f), btVector3(x, y, z)));
	wheel[2]->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll - M_PI * 0.5f), btVector3(x, y, z)));
	wheel[3]->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll - M_PI * 0.5f), btVector3(x, y, z)));

	carbody->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll), btVector3(x, y, z)));
}

void CarRig::SetCarPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
	btQuaternion rot_quat = btQuaternion(yaw, pitch, roll);

	btMatrix3x3 rotation = btMatrix3x3(rot_quat);

	btVector3 off0 = rotation * wheel_offset[0];
	btVector3 off1 = rotation * wheel_offset[1];
	btVector3 off2 = rotation * wheel_offset[2];
	btVector3 off3 = rotation * wheel_offset[3];

	wheel[0]->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll - M_PI * 0.5f), btVector3(x + off0.getX(), y + off0.getY(), z + off0.getZ())));
	wheel[1]->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll - M_PI * 0.5f), btVector3(x + off0.getX(), y + off0.getY(), z + off0.getZ())));
	wheel[2]->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll - M_PI * 0.5f), btVector3(x + off0.getX(), y + off0.getY(), z + off0.getZ())));
	wheel[3]->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll - M_PI * 0.5f), btVector3(x + off0.getX(), y + off0.getY(), z + off0.getZ())));

	//wheel[0]->SetPosition(x + off0.getX(), y + off0.getY(), z + off0.getZ(), yaw, pitch, roll - M_PI*0.5f);// setWorldTransform(transform1);
	//wheel[1]->SetPosition(x + off1.getX(), y + off1.getY(), z + off1.getZ(), yaw, pitch, roll - M_PI*0.5f);
	//wheel[2]->SetPosition(x + off2.getX(), y + off2.getY(), z + off2.getZ(), yaw, pitch, roll - M_PI*0.5f);
	//wheel[3]->SetPosition(x + off3.getX(), y + off3.getY(), z + off3.getZ(), yaw, pitch, roll - M_PI*0.5f);

	carbody->setLinearVelocity(btVector3(0.0f, 0.1f, 0.0f));
	carbody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));

	wheel[0]->setLinearVelocity(btVector3(0.0f, 0.1f, 0.0f));
	wheel[0]->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
	wheel[1]->setLinearVelocity(btVector3(0.0f, 0.1f, 0.0f));
	wheel[1]->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
	wheel[2]->setLinearVelocity(btVector3(0.0f, 0.1f, 0.0f));
	wheel[2]->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
	wheel[3]->setLinearVelocity(btVector3(0.0f, 0.1f, 0.0f));
	wheel[3]->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));

	carbody->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll), btVector3(x, y, z)));
}

CarRig::~CarRig(void)
{
}