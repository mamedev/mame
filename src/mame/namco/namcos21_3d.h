// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood
#ifndef MAME_NAMCO_NAMCOS21_3D_H
#define MAME_NAMCO_NAMCOS21_3D_H

#pragma once

class namcos21_3d_device : public device_t
{
public:
	namcos21_3d_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// config
	void set_fixed_palbase(int base) { m_fixed_palbase = base; }
	void set_zz_shift_mult(int shift, int mult) { m_zz_shift = shift; m_zzmult = mult; }
	void set_depth_reverse(bool reverse) { m_depth_reverse = reverse; }

	void set_framebuffer_size(int width, int height)
	{
		m_poly_frame_width = width;
		m_poly_frame_height = height;
		m_framebuffer_size_in_bytes = sizeof(u16) * m_poly_frame_width * m_poly_frame_height;
	}

	int width() { return m_poly_frame_width; }
	int height() { return m_poly_frame_height; }

	void copy_visible_poly_framebuffer(bitmap_ind16 &bitmap, const rectangle &cliprect, int zlo, int zhi);
	void swap_and_clear_poly_framebuffer();
	u16 *get_visible_zbuffer() { return m_mpPolyFrameBufferZ2.get(); }

	void draw_direct_quad(const u16 *source, u16 color);
	void draw_quads(const u16 *source, const u8 *pointram, const u32 ptram_size, u32 quad_idx);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	struct n21_vertex
	{
		double x,y;
		double z;
	};

	struct edge
	{
		double x;
		double z;
	};

	void renderscanline_flat(const edge *e1, const edge *e2, int sy, u16 color, int depthcueenable);
	void rendertri(const n21_vertex *v0, const n21_vertex *v1, const n21_vertex *v2, u16 color, int depthcueenable);
	void blit_single_quad(int sx[4], int sy[4], int zcode[4], u16 color);
	void allocate_poly_framebuffer();

	std::unique_ptr<u16[]> m_mpPolyFrameBufferPens;
	std::unique_ptr<u16[]> m_mpPolyFrameBufferZ;
	std::unique_ptr<u16[]> m_mpPolyFrameBufferPens2;
	std::unique_ptr<u16[]> m_mpPolyFrameBufferZ2;

	int m_fixed_palbase;
	int m_zz_shift, m_zzmult;
	bool m_depth_reverse;

	int m_poly_frame_width;
	int m_poly_frame_height;
	int m_framebuffer_size_in_bytes;
};

DECLARE_DEVICE_TYPE(NAMCOS21_3D, namcos21_3d_device)

#endif // MAME_NAMCO_NAMCOS21_3D_H
