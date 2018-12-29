#include "pch.h"

#include "CameraPath.h"

using namespace Game;

CameraPath::CameraPath(AllResources* p_Resources)
{
	m_Res = p_Resources;
	m_deviceResources = m_Res->m_deviceResources;

	camera_path.clear();

	current_cam_pos_from = 0.0f;
	current_cam_pos = current_cam_pos_from + 0.01f;// (timeDelta*10.00f);

	current_cam_pos_to = current_cam_pos_from + 0.02f;// (timeDelta*20.00f);
}

concurrency::task<void> CameraPath::LoadBinary(int level)
{
	return concurrency::create_task([this, level]()
	{
		int i, total;
		CameraPathPoint path;

		Reset();
		camera_path.clear();
		//return true;

		char info_filename[140];

		if (m_Res->bContentFolder == false)
		{
			sprintf_s(info_filename, "%s\\LevelBinary\\CamPath%d.bmp", m_Res->local_file_folder, level);
		}
		else
		{
			sprintf_s(info_filename, "Assets\\LevelBinary\\CamPath%d.bmp", level);
		}

		FILE * pFile;

		fopen_s(&pFile, info_filename, "rb");
		if (pFile != NULL)
		{
			fread_s(&total, sizeof(int), sizeof(int), 1, pFile);
			for (i = 0; i < total; i++)
			{
				fread_s(&path, sizeof(CameraPathPoint), sizeof(CameraPathPoint), 1, pFile);
				camera_path.push_back(path);
			}
			fclose(pFile);

			return;
		}
		else
		{
			return;
		}
	});
}

bool CameraPath::SaveBinary(int level)
{
	int i;
	char info_filename[140];
	FILE * pFile;

	sprintf_s(info_filename, "%s\\CamPath%d.bmp", m_Res->local_file_folder, level);

	fopen_s(&pFile, info_filename, "wb");
	if (pFile != NULL)
	{
		int total = 0;
		for (CameraPathPoint s : camera_path)
		{
			total++;
		}
		fwrite(&total, sizeof(int), 1, pFile);
		for (CameraPathPoint s : camera_path)
		{
			fwrite(&s, sizeof(CameraPathPoint), 1, pFile);
		}
		fclose(pFile);
	}
	else
	{
		return false;
	}

	return true;
}

void CameraPath::Reset()
{
	current_cam_pos = 0;
}

void CameraPath::Update(float timeDelta, float timeTotal)
{
	//UpdateRollerCoaster(timeDelta, timeTotal);
	///*
	current_cam_pos += (timeDelta*0.25f);

	if ((int)current_cam_pos > camera_path.size() - 1)
		current_cam_pos -= (float)camera_path.size();

	m_Res->m_Camera->Up(XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_Res->m_Camera->LookAt(GetCatRomAt());
	m_Res->m_Camera->Eye(GetCatRomEye());
	//*/
}

void CameraPath::UpdateRollerCoaster(float timeDelta, float timeTotal)
{
	float adding = (timeDelta*0.25f);
	current_cam_pos_from += adding;

	current_cam_pos = current_cam_pos_from + 0.1f;

	current_cam_pos_to = current_cam_pos_from + 0.2f;

	if ((int)current_cam_pos_from > camera_path.size()-1)
		current_cam_pos_from -= (float)camera_path.size();

	if ((int)current_cam_pos > camera_path.size()-1)
		current_cam_pos -= (float)camera_path.size();

	if ((int)current_cam_pos_to > camera_path.size()-1)
		current_cam_pos_to -= (float)camera_path.size();

	XMFLOAT3 cam_from = GetCatRomPoint(current_cam_pos_from);
	XMFLOAT3 cam_at = GetCatRomPoint(current_cam_pos);
	XMFLOAT3 cam_to = GetCatRomPoint(current_cam_pos_to);


	XMVECTOR vec_cam_from = XMLoadFloat3(&cam_from);
	XMVECTOR vec_cam_at = XMLoadFloat3(&cam_at);
	XMVECTOR vec_cam_to = XMLoadFloat3(&cam_to);


	//XMStoreFloat3(&vec_cam_at, cam_at)

	m_Res->m_Camera->Eye(cam_at);
	m_Res->m_Camera->LookAt(cam_to);

	

	// calculate the normal
	XMFLOAT3 normvec;
	XMFLOAT3 gravityvec=XMFLOAT3(0.0f,-1.0f,0.0f);
	
	XMStoreFloat3(&normvec, XMVector3Cross(vec_cam_at - vec_cam_from, vec_cam_to - vec_cam_from));

	m_Res->m_Camera->Up(XMFLOAT3(0.0f, 1.0f, 0.0f));
	//m_Res->m_Camera->Up(XMFLOAT3(normvec.z, 1.0f - normvec.y, -normvec.x));

	//m_Res->m_Camera->LookAt(GetCatRomAt());
	//m_Res->m_Camera->Eye(GetCatRomEye());
}

XMFLOAT3 CameraPath::GetCatRomPoint(double point_pos)
{
	static XMFLOAT3 eye;

	if (camera_path.size() < 3)
		return XMFLOAT3(0.0f, 0.0f, 0.0f);

	int _point_index = static_cast<int>(point_pos);
	float place_in_quad_vector = point_pos - static_cast<float>(_point_index);
	if (place_in_quad_vector > 1.0f)
	{
		exit(0);
	};

	int space_at1 = _point_index - 1;
	int space_at2 = _point_index;
	int space_at3 = _point_index + 1;
	int space_at4 = _point_index + 2;

	if (space_at1 < 0)
		space_at1 += camera_path.size();

	if (space_at1 > camera_path.size() - 1)
		space_at1 -= camera_path.size();

	if (space_at2 > camera_path.size() - 1)
		space_at2 -= camera_path.size();

	if (space_at3 > camera_path.size() - 1)
		space_at3 -= camera_path.size();

	if (space_at4 > camera_path.size() - 1)
		space_at4 -= camera_path.size();



	XMStoreFloat3(&eye, XMVectorCatmullRom(XMLoadFloat3(&camera_path.at(space_at1).eye),
		XMLoadFloat3(&camera_path.at(space_at2).eye),
		XMLoadFloat3(&camera_path.at(space_at3).eye),
		XMLoadFloat3(&camera_path.at(space_at4).eye),
		place_in_quad_vector));

	return eye;
}

void CameraPath::AddPoint(XMFLOAT3 eye, XMFLOAT3 at)
{
	CameraPathPoint pp;
	pp.eye = eye;
	pp.at = at;

	camera_path.push_back(pp);
}

std::vector<CameraPathPoint> CameraPath::GetPoints()
{
	return camera_path;
}

void CameraPath::ClearPath()
{
	camera_path.clear();
}



XMFLOAT3 CameraPath::GetCatRomEye()
{
	static XMFLOAT3 eye;

	if (camera_path.size() < 3)
		return XMFLOAT3(0.0f, 0.0f, 0.0f);

	int _point_index = static_cast<int>(current_cam_pos);
	float place_in_quad_vector = current_cam_pos - static_cast<float>(_point_index);
	//if (place_in_quad_vector > 1.0f){exit(0); };

	int space_at1 = _point_index - 1;
	int space_at2 = _point_index;
	int space_at3 = _point_index + 1;
	int space_at4 = _point_index + 2;

	if (space_at1 < 0)
		space_at1 = camera_path.size() - 1;

	if (space_at3 > camera_path.size() - 1)
		space_at3 -= camera_path.size();

	if (space_at4 > camera_path.size() - 1)
		space_at4 -= camera_path.size();

	XMStoreFloat3(&eye, XMVectorCatmullRom(XMLoadFloat3(&camera_path.at(space_at1).eye),
		XMLoadFloat3(&camera_path.at(space_at2).eye),
		XMLoadFloat3(&camera_path.at(space_at3).eye),
		XMLoadFloat3(&camera_path.at(space_at4).eye),
		place_in_quad_vector));

	return eye;
}

XMFLOAT3 CameraPath::GetCatRomAt()
{
	static XMFLOAT3 at;

	if (camera_path.size() < 3)
		return XMFLOAT3(0.0f, 0.0f, 0.0f);

	int _point_index = static_cast<int>(current_cam_pos);
	float place_in_quad_vector = current_cam_pos - static_cast<float>(_point_index);

	int space_at1 = _point_index - 1;
	int space_at2 = _point_index;
	int space_at3 = _point_index + 1;
	int space_at4 = _point_index + 2;

	if (space_at1 < 0)
		space_at1 = camera_path.size() - 1;

	if (space_at3 > camera_path.size() - 1)
		space_at3 -= camera_path.size();

	if (space_at4 > camera_path.size() - 1)
		space_at4 -= camera_path.size();
	//XMVectorCatmullRom()
	XMStoreFloat3(&at, XMVectorCatmullRom(XMLoadFloat3(&camera_path.at(space_at1).at),
		XMLoadFloat3(&camera_path.at(space_at2).at),
		XMLoadFloat3(&camera_path.at(space_at3).at),
		XMLoadFloat3(&camera_path.at(space_at4).at),
		place_in_quad_vector));

	return at;
}