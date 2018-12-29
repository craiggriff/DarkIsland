#ifndef FilterableVehicleRaycaster_H
#define FilterableVehicleRaycaster_H
//#include "../Bullet/src/LinearMath/btVector3.h"
//#include "../Bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h"
//#include "../Bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h"
//#include "../Bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h"

#include "Physics.h"

namespace Game
{

	class FilterableVehicleRaycaster : public btDefaultVehicleRaycaster {
	protected:
		btDynamicsWorld* m_testWorld;
		short m_collisionFilterMask;
		short m_collisionFilterGroup;
	public:
		FilterableVehicleRaycaster(btDynamicsWorld* world)
			:btDefaultVehicleRaycaster(world),
			m_testWorld(world),
			m_collisionFilterMask(btBroadphaseProxy::AllFilter),
			m_collisionFilterGroup(btBroadphaseProxy::DefaultFilter) {};
		void setCollisionFilterMask(short collisionFilterMask);
		void setCollisionFilterGroup(short collisionFilterGroup);
		void* castRay(const btVector3& from, const btVector3& to, btVehicleRaycasterResult& result);
	};
}

#endif // FilterableVehicleRaycaster_H