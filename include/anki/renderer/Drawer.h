#ifndef ANKI_RENDERER_DRAWER_H
#define ANKI_RENDERER_DRAWER_H

#include "anki/util/StdTypes.h"
#include "anki/resource/PassLevelKey.h"

namespace anki {

class PassLevelKey;
class Renderer;
class Frustumable;
class SceneNode;
class ShaderProgram;
class Renderable;

/// It includes all the functions to render a Renderable
class RenderableDrawer
{
public:
	static const U UNIFORM_BLOCK_MAX_SIZE = 256;

	enum RenderingStage
	{
		RS_MATERIAL,
		RS_BLEND
	};

	/// The one and only constructor
	RenderableDrawer(Renderer* r_)
		: r(r_)
	{}

	void prepareDraw()
	{}

	void render(
		SceneNode& frsn,
		RenderingStage stage, 
		Pass pass, 
		SceneNode& renderableSceneNode,
		U32* subSpatialIndices,
		U subSpatialIndicesCount);

private:
	Renderer* r;

	void setupShaderProg(
		const PassLevelKey& key,
		const Frustumable& fr,
		const ShaderProgram& prog,
		Renderable& renderable,
		U32* subSpatialIndices,
		U subSpatialIndicesCount);
};

} // end namespace anki

#endif
