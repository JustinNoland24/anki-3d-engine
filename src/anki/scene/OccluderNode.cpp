// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/OccluderNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/OccluderComponent.h>
#include <anki/resource/MeshLoader.h>

namespace anki
{

/// Feedback component.
class OccluderNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			OccluderNode& mnode = static_cast<OccluderNode&>(node);
			mnode.onMoveComponentUpdate(move);
		}

		return Error::NONE;
	}
};

OccluderNode::~OccluderNode()
{
	m_vertsL.destroy(getAllocator());
	m_vertsW.destroy(getAllocator());
}

Error OccluderNode::init(const CString& meshFname)
{
	// Load mesh
	MeshLoader loader(&getSceneGraph().getResourceManager());
	ANKI_CHECK(loader.load(meshFname));

	const U32 indexCount = loader.getHeader().m_totalIndexCount;
	m_vertsL.create(getAllocator(), indexCount);
	m_vertsW.create(getAllocator(), indexCount);

	DynamicArrayAuto<Vec3> positions(getAllocator());
	DynamicArrayAuto<U32> indices(getAllocator());
	ANKI_CHECK(loader.storeIndicesAndPosition(indices, positions));

	for(U32 i = 0; i < indices.getSize(); ++i)
	{
		m_vertsL[i] = positions[indices[i]];
	}

	// Create the components
	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	newComponent<OccluderComponent>();

	return Error::NONE;
}

void OccluderNode::onMoveComponentUpdate(MoveComponent& movec)
{
	const Transform& trf(movec.getWorldTransform());
	U32 count = m_vertsL.getSize();
	while(count--)
	{

		m_vertsW[count] = trf.transform(m_vertsL[count]);
	}

	getComponent<OccluderComponent>().setVertices(&m_vertsW[0], m_vertsW.getSize(), sizeof(m_vertsW[0]));
}

} // end namespace anki
