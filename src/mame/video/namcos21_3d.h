// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_NAMCOS21_3D_H
#define MAME_VIDEO_NAMCOS21_3D_H

#pragma once

#include "emupal.h"

#define NAMCOS21_POLY_FRAME_WIDTH 496
#define NAMCOS21_POLY_FRAME_HEIGHT 480

#define FRAMEBUFFER_SIZE_IN_BYTES (sizeof(uint16_t)*NAMCOS21_POLY_FRAME_WIDTH*NAMCOS21_POLY_FRAME_HEIGHT)


class namcos21_3d_device : public device_t
{
public:
	namcos21_3d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// config
	void set_fixed_palbase(int base) { m_fixed_palbase = base; }
	void set_zz_shift_mult(int shift, int mult) { m_zz_shift = shift; m_zzmult = mult;  }
	void set_depth_reverse(bool reverse) { m_depth_reverse = reverse;  }


	void copy_visible_poly_framebuffer(bitmap_ind16 &bitmap, const rectangle &clip, int zlo, int zhi);
	void clear_poly_framebuffer();

	void draw_quad(int sx[4], int sy[4], int zcode[4], int color);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{	/* Namco System21 */
		NAMCOS21_AIRCOMBAT = 0x4000,
		NAMCOS21_STARBLADE,
		NAMCOS21_CYBERSLED,
		NAMCOS21_SOLVALOU,
		NAMCOS21_WINRUN91,
		NAMCOS21_DRIVERS_EYES,
	};

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

	void renderscanline_flat(const edge *e1, const edge *e2, int sy, unsigned color, int depthcueenable);
	void rendertri(const n21_vertex *v0, const n21_vertex *v1, const n21_vertex *v2, unsigned color, int depthcueenable);
	void allocate_poly_framebuffer();

	std::unique_ptr<uint16_t[]> m_mpPolyFrameBufferPens;
	std::unique_ptr<uint16_t[]> m_mpPolyFrameBufferZ;
	std::unique_ptr<uint16_t[]> m_mpPolyFrameBufferPens2;
	std::unique_ptr<uint16_t[]> m_mpPolyFrameBufferZ2;

	int m_fixed_palbase;
	int m_zz_shift, m_zzmult;
	bool m_depth_reverse;
};

DECLARE_DEVICE_TYPE(NAMCOS21_3D, namcos21_3d_device)

#endif // MAME_VIDEO_NAMCOS21_3D_H
