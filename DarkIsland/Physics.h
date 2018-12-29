#pragma once

#include "../Bullet/src/btBulletDynamicsCommon.h"

///#include "../Bullet/src/BulletCollision/CollisionShapes/btShapeHull.h"

#include "AllStructures.h"

namespace Game
{
	class Physics
	{
	public:
		Physics(void);
		~Physics(void);

		task<void> Update(float timeDelta, float timeTotal);
		btRigidBody* AddPhysicalObject(btCollisionShape* collisionShape, btMotionState* motionState, const btVector3& inertia, ObjInfo* info);

		void RemoveAllObjects(int offset);

		int cur_unique_id;

		/*
		btRigidBody* MakePhysicsSquareplane(ObjInfo* ob_info, float size, float y_offset);

		btRigidBody* MakePhysicsEllipseoidFromFBX(Game::Mesh* p_model, ObjInfo* ob_info, float _scale);
		btRigidBody* MakePhysicsBoxFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale);
		btRigidBody* MakePhysicsCompoundBoxFromFBX(Mesh* p_model, ObjInfo* ob_info, float x, float y, float z, float _scale);
		btRigidBody* MakePhysicsSphereFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale);
		btRigidBody* MakePhysicsCylinderFBX(Mesh* p_model, ObjInfo* ob_info, float _scale);
		btRigidBody* MakePhysicsCylinderExtents(ObjInfo* ob_info, float extent1, float extent2, float extent3, float _scale);
		btRigidBody* MakePhysicsBoxExtents(ObjInfo* ob_info, float extent1, float extent2, float extent3, float _scale);
		btRigidBody* MakePhysicsConvexHullFromFBX(Mesh* p_model, ObjInfo* ob_info, float _scale);
		btRigidBody* MakePhysicsNonColision();
		*/

		std::vector<btRigidBody> m_RigidBodyContact;

		task<void> CheckForCollisions();

		btRigidBody* MakeWallPlane(ObjInfo* info);
		//	void Pause();
		//	void Continue();

		int bCollideTest;
		float high_impulse;

		//private:
		void ClearPhysicsObjects();
		void myTickCallback(btDynamicsWorld *world, btScalar timeStep);

		Physics(const Physics&) {}
		Physics& operator=(const Physics&) { return *this; }

		std::unique_ptr<btBroadphaseInterface> m_broadphase;
		std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
		std::unique_ptr<btCollisionDispatcher> m_dispatcher;
		std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
		std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;

		std::vector<std::unique_ptr<btCollisionShape>> m_shapes;
		std::vector<std::unique_ptr<btRigidBody>> m_rigidBodies;
	};
}