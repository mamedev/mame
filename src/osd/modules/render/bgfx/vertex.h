// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  vertex.h - BGFX screen vertex data
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_VERTEX__
#define __DRAWBGFX_VERTEX__

#include <bgfx/bgfx.h>

struct ScreenVertex
{
	float m_x;
	float m_y;
	float m_z;
	UINT32 m_rgba;
	float m_u;
	float m_v;

	static void init()
	{
		ms_decl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();
	}

	static bgfx::VertexDecl ms_decl;
};

#endif // __DRAWBGFX_VERTEX__
