// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Tatsumi TX-1/Buggy Boy video hardware

****************************************************************************/

#include "emu.h"
#include "render.h"
#include "video/resnet.h"
#include "cpu/i86/i86.h"
#include "includes/tx1.h"


#define OBJ_FRAC    16

/*************************************
 *
 *  HD46505S-2 CRT Controller
 *
 *************************************/

#define CURSOR_YPOS 239
#define CURSOR_XPOS 168
#define PRINT_CRTC_DATA 0

/*
    6845 cursor output is connected to the main CPU interrupt pin.
    The CRTC is programmed to provide a rudimentary VBLANK interrupt.

    TODO: Calc TX-1 values...
*/

/*
    TODO: Check interrupt timing from CRT config. Probably different between games.
*/
TIMER_CALLBACK_MEMBER(tx1_state::interrupt_callback)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
	m_interrupt_timer->adjust(m_screen->time_until_pos(CURSOR_YPOS, CURSOR_XPOS));
}


READ16_MEMBER(tx1_state::tx1_crtc_r)
{
	return 0xffff;
}

WRITE16_MEMBER(tx1_state::tx1_crtc_w)
{
if (PRINT_CRTC_DATA)
{
	data &= 0xff;
	if (offset == 0)
	{
		switch (data)
		{
			case 0x00: osd_printf_debug("Horizontal Total         "); break;
			case 0x01: osd_printf_debug("Horizontal displayed     "); break;
			case 0x02: osd_printf_debug("Horizontal sync position "); break;
			case 0x03: osd_printf_debug("Horizontal sync width    "); break;
			case 0x04: osd_printf_debug("Vertical total           "); break;
			case 0x05: osd_printf_debug("Vertical total adjust    "); break;
			case 0x06: osd_printf_debug("Vertical displayed       "); break;
			case 0x07: osd_printf_debug("Vertical sync position   "); break;
			case 0x08: osd_printf_debug("Interlace mode           "); break;
			case 0x09: osd_printf_debug("Max. scan line address   "); break;
			case 0x0a: osd_printf_debug("Cursror start            "); break;
			case 0x0b: osd_printf_debug("Cursor end               "); break;
			case 0x0c: osd_printf_debug("Start address (h)        "); break;
			case 0x0d: osd_printf_debug("Start address (l)        "); break;
			case 0x0e: osd_printf_debug("Cursor (h)               "); break;
			case 0x0f: osd_printf_debug("Cursor (l)               "); break;
			case 0x10: osd_printf_debug("Light pen (h))           "); break;
			case 0x11: osd_printf_debug("Light pen (l)            "); break;
		}
	}
	else if (offset == 1)
	{
		osd_printf_debug("0x%.2x, (%d)\n",data, data);
	}
}
}


/***************************************************************************

  TX-1

***************************************************************************/

enum
{
	TX1_RDFLAG_RVA8 = 0,
	TX1_RDFLAG_RVA9,
	TX1_RDFLAG_RVA7,
	TX1_RDFLAG_TNLF,
	TX1_RDFLAG_STLF,
	TX1_RDFLAG_SCCHGF
};



/***************************************************************************

  Palette initialisation

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1.0kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(tx1_state,tx1)
{
	const UINT8 *const color_prom = &m_proms[0];
	int i;

	static const res_net_info tx1_net_info =
	{
		RES_NET_VCC_5V | RES_NET_VIN_TTL_OUT,
		{
			{ RES_NET_AMP_NONE, 0, 0, 4, { 2200, 1000, 470, 220 } },
			{ RES_NET_AMP_NONE, 0, 0, 4, { 2200, 1000, 470, 220 } },
			{ RES_NET_AMP_NONE, 0, 0, 4, { 2200, 1000, 470, 220 } }
		}
	};

	for (i = 0; i < 256; ++i)
	{
		int r, g, b;

		r = compute_res_net(color_prom[i + 0x300] & 0xf, 0, tx1_net_info);
		g = compute_res_net(color_prom[i + 0x400] & 0xf, 1, tx1_net_info);
		b = compute_res_net(color_prom[i + 0x500] & 0xf, 2, tx1_net_info);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*************************************
 *
 *  Video Control Registers
 *
 *************************************/

WRITE16_MEMBER(tx1_state::tx1_bankcs_w)
{
	vregs_t &tx1_vregs = m_vregs;

	// AAB2 = /BASET0
	// AAB3 = /BASET
	// AAB4 = /BSET
	// AAB5 = /HASET
	// AAB6 = /HSET

	offset <<= 1;

	if (offset & 0x04)
	{
		tx1_vregs.ba_inc &= ~0x0000ffff;
		tx1_vregs.ba_inc |= data;

		if (!(offset & 2))
			tx1_vregs.ba_val &= ~0x000ffff;
	}
	if (offset & 0x08)
	{
		data &= 0xff;
		tx1_vregs.ba_inc &= ~0xffff0000;
		tx1_vregs.ba_inc |= data << 16;

		// TODO: Schems say D15.
		tx1_vregs.bank_mode = BIT(data, 1);

		if (!(offset & 2))
			tx1_vregs.ba_val &= ~0xffff0000;
	}
	if ( !(offset & 0x10) )
	{
		/* Ignore data */
		if (offset & 2)
			tx1_vregs.ba_val = (tx1_vregs.ba_inc + tx1_vregs.ba_val) & 0x00ffffff;
	}
	if (offset & 0x20)
	{
		tx1_vregs.h_inc = data;

		if ( !(offset & 2) )
			tx1_vregs.h_val = 0;
	}
	if (!(offset & 0x40))
	{
		/* TODO: Looks safe to remove this */
//      if ( offset & 2 )
			tx1_vregs.h_val += tx1_vregs.h_inc;
	}
}

WRITE16_MEMBER(tx1_state::tx1_slincs_w)
{
	if (offset == 1)
		m_vregs.slin_inc = data;
	else
		m_vregs.slin_inc = m_vregs.slin_val = 0;
}

WRITE16_MEMBER(tx1_state::tx1_slock_w)
{
	m_vregs.slock = data & 1;
}

WRITE16_MEMBER(tx1_state::tx1_scolst_w)
{
	m_vregs.scol = data & 0x0707;
}

WRITE16_MEMBER(tx1_state::tx1_flgcs_w)
{
	m_vregs.flags = data & 0xff;
}


/*************************************
 *
 *  Characters
 *
 *************************************/

void tx1_state::tx1_draw_char(UINT8 *bitmap)
{
	UINT16 *tx1_vram = m_vram;
	INT32 x, y;
	UINT32 scroll_x;

	/* 2bpp characters */
	const UINT8 *const chars = &m_char_tiles[0];
	const UINT8 *const gfx2 = &m_char_tiles[0x4000];

	/* X scroll value is the last word in char RAM */
	scroll_x = tx1_vram[0xfff] & 0x3ff;

	for (y = 0; y < 240; ++y)
	{
		UINT32 d0 = 0, d1 = 0;
		UINT32 colour = 0;
		UINT32 y_offs;
		UINT32 x_offs;
		UINT32 y_gran;

		/* No y-scrolling? */
		y_offs = y;

		if ((y_offs >= 64) && (y_offs < 128))
			x_offs = m_vregs.slock ? scroll_x : 0;
		else
			x_offs = 0;

		y_gran = y_offs & 7;

		if (x_offs & 7)
		{
			UINT32 tilenum;
			UINT16 ram_val = tx1_vram[((y_offs << 4) & 0xf80) + ((x_offs >> 3) & 0x7f)];

			tilenum = (ram_val & 0x03ff) | ((ram_val & 0x8000) >> 5);
			colour = (ram_val & 0xfc00) >> 8;
			d0 = *(gfx2 + (tilenum << 3) + y_gran);
			d1 = *(chars + (tilenum << 3) + y_gran);
		}

		for (x = 0; x < 256 * 3; ++x)
		{
			UINT32 x_gran = x_offs & 7;

			if (!x_gran)
			{
				UINT32 tilenum;
				UINT16 ram_val = tx1_vram[((y_offs << 4) & 0xf80) + ((x_offs >> 3) & 0x7f)];

				tilenum = (ram_val & 0x03ff) | ((ram_val & 0x8000) >> 5);
				colour = (ram_val & 0xfc00) >> 8;
				d0 = *(gfx2 + (tilenum << 3) + y_gran);
				d1 = *(chars + (tilenum << 3) + y_gran);
			}

			*bitmap++ = colour |
						(((d1 >> (7 ^ x_gran)) & 1) << 1) |
						((d0 >> (7 ^ x_gran)) & 1);

			x_offs = (x_offs + 1) & 0x3ff;
		}
	}
}


/*************************************
 *
 *  Road
 *
 *************************************/

#define TX1_GET_ROADPIX(NUM) \
{ \
	UINT32 addr = (rva6_0 << 5) | (road##NUM##_hcnt & 0x1f); \
	UINT8 promaddr1, promaddr2, promaddr3; \
	promaddr1 = rom_a[addr]; \
	promaddr2 = rom_b[addr]; \
	promaddr3 = rom_c[addr]; \
	pix[NUM][0][0] = prom_a[promaddr1]; pix[NUM][0][1] = prom_b[promaddr1]; pix[NUM][0][2] = prom_c[promaddr1]; \
	pix[NUM][1][0] = prom_a[promaddr2]; pix[NUM][1][1] = prom_b[promaddr2]; pix[NUM][1][2] = prom_c[promaddr2]; \
	pix[NUM][2][0] = prom_a[promaddr3]; pix[NUM][2][1] = prom_b[promaddr3]; pix[NUM][2][2] = prom_c[promaddr3]; \
	pix[NUM][3][0] = prom_a[0];         pix[NUM][3][1] = prom_b[0];         pix[NUM][3][2] = prom_c[0]; \
}

void tx1_state::tx1_draw_road_pixel(int screen, UINT8 *bmpaddr,
									UINT8 apix[3], UINT8 bpix[3], UINT32 pixnuma, UINT32 pixnumb,
									UINT8 stl, UINT8 sld, UINT8 selb,
									UINT8 bnk, UINT8 rorev, UINT8 eb, UINT8 r, UINT8 delr)
{
	vregs_t &tx1_vregs = m_vregs;
	UINT8 a0 = BIT(apix[0], pixnuma);
	UINT8 a1 = BIT(apix[1], pixnuma);
	UINT8 a2 = BIT(apix[2], pixnuma);

	UINT8 b0 = BIT(bpix[0], pixnumb);
	UINT8 b1 = BIT(bpix[1], pixnumb);
	UINT8 b2 = BIT(bpix[2], pixnumb);

	UINT8 d3;
	UINT8 d2;
	UINT8 d1;
	UINT8 d0;
	UINT8 sel;
	UINT8 c6 = BIT(sld, 6);
	UINT8 c5 = BIT(sld, 5);
	UINT8 c4 = BIT(sld, 4);
	UINT8 c3 = BIT(sld, 2);

	UINT32 addr_offset = screen * 256;

	sel = !bnk && ( (a2 && !b0) || (!a0 && b2) || !a1 || !a2 || !b1 || !b2 );

	d3 =
		(a2 && a1 && a0 && b2 && b1 && b0 && !rorev)
		|| (a1 && b1 && !b0 && stl && !bnk && !c6)
		|| (a1 && !a0 && b1 && stl && !bnk && !c6)
		|| (a1 && !b2 && b1 && stl && !bnk && !c6)
		|| (a2 && b2 && !b1 && stl && !bnk && !c4)
		|| (a2 && !a1 && b2 && stl && !bnk && !c4)
		|| (!a2 && a1 && b2 && stl && !bnk && !c6)
		|| (a2 && !b2 && b1 && stl && !bnk && !c6)
		|| (!a2 && !a1 && stl && !bnk && !c5)
		|| (!b2 && !b1 && stl && !bnk && !c5)
		|| (!b0 && !stl && !bnk && !c3)
		|| (!b1 && !stl && !bnk && !c3)
		|| (!a0 && !stl && !bnk && !c3)
		|| (!a1 && !stl && !bnk && !c3)
		|| (!a2 && !stl && !bnk && !c3)
		|| (!b2 && !stl && !bnk && !c3)
		|| (bnk && !rorev);

	if (eb == 0)
	{
		if (sel == 1)
		{
			d2 = stl && (!a2 || !b2);
			d1 = !stl || (!a2 && !a1) || (!b1 && !b2) || (a2 && !b1) || (b2 && !a1);
			d0 = !stl
				|| (!a2 && !a0 && b1)
				|| (a1 && !b2 && !b0)
				|| (!a2 && !a1 && !a0)
				|| (!b2 && !b1 && !b0)
				|| (a2 && a1 && !b0)
				|| (!a0 && b2 && b1)
				|| (a2 && !b1 && !b0)
				|| (!a1 && !a0 && b2)
				|| (!a2 && !a0 && b2)
				|| (a2 && !b2 && !b0);
		}
		else
		{
			d2 = BIT(tx1_vregs.scol, selb ? 2 : 10);
			d1 = BIT(tx1_vregs.scol, selb ? 1 : 9);
			d0 = BIT(tx1_vregs.scol, selb ? 0 : 8);
		}
	}
	else
		d2 = d1 = d0 = 0;

	*(bmpaddr + addr_offset) = (bnk << 6) | (r << 5) | (!(sel && delr) << 4 ) | (d3 << 3) | (d2 << 2) | (d1 << 1) | d0;
}

/* This could do with a tidy up and more comments... */
void tx1_state::tx1_draw_road(UINT8 *bitmap)
{
	UINT16 *tx1_rcram = m_rcram;
	vregs_t &tx1_vregs = m_vregs;
	INT32   y;
	UINT32  rva9_8;
	UINT32  rva7;
	UINT32  tnlf;
	UINT32  stlf;
	UINT32  scchgf;

	UINT32  vc = 0;

	UINT16  road0_hcnt;
	UINT8   road0_pcnt;
	UINT16  road1_hcnt;
	UINT8   road1_pcnt;
	UINT8   pix[2][4][3];

	/* Road slice map ROMs */
	const UINT8 *const rom_a = &m_road_rom[0];
	const UINT8 *const rom_b = &m_road_rom[0x2000];
	const UINT8 *const rom_c = &m_road_rom[0x4000];

	/* Pixel data */
	const UINT8 *const prom_a = &m_proms[0x1100];
	const UINT8 *const prom_b = &m_proms[0x1300];
	const UINT8 *const prom_c = &m_proms[0x1500];
	const UINT8 *const vprom  = &m_proms[0x1700];

	rva9_8  = (tx1_vregs.flags & 3) << 8;
	rva7    = !BIT(tx1_vregs.flags, TX1_RDFLAG_RVA7) << 7;
	tnlf    = BIT(tx1_vregs.flags, TX1_RDFLAG_TNLF);
	scchgf  = BIT(tx1_vregs.flags, TX1_RDFLAG_SCCHGF);
	stlf    = BIT(tx1_vregs.flags, TX1_RDFLAG_STLF);

	for (y = 0; y < 240; ++y)
	{
		UINT32  x;
		UINT8   sld;
		UINT8   rva6_0;
		UINT16  rcrdb15_0;
		UINT16  rva_addr;
		UINT32  bank_cnt;

		UINT16  vat;
		UINT16  vp0;
		UINT16  vp1;
		UINT16  vp2;
		UINT16  vp3;
		UINT16  vp4;

		UINT32  va8, va9, va10, va11, va12, va13, va14, va15;
		UINT32  v0, v1, v2;

		UINT8   hc1_u, hc1_l;
		UINT8   hc0_u, hc0_l;

		UINT32  bnkls, bnkcs, bnkrs;
		UINT32  rl, rc, rr;

		UINT32  stl;
		UINT32  selb;
		UINT32  ebls, ebcs, ebrs;

		UINT8 *bmpaddr = bitmap + (y * 768);

		UINT32 rltmp, rctmp, rrtmp;
		UINT32 eltmp, ectmp, ertmp;

		UINT32 tmpm;
		UINT32 tmpn;
		UINT32 tmpc;
		UINT32 tmpd;

		UINT8 scrcnt0;
		UINT8 scrcnt1;

		UINT8 rorevls = 0;
		UINT8 rorevcs = 0;
		UINT8 rorevrs = 0;

		int febl[2] = { 0, 0 };
		int febr[2] = { 0, 0 };
		int febc[2] = { 0, 0 };

		/* Road vertical address */
		if (BIT(tx1_vregs.h_val, 15))
			rva6_0 = 0x7f;
		else
			rva6_0 = (~tx1_vregs.h_val >> 7) & 0x7f;

		rva_addr = rva9_8 | rva6_0;

		/* Get the stripe data also */
		sld = vprom[rva6_0] + tx1_vregs.slin_val;

		/* Load the left road h-counter */
		rcrdb15_0 = tx1_rcram[(0x80 + rva_addr) ^ 0x7ff];
		road0_pcnt = rcrdb15_0 & 0x7;
		road0_hcnt = (rcrdb15_0 >> 3) & 0x7f;
		road0_hcnt |= rcrdb15_0 & 0xfc00 ? 0x80 : 0;
		road0_hcnt |= (rcrdb15_0 & 0xfc00) == 0xfc00 ? 0x100 : 0x000;

		/* Load the second road h-counter */
		rcrdb15_0 = tx1_rcram[(rva7 + rva_addr) ^ 0x7ff];
		road1_pcnt = rcrdb15_0 & 0x7;
		road1_hcnt = (rcrdb15_0 >> 3) & 0x7f;
		road1_hcnt |= rcrdb15_0 & 0xfc00 ? 0x80 : 0;
		road1_hcnt |= (rcrdb15_0 & 0xfc00) == 0xfc00 ? 0x100 : 0x000;
		road1_hcnt |= (rcrdb15_0 & 0x800);

		scrcnt0 = (road0_hcnt >> 5) & 0xf;
		scrcnt1 = (road1_hcnt >> 5) & 0xf;

		if (road0_pcnt & 7)
		{
			TX1_GET_ROADPIX(0);

			febl[0] = (scrcnt0 + 0) & 0xc;
			febc[0] = (scrcnt0 + 1) & 0xc;
			febr[0] = (scrcnt0 + 2) & 0xc;
		}

		if (road1_pcnt & 7)
		{
			UINT32 y4 = (road1_hcnt >> 4) & 1;
			UINT32 temp = (road1_hcnt >> 5) & 0xf;
			UINT32 x, u;
			UINT32 fl11 = BIT(road1_hcnt, 15);

			TX1_GET_ROADPIX(1);

			x = (temp & 0xc);
			u = !(((temp & 1) && y4) || (temp & 2));
			rorevls = ((fl11 && !x) || (fl11 && x && u));
			febl[1] = x;

			temp += 1;
			x = (temp & 0xc);
			u = !(((temp & 1) && y4) || (temp & 2));
			rorevcs = ((fl11 && !x) || (fl11 && x && u));
			febc[1] = x;

			temp += 1;
			x = (temp & 0xc);
			u = !(((temp & 1) && y4) || (temp & 2));
			rorevrs = ((fl11 && !x) || (fl11 && x && u));
			febr[1] = x;
		}

		/* Load the bank counter with accumulator bits 14-5 */
		bank_cnt = (tx1_vregs.ba_val >> 5) & 0x3ff;

		/* Load vertical position data */
		vat = tx1_rcram[(rva9_8 + 7) ^ 0x7f8];
		vp0 = tx1_rcram[(rva9_8 + 6) ^ 0x7f8];
		vp1 = tx1_rcram[(rva9_8 + 5) ^ 0x7f8];
		vp2 = tx1_rcram[(rva9_8 + 4) ^ 0x7f8];
		vp3 = tx1_rcram[(rva9_8 + 3) ^ 0x7f8];
		vp4 = tx1_rcram[(rva9_8 + 2) ^ 0x7f8];

		/*  */
		if (y-1 == vp0) vc++;
		if (y-1 == vp1) vc++;
		if (y-1 == vp2) vc++;
		if (y-1 == vp3) vc++;
		if (y-1 == vp4) vc++;

		/* */
		va8 = BIT(vat, 8);
		va9 = BIT(vat, 9);
		va10 = BIT(vat, 10);
		va11 = BIT(vat, 11);
		va12 = BIT(vat, 12);
		va13 = BIT(vat, 13);
		va14 = BIT(vat, 14);
		va15 = BIT(vat, 15);

		v0 = BIT(vc, 0);
		v1 = BIT(vc, 1);
		v2 = BIT(vc, 2);

		/* Load the horizontal tunnel position counters */
		hc1_u = tx1_rcram[(rva9_8 + 1) ^ 0x7f8] >> 8;
		hc1_l = tx1_rcram[(rva9_8 + 1) ^ 0x7f8] & 0xff;
		hc0_u = tx1_rcram[(rva9_8 + 0) ^ 0x7f8] >> 8;
		hc0_l = tx1_rcram[(rva9_8 + 0) ^ 0x7f8] & 0xff;

		stl = !stlf || v0 || (!v2 && !scchgf) || scchgf;
		selb = (!v2 && !scchgf) || v2;

		rltmp =
		(v2 && !v0 && !va10)
		|| (!v2 && v0 && !va10)
		|| (v2 && !v0 && !va11)
		|| (v2 && !v1 && !v0)
		|| (!v2 && v0 && !va11)
		|| (!v2 && v1 && !v0)
		|| (!v2 && !v1 && v0);

		rctmp =
		(v2 && !v0 && va9 && va8)
		|| (!v2 && v0 && va9 && va8)
		|| (v2 && !v0 && !va11)
		|| (v2 && !v1 && !v0)
		|| (!v2 &&  v0 && !va11)
		|| (!v2 &&  v1 && !v0)
		|| (!v2 && !v1 &&  v0);

		rrtmp =
		(v2 && !v0 && !va11 && !va10)
		|| (!v2 && v0 && !va11 && !va10)
		|| (v2 && !v0 && va9)
		|| (!v2 && v0 && va9)
		|| (v2 && !v1 && !v0)
		|| (!v2 && v1 && !v0)
		|| (!v2 && !v1 && v0);

		ectmp =
		(v2 && !v0 && va13 && va12)
		|| (!v2 && v1 && va13 && va12)
		|| (!v2 && !v1 && v0)
		|| (v2 && !v0 && !va15)
		|| (!v2 && v1 && !va15);

		eltmp =
		(v2 && !v0 && !va14)
		|| (!v2 && v1 && !va14)
		|| (!v2 && !v1 && v0)
		|| (v2 && !v0 && !va15)
		|| (!v2 && v1 && !va15);

		ertmp =
		(v2 && !v0 && !va15 && !va14)
		|| (!v2 && v1 && !va15 && !va14)
		|| (v2 && !v0 && va13)
		|| (!v2 && v1 && va13)
		|| (!v2 && !v1 && v0);

		tmpn = (v2 && !v0) || (!v2 && v0);
		tmpm = (v2 && !v0) || (!v2 && v1);
		tmpc = (!v1 && !v0) || (v2);
		tmpd = v1 && v0 && va11;

		for (x = 0; x < 256; x++)
		{
			UINT32 pixnum0 = (road0_pcnt & 7) ^ 7;
			UINT32 pixnum1 = (road1_pcnt & 7) ^ 7;

			UINT32 cyu, cyl;
			UINT32 delrl, delrc, delrr;

			/* */
			scrcnt0 = (road0_hcnt >> 5) & 0xf;
			scrcnt1 = (road1_hcnt >> 5) & 0xf;

			/* Get new pixel data? */
			if (!(road0_pcnt & 7))
			{
				TX1_GET_ROADPIX(0);

				/* Road 0 enables */
				febl[0] = (scrcnt0 + 0) & 0xc;
				febc[0] = (scrcnt0 + 1) & 0xc;
				febr[0] = (scrcnt0 + 2) & 0xc;
			}

			if (!(road1_pcnt & 7))
			{
				UINT32 y4 = (road1_hcnt >> 4) & 1;
				UINT32 temp = (road1_hcnt >> 5) & 0xf;
				UINT32 x, u;
				UINT32 fl11 = BIT(road1_hcnt, 15);

				TX1_GET_ROADPIX(1);

				x = (temp & 0xc);
				u = !(((temp & 1) && y4) || (temp & 2));
				rorevls = ((fl11 && !x) || (fl11 && x && u));
				febl[1] = x;

				temp += 1;
				x = (temp & 0xc);
				u = !(((temp & 1) && y4) || (temp & 2));
				rorevcs = ((fl11 && !x) || (fl11 && x && u));
				febc[1] = x;

				temp += 1;
				x = (temp & 0xc);
				u = !(((temp & 1) && y4) || (temp & 2));
				rorevrs = ((fl11 && !x) || (fl11 && x && u));
				febr[1] = x;
			}

			/* Road camber/banking */
			if (BIT(tx1_vregs.ba_val, 23))
			{
				bnkls = 1;
				bnkcs = 1;
				bnkrs = 1;
			}
			else if (tx1_vregs.ba_val & 0x007f8000)
			{
				bnkls = 0;
				bnkcs = 0;
				bnkrs = 0;
			}
			else
			{
				bnkls = bank_cnt < 0x400;
				bnkcs = bank_cnt < 0x300;
				bnkrs = bank_cnt < 0x200;
			}

			if (tx1_vregs.bank_mode)
			{
				bnkls ^= 1;
				bnkcs ^= 1;
				bnkrs ^= 1;
			}

			cyu = hc1_u == 0xff;
			cyl = hc1_l == 0xff;

			rl = tnlf && ( rltmp || (tmpn && (!cyl || (va8 && va9 && cyu))) );
			rc = tnlf && ( rctmp || (tmpn && ((!va10 && !cyl) || (va9 && cyu))) );
			rr = tnlf && ( rrtmp || (tmpn && ((!va11 && !cyl) || (va8 && cyu))) );

			/* Evaluates to 0 for tunnels */
			delrl = !tnlf || tmpc || ((tmpd && va10) && ((!va8 && cyl) || (!va9 && cyl) || (!cyu && !cyl)));

			delrr =
				!tnlf
				|| tmpc
				|| (v1 && v0 && va10 && !va9 && !va8 && cyl)
				|| (v1 && v0 && va10 && !va9 && cyl && !cyu)
				|| (tmpd && !va9 && !cyu)
				|| (tmpd && !va9 && !va8);

			delrc = !tnlf || tmpc || (tmpd && ((!va8 && cyl && !cyu) || (!va8 && va10 && !cyu) || (!va9 && cyl) || (va10 && !va9)));

			cyu = hc0_u == 0xff;
			cyl = hc0_l == 0xff;

			ebls = tnlf && ( eltmp || (tmpm && (!cyl || (va12 && va13 && cyu))));
			ebcs = tnlf && ( ectmp || (tmpm && ((!va14 && !cyl) || (va13 && cyu))));
			ebrs = tnlf && ( ertmp || (tmpm && ((!va15 && !cyl) || (va12 && cyu))));

			/* Carry for horizontal position counters */
			cyu = hc0_u == 0xff;
			cyl = hc0_l == 0xff;

			if (!(bnkls && !rl))
			{
				int a, b;

				if (!febl[0])
					a = (scrcnt0) & 3;
				else
					a = 3;

				if (!febl[1])
					b = (scrcnt1) & 3;
				else
					b = 3;

				tx1_draw_road_pixel(0, bmpaddr,
							&pix[0][a][0], &pix[1][b][0],
							pixnum0, pixnum1,
							stl, sld, selb, bnkls, rorevls, ebls, rl, delrl);
			}
			else
				*(bmpaddr) = (bnkls << 6) | (rl << 5);

			if (!(bnkcs && !rc))
			{
				int a, b;

				if (!febc[0])
					a = (scrcnt0 + 1) & 3;
				else
					a = 3;

				if (!febc[1])
					b = (scrcnt1 + 1) & 3;
				else
					b = 3;

				tx1_draw_road_pixel(1, bmpaddr,
							&pix[0][a][0], &pix[1][b][0],
							pixnum0, pixnum1,
							stl, sld, selb, bnkcs, rorevcs, ebcs, rc, delrc);
			}
			else
				*(bmpaddr + 256) = (bnkcs << 6) | (rc << 5);

			if (!(bnkrs && !rr))
			{
				int a, b;

				if (!febr[0])
					a = (scrcnt0 + 2) & 3;
				else
					a = 3;

				if (!febr[1])
					b = (scrcnt1 + 2) & 3;
				else
					b = 3;

				tx1_draw_road_pixel(2, bmpaddr,
							&pix[0][a][0], &pix[1][b][0],
							pixnum0, pixnum1,
							stl, sld, selb, bnkrs, rorevrs, ebrs, rr, delrr);
			}
			else
				*(bmpaddr + 512) = (bnkrs << 6) | (rr << 5);

			++bmpaddr;

			/* Update road counters */
			if (!(++road0_pcnt & 7))
				++road0_hcnt;

			if (!(++road1_pcnt & 7))
				++road1_hcnt;

			/* Increment horizontal counters (stop at 0xff) */
			if (hc0_u != 0xff) ++hc0_u;
			if (hc0_l != 0xff) ++hc0_l;
			if (hc1_u != 0xff) ++hc1_u;
			if (hc1_l != 0xff) ++hc1_l;

			/* Update bank counter */
			bank_cnt = (bank_cnt + 1) & 0x7ff;
		}

		tx1_vregs.h_val += tx1_vregs.h_inc;

		/* Finally, increment the bank accumulator */
		tx1_vregs.ba_val = (tx1_vregs.ba_val + tx1_vregs.ba_inc) & 0x00ffffff;
	}

}

/*************************************
 *
 *  Objects
 *
 *************************************/

void tx1_state::tx1_draw_objects(UINT8 *bitmap)
{
	UINT16 *tx1_objram = m_objram;

	UINT32 offs;

	/* The many lookup table ROMs */
	const UINT8 *const ic48 = &m_obj_luts[0];
	const UINT8 *const ic281 = &m_obj_luts[0x2000];

	const UINT8 *const ic190 = &m_proms[0xc00];
	const UINT8 *const ic162 = &m_proms[0xe00];
	const UINT8 *const ic25  = &m_proms[0x1000];

	const UINT8 *const ic106 = &m_obj_map[0];
	const UINT8 *const ic73  = &m_obj_map[0x4000];

	const UINT8 *const pixdata_rgn = &m_obj_tiles[0];

	for (offs = 0x0; offs <= 0x300; offs += 8)
	{
		UINT32  x;
		UINT32  y;
		UINT32  gxflip;

		UINT32  x_scale;
		UINT32  x_step;
		UINT16  y_scale;
		UINT16  y_step;

		UINT8   pctmp0_7;
		UINT8   code;

		/* Check for end of object list */
		if ((tx1_objram[offs] & 0xff00) == 0xff00)
			break;

		/* X scale */
		x_scale = tx1_objram[offs + 2] & 0xff;

		/* TODO: Confirm against hardware? */
		if (x_scale == 0)
			continue;

		/* 16-bit y-scale accumulator */
		y_scale = tx1_objram[offs + 1];
		y_step  = tx1_objram[offs + 3];

		/* Object number */
		code = tx1_objram[offs] & 0xff;

		/* Attributes */
		pctmp0_7 = tx1_objram[offs + 2] >> 8;

		/* Global x-flip */
		gxflip = (pctmp0_7 & 0x80) >> 7;

		/* Add 1 to account for line buffering */
		y = (tx1_objram[offs] >> 8) + 1;

		for (; y < 240; ++y)
		{
			UINT32  rom_addr2   = 0;
			UINT8   ic106_data  = 0;
			UINT8   ic73_data;

			/* Are we drawing on this line? */

			/* TODO: See big lampposts. */
			if (y_scale & 0x8000)
				break;

			{
				UINT32  psa0_11;
				UINT32  ic48_addr;
				UINT32  ic48_data;
				UINT32  rom_addr;
				UINT32  x_acc;
				UINT32  newtile = 1;
				UINT32  dataend = 0;
				UINT8   data1 = 0;
				UINT8   data2 = 0;
				UINT32  xflip = 0;
				UINT32  opcd0_7 = 0;
				UINT32  lasttile = 0;

				/* Use the object code to lookup the tile sequence data in ROM */
				ic48_addr = code << 4;
				ic48_addr |= ((y_scale >> 11) & 0xf);
				ic48_data = ic48[ic48_addr];

				/* Reached the bottom of the object? (/PASS2E) */
				if (ic48_data == 0xff)
					break;

				/* Combine ROM and PROM data */
				psa0_11 = ((ic25[code] << 8) | ic48_data) & 0xfff;

				/* psa8_11 */
				rom_addr = (psa0_11 & ~0xff) << 2;

				/* Prepare the x-scaling */
				x_step = (128 << OBJ_FRAC) / x_scale;
				x_acc = (psa0_11 & 0xff) << (OBJ_FRAC + 5);

#define TX1_MASK    0xfff

				x = tx1_objram[offs + 4] & TX1_MASK;

				for (;;)
				{
					if (newtile)
					{
						UINT32  psbb0_12;
						UINT32  pscb0_14;
						UINT32  pscb11;
						UINT8   *romptr;
						UINT32  ic281_addr;
						UINT32  grom_addr;
						UINT32  lut_data;
						UINT32  low_addr = ((x_acc >> (OBJ_FRAC + 3)) & TX1_MASK);

						if (gxflip)
						{
							UINT32 xor_mask;

							if (BIT(psa0_11, 11) && BIT(psa0_11, 10))
								xor_mask = 0xf;
							else if (BIT(psa0_11, 11) || BIT(psa0_11, 10) || BIT(psa0_11, 9))
								xor_mask = 0x7;
							else
								xor_mask = 0x3;

							rom_addr2 = rom_addr + (xor_mask ^ low_addr);
						}
						else
							rom_addr2 = rom_addr + low_addr;

						ic106_data = ic106[rom_addr2 & 0x3fff];

						if ((ic106_data & 0x40) && dataend)
							lasttile = 1;

						dataend |= ic106_data & 0x40;

						/* Retrieve data for an 8x8 tile */
						ic73_data = ic73[rom_addr2 & 0x3fff];

						/* This is the data from the LUT pair */
						lut_data = (ic106_data << 8) | ic73_data;
						psbb0_12 = lut_data & 0x1fff;

						pscb0_14 = (psbb0_12 & 0xc3f);

						/* Bits 9_6 are from PCTMP11-8 or PSBB9-6 */
						if (BIT(psbb0_12, 12))
							pscb0_14 |= psbb0_12 & 0x3c0;
						else
							pscb0_14 |= (pctmp0_7 & 0xf) << 6;

						if (BIT(lut_data, 13))
							pscb0_14 |= BIT(psbb0_12, 10) << 12;
						else
							pscb0_14 |= ((pctmp0_7 & 0x70) << 8);

						/* Bit 12 is bit 10 duplicated. */
						pscb0_14 &= ~(1 << 12);
						pscb0_14 |= BIT(psbb0_12, 10) << 12;

						pscb11 = BIT(pscb0_14, 11);

						/* TODO: Remove this - it's constant. */
						romptr = (UINT8*)(pixdata_rgn + pscb11 * (0x4000 * 2));

						grom_addr = ((pscb0_14 << 3) | ((y_scale >> 8) & 7)) & 0x3fff;

						/* Get raw 8x8 2bpp pixel row data */
						data1 = *(grom_addr + romptr);
						data2 = *(grom_addr + romptr + 0x4000);

						/* Determine flip state (global XOR local) */
						xflip = gxflip ^ !BIT(lut_data, 15);

						ic281_addr = pscb0_14 & 0x3ff;
						ic281_addr |= ((pscb0_14 & 0x7000) >> 2);
						ic281_addr |= pscb11 << 13;

						opcd0_7 = ic281[ic281_addr];

						newtile = 0;
					}

					/* Draw a pixel? */
					if (x < 768)
					{
						UINT8   pix;
						UINT8   bit;

						bit = (x_acc >> OBJ_FRAC) & 7;

						if (xflip)
							bit ^= 7;

						pix = (((data1 >> bit) & 1) << 1) | ((data2 >> bit) & 1);

						/* Draw pixel, if not transparent */
						if ( !(!(opcd0_7 & 0x80) && !pix) )
						{
							UINT8 color;
							UINT32 prom_addr;

							prom_addr = ((opcd0_7 << 2) | pix) & 0x1ff;

							/* Inverted on schematic */
							if (x & 1)
								color = ~ic190[prom_addr] & 0x3f;
							else
								color = ~ic162[prom_addr] & 0x3f;

							*(bitmap + 768*y + x) = 0x40 | color;
						}
					}

					/* Check if we've stepped into a new 8x8 tile */

					if ((((x_acc + x_step) >> (OBJ_FRAC + 3)) & TX1_MASK) != ((x_acc >> (OBJ_FRAC + 3)) & TX1_MASK))
					{
						if (lasttile)
							break;

						newtile = 1;
					}

					x = (x + 1) & TX1_MASK;
					x_acc += x_step;
				}
			}
			y_scale += y_step;
		}
	}
}


/*************************************
 *
 *  Core Functions
 *
 *************************************/

VIDEO_START_MEMBER(tx1_state,tx1)
{
	/* Allocate a large bitmap that covers the three screens */
	m_bitmap = auto_bitmap_ind16_alloc(machine(), 768, 256);

	/* Allocate some bitmaps */
	m_chr_bmp = auto_alloc_array(machine(), UINT8, 256 * 3 * 240);
	m_obj_bmp = auto_alloc_array(machine(), UINT8, 256 * 3 * 240);
	m_rod_bmp = auto_alloc_array(machine(), UINT8, 256 * 3 * 240);

	/* Set a timer to run the interrupts */
	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(tx1_state::interrupt_callback),this));

	/* /CUDISP CRTC interrupt */
	m_interrupt_timer->adjust(m_screen->time_until_pos(CURSOR_YPOS, CURSOR_XPOS));
}

void tx1_state::screen_eof_tx1(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		/* /VSYNC: Update TZ113 */
		m_vregs.slin_val += m_vregs.slin_inc;

		m_needs_update = true;
	}
}

void tx1_state::tx1_combine_layers(bitmap_ind16 &bitmap, int screen)
{
	int x, y;
	UINT8 *chr_pal = &m_proms[0x900];

	int x_offset = screen * 256;

	for (y = 0; y < 240; ++y)
	{
		UINT16 *bmp_addr = &bitmap.pix16(y);

		UINT32 bmp_offset = y * 768 + x_offset;

		UINT8 *chr_addr = m_chr_bmp + bmp_offset;
		UINT8 *rod_addr = m_rod_bmp + bmp_offset;
		UINT8 *obj_addr = m_obj_bmp + bmp_offset;

		for (x = 0; x < 256; ++x)
		{
			UINT8 out_val;
			UINT32 char_val = chr_addr[x];
			UINT32 c7 = BIT(char_val, 7);
			UINT32 c1 = BIT(char_val, 1);
			UINT32 c0 = BIT(char_val, 0);

			UINT32 road_val = rod_addr[x];
			UINT32 r6 = BIT(road_val, 6);
			UINT32 r5 = BIT(road_val, 5);

			UINT32 obj_val = obj_addr[x];
			UINT32 obj6 = BIT(obj_val, 6);

			UINT32 term1 = !(c7 && c1);
			UINT32 term2 = !(c7 && c0);
			UINT32 term3 = r5 || !r6;
			UINT32 p12 = !(term1 && term2 && term3);
			UINT32 p6 = !(obj6 && term1 && term2);
			UINT32 sel =  p12 | (p6 << 1);

			UINT32 psel =  (!(p6 && p12) << 1) | p6;

			if      (sel == 3)  out_val = ((char_val & 0xc0) >> 2) | (chr_pal[char_val] & 0xf);
			else if (sel == 2)  out_val = road_val & 0x3f;
			else                out_val = obj_val & 0x3f;

			*bmp_addr++ = (psel << 6) | out_val;
		}
	}
}


void tx1_state::tx1_update_layers()
{
	memset(m_obj_bmp, 0, 768*240);

	tx1_draw_char(m_chr_bmp);
	tx1_draw_road(m_rod_bmp);
	tx1_draw_objects(m_obj_bmp);

	m_needs_update = false;
}

UINT32 tx1_state::screen_update_tx1_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_needs_update)
		tx1_update_layers();

	tx1_combine_layers(bitmap, 0);
	return 0;
}

UINT32 tx1_state::screen_update_tx1_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_needs_update)
		tx1_update_layers();

	tx1_combine_layers(bitmap, 1);
	return 0;
}

UINT32 tx1_state::screen_update_tx1_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_needs_update)
		tx1_update_layers();

	tx1_combine_layers(bitmap, 2);
	return 0;
}


/***************************************************************************

  Buggy Boy

***************************************************************************/

/* Road register bits */
#define BB_RDFLAG_WAVE1     7
#define BB_RDFLAG_WAVE0     6
#define BB_RDFLAG_TNLMD1    5
#define BB_RDFLAG_TNLMD0    4
#define BB_RDFLAG_TNLF      3
#define BB_RDFLAG_LINF      2
#define BB_RDFLAG_RVA7      1
#define BB_RDFLAG_WANGL     0

/***************************************************************************

  Convert the color PROMs into a more useable format.

  IC39, BB12 = Blue
  IC40, BB11 = Green
  IC41, BB10 = Red

  IC42, BB13 = Brightness

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1.0kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

  bit 0 -- 4.7kohm resistor  -- BLUE
  bit 1 -- 4.7kohm resistor  -- GREEN
  bit 2 -- 4.7kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT_MEMBER(tx1_state,buggyboy)
{
	const UINT8 *const color_prom = &m_proms[0];
	int i;

	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3, bit4;
		int r, g, b;

		bit0 = BIT(color_prom[i + 0x000], 0);
		bit1 = BIT(color_prom[i + 0x000], 1);
		bit2 = BIT(color_prom[i + 0x000], 2);
		bit3 = BIT(color_prom[i + 0x000], 3);
		bit4 = BIT(color_prom[i + 0x300], 2);
		r = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		bit0 = BIT(color_prom[i + 0x100], 0);
		bit1 = BIT(color_prom[i + 0x100], 1);
		bit2 = BIT(color_prom[i + 0x100], 2);
		bit3 = BIT(color_prom[i + 0x100], 3);
		bit4 = BIT(color_prom[i + 0x300], 1);
		g = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		bit0 = BIT(color_prom[i + 0x200], 0);
		bit1 = BIT(color_prom[i + 0x200], 1);
		bit2 = BIT(color_prom[i + 0x200], 2);
		bit3 = BIT(color_prom[i + 0x200], 3);
		bit4 = BIT(color_prom[i + 0x300], 0);
		b = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*************************************
 *
 *  Characters
 *
 *************************************/

void tx1_state::buggyboy_draw_char(UINT8 *bitmap, bool wide)
{
	UINT16 *buggyboy_vram = m_vram;
	INT32 x, y;
	UINT32 scroll_x, scroll_y;
	UINT32 total_width;
	UINT32 x_mask;

	/* 2bpp characters */
	const UINT8 *const chars = &m_char_tiles[0];
	const UINT8 *const gfx2 = &m_char_tiles[0x4000];

	/* X/Y scroll values are the last word in char RAM */
	if (wide)
	{
		scroll_y = (buggyboy_vram[0xfff] >> 10) & 0x3f;
		scroll_x = buggyboy_vram[0xfff] & 0x3ff;
		total_width = 768;
		x_mask = 0x3ff;
	}
	else
	{
		scroll_y = (buggyboy_vram[0x7ff] >> 10) & 0x3f;
		scroll_x = buggyboy_vram[0x7ff] & 0x1ff;
		total_width = 256;
		x_mask = 0x1ff;
	}

	for (y = 0; y < 240; ++y)
	{
		UINT32 d0 = 0, d1 = 0;
		UINT32 colour = 0;
		UINT32 y_offs;
		UINT32 x_offs;
		UINT32 y_gran;

		/* There's no y-scrolling between scanlines 0 and 63 */
		if (y < 64)
			y_offs = y;
		else
		{
			y_offs = (y + (scroll_y | 0xc0) + 1) & 0xff;

			/* Clamp */
			if (y_offs < 64)
				y_offs |= 0xc0;
		}

		if ((y_offs >= 64) && (y_offs < 128))
			x_offs = scroll_x;
		else
			x_offs = 0;


		y_gran = y_offs & 7;

		if (x_offs & 7)
		{
			UINT32 tilenum;
			UINT16 ram_val;

			if (wide)
				ram_val = buggyboy_vram[((y_offs << 4) & 0xf80) + ((x_offs >> 3) & 0x7f)];
			else
				ram_val = buggyboy_vram[((y_offs << 3) & 0x7c0) + ((x_offs >> 3) & 0x3f)];

			tilenum = (ram_val & 0x03ff) | ((ram_val & 0x8000) >> 5);
			colour = (ram_val & 0xfc00) >> 8;
			d0 = *(gfx2 + (tilenum << 3) + y_gran);
			d1 = *(chars + (tilenum << 3) + y_gran);
		}

		for (x = 0; x < total_width; ++x)
		{
			UINT32 x_gran = x_offs & 7;

			if (!x_gran)
			{
				UINT32 tilenum;
				UINT16 ram_val;

				if (wide)
					ram_val = buggyboy_vram[((y_offs << 4) & 0xf80) + ((x_offs >> 3) & 0x7f)];
				else
					ram_val = buggyboy_vram[((y_offs << 3) & 0x7c0) + ((x_offs >> 3) & 0x3f)];

				tilenum = (ram_val & 0x03ff) | ((ram_val & 0x8000) >> 5);
				colour = (ram_val & 0xfc00) >> 8;
				d0 = *(gfx2 + (tilenum << 3) + y_gran);
				d1 = *(chars + (tilenum << 3) + y_gran);
			}

			*bitmap++ = colour |
						(((d1 >> (7 ^ x_gran)) & 1) << 1) |
						((d0 >> (7 ^ x_gran)) & 1);

			x_offs = (x_offs + 1) & x_mask;
		}
	}
}


/***************************************************************************

  Buggy Boy Road Hardware

  A hacked up version of TX-1 but without the second road.

  There are two lists in road/common RAM (double buffered) starting at 0x800
  and 0xa00:

  0x1800 - 0x18ff:    Road line horizontal position word (128 entries).
  0x19e0 - 0x19ef:    Vertical positions (starting line, water, tunnels etc)
  0x19f0 - 0x19ff:    Horizontal positions (walls and tunnels)

  Three TZ1113 accumulators are used to vary:
  * Road camber (update per pixel)
  * Road vertical scale/position (update per scanline)
  * Road 'speed' (update per frame)

  Road flags register (0x24E0):
  7 : Water sparkle control 1
  6 : Water sparkle control 0 ('WAVE0,1')
  5 : Tunnel mode 1
  4 : Tunnel mode 0 ('TNLMD0,1')
  3 : Tunnel flag ('TNLF')
  2 : Starting Line flag ('LINF')
  1 : Road list select
  0 : Wall angle enable ('WANGL')

  Buggy Boy Jr. Road PAL equations:

  http://philwip.mameworld.info/buggyboy/PAL14H4.149.htm
  http://philwip.mameworld.info/buggyboy/PAL14L4.151.htm
  http://philwip.mameworld.info/buggyboy/PAL16H2.3.htm
  http://philwip.mameworld.info/buggyboy/PAL16L8.4.htm
  http://philwip.mameworld.info/buggyboy/PAL16L8.150.htm

***************************************************************************/

void tx1_state::buggyboy_get_roadpix(int screen, int ls161, UINT8 rva0_6, UINT8 sld, UINT32 *_rorev,
									UINT8 *rc0, UINT8 *rc1, UINT8 *rc2, UINT8 *rc3)
{
	/* Counter Q10-7 are added to 384 */
	UINT16 ls283_159 = (ls161 & 0x780) + 128 + (256 * screen);
	UINT32 ls283_159_co = ls283_159 & 0x800;
	UINT32 rom_flip = ls283_159 & 0x200 ? 0 : 1;
	UINT32 rom_en = !(ls283_159 & 0x400) && !(ls283_159_co ^ (ls161 & 0x800));
	UINT8 d0 = 0;
	UINT8 d1 = 0;

	/* ROM/PROM lookup tables */
	const UINT8 *const rom   = &m_road_rom[0];
	const UINT8 *const prom0 = &m_road_rom[0x4000];
	const UINT8 *const prom1 = &m_road_rom[0x4200];
	const UINT8 *const prom2 = &m_road_rom[0x4400];

	/* Latch road reverse bit */
	*_rorev = !( (rom_en && rom_flip) || (!rom_en && (ls161 & 0x4000)) );

	/* TODO: ROM data is 0xff if not enabled. */
	if (rom_en)
	{
		UINT8  rom_data;
		UINT16 prom_addr;

		/* 6 bit road horizontal address */
		UINT16 rha = (ls283_159 & 0x180) | (ls161 & 0x78);

		if (rom_flip)
			rha ^= 0x1f8;

		/* Get road chunk first */
		rom_data = rom[(1 << 13) | (rha << 4) | rva0_6];
		prom_addr = (rom_flip ? 0x80 : 0) | (rom_data & 0x7f);

		*rc0 = prom0[prom_addr];
		*rc1 = prom1[prom_addr];
		*rc2 = prom2[prom_addr];

		/* Now get the dirt chunk */
		rom_data = rom[(rha << 4) | rva0_6];
		prom_addr = 0x100 | rom_data;

		d0 = prom0[prom_addr];
		d1 = prom1[prom_addr];
	}
	else
	{
		/*
		    TODO: When ROM is not enabled, data = 0xff
		    But does anybody care?
		*/
		*rc0 = *rc1 = *rc2 = *rc3 = 0;
	}

	/* The data is mixed by two TZ0314 PALs */
	if (BIT(sld, 4))
	{
		if (BIT(sld, 5))
			d1 = ~d1;

		*rc3 = d0 & d1;

		if (rom_flip)
			*rc3 = BITSWAP8(*rc3, 0, 1, 2, 3, 4, 5, 6, 7);
	}
	else
		*rc3 = 0;
}

#define LOAD_HPOS_COUNTER(NUM)                                                  \
	ram_val = buggyboy_rcram[(rva_offs + 0x1f8 + (2*NUM)) >> 1];                \
	rcrs10 = ram_val & 0xfc00 ? 0x0400 : 0x0000;                                \
	hp = vregs.wa8 + ((BIT(ram_val, 15) << 11) | rcrs10 | (ram_val & 0x03ff));  \
	hp##NUM = hp & 0xff;                                                        \
	hp >>= 8;                                                                   \
	hps##NUM##0 = (BIT(hp, 0) || BIT(hp, 2)) && !BIT(hp, 3);                    \
	hps##NUM##1 = (BIT(hp, 1) || BIT(hp, 2)) && !BIT(hp, 3);                    \
	hps##NUM##2 = BIT(hp, 2);
#define UPDATE_HPOS(NUM)                \
	if (hp##NUM##_en)                   \
	{                                   \
		if ((hp##NUM & 0xff) == 0xff)   \
			hp##NUM##_cy = 1;           \
		else                            \
			hp##NUM = hp##NUM + 1;      \
	}
void tx1_state::buggyboy_draw_road(UINT8 *bitmap)
{
	UINT16 *buggyboy_rcram = m_rcram;
	vregs_t &vregs = m_vregs;
	INT32 x;
	UINT32 y;
	UINT16 rva_offs;
	UINT32 tnlmd0;
	UINT32 tnlmd1;
	UINT32 linf;
	UINT32 tnlf;
	UINT32 wangl;
	UINT32 tcmd;
	UINT32 wave0;
	UINT32 wave1;
	UINT32 rva20_6;

	/* ROM/PROM lookup tables */
	const UINT8 *const rcols = &m_proms[0x1500];
	const UINT8 *const vprom = &m_road_rom[0x4600];

	/* Extract constant values */
	tcmd     = ((vregs.scol & 0xc000) >> 12) | ((vregs.scol & 0x00c0) >> 6);
	tnlmd0   = BIT(vregs.flags, BB_RDFLAG_TNLMD0);
	tnlmd1   = BIT(vregs.flags, BB_RDFLAG_TNLMD1);
	linf     = BIT(vregs.flags, BB_RDFLAG_LINF);
	tnlf     = BIT(vregs.flags, BB_RDFLAG_TNLF);
	wangl    = BIT(vregs.flags, BB_RDFLAG_WANGL);
	wave0    = BIT(vregs.flags, BB_RDFLAG_WAVE0);
	wave1    = BIT(vregs.flags, BB_RDFLAG_WAVE1);
	rva_offs = BIT(vregs.flags, BB_RDFLAG_RVA7) ? 0x800 : 0xc00;

	for (y = 0; y < 240; ++y)
	{
		UINT8   rva0_6;
		UINT8   ram_addr;
		UINT16  rcrdb0_15;
		UINT16  rcrs10;
		UINT16  ls161_156_a;
		UINT16  ls161;
		UINT8   sld;
		UINT32  rva8;
		UINT32  rm0, rm1;
		UINT32  rcmd;
		UINT32  bnkls = 1;
		UINT32  bnkcs = 1;
		UINT32  bnkrs = 1;

//      UINT32  x_offs;
		UINT8 sf;

		/* Vertical positions shift register */
		UINT32  ram_val;
		UINT32  hp;
		UINT32  vp1, vp2, vp3, vp4, vp5, vp6, vp7;

		/* Horizontal positions */
		UINT32  hp0, hp1, hp2, hp3;
		UINT8   hps00, hps01, hps02;
		UINT8   hps10, hps11, hps12;
		UINT8   hps20, hps21, hps22;
		UINT8   hps30, hps31, hps32;

		/* Road pixel data planes */
		UINT8   rc0[3] = {0, 0, 0};
		UINT8   rc1[3] = {0, 0, 0};
		UINT8   rc2[3] = {0, 0, 0};
		UINT8   rc3[3] = {0, 0, 0};

		/* Horizontal position counter carry out */
		UINT8   hp0_cy = 0, hp1_cy = 0, hp2_cy = 0, hp3_cy = 0;

		UINT8   *bmpaddr = bitmap + (y * 256 * 3);

		UINT32  bank_cnt;
		UINT32  _rorevls = 0;
		UINT32  _rorevcs = 0;
		UINT32  _rorevrs = 0;

		UINT32  ic96_o17;
		UINT32  ic96_term1;
		UINT32  ic97_o12;
		UINT32  ic97_o13;
		UINT32  ic79_p19;

		rva8 = (vregs.h_val & 0x8000) || !(vregs.shift & 0x80);

		/* Get RVA0_6 from TZ113 accumulator chain @ 122/123 */
		rva0_6 = (vregs.h_val >> 7) & 0x7f;

		/* For /WAVE bit logic later */
		rva20_6 = ((rva0_6 >> 3) & 0xe) | ((rva0_6 & 2) >> 1);

		/* RVA is inverted! */
		ram_addr = (~rva0_6 & 0x7f) << 1;

		/* Get the road RAM data for this line */
		rcrdb0_15 = buggyboy_rcram[(rva_offs + ram_addr) >> 1];

		/* If 15-10 == 000000, then 0 */
		rcrs10 = rcrdb0_15 & 0xfc00 ? 0x0400 : 0x0000;

		/* If 15-10 == 111111, then 1 */
		ls161_156_a = (rcrdb0_15 & 0xfc00) == 0xfc00 ? 0x800 : 0x0000;

		/* LS161 15-bit counter chain - loaded with RAM data (bar bits 10-13) */
		ls161 =  ((rcrdb0_15 & 0x8000) >> 1) | ls161_156_a | rcrs10 | (rcrdb0_15 & 0x03ff);

		/* SLD */
		sld = (vprom[rva0_6] + vregs.slin_val) & 0x38;

		/* Determine the x-offset */
//      x_offs = ls161 & 7;

		/* Fill vertical position shift register with bits for this line */
		/* TODO; cheated slightly to shift stuff up one pixel*/
		vp1 = buggyboy_rcram[(rva_offs + 0x1e2) >> 1] >= y ? 0 : 1;
		vp2 = buggyboy_rcram[(rva_offs + 0x1e4) >> 1] >= y ? 0 : 1;
		vp3 = buggyboy_rcram[(rva_offs + 0x1e6) >> 1] >= y ? 0 : 1;
		vp4 = buggyboy_rcram[(rva_offs + 0x1e8) >> 1] >= y ? 0 : 1;
		vp5 = buggyboy_rcram[(rva_offs + 0x1ea) >> 1] >= y ? 0 : 1;
		vp6 = buggyboy_rcram[(rva_offs + 0x1ec) >> 1] >= y ? 0 : 1;
		vp7 = buggyboy_rcram[(rva_offs + 0x1ee) >> 1] >= y ? 0 : 1;

		/* Stuff */
		rm0 = vp7 ? BIT(vregs.scol, 4) : BIT(vregs.scol, 12);
		rm1 = vp7 ? BIT(vregs.scol, 5) : BIT(vregs.scol, 13);

		/* Wall/tunnel control */
		rcmd = (vp7 ? vregs.scol : vregs.scol >> 8) & 0xf;

		/* Load 'em up */
		LOAD_HPOS_COUNTER(0);
		LOAD_HPOS_COUNTER(1);
		LOAD_HPOS_COUNTER(2);
		LOAD_HPOS_COUNTER(3);

		/* Load the bank counter with accumulator bits 14-5 */
		bank_cnt = (vregs.ba_val >> 5) & 0x3ff;

		/* Have we crossed a road gfx strip boundary? */
		if (ls161 & 7)
		{
			buggyboy_get_roadpix(0, ls161, rva0_6, sld, &_rorevls, &rc0[0], &rc1[0], &rc2[0], &rc3[0]);
			buggyboy_get_roadpix(1, ls161, rva0_6, sld, &_rorevcs, &rc0[1], &rc1[1], &rc2[1], &rc3[1]);
			buggyboy_get_roadpix(2, ls161, rva0_6, sld, &_rorevrs, &rc0[2], &rc1[2], &rc2[2], &rc3[2]);
		}

		/* We can evaluate some of the pixel logic outside of the x-loop */
		ic96_term1 = !tnlf || (vp5 && vp6) || (!vp3 && vp6) || (!vp4 && vp5) || (!vp3 && !vp4);
		ic96_o17 = (vp3 && vp4) || tnlmd0 || tnlmd1 || !vp1 || !tnlf;

		ic97_o12 = (!vp1 && !vp2 && !vp6) || (!vp1 && !vp2 && vp7) || (vp4 && !vp6) || (vp4 && vp7);
		ic97_o13 = (!vp1 && !vp2 && !vp5) || (!vp1 && !vp2 && vp7) || (vp3 && !vp5) || (vp3 && vp7);

		ic79_p19 = !(!(vp5 || vp6) || vp7) && !(tnlmd0 || tnlmd1);

		for (x = 0; x < 256; ++x)
		{
			UINT32  pix;
			UINT32  hp0_en, hp1_en, hp2_en, hp3_en;

			UINT32  ic97_o17;
			UINT32  ic97_o18;
			UINT32  ic97_o19;

			UINT32  ic96_o14;
			UINT32  ic96_o15;
			UINT32  ic96_o16;

			UINT32  ic79_o15;
			UINT32  ic79_o16;
			UINT32  ic79_o17;

			UINT32  ic82_o17;
			UINT32  ic82_o16;
			UINT32  ic82_o15;
			UINT32  ic82_o14;

			UINT32  ic80_o17;
			UINT32  ic80_o16;
			UINT32  ic80_o15;
			UINT32  ic80_o14;

			UINT32  ic78_o17;
			UINT32  ic78_o16;
			UINT32  ic78_o15;
			UINT32  ic78_o14;

			UINT32  ic48_o12, ic50_o12, ic52_o12;
			UINT32  ic48_o16, ic50_o16, ic52_o16;
			UINT32  ic48_o17, ic50_o17, ic52_o17;
			UINT32  ic48_o18, ic50_o18, ic52_o18;
			UINT32  ic48_o19, ic50_o19, ic52_o19;

			UINT32  tmp;

			UINT8   px0[3];
			UINT8   px1[3];
			UINT8   px2[3];
			UINT8   px3[3];

			UINT32 lfsr = vregs.wave_lfsr;
			UINT32 wave =
						(wave0 ^ BIT(lfsr, 0))
						&& (wave1 ^ BIT(lfsr, 3))
						&& BIT(lfsr, 5)
						&& !BIT(lfsr, 15)
						&& BIT(lfsr, 11)
						&& BIT(lfsr, 13)
						&& (rva20_6 < ((lfsr >> 8) & 0xf));


			/* Strip pixel number */
			pix = (ls161 & 7) ^ 7;

			/* Horizontal position counter enables - also used as PAL inputs */
			hp0_en = !(hp0_cy || hps02);
			hp1_en = !(hp1_cy || hps12);
			hp2_en = !(hp2_cy || hps22);
			hp3_en = !(hp3_cy || hps32);

			/* Load in a new road gfx strip? */
			if (!(ls161 & 7))
			{
				buggyboy_get_roadpix(0, ls161, rva0_6, sld, &_rorevls, &rc0[0], &rc1[0], &rc2[0], &rc3[0]);
				buggyboy_get_roadpix(1, ls161, rva0_6, sld, &_rorevcs, &rc0[1], &rc1[1], &rc2[1], &rc3[1]);
				buggyboy_get_roadpix(2, ls161, rva0_6, sld, &_rorevrs, &rc0[2], &rc1[2], &rc2[2], &rc3[2]);
			}

			/* Road camber/banking */
			if (BIT(vregs.ba_val, 23))
			{
				bnkls = 1; bnkcs = 1; bnkrs = 1;
			}
			else if (vregs.ba_val & 0x007f8000)
			{
				bnkls = 0; bnkcs = 0; bnkrs = 0;
			}
			else
			{
				bnkls = bank_cnt < 0x400;
				bnkcs = bank_cnt < 0x300;
				bnkrs = bank_cnt < 0x200;
			}

			if (vregs.bank_mode)
			{
				bnkls ^= 1; bnkcs ^= 1; bnkrs ^= 1;
			}

			px0[0] = BIT(rc0[0], pix);
			px1[0] = BIT(rc1[0], pix);
			px2[0] = BIT(rc2[0], pix);
			px3[0] = BIT(rc3[0], pix);

			px0[1] = BIT(rc0[1], pix);
			px1[1] = BIT(rc1[1], pix);
			px2[1] = BIT(rc2[1], pix);
			px3[1] = BIT(rc3[1], pix);

			px0[2] = BIT(rc0[2], pix);
			px1[2] = BIT(rc1[2], pix);
			px2[2] = BIT(rc2[2], pix);
			px3[2] = BIT(rc3[2], pix);

			/*
			    Now evaluate the pixel logic for each of the three screens

			    TODO: A lot of this could be macrofied to avoid repetition.
			    Shuffling the equations around would squeeze out some extra speed.
			*/

			/* Left */
			ic96_o14 =
			ic96_term1
			|| (!hp1_en && hps10 && vp4 && !hps20 && !hps21)
			|| (!hp1_en && hps10 && vp4 && hp2_en && !hps21)
			|| (!hp1_en && hps10 && vp4 && vp6)
			|| (hps11 && vp4 && !hps20 && !hps21)
			|| (hps11 && vp4 && hp2_en && !hps21)
			|| (vp5 && !hps20 && !hps21)
			|| (!vp3 && !hps20 && !hps21)
			|| (vp5 && hp2_en && !hps21)
			|| (!vp3 && hp2_en && !hps21)
			|| (!hp1_en && hps10 && !vp4)
			|| (hps11 && vp4 && vp6)
			|| (hps11 && !vp4);

			/* Centre */
			ic96_o15 =
			ic96_term1
			|| (hps10 && hps11 && vp4 && hp2_en && !hps20)
			||(!hp1_en && hps11 && vp4 && hp2_en && !hps20)
			|| (hps10 && hps11 && vp6)
			|| (!hp1_en && hps11 && vp6)
			|| (hps10 && hps11 && vp4 && !hps21)
			|| (!hp1_en && hps11 && vp4 && !hps21)
			|| (vp5 && hp2_en && !hps20)
			|| (!vp3 && hp2_en && !hps20)
			|| (hps10 && hps11 && !vp4)
			|| (!hp1_en && hps11 && !vp4)
			|| (vp5 && !hps21)
			|| (!vp3 && !hps21);

			/* Right */
			ic96_o16 =
			ic96_term1
			|| (!hp1_en && hps10 && hps11 && vp6)
			|| (!hp1_en && hps10 && hps11 && !vp4)
			|| (!hp1_en && hps10 && hps11 && !hps20)
			|| (!hp1_en && hps10 && hps11 && hp2_en)
			|| (!hp1_en && hps10 && hps11 && !hps21)
			|| (vp5 && !hps20)
			|| (!vp3 && !hps20)
			|| (vp5 && hp2_en)
			|| (!vp3 && hp2_en)
			|| (vp5 && !hps21)
			|| (!vp3 && !hps21);

			tmp = (!vp1 && !vp2) || (vp2 && vp7);
			ic97_o17 = tmp || (!hp0_en && hps00 && vp2 && !hps30 && !hps31) || (!hp0_en && hps00 && vp2 && hp3_en && !hps31) || (hps01 && vp2 && !hps30 && !hps31) || (hps01 && vp2 && hp3_en && !hps31);
			ic97_o18 = tmp || (hps00 && hps01 && vp2 && hp3_en && !hps30) || (!hp0_en && hps01 && vp2 && hp3_en && !hps30) || (hps00 && hps01 && vp2 && !hps31) || (!hp0_en && hps01 && vp2 && !hps31);
			ic97_o19 = tmp || (!hp0_en && hps00 && hps01 && vp2 && !hps30) || (!hp0_en && hps00 && hps01 && vp2 && hp3_en) || (!hp0_en && hps00 && hps01 && vp2 && !hps31);


			/* Left */
			{
			UINT32 P4, P6, P7, P18, P19;

			P4 = !(ic96_o16 && ic96_o17 && ic97_o19);
			P6 = BIT(sld, 3);
			P7 = (!vp5 && !vp6) || vp7 || !linf;
			P19 = wangl && bnkls;

			ic78_o17 = (_rorevls && !tnlmd1 && tnlmd0) || (!_rorevls && tnlmd1 && !tnlmd0) || (_rorevls && ic97_o12) || (!_rorevls && ic97_o13);
			P18 = ic78_o17;
			ic78_o16 = P7 || (px1[0] && !px0[0] && tnlmd1 && !tnlmd0) || (px2[0] && tnlmd1 && tnlmd0);
			ic78_o15 = !tnlf || (!P4 && px0[0] && P19) || (!P4 && px1[0] && P19) || (!P4 && P18) || (!P4 && px2[0]);
			ic78_o14 = !P6 || tnlmd0 || tnlmd1 || P7;
			}

			/* Centre */
			{
			UINT32 P4, P6, P7, P18, P19;

			P4 = !(ic96_o15 && ic96_o17 && ic97_o18);
			P6 = BIT(sld, 3);
			P7 = (!vp5 && !vp6) || vp7 || !linf;
			P19 = wangl && bnkcs;

			ic80_o17 = (_rorevcs && !tnlmd1 && tnlmd0) || (!_rorevcs && tnlmd1 && !tnlmd0) || (_rorevcs && ic97_o12) || (!_rorevcs && ic97_o13);
			P18 = ic80_o17;
			ic80_o16 = P7 || (px1[1] && !px0[1] && tnlmd1 && !tnlmd0) || (px2[1] && tnlmd1 && tnlmd0);
			ic80_o15 = !tnlf || (!P4 && px0[1] && P19) || (!P4 && px1[1] && P19) || (!P4 && P18) || (!P4 && px2[1]);
			ic80_o14 = !P6 || tnlmd0 || tnlmd1 || P7;
			}

			/* Right */
			{
			UINT32 P4, P6, P7, P18, P19;

			P4 = !(ic96_o14 && ic96_o17 && ic97_o17);
			P6 = BIT(sld, 3);
			P7 = (!vp5 && !vp6) || vp7 || !linf;
			P19 = wangl && bnkrs;

			ic82_o17 = (_rorevrs && !tnlmd1 && tnlmd0) || (!_rorevrs && tnlmd1 && !tnlmd0) || (_rorevrs && ic97_o12) || (!_rorevrs && ic97_o13);
			P18 = ic82_o17;
			ic82_o16 = P7 || (px1[2] && !px0[2] && tnlmd1 && !tnlmd0) || (px2[2] && tnlmd1 && tnlmd0);
			ic82_o15 = !tnlf || (!P4 && px0[2] && P19) || (!P4 && px1[2] && P19) || (!P4 && P18) || (!P4 && px2[2]);
			ic82_o14 = !P6 || tnlmd0 || tnlmd1 || P7;
			}


			ic79_o17 = (!px2[0] && _rorevls && ic78_o15) || (tnlf && !ic97_o19) || (tnlf && ic79_p19 && px2[0] && ic78_o15);
			ic79_o16 = (!px2[1] && _rorevcs && ic80_o15) || (tnlf && !ic97_o18) || (tnlf && ic79_p19 && px2[1] && ic80_o15);
			ic79_o15 = (!px2[2] && _rorevrs && ic82_o15) || (tnlf && !ic97_o17) || (tnlf && ic79_p19 && px2[2] && ic82_o15);


			/* Left */
			{
			UINT32 P5,P7, P8;
			UINT32 rcsd0_3;
			UINT32 cprom_addr;

			P5 = BIT(tcmd, 3) ? ((!vp5 && !vp6) || vp7 || !linf) : ic78_o16;
			P7 = BIT(sld, 5);
			P8 = BIT(sld, 4);

			ic48_o19 = (px2[0] && !rva8) || !bnkls || !P5 || !ic78_o15;

			if (ic48_o19)
			{
				ic48_o12 =
				ic78_o14 &&
				(
					(px2[0] && px1[0] && px0[0] && rm1 && !rm0)
					|| (!px2[0] && px1[0] && px0[0] && !P8 && rm0)
					|| (px2[0] && px0[0] && !P7 && !rm1 && !rm0)
					|| (px2[0] && !px1[0] && px0[0] && !P7 && !rm1)
					|| (px2[0] && px1[0] && px0[0] && !P7 && !P8)
					|| (px2[0] && px1[0] && px0[0] && !P8 && rm1)
					|| (!px2[0] && !px3[0] && !rm0)
					|| (!px1[0] && !px3[0] && rm1)
					|| (!px2[0] && !px1[0] && !px3[0])
					|| (rva8)
					|| (!px0[0] && !px3[0])
					|| (!ic78_o15)
					|| (!P5)
				);

				ic48_o16 = (px2[0] && P5 && rm1) || (P5 && rva8 && ic78_o15) || (!px0[0] && P5) || !ic78_o15;
				ic48_o17 = (px0[0] && P5 && !rm0 && ic78_o15) || (!px1[0] && P5 && ic78_o15) || (P5 && rva8 && ic78_o15);
				ic48_o18 = (!px2[0] && P5 && ic78_o15) || (P5 && rva8 && ic78_o15);

				if (vp6 || ic78_o16)
				{
					if (!(ic78_o15 && P5))
						cprom_addr = (tcmd & 0x7) | (ic78_o16 << 3);
					else
						cprom_addr = rcmd;

					cprom_addr = (~cprom_addr & 0xf) << 4;
				}
				else
					cprom_addr = 0xf0;

				cprom_addr |= (ic79_o17 << 3) |
								(ic48_o18 << 2) |
								(ic48_o17 << 1) |
								ic48_o16;

				rcsd0_3 = rcols[cprom_addr] & 0xf;
				*(bmpaddr + 0) = 0x40 | (!wave << 5) | (ic48_o12 << 4) | rcsd0_3;
			}
			else
				*(bmpaddr + 0) = 0;
			}

			/* Centre */
			{
			UINT32 rcsd0_3;
			UINT32 cprom_addr;
			UINT32 P5, P7, P8;

			P5 = BIT(tcmd, 3) ? ((!vp5 && !vp6) || vp7 || !linf) : ic80_o16;
			P7 = BIT(sld, 5);
			P8 = BIT(sld, 4);

			ic50_o19 = (px2[1] && !rva8) || !bnkcs || !P5 || !ic80_o15;

			if (ic50_o19)
			{
				if (ic80_o14)
					ic50_o12 =
					(px2[1] && px1[1] && px0[1] && rm1 && !rm0)
					|| (!px2[1] && px1[1] && px0[1] && !P8 && rm0)
					|| (px2[1] && px0[1] && !P7 && !rm1 && !rm0)
					|| (px2[1] && !px1[1] && px0[1] && !P7 && !rm1)
					|| (px2[1] && px1[1] && px0[1] && !P7 && !P8)
					|| (px2[1] && px1[1] && px0[1] && !P8 && rm1)
					|| (!px2[1] && !px3[1] && !rm0)
					|| (!px1[1] && !px3[1] && rm1)
					|| (!px2[1] && !px1[1] && !px3[1])
					|| (rva8)
					|| (!px0[1] && !px3[1])
					|| (!ic80_o15)
					|| (!P5);
				else
					ic50_o12 = 0;


				ic50_o16 = (P5 && px2[1] && rm1) || (P5 && rva8 && ic80_o15) || (P5 && !px0[1]) || !ic80_o15;
				ic50_o17 = (P5 && px0[1] && !rm0 && ic80_o15) || (P5 && !px1[1] && ic80_o15) || (P5 && rva8 && ic80_o15);
				ic50_o18 = (P5 && !px2[1] && ic80_o15) || (P5 && rva8 && ic80_o15);

				if (vp6 || ic80_o16)
				{
					if (!(ic80_o15 && P5))
						cprom_addr = (tcmd & 0x7) | (ic80_o16 << 3);
					else
						cprom_addr = rcmd;

					cprom_addr = ((~cprom_addr) & 0xf) << 4;
				}
				else
					cprom_addr = 0xf0;

				cprom_addr |= (ic79_o16 << 3) |
								(ic50_o18 << 2) |
								(ic50_o17 << 1) |
								ic50_o16;

				rcsd0_3 = rcols[cprom_addr] & 0xf;
				*(bmpaddr + 256) = 0x40 | (!wave << 5) | (ic50_o12 << 4) | rcsd0_3;
			}
			else
				*(bmpaddr + 256) = 0;
			}

			/* Right */
			{
			UINT32 rcsd0_3;
			UINT32 cprom_addr;
			UINT32 P5, P7, P8;

			P5 = BIT(tcmd, 3) ? ((!vp5 && !vp6) || vp7 || !linf) : ic82_o16;
			P7 = BIT(sld, 5);
			P8 = BIT(sld, 4);

			ic52_o19 = (px2[2] && !rva8) || !bnkrs || !P5 || !ic82_o15;

			if (ic52_o19)
			{
				ic52_o12 =
				ic82_o14 &&
				(
					(px2[2] && px1[2] && px0[2] && rm1 && !rm0)
					|| (!px2[2] && px1[2] && px0[2] && !P8 && rm0)
					|| (px2[2] && px0[2] && !P7 && !rm1 && !rm0)
					|| (px2[2] && !px1[2] && px0[2] && !P7 && !rm1)
					|| (px2[2] && px1[2] && px0[2] && !P7 && !P8)
					|| (px2[2] && px1[2] && px0[2] && !P8 && rm1)
					|| (!px2[2] && !px3[2] && !rm0)
					|| (!px1[2] && !px3[2] && rm1)
					|| (!px2[2] && !px1[2] && !px3[2])
					|| (rva8)
					|| (!px0[2] && !px3[2])
					|| (!ic82_o15)
					|| (!P5)
				);

				ic52_o16 = (px2[2] && P5 && rm1) || (P5 && rva8 && ic82_o15) || (!px0[2] && P5) || !ic82_o15;
				ic52_o17 = (px0[2] && P5 && !rm0 && ic82_o15) || (!px1[2] && P5 && ic82_o15) || (P5 && rva8 && ic82_o15);
				ic52_o18 = (P5 && ic82_o15 && !px2[2]) || (P5 && ic82_o15 && rva8);

				if (vp6 || ic82_o16)
				{
					if (!(ic82_o15 && P5))
						cprom_addr = (tcmd & 0x7) | (ic82_o16 << 3);
					else
						cprom_addr = rcmd;

					cprom_addr = (~cprom_addr & 0xf) << 4;
				}
				else
					cprom_addr = 0xf0;

				cprom_addr |= (ic79_o15 << 3) |
								(ic52_o18 << 2) |
								(ic52_o17 << 1) |
								ic52_o16;

				rcsd0_3 = rcols[cprom_addr] & 0xf;

				*(bmpaddr + 512) = 0x40 | (!wave << 5) | (ic52_o12 ? 0x10 : 0) | rcsd0_3;
			}
			else
				*(bmpaddr + 512) = 0;
			}

			/* Now update counters and whatnot */
			++bmpaddr;

			UPDATE_HPOS(0);
			UPDATE_HPOS(1);
			UPDATE_HPOS(2);
			UPDATE_HPOS(3);

			/* Update the wave LFSR */
			vregs.wave_lfsr = (vregs.wave_lfsr << 1) | (BIT(vregs.wave_lfsr, 6) ^ !BIT(vregs.wave_lfsr, 15));

			/* Increment the bank counter */
			bank_cnt = (bank_cnt + 1) & 0x7ff;

			/* X pos */
			ls161 = (ls161 + 1) & 0x7fff;
		}

		/* WANGL active? Update the 8-bit counter */
		if (wangl)
		{
			if (BIT(vregs.flags, BB_RDFLAG_TNLMD0))
				--vregs.wa8;
			else
				++vregs.wa8;
		}

		/* No carry out - just increment */
		if (vregs.wa4 != 0xf)
			++vregs.wa4;
		else
		{
			/* Carry out; increment again on /TMG2S rise */
			if (wangl)
			{
				if (BIT(vregs.flags, BB_RDFLAG_TNLMD0))
					--vregs.wa8;
				else
					++vregs.wa8;
			}
			vregs.wa4 = 1;
		}

		/* Update accumulator */
		vregs.h_val += vregs.h_inc;

		/* Seems correct */
		sf = vregs.shift;

		if ((vregs.shift & 0x80) == 0)
		{
			vregs.shift <<= 1;

			if ((sf & 0x08) == 0)
				vregs.shift |= BIT(vregs.h_val, 15);
		}

		if ((sf & 0x08) && !(vregs.shift & 0x08))
			vregs.h_inc = vregs.gas;

		/* Finally, increment the banking accumulator */
		vregs.ba_val = (vregs.ba_val + vregs.ba_inc) & 0x00ffffff;
	}
}

void tx1_state::buggybjr_draw_road(UINT8 *bitmap)
{
	UINT16 *buggyboy_rcram = m_rcram;
	vregs_t &vregs = m_vregs;
	INT32 x;
	UINT32 y;
	UINT16 rva_offs;
	UINT32 tnlmd0;
	UINT32 tnlmd1;
	UINT32 linf;
	UINT32 tnlf;
	UINT32 wangl;
	UINT32 tcmd;
	UINT32 wave0;
	UINT32 wave1;
	UINT32 rva20_6;

	/* ROM/PROM lookup tables */
	const UINT8 *const rcols = &m_proms[0x1500];
	const UINT8 *const vprom = &m_road_rom[0x4600];

	/* Extract constant values */
	tcmd     = ((vregs.scol & 0xc000) >> 12) | ((vregs.scol & 0x00c0) >> 6);
	tnlmd0   = BIT(vregs.flags, BB_RDFLAG_TNLMD0);
	tnlmd1   = BIT(vregs.flags, BB_RDFLAG_TNLMD1);
	linf     = BIT(vregs.flags, BB_RDFLAG_LINF);
	tnlf     = BIT(vregs.flags, BB_RDFLAG_TNLF);
	wangl    = BIT(vregs.flags, BB_RDFLAG_WANGL);
	wave0    = BIT(vregs.flags, BB_RDFLAG_WAVE0);
	wave1    = BIT(vregs.flags, BB_RDFLAG_WAVE1);
	rva_offs = BIT(vregs.flags, BB_RDFLAG_RVA7) ? 0x800 : 0xc00;

	for (y = 0; y < 240; ++y)
	{
		UINT8   rva0_6;
		UINT8   ram_addr;
		UINT16  rcrdb0_15;
		UINT16  rcrs10;
		UINT16  ls161_156_a;
		UINT16  ls161;
		UINT8   sld;
		UINT32  rva8;
		UINT32  rm0, rm1;
		UINT32  rcmd;
		UINT32  bnkcs = 1;

//      UINT32  x_offs;
		UINT8   sf;

		/* Vertical positions shift register */
		UINT32  ram_val;
		UINT32  hp;
		UINT32  vp1, vp2, vp3, vp4, vp5, vp6, vp7;

		/* PAL outputs */
		UINT32  ic4_o12;
		UINT32  ic4_o13;
		UINT32  ic149_o15;
		UINT32  ic151_o14;

		/* Horizontal positions */
		UINT32  hp0, hp1, hp2, hp3;
		UINT8   hps00, hps01, hps02;
		UINT8   hps10, hps11, hps12;
		UINT8   hps20, hps21, hps22;
		UINT8   hps30, hps31, hps32;

		/* Road pixel data planes */
		UINT8   rc0 = 0, rc1 = 0, rc2 = 0, rc3 = 0;

		/* Horizontal position counter carry out */
		UINT8   hp0_cy = 0, hp1_cy = 0, hp2_cy = 0, hp3_cy = 0;

		UINT8   *bmpaddr = bitmap + (y * 256);

		UINT32  bank_cnt;
		UINT32  _rorevcs = 0;

		rva8 = (vregs.h_val & 0x8000) || !(vregs.shift & 0x80);

		/* Get RVA0_6 from TZ113 accumulator chain @ 122/123 */
		rva0_6 = (vregs.h_val >> 7) & 0x7f;

		/* For /WAVE bit logic later */
		rva20_6 = ((rva0_6 >> 3) & 0xe) | ((rva0_6 & 2) >> 1);

		/* RVA is inverted! */
		ram_addr = (~rva0_6 & 0x7f) << 1;

		/* Get the road RAM data for this line */
		rcrdb0_15 = buggyboy_rcram[(rva_offs + ram_addr) >> 1];

		/* If 15-10 == 000000, then 0 */
		rcrs10 = rcrdb0_15 & 0xfc00 ? 0x0400 : 0x0000;

		/* If 15-10 == 111111, then 1 */
		ls161_156_a = (rcrdb0_15 & 0xfc00) == 0xfc00 ? 0x800 : 0x0000;

		/* LS161 15-bit counter chain - loaded with RAM data (bar bits 10-13) */
		ls161 =  ((rcrdb0_15 & 0x8000) >> 1) | ls161_156_a | rcrs10 | (rcrdb0_15 & 0x03ff);

		/* SLD */
		sld = (vprom[rva0_6] + vregs.slin_val) & 0x38;

		/* Determine the x-offset */
//      x_offs = ls161 & 7;

		/* Fill vertical position shift register with bits for this line */
		/* TODO; cheated slightly to shift stuff up one pixel*/
		vp1 = buggyboy_rcram[(rva_offs + 0x1e2) >> 1] >= y ? 0 : 1;
		vp2 = buggyboy_rcram[(rva_offs + 0x1e4) >> 1] >= y ? 0 : 1;
		vp3 = buggyboy_rcram[(rva_offs + 0x1e6) >> 1] >= y ? 0 : 1;
		vp4 = buggyboy_rcram[(rva_offs + 0x1e8) >> 1] >= y ? 0 : 1;
		vp5 = buggyboy_rcram[(rva_offs + 0x1ea) >> 1] >= y ? 0 : 1;
		vp6 = buggyboy_rcram[(rva_offs + 0x1ec) >> 1] >= y ? 0 : 1;
		vp7 = buggyboy_rcram[(rva_offs + 0x1ee) >> 1] >= y ? 0 : 1;

		/* Stuff */
		rm0 = vp7 ? BIT(vregs.scol, 4) : BIT(vregs.scol, 12);
		rm1 = vp7 ? BIT(vregs.scol, 5) : BIT(vregs.scol, 13);

		/* Wall/tunnel control */
		rcmd = (vp7 ? vregs.scol : vregs.scol >> 8) & 0xf;

		/* Load 'em up */
		LOAD_HPOS_COUNTER(0);
		LOAD_HPOS_COUNTER(1);
		LOAD_HPOS_COUNTER(2);
		LOAD_HPOS_COUNTER(3);

		/* Some PAL equations that we can evaluate outside of the x-loop */
		ic4_o12 = (!vp1 && !vp2 && !vp6) || (!vp1 && !vp2 && vp7) || (vp4 && !vp6) || (vp4 && vp7);
		ic4_o13 = (!vp1 && !vp2 && !vp5) || (!vp1 && !vp2 && vp7) || (vp3 && !vp5) || (vp3 && vp7);
		ic149_o15 = (!vp5 && !vp6) || vp7 || !linf;
		ic151_o14 = !BIT(sld, 3) || tnlmd0 || tnlmd1 || ic149_o15;

		/* Load the bank counter with accumulator bits 14-5 */
		bank_cnt = (vregs.ba_val >> 5) & 0x3ff;

		/* Have we crossed a road gfx strip boundary? */
		if (ls161 & 7)
			buggyboy_get_roadpix(1, ls161, rva0_6, sld, &_rorevcs, &rc0, &rc1, &rc2, &rc3);

		for (x = 0; x < 256; ++x)
		{
			UINT32  pix;
			UINT32  hp0_en, hp1_en, hp2_en, hp3_en;
			UINT32  ic149_o16;
			UINT32  ic4_o18;
			UINT32  ic3_o15;
			UINT32  ic150_o12 = 0;
			UINT32  ic150_o16;
			UINT32  ic150_o17;
			UINT32  ic150_o18;
			UINT32  ic150_o19;
			UINT32  ic151_o15;
			UINT32  ic151_o16;
			UINT32  ic151_o17;
			UINT32  rcsd0_3 = 0;
			UINT32  sld5 = BIT(sld, 5);
			UINT32  sld4 = BIT(sld, 4);
			UINT32  mux;
			UINT32  cprom_addr;
			UINT8   px0, px1, px2, px3;

			/* Strip pixel number */
			pix = (ls161 & 7) ^ 7;

			/* Horizontal position counter enables - also used as PAL inputs */
			hp0_en = !(hp0_cy || hps02);
			hp1_en = !(hp1_cy || hps12);
			hp2_en = !(hp2_cy || hps22);
			hp3_en = !(hp3_cy || hps32);

			/* Load in a new road gfx strip? */
			if (!(ls161 & 7))
				buggyboy_get_roadpix(1, ls161, rva0_6, sld, &_rorevcs, &rc0, &rc1, &rc2, &rc3);

			/* Road camber */
			if (vregs.bank_mode == 0)
			{
				if (BIT(vregs.ba_val, 23))
					bnkcs = 1;
				else if (vregs.ba_val & 0x007f8000)
					bnkcs = 0;
				else
					bnkcs = bank_cnt < 0x300;
			}
			else
			{
				if (BIT(vregs.ba_val, 23))
					bnkcs = 0;
				else if (vregs.ba_val & 0x007f8000)
					bnkcs = 1;
				else
					bnkcs = bank_cnt >= 0x300;
			}

			px0 = BIT(rc0, pix);
			px1 = BIT(rc1, pix);
			px2 = BIT(rc2, pix);
			px3 = BIT(rc3, pix);

			/* Now go through and evaluate all the pixel logic */
			if (vp2)
				ic4_o18 = (hps00 && hps01 && hp3_en && !hps30)      ||
							(!hp0_en && hps01 && hp3_en && !hps30)  ||
							(hps00 && hps01 && !hps31)              ||
							(!hp0_en && hps01 && !hps31)                ||
							vp7;
			else
				ic4_o18 = !vp1;


			if (tnlf)
				ic3_o15 = (vp4 && !vp6 && !hp2_en && hps21)     ||
							(vp4 && !vp6 && hps20 && hps21)     ||
							(vp1 && !vp4 && !tnlmd1 && !tnlmd0) ||
							(vp1 && !vp3 && !tnlmd1 && !tnlmd0) ||
							(hp1_en && !hps10 && vp3 && !vp5)       ||
							(!hps11 && vp3 && !vp5);
			else
				ic3_o15 = !ic4_o18;

			ic151_o17 = (_rorevcs && !tnlmd1 && tnlmd0)     ||
						(!_rorevcs && tnlmd1 && !tnlmd0)    ||
						(_rorevcs && ic4_o12)               ||
						(!_rorevcs && ic4_o13);

			if (!ic3_o15)
				ic151_o15 = (px0 && (bnkcs && wangl))   ||
							(px1 && (bnkcs && wangl))   ||
							ic151_o17                   ||
							px2                         ||
							!tnlf;
			else
				ic151_o15 = !tnlf;

			ic151_o16 = (px1 && !px0 && tnlmd1 && !tnlmd0)  ||
						(px2 && tnlmd1 && tnlmd0)           ||
						ic149_o15;

			mux = BIT(tcmd, 3) ? ic149_o15 : ic151_o16;

			ic150_o19 = (px2 && !rva8)  ||
						!bnkcs          ||
						!mux            ||
						!ic151_o15;

			/* Don't calculate the pixel colour if not visible */
			if (ic150_o19)
			{
				ic149_o16 = (_rorevcs && !px2 && ic151_o15)                                 ||
							(tnlf && vp5 && !vp7 && px2 && !tnlmd0 && !tnlmd1 && ic151_o15) ||
							(tnlf && vp6 && !vp7 && px2 && !tnlmd0 && !tnlmd1 && ic151_o15) ||
							(tnlf && !ic4_o18);

				ic150_o16 = (px2 && mux && rm1)         ||
							(mux && rva8 && ic151_o15)  ||
							(!px0 && mux)               ||
							!ic151_o15;

				{
					UINT32 a = mux && ic151_o15;

					ic150_o17 = (a && !rm0 && px0)  ||
								(a && !px1)         ||
								(rva8 && a);

					ic150_o18 = (a && !px2) ||
								(rva8 && a);
				}

				if (ic151_o14)
					ic150_o12 = rva8 || !mux || !ic151_o15              ||
								(px2 && px1 && px0 && rm1 && !rm0)      ||
								(!px2 && px1 && px0 && !sld4 && rm0)    ||
								(px2 && px0 && !sld5 && !rm1 && !rm0)   ||
								(px2 && !px1 && px0 && !sld5 && !rm1)   ||
								(px2 && px1 && px0 && !sld5 && !sld4)   ||
								(px2 && px1 && px0 && !sld4 && rm1)     ||
								(!px2 && !px3 && !rm0)                  ||
								(!px1 && !px3 && rm1)                   ||
								(!px2 && !px1 && !px3)                  ||
								(!px0 && !px3);
				else
					ic150_o12 = 0;

				if (vp6 || ic151_o16)
				{
					UINT32 ic150_i5 = BIT(tcmd, 3) ? ic149_o15 : ic151_o16;

					if (!(ic151_o15 && ic150_i5))
						cprom_addr = (tcmd & 0x7) | (ic151_o16 ? 0x08 : 0);
					else
						cprom_addr = rcmd;

					/* Inverted! */
					cprom_addr = ((~cprom_addr) & 0xf) << 4;
				}
				else
					cprom_addr = 0xf0;

				cprom_addr |= (ic149_o16 ? 0x8 : 0) |
								(ic150_o18 ? 0x4 : 0) |
								(ic150_o17 ? 0x2 : 0) |
								(ic150_o16 ? 0x1 : 0);

				/* Lower four bits of colour output come from PROM BB7 @ 188 */
				rcsd0_3 = rcols[cprom_addr] & 0xf;

				{
					UINT32 lfsr = vregs.wave_lfsr;
					UINT32 wave =
								(wave0 ^ BIT(lfsr, 0))  &&
								(wave1 ^ BIT(lfsr, 3))  &&
								BIT(lfsr, 5)            &&
								!BIT(lfsr, 15)          &&
								BIT(lfsr, 11)           &&
								BIT(lfsr, 13)           &&
								(rva20_6 < ((lfsr >> 8) & 0xf));

					*bmpaddr++ = 0x40 | (wave ? 0 : 0x20) | (ic150_o12 ? 0x10 : 0) | rcsd0_3;
				}
			}
			else
				*bmpaddr++ = 0;

			/* Update the various horizontal counters */
			UPDATE_HPOS(0);
			UPDATE_HPOS(1);
			UPDATE_HPOS(2);
			UPDATE_HPOS(3);

			/* Update the LFSR */
			vregs.wave_lfsr = (vregs.wave_lfsr << 1) | (BIT(vregs.wave_lfsr, 6) ^ !BIT(vregs.wave_lfsr, 15));

			/* Increment the bank counter */
			bank_cnt = (bank_cnt + 1) & 0x7ff;

			/* X pos */
			ls161 = (ls161 + 1) & 0x7fff;
		}

		/* WANGL active? Then update the 8-bit counter */
		if (wangl)
		{
			if (BIT(vregs.flags, BB_RDFLAG_TNLMD0))
				--vregs.wa8;
			else
				++vregs.wa8;
		}

		/* No carry out - just increment */
		if (vregs.wa4 != 0xf)
			++vregs.wa4;
		else
		{
			/* Carry out; increment again on /TMG2S rise */
			if (wangl)
			{
				if (BIT(vregs.flags, BB_RDFLAG_TNLMD0))
					--vregs.wa8;
				else
					++vregs.wa8;
			}
			vregs.wa4 = 1;
		}

		/* Update accumulator */
		vregs.h_val += vregs.h_inc;

		/* Seems correct */
		sf = vregs.shift;

		if ((vregs.shift & 0x80) == 0)
		{
			vregs.shift <<= 1;

			if ((sf & 0x08) == 0)
				vregs.shift |= BIT(vregs.h_val, 15);
		}

		if ((sf & 0x08) && !(vregs.shift & 0x08))
			vregs.h_inc = vregs.gas;

		/* Finally, increment the banking accumulator */
		vregs.ba_val = (vregs.ba_val + vregs.ba_inc) & 0x00ffffff;
	}
}


/***************************************************************************

    Buggy Boy Object Drawing

    X-scaling isn't quite right but you wouldn't notice...

    -------- xxxxxxxx       Object number
    xxxxxxxx --------       Y position

    xxxxxxxx xxxxxxxx       Y scale value

    -------- xxxxxxxx       X scale
                             00 = Invisible?
                             80 = 1:1
                             FF = Double size
    xxxxxxxx --------       Attributes

    xxxxxxxx xxxxxxxx       Y scale delta

    ------xx xxxxxxxx       X position

**************************************************************************/

void tx1_state::buggyboy_draw_objs(UINT8 *bitmap, bool wide)
{
	UINT16 *buggyboy_objram = m_objram;

	UINT32 offs;

	UINT32 x_mask;
	UINT32 x_stride;

	/* The many lookup table ROMs */
	const UINT8 *const bug13  = &m_obj_luts[0];
	const UINT8 *const bug18s = &m_obj_luts[0x2000];
	const UINT8 *const bb8    = &m_proms[0x1600];

	const UINT8 *const bug16s = &m_obj_map[0];
	const UINT8 *const bug17s = &m_obj_map[0x8000];

	const UINT8 *const bb9o = &m_proms[0x500];
	const UINT8 *const bb9e = &m_proms[0xd00];

	const UINT8 *const pixdata_rgn = &m_obj_tiles[0];

	if (wide)
	{
		x_mask = 0x7ff;
		x_stride = 768;
	}
	else
	{
		x_mask = 0x3ff;
		x_stride = 256;
	}

	for (offs = 0; offs <= 0x300; offs += 8)
	{
		UINT32  x;
		UINT32  y;
		UINT32  gxflip;

		UINT32  x_scale;
		UINT32  x_step;
		UINT16  y_scale;
		UINT16  y_step;

		UINT8   pctmp0_7;
		UINT8   code;

		/* Check for end of object list */
		if ((buggyboy_objram[offs] & 0xff00) == 0xff00)
			break;

		/* X scale */
		x_scale = buggyboy_objram[offs + 2] & 0xff;

		/* TODO: Confirm against hardware? */
		if (x_scale == 0)
			continue;

		/* 16-bit y-scale accumulator */
		y_scale = buggyboy_objram[offs + 1];
		y_step  = buggyboy_objram[offs + 3];

		/* Object number */
		code = buggyboy_objram[offs] & 0xff;

		/* Attributes */
		pctmp0_7 = buggyboy_objram[offs + 2] >> 8;

		/* Global x-flip */
		gxflip = (pctmp0_7 & 0x80) >> 7;

		/* Add 1 to account for line buffering */
		y = (buggyboy_objram[offs] >> 8) + 1;

		for (; y < 240; ++y)
		{
			UINT32  rom_addr2   = 0;
			UINT8   bug17s_data = 0;
			UINT8   bug16s_data;

			/* Are we drawing on this line? */

			// TODO: See big lampposts.
			if (y_scale & 0x8000)
				break;

			{
				UINT32  psa0_12;
				UINT32  bug13_addr;
				UINT32  bug13_data;
				UINT32  rom_addr;
				UINT32  x_acc;
				UINT32  newtile = 1;
				UINT32  dataend = 0;
				UINT8   data1 = 0;
				UINT8   data2 = 0;
				UINT32  xflip = 0;
				UINT32  opcd10_11;
				UINT32  opcd8_9;
				UINT32  opcd0_11 = 0;
				UINT32  lasttile = 0;

				/* Use the object code to lookup the tile sequence data */
				bug13_addr = code << 4;
				bug13_addr |= ((y_scale >> 11) & 0xf);
				bug13_data = bug13[bug13_addr];

				/* Reached the bottom of the object */
				if (bug13_data == 0xff)
					break;

				psa0_12  = (((code & 0x80) << 5) | ((code & 0x40) << 6)) & 0x1000;
				psa0_12 |= ((bb8[code] << 8) | bug13_data) & 0x1fff;

				/* Static part of the BUG17S/BUG16S ROM address */
				rom_addr = (psa0_12 & ~0xff) << 2;

				/* Prepare the x-scaling */
				x_step = (128 << OBJ_FRAC) / x_scale;
				x_acc = (psa0_12 & 0xff) << (OBJ_FRAC + 5);

				/* TODO Add note */
				x = buggyboy_objram[offs + 4] & x_mask;

				for (;;)
				{
					/* Get data and attributes for an 8x8 tile */
					if (newtile)
					{
						UINT32  pscb0_11;
						UINT32  psbb0_15;
						UINT32  psbb6_7;
						UINT32  rombank;
						UINT8   *romptr;
						UINT32  bug18s_data;
						UINT32  low_addr = ((x_acc >> (OBJ_FRAC + 3)) & x_mask);

						/*
						    Objects are grouped by width (either 16, 8 or 4 tiles) in
						    the LUT ROMs. The ROM address lines therefore indicate
						    width and are used to determine the correct scan order
						    when x-flip is set.
						*/
						if (gxflip)
						{
							UINT32  xor_mask;

							if (BIT(psa0_12, 11) || !BIT(psa0_12, 12))
								xor_mask = 0xf;
							else if (!BIT(psa0_12, 9))
								xor_mask = 0x7;
							else
								xor_mask = 0x3;

							rom_addr2 = rom_addr + (low_addr ^ xor_mask);
						}
						else
							rom_addr2 = rom_addr + low_addr;

						bug17s_data = bug17s[rom_addr2 & 0x7fff];

						if ((bug17s_data & 0x40) && dataend)
							lasttile = 1;

						dataend |= (bug17s_data & 0x40);

						/* Retrieve data for an 8x8 tile */
						bug16s_data = bug16s[rom_addr2 & 0x7fff];
						psbb0_15 = (bug17s_data << 8) | bug16s_data;
						psbb6_7 = (BIT(psbb0_15, 12) ? psbb0_15 : (pctmp0_7 << 6)) & 0xc0;

						/* Form the tile ROM address */
						pscb0_11 = ((((psbb0_15 & ~0xc0) | psbb6_7) << 3) | ((y_scale >> 8) & 7)) & 0x7fff;

						/* Choose from one of three banks */
						rombank = ((BIT(pctmp0_7, 4) << 1) | BIT(psbb0_15, 13)) & 3;

						romptr = (UINT8*)(pixdata_rgn + rombank * (0x8000 * 2));

						/* Get raw 8x8 pixel row data */
						data1 = *(pscb0_11 + romptr);
						data2 = *(pscb0_11 + romptr + 0x8000);

						/* Determine flip state (global XOR local) */
						xflip = gxflip ^ !BIT(psbb0_15, 15);

						bug18s_data = bug18s[ (BIT(pctmp0_7, 4)  << 13) |
												(BIT(psbb0_15, 13) << 12)   |
												(psbb0_15 & ~0xf0c0)        |
												psbb6_7 ];

						/* Get the colour data. Note that bits 11 and 10 are inverted */
						opcd10_11 = ((pctmp0_7 << 8) & 0xc00) ^ 0xc00;
						opcd8_9 = ((pctmp0_7 & 0x60) << 3);
						opcd0_11 = (opcd10_11 | opcd8_9 | bug18s_data) & 0xfff;

						newtile = 0;
					}

					/* Draw a pixel? */
					if (x < x_stride)
					{
						UINT8   pix;
						UINT8   bit;

						bit = (x_acc >> OBJ_FRAC) & 7;

						if (xflip)
							bit ^= 7;

						pix = (((data1 >> bit) & 1) << 1) | ((data2 >> bit) & 1);

						/* Write the pixel if not transparent */
						if (!(!(opcd0_11 & 0x80) && !pix))
						{
							UINT8 color;
							UINT32 bb9_addr;

							bb9_addr = ((opcd0_11 << 1) & 0x600) | ((opcd0_11 & 0x7f) << 2) | pix;
							color = ((opcd0_11 >> 6) & 0x30);

							/* Inverted on schematic */
							if (x & 1)
								color = ~(color | bb9o[bb9_addr]) & 0x3f;
							else
								color = ~(color | bb9e[bb9_addr]) & 0x3f;

							*(bitmap + x_stride*y + x) = 0x40 | color;
						}
					}

					/* Check if we've stepped into a new 8x8 tile */
					if ((((x_acc + x_step) >> (OBJ_FRAC + 3)) & x_mask) != ((x_acc >> (OBJ_FRAC + 3)) & x_mask))
					{
						if (lasttile)
							break;

						newtile = 1;
					}

					x = (x + 1) & x_mask;
					x_acc += x_step;
				}
			}// if (yscale)
			y_scale += y_step;
		} /* for (y) */
	}/* for (offs) */
}


/*************************************
 *
 *  Video Control Registers
 *
 *************************************/

/*
    2400-24FF is road control (R/W)

    /GAS = 24XX:
    /BASET0 = 2400-F, 2410-F
    /BASET1 = 2420-F, 2430-F
    /BSET   = 2440-F, 2450-F
    /HASET  = 2460-F, 2470-F
    /HSET   = 2480-F, 2490-F
    /WASET  = 24A0-F, 24B0-F
    /FLAGS  = 24E0-F, 24F0-F
*/
WRITE16_MEMBER(tx1_state::buggyboy_gas_w)
{
	vregs_t &vregs = m_vregs;
	offset <<= 1;

	switch (offset & 0xe0)
	{
		case 0x00:
		{
			vregs.ba_inc &= ~0x0000ffff;
			vregs.ba_inc |= data;

			if (!(offset & 2))
				vregs.ba_val &= ~0x0000ffff;

			break;
		}
		case 0x20:
		{
			data &= 0xff;
			vregs.ba_inc &= ~0xffff0000;
			vregs.ba_inc |= data << 16;

			vregs.bank_mode = data & 1;

			if (!(offset & 2))
				vregs.ba_val &= ~0xffff0000;

			break;
		}
		case 0x40:
		{
			/* Ignore data? */
			if (offset & 2)
				vregs.ba_val = (vregs.ba_inc + vregs.ba_val) & 0x00ffffff;

			break;
		}
		case 0x60:
		{
			vregs.h_inc = data;
			vregs.shift = 0;

			if (!(offset & 2))
				vregs.h_val = 0;

			break;
		}
		case 0x80:
		{
			/* Ignore data? */
			if (offset & 2)
				vregs.h_val += vregs.h_inc;
			break;
		}
		case 0xa0:
		{
			vregs.wa8 = data >> 8;
			vregs.wa4 = 0;
			break;
		}
		case 0xe0:
		{
			m_mathcpu->set_input_line(INPUT_LINE_TEST, CLEAR_LINE);
			vregs.flags = data;
			break;
		}
	}

	/* Value is latched by LS373 76/77 */
	vregs.gas = data;
}

WRITE16_MEMBER(tx1_state::buggyboy_sky_w)
{
	m_vregs.sky = data;
}

WRITE16_MEMBER(tx1_state::buggyboy_scolst_w)
{
	m_vregs.scol = data;
}


/*************************************
 *
 *  Core Functions
 *
 *************************************/

void tx1_state::bb_combine_layers(bitmap_ind16 &bitmap, int screen)
{
	UINT8 *chr_pal = &m_proms[0x400];
	UINT32 bmp_stride;
	UINT32 x_offset;
	UINT32 y;

	if (screen < 0)
	{
		bmp_stride = 256;
		x_offset = 0;
	}
	else
	{
		bmp_stride = 768;
		x_offset = 256 * screen;
	}

	for (y = 0; y < 240; ++y)
	{
		UINT32 x;

		UINT32 bmp_offset = y * bmp_stride + x_offset;

		UINT8 *chr_addr = m_chr_bmp + bmp_offset;
		UINT8 *rod_addr = m_rod_bmp + bmp_offset;
		UINT8 *obj_addr = m_obj_bmp + bmp_offset;

		UINT32 sky_en = BIT(m_vregs.sky, 7);
		UINT32 sky_val = (((m_vregs.sky & 0x7f) + y) >> 2) & 0x3f;

		UINT16 *bmp_addr = &bitmap.pix16(y);

		for (x = 0; x < 256; ++x)
		{
			UINT32 out_val;

			UINT32 char_val = *chr_addr++;
			UINT32 char_6_7 = (char_val & 0xc0) >> 2;

			UINT32 obj_val = *obj_addr++;
			UINT32 obj6 = BIT(obj_val, 6);

			UINT32 rod_val = *rod_addr++;
			UINT32 rod6 = BIT(rod_val, 6);

			UINT32 chr = !(BIT(char_val, 7) && (char_val & 3) );

			UINT32 sel =
			(
				( BIT(obj_val, 6) && chr) ||
				( sky_en && !(char_val & 3) && (!obj6 && !rod6) )
			) ? 0 : 1;

			sel |= (!(obj6 || rod6) || !chr) ? 2 : 0;

			/* Select the layer */
			if      (sel == 0)  out_val = obj_val & 0x3f;
			else if (sel == 1)  out_val = rod_val & 0x3f;
			else if (sel == 2)  out_val = sky_val;
			else                out_val = char_6_7 + chr_pal[char_val];

			*bmp_addr++ = (sel << 6) + out_val;
		}
	}
}

VIDEO_START_MEMBER(tx1_state,buggyboy)
{
	/* Allocate some bitmaps */
	m_chr_bmp = auto_alloc_array(machine(), UINT8, 3 * 256 * 240);
	m_obj_bmp = auto_alloc_array(machine(), UINT8, 3 * 256 * 240);
	m_rod_bmp = auto_alloc_array(machine(), UINT8, 3 * 256 * 240);

	/* Set a timer to run the interrupts */
	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(tx1_state::interrupt_callback),this));

	/* /CUDISP CRTC interrupt */
	m_interrupt_timer->adjust(m_screen->time_until_pos(CURSOR_YPOS, CURSOR_XPOS));
}

VIDEO_START_MEMBER(tx1_state,buggybjr)
{
	/* Allocate some bitmaps */
	m_chr_bmp = auto_alloc_array(machine(), UINT8, 256 * 240);
	m_obj_bmp = auto_alloc_array(machine(), UINT8, 256 * 240);
	m_rod_bmp = auto_alloc_array(machine(), UINT8, 256 * 240);

	/* Set a timer to run the interrupts */
	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(tx1_state::interrupt_callback),this));

	/* /CUDISP CRTC interrupt */
	m_interrupt_timer->adjust(m_screen->time_until_pos(CURSOR_YPOS, CURSOR_XPOS));
}

void tx1_state::screen_eof_buggyboy(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		/* /VSYNC: Update TZ113 @ 219 */
		m_vregs.slin_val += m_vregs.slin_inc;

		/* /VSYNC: Clear wave LFSR */
		m_vregs.wave_lfsr = 0;

		m_needs_update = true;
	}
}


void tx1_state::bb_update_layers()
{
	memset(m_obj_bmp, 0, 768*240);
	memset(m_rod_bmp, 0, 768*240);

	buggyboy_draw_char(m_chr_bmp, 1);
	buggyboy_draw_road(m_rod_bmp);
	buggyboy_draw_objs(m_obj_bmp, 1);

	m_needs_update = false;
}

UINT32 tx1_state::screen_update_buggyboy_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_needs_update)
		bb_update_layers();

	bb_combine_layers(bitmap, 0);
	return 0;
}

UINT32 tx1_state::screen_update_buggyboy_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_needs_update)
		bb_update_layers();

	bb_combine_layers(bitmap, 1);
	return 0;
}

UINT32 tx1_state::screen_update_buggyboy_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_needs_update)
		bb_update_layers();

	bb_combine_layers(bitmap, 2);
	return 0;
}

UINT32 tx1_state::screen_update_buggybjr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	memset(m_obj_bmp, 0, 256*240);

	buggyboy_draw_char(m_chr_bmp, 0);
	buggybjr_draw_road(m_rod_bmp);
	buggyboy_draw_objs(m_obj_bmp, 0);

	bb_combine_layers(bitmap, -1);
	return 0;
}
