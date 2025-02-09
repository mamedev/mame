// license:LGPL-2.1+
// copyright-holders:Ville Linde, Angelo Salese, hap

#ifndef MAME_TAITO_TC0780FPA_H
#define MAME_TAITO_TC0780FPA_H

#pragma once

#include "video/poly.h"


struct tc0780fpa_polydata
{
	int tex_base_x;
	int tex_base_y;
	int tex_wrap_x;
	int tex_wrap_y;
};


class tc0780fpa_renderer : public poly_manager<float, tc0780fpa_polydata, 6>
{
public:
	tc0780fpa_renderer(device_t &parent, screen_device &screen, const uint8_t *texture_ram);

	void render_solid_scan(int32_t scanline, const extent_t &extent, const tc0780fpa_polydata &extradata, int threadid);
	void render_shade_scan(int32_t scanline, const extent_t &extent, const tc0780fpa_polydata &extradata, int threadid);
	void render_texture_scan(int32_t scanline, const extent_t &extent, const tc0780fpa_polydata &extradata, int threadid);

	void render(uint16_t *polygon_fifo, int length);
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void swap_buffers();

private:
	bitmap_ind16 m_fb[2];
	bitmap_ind16 m_zb;
	const uint8_t *m_texture;

	rectangle m_cliprect;

	int m_current_fb;
};


class tc0780fpa_device : public device_t, public device_video_interface
{
public:
	tc0780fpa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint16_t tex_addr_r();
	void tex_addr_w(uint16_t data);
	void tex_w(uint16_t data);
	void poly_fifo_w(uint16_t data);
	void render_w(uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	std::unique_ptr<uint8_t[]> m_texture;
	std::unique_ptr<uint16_t[]> m_poly_fifo;
	int32_t m_poly_fifo_ptr = 0;

	std::unique_ptr<tc0780fpa_renderer> m_renderer;

	uint16_t m_tex_address = 0;
	uint16_t m_tex_offset = 0;
	int32_t m_texbase_x = 0;
	int32_t m_texbase_y = 0;
};

DECLARE_DEVICE_TYPE(TC0780FPA, tc0780fpa_device)

#endif // MAME_TAITO_TC0780FPA_H
