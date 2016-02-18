#include "target.h"

bgfx_target::bgfx_target(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, bool filter)
	: bgfx_texture(name, format, width, height, nullptr, BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP | (filter ? 0 : (BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT)))
{
	m_target = bgfx::createFrameBuffer(1, &m_handle, false);
}

bgfx_target::~bgfx_target()
{
	bgfx::destroyFrameBuffer(m_target);
}
