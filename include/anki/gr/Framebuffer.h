// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Texture.h>
#include <cstring>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Framebuffer attachment info.
class Attachment
{
public:
	TexturePtr m_texture;
	U32 m_arrayIndex = 0; ///< For array textures
	U32 m_depth = 0; ///< For 3D textures
	U32 m_faceIndex = 0; ///< For cubemap textures
	U32 m_mipmap = 0;
	PixelFormat m_format;
	AttachmentLoadOperation m_loadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_storeOperation = AttachmentStoreOperation::STORE;
	union
	{
		Array<F32, 4> m_colorf = {{0.0, 0.0, 0.0, 0.0}};
		Array<I32, 4> m_colori;
		Array<U32, 4> m_coloru;
		struct
		{
			F32 m_depth;
			I32 m_stencil;
		} m_depthStencil;
	} m_clearValue;

	Attachment() = default;

	Attachment(const Attachment& b)
	{
		operator=(b);
	}

	~Attachment() = default;

	Attachment& operator=(const Attachment& b)
	{
		m_texture = b.m_texture;
		m_arrayIndex = b.m_arrayIndex;
		m_depth = b.m_depth;
		m_faceIndex = b.m_faceIndex;
		m_mipmap = b.m_mipmap;
		m_format = b.m_format;
		m_loadOperation = b.m_loadOperation;
		m_storeOperation = b.m_storeOperation;
		memcpy(&m_clearValue, &b.m_clearValue, sizeof(m_clearValue));
		return *this;
	}
};

/// Framebuffer initializer.
class FramebufferInitializer
{
public:
	Array<Attachment, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
	U32 m_colorAttachmentsCount = 0;
	Attachment m_depthStencilAttachment;

	FramebufferInitializer() = default;

	FramebufferInitializer(const FramebufferInitializer& b)
	{
		operator=(b);
	}

	~FramebufferInitializer() = default;

	FramebufferInitializer& operator=(const FramebufferInitializer& b)
	{
		for(U i = 0; i < b.m_colorAttachmentsCount; i++)
		{
			m_colorAttachments[i] = b.m_colorAttachments[i];
		}

		m_colorAttachmentsCount = b.m_colorAttachmentsCount;
		m_depthStencilAttachment = b.m_depthStencilAttachment;
		return *this;
	}
};

/// GPU framebuffer.
class Framebuffer : public GrObject
{
public:
	/// Construct.
	Framebuffer(GrManager* manager);

	/// Destroy.
	~Framebuffer();

	/// Access the implementation.
	FramebufferImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Create.
	void create(const FramebufferInitializer& init);

private:
	UniquePtr<FramebufferImpl> m_impl;
};
/// @}

} // end namespace anki
