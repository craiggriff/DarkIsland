#ifndef BT_MY_MOTION_STATE_H
#define BT_MY_MOTION_STATE_H

#include "../Bullet/src/LinearMath/btMotionState.h"
//#include "../Bullet/src/btBulletDynamicsCommon.h"
///The btDefaultMotionState provides a common implementation to synchronize world transforms with offsets.

namespace Game
{
	ATTRIBUTE_ALIGNED16(struct)	btMyMotionState : public btMotionState
	{
		btTransform m_graphicsWorldTrans;
	btTransform	m_centerOfMassOffset;
	btTransform m_startWorldTrans;
	void*		m_userPointer;

	BT_DECLARE_ALIGNED_ALLOCATOR();

	btMyMotionState(const btTransform& startTrans = btTransform::getIdentity(),const btTransform& centerOfMassOffset = btTransform::getIdentity())
		: m_graphicsWorldTrans(startTrans),
		m_centerOfMassOffset(centerOfMassOffset),
		m_startWorldTrans(startTrans),
		m_userPointer(0)

	{
	}

	///synchronizes world transform from user to physics
	virtual void	getWorldTransform(btTransform& centerOfMassWorldTrans) const
	{
		centerOfMassWorldTrans = m_centerOfMassOffset.inverse() * m_graphicsWorldTrans;
	}

	///synchronizes world transform from physics to user
	///Bullet only calls the update of worldtransform for active objects
	virtual void	setWorldTransform(const btTransform& centerOfMassWorldTrans)
	{
		m_graphicsWorldTrans = centerOfMassWorldTrans * m_centerOfMassOffset;
	}
	};
}
#endif //BT_MY_MOTION_STATE_H
