#pragma once
#include "AllResources.h"
//#include "MeshModel.h"
#include "mesh.h"

namespace Game
{
	class SimpleMotion2 : btMotionState
	{
	public:
		SimpleMotion2();
		~SimpleMotion2();

		void getWorldTransform(btTransform &worldTransform) const;
		void setWorldTransform(const btTransform &worldTransform);

		btTransform m_initialTransform;
	};


	class MeshTools
	{
	public:
		MeshTools(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Physics* phys);
		~MeshTools();

		btRigidBody* MakePhysicsSquareplane(ObjInfo* ob_info, float size, float y_offset);

		//btRigidBody*  MeshTools::MakePhysicsBoxFromMeshModel(MeshModel* p_model, ObjInfo* ob_info, float _scale);


		btRigidBody* MakePhysicsEllipseoidFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale);
		btRigidBody* MakePhysicsBoxFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale);
		btRigidBody* MakePhysicsCompoundBoxFromFBX(Mesh* p_model, ObjInfo* ob_info, float x, float y, float z, float _scale);
		btRigidBody* MakePhysicsSphereFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale);
		btRigidBody* MakePhysicsCylinderFBX(Mesh* p_model, ObjInfo* ob_info, float _scale);
		btRigidBody* MakePhysicsCylinderExtents(ObjInfo* ob_info, float extent1, float extent2, float extent3, float _scale);
		btRigidBody* MakePhysicsBoxExtents(ObjInfo* ob_info, float extent1, float extent2, float extent3, float _scale);
		btRigidBody* MakePhysicsConvexHullFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale);
		btRigidBody* MakePhysicsNonColision();

		XMFLOAT4X4 GetMatrix(btRigidBody* p_body);
		XMFLOAT4X4 GetMatrix(btTransform* p_body);

		SimpleMotion2* m_motion;

		Physics* p_Phys;
	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
	};

}