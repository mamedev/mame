// license:LGPL-2.1+
// copyright-holders:Ville Linde, Angelo Salese, hap

#pragma once
#ifndef __TC0780FPA_H__
#define __TC0780FPA_H__

#include "video/poly.h"


struct tc0780fpa_polydata
{
	int tex_base_x;
	int tex_base_y;
	int tex_wrap_x;
	int tex_wrap_y;
};


class tc0780fpa_renderer : public poly_manager<float, tc0780fpa_polydata, 6, 10000>
{
public:
	tc0780fpa_renderer(device_t &parent, screen_device &screen, const UINT8 *texture_ram);
	~tc0780fpa_renderer() {}

	void render_solid_scan(INT32 scanline, const extent_t &extent, const tc0780fpa_polydata &extradata, int threadid);
	void render_shade_scan(INT32 scanline, const extent_t &extent, const tc0780fpa_polydata &extradata, int threadid);
	void render_texture_scan(INT32 scanline, const extent_t &extent, const tc0780fpa_polydata &extradata, int threadid);

	void render_polygons(UINT16 *polygon_fifo, int length);
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void clear_frame();

private:
	std::unique_ptr<bitmap_ind16> m_fb;
	std::unique_ptr<bitmap_ind16> m_zb;
	const UINT8 *m_texture;

	rectangle m_cliprect;
};


class tc0780fpa_device : public device_t, public device_video_interface
{
public:
	tc0780fpa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0780fpa_device() {}

	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void texture_write(offs_t address, UINT8 data);

	DECLARE_READ16_MEMBER(tex_addr_r);
	DECLARE_WRITE16_MEMBER(tex_addr_w);
	DECLARE_WRITE16_MEMBER(tex_w);
	DECLARE_WRITE16_MEMBER(poly_fifo_w);
	DECLARE_WRITE16_MEMBER(render_w);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

private:
	std::unique_ptr<UINT8[]> m_texture;
	std::unique_ptr<UINT16[]> m_poly_fifo;
	int m_poly_fifo_ptr;

	tc0780fpa_renderer *m_renderer;

	UINT16 m_tex_address;
	UINT16 m_tex_offset;
	int m_texbase_x;
	int m_texbase_y;
};

extern const device_type TC0780FPA;

#endif