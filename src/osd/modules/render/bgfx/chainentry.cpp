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
#include <bx/fpumath.h>

#include "chainentry.h"

#include "effect.h"
#include "texture.h"
#include "target.h"
#include "entryuniform.h"
#include "texturemanager.h"
#include "vertex.h"
#include "suppressor.h"

bgfx_chain_entry::bgfx_chain_entry(std::string name, bgfx_effect* effect, std::vector<bgfx_suppressor*> suppressors, std::vector<bgfx_input_pair> inputs, std::vector<bgfx_entry_uniform*> uniforms, target_manager& targets, std::string output)
	: m_name(name)
	, m_effect(effect)
    , m_suppressors(suppressors)
    , m_inputs(inputs)
    , m_uniforms(uniforms)
    , m_targets(targets)
	, m_output(output)
{
}

bgfx_chain_entry::~bgfx_chain_entry()
{
	for (bgfx_entry_uniform* uniform : m_uniforms)
	{
		delete uniform;
	}
	m_uniforms.clear();
}

void bgfx_chain_entry::submit(render_primitive* prim, int view, texture_manager& textures, uint16_t screen_width, uint16_t screen_height, uint64_t blend)
{
    bgfx::setViewSeq(view, true);

	setup_view(view, screen_width, screen_height);

    for (bgfx_input_pair input : m_inputs)
    {
        input.bind(m_effect, textures);
    }

    bgfx::TransientVertexBuffer buffer;
    put_screen_buffer(prim, &buffer);
    bgfx::setVertexBuffer(&buffer);

    bgfx_uniform* inv_screen_dims = m_effect->uniform("u_inv_screen_dims");
    bgfx_uniform* screen_dims = m_effect->uniform("u_screen_dims");
    if (screen_dims != nullptr || inv_screen_dims != nullptr)
    {
        float values[2];
        float width = screen_width;
        float height = screen_height;
        if (m_inputs.size() > 0)
        {
            width = float(textures.provider(m_inputs[0].texture())->width());
            height = float(textures.provider(m_inputs[0].texture())->height());
        }
        
        values[0] = 1.0f / width;
        values[1] = 1.0f / height;
        if (inv_screen_dims != nullptr) {
            inv_screen_dims->set(values, sizeof(float) * 2);
        }

        values[0] = width;
        values[1] = height;
        if (screen_dims != nullptr) {
            screen_dims->set(values, sizeof(float) * 2);
        }
    }

	bgfx_uniform* quad_dims = m_effect->uniform("u_quad_dims");
	if (quad_dims != nullptr)
	{
		float values[2];
		values[0] = prim->bounds.x1 - prim->bounds.x0;
		values[1] = prim->bounds.y1 - prim->bounds.y0;
		quad_dims->set(values, sizeof(float) * 2);
	}

    bgfx_uniform* source_dims = m_effect->uniform("u_source_dims");
    if (source_dims != nullptr)
    {
        float values[2];
        values[0] = float(prim->texture.width);
        values[1] = float(prim->texture.height);
        if (source_dims != nullptr)
        {
            source_dims->set(values, sizeof(float) * 2);
        }
    }
        
    for (bgfx_entry_uniform* uniform : m_uniforms)
    {
        if (uniform->name() != "DiffuseSampler")
        {
            uniform->bind();
        }
	}

    m_effect->submit(view, blend);
    
    if (m_targets.target(m_output) != nullptr)
    {
        m_targets.target(m_output)->page_flip();
    }
}

void bgfx_chain_entry::setup_view(int view, uint16_t screen_width, uint16_t screen_height)
{
	bgfx::FrameBufferHandle handle = BGFX_INVALID_HANDLE;
	uint16_t width = screen_width;
	uint16_t height = screen_height;
    if (m_targets.target(m_output) != nullptr)
    {
        bgfx_target* output = m_targets.target(m_output);
		handle = output->target();
		width = output->width();
		height = output->height();
    }

    bgfx::setViewFrameBuffer(view, handle);
	bgfx::setViewRect(view, 0, 0, width, height);

	float viewMat[16];
	bx::mtxIdentity(viewMat);

	float projMat[16];
	bx::mtxOrtho(projMat, 0.0f, screen_width, screen_height, 0.0f, 0.0f, 100.0f);
	bgfx::setViewTransform(view, viewMat, projMat);

    bgfx::setViewClear(view, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
}

void bgfx_chain_entry::put_screen_buffer(render_primitive* prim, bgfx::TransientVertexBuffer* buffer)
{
    if (bgfx::checkAvailTransientVertexBuffer(6, ScreenVertex::ms_decl))
    {
        bgfx::allocTransientVertexBuffer(buffer, 6, ScreenVertex::ms_decl);
    }
    else
    {
        return;
    }

    ScreenVertex* vertex = reinterpret_cast<ScreenVertex*>(buffer->data);

    vertex[0].m_x = prim->bounds.x0;
    vertex[0].m_y = prim->bounds.y0;
    vertex[0].m_z = 0;
    vertex[0].m_rgba = 0xffffffff;
    vertex[0].m_u = prim->texcoords.tl.u;
    vertex[0].m_v = prim->texcoords.tl.v;

    vertex[1].m_x = prim->bounds.x1;
    vertex[1].m_y = prim->bounds.y0;
    vertex[1].m_z = 0;
    vertex[1].m_rgba = 0xffffffff;
    vertex[1].m_u = prim->texcoords.tr.u;
    vertex[1].m_v = prim->texcoords.tr.v;

    vertex[2].m_x = prim->bounds.x1;
    vertex[2].m_y = prim->bounds.y1;
    vertex[2].m_z = 0;
    vertex[2].m_rgba = 0xffffffff;
    vertex[2].m_u = prim->texcoords.br.u;
    vertex[2].m_v = prim->texcoords.br.v;

    vertex[3].m_x = prim->bounds.x1;
    vertex[3].m_y = prim->bounds.y1;
    vertex[3].m_z = 0;
    vertex[3].m_rgba = 0xffffffff;
    vertex[3].m_u = prim->texcoords.br.u;
    vertex[3].m_v = prim->texcoords.br.v;

    vertex[4].m_x = prim->bounds.x0;
    vertex[4].m_y = prim->bounds.y1;
    vertex[4].m_z = 0;
    vertex[4].m_rgba = 0xffffffff;
    vertex[4].m_u = prim->texcoords.bl.u;
    vertex[4].m_v = prim->texcoords.bl.v;

    vertex[5].m_x = prim->bounds.x0;
    vertex[5].m_y = prim->bounds.y0;
    vertex[5].m_z = 0;
    vertex[5].m_rgba = 0xffffffff;
    vertex[5].m_u = prim->texcoords.tl.u;
    vertex[5].m_v = prim->texcoords.tl.v;
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