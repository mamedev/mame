// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun cgsix-series accelerated 8-bit color video controller

***************************************************************************/

#ifndef MAME_BUS_SBUS_CGSIX_H
#define MAME_BUS_SBUS_CGSIX_H

#pragma once

#include "sbus.h"
#include "video/bt45x.h"
#include "screen.h"

class sbus_cgsix_device : public device_t, public device_sbus_card_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

protected:
	// construction/destruction
	sbus_cgsix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const uint32_t vram_size)
		: sbus_cgsix_device(mconfig, type, tag, owner, clock)
	{
		set_vram_size(vram_size);
	}

	sbus_cgsix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_vram_size(uint32_t vram_size) { m_vram_size = vram_size; }

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint32_t rom_r(offs_t offset);
	uint32_t unknown_r(offs_t offset, uint32_t mem_mask = ~0);
	void unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fbc_r(offs_t offset, uint32_t mem_mask = ~0);
	void fbc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t thc_misc_r(offs_t offset, uint32_t mem_mask = ~0);
	void thc_misc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cursor_address_r();
	void cursor_address_w(uint32_t data);
	uint32_t cursor_ram_r(offs_t offset);
	void cursor_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vblank_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint8_t perform_rasterop(uint8_t src, uint8_t dst, uint8_t mask = 0xff);
	void handle_font_poke();
	void handle_draw_command();
	void handle_blit_command();

	void base_map(address_map &map) ATTR_COLD;

	enum
	{
		ROP_CLR             = 0x00,
		ROP_SRC_NOR_DST     = 0x01,
		ROP_NSRC_AND_DST    = 0x02,
		ROP_NOT_SRC         = 0x03,
		ROP_SRC_AND_NDST    = 0x04,
		ROP_NOT_DST         = 0x05,
		ROP_SRC_XOR_DST     = 0x06,
		ROP_SRC_NAND_DST    = 0x07,
		ROP_SRC_AND_DST     = 0x08,
		ROP_SRC_XNOR_DST    = 0x09,
		ROP_DST             = 0x0a,
		ROP_NSRC_OR_DST     = 0x0b,
		ROP_SRC             = 0x0c,
		ROP_SRC_OR_NDST     = 0x0d,
		ROP_SRC_OR_DST      = 0x0e,
		ROP_SET             = 0x0f
	};

	enum
	{
		THC_MISC_IRQ_BIT    = 4,
		THC_MISC_IRQEN_BIT  = 5,
		THC_MISC_CURRES_BIT = 6,
		THC_MISC_SYNCEN_BIT = 7,
		THC_MISC_VSYNC_BIT  = 8,
		THC_MISC_SYNC_BIT   = 9,
		THC_MISC_ENVID_BIT  = 10,
		THC_MISC_RESET_BIT  = 12,
		THC_MISC_REV        = 0x00010000,
		THC_MISC_WRITE_MASK = 0x000014ff
	};

	enum
	{
		FBC_CONFIG_FBID     = 0x60000000,
		FBC_CONFIG_VERSION  = 0x00100000,
		FBC_CONFIG_MASK     = 0x000f3fff
	};

	enum
	{
		FBC_MISC_INDEX_SHIFT        = 4,
		FBC_MISC_INDEX_MOD_SHIFT    = 6,
		FBC_MISC_BDISP_SHIFT        = 7,
		FBC_MISC_BREAD_SHIFT        = 9,
		FBC_MISC_BWRITE1_SHIFT      = 11,
		FBC_MISC_BWRITE0_SHIFT      = 13,
		FBC_MISC_DRAW_SHIFT         = 15,
		FBC_MISC_DATA_SHIFT         = 17,
		FBC_MISC_VBLANK_SHIFT       = 19,
		FBC_MISC_BLIT_SHIFT         = 20
	};

	enum
	{
		FBC_MISC_INDEX_MASK     = 0x3,
		FBC_MISC_INDEX_MOD_MASK = 0x1,
		FBC_MISC_BDISP_MASK     = 0x3,
		FBC_MISC_BREAD_MASK     = 0x3,
		FBC_MISC_BWRITE1_MASK   = 0x3,
		FBC_MISC_BWRITE0_MASK   = 0x3,
		FBC_MISC_DRAW_MASK      = 0x3,
		FBC_MISC_DATA_MASK      = 0x3,
		FBC_MISC_VBLANK_MASK    = 0x1,
		FBC_MISC_BLIT_MASK      = 0x3
	};

	enum
	{
		FBC_MISC_BDISP_IGNORE       = 0,
		FBC_MISC_BDISP_0            = 1,
		FBC_MISC_BDISP_1            = 2,
		FBC_MISC_BDISP_ILLEGAL      = 3,

		FBC_MISC_BREAD_IGNORE       = 0,
		FBC_MISC_BREAD_0            = 1,
		FBC_MISC_BREAD_1            = 2,
		FBC_MISC_BREAD_ILLEGAL      = 3,

		FBC_MISC_BWRITE1_IGNORE     = 0,
		FBC_MISC_BWRITE1_ENABLE     = 1,
		FBC_MISC_BWRITE1_DISABLE    = 2,
		FBC_MISC_BWRITE1_ILLEGAL    = 3,

		FBC_MISC_BWRITE0_IGNORE     = 0,
		FBC_MISC_BWRITE0_ENABLE     = 1,
		FBC_MISC_BWRITE0_DISABLE    = 2,
		FBC_MISC_BWRITE0_ILLEGAL    = 3,

		FBC_MISC_DRAW_IGNORE        = 0,
		FBC_MISC_DRAW_RENDER        = 1,
		FBC_MISC_DRAW_PICK          = 2,
		FBC_MISC_DRAW_ILLEGAL       = 3,

		FBC_MISC_DATA_IGNORE        = 0,
		FBC_MISC_DATA_COLOR8        = 1,
		FBC_MISC_DATA_COLOR1        = 2,
		FBC_MISC_DATA_HRMONO        = 3,

		FBC_MISC_BLIT_IGNORE        = 0,
		FBC_MISC_BLIT_NOSRC         = 1,
		FBC_MISC_BLIT_SRC           = 2,
		FBC_MISC_BLIT_ILLEGAL       = 3
	};

	inline uint32_t fbc_misc_index()        { return (m_fbc.m_misc >> FBC_MISC_INDEX_SHIFT) & FBC_MISC_INDEX_MASK; }
	inline uint32_t fbc_misc_index_mod()    { return (m_fbc.m_misc >> FBC_MISC_INDEX_MOD_SHIFT) & FBC_MISC_INDEX_MOD_MASK; }
	inline uint32_t fbc_misc_bdisp()        { return (m_fbc.m_misc >> FBC_MISC_BDISP_SHIFT) & FBC_MISC_BDISP_MASK; }
	inline uint32_t fbc_misc_bread()        { return (m_fbc.m_misc >> FBC_MISC_BREAD_SHIFT) & FBC_MISC_BREAD_MASK; }
	inline uint32_t fbc_misc_bwrite1()      { return (m_fbc.m_misc >> FBC_MISC_BWRITE1_SHIFT) & FBC_MISC_BWRITE1_MASK; }
	inline uint32_t fbc_misc_bwrite0()      { return (m_fbc.m_misc >> FBC_MISC_BWRITE0_SHIFT) & FBC_MISC_BWRITE0_MASK; }
	inline uint32_t fbc_misc_draw()         { return (m_fbc.m_misc >> FBC_MISC_DRAW_SHIFT) & FBC_MISC_DRAW_MASK;  }
	inline uint32_t fbc_misc_data()         { return (m_fbc.m_misc >> FBC_MISC_DATA_SHIFT) & FBC_MISC_DATA_MASK;  }
	inline uint32_t fbc_misc_blit()         { return (m_fbc.m_misc >> FBC_MISC_BLIT_SHIFT) & FBC_MISC_BLIT_MASK;  }
	inline uint8_t fbc_get_plane_mask();
	inline uint32_t fbc_get_pixel_mask();

	enum
	{
		FBC_RASTEROP_ROP00_SHIFT    = 0,
		FBC_RASTEROP_ROP01_SHIFT    = 4,
		FBC_RASTEROP_ROP10_SHIFT    = 8,
		FBC_RASTEROP_ROP11_SHIFT    = 12,
		FBC_RASTEROP_PLOT_SHIFT     = 16,
		FBC_RASTEROP_RAST_SHIFT     = 17,
		FBC_RASTEROP_ATTR_SHIFT     = 22,
		FBC_RASTEROP_POLYG_SHIFT    = 24,
		FBC_RASTEROP_PATT_SHIFT     = 26,
		FBC_RASTEROP_PIXEL_SHIFT    = 28,
		FBC_RASTEROP_PLANE_SHIFT    = 30
	};

	enum
	{
		FBC_RASTEROP_ROP00_MASK = 0xf,
		FBC_RASTEROP_ROP01_MASK = 0xf,
		FBC_RASTEROP_ROP10_MASK = 0xf,
		FBC_RASTEROP_ROP11_MASK = 0xf,
		FBC_RASTEROP_PLOT_MASK  = 0x1,
		FBC_RASTEROP_RAST_MASK  = 0x1,
		FBC_RASTEROP_ATTR_MASK  = 0x3,
		FBC_RASTEROP_POLYG_MASK = 0x3,
		FBC_RASTEROP_PATT_MASK  = 0x3,
		FBC_RASTEROP_PIXEL_MASK = 0x3,
		FBC_RASTEROP_PLANE_MASK = 0x3
	};

	enum
	{
		FBC_RASTEROP_PLOT_PLOT = 0,
		FBC_RASTEROP_PLOT_UNPLOT = 1,

		FBC_RASTEROP_RAST_BOOL = 0,
		FBC_RASTEROP_RAST_LINEAR = 1,

		FBC_RASTEROP_ATTR_IGNORE = 0,
		FBC_RASTEROP_ATTR_UNSUPP = 1,
		FBC_RASTEROP_ATTR_SUPP = 2,
		FBC_RASTEROP_ATTR_ILLEGAL = 3,

		FBC_RASTEROP_POLYG_IGNORE = 0,
		FBC_RASTEROP_POLYG_OVERLAP = 1,
		FBC_RASTEROP_POLYG_NONOVERLAP = 2,
		FBC_RASTEROP_POLYG_ILLEGAL = 3,

		FBC_RASTEROP_PATTERN_IGNORE = 0,
		FBC_RASTEROP_PATTERN_ZEROES = 1,
		FBC_RASTEROP_PATTERN_ONES = 2,
		FBC_RASTEROP_PATTERN_MSK = 3,

		FBC_RASTEROP_PIXEL_IGNORE = 0,
		FBC_RASTEROP_PIXEL_ZEROES = 1,
		FBC_RASTEROP_PIXEL_ONES = 2,
		FBC_RASTEROP_PIXEL_MSK = 3,

		FBC_RASTEROP_PLANE_IGNORE = 0,
		FBC_RASTEROP_PLANE_ZEROES = 1,
		FBC_RASTEROP_PLANE_ONES = 2,
		FBC_RASTEROP_PLANE_MSK = 3
	};

	inline uint32_t fbc_rasterop_rop00()    { return (m_fbc.m_rasterop >> FBC_RASTEROP_ROP00_SHIFT) & FBC_RASTEROP_ROP00_MASK; }
	inline uint32_t fbc_rasterop_rop01()    { return (m_fbc.m_rasterop >> FBC_RASTEROP_ROP01_SHIFT) & FBC_RASTEROP_ROP01_MASK; }
	inline uint32_t fbc_rasterop_rop10()    { return (m_fbc.m_rasterop >> FBC_RASTEROP_ROP10_SHIFT) & FBC_RASTEROP_ROP10_MASK; }
	inline uint32_t fbc_rasterop_rop11()    { return (m_fbc.m_rasterop >> FBC_RASTEROP_ROP11_SHIFT) & FBC_RASTEROP_ROP11_MASK; }
	inline uint32_t fbc_rasterop_plot()     { return (m_fbc.m_rasterop >> FBC_RASTEROP_PLOT_SHIFT)  & FBC_RASTEROP_PLOT_MASK;  }
	inline uint32_t fbc_rasterop_rast()     { return (m_fbc.m_rasterop >> FBC_RASTEROP_RAST_SHIFT)  & FBC_RASTEROP_RAST_MASK;  }
	inline uint32_t fbc_rasterop_attr()     { return (m_fbc.m_rasterop >> FBC_RASTEROP_ATTR_SHIFT)  & FBC_RASTEROP_ATTR_MASK;  }
	inline uint32_t fbc_rasterop_polyg()    { return (m_fbc.m_rasterop >> FBC_RASTEROP_POLYG_SHIFT) & FBC_RASTEROP_POLYG_MASK; }
	inline uint32_t fbc_rasterop_pattern()  { return (m_fbc.m_rasterop >> FBC_RASTEROP_PATT_SHIFT)  & FBC_RASTEROP_PATT_MASK;  }
	inline uint32_t fbc_rasterop_pixel()    { return (m_fbc.m_rasterop >> FBC_RASTEROP_PIXEL_SHIFT) & FBC_RASTEROP_PIXEL_MASK; }
	inline uint32_t fbc_rasterop_plane()    { return (m_fbc.m_rasterop >> FBC_RASTEROP_PLANE_SHIFT) & FBC_RASTEROP_PLANE_MASK; }

	enum
	{
		FBC_CONFIG      = 0x000/4,
		FBC_MISC        = 0x004/4,
		FBC_CLIP_CHECK  = 0x008/4,

		FBC_STATUS      = 0x010/4,
		FBC_DRAW_STATUS = 0x014/4,
		FBC_BLIT_STATUS = 0x018/4,
		FBC_FONT        = 0x01c/4,

		FBC_X0          = 0x080/4,
		FBC_Y0          = 0x084/4,
		FBC_Z0          = 0x088/4,
		FBC_COLOR0      = 0x08c/4,
		FBC_X1          = 0x090/4,
		FBC_Y1          = 0x094/4,
		FBC_Z1          = 0x098/4,
		FBC_COLOR1      = 0x09c/4,
		FBC_X2          = 0x0a0/4,
		FBC_Y2          = 0x0a4/4,
		FBC_Z2          = 0x0a8/4,
		FBC_COLOR2      = 0x0ac/4,
		FBC_X3          = 0x0b0/4,
		FBC_Y3          = 0x0b4/4,
		FBC_Z3          = 0x0b8/4,
		FBC_COLOR3      = 0x0bc/4,

		FBC_RASTER_OFFX = 0x0c0/4,
		FBC_RASTER_OFFY = 0x0c4/4,
		FBC_AUTOINCX    = 0x0d0/4,
		FBC_AUTOINCY    = 0x0d4/4,
		FBC_CLIP_MINX   = 0x0e0/4,
		FBC_CLIP_MINY   = 0x0e4/4,
		FBC_CLIP_MAXX   = 0x0f0/4,
		FBC_CLIP_MAXY   = 0x0f4/4,

		FBC_FCOLOR      = 0x100/4,
		FBC_BCOLOR      = 0x104/4,
		FBC_RASTEROP    = 0x108/4,
		FBC_PLANE_MASK  = 0x10c/4,
		FBC_PIXEL_MASK  = 0x110/4,

		FBC_PATT_ALIGN  = 0x11c/4,
		FBC_PATTERN0    = 0x120/4,
		FBC_PATTERN1    = 0x124/4,
		FBC_PATTERN2    = 0x128/4,
		FBC_PATTERN3    = 0x12c/4,
		FBC_PATTERN4    = 0x130/4,
		FBC_PATTERN5    = 0x134/4,
		FBC_PATTERN6    = 0x138/4,
		FBC_PATTERN7    = 0x13c/4,

		FBC_IPOINT_ABSX = 0x800/4,
		FBC_IPOINT_ABSY = 0x804/4,
		FBC_IPOINT_ABSZ = 0x808/4,
		FBC_IPOINT_RELX = 0x810/4,
		FBC_IPOINT_RELY = 0x814/4,
		FBC_IPOINT_RELZ = 0x818/4,
		FBC_IPOINT_R    = 0x830/4,
		FBC_IPOINT_G    = 0x834/4,
		FBC_IPOINT_B    = 0x838/4,
		FBC_IPOINT_A    = 0x83c/4,

		FBC_ILINE_ABSX  = 0x840/4,
		FBC_ILINE_ABSY  = 0x844/4,
		FBC_ILINE_ABSZ  = 0x848/4,
		FBC_ILINE_RELX  = 0x850/4,
		FBC_ILINE_RELY  = 0x854/4,
		FBC_ILINE_RELZ  = 0x858/4,
		FBC_ILINE_R     = 0x870/4,
		FBC_ILINE_G     = 0x874/4,
		FBC_ILINE_B     = 0x878/4,
		FBC_ILINE_A     = 0x87c/4,

		FBC_ITRI_ABSX   = 0x880/4,
		FBC_ITRI_ABSY   = 0x884/4,
		FBC_ITRI_ABSZ   = 0x888/4,
		FBC_ITRI_RELX   = 0x890/4,
		FBC_ITRI_RELY   = 0x894/4,
		FBC_ITRI_RELZ   = 0x898/4,
		FBC_ITRI_R      = 0x8b0/4,
		FBC_ITRI_G      = 0x8b4/4,
		FBC_ITRI_B      = 0x8b8/4,
		FBC_ITRI_A      = 0x8bc/4,

		FBC_IQUAD_ABSX  = 0x8c0/4,
		FBC_IQUAD_ABSY  = 0x8c4/4,
		FBC_IQUAD_ABSZ  = 0x8c8/4,
		FBC_IQUAD_RELX  = 0x8d0/4,
		FBC_IQUAD_RELY  = 0x8d4/4,
		FBC_IQUAD_RELZ  = 0x8d8/4,
		FBC_IQUAD_R     = 0x8f0/4,
		FBC_IQUAD_G     = 0x8f4/4,
		FBC_IQUAD_B     = 0x8f8/4,
		FBC_IQUAD_A     = 0x8fc/4,

		FBC_IRECT_ABSX  = 0x900/4,
		FBC_IRECT_ABSY  = 0x904/4,
		FBC_IRECT_ABSZ  = 0x908/4,
		FBC_IRECT_RELX  = 0x910/4,
		FBC_IRECT_RELY  = 0x914/4,
		FBC_IRECT_RELZ  = 0x918/4,
		FBC_IRECT_R     = 0x930/4,
		FBC_IRECT_G     = 0x934/4,
		FBC_IRECT_B     = 0x938/4,
		FBC_IRECT_A     = 0x93c/4,
	};

	struct vertex
	{
		uint32_t m_absx;
		uint32_t m_absy;
		uint32_t m_absz;
		uint32_t m_relx;
		uint32_t m_rely;
		uint32_t m_relz;
		uint32_t m_r;
		uint32_t m_g;
		uint32_t m_b;
		uint32_t m_a;
	};

	enum prim_type : uint32_t
	{
		PRIM_POINT = 0,
		PRIM_LINE,
		PRIM_TRI,
		PRIM_QUAD,
		PRIM_RECT,

		PRIM_COUNT
	};

	struct fbc
	{
		uint32_t m_config;
		uint32_t m_misc;
		uint32_t m_clip_check;
		uint32_t m_status;
		uint32_t m_draw_status;
		uint32_t m_blit_status;
		uint32_t m_font;

		uint32_t m_x0;
		uint32_t m_y0;
		uint32_t m_z0;
		uint32_t m_color0;
		uint32_t m_x1;
		uint32_t m_y1;
		uint32_t m_z1;
		uint32_t m_color1;
		uint32_t m_x2;
		uint32_t m_y2;
		uint32_t m_z2;
		uint32_t m_color2;
		uint32_t m_x3;
		uint32_t m_y3;
		uint32_t m_z3;
		uint32_t m_color3;

		uint32_t m_raster_offx;
		uint32_t m_raster_offy;

		uint32_t m_autoincx;
		uint32_t m_autoincy;

		uint32_t m_clip_minx;
		uint32_t m_clip_miny;

		uint32_t m_clip_maxx;
		uint32_t m_clip_maxy;

		uint8_t m_fcolor;
		uint8_t m_bcolor;

		uint32_t m_rasterop;

		uint32_t m_plane_mask;
		uint32_t m_pixel_mask;

		uint32_t m_patt_align;
		uint16_t m_patt_align_x;
		uint16_t m_patt_align_y;
		uint32_t m_pattern[8];
		uint16_t m_patterns[16];

		uint32_t m_ipoint_absx;
		uint32_t m_ipoint_absy;
		uint32_t m_ipoint_absz;
		uint32_t m_ipoint_relx;
		uint32_t m_ipoint_rely;
		uint32_t m_ipoint_relz;
		uint32_t m_ipoint_r;
		uint32_t m_ipoint_g;
		uint32_t m_ipoint_b;
		uint32_t m_ipoint_a;

		uint32_t m_iline_absx;
		uint32_t m_iline_absy;
		uint32_t m_iline_absz;
		uint32_t m_iline_relx;
		uint32_t m_iline_rely;
		uint32_t m_iline_relz;
		uint32_t m_iline_r;
		uint32_t m_iline_g;
		uint32_t m_iline_b;
		uint32_t m_iline_a;

		uint32_t m_itri_absx;
		uint32_t m_itri_absy;
		uint32_t m_itri_absz;
		uint32_t m_itri_relx;
		uint32_t m_itri_rely;
		uint32_t m_itri_relz;
		uint32_t m_itri_r;
		uint32_t m_itri_g;
		uint32_t m_itri_b;
		uint32_t m_itri_a;

		uint32_t m_iquad_absx;
		uint32_t m_iquad_absy;
		uint32_t m_iquad_absz;
		uint32_t m_iquad_relx;
		uint32_t m_iquad_rely;
		uint32_t m_iquad_relz;
		uint32_t m_iquad_r;
		uint32_t m_iquad_g;
		uint32_t m_iquad_b;
		uint32_t m_iquad_a;

		uint32_t m_irect_absx;
		uint32_t m_irect_absy;
		uint32_t m_irect_absz;
		uint32_t m_irect_relx;
		uint32_t m_irect_rely;
		uint32_t m_irect_relz;
		uint32_t m_irect_r;
		uint32_t m_irect_g;
		uint32_t m_irect_b;
		uint32_t m_irect_a;

		std::unique_ptr<vertex[]> m_prim_buf;
		uint32_t m_vertex_count;
		uint32_t m_curr_prim_type;
	};

	uint32_t m_thc_misc;
	int16_t m_cursor_x;
	int16_t m_cursor_y;

	required_memory_region m_rom;
	std::unique_ptr<uint32_t[]> m_vram;
	std::unique_ptr<uint32_t[]> m_cursor_ram;
	required_device<screen_device> m_screen;
	required_device<bt458_device> m_ramdac;

	fbc m_fbc;

	uint32_t m_vram_size;
};

class sbus_turbogx_device : public sbus_cgsix_device
{
public:
	sbus_turbogx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override;

	// device_sbus_slot_interface overrides
	virtual void install_device() override;

	void mem_map(address_map &map) override;
};

class sbus_turbogxp_device : public sbus_cgsix_device
{
public:
	sbus_turbogxp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override;

	// device_sbus_slot_interface overrides
	virtual void install_device() override;

	void mem_map(address_map &map) override;
};

DECLARE_DEVICE_TYPE(SBUS_TURBOGX, sbus_turbogx_device)
DECLARE_DEVICE_TYPE(SBUS_TURBOGXP, sbus_turbogxp_device)

#endif // MAME_BUS_SBUS_CGSIX_H
