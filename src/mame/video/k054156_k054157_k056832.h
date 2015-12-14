// license:BSD-3-Clause
// copyright-holders:David Haywood, Olivier Galibert
#pragma once
#ifndef __K056832_H__
#define __K056832_H__

#define MCFG_K054156_054157_ADD(_tag, _dotclock, _sizex, _sizey, _vramwidth, _palette) \
	MCFG_DEVICE_ADD(_tag, K054156_054157, _dotclock)								\
	downcast<k054156_054157_device *>(device)->set_info(_sizex, _sizey, _vramwidth, _palette);

#define MCFG_K054156_054157_INT1_CB(_devcb) \
	devcb = &k054156_054157_device::set_int1_cb(*device, DEVCB_##_devcb);

#define MCFG_K054156_054157_VBLANK_CB(_devcb) \
	devcb = &k054156_054157_device::set_vblank_cb(*device, DEVCB_##_devcb);

#define MCFG_K054156_054157_5BPP_ADD(_tag, _dotclock,_sizex, _sizey, _vramwidth, _palette) \
	MCFG_DEVICE_ADD(_tag, K054156_054157, _dotclock)								\
	downcast<k054156_054157_device *>(device)->set_5bpp();                  \
	downcast<k054156_054157_device *>(device)->set_info(_sizex, _sizey, _vramwidth, _palette);

#define MCFG_K054156_DUAL_054157_ADD(_tag, _dotclock,_sizex, _sizey, _vramwidth, _palette) \
	MCFG_DEVICE_ADD(_tag, K054156_054157, _dotclock)								     \
	downcast<k054156_054157_device *>(device)->set_dual();                       \
	downcast<k054156_054157_device *>(device)->set_info(_sizex, _sizey, _vramwidth, _palette);

#define MCFG_K054156_056832_ADD(_tag, _dotclock,_sizex, _sizey, _vramwidth, _palette) \
	MCFG_DEVICE_ADD(_tag, K054156_056832, _dotclock)								  \
	downcast<k054156_056832_device *>(device)->set_info(_sizex, _sizey, _vramwidth, _palette);

#define MCFG_K058143_056832_ADD(_tag, _dotclock,_sizex, _sizey, _vramwidth, _palette) \
	MCFG_DEVICE_ADD(_tag, K058143_056832, _dotclock)								\
	downcast<k058143_056832_device *>(device)->set_info(_sizex, _sizey, _vramwidth, _palette);



class k054156_056832_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	k054156_056832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	k054156_056832_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	template<class _Object> static devcb_base &set_int1_cb(device_t &device, _Object object) { return downcast<k054156_056832_device &>(device).m_int1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_vblank_cb(device_t &device, _Object object) { return downcast<k054156_056832_device &>(device).m_vblank_cb.set_callback(object); }

	void set_info(int _sizex, int _sizey, int _vramwidth, const char *_palette);

	DECLARE_ADDRESS_MAP(vacset, 16);
	DECLARE_ADDRESS_MAP(vacset8, 8);
	virtual DECLARE_ADDRESS_MAP(vsccs, 16);
	virtual DECLARE_ADDRESS_MAP(vsccs8, 8);

	DECLARE_WRITE_LINE_MEMBER(vsync_w);

	DECLARE_WRITE16_MEMBER(reg1_w);
	DECLARE_WRITE16_MEMBER(reg2_w);
	DECLARE_WRITE16_MEMBER(reg3_w);
	DECLARE_WRITE16_MEMBER(reg4_w);
	DECLARE_WRITE16_MEMBER(reg5_w);
	DECLARE_WRITE16_MEMBER(rzs_w);
	DECLARE_WRITE16_MEMBER(ars_w);
	DECLARE_WRITE16_MEMBER(bv_w);
	DECLARE_WRITE16_MEMBER(bh_w);
	DECLARE_WRITE16_MEMBER(mv_w);
	DECLARE_WRITE16_MEMBER(mh_w);
	DECLARE_WRITE16_MEMBER(mpz_w);
	DECLARE_WRITE16_MEMBER(mpa_w);
	DECLARE_WRITE16_MEMBER(cadlm_w);
	DECLARE_WRITE16_MEMBER(cadh_w);
	DECLARE_WRITE16_MEMBER(vrc_w);
	DECLARE_WRITE16_MEMBER(offh_w);
	DECLARE_WRITE16_MEMBER(offv_w);

	DECLARE_WRITE8_MEMBER (reg1_8w);
	DECLARE_WRITE8_MEMBER (reg2_8w);
	DECLARE_WRITE8_MEMBER (reg3_8w);
	DECLARE_WRITE8_MEMBER (reg4_8w);
	DECLARE_WRITE8_MEMBER (reg5_8w);
	DECLARE_WRITE8_MEMBER (rzs_8w);
	DECLARE_WRITE8_MEMBER (ars_8w);
	DECLARE_WRITE8_MEMBER (bv_8w);
	DECLARE_WRITE8_MEMBER (bh_8w);
	DECLARE_WRITE8_MEMBER (mv_8w);
	DECLARE_WRITE8_MEMBER (mh_8w);
	DECLARE_WRITE8_MEMBER (mpz_8w);
	DECLARE_WRITE8_MEMBER (mpa_8w);
	DECLARE_WRITE8_MEMBER (cadlm_8w);
	DECLARE_WRITE8_MEMBER (cadh_8w);
	DECLARE_WRITE8_MEMBER (vrc_8w);
	DECLARE_WRITE8_MEMBER (offh_8w);
	DECLARE_WRITE8_MEMBER (offv_8w);

	DECLARE_WRITE16_MEMBER(vrc2_w);
	DECLARE_WRITE8_MEMBER (vrc2_8w);

	DECLARE_WRITE8_MEMBER (reg1b_w);
	DECLARE_WRITE8_MEMBER (reg2b_w);
	DECLARE_WRITE8_MEMBER (reg3b_w);
	DECLARE_WRITE8_MEMBER (reg4b_w);

	DECLARE_READ8_MEMBER  (vram8_r);
	DECLARE_WRITE8_MEMBER (vram8_w);
	DECLARE_READ16_MEMBER (vram16_r);
	DECLARE_WRITE16_MEMBER(vram16_w);
	DECLARE_READ32_MEMBER (vram32_r);
	DECLARE_WRITE32_MEMBER(vram32_w);

	DECLARE_READ8_MEMBER  (rom8_r);
	DECLARE_READ16_MEMBER (rom16_r);
	DECLARE_READ32_MEMBER (rom32_r);

	void bitmap_update(bitmap_ind16 *bitmap, const rectangle &cliprect, int layer);

protected:
	bool m_is_054157;
	bool m_is_5bpp;
	bool m_is_dual;

	// device-level overrides
	void device_start() override;
	void device_post_load() override;

private:
	enum class vram_access {
		l8w16  = 0*2+0,
		l8w24  = 0*2+1,
		l16w16 = 1*2+0,
		l16w24 = 1*2+1,
		l32w16 = 2*2+0,
		l32w24 = 2*2+1
	};

	devcb_write_line m_int1_cb, m_vblank_cb;
	required_memory_region m_region;

	int m_sizex, m_sizey, m_vramwidth;

	uint32_t m_x[4], m_y[4], m_sx[4], m_sy[4];
	uint16_t m_mv[4], m_mh[4];
	uint16_t m_cadlm, m_vrc, m_offh, m_offv;
	uint32_t m_cpu_cur_x, m_cpu_cur_y;
	uint8_t m_vrc2[8];
	uint8_t m_bv[4], m_bh[4];
	uint8_t m_reg1h, m_reg1l, m_reg2, m_reg3h, m_reg3l, m_reg4, m_reg5;
	uint8_t m_rzs, m_ars, m_mpz, m_mpa, m_cadh;
	uint8_t m_reg1b, m_reg2b, m_reg3b, m_reg4b;

	std::vector<uint32_t> m_videoram;
	uint32_t *m_page_pointers[8][8];
	uint32_t *m_tilemap_page[4][8][8];
	uint32_t *m_cur_cpu_page;
	uint32_t *m_cur_linescroll_page;
	void (*m_info_to_color[4])(uint32_t info, uint32_t &color, int &flipx, int &flipy);

	vram_access m_cur_vram_access;

	int m_cur_a0;

	void screen_vblank(screen_device &src, bool state);
	void select_cpu_page();
	void select_linescroll_page();
	void select_vram_access();
	void setup_tilemap(int layer);
	void decode_character_roms();
	void convert_chunky_planar();
	void convert_planar_chunky();
	template<bool gflipx> static uint32_t screen_to_tile_x(int32_t x, uint32_t delta);
	template<bool gflipx> static int32_t tile_to_screen_x(uint32_t tx, uint32_t delta);
	template<bool gflipy> static uint32_t screen_to_tile_y(int32_t y, uint32_t delta);
	template<bool gflipy> static int32_t tile_to_screen_y(uint32_t ty, uint32_t delta);

	template<bool gflipx, bool gflipy> void draw_page_512x1(bitmap_ind16 *bitmap, int layer, const rectangle &cliprect, const uint32_t *page, gfx_element *g, uint32_t min_x, uint32_t max_x, uint32_t min_y, uint32_t max_y, int32_t basex, int32_t basey);
	template<bool gflipx, bool gflipy> void draw_page_8x8(bitmap_ind16 *bitmap, int layer, const rectangle &cliprect, const uint32_t *page, gfx_element *g, uint32_t min_x, uint32_t max_x, uint32_t min_y, uint32_t max_y, int32_t basex, int32_t basey);
	template<bool gflipx, bool gflipy> void draw_line_block(bitmap_ind16 *bitmap, int layer, const rectangle &cliprect, uint32_t deltay, uint32_t deltax);
};

class k054156_054157_device : public k054156_056832_device
{
public:
	k054156_054157_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_ADDRESS_MAP(vsccs, 16) override;
	DECLARE_ADDRESS_MAP(vsccs8, 8) override;

	void set_5bpp();
	void set_dual();
};

class k058143_056832_device : public k054156_056832_device
{
public:
	k058143_056832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER (lvram16_r);
	DECLARE_WRITE32_MEMBER(lvram16_w);
	DECLARE_READ32_MEMBER (lvram32_r);
	DECLARE_WRITE32_MEMBER(lvram32_w);
};

extern const device_type K054156_054157;
extern const device_type K054156_056832;
extern const device_type K058143_056832;

#endif
