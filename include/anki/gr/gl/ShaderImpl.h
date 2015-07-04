// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/gr/Shader.h"
#include "anki/gr/gl/GlObject.h"

namespace anki {

// Forward
class CString;
class String;

/// @addtogroup opengl
/// @{

/// Shader program. It only contains a single shader and it can be combined
/// with other programs in a program pipiline.
class ShaderImpl: public GlObject
{
public:
	GLenum m_glType = 0;
	ShaderType m_type;

	ShaderImpl(GrManager* manager)
		: GlObject(manager)
	{}

	~ShaderImpl();

	/// Create the shader.
	/// @param shaderType The type of the shader in the program
	/// @param source The shader's source
	ANKI_USE_RESULT Error create(ShaderType shaderType, const CString& source);

private:
	void handleError(String& src);
};
/// @}

} // end namespace anki

