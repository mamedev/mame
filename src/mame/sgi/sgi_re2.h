// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SGI_SGI_RE2_H
#define MAME_SGI_SGI_RE2_H

#pragma once

#include "sgi_xmap2.h"
#include "video/bt45x.h"
#include "video/bt431.h"
#include "screen.h"

class sgi_re2_device : public device_t
{
public:
	sgi_re2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	auto out_rdy() { return m_rdy_cb.bind(); }
	auto out_drq() { return m_drq_cb.bind(); }

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	enum re_register : unsigned
	{
		// buffered registers (write only unless noted)
		REG_ENABRGB   = 0x04, // enable 8 bit rgb (re2 only) (1)
		REG_BIGENDIAN = 0x05, // enable big endian mode (1)
		REG_FUNC      = 0x06, // raster op function (4)
		REG_HADDR     = 0x07, // starting pixel location (2)
		REG_NOPUP     = 0x08, // size of uaux (1)
		REG_XYFRAC    = 0x09, // initial xyfrac (4)
		REG_RGB       = 0x0a, // initial color values (27)
		REG_YX        = 0x0b, // initial y and x (22)
		REG_PUPDATA   = 0x0c, // pup data (2)
		REG_PATL      = 0x0d, // pattern mask low (16)
		REG_PATH      = 0x0e, // pattern mask high (16)
		REG_DZI       = 0x0f, // delta z integer (24)
		REG_DZF       = 0x10, // delta z fraction (14)
		REG_DR        = 0x11, // delta red (24)
		REG_DG        = 0x12, // delta green (20)
		REG_DB        = 0x13, // delta blue (20)
		REG_Z         = 0x14, // initial z integer (24)
		REG_R         = 0x15, // initial red (23)
		REG_G         = 0x16, // initial green (19)
		REG_B         = 0x17, // initial blue (19)
		REG_STIP      = 0x18, // stipple pattern (16, rw)
		REG_STIPCOUNT = 0x19, // stipple repeat (8, rw)
		REG_DX        = 0x1a, // delta x (16)
		REG_DY        = 0x1b, // delta y (16)
		REG_NUMPIX    = 0x1c, // pixel count (11)
		REG_X         = 0x1d, // initial x (12)
		REG_Y         = 0x1e, // initial y (11)
		REG_IR        = 0x1f, // instruction (3)

		// unbuffered registers (write only unless noted)
		REG_RWDATA    = 0x20, // read/write data (32, rw)
		REG_PIXMASK   = 0x21, // pixel mask (24)
		REG_AUXMASK   = 0x22, // auxiliary mask (9)
		REG_WIDDATA   = 0x23, // window id (4)
		REG_UAUXDATA  = 0x24, // uaux data (4)
		REG_RWMODE    = 0x25, // read/write mode (3)
		REG_READBUF   = 0x26, // buffer select (1)
		REG_PIXTYPE   = 0x27, // pixel type (2)
		REG_ASELECT   = 0x28, // antialias select (6)
		REG_ALIGNPAT  = 0x29, // pattern alignment (1)
		REG_ENABPAT   = 0x2a, // enable pattern mask (1)
		REG_ENABSTIP  = 0x2b, // enable stipple (1)
		REG_ENABDITH  = 0x2c, // enable dithering (1)
		REG_ENABWID   = 0x2d, // enable wid check (1)
		REG_CURWID    = 0x2e, // current wid (4)
		REG_DEPTHFN   = 0x2f, // depth function (4)
		REG_REPSTIP   = 0x30, // stipple repeat (8)
		REG_ENABLWID  = 0x31, // enable line wid (1)
		REG_FBOPTION  = 0x32, // frame buffer option (2)
		REG_TOPSCAN   = 0x33, // first row/column (18)
		REG_TESTMODE  = 0x34, // ??
		REG_TESTDATA  = 0x35, // ??
		REG_ZBOPTION  = 0x36, // z buffer option (1)
		REG_XZOOM     = 0x37, // x zoom factor (8)
		REG_UPACMODE  = 0x38, // packing mode (2)
		REG_YMIN      = 0x39, // bottom screen mask (11)
		REG_YMAX      = 0x3a, // top screen mask (11)
		REG_XMIN      = 0x3b, // left screen mask (12)
		REG_XMAX      = 0x3c, // right screen mask (12)
		REG_COLORCMP  = 0x3d, // z compare source (1)
		REG_MEGOPTION = 0x3e, // vram size (1)
	};

	enum re_rwmode : unsigned
	{
		RWMODE_FB   = 0, // frame buffer bitplanes
		RWMODE_PUP  = 1, // pup bitplanes
		RWMODE_UAUX = 2, // uaux bitplanes
		RWMODE_ZB   = 3, // z buffer bitplanes
		RWMODE_WID  = 4, // wid bitplanes
		RWMODE_FB_P = 6, // frame buffer port
		RWMODE_ZB_P = 7, // z buffer port
	};

	enum re_ir : unsigned
	{
		IR_SHADED     = 1, // draw shaded span
		IR_FLAT       = 2, // draw 1x5 flat span
		IR_FLAT4      = 3, // draw 1x20 flat span (megoption=0)
		IR_BLOCKWRITE = 3, // 20 pix blkwrt mode (megoption=1)
		IR_TOPLINE    = 4, // draw top of antialised line
		IR_BOTLINE    = 5, // draw bottom of antialised line
		IR_READBUF    = 6, // read buffer
		IR_WRITEBUF   = 7, // write buffer
	};

	u32 reg_r(offs_t offset);
	void reg_w(offs_t offset, u32 data);

protected:
	// state machine
	void step(s32 param = 0);
	void execute();

	// drawing functions
	void draw_shaded_span();
	void draw_flat_span(unsigned const n);
	void read_buffer();
	void write_buffer();

	// line helpers
	void set_rdy(bool state)
	{
		if (state != m_rdy)
		{
			m_rdy = state;
			m_rdy_cb(m_rdy);
		}
	}
	void set_drq(bool state)
	{
		if (state != m_drq)
		{
			m_drq = state;
			m_drq_cb(m_drq);
		}
	}

	// write condition helpers
	u32 unpack(u32 data, unsigned const n, u32 const mode) const;
	bool wid(unsigned const ir, offs_t const offset);
	bool pattern(unsigned const x, unsigned const n) const;

	void increment();

	void vram_w(offs_t const offset, u32 const data, u32 const mem_mask) { m_vram[offset] = (m_vram[offset] & ~mem_mask) | (data & mem_mask & m_vram_mask); }

private:
	required_device_array<sgi_xmap2_device, 5> m_xmap;
	required_device_array<bt431_device, 2> m_cursor;
	required_device_array<bt457_device, 3> m_ramdac;
	required_ioport m_options_port;

	devcb_write_line m_rdy_cb;
	devcb_write_line m_drq_cb;

	// state machine
	emu_timer *m_step;
	enum re_state : unsigned
	{
		IDLE,
		EXECUTE,
		DMA_R,
		DMA_W,
	}
	m_state;
	bool m_ir_pending;

	// line state
	bool m_rdy;
	bool m_drq;

	// registers
	u32 m_reg[64];

	// active command state
	bool m_enabrgb;
	bool m_bigendian;
	u32 m_func[4];
	// haddr
	bool m_nopup;
	// xyfrac
	unsigned m_pupdata;
	u32 m_pat;
	s32 m_dz;
	s32 m_dr;
	s32 m_dg;
	s32 m_db;
	s64 m_z;
	u32 m_r;
	u32 m_g;
	u32 m_b;
	u16 m_stip;
	u8 m_stipcount;
	s32 m_dx;
	s32 m_dy;
	unsigned m_numpix;
	u32 m_x;
	u32 m_y;
	unsigned m_ir;

	rectangle m_clip;

	std::unique_ptr<u32[]> m_vram;
	std::unique_ptr<u32[]> m_dram;
	u32 m_vram_mask;
	u32 m_dram_mask;
};

DECLARE_DEVICE_TYPE(SGI_RE2, sgi_re2_device)

#endif // MAME_SGI_SGI_RE2_H
