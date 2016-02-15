#pragma once

#ifndef __RENDER_BGFX__
#define __RENDER_BGFX__

#include <map>
#include <vector>

#include "binpacker.h"

/* sdl_info is the information about SDL for the current screen */
class renderer_bgfx : public osd_renderer
{
public:
	renderer_bgfx(osd_window *w)
		: osd_renderer(w, FLAG_NONE)
		, m_dimensions(0, 0)
	{
	}

	virtual int create() override;
	virtual int draw(const int update) override;
#ifdef OSD_SDL
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;
#else
	virtual void save() override { }
	virtual void record() override { }
	virtual void toggle_fsfx() override { }
#endif
	virtual void destroy() override;
	virtual render_primitive_list *get_primitives() override
	{
		osd_dim wdim = window().get_size();
		window().target()->set_bounds(wdim.width(), wdim.height(), window().aspect());
		return &window().target()->get_primitives();
	}

private:
	struct PosColorTexCoord0Vertex
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

	struct PosColorVertex
	{
		float m_x;
		float m_y;
		float m_z;
		UINT32 m_rgba;

		static void init()
		{
			ms_decl
				.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.end();
		}

		static bgfx::VertexDecl ms_decl;
	};

	void allocate_buffers(bgfx::TransientVertexBuffer *flat_buffer, bgfx::TransientVertexBuffer *textured_buffer);

	void render_textured_quad(int view, render_primitive* prim);
	void render_flat_quad(int view, render_primitive *prim);

	void put_packed_quad(render_primitive *prim, UINT32 hash, PosColorTexCoord0Vertex* vertex);
	void put_polygon(const float* coords, UINT32 num_coords, float r, UINT32 rgba, PosColorVertex* vertex);
	void put_line(float x0, float y0, float x1, float y1, float r, UINT32 rgba, PosColorVertex* vertex, float fth = 1.0f);

	void set_bgfx_state(UINT32 blend);

	uint32_t u32Color(uint32_t r, uint32_t g, uint32_t b, uint32_t a);

	bool check_for_dirty_atlas();
	bool update_atlas();
	const bgfx::Memory* mame_texture_data_to_bgfx_texture_data(UINT32 format, int width, int height, int rowpixels, const rgb_t *palette, void *base);

	bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName);
	bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName);

	bgfx::ProgramHandle m_progQuad;
	bgfx::ProgramHandle m_progQuadTexture;
	bgfx::UniformHandle m_s_texColor;
	bgfx::FrameBufferHandle fbh;
	bgfx::TextureHandle m_texture_cache;

	// Original display_mode
	osd_dim m_dimensions;

	std::map<UINT32, rectangle_packer::packed_rectangle> m_hash_to_entry;
	std::vector<rectangle_packer::packable_rectangle> m_texinfo;
	rectangle_packer m_packer;

	static const uint16_t CACHE_SIZE = 1024;
};

#endif