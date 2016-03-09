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

bgfx_chain_entry::bgfx_chain_entry(std::string name, bgfx_effect* effect, std::vector<bgfx_input_pair>& inputs, std::vector<bgfx_entry_uniform*> uniforms, bgfx_target* output)
	: m_name(name)
	, m_effect(effect)
	, m_output(output)
{
	for (bgfx_input_pair input : inputs)
	{
		m_inputs.push_back(input);
	}
	for (bgfx_entry_uniform* uniform : uniforms)
	{
		m_uniforms.push_back(uniform);
	}
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

    for (bgfx_input_pair input : m_inputs) {
        input.bind(m_effect, textures);
    }

    bgfx::TransientVertexBuffer buffer;
    put_screen_buffer(prim, &buffer);
    bgfx::setVertexBuffer(&buffer);

    for (bgfx_entry_uniform* uniform : m_uniforms)
    {
		uniform->bind();
	}

	m_effect->submit(view, blend);

    if (m_output != nullptr)
    {
        m_output->page_flip();
    }
}

void bgfx_chain_entry::setup_view(int view, uint16_t screen_width, uint16_t screen_height)
{
	bgfx::FrameBufferHandle handle = BGFX_INVALID_HANDLE;
	uint16_t width = screen_width;
	uint16_t height = screen_height;
    if (m_output != nullptr)
    {
		handle = m_output->target();
		width = m_output->width();
		height = m_output->height();
    }

    bgfx::setViewFrameBuffer(view, handle);
	bgfx::setViewRect(view, 0, 0, width, height);

    {
        float viewMat[16];
        bx::mtxIdentity(viewMat);

        float projMat[16];
        bx::mtxOrtho(projMat, 0.0f, width, height, 0.0f, 0.0f, 100.0f);
        bgfx::setViewTransform(view, viewMat, projMat);
    }

    bgfx::setViewClear(view, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
}

void bgfx_chain_entry::put_screen_buffer(render_primitive* prim, bgfx::TransientVertexBuffer* buffer)
{
    if (bgfx::checkAvailTransientVertexBuffer(6, ScreenVertex::ms_decl)) {
        bgfx::allocTransientVertexBuffer(buffer, 6, ScreenVertex::ms_decl);
    } else {
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