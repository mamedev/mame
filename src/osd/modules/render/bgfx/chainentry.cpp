#include "emu.h"

#include <bgfx/bgfx.h>

#include "effect.h"
#include "texture.h"
#include "target.h"
#include "chainentry.h"

bgfx_chain_entry::bgfx_chain_entry(std::string name, bgfx_effect* effect, std::vector<bgfx_input_pair>& inputs, bgfx_target* output)
	: m_name(name)
	, m_effect(effect)
	, m_output(output)
{
	for (bgfx_input_pair input : inputs)
	{
		m_inputs.push_back(input);
	}
}

bgfx_chain_entry::~bgfx_chain_entry()
{
}

void bgfx_chain_entry::submit(render_primitive* prim, int view)
{
	for (bgfx_input_pair input : m_inputs)
	{
		input.bind(m_effect);
	}
	bgfx::setViewFrameBuffer(view, m_output->target());
	m_effect->submit(view);
}
