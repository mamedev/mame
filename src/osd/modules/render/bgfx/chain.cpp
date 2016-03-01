// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chain.cpp - BGFX screen-space post-effect chain
//
//============================================================

#include "emu.h"

#include "slider.h"
#include "parameter.h"
#include "chainentry.h"
#include "entryuniform.h"
#include "texturemanager.h"
#include "vertex.h"

#include "chain.h"

bgfx_chain::bgfx_chain(std::string name, std::string author, std::vector<bgfx_slider*> sliders, std::vector<bgfx_parameter*> params, std::vector<bgfx_chain_entry*> entries)
	: m_name(name)
	, m_author(author)
	, m_sliders(sliders)
	, m_params(params)
	, m_entries(entries)
{
    for (bgfx_slider* slider : m_sliders)
    {
        m_slider_map[slider->name()] = slider;
    }
}

bgfx_chain::~bgfx_chain()
{
	for (bgfx_slider* slider : m_sliders)
	{
		delete slider;
	}
	for (bgfx_parameter* param : m_params)
	{
		delete param;
	}
	for (bgfx_chain_entry* entry : m_entries)
	{
		delete entry;
	}
}

void bgfx_chain::process(render_primitive* prim, int view, texture_manager& textures, uint16_t screen_width, uint16_t screen_height, uint64_t blend)
{
	for (bgfx_chain_entry* entry : m_entries)
	{
        bgfx::TransientVertexBuffer buffer;
        if (bgfx::checkAvailTransientVertexBuffer(6, ScreenVertex::ms_decl))
        {
            bgfx::allocTransientVertexBuffer(&buffer, 6, ScreenVertex::ms_decl);
        }
        else
        {
            return;
        }

        ScreenVertex* vertex = reinterpret_cast<ScreenVertex*>(buffer.data);

        const uint32_t r = uint32_t(prim->color.r * 255);
        const uint32_t g = uint32_t(prim->color.g * 255) << 8;
        const uint32_t b = uint32_t(prim->color.b * 255) << 16;
        const uint32_t a = uint32_t(prim->color.a * 255) << 24;
        UINT32 rgba = r | g | b | a;

        vertex[0].m_x = prim->bounds.x0;
        vertex[0].m_y = prim->bounds.y0;
        vertex[0].m_z = 0;
        vertex[0].m_rgba = rgba;
        vertex[0].m_u = prim->texcoords.tl.u;
        vertex[0].m_v = prim->texcoords.tl.v;

        vertex[1].m_x = prim->bounds.x1;
        vertex[1].m_y = prim->bounds.y0;
        vertex[1].m_z = 0;
        vertex[1].m_rgba = rgba;
        vertex[1].m_u = prim->texcoords.tr.u;
        vertex[1].m_v = prim->texcoords.tr.v;

        vertex[2].m_x = prim->bounds.x1;
        vertex[2].m_y = prim->bounds.y1;
        vertex[2].m_z = 0;
        vertex[2].m_rgba = rgba;
        vertex[2].m_u = prim->texcoords.br.u;
        vertex[2].m_v = prim->texcoords.br.v;

        vertex[3].m_x = prim->bounds.x1;
        vertex[3].m_y = prim->bounds.y1;
        vertex[3].m_z = 0;
        vertex[3].m_rgba = rgba;
        vertex[3].m_u = prim->texcoords.br.u;
        vertex[3].m_v = prim->texcoords.br.v;

        vertex[4].m_x = prim->bounds.x0;
        vertex[4].m_y = prim->bounds.y1;
        vertex[4].m_z = 0;
        vertex[4].m_rgba = rgba;
        vertex[4].m_u = prim->texcoords.bl.u;
        vertex[4].m_v = prim->texcoords.bl.v;

        vertex[5].m_x = prim->bounds.x0;
        vertex[5].m_y = prim->bounds.y0;
        vertex[5].m_z = 0;
        vertex[5].m_rgba = rgba;
        vertex[5].m_u = prim->texcoords.tl.u;
        vertex[5].m_v = prim->texcoords.tl.v;
        
        bgfx::setVertexBuffer(&buffer);
        
        bgfx::setViewClear(view
            , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
            , 0x000000ff
            , 1.0f
            , 0
            );

        entry->submit(prim, view, textures, screen_width, screen_height, blend);
	}
}
