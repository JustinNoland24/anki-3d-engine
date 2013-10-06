#include "anki/event/MoveEvent.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/MoveComponent.h"
#include "anki/util/Functions.h"

namespace anki {

//==============================================================================
MoveEvent::MoveEvent(EventManager* manager, F32 startTime, F32 duration,
	SceneNode* node, const MoveEventData& data)
	: Event(manager, startTime, duration, node, EF_NONE)
{
	ANKI_ASSERT(node);
	*static_cast<MoveEventData*>(this) = data;

	originalPos =
		node->getMoveComponent()->getLocalTransform().getOrigin();

	for(U i = 0; i < 3; i++)
	{
		newPos[i] = randRange(posMin[i], posMax[i]);
	}
	newPos += node->getMoveComponent()->getLocalTransform().getOrigin();
}

//==============================================================================
void MoveEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	SceneNode* node = getSceneNode();
	ANKI_ASSERT(node);

	Transform trf = node->getMoveComponent()->getLocalTransform();

	F32 factor = sin(getDelta(crntTime) * getPi<F32>());

	trf.getOrigin() = interpolate(originalPos, newPos, factor);

	node->getMoveComponent()->setLocalTransform(trf);
}

} // end namespace anki
