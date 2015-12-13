// license:BSD-3-Clause
// copyright-holders:David Haywood
#pragma once
#ifndef __K001005_H__
#define __K001005_H__

#include <float.h>
#include "video/poly.h"
#include "video/k001006.h"
#include "cpu/sharc/sharc.h"


struct k001005_polydata
{
	UINT32 color;
	int texture_x, texture_y;
	int texture_width, texture_height;
	int texture_page;
	int texture_palette;
	int texture_mirror_x;
	int texture_mirror_y;
	int light_r, light_g, light_b;
	int ambient_r, ambient_g, ambient_b;
	int fog_r, fog_g, fog_b;
	UINT32 flags;
};

enum k001005_param
{
	K001005_LIGHT_R,
	K001005_LIGHT_G,
	K001005_LIGHT_B,
	K001005_AMBIENT_R,
	K001005_AMBIENT_G,
	K001005_AMBIENT_B,
	K001005_FOG_R,
	K001005_FOG_G,
	K001005_FOG_B,
	K001005_FAR_Z
};


class k001005_renderer : public poly_manager<float, k001005_polydata, 8, 50000>
{
public:
	k001005_renderer(device_t &parent, screen_device &screen, device_t *k001006);
	~k001005_renderer() {}

	void reset();
	void push_data(UINT32 data);
	void render_polygons();
	void swap_buffers();
	bool fifo_filled();
	void draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void set_param(k001005_param param, UINT32 value);

	void draw_scanline_2d(INT32 scanline, const extent_t &extent, const k001005_polydata &extradata, int threadid);
	void draw_scanline_2d_tex(INT32 scanline, const extent_t &extent, const k001005_polydata &extradata, int threadid);
	void draw_scanline(INT32 scanline, const extent_t &extent, const k001005_polydata &extradata, int threadid);
	void draw_scanline_tex(INT32 scanline, const extent_t &extent, const k001005_polydata &extradata, int threadid);
	void draw_scanline_gouraud_blend(INT32 scanline, const extent_t &extent, const k001005_polydata &extradata, int threadid);

	static const int POLY_Z = 0;
	static const int POLY_FOG = 1;
	static const int POLY_BRI = 2;
	static const int POLY_U = 3;
	static const int POLY_V = 4;
	static const int POLY_W = 5;
	static const int POLY_A = 2;
	static const int POLY_R = 3;
	static const int POLY_G = 4;
	static const int POLY_B = 5;

private:
	bitmap_rgb32 *m_fb[2];
	bitmap_ind32 *m_zb;
	rectangle m_cliprect;
	int m_fb_page;

	UINT32 *m_3dfifo;
	int m_3dfifo_ptr;

	vertex_t m_prev_v[4];

	UINT32 m_light_r;
	UINT32 m_light_g;
	UINT32 m_light_b;
	UINT32 m_ambient_r;
	UINT32 m_ambient_g;
	UINT32 m_ambient_b;
	UINT32 m_fog_r;
	UINT32 m_fog_g;
	UINT32 m_fog_b;
	float m_far_z;

	device_t *m_k001006;

	int *m_tex_mirror_table[2][8];
};


class k001005_device : public device_t,
								public device_video_interface
{
public:
	k001005_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001005_device() {}

	static void set_texel_chip(device_t &device, const char *tag);

	void draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void swap_buffers();
	void preprocess_texture_data(UINT8 *rom, int length, int gticlub);
	void render_polygons();

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

private:
	// internal state
	device_t *m_k001006;
	const char *m_k001006_tag;

	UINT16 *     m_ram[2];
	UINT32 *     m_fifo;
	UINT32 m_status;

	int m_ram_ptr;
	int m_fifo_read_ptr;
	int m_fifo_write_ptr;
	UINT32 m_reg_far_z;


	k001005_renderer *m_renderer;
};

extern const device_type K001005;


#define MCFG_K001005_TEXEL_CHIP(_tag) \
	k001005_device::set_texel_chip(*device, _tag);

#endif
