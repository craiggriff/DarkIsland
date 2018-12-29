#pragma once

#include "AllResources.h"
#include "meshmodel.h"
#include "Stuff.h"
namespace Game
{
	typedef struct
	{
		XMFLOAT3 pos;
		XMFLOAT3 vel;
		int model_type;
	} ball_t;

	class GunBall
	{
	public:
		GunBall(AllResources* p_Resources, Stuff* pp_Stuff);

		void CreateOne(XMFLOAT3 pos, XMFLOAT3 vel);

		void Render();

		void UpdatePhysics();

		bool IsGunBall(btRigidBody* rb);

		std::vector<ball_t> m_newballs;

		MeshModel* m_meshModel;

		std::vector<btRigidBody*> m_Body;

		Stuff* p_Stuff;

		int model_type;

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Res;
	};
}