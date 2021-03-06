// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ssr.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/core/ConfigSet.h>
#include <shaders/glsl_cpp_common/Ssr.h>

namespace anki
{

ANKI_REGISTER_CONFIG_OPTION(r_ssrMaxSteps, 64, 1, 2048)
ANKI_REGISTER_CONFIG_OPTION(r_ssrHistoryBlendFactor, 0.3, 0.0, MAX_F64)

Ssr::~Ssr()
{
}

Error Ssr::init(const ConfigSet& cfg)
{
	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize reflection pass");
	}
	return err;
}

Error Ssr::initInternal(const ConfigSet& cfg)
{
	U32 width = m_r->getWidth() / SSR_FRACTION;
	U32 height = m_r->getHeight() / SSR_FRACTION;
	ANKI_R_LOGI("Initializing SSR pass (%ux%u)", width, height);

	// Create RTs
	TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(width,
		height,
		Format::R16G16B16A16_SFLOAT,
		TextureUsageBit::IMAGE_COMPUTE_READ_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		"SSR");
	texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
	m_rt = m_r->createAndClearRenderTarget(texinit);

	// Create shader
	ANKI_CHECK(getResourceManager().loadResource("shaders/Ssr.glslp", m_prog));

	ShaderProgramResourceConstantValueInitList<5> consts(m_prog);
	consts.add("FB_SIZE", UVec2(width, height));
	consts.add("WORKGROUP_SIZE", UVec2(m_workgroupSize[0], m_workgroupSize[1]));
	consts.add("MAX_STEPS", cfg.getNumberU32("r_ssrMaxSteps"));
	consts.add("LIGHT_BUFFER_MIP_COUNT", U32(m_r->getDownscaleBlur().getMipmapCount()));
	consts.add("HISTORY_COLOR_BLEND_FACTOR", cfg.getNumberF32("r_ssrHistoryBlendFactor"));

	ShaderProgramResourceMutationInitList<1> mutators(m_prog);
	mutators.add("VARIANT", 0);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
	m_grProg[0] = variant->getProgram();

	mutators[0].m_value = 1;
	m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
	m_grProg[1] = variant->getProgram();

	return Error::NONE;
}

void Ssr::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;

	// Create RTs
	m_runCtx.m_rt = rgraph.importRenderTarget(m_rt, TextureUsageBit::SAMPLED_FRAGMENT);

	// Create pass
	ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SSR");
	rpass.setWork(
		[](RenderPassWorkContext& rgraphCtx) { static_cast<Ssr*>(rgraphCtx.m_userData)->run(rgraphCtx); }, this, 0);

	rpass.newDependency({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_READ_WRITE});
	rpass.newDependency({m_r->getGBuffer().getColorRt(1), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newDependency({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE});

	TextureSubresourceInfo hizSubresource; // Only first mip
	rpass.newDependency({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE, hizSubresource});

	rpass.newDependency({m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_COMPUTE});
}

void Ssr::run(RenderPassWorkContext& rgraphCtx)
{
	RenderingContext& ctx = *m_runCtx.m_ctx;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_grProg[m_r->getFrameCount() & 1u]);

	// Bind all
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);

	rgraphCtx.bindColorTexture(0, 1, m_r->getGBuffer().getColorRt(1));
	rgraphCtx.bindColorTexture(0, 2, m_r->getGBuffer().getColorRt(2));

	TextureSubresourceInfo hizSubresource; // Only first mip
	rgraphCtx.bindTexture(0, 3, m_r->getDepthDownscale().getHiZRt(), hizSubresource);

	rgraphCtx.bindColorTexture(0, 4, m_r->getDownscaleBlur().getRt());

	rgraphCtx.bindImage(0, 5, m_runCtx.m_rt, TextureSubresourceInfo());

	// Bind uniforms
	SsrUniforms* unis = allocateAndBindUniforms<SsrUniforms*>(sizeof(SsrUniforms), cmdb, 0, 6);
	unis->m_nearPad3 = Vec4(ctx.m_renderQueue->m_cameraNear);
	unis->m_prevViewProjMatMulInvViewProjMat =
		ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_viewProjectionJitter.getInverse();
	unis->m_projMat = ctx.m_matrices.m_projectionJitter;
	unis->m_invProjMat = ctx.m_matrices.m_projectionJitter.getInverse();
	unis->m_normalMat = Mat3x4(ctx.m_matrices.m_view.getRotationPart());

	// Dispatch
	const U32 sizeX = (m_r->getWidth() / SSR_FRACTION + m_workgroupSize[0] - 1) / m_workgroupSize[0];
	const U32 sizeY = (m_r->getHeight() / SSR_FRACTION + m_workgroupSize[1] - 1) / m_workgroupSize[1];
	cmdb->dispatchCompute(sizeX / 2, sizeY, 1);
}

} // end namespace anki
