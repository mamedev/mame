#include "emu.h"

#include <bgfx/bgfx.h>

#include "effect.h"
#include "texture.h"
#include "target.h"
#include "pass.h"

bgfx_pass::bgfx_pass(std::string name, bgfx_effect* effect, std::vector<bgfx_texture*>& inputs, bgfx_target* output)
	: m_name(name)
	, m_effect(effect)
	, m_output(output)
{
	for (bgfx_texture* input : inputs)
	{
		m_inputs.push_back(input);
	}
}

bgfx_pass::~bgfx_pass()
{
}

void bgfx_pass::submit(render_primitive* prim, int view)
{
	for (int index = 0; index < m_inputs.size(); index++)
	{
		bgfx::setTexture(index, m_effect->uniform(m_inputs[index]->name())->handle(), m_inputs[index]->handle());
	}
	bgfx::setViewFrameBuffer(view, m_output->target());
	m_effect->submit(view);
}
