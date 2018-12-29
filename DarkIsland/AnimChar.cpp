#include "pch.h"
#include "AnimChar.h"

using namespace Game;

AnimChar::AnimChar(AllResources* p_Resources, Level* pp_Level)
{
	//pos = XMFLOAT3(0.0f,5.0f,)
	p_Level = pp_Level;
	m_Res = p_Resources;
	m_deviceResources = m_Res->m_deviceResources;

	view_angle = 0.0f;
	view_angle_to = 0.0f;
	view_angle_temp = 0.0f;

	model_pos_y = 0.5f;

	view_pos_y = 0.0f;

	x_motion = 0.0f;
	z_motion = 0.0f;
	y_motion = 0.0f;

	x_motion_to = 0.0f;

	jump_anim = 0.0f;

	touch_jump = false;

	bJump = false;

	bAttack = false;

	attack_timer = 0.0f;

	attack_anim = 0.0f;

	bPlayerActive = false;

	walk_speed = 0.0f;

	touch_rotate = 0.0f;

	bWorldContact = false;

	bTouchAttack = false;

	char_angle = 0.0f;
	char_angle_to = 0.0f;

	model_matrix = m_Res->MakeMatrix(cvalues.off_x, cvalues.off_y, cvalues.off_z, cvalues.rot_x, cvalues.rot_y, cvalues.rot_z, cvalues.mesh_scale);

	m_meshModels = new MeshModel(m_deviceResources, &m_Res->m_Physics);
}

task<void> AnimChar::LoadModels()
{
	std::vector<task<void>> tasks;

	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Res,
		L"Assets\\Compiled\\" + cvalues.model_fname,
		L"",
		L"",
		m_meshModels->m_mesh,
		1.0f).then([this]()
	{
		for (Mesh* m : m_meshModels->m_mesh)
		{
			if (m->BoneInfoCollection().empty() == false)
			{
				auto animState = new AnimationState();
				animState->m_boneWorldTransforms.resize(m->BoneInfoCollection().size());
				animState->m_boneWorldTransformsB.resize(m->BoneInfoCollection().size());
				animState->m_boneWorldTransformsC.resize(m->BoneInfoCollection().size());
				animState->m_boneWorldTransformsD.resize(m->BoneInfoCollection().size());
				m->Tag = animState;
			}
		}
		m_time.clear();
		for (size_t i = 0; i < m_meshModels->m_mesh.size(); i++)
		{
			m_time.push_back(0.0f);
		}
	}));

	return when_all(begin(tasks), end(tasks));
}

task<void> AnimChar::LoadTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	for (char_tex t : cvalues.textures)
	{
		if (t.texture_type == L"t")
		{
			tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(t.texture_fname.c_str()), nullptr, m_meshModels->GetMaterialTexture(t.material_name, 0, 0)));
		}
		if (t.texture_type == L"n")
		{
			tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(t.texture_fname.c_str()), nullptr, m_meshModels->GetMaterialNormal(t.material_name, 1.0f)));
		}
		//if (t.texture_type == L"e")
		//{
		//	tasks.push_back(loader->LoadTextureAsync(texture_info[i].s_filename, nullptr, m_static_model[texture_info[i].index]->GetMaterialEmmit(m_Res->GetWC(texture_info[i].material_name), texture_info[i].intensity)));
		//}
	}

	//tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(cvalues.texture_fname.c_str()), nullptr, m_meshModels.GetMaterialTexture(cvalues.material_name, 0, 0)));
	//tasks.push_back(loader->LoadTextureAsync(ref new Platform::String(cvalues.normal_fname.c_str()), nullptr, m_meshModels.GetMaterialNormal(cvalues.material_name, 1.0f)));

	return when_all(begin(tasks), end(tasks));
}

void AnimChar::MakePhysics()
{
	ObjInfo bert_info = {
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.1f, 0.1f, 1.0f),
		0
		, (COL_WALLS | COL_CARBODY | COL_WHEEL | COL_OBJECTS)
		, (COL_CHAR) };

	m_bert_motion = MakePhysicsCylinderExtents(&bert_info, 1.0f, 1.0f, 1.0f, 0.5f);

	m_bert_motion->setDamping(0.0f, 0.2f);
	m_bert_motion->setActivationState(DISABLE_DEACTIVATION);
}

btRigidBody*  AnimChar::MakePhysicsCylinderExtents(ObjInfo* ob_info, float extent1, float extent2, float extent3, float _scale)
{
	extent1 *= _scale;
	extent2 *= _scale;
	extent3 *= _scale;

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z)));

	//m_motion->m_initialTransform = ;

	//m_motion->setWorldTransform(m_motion->m_initialTransform);

	auto fallShape = new btCylinderShape(btCylinderShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = m_Res->m_Physics.AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

void AnimChar::FinalizeCreateDeviceResources()
{
	// initialize here because it used device context so must be syncronous
	m_skinnedMeshRenderer.Initialize(m_Res->m_deviceResources->GetD3DDevice(), m_Res->m_materialBuffer);
}
void AnimChar::SetPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
	btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(x, y + 1.0f, z), btVector3(x, y - 100.0f, z));

	m_Res->m_Physics.m_dynamicsWorld->rayTest(btVector3(x, y + 1.0f, z), btVector3(x, y - 100.0f, z), RayCallback);

	if (RayCallback.hasHit())
	{
		y = RayCallback.m_hitPointWorld.getY() + 0.45f;
	}
	m_bert_motion->setWorldTransform(btTransform(btQuaternion(yaw, pitch, roll), btVector3(x, y, z)));

	bert_pos = XMFLOAT3(x, y, z);
	bert_mom = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void AnimChar::UpdatePhysics(float timeDelta, float timeTotal)
{
	if (m_bert_motion != nullptr)///(bKeyDown == false)
	{
		btTransform tran = m_bert_motion->getWorldTransform();
		float xdif = abs(bert_pos.x - tran.getOrigin().getX());
		float zdif = abs(bert_pos.z - tran.getOrigin().getZ());
		//float ydif = abs(bert_pos.y - tran.getOrigin().getY());

		/*
		btVector3 vec_dif = btVector3(abs(bert_pos.x - tran.getOrigin().getX()), abs(bert_pos.y - tran.getOrigin().getY()), abs(bert_pos.z - tran.getOrigin().getZ()));

		if (xdif > 0.000005f && xdif<1.0f &&
		ydif>0.000005f && ydif<1.0f &&
		zdif>0.000005f && zdif < 1.0f)
		{
		if (vec_dif.length() > 1.0f)
		vec_dif.normalize();

		bert_pos.x = vec_dif.getX();
		bert_pos.y = vec_dif.getY();
		bert_pos.z = vec_dif.getZ();
		}
		*/

		if (xdif > 0.000005f && xdif < 1.0f)
			bert_pos.x = tran.getOrigin().getX();

		//bert_pos.y = tran.getOrigin().getY();
		if (zdif > 0.000005f && zdif < 1.0f)
			bert_pos.z = tran.getOrigin().getZ();

		//if (ydif>0.000005f && ydif<1.0f)
		//bert_pos.y = tran.getOrigin().getY();

		/*
		btVector3 vel = m_bert_motion->m_rigidbody->getLinearVelocity();
		bert_mom.x = vel.getX();// *timeDelta;
		//bert_mom.y = vel.getY()*timeDelta*4.0f;
		bert_mom.z = vel.getZ();// *timeDelta;
		*/
	}

	/*
	if (m_Res->m_PadInput->PanCommandX() != 0.0f)
	{
	view_angle += m_Res->m_PadInput->PanCommandX()*0.04f;
	if (view_angle > M_PI*2.0f)
	view_angle -= M_PI*2.0f;

	if (view_angle <0)
	view_angle += M_PI*2.0f;
	}
	else
	if (touch_rotate != 0.0f)
	{
	view_angle += touch_rotate;
	if (view_angle > M_PI*2.0f)
	view_angle -= M_PI*2.0f;

	if (view_angle <0)
	view_angle += M_PI*2.0f;
	}
	else
	if (m_Res->m_PadInput->KeyState(Windows::System::VirtualKey::Left) == true)
	{
	view_angle -= 0.04f;
	if (view_angle <0)
	view_angle += M_PI*2.0f;
	}
	else
	if (m_Res->m_PadInput->KeyState(Windows::System::VirtualKey::Right) == true)
	{
	view_angle += 0.04f;
	if (view_angle > M_PI*2.0f)
	view_angle -= M_PI*2.0f;
	}
	*/
	//else
	{
		//view_angle += 10.0f;
		//char_angle += 10.0f;
		view_angle_to = char_angle;

		if (view_angle_to > M_PI*2.0f)
			view_angle_to -= M_PI * 2.0f;

		if (view_angle_temp < 0.0f)
			view_angle_temp += M_PI * 2.0f;

		if (view_angle_temp > M_PI*2.0f)
			view_angle_temp -= M_PI * 2.0f;

		if ((view_angle_to - view_angle_temp) > M_PI)
		{
			view_angle_to -= M_PI * 2.0f;
		}
		else
		{
			if ((view_angle_to - view_angle_temp) < -M_PI)
			{
				view_angle_to += M_PI * 2.0f;
			}
		}

		float diffr = abs(abs(view_angle_temp) - abs(view_angle_to));
		if (diffr > 0.05f)
		{
			diffr = 0.05f;
		}

		if (view_angle_temp > view_angle_to)
			view_angle_temp -= diffr;
		if (view_angle_temp < view_angle_to)
			view_angle_temp += diffr;

		view_angle = view_angle_temp - M_PI * 0.5f;
		/*

		//view_angle_to = view_angle;// +(M_PI*0.5f);
		if (true)//(char_angle > M_PI*0.5f)
		{
		view_angle_to -= (view_angle_to - (char_angle))*timeDelta*3.0f;
		}
		else
		{
		//view_angle -= (view_angle - (char_angle + (M_PI*1.5f)))*timeDelta*3.0f;
		}
		view_angle = view_angle_to-(M_PI*0.5f);
		//view_angle -= 10.0f;
		//char_angle -= 10.0f;
		*/
		//if (view_angle <0)
		//	view_angle += M_PI*2.0f;
		//if (view_angle > M_PI*2.0f)
		//	view_angle -= M_PI*2.0f;
	}

	//y_motion = 0.0f;

	//bKeyDown = false;

	if (bAttack == true)
	{
		if (attack_anim < 1.0f)
			attack_anim += 0.2f;
		if (attack_anim > 1.0f)
			attack_anim = 1.0f;

		attack_timer += timeDelta;
		if (attack_timer > 0.26f)
		{
			//bAttack = false;
			if (attack_anim > 0.0f)
				attack_anim -= 0.2f;
			if (attack_anim < 0.0f)
				attack_anim = 0.0f;
		}

		if (attack_timer > 0.3f)
		{
			bAttack = false;
		}
	}
	else
	{
		//bAttack = false;
		if (attack_anim > 0.0f)
			attack_anim -= 0.1f;
		if (attack_anim < 0.0f)
			attack_anim = 0.0f;
	}
	//x_motion = 0.0f;
	//z_motion = 0.0f;

	if (touch_move_x != 0.0f || touch_move_y != 0.0f)
	{
		float x, z;

		x = -(touch_move_x - 90.0f);
		z = -(touch_move_y - 90.0f)*0.01;

		if (x<20.0f && x>-20.0f)
		{
			x = 0.0f;
		}
		else
		{
			if (x > 20.0f)
				x -= 20.0f;
			else
				if (x < -20.0f)
					x += 20.0f;
		}

		x *= 0.01f;

		x_motion = x * 10.0f;
		z_motion = z * 10.0f;
		bKeyDown = true;
	}

	btVector3 v2 = btVector3(x_motion, 0.0f, z_motion);
	v2.normalize();
	btVector3 v3 = v2.rotate(btVector3(0.0f, 1.0f, 0.0f), -view_angle);

	if (bKeyDown == true)
	{
		char_angle_to = atan2f(v3.getZ(), v3.getX());// +view_angle;

		if (char_angle_to > M_PI*2.0f)
			char_angle_to -= M_PI * 2.0f;

		if (char_angle < 0.0f)
			char_angle += M_PI * 2.0f;

		if (char_angle > M_PI*2.0f)
			char_angle -= M_PI * 2.0f;

		if ((char_angle_to - char_angle) > M_PI)
		{
			char_angle_to -= M_PI * 2.0f;
		}
		else
		{
			if ((char_angle_to - char_angle) < -M_PI)
			{
				char_angle_to += M_PI * 2.0f;
			}
		}

		float diffr = abs(abs(char_angle) - abs(char_angle_to));
		if (diffr > 0.2f)
		{
			diffr = 0.2f;
		}

		if (char_angle < char_angle_to)
			char_angle += diffr;
		if (char_angle > char_angle_to)
			char_angle -= diffr;
		// knight
		//XMFLOAT4X4 matr = m_Res->MakeMatrix(0.0f, 0.0f, 0.0f, M_PI*0.5f, M_PI*0.5f - char_angle, 0.0f, char_scale);
		// lily
		//XMFLOAT4X4 matr = m_Res->MakeMatrix(0.0f, 0.0f, 0.0f, 0.0f, M_PI*0.5f - ang, 0.0f, char_scale);
		// Elf
		XMFLOAT4X4 matr = m_Res->MakeMatrix(cvalues.off_x, cvalues.off_y, cvalues.off_z, cvalues.rot_y - char_angle, cvalues.rot_x, cvalues.rot_z, cvalues.mesh_scale);

		model_matrix._11 = matr._11;
		model_matrix._12 = matr._12;
		model_matrix._13 = matr._13;
		model_matrix._21 = matr._21;
		model_matrix._22 = matr._22;
		model_matrix._23 = matr._23;
		model_matrix._31 = matr._31;
		model_matrix._32 = matr._32;
		model_matrix._33 = matr._33;

		if (walk_speed < cvalues.max_walk_speed)
			walk_speed += timeDelta * 18.0f;

		if (walk_speed >= cvalues.max_walk_speed)
			walk_speed = cvalues.max_walk_speed;
	}
	else
	{
		if (walk_speed > 0.0f)
			walk_speed -= timeDelta * 12.0f;

		if (walk_speed <= 0.0f)
			walk_speed = 0.0f;
	}
	//x_motion = v3.getX() *walk_speed;
	//z_motion = v3.getZ() *walk_speed;

	x_motion = cos(char_angle) *walk_speed;
	z_motion = sin(char_angle) *walk_speed;

	btCollisionWorld::ClosestRayResultCallback RayCallbackB(btVector3(bert_pos.x, bert_pos.y + 1.0f, bert_pos.z), btVector3(bert_pos.x, bert_pos.y + 10.0f, bert_pos.z));
	m_Res->m_Physics.m_dynamicsWorld->rayTest(btVector3(bert_pos.x, bert_pos.y + 1.0f, bert_pos.z), btVector3(bert_pos.x, bert_pos.y + 10.0f, bert_pos.z), RayCallbackB);
	if (RayCallbackB.hasHit())
	{
		btVector3 pos = RayCallbackB.m_hitPointWorld;
		float dist = pos.getY() - model_matrix._24;
		if (dist < 0.3f)
		{
			bert_pos.y = pos.getY() - 0.3f;
			bert_mom.y = 0.0f;
		}
	}

	btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(bert_pos.x, bert_pos.y + 1.0f, bert_pos.z), btVector3(bert_pos.x, bert_pos.y - 100.0f, bert_pos.z));
	m_Res->m_Physics.m_dynamicsWorld->rayTest(btVector3(bert_pos.x, bert_pos.y + 1.0f, bert_pos.z), btVector3(bert_pos.x, bert_pos.y - 100.0f, bert_pos.z), RayCallback);
	if (RayCallback.hasHit())
	{
		//RayCallback.m_collisionObject->
		btVector3 pos = RayCallback.m_hitPointWorld;
		float dist = model_matrix._24 - pos.getY();

		if (dist > 0.86f)
		{
			if (jump_anim < 1.0f)
				jump_anim += 0.2f;
			if (jump_anim > 1.0f)
				jump_anim = 1.0f;

			bert_mom.y -= timeDelta * 0.5f;
			bWorldContact = false;
			bJump = false;
		}
		else
		{
			if (jump_anim > 0.0f)
				jump_anim -= 0.2f;
			if (jump_anim < 0.0f)
				jump_anim = 0.0f;

			bert_pos.y = pos.getY() + 1.25f;

			if (bJump == false)
			{
				bert_mom.y = 0.0f;
			}
			bWorldContact = true;
		}
	}
	else
	{
		if (jump_anim > 0.0f)
			jump_anim -= 0.2f;
		if (jump_anim < 0.0f)
			jump_anim = 0.0f;

		bWorldContact = false;
		if (bJump == false)
		{
			bert_mom.y = 0.0f;
		}
	}

	float anim_speed = timeDelta;
	m_skinnedMeshRenderer.UpdateAnimationMix(timeDelta, m_meshModels->m_mesh, (1.0f / cvalues.max_walk_speed)*(walk_speed), jump_anim, attack_anim, cvalues.model_idle, cvalues.model_run, cvalues.model_jump, cvalues.model_attack);
	//m_skinnedMeshRenderer.UpdateAnimation(timeDelta, m_meshModels, L"my_avatar|run");

	bert_pos_last = bert_pos;
	//bert_pos.x += x_motion *timeDelta;
	bert_pos.y += bert_mom.y;
	//bert_pos.z += z_motion*timeDelta;
	m_bert_motion->setWorldTransform(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(bert_pos.x, bert_pos.y + 0.5f, bert_pos.z)));
	m_bert_motion->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));

	//view_angle += 10.0f;
	//char_angle += 10.0f;

	float t_height = p_Level->GetTerrainHeight(bert_pos.x, bert_pos.z);
	float t_height2 = p_Level->GetTerrainHeight(bert_pos.x + x_motion, bert_pos.z + z_motion);

	//x_motion *= (p_Level->GetNormal(bert_pos.x+ x_motion, bert_pos.z+ z_motion).getY()-0.4f);
	//z_motion *= (p_Level->GetNormal(bert_pos.x+ x_motion, bert_pos.z+ z_motion).getY() - 0.4f);
	//view_angle -= 10.0f;
	//char_angle -= 10.0f;

	if (true)
	{
		m_bert_motion->setLinearVelocity(btVector3(x_motion*0.5f, 0.0f, z_motion*0.5f));
	}
	else
	{
		//m_bert_motion->m_rigidbody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
	}
}

btVector3* AnimChar::GetPosition()
{
	static btVector3 player_pos;

	player_pos = btVector3(bert_pos.x, bert_pos.y, bert_pos.z);

	return &player_pos;
}

void AnimChar::RenderDepth(int alpha_mode, int point_plane)
{
	if (bPlayerActive == false)
		return;

	XMFLOAT4X4 model_matrix_b;

	bool bRender;

	//m_bert_motion->MakeMatrix();
	//m_bert_motion->GetMatrix(m_Res->ConstantModelBuffer());

	m_Res->m_Camera->m_constantBufferData.model = m_Res->GetMatrix(m_bert_motion);
	//m_Res->m_Camera->UpdateConstantBuffer();

	m_Res->m_Camera->m_constantBufferData.model._14 = bert_pos.x;
	m_Res->m_Camera->m_constantBufferData.model._24 = bert_pos.y;
	m_Res->m_Camera->m_constantBufferData.model._34 = bert_pos.z;

	model_matrix_b = model_matrix;

	model_matrix_b._24 += model_pos_y;

	//m_Res->m_Camera->SetModelMatrix(&model_matrix);

	m_Res->m_Camera->UpdateConstantBuffer();

	//model_matrix = m_Res->MakeMatrix(0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f);
	//m_bert->Render(alpha_mode);

	//SubMesh mesh = m_meshModels[0]->SubMeshes();

	if (true) //(m_loadingComplete == true)
	{
		//m_Res->SetTexture(cvalues.texture_fname, 0);

		//m_Res->SetSkinShader();

		//m_Res->m_Lights->UpdateConstantBuffer();
		//m_Res->m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_Res->m_materialBuffer.Get(), 0, nullptr, &cvalues.model_material, 0, 0);
		bRender = true;
		if (point_plane > -1)
		{
			// check point light frustum
			bRender = m_Res->m_Lights->CheckPointPlanes(point_plane, bert_pos.x, bert_pos.y, bert_pos.z, m_meshModels->m_mesh[0]->Extents().Radius);
			//bRender = true;
		}

		if (bRender == true)//m_Res->m_Lights->CheckPoint(m_static[p_index[i].part_no].x, m_static[p_index[i].part_no].y, m_static[p_index[i].part_no].z, m_static_model[m_static[p_index[i].part_no].model_index]->m_meshModels[0]->Extents().Radius, nullptr) > 0.0f)
		{
			for (UINT i = 0; i < m_meshModels->m_mesh.size(); i++)
			{
				if (m_meshModels->m_mesh[i]->alpha_mode == alpha_mode)
				{
					if (m_meshModels->m_mesh[i]->Tag != nullptr)
					{
						//m_meshModels[i]->Render(*m_Res);
						m_skinnedMeshRenderer.RenderSkinnedMesh(m_meshModels->m_mesh[i], *m_Res);
					}
				}
			}
		}

		/*
		m_Res->SetMeshShader();
		for (UINT i = 0; i < m_meshModels.size(); i++)
		{
		if (m_meshModels[i]->Tag == nullptr)
		{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		for (const Game::Mesh::SubMesh& submesh : m_meshModels[i]->SubMeshes())
		{
		const ID3D11Buffer* vbs = m_meshModels[i]->VertexBuffers()[submesh.VertexBufferIndex];

		m_Res->m_deviceResources->GetD3DDeviceContext()->IASetIndexBuffer(m_meshModels[i]->IndexBuffers()[submesh.IndexBufferIndex], DXGI_FORMAT_R16_UINT, 0);
		m_Res->m_deviceResources->GetD3DDeviceContext()->IASetVertexBuffers(0, 1, &m_meshModels[i]->VertexBuffers()[submesh.VertexBufferIndex], &stride, &offset);

		m_Res->m_deviceResources->GetD3DDeviceContext()->DrawIndexed(submesh.PrimCount * 3, submesh.StartIndex, 0);
		}
		}
		}
		*/
	}
}

void AnimChar::Render(int alpha_mode)
{
	//if (bPlayerActive == false)
	//	return;

	XMFLOAT4X4 model_matrix_b;

	model_matrix_b = model_matrix;

	model_matrix_b._24 += model_pos_y;

	//m_bert_motion->MakeMatrix();
	m_Res->m_Camera->m_constantBufferData.model = model_matrix_b;// m_Motion->GetMatrix(m_bert_motion);

	m_Res->m_Camera->m_constantBufferData.model._14 = bert_pos.x;
	m_Res->m_Camera->m_constantBufferData.model._24 = bert_pos.y;
	m_Res->m_Camera->m_constantBufferData.model._34 = bert_pos.z;

	//m_Res->m_Camera->m_constantBufferData.model = model_matrix_b;
	//m_Res->m_Camera->SetModelMatrix(&model_matrix_b);

	m_Res->m_Camera->UpdateConstantBuffer();

	//model_matrix = m_Res->MakeMatrix(0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f);
	//m_bert->Render(alpha_mode);

	//SubMesh mesh = m_meshModels[0]->SubMeshes();

	if (true)//(m_loadingComplete == true)
	{
		//m_Res->SetTexture(cvalues.texture_fname, 0);
		//m_Res->SetTexture(cvalues.normal_fname, 3);

		//m_meshModels.GetMaterialTexture(
		//m_Res->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
		//m_Res->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(3, 1, m_Texture2.GetAddressOf());

		//m_Res->m_Lights->UpdateConstantBuffer();
		m_Res->m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_Res->m_materialBuffer.Get(), 0, nullptr, &cvalues.model_material, 0, 0);

		for (UINT i = 0; i < m_meshModels->m_mesh.size(); i++)
		{
			if (true)//(m_meshModels.m_mesh[i]->Tag != nullptr && m_meshModels.m_mesh[i]->alpha_mode == alpha_mode)
			{
				//m_meshModels[i]->Render(*m_Res);
				m_skinnedMeshRenderer.RenderSkinnedMesh(m_meshModels->m_mesh[i], *m_Res);
			}
		}

		for (UINT i = 0; i < m_meshModels->m_mesh.size(); i++)
		{
			if (false)//(m_meshModels.m_mesh[i]->Tag == nullptr && m_meshModels.m_mesh[i]->alpha_mode == alpha_mode)
			{
				m_meshModels->m_mesh[i]->Render(*m_Res);
				/*
				UINT stride = sizeof(Vertex);
				UINT offset = 0;
				for (const Game::Mesh::SubMesh& submesh : m_meshModels.m_mesh[i]->SubMeshes())
				{
					const ID3D11Buffer* vbs = m_meshModels.m_mesh[i]->VertexBuffers()[submesh.VertexBufferIndex];

					m_Res->m_deviceResources->GetD3DDeviceContext()->IASetIndexBuffer(m_meshModels.m_mesh[i]->IndexBuffers()[submesh.IndexBufferIndex], DXGI_FORMAT_R16_UINT, 0);
					m_Res->m_deviceResources->GetD3DDeviceContext()->IASetVertexBuffers(0, 1, &m_meshModels.m_mesh[i]->VertexBuffers()[submesh.VertexBufferIndex], &stride, &offset);

					m_Res->m_deviceResources->GetD3DDeviceContext()->DrawIndexed(submesh.PrimCount * 3, submesh.StartIndex, 0);
				}*/
			}
		}
	}

	// to see See the cylinder
	/*
	m_bert_motion->MakeMatrix();
	m_bert_motion->GetMatrix(m_Res->ConstantModelBuffer());
	m_Res->m_Camera->UpdateConstantBuffer();
	m_cube->Render(alpha_mode);
	*/
}