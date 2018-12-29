#pragma once

#include "AllResources.h"

namespace Game
{
	class CameraPath
	{
	public:
		CameraPath(AllResources* p_Resources);

		void AddPoint(XMFLOAT3 eye, XMFLOAT3 at);

		XMFLOAT3 GetCatRomEye();
		XMFLOAT3 GetCatRomAt();

		XMFLOAT3 GetCatRomPoint(double point_pos);

		std::vector<CameraPathPoint> GetPoints();

		bool SaveBinary(int level);
		concurrency::task<void> LoadBinary(int level);
		void ClearPath();
		void Reset();

		void Update(float timeDelta, float timeTotal);
		void UpdateRollerCoaster(float timeDelta, float timeTotal);

		int GetTotalPoints()
		{
			return camera_path.size();
		}

	private:

		double current_cam_pos;
		double current_cam_pos_to;
		double current_cam_pos_from;
		//std::vector<TerrainPath> terrain_path;

		std::vector<CameraPathPoint> camera_path;

		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Res;
	};
}