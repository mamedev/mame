#pragma once
#ifndef __K001005_H__
#define __K001005_H__

#include "video/polylgcy.h"
#include "cpu/sharc/sharc.h"

#define POLY_DEVICE 0

struct k001005_interface
{
	const char     *m_cpu_tag;
	const char     *m_dsp_tag;
	const char     *m_k001006_1_tag;
	const char     *m_k001006_2_tag;

	const char     *m_gfx_memory_region_tag;
	int            m_gfx_index;
};


class k001005_device : public device_t,
								public device_video_interface,
								public k001005_interface
{
public:
	k001005_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001005_device() {}

	static void static_set_palette_tag(device_t &device, const char *tag);

	void draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void swap_buffers();
	void preprocess_texture_data(UINT8 *rom, int length, int gticlub);
	void render_polygons();

	#if POLY_DEVICE

	void draw_scanline( void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid );
	void draw_scanline_tex( void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid );


	#endif

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();

private:
	// internal state
	device_t *m_cpu;
	adsp21062_device *m_dsp;
	device_t *m_k001006_1;
	device_t *m_k001006_2;

	UINT8  *     m_texture;
	UINT16 *     m_ram[2];
	UINT32 *     m_fifo;
	UINT32 *     m_3d_fifo;

	UINT32    m_status;
	bitmap_rgb32 *m_bitmap[2];
	bitmap_ind32 *m_zbuffer;
	rectangle m_cliprect;
	int    m_ram_ptr;
	int    m_fifo_read_ptr;
	int    m_fifo_write_ptr;
	int    m_3d_fifo_ptr;

	int m_tex_mirror_table[4][128];

	int m_bitmap_page;

	legacy_poly_manager *m_poly;
	poly_vertex m_prev_v[4];
	int m_prev_poly_type;

	UINT8 *m_gfxrom;
	required_device<palette_device> m_palette;
};

extern const device_type K001005;


#define MCFG_K001005_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K001005, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K001005_PALETTE(_palette_tag) \
	k001005_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
