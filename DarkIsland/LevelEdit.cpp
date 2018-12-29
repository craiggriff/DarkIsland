#include "pch.h"
#include "LevelEdit.h"

using namespace Game;

// std::sort with inlining
struct statics_ptr_compare {
	// the compiler will automatically inline this
	bool operator()(const item_pointer &a, const item_pointer &b) {
		return strcmp(a.group, b.group) > 0;
	}
};

LevelEdit::LevelEdit(AllResources* p_Resources, MoveLookController^ p_controller, Level* pp_Level, Statics* pp_Statics, Stuff* pp_Stuff) :
	m_Res(p_Resources), p_Level(pp_Level), p_Statics(pp_Statics), p_Stuff(pp_Stuff), m_controller(p_controller)
{
	m_deviceResources = p_Resources->m_deviceResources;

	design_spacing = 1;

	stretch_width = 5.0f;

	bMakePath = false;
	bClearPaths = false;

	bLoopPath = true;
	bDipPath = true;

	bColKeyDown = false;

	bCamLookPosition = false;
	bSnapGridEdit = true;

	ground_texture_index = 0;

	area_to_clear = 1;

	yaw_angle_snap = 0;
	yaw_angle = 0.0f;

	camera_height = 12.0f;

	m_Marker = new MeshModel(m_deviceResources, &p_Resources->m_Physics);
	m_MarkerPath = new MeshModel(m_deviceResources, &p_Resources->m_Physics);

	m_CameraPath = new CameraPath(m_Res);
}

void LevelEdit::SetModelNumbers()
{
	ptr_statics.clear(); // resize(p_Statics->total_static_models);

	// create static model pointers
	for (int i = 0; i < p_Statics->total_static_models + 1; i++)
	{
		item_pointer pt_statics;
		pt_statics.index = i;
		sprintf_s(pt_statics.group, "%s", p_Statics->m_static_model[i]->group);
		pt_statics.type = p_Statics->m_static_model[i]->type;

		ptr_statics.push_back(pt_statics);
	}

	std::sort(ptr_statics.begin(), ptr_statics.end(), statics_ptr_compare{});

	current_item_group = 0;

	int current_item_index = 0;
	total_item_groups = -1;

	char item_group[40] = "def";
	for (item_pointer& itemPtr : ptr_statics)
	{
		if (strcmp(itemPtr.group, item_group))
		{
			total_item_groups += 1;
			strcpy(item_group, itemPtr.group);

			itemPtr.group_index_start = current_item_index;
		}
		itemPtr.group_index = total_item_groups;
		current_item_index++;
	}

	max_model_pointer_id = p_Statics->total_static_models + p_Stuff->total_stuff_models;
	model_pointer_id = -1;
}

void LevelEdit::UpdateCurrentItemPointer(int pointer_delta)
{
	if (pointer_delta != 0)
	{
		if (m_controller->KeyState(Windows::System::VirtualKey::Shift, false))
		{
			current_item_group += pointer_delta;
			if (current_item_group > total_item_groups)
			{
				current_item_group = total_item_groups;
			}
			if (current_item_group < 0)
			{
				current_item_group = 0;
			}

			for (item_pointer& itemPtr : ptr_statics)
			{
				if (itemPtr.group_index == current_item_group)
				{
					model_pointer_id = itemPtr.group_index_start;
					break;
				}
			}
		}
		else
		{
			model_pointer_id += pointer_delta;

			if (model_pointer_id < -2)
				model_pointer_id = -2;

			if (model_pointer_id > max_model_pointer_id + 2)
				model_pointer_id = max_model_pointer_id + 2;


			current_item_group = ptr_statics[model_pointer_id].group_index;
		}
	}
}

void LevelEdit::UpdatePhysics()
{
	p_Statics->UpdatePhysics();
}

void LevelEdit::LeftMouse()
{
	srand(m_Res->p_Timer->GetElapsedTicks());

	if (model_pointer_id == -2)
	{
		PathPoint ppoint;
		ppoint.pos.x = hit_point.x;
		ppoint.pos.y = hit_point.y;
		ppoint.pos.z = hit_point.z;
		p_Level->m_Paths->AddPoint(0, &ppoint);
	}
	else
	{
		if (model_pointer_id == -1 || m_controller->KeyState(Windows::System::VirtualKey::F10, false) == true)
		{
			p_Stuff->Clear(hit_point.x, hit_point.z);
			p_Statics->Clear(hit_point.x, hit_point.z);
		}
		else
		{
			if (model_pointer_id < p_Statics->total_static_models + 1)
			{
				p_Statics->CreateOne(ptr_statics[model_pointer_id].index, hit_point.x, hit_point.y, hit_point.z, yaw_angle, 0.0f, 0.0f, hit_point.y - p_Level->GetTerrainHeight(hit_point.x, hit_point.z));

				if (m_controller->KeyState(Windows::System::VirtualKey::F1, false) == true)
				{
					for (int i = 0; i < 200; i++)
					{
						int res = rand() % p_Statics->total_static_models;
						if (!strcmp(p_Statics->m_static_model[ptr_statics[model_pointer_id].index]->group, p_Statics->m_static_model[ptr_statics[res].index]->group))
						{
							model_pointer_id = res;
							break;
						}
					}
				}
				if (m_controller->KeyState(Windows::System::VirtualKey::F2, false) == true)
				{
					yaw_angle = (float(rand() % 1000)*0.001)*(M_PI*2.0f);
				}
			}
			else
			{
				if (model_pointer_id < max_model_pointer_id + 2)
				{
					p_Stuff->CreateOne(model_pointer_id - p_Statics->total_static_models - 1, hit_point.x, hit_point.y, hit_point.z, yaw_angle, 0.0f, 0.0f, hit_point.y - p_Level->GetTerrainHeight(hit_point.x, hit_point.z));
					//for (Mesh* m : p_Stuff->m_stuff_model[model_pointer_id - p_Statics->total_static_models]->m_mesh)
					//{
					//	m->Render(*m_Res);
					//}
				}
			}
		}
	}
}

void LevelEdit::MiddleMouse()
{
	if (model_pointer_id == -1 || m_controller->KeyState(Windows::System::VirtualKey::Escape, false) == true)
	{
		p_Stuff->Clear(hit_point.x, hit_point.z);
		p_Statics->Clear(hit_point.x, hit_point.z);
	}
	else
	{
		if (last_hit_point.x != hit_point.x && last_hit_point.z != hit_point.z)
		{
			last_hit_point = hit_point;

			if (model_pointer_id < p_Statics->total_static_models + 1)
			{
				p_Statics->CreateOne(ptr_statics[model_pointer_id].index, hit_point.x, hit_point.y, hit_point.z, yaw_angle, 0.0f, 0.0f, hit_point.y - p_Level->GetTerrainHeight(hit_point.x, hit_point.z));
				if (m_controller->KeyState(Windows::System::VirtualKey::F1, false) == true)
				{
					for (int i = 0; i < 200; i++)
					{
						int res = rand() % p_Statics->total_static_models;
						if (!strcmp(p_Statics->m_static_model[ptr_statics[model_pointer_id].index]->group, p_Statics->m_static_model[ptr_statics[res].index]->group))
						{
							model_pointer_id = res;
							break;
						}
					}
				}
				if (m_controller->KeyState(Windows::System::VirtualKey::F2, false) == true)
				{
					yaw_angle = (float(rand() % 1000)*0.001)*(M_PI*2.0f);
				}
			}
			else
			{
				if (model_pointer_id < max_model_pointer_id + 2)
				{
					p_Stuff->CreateOne(model_pointer_id - p_Statics->total_static_models - 1, hit_point.x, hit_point.y, hit_point.z, 0.0f, 0.0f, 0.0f, hit_point.y - p_Level->GetTerrainHeight(hit_point.x, hit_point.z));
					//for (Mesh* m : p_Stuff->m_stuff_model[model_pointer_id - p_Statics->total_static_models]->m_mesh)
					//{
					//	m->Render(*m_Res);
					//}
				}
			}
		}
	}
}

void LevelEdit::RightMouse()
{
	//Causes Pause
}

void LevelEdit::SaveLevel()
{
	FILE * pFile;
	char bat_filename[140];
	wchar_t state_folder[140];
	wchar_t game_folderC[140];
	wchar_t game_folderH[140];

	wchar_t info_folder[140];

	swprintf_s(state_folder, L"%s", Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data());

	swprintf_s(game_folderC, L"C:\\Users\\craig\\Desktop\\DarkIsland\\Assets\\", Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data());
	swprintf_s(game_folderH, L"H:\\DarkIsland\\Assets\\");

	swprintf_s(info_folder, L"%s\\LevelBinary", Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data());

	CreateDirectory(info_folder, NULL);

	/*
	CopyFile2(
		game_folderC,
		state_folder,
		false
	);
	*/

	p_Level->SaveBinary(game_level);
	p_Statics->SaveBinary(game_level);
	p_Stuff->SaveBinary(game_level);

	m_CameraPath->SaveBinary(game_level);

	sprintf_s(bat_filename, "%s\\ToContentDeskG.bat", m_Res->local_file_folder);
	fopen_s(&pFile, bat_filename, "w");
	if (pFile != NULL)
	{
		fprintf_s(pFile, "xcopy %s\\*.bmp G:\\Desktop\\DarkIsland\\Assets\\LevelBinary\\*.* /G\n", m_Res->local_file_folder);
		fprintf_s(pFile, "xcopy %s\\game-data.db G:\\Desktop\\DarkIsland\\Assets\\game-data.db /G\n", m_Res->local_file_folder);
		fprintf_s(pFile, "del %s\\*.bmp\n", m_Res->local_file_folder);
		fclose(pFile);
	}

	sprintf_s(bat_filename, "%s\\ToContentDeskD.bat", m_Res->local_file_folder);
	fopen_s(&pFile, bat_filename, "w");
	if (pFile != NULL)
	{
		fprintf_s(pFile, "xcopy %s\\*.bmp D:\\craiggriffiths2007\\DarkIsland\\Assets\\LevelBinary\\*.* /G\n", m_Res->local_file_folder);
		fprintf_s(pFile, "xcopy %s\\game-data.db D:\\craiggriffiths2007\\DarkIsland\\Assets\\game-data.db /G\n", m_Res->local_file_folder);
		fprintf_s(pFile, "del %s\\*.bmp\n", m_Res->local_file_folder);
		fclose(pFile);
	}

	// write model info
	sprintf_s(bat_filename, "%s\\model_info.txt", m_Res->local_file_folder);
	fopen_s(&pFile, bat_filename, "w");
	if (pFile != NULL)
	{
		for (int i = 0; i < p_Statics->total_static_models; i++)
		{
			fprintf_s(pFile, "Filename : %s\n\n", m_Res->to_narrow_str(p_Statics->m_static_model[i]->m_filename.c_str()));

			for (Mesh* m : p_Statics->m_static_model[i]->m_mesh)
			{
				fprintf_s(pFile, "Mesh name : %s\n", m_Res->to_narrow_str(m->Name()));

				for (Mesh::Material mat : m->GetMaterials())
				{
					fprintf_s(pFile, "Material name : %s\n", m_Res->to_narrow_str(mat.Name.c_str()));

					/*
					memcpy(&mat_buffer.Ambient, material.Ambient, sizeof(material.Ambient));
					memcpy(&mat_buffer.Diffuse, material.Diffuse, sizeof(material.Diffuse));
					memcpy(&mat_buffer.Specular, material.Specular, sizeof(material.Specular));
					memcpy(&mat_buffer.Emissive, material.Emissive, sizeof(material.Emissive));
					mat_buffer.SpecularPower = material.SpecularPower;
					mat_buffer.normal_height = material.normal_height;
					mat_buffer.emmit_brightness = material.emmit_brightness;
					*/

					fprintf_s(pFile, "Ambient :\t%f\t%f\t%f\t%f\n", mat.Ambient[0], mat.Ambient[1], mat.Ambient[2], mat.Ambient[3]);
					fprintf_s(pFile, "Diffuse :\t%f\t%f\t%f\t%f\n", mat.Diffuse[0], mat.Diffuse[1], mat.Diffuse[2], mat.Diffuse[3]);
					fprintf_s(pFile, "Specular:\t%f\t%f\t%f\t%f\n", mat.Specular[0], mat.Specular[1], mat.Specular[2], mat.Specular[3]);
					fprintf_s(pFile, "Emmisive:\t%f\t%f\t%f\t%f\n", mat.Emissive[0], mat.Emissive[1], mat.Emissive[2], mat.Emissive[3]);
					fprintf_s(pFile, "Spec Pow:\t%f\n", mat.SpecularPower);
					fprintf_s(pFile, "Norm Hei:\t%f\n", mat.normal_height);
					fprintf_s(pFile, "EmmitBri:\t%f\n", mat.emmit_brightness);
				}
			}
			fprintf_s(pFile, "\n\n");
		}

		fclose(pFile);
	}

	m_Res->SaveLighting();

	m_Res->SaveDatabaseLevelInfo();

	Windows::System::Launcher::LaunchFolderAsync(Windows::Storage::ApplicationData::Current->LocalFolder);
}

task<void> LevelEdit::LoadModels()
{
	std::vector<task<void>> tasks;

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Res, L"Assets\\Compiled\\flag-pole.cmo", L"", L"", m_Marker->m_mesh).then([this]()
	{
		//m_static_model[1]->type = 0;
		//m_static_model[1]->physics_shape = 0;
	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Res, L"Assets\\Compiled\\flag-pole.cmo", L"", L"", m_MarkerPath->m_mesh).then([this]()
	{
		//m_static_model[1]->type = 0;
		//m_static_model[1]->physics_shape = 0;
	}));

	return when_all(tasks.begin(), tasks.end());
}

task<void> LevelEdit::LoadTextures()
{
	BasicLoader^ loader = ref new BasicLoader(m_deviceResources->GetD3DDevice());

	std::vector<task<void>> tasks;

	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\gemred.dds", nullptr, m_Marker->GetMaterialTexture(L"material", 0, 0)));
	tasks.push_back(loader->LoadTextureAsync("Assets\\Compiled\\gemblue.dds", nullptr, m_MarkerPath->GetMaterialTexture(L"material", 0, 0)));

	return when_all(tasks.begin(), tasks.end());
}

void LevelEdit::ClearPaths()
{
	bClearPaths = true;
}

void LevelEdit::MakePath()
{
	// XMLoadFloat3(&XMFLOAT3((float)j*2.0f - left_right_walls + 1.0f, height_map[j][i], (float)i*2.0f - front_back_walls + 1.0f)));
	bMakePath = true;
}

void LevelEdit::Update(float timeDelta, float timeTotal)
{
	int i;

	btVector3 eye_pos = btVector3(m_Res->m_Camera->Eye().x, m_Res->m_Camera->Eye().y, m_Res->m_Camera->Eye().z);

	btVector3 look_dir = btVector3(m_Res->m_Camera->LookAt().x - m_Res->m_Camera->Eye().x,
		m_Res->m_Camera->LookAt().y - m_Res->m_Camera->Eye().y,
		m_Res->m_Camera->LookAt().z - m_Res->m_Camera->Eye().z).normalize();

	if (bCamLookPosition == false)
	{
		bTerrainHit = false;
		for (i = 0; i < 220; i++)
		{
			t_height = p_Level->GetTerrainHeight(eye_pos.getX(), eye_pos.getZ());
			if (t_height > eye_pos.getY())
			{
				if (bSnapGridEdit == true)
				{
					if (design_spacing > 1)
					{
						eye_pos.setX(((int)eye_pos.getX() / design_spacing)*design_spacing);// +(int)pos.x % design_spacing;
						eye_pos.setZ(((int)eye_pos.getZ() / design_spacing)*design_spacing);// (int)pos.z +(int)pos.z % design_spacing;
					}
					else
					{
						eye_pos.setX((int)eye_pos.getX());// +(int)pos.x % 2;
						eye_pos.setZ((int)eye_pos.getZ());// +(int)pos.z % 2;
					}
				}

				t_height = p_Level->GetTerrainHeight(eye_pos.getX(), eye_pos.getZ());
				hit_point.x = eye_pos.getX();
				if (m_controller->KeyState(Windows::System::VirtualKey::Control, false) == false)
				{
					hit_point.y = t_height;
				}
				hit_point.z = eye_pos.getZ();
				bTerrainHit = true;
				break;
			}
			else
			{
				eye_pos += look_dir;
			}
		}

		if (m_controller->KeyState(Windows::System::VirtualKey::Control, false) == false)
		{
			btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(hit_point.x, camera_height, hit_point.z), btVector3(hit_point.x, hit_point.y, hit_point.z));

			m_Res->m_Physics.m_dynamicsWorld->rayTest(btVector3(hit_point.x, camera_height, hit_point.z), btVector3(hit_point.x, hit_point.y, hit_point.z), RayCallback);

			if (RayCallback.hasHit())
			{
				hit_point.y = RayCallback.m_hitPointWorld.getY();
			}

			if (model_pointer_id < p_Statics->total_static_models + 1)
			{
			}
			else
			{
				if (model_pointer_id < max_model_pointer_id + 2)
				{
					if (model_pointer_id < max_model_pointer_id + 2 && p_Stuff->m_stuff_model[model_pointer_id - p_Statics->total_static_models - 1]->m_mesh[0] != nullptr)
					{
						hit_point.y += p_Stuff->m_stuff_model[model_pointer_id - p_Statics->total_static_models - 1]->OverallMaxY;// m_mesh[0]->Extents().MaxY;
					}
				}
			}
		}
	}
	else
	{
		btVector3 new_point = eye_pos + look_dir * 4.0f;
		if (bSnapGridEdit == true)
		{
			if (design_spacing > 1)
			{
				new_point.setX(((int)new_point.getX() / design_spacing)*design_spacing);// +(int)pos.x % design_spacing;
				new_point.setZ(((int)new_point.getZ() / design_spacing)*design_spacing);// (int)pos.z +(int)pos.z % design_spacing;
			}
			else
			{
				new_point.setX((int)new_point.getX());// +(int)pos.x % 2;
				new_point.setZ((int)new_point.getZ());// +(int)pos.z % 2;
			}
		}
		hit_point = XMFLOAT3(new_point.getX(), new_point.getY(), new_point.getZ());
	}

	pos = hit_point;

	if (m_controller->KeyState(Windows::System::VirtualKey::NumberPad1, true) == true)
		ground_texture_index = 0;
	if (m_controller->KeyState(Windows::System::VirtualKey::NumberPad2, true) == true)
		ground_texture_index = 1;
	if (m_controller->KeyState(Windows::System::VirtualKey::NumberPad3, true) == true)
		ground_texture_index = 2;
	if (m_controller->KeyState(Windows::System::VirtualKey::NumberPad4, true) == true)
		ground_texture_index = 3;
	if (m_controller->KeyState(Windows::System::VirtualKey::NumberPad5, true) == true)
		ground_texture_index = 4;
	if (m_controller->KeyState(Windows::System::VirtualKey::NumberPad6, true) == true)
		ground_texture_index = 5;
	if (m_controller->KeyState(Windows::System::VirtualKey::NumberPad7, true) == true)
		ground_texture_index = 6;
	if (m_controller->KeyState(Windows::System::VirtualKey::NumberPad8, true) == true)
		ground_texture_index = 7;

	if (m_controller->KeyState(Windows::System::VirtualKey::NumberPad0, true) == true)
	{
		p_Level->SetAllLevelTexture(ground_texture_index);
		p_Level->UpdateTerrain(true).wait();
		p_Level->UpdateVertexBuffers(true);
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::PageUp, false) == true)
	{
		yaw_angle += 0.05f;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::PageDown, false) == true)
	{
		yaw_angle -= 0.05f;
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::Home, true) == true)
	{
		yaw_angle_snap += 1;
		if (yaw_angle_snap > 3)
			yaw_angle_snap = 0;

		yaw_angle = (float)yaw_angle_snap * (3.14159 *0.5f);
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::L, true) == true)
	{
		p_Level->FlattenTerrain();
		p_Level->BakePinch(true);
		SetStaticsHeightToTerrrain();
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::J, true) == true)
	{
		p_Statics->Reset();
		p_Stuff->Reset();
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::K, true) == true)
	{
		p_Level->MakeNewTerrain((rand() % 100)*0.2f, 1.0f, (rand() % 100)*0.2f, (rand() % 100)*0.1f);
		p_Level->BakePinch(true);
		SetStaticsHeightToTerrrain();
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::C, true) == true)
	{
		m_CameraPath->AddPoint(m_Res->m_Camera->Eye(), m_Res->m_Camera->LookAt());
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::V, true) == true)
	{
		m_CameraPath->ClearPath();
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Back, true) == true)
	{
		p_Level->m_Paths->ClearPath(0);
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Enter, true) == true)
	{
		p_Level->ClearPlaneUpdate();

		if (bLoopPath == true)
		{
			if (p_Level->m_Paths->GetTotalPoints(0) > 3)
			{
				for (int i = 0; i < p_Level->m_Paths->GetTotalPoints(0); i++)
				{
					for (int j = 0; j < 100; j++)
					{
						XMFLOAT3 pos;
						XMStoreFloat3(&pos, p_Level->m_Paths->GetCatRomPoint(0, i, (float)j*0.01f));

						p_Level->UpdateGeneratedTerrainPath(pos.z, pos.y, pos.x, stretch_width, 1, bDipPath);
					}
				}
			}
		}
		else
		{
			if (p_Level->m_Paths->GetTotalPoints(0) > 3)
			{
				for (int i = 1; i < p_Level->m_Paths->GetTotalPoints(0) - 2; i++)
				{
					for (int j = 0; j < 100; j++)
					{
						XMFLOAT3 pos;
						XMStoreFloat3(&pos, p_Level->m_Paths->GetCatRomPoint(0, i, (float)j*0.01f));

						p_Level->UpdateGeneratedTerrainPath(pos.z, pos.y, pos.x, stretch_width, 1, bDipPath);
					}
				}

				PathPoint point1 = p_Level->m_Paths->GetPoints(0).at(0);
				PathPoint point2 = p_Level->m_Paths->GetPoints(0).at(1);
				btVector3 vpoint1 = btVector3(point1.pos.x, point1.pos.y, point1.pos.z);
				btVector3 vpoint2 = btVector3(point2.pos.x, point2.pos.y, point2.pos.z);
				for (int j = 0; j < 100; j++)
				{
					btVector3 lpoint = vpoint1.lerp(vpoint2, (float)j*0.01f);

					p_Level->UpdateGeneratedTerrainPath(lpoint.getZ(), lpoint.getY(), lpoint.getX(), stretch_width, 1, bDipPath);
				}
				point1 = p_Level->m_Paths->GetPoints(0).at(p_Level->m_Paths->GetTotalPoints(0) - 2);
				point2 = p_Level->m_Paths->GetPoints(0).at(p_Level->m_Paths->GetTotalPoints(0) - 1);
				vpoint1 = btVector3(point1.pos.x, point1.pos.y, point1.pos.z);
				vpoint2 = btVector3(point2.pos.x, point2.pos.y, point2.pos.z);
				for (int j = 0; j < 100; j++)
				{
					btVector3 lpoint = vpoint1.lerp(vpoint2, (float)j*0.01f);

					p_Level->UpdateGeneratedTerrainPath(lpoint.getZ(), lpoint.getY(), lpoint.getX(), stretch_width, 1, bDipPath);
				}
			}
			else
			{
				if (p_Level->m_Paths->GetTotalPoints(0) > 2)
				{
					PathPoint point1 = p_Level->m_Paths->GetPoints(0).at(0);
					PathPoint point2 = p_Level->m_Paths->GetPoints(0).at(1);
					btVector3 vpoint1 = btVector3(point1.pos.x, point1.pos.y, point1.pos.z);
					btVector3 vpoint2 = btVector3(point2.pos.x, point2.pos.y, point2.pos.z);
					for (int j = 0; j < 100; j++)
					{
						btVector3 lpoint = vpoint1.lerp(vpoint2, (float)j*0.01f);

						p_Level->UpdateGeneratedTerrainPath(lpoint.getZ(), lpoint.getY(), lpoint.getX(), stretch_width, 1, bDipPath);
					}
					point1 = p_Level->m_Paths->GetPoints(0).at(1);
					point2 = p_Level->m_Paths->GetPoints(0).at(2);
					vpoint1 = btVector3(point1.pos.x, point1.pos.y, point1.pos.z);
					vpoint2 = btVector3(point2.pos.x, point2.pos.y, point2.pos.z);
					for (int j = 0; j < 100; j++)
					{
						btVector3 lpoint = vpoint1.lerp(vpoint2, (float)j*0.01f);

						p_Level->UpdateGeneratedTerrainPath(lpoint.getZ(), lpoint.getY(), lpoint.getX(), stretch_width, 1, bDipPath);
					}
				}
				else
				{
					if (p_Level->m_Paths->GetTotalPoints(0) > 1)
					{
						PathPoint point1 = p_Level->m_Paths->GetPoints(0).at(0);
						PathPoint point2 = p_Level->m_Paths->GetPoints(0).at(1);
						btVector3 vpoint1 = btVector3(point1.pos.x, point1.pos.y, point1.pos.z);
						btVector3 vpoint2 = btVector3(point2.pos.x, point2.pos.y, point2.pos.z);
						for (int j = 0; j < 100; j++)
						{
							btVector3 lpoint = vpoint1.lerp(vpoint2, (float)j*0.01f);

							p_Level->UpdateGeneratedTerrainPath(lpoint.getZ(), lpoint.getY(), lpoint.getX(), stretch_width, 1, bDipPath);
						}
					}
				}
			}
		}
		p_Level->BakeAll();
		//p_Level->UpdatePlaneUpdate();
		SetStaticsHeightToTerrrain();

		return;
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::CapitalLock) == true)
	{
		if (pos.x != last_pos.x || pos.z != last_pos.z)
		{
			int i;
			float x_dist = (last_pos.x - pos.x)*0.2f;
			float z_dist = (last_pos.z - pos.z)*0.2f;
			p_Level->ClearPlaneUpdate();

			for (i = 0; i < 5; i++)
			{
				p_Level->UpdateGeneratedTerrainPinch(1, 0.0f, pos.z - (z_dist*(float)i), pos.x - (x_dist*(float)i), fixed_height, stretch_width, true);
			}
			//p_Level->UpdateGeneratedTerrainPinch(1, 0.0f, pos.z, pos.x, fixed_height, stretch_width, true);
			p_Level->BakePinch(false);
			SetStaticsHeightToTerrrain();
		}
	}
	else
	{
		fixed_height = t_height;
	}

	if (pos.x != last_pos.x || pos.z != last_pos.z)
	{
		bColKeyDown = false;
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::X) == true)
	{
		if (bColKeyDown == false)
		{
			bColKeyDown = true;
			p_Level->UpdateGeneratedTerrainTex(pos.z, pos.x, stretch_width, ground_texture_index);
			p_Level->UpdateVertexBuffers(false);
		}
	}
	else
	{
		bColKeyDown = false;
	}

	last_pos = pos;

	if (m_controller->KeyState(Windows::System::VirtualKey::Up) == true)
	{
		p_Level->UpdateGeneratedTerrainPinch(1, 0.0f, pos.z, pos.x, 0.03f, stretch_width, false);
		p_Level->BakePinch(false);
		SetStaticsHeightToTerrrain();
	}
	if (m_controller->KeyState(Windows::System::VirtualKey::Down) == true)
	{
		p_Level->UpdateGeneratedTerrainPinch(1, 0.0f, pos.z, pos.x, -0.03f, stretch_width, false);
		p_Level->BakePinch(false);
		SetStaticsHeightToTerrrain();
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::N, true) == true)
	{
		p_Level->CreateNormals(true);
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Add) == true)
	{
		stretch_width += 0.1f;
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Subtract) == true)
	{
		stretch_width -= 0.1f;
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Number4, true) == true)
	{
		if (bCamLookPosition == false)
		{
			bCamLookPosition = true;
		}
		else
		{
			bCamLookPosition = false;
		}
	}

	if (m_controller->KeyState(Windows::System::VirtualKey::Number5, true) == true)
	{
		if (bSnapGridEdit == false)
		{
			bSnapGridEdit = true;
		}
		else
		{
			bSnapGridEdit = false;
		}
	}

	CG_POINT_LIGHT fire_point_light;
	fire_point_light.ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	fire_point_light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	fire_point_light.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	fire_point_light.pos = XMFLOAT3(hit_point.x, t_height + (stretch_width*0.5), hit_point.z);
	//fire_point_light.radius = m_static_model[static_item.model_index]->light_radius;
	fire_point_light._specular_power = 0.0f;
	fire_point_light.bCastShadows = false;
	fire_point_light.radius = stretch_width * 2.0f;
	fire_point_light.fire_scale = 0.8f;
	//fire_point_light.dist = light_dist;

	m_Res->m_Lights->AddPoint(fire_point_light);
}

void LevelEdit::SetStaticsHeightToTerrrain()
{
	int i;

	for (i = 0; i < p_Statics->m_static.size(); i++)
	{
		if (p_Statics->m_static[i].stuff.bActive == 1)
		{
			float height_diff = (p_Statics->m_static[i].stuff.y + p_Statics->m_static[i].stuff.height_from_terrain) - p_Statics->m_static[i].stuff.model_matrix._24;

			p_Statics->m_static[i].stuff.y = p_Level->GetTerrainHeight(p_Statics->m_static[i].stuff.x, p_Statics->m_static[i].stuff.z);
			p_Statics->m_static[i].stuff.model_matrix._24 = p_Statics->m_static[i].stuff.y + p_Statics->m_static[i].stuff.height_from_terrain;// =
			//p_Statics->m_static[i].model_matrix = m_Resources->MakeMatrix(p_Statics->m_static[i].x, p_Statics->m_static[i].y + p_Statics->m_static[i].height_from_terrain, p_Statics->m_static[i].z, p_Statics->m_static[i].rot_x, p_Statics->m_static[i].rot_y, p_Statics->m_static[i].rot_z, p_Statics->m_static[i].scale);
			//p_Statics->m_static[i].
			for (btRigidBody* b : p_Statics->m_static[i].m_Body)
			{
				btTransform tran = b->getWorldTransform();
				tran.setOrigin(btVector3(tran.getOrigin().getX(), tran.getOrigin().getY() + height_diff, tran.getOrigin().getZ()));
				b->setWorldTransform(tran);
			}
		}
	}

	for (i = 0; i < p_Stuff->m_stuff.size(); i++)
	{
		if (p_Stuff->m_stuff[i].stuff.bActive == 1)
		{
			p_Stuff->m_stuff[i].stuff.info.pos.y = p_Level->GetTerrainHeight(p_Stuff->m_stuff[i].stuff.info.pos.x, p_Stuff->m_stuff[i].stuff.info.pos.z) + p_Stuff->m_stuff[i].stuff.height_from_terrain;
			p_Stuff->m_stuff[i].stuff.model_matrix._24 = p_Stuff->m_stuff[i].stuff.info.pos.y;// +p_Stuff->m_stuff_model[p_Stuff->m_stuff[i].model_index]->model->box_extents.y;// = m_Resources->MakeMatrix(p_Statics->m_static[i].x, p_Statics->m_static[i].y, p_Statics->m_static[i].z, p_Statics->m_static[i].rot_x, p_Statics->m_static[i].rot_y, p_Statics->m_static[i].rot_z, p_Statics->m_static[i].scale);
			btTransform tran = p_Stuff->m_stuff[i].m_Body->getWorldTransform();
			btVector3 origin = tran.getOrigin();
			origin.setY(p_Stuff->m_stuff[i].stuff.model_matrix._24);
			tran.setOrigin(origin);
			p_Stuff->m_stuff[i].m_Body->setWorldTransform(tran);
		}
	}
}

void LevelEdit::Render()
{
	m_Res->m_Camera->m_constantBufferData.model = m_Res->MakeMatrix(hit_point.x, hit_point.y, hit_point.z, yaw_angle, 0.0f, 0.0f, 1.0f);
	m_Res->m_Camera->UpdateConstantBuffer();
	if (model_pointer_id == -2)
	{
		m_MarkerPath->m_mesh[0]->Render(*m_Res);
	}
	else
		if (model_pointer_id == -1 || m_controller->KeyState(Windows::System::VirtualKey::Escape, false) == true)
		{
			m_Marker->m_mesh[0]->Render(*m_Res);
		}
		else
		{
			if (model_pointer_id < p_Statics->total_static_models + 1)
			{
				char ind_str[10];
				char group_str[10];
				sprintf(ind_str, "%d", model_pointer_id);
				sprintf(group_str, "%d", current_item_group);

				m_Res->m_uiControl->SetDat(1, ref new Platform::String(m_Res->GetWC(group_str)) + L" - " + ref new Platform::String(m_Res->GetWC(p_Statics->m_static_model[ptr_statics[model_pointer_id].index]->group)));
				m_Res->m_uiControl->SetDat(2, ref new Platform::String(m_Res->GetWC(ind_str)) + L" - " + ref new Platform::String(m_Res->GetWC(p_Statics->m_static_model[ptr_statics[model_pointer_id].index]->filename)));

				for (Mesh* m : p_Statics->m_static_model[ptr_statics[model_pointer_id].index]->m_mesh)
				{
					m->Render(*m_Res);
				}
			}
			else
			{
				if (model_pointer_id < max_model_pointer_id + 2)
				{
					char ind_str[10];
					char group_str[10];
					sprintf(ind_str, "%d", model_pointer_id - p_Statics->total_static_models - 1);
					sprintf(group_str, "%d", current_item_group);

					m_Res->m_uiControl->SetDat(1, ref new Platform::String(m_Res->GetWC(group_str)) + L" - " + ref new Platform::String(m_Res->GetWC(p_Stuff->m_stuff_model[model_pointer_id - p_Statics->total_static_models - 1]->group)));
					m_Res->m_uiControl->SetDat(2, ref new Platform::String(m_Res->GetWC(ind_str)) + L" - " + ref new Platform::String(m_Res->GetWC(p_Stuff->m_stuff_model[model_pointer_id - p_Statics->total_static_models - 1]->filename)));

					for (Mesh* m : p_Stuff->m_stuff_model[model_pointer_id - p_Statics->total_static_models - 1]->m_mesh)
					{
						m->Render(*m_Res);
					}
				}
			}
		}

	for (int i = 0; i < p_Level->m_Paths->GetTotalPoints(0); i++)
	{
		m_Res->m_Camera->m_constantBufferData.model = m_Res->MakeMatrix(p_Level->m_Paths->GetPoints(0).at(i).pos.x, p_Level->m_Paths->GetPoints(0).at(i).pos.y, p_Level->m_Paths->GetPoints(0).at(i).pos.z, 0.0f, 0.0f, 0.0f, 1.0f);
		m_Res->m_Camera->UpdateConstantBuffer();
		//m_Marker[0]->Render(*m_Resources);
		m_MarkerPath->m_mesh[0]->Render(*m_Res);
	}

	for (int i = 0; i < m_CameraPath->GetTotalPoints(); i++)
	{
		m_Res->m_Camera->m_constantBufferData.model = m_Res->MakeMatrix(m_CameraPath->GetPoints().at(i).eye.x, m_CameraPath->GetPoints().at(i).eye.y, m_CameraPath->GetPoints().at(i).eye.z, 0.0f, 0.0f, 0.0f, 1.0f);
		m_Res->m_Camera->UpdateConstantBuffer();
		//m_Marker[0]->Render(*m_Resources);
		m_MarkerPath->m_mesh[0]->Render(*m_Res);

		m_Res->m_Camera->m_constantBufferData.model = m_Res->MakeMatrix(m_CameraPath->GetPoints().at(i).at.x, m_CameraPath->GetPoints().at(i).at.y, m_CameraPath->GetPoints().at(i).at.z, 0.0f, 0.0f, 0.0f, 1.0f);
		m_Res->m_Camera->UpdateConstantBuffer();
		//m_Marker[0]->Render(*m_Resources);
		m_Marker->m_mesh[0]->Render(*m_Res);
	}

	//m_CameraPath->Render();
}

LevelEdit::~LevelEdit()
{
}