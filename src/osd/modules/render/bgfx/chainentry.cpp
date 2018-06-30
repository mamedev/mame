// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainentry.cpp - BGFX shader post-processing node
//
//  Represents a single entry in a list of post-processing
//  passes to be applied to a screen quad or buffer.
//
//============================================================

#include "emu.h"

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include "chainentry.h"

#include "effect.h"
#include "clear.h"
#include "texture.h"
#include "target.h"
#include "entryuniform.h"
#include "texturemanager.h"
#include "vertex.h"
#include "suppressor.h"

#include "render.h"


bgfx_chain_entry::bgfx_chain_entry(std::string name, bgfx_effect* effect, clear_state* clear, std::vector<bgfx_suppressor*> suppressors, std::vector<bgfx_input_pair*> inputs, std::vector<bgfx_entry_uniform*> uniforms, target_manager& targets, std::string output)
	: m_name(name)
	, m_effect(effect)
	, m_clear(clear)
	, m_suppressors(suppressors)
	, m_inputs(inputs)
	, m_uniforms(uniforms)
	, m_targets(targets)
	, m_output(output)
{
}

bgfx_chain_entry::~bgfx_chain_entry()
{
	for (bgfx_input_pair* input : m_inputs)
	{
		delete input;
	}
	m_inputs.clear();
	for (bgfx_entry_uniform* uniform : m_uniforms)
	{
		delete uniform;
	}
	m_uniforms.clear();
	delete m_clear;
}

void bgfx_chain_entry::submit(int view, render_primitive* prim, texture_manager& textures, uint16_t screen_count, uint16_t screen_width, uint16_t screen_height, float screen_scale_x, float screen_scale_y, float screen_offset_x, float screen_offset_y, uint32_t rotation_type, bool swap_xy, uint64_t blend, int32_t screen)
{
	if (!setup_view(view, screen_width, screen_height, screen))
	{
		return;
	}

	for (bgfx_input_pair* input : m_inputs)
	{
		input->bind(m_effect, screen);
	}

	bgfx::TransientVertexBuffer buffer;
	put_screen_buffer(prim, &buffer);
	bgfx::setVertexBuffer(0,&buffer);

	setup_auto_uniforms(prim, textures, screen_count, screen_width, screen_height, screen_scale_x, screen_scale_y, screen_offset_x, screen_offset_y, rotation_type, swap_xy, screen);

	for (bgfx_entry_uniform* uniform : m_uniforms)
	{
		if (uniform->name() != "s_tex")
		{
			uniform->bind();
		}
	}

	m_effect->submit(view, blend);

	if (m_targets.target(screen, m_output) != nullptr)
	{
		m_targets.target(screen, m_output)->page_flip();
	}
}

void bgfx_chain_entry::setup_screensize_uniforms(texture_manager& textures, uint16_t screen_width, uint16_t screen_height, int32_t screen)
{
	float width = screen_width;
	float height = screen_height;
	if (m_inputs.size() > 0)
	{
		std::string name = m_inputs[0]->texture() + std::to_string(screen);
		width = float(textures.provider(name)->width());
		height = float(textures.provider(name)->height());
	}

	bgfx_uniform* screen_dims = m_effect->uniform("u_screen_dims");
	if (screen_dims != nullptr)
	{
		float values[2] = { width, height };
		screen_dims->set(values, sizeof(float) * 2);
	}

	bgfx_uniform* inv_screen_dims = m_effect->uniform("u_inv_screen_dims");
	if (inv_screen_dims != nullptr)
	{
		float values[2] = { 1.0f / width, 1.0f / height };
		inv_screen_dims->set(values, sizeof(float) * 2);
	}
}

void bgfx_chain_entry::setup_screenscale_uniforms(float screen_scale_x, float screen_scale_y)
{
	bgfx_uniform* screen_scale = m_effect->uniform("u_screen_scale");
	if (screen_scale != nullptr)
	{
		float values[2] = { screen_scale_x, screen_scale_y };
		screen_scale->set(values, sizeof(float) * 2);
	}
}

void bgfx_chain_entry::setup_screenoffset_uniforms(float screen_offset_x, float screen_offset_y)
{
	bgfx_uniform* screen_offset = m_effect->uniform("u_screen_offset");
	if (screen_offset != nullptr)
	{
		float values[2] = { screen_offset_x, screen_offset_y };
		screen_offset->set(values, sizeof(float) * 2);
	}
}

void bgfx_chain_entry::setup_screencount_uniforms(uint16_t screen_count)
{
	bgfx_uniform* u_screen_count = m_effect->uniform("u_screen_count");
	if (u_screen_count != nullptr)
	{
		float values[1] = { float(screen_count) };
		u_screen_count->set(values, sizeof(float));
	}
}

void bgfx_chain_entry::setup_sourcesize_uniform(render_primitive* prim) const
{
	bgfx_uniform* source_dims = m_effect->uniform("u_source_dims");
	if (source_dims != nullptr)
	{
		float values[2] = { float(prim->texture.width), float(prim->texture.height) };
		source_dims->set(values, sizeof(float) * 2);
	}
}

void bgfx_chain_entry::setup_targetsize_uniform(int32_t screen) const
{
	bgfx_uniform* target_dims = m_effect->uniform("u_target_dims");
	if (target_dims != nullptr)
	{
		bgfx_target* output = m_targets.target(screen, m_output);
		if (output != nullptr)
		{
			float values[2] = { float(output->width()), float(output->height()) };
			target_dims->set(values, sizeof(float) * 2);
		}
	}
}

void bgfx_chain_entry::setup_targetscale_uniform(int32_t screen) const
{
	bgfx_uniform* target_scale = m_effect->uniform("u_target_scale");
	if (target_scale != nullptr)
	{
		bgfx_target* output = m_targets.target(screen, m_output);
		if (output != nullptr)
		{
			float values[2] = { float(output->scale()), float(output->scale()) };
			target_scale->set(values, sizeof(float) * 2);
		}
	}
}

void bgfx_chain_entry::setup_rotationtype_uniform(uint32_t rotation_type) const
{
	bgfx_uniform* rotation_type_uniform = m_effect->uniform("u_rotation_type");
	if (rotation_type_uniform != nullptr)
	{
		float values[1] = { float(rotation_type) };
		rotation_type_uniform->set(values, sizeof(float));
	}
}

void bgfx_chain_entry::setup_swapxy_uniform(bool swap_xy) const
{
	bgfx_uniform* swap_xy_uniform = m_effect->uniform("u_swap_xy");
	if (swap_xy_uniform != nullptr)
	{
		float values[1] = { swap_xy ? 1.0f : 0.0f };
		swap_xy_uniform->set(values, sizeof(float));
	}
}

void bgfx_chain_entry::setup_quaddims_uniform(render_primitive* prim) const
{
	bgfx_uniform* quad_dims_uniform = m_effect->uniform("u_quad_dims");
	if (quad_dims_uniform != nullptr)
	{
		float values[2] = { float(floor((prim->bounds.x1 - prim->bounds.x0) + 0.5f)), float(floor((prim->bounds.y1 - prim->bounds.y0) + 0.5f)) };
		quad_dims_uniform->set(values, sizeof(float) * 2);
	}
}

void bgfx_chain_entry::setup_screenindex_uniform(int32_t screen) const
{
	bgfx_uniform* screen_index = m_effect->uniform("u_screen_index");
	if (screen_index != nullptr)
	{
		float values[1] = { float(screen) };
		screen_index->set(values, sizeof(float));
	}
}

void bgfx_chain_entry::setup_auto_uniforms(render_primitive* prim, texture_manager& textures, uint16_t screen_count, uint16_t screen_width, uint16_t screen_height, float screen_scale_x, float screen_scale_y, float screen_offset_x, float screen_offset_y, uint32_t rotation_type, bool swap_xy, int32_t screen)
{
	setup_screensize_uniforms(textures, screen_width, screen_height, screen);
	setup_screenscale_uniforms(screen_scale_x, screen_scale_y);
	setup_screenoffset_uniforms(screen_offset_x, screen_offset_y);
	setup_screencount_uniforms(screen_count);
	setup_sourcesize_uniform(prim);
	setup_targetsize_uniform(screen);
	setup_targetscale_uniform(screen);
	setup_rotationtype_uniform(rotation_type);
	setup_swapxy_uniform(swap_xy);
	setup_quaddims_uniform(prim);
	setup_screenindex_uniform(screen);
}

bool bgfx_chain_entry::setup_view(int view, uint16_t screen_width, uint16_t screen_height, int32_t screen) const
{
	bgfx::FrameBufferHandle handle = BGFX_INVALID_HANDLE;
	uint16_t width = screen_width;
	uint16_t height = screen_height;
	if (m_targets.target(screen, m_output) != nullptr)
	{
		bgfx_target* output = m_targets.target(screen, m_output);
		if (output->width() == 0)
		{
			return false;
		}
		handle = output->target();
		width = output->width();
		height = output->height();
	}

	bgfx::setViewFrameBuffer(view, handle);
	bgfx::setViewRect(view, 0, 0, width, height);

	const bgfx::Caps* caps = bgfx::getCaps();

	float projMat[16];
	bx::mtxOrtho(projMat, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, caps->homogeneousDepth);
	bgfx::setViewTransform(view, nullptr, projMat);

	m_clear->bind(view);
	return true;
}

void bgfx_chain_entry::put_screen_buffer(render_primitive* prim, bgfx::TransientVertexBuffer* buffer) const
{
	if (6 == bgfx::getAvailTransientVertexBuffer(6, ScreenVertex::ms_decl))
	{
		bgfx::allocTransientVertexBuffer(buffer, 6, ScreenVertex::ms_decl);
	}
	else
	{
		return;
	}

	ScreenVertex* vertex = reinterpret_cast<ScreenVertex*>(buffer->data);

	float x[4] = { 0, 1, 0, 1 };
	float y[4] = { 0, 0, 1, 1 };
	float u[4] = { 0, 1, 0, 1 };
	float v[4] = { 0, 0, 1, 1 };

	bgfx::RendererType::Enum renderer_type = bgfx::getRendererType();
	if (renderer_type == bgfx::RendererType::OpenGL || renderer_type == bgfx::RendererType::OpenGLES)
	{
		v[0] = v[1] = 1;
		v[2] = v[3] = 0;
	}

	vertex[0].m_x = x[0];
	vertex[0].m_y = y[0];
	vertex[0].m_z = 0;
	vertex[0].m_rgba = 0xffffffff;
	vertex[0].m_u = u[0];
	vertex[0].m_v = v[0];

	vertex[1].m_x = x[1];
	vertex[1].m_y = y[1];
	vertex[1].m_z = 0;
	vertex[1].m_rgba = 0xffffffff;
	vertex[1].m_u = u[1];
	vertex[1].m_v = v[1];

	vertex[2].m_x = x[3];
	vertex[2].m_y = y[3];
	vertex[2].m_z = 0;
	vertex[2].m_rgba = 0xffffffff;
	vertex[2].m_u = u[3];
	vertex[2].m_v = v[3];

	vertex[3].m_x = x[3];
	vertex[3].m_y = y[3];
	vertex[3].m_z = 0;
	vertex[3].m_rgba = 0xffffffff;
	vertex[3].m_u = u[3];
	vertex[3].m_v = v[3];

	vertex[4].m_x = x[2];
	vertex[4].m_y = y[2];
	vertex[4].m_z = 0;
	vertex[4].m_rgba = 0xffffffff;
	vertex[4].m_u = u[2];
	vertex[4].m_v = v[2];

	vertex[5].m_x = x[0];
	vertex[5].m_y = y[0];
	vertex[5].m_z = 0;
	vertex[5].m_rgba = 0xffffffff;
	vertex[5].m_u = u[0];
	vertex[5].m_v = v[0];
}

bool bgfx_chain_entry::skip()
{
	if (m_suppressors.size() == 0)
	{
		return false;
	}

	// Group all AND/OR'd results together and OR them together (hack for now)
	// TODO: Make this a bit more logical

	bool or_suppress = false;
	int and_count = 0;
	int and_suppressed = 0;
	for (bgfx_suppressor* suppressor : m_suppressors)
	{
		if (suppressor->combine() == bgfx_suppressor::combine_mode::COMBINE_AND)
		{
			and_count++;
			if (suppressor->suppress())
			{
				and_suppressed++;
			}
		}
		else if (suppressor->combine() == bgfx_suppressor::combine_mode::COMBINE_OR)
		{
			or_suppress |= suppressor->suppress();
		}
	}

	return (and_count != 0 && and_suppressed == and_count) || or_suppress;
}
