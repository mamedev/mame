// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/*******************************************************************************************

Night Gal (c) 1984 Nichibutsu

a.k.a. same Jangou blitter but with NCS CPU for displaying graphics as protection.

preliminary driver by David Haywood & Angelo Salese
many thanks to Charles MacDonald for the schematics / documentation of this HW.

TODO:
-Night Gal Summer trips illegal opcodes on the NCS side, presumably a CPU bug;
-Fix Sweet Gal/Sexy Gal gfxs if necessary (i.e. if the bugs aren't all caused by irq/nmi
 wrong firing);
-Proper Z80<->MCU comms,many video problems because of that;
-Abstract the video chip to a proper video file and get the name of that chip;
-Minor graphic glitches in Royal Queen (cross hatch test, some little glitches during gameplay),
 presumably due of the unemulated wait states on the comms.

*******************************************************************************************/

#include "emu.h"
#include "sound/ay8910.h"
#include "sound/2203intf.h"
#include "cpu/z80/z80.h"
#include "cpu/m6800/m6800.h"
#include "video/resnet.h"

#define MASTER_CLOCK    XTAL_19_968MHz

class nightgal_state : public driver_device
{
public:
	nightgal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_comms_ram(*this, "comms_ram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxrom(*this, "gfx1"),
		m_io_cr_clear(*this, "CR_CLEAR"),
		m_io_coins(*this, "COINS"),
		m_io_pl1_1(*this, "PL1_1"),
		m_io_pl1_2(*this, "PL1_2"),
		m_io_pl1_3(*this, "PL1_3"),
		m_io_pl1_4(*this, "PL1_4"),
		m_io_pl1_5(*this, "PL1_5"),
		m_io_pl1_6(*this, "PL1_6"),
		m_io_pl2_1(*this, "PL2_1"),
		m_io_pl2_2(*this, "PL2_2"),
		m_io_pl2_3(*this, "PL2_3"),
		m_io_pl2_4(*this, "PL2_4"),
		m_io_pl2_5(*this, "PL2_5"),
		m_io_pl2_6(*this, "PL2_6"),
		m_io_system(*this, "SYSTEM"),
		m_io_sysa(*this, "SYSA"),
		m_io_dswa(*this, "DSWA"),
		m_io_dswb(*this, "DSWB"),
		m_io_dswc(*this, "DSWC"),
		m_palette(*this, "palette") { }

	/* video-related */
	UINT8 m_blit_raw_data[3];
	UINT8 m_true_blit[7];
	UINT8 m_pen_data[0x10];
	UINT8 m_pen_raw_data[0x10];

	/* misc */
	UINT8 m_nsc_latch;
	UINT8 m_z80_latch;
	UINT8 m_mux_data;

	required_shared_ptr<UINT8> m_comms_ram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;

	/* memory */
	UINT8      m_blit_buffer[256*256];
	DECLARE_READ8_MEMBER(blitter_status_r);
	DECLARE_WRITE8_MEMBER(nsc_true_blitter_w);
	DECLARE_WRITE8_MEMBER(sexygal_nsc_true_blitter_w);
	DECLARE_WRITE8_MEMBER(nsc_latch_w);
	DECLARE_READ8_MEMBER(nsc_latch_r);
	DECLARE_WRITE8_MEMBER(z80_latch_w);
	DECLARE_READ8_MEMBER(z80_latch_r);
	DECLARE_WRITE8_MEMBER(blitter_w);
	DECLARE_READ8_MEMBER(nsc_blit_r);
	DECLARE_WRITE8_MEMBER(royalqn_blitter_0_w);
	DECLARE_WRITE8_MEMBER(royalqn_blitter_1_w);
	DECLARE_WRITE8_MEMBER(royalqn_blitter_2_w);
	DECLARE_READ8_MEMBER(royalqn_nsc_blit_r);
	DECLARE_READ8_MEMBER(royalqn_comm_r);
	DECLARE_WRITE8_MEMBER(royalqn_comm_w);
	DECLARE_WRITE8_MEMBER(blit_vregs_w);
	DECLARE_READ8_MEMBER(blit_vregs_r);
	DECLARE_WRITE8_MEMBER(blit_true_vregs_w);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_READ8_MEMBER(input_1p_r);
	DECLARE_READ8_MEMBER(input_2p_r);
	DECLARE_DRIVER_INIT(ngalsumr);
	DECLARE_DRIVER_INIT(royalqn);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(nightgal);
	UINT32 screen_update_nightgal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	required_region_ptr<UINT8> m_gfxrom;
	required_ioport m_io_cr_clear;
	required_ioport m_io_coins;
	required_ioport m_io_pl1_1;
	required_ioport m_io_pl1_2;
	required_ioport m_io_pl1_3;
	required_ioport m_io_pl1_4;
	required_ioport m_io_pl1_5;
	required_ioport m_io_pl1_6;
	required_ioport m_io_pl2_1;
	required_ioport m_io_pl2_2;
	required_ioport m_io_pl2_3;
	required_ioport m_io_pl2_4;
	required_ioport m_io_pl2_5;
	required_ioport m_io_pl2_6;
	required_ioport m_io_system;
	required_ioport m_io_sysa;
	required_ioport m_io_dswa;
	required_ioport m_io_dswb;
	required_ioport m_io_dswc;
	required_device<palette_device> m_palette;

	UINT8 nightgal_gfx_nibble( int niboffset );
	void plot_nightgal_gfx_pixel( UINT8 pix, int x, int y );
};



READ8_MEMBER(nightgal_state::blitter_status_r)
{
	return 0x80;
}

void nightgal_state::video_start()
{
	save_item(NAME(m_blit_buffer));
}

UINT32 nightgal_state::screen_update_nightgal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	for (y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		UINT8 *src = &m_blit_buffer[y * 512 / 2 + cliprect.min_x];
		UINT16 *dst = &bitmap.pix16(y, cliprect.min_x);

		for (x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			UINT32 srcpix = *src++;
			*dst++ = m_palette->pen(srcpix & 0xf);
			*dst++ = m_palette->pen((srcpix >> 4) & 0xf);
		}
	}


	return 0;
}

UINT8 nightgal_state::nightgal_gfx_nibble( int niboffset )
{
	if (niboffset & 1)
	{
		return (m_gfxrom[(niboffset >> 1) & 0x1ffff] & 0xf0) >> 4;
	}
	else
	{
		return (m_gfxrom[(niboffset >> 1) & 0x1ffff] & 0x0f);
	}
}

void nightgal_state::plot_nightgal_gfx_pixel( UINT8 pix, int x, int y )
{
	if (y >= 512) return;
	if (x >= 512) return;
	if (y < 0) return;
	if (x < 0) return;

	if (x & 1)
		m_blit_buffer[(y * 256) + (x >> 1)] = (m_blit_buffer[(y * 256) + (x >> 1)] & 0x0f) | ((pix << 4) & 0xf0);
	else
		m_blit_buffer[(y * 256) + (x >> 1)] = (m_blit_buffer[(y * 256) + (x >> 1)] & 0xf0) | (pix & 0x0f);
}

WRITE8_MEMBER(nightgal_state::nsc_true_blitter_w)
{
	int src, x, y, h, w, flipx;
	m_true_blit[offset] = data;

	/*trigger blitter write to ram,might not be correct...*/
	if (offset == 5)
	{
		//printf("%02x %02x %02x %02x %02x %02x %02x\n", m_true_blit[0], m_true_blit[1], m_true_blit[2], m_true_blit[3], m_true_blit[4], m_true_blit[5], m_true_blit[6]);
		w = (m_true_blit[4] & 0xff) + 1;
		h = (m_true_blit[5] & 0xff) + 1;
		src = ((m_true_blit[1] << 8) | (m_true_blit[0] << 0));
		src |= (m_true_blit[6] & 3) << 16;

		x = (m_true_blit[2] & 0xff);
		y = (m_true_blit[3] & 0xff);

		// lowest bit of src controls flipping / draw direction?
		flipx = (m_true_blit[0] & 1);

		if (!flipx)
			src += (w * h) - 1;
		else
			src -= (w * h) - 1;

		{
			int count = 0;
			int xcount, ycount;
			for (ycount = 0; ycount < h; ycount++)
			{
				for (xcount = 0; xcount < w; xcount++)
				{
					int drawx = (x + xcount) & 0xff;
					int drawy = (y + ycount) & 0xff;
					UINT8 dat = nightgal_gfx_nibble(src + count);
					UINT8 cur_pen_hi = m_pen_data[(dat & 0xf0) >> 4];
					UINT8 cur_pen_lo = m_pen_data[(dat & 0x0f) >> 0];

					dat = cur_pen_lo | (cur_pen_hi << 4);

					if ((dat & 0xff) != 0)
						plot_nightgal_gfx_pixel(dat, drawx, drawy);

					if (!flipx)
						count--;
					else
						count++;
				}
			}
		}
	}
}

/* different register writes (probably a PAL line swapping).*/
WRITE8_MEMBER(nightgal_state::sexygal_nsc_true_blitter_w)
{
	int src, x, y, h, w, flipx;
	m_true_blit[offset] = data;

	/*trigger blitter write to ram,might not be correct...*/
	if (offset == 6)
	{
		//printf("%02x %02x %02x %02x %02x %02x %02x\n", m_true_blit[0], m_true_blit[1], m_true_blit[2], m_true_blit[3], m_true_blit[4], m_true_blit[5], m_true_blit[6]);
		w = (m_true_blit[5] & 0xff) + 1;
		h = (m_true_blit[6] & 0xff) + 1;
		src = ((m_true_blit[1] << 8) | (m_true_blit[0] << 0));
		src |= (m_true_blit[2] & 3) << 16;


		x = (m_true_blit[3] & 0xff);
		y = (m_true_blit[4] & 0xff);

		// lowest bit of src controls flipping / draw direction?
		flipx = (m_true_blit[0] & 1);

		if (!flipx)
			src += (w * h) - 1;
		else
			src -= (w * h) - 1;

		{
			int count = 0;
			int xcount, ycount;
			for (ycount = 0; ycount < h; ycount++)
			{
				for (xcount = 0; xcount < w; xcount++)
				{
					int drawx = (x + xcount) & 0xff;
					int drawy = (y + ycount) & 0xff;
					UINT8 dat = nightgal_gfx_nibble(src + count);
					UINT8 cur_pen_hi = m_pen_data[(dat & 0xf0) >> 4];
					UINT8 cur_pen_lo = m_pen_data[(dat & 0x0f) >> 0];

					dat = cur_pen_lo | cur_pen_hi << 4;

					if ((dat & 0xff) != 0)
						plot_nightgal_gfx_pixel(dat, drawx, drawy);

					if (!flipx)
						count--;
					else
						count++;
				}
			}
			//m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE );
		}
	}
}

/* guess: use the same resistor values as Crazy Climber (needs checking on the real HW) */
PALETTE_INIT_MEMBER(nightgal_state, nightgal)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_rg[3], weights_b[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 0,
			2, resistances_b,  weights_b,  0, 0,
			0, nullptr, nullptr, 0, 0);

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		r = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* green component */
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		g = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* blue component */
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		b = combine_2_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

/********************************************
*
* Z80-MCU communications
*
********************************************/

/*
(note:when I say "0x80" I just mean a negative result)
master-slave algorithm
-z80 writes the data for the mcu;
-z80 writes 0 to c200;
-it waits with the bit 0x80 on c100 clears (i.e. the z80 halts),when this happens the z80 continues his logic algorithm (so stop it until we are done!!!)

-nsc takes an irq
-puts ff to [1100]
-it waits that the bit 0x80 on [1100] clears
-(puts default clut data,only the first time around)
-reads params from z80 and puts them on the blitter chip
-expects that bit [80] is equal to 0x80;
-clears [1100] and expects that [1100] is 0
-executes a wai (i.e. halt) opcode then expects to receive another irq...
*/

#define MAIN_Z80_RUN   if(offset == 2) m_z80_latch = 0x00
#define MAIN_Z80_HALT  if(offset == 2) m_z80_latch = 0x80
//#define SUB_NCS_RUN m_ncs_latch = 0x00
//#define SUB_NCS_HALT m_ncs_latch = 0x80
#ifdef UNUSED_CODE
WRITE8_MEMBER(nightgal_state::nsc_latch_w)
{
	m_subcpu->set_input_line(0, HOLD_LINE );
}

READ8_MEMBER(nightgal_state::nsc_latch_r)
{
	return m_z80_latch;
}

WRITE8_MEMBER(nightgal_state::z80_latch_w)
{
	m_nsc_latch = data;
}

READ8_MEMBER(nightgal_state::z80_latch_r)
{
	return m_nsc_latch;
}

/*z80 -> MCU video params*/
WRITE8_MEMBER(nightgal_state::blitter_w)
{
	m_blit_raw_data[offset] = data;
	MAIN_Z80_HALT;
}

READ8_MEMBER(nightgal_state::nsc_blit_r)
{
	MAIN_Z80_RUN;
	return m_blit_raw_data[offset];
}
#endif
/* TODO: simplify this (error in the document) */

WRITE8_MEMBER(nightgal_state::royalqn_blitter_0_w)
{
	m_blit_raw_data[0] = data;
}

WRITE8_MEMBER(nightgal_state::royalqn_blitter_1_w)
{
	m_blit_raw_data[1] = data;
}

WRITE8_MEMBER(nightgal_state::royalqn_blitter_2_w)
{
	m_blit_raw_data[2] = data;
	m_subcpu->set_input_line(0, ASSERT_LINE );
}

READ8_MEMBER(nightgal_state::royalqn_nsc_blit_r)
{
	if(offset == 2)
		m_subcpu->set_input_line(0, CLEAR_LINE );

	return m_blit_raw_data[offset];
}

READ8_MEMBER(nightgal_state::royalqn_comm_r)
{
	return (m_comms_ram[offset] & 0x80) | (0x7f); //bits 6-0 are undefined, presumably open bus
}

WRITE8_MEMBER(nightgal_state::royalqn_comm_w)
{
	m_comms_ram[offset] = data & 0x80;
}

#ifdef UNUSED_CODE
WRITE8_MEMBER(nightgal_state::blit_vregs_w)
{
	m_pen_raw_data[offset] = data;
}

READ8_MEMBER(nightgal_state::blit_vregs_r)
{
	return m_pen_raw_data[offset];
}
#endif
WRITE8_MEMBER(nightgal_state::blit_true_vregs_w)
{
	m_pen_data[offset] = data;
}

/********************************************
*
* Input Multiplexer handling
*
********************************************/

WRITE8_MEMBER(nightgal_state::mux_w)
{
	m_mux_data = ~data;
	//printf("%02x\n", m_mux_data);
}

READ8_MEMBER(nightgal_state::input_1p_r)
{
	UINT8 cr_clear = m_io_cr_clear->read();

	switch (m_mux_data)
	{
		case 0x01: return m_io_pl1_1->read() | cr_clear;
		case 0x02: return m_io_pl1_2->read() | cr_clear;
		case 0x04: return m_io_pl1_3->read() | cr_clear;
		case 0x08: return m_io_pl1_4->read() | cr_clear;
		case 0x10: return m_io_pl1_5->read() | cr_clear;
		case 0x20: return m_io_pl1_6->read() | cr_clear;
	}
	//printf("%04x\n", m_mux_data);

	return (m_io_pl1_1->read() & m_io_pl1_2->read() & m_io_pl1_3->read() &
			m_io_pl1_4->read() & m_io_pl1_5->read() & m_io_pl1_6->read()) | cr_clear;
}

READ8_MEMBER(nightgal_state::input_2p_r)
{
	UINT8 coin_port = m_io_coins->read();

	switch (m_mux_data)
	{
		case 0x01: return m_io_pl2_1->read() | coin_port;
		case 0x02: return m_io_pl2_2->read() | coin_port;
		case 0x04: return m_io_pl2_3->read() | coin_port;
		case 0x08: return m_io_pl2_4->read() | coin_port;
		case 0x10: return m_io_pl2_5->read() | coin_port;
		case 0x20: return m_io_pl2_6->read() | coin_port;
	}
	//printf("%04x\n", m_mux_data);

	return (m_io_pl2_1->read() & m_io_pl2_2->read() & m_io_pl2_3->read() &
			m_io_pl2_4->read() & m_io_pl2_5->read() & m_io_pl2_6->read()) | coin_port;
}

/********************************************
*
* Memory Maps
*
********************************************/

/********************************
* Night Gal
********************************/
#ifdef UNUSED_CODE
static ADDRESS_MAP_START( nightgal_map, AS_PROGRAM, 8, nightgal_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc100, 0xc100) AM_READ(nsc_latch_r)
	AM_RANGE(0xc200, 0xc200) AM_WRITE(nsc_latch_w)
	AM_RANGE(0xc300, 0xc30f) AM_WRITE(blit_vregs_w)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( nightgal_io, AS_IO, 8, nightgal_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01,0x01) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x02,0x03) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
//  AM_RANGE(0x10,0x10) AM_WRITE(output_w)
	AM_RANGE(0x10,0x10) AM_READ_PORT("DSWC")
	AM_RANGE(0x11,0x11) AM_READ_PORT("SYSA")
	AM_RANGE(0x12,0x12) AM_READ_PORT("DSWA")
	AM_RANGE(0x13,0x13) AM_READ_PORT("DSWB")
	AM_RANGE(0x11,0x11) AM_WRITE(mux_w)
	AM_RANGE(0x12,0x14) AM_WRITE(blitter_w) //data for the nsc to be processed
ADDRESS_MAP_END

static ADDRESS_MAP_START( nsc_map, AS_PROGRAM, 8, nightgal_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x0080) AM_READ(blitter_status_r)
	AM_RANGE(0x0081, 0x0083) AM_READ(nsc_blit_r)
	AM_RANGE(0x0080, 0x0086) AM_WRITE(nsc_true_blitter_w)

	AM_RANGE(0x00a0, 0x00af) AM_WRITE(blit_true_vregs_w)

	AM_RANGE(0x1100, 0x1100) AM_READWRITE(z80_latch_r,z80_latch_w) //irq control?
	AM_RANGE(0x1200, 0x1200) AM_READNOP //flip screen set bit
	AM_RANGE(0x1300, 0x130f) AM_READ(blit_vregs_r)
//  AM_RANGE(0x1000, 0xdfff) AM_ROM AM_REGION("gfx1", 0 )
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_WRITENOP
ADDRESS_MAP_END
#endif

/********************************
* Sexy Gal
********************************/

static ADDRESS_MAP_START( sexygal_map, AS_PROGRAM, 8, nightgal_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_RAM //???
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(royalqn_comm_r, royalqn_comm_w) AM_SHARE("comms_ram")
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sexygal_io, AS_IO, 8, nightgal_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x01) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
//  AM_RANGE(0x10,0x10) AM_WRITE(output_w)
	AM_RANGE(0x10,0x10) AM_READ_PORT("DSWC")
	AM_RANGE(0x11,0x11) AM_READ_PORT("SYSA") AM_WRITE(mux_w)
	AM_RANGE(0x12,0x12) AM_MIRROR(0xe8) AM_READ_PORT("DSWA") AM_WRITE(royalqn_blitter_0_w)
	AM_RANGE(0x13,0x13) AM_MIRROR(0xe8) AM_READ_PORT("DSWB") AM_WRITE(royalqn_blitter_1_w)
	AM_RANGE(0x14,0x14) AM_MIRROR(0xe8) AM_READNOP AM_WRITE(royalqn_blitter_2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sexygal_nsc_map, AS_PROGRAM, 8, nightgal_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x0080) AM_READ(blitter_status_r)
	AM_RANGE(0x0081, 0x0083) AM_READ(royalqn_nsc_blit_r)
	AM_RANGE(0x0080, 0x0086) AM_WRITE(sexygal_nsc_true_blitter_w)

	AM_RANGE(0x00a0, 0x00af) AM_WRITE(blit_true_vregs_w)
	AM_RANGE(0x00b0, 0x00b0) AM_WRITENOP // bltflip register

	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0x2c00) AM_READWRITE(royalqn_comm_r, royalqn_comm_w) AM_SHARE("comms_ram")
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_WRITENOP
ADDRESS_MAP_END

/********************************
* Royal Queen
********************************/

static ADDRESS_MAP_START( royalqn_map, AS_PROGRAM, 8, nightgal_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_NOP
	AM_RANGE(0xc000, 0xdfff) AM_READWRITE(royalqn_comm_r, royalqn_comm_w) AM_SHARE("comms_ram")
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( royalqn_io, AS_IO, 8, nightgal_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01,0x01) AM_MIRROR(0xec) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x02,0x03) AM_MIRROR(0xec) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x10,0x10) AM_MIRROR(0xe8) AM_READ_PORT("DSWC") AM_WRITENOP //AM_WRITE(output_w)
	AM_RANGE(0x11,0x11) AM_MIRROR(0xe8) AM_READ_PORT("SYSA") AM_WRITE(mux_w)
	AM_RANGE(0x12,0x12) AM_MIRROR(0xe8) AM_READ_PORT("DSWA") AM_WRITE(royalqn_blitter_0_w)
	AM_RANGE(0x13,0x13) AM_MIRROR(0xe8) AM_READ_PORT("DSWB") AM_WRITE(royalqn_blitter_1_w)
	AM_RANGE(0x14,0x14) AM_MIRROR(0xe8) AM_READNOP AM_WRITE(royalqn_blitter_2_w)
	AM_RANGE(0x15,0x15) AM_MIRROR(0xe8) AM_NOP
	AM_RANGE(0x16,0x16) AM_MIRROR(0xe8) AM_NOP
	AM_RANGE(0x17,0x17) AM_MIRROR(0xe8) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( royalqn_nsc_map, AS_PROGRAM, 8, nightgal_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x0080) AM_READ(blitter_status_r)
	AM_RANGE(0x0081, 0x0083) AM_READ(royalqn_nsc_blit_r)
	AM_RANGE(0x0080, 0x0086) AM_WRITE(nsc_true_blitter_w)

	AM_RANGE(0x00a0, 0x00af) AM_WRITE(blit_true_vregs_w)
	AM_RANGE(0x00b0, 0x00b0) AM_WRITENOP // bltflip register

	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0x2c00) AM_READWRITE(royalqn_comm_r,royalqn_comm_w)
	AM_RANGE(0x4000, 0x4000) AM_NOP
	AM_RANGE(0x8000, 0x8000) AM_NOP //open bus or protection check
	AM_RANGE(0xc000, 0xdfff) AM_MIRROR(0x2000) AM_ROM
ADDRESS_MAP_END

/********************************************
*
* Input ports
*
********************************************/

static INPUT_PORTS_START( sexygal )
	PORT_START("CR_CLEAR")
	PORT_DIPNAME( 0x40, 0x40, "Credit Clear-1" )//button
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Credit Clear-2" )//button
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) //player-1 side
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) //player-2 side

	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_CODE(KEYCODE_3)//rate button

	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //another D button

	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED ) //another opt 1 button

	PORT_START("PL1_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 4")

	PORT_START("PL1_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Pass") //???
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_CODE(KEYCODE_4) PORT_PLAYER(2)//rate button

	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //another D button

	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED ) //another opt 1 button

	PORT_START("PL2_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 1") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 2") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 3") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 4") PORT_PLAYER(2)

	PORT_START("PL2_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Pass") PORT_PLAYER(2) //???
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DSWB" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, "DSWC" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void nightgal_state::machine_start()
{
	save_item(NAME(m_nsc_latch));
	save_item(NAME(m_z80_latch));
	save_item(NAME(m_mux_data));

	save_item(NAME(m_blit_raw_data));
	save_item(NAME(m_true_blit));
	save_item(NAME(m_pen_data));
	save_item(NAME(m_pen_raw_data));
}

void nightgal_state::machine_reset()
{
	m_nsc_latch = 0;
	m_z80_latch = 0;
	m_mux_data = 0;

	memset(m_blit_raw_data, 0, ARRAY_LENGTH(m_blit_raw_data));
	memset(m_true_blit, 0, ARRAY_LENGTH(m_true_blit));
	memset(m_pen_data, 0, ARRAY_LENGTH(m_pen_data));
	memset(m_pen_raw_data, 0, ARRAY_LENGTH(m_pen_raw_data));
}

static MACHINE_CONFIG_START( royalqn, nightgal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,MASTER_CLOCK / 8)        /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(royalqn_map)
	MCFG_CPU_IO_MAP(royalqn_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nightgal_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", NSC8105, MASTER_CLOCK / 8)
	MCFG_CPU_PROGRAM_MAP(royalqn_nsc_map)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")


	/* video hardware */
	/* TODO: blitter clock is MASTER_CLOCK / 4, 320 x 264 pixels, 256 x 224 of visible area */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(nightgal_state, screen_update_nightgal)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x10)
	MCFG_PALETTE_INIT_OWNER(nightgal_state, nightgal)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK / 8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(nightgal_state, input_1p_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(nightgal_state, input_2p_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sexygal, royalqn )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sexygal_map)
	MCFG_CPU_IO_MAP(sexygal_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(nightgal_state, nmi_line_pulse, 244)//???

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(sexygal_nsc_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nightgal_state,  irq0_line_hold)

	MCFG_DEVICE_REMOVE("aysnd")

	MCFG_SOUND_ADD("ymsnd", YM2203, MASTER_CLOCK / 8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(nightgal_state, input_1p_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(nightgal_state, input_2p_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ngalsumr, royalqn )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(royalqn_map)
	MCFG_CPU_IO_MAP(royalqn_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(nightgal_state, nmi_line_pulse, 244)//???
MACHINE_CONFIG_END

/*
Night Gal
(c)1984 NihonBussan Co.,Ltd.

OSC:20MHz
CPU:Z80
SND:AY-3-8910
ETC:CUSTOM(The surface of the chip is scrached, so the name of the chip is unknown), MemoryBackup

NGAL_01.BIN graphic
NGAL_02.BIN graphic
NGAL_03.BIN graphic
NGAL_04.BIN graphic
NGAL_05.BIN graphic
NGAL_06.BIN graphic
NGAL_07.BIN graphic
NGAL_08.BIN graphic
NGAL_09.BIN program
NGAL_10.BIN program
NGAL_11.BIN program
NGAL_12.BIN program
NGAL_BP.BIN color

Dumped by Gastroptosis. 2000/06/04
Dumped by Uki. 2000/06/11
?
*/

ROM_START( nightgal )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ngal_10.bin", 0x00000, 0x02000, CRC(5eb28742) SHA1(d48045b7cbce69093494c4ec764cf4fb3c120bd6) )
	ROM_LOAD( "ngal_11.bin", 0x02000, 0x02000, CRC(c52f7942) SHA1(e23b9e4936f9b3111ea14c0250190ee6de1ed4ab) )
	ROM_LOAD( "ngal_12.bin", 0x04000, 0x02000, CRC(515e69a7) SHA1(234247c829c2b082360d7d44c1488fc5fcf45cd2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ngal_09.bin", 0x0c000, 0x02000, CRC(da3dcc08) SHA1(6f5319c1777dabf7041286698ac8f25eca1545a1) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "ngal_01.bin",  0x00000, 0x02000, CRC(8e4c92ad) SHA1(13cebe765ebabe6be79c9c9ac3f778550e450380) )
	ROM_LOAD( "ngal_02.bin",  0x02000, 0x02000, CRC(c60f7dc1) SHA1(273fd05c62e1efe26538efd2d4f0973c5eba65e4) )
	ROM_LOAD( "ngal_03.bin",  0x04000, 0x02000, CRC(824b7d9e) SHA1(04d3340cbb954add0d70c093df4ccb669e5ed12b) )
	ROM_LOAD( "ngal_04.bin",  0x06000, 0x02000, CRC(d1981ad6) SHA1(668a7aaa43b4e727a90a4e11cee659509465e546) )
	ROM_LOAD( "ngal_05.bin",  0x08000, 0x02000, CRC(ed5e4a28) SHA1(5d9441a2c79ad3a3d1b2ad7187bba49bdbd0a76e) )
	ROM_LOAD( "ngal_06.bin",  0x0a000, 0x02000, CRC(81de181d) SHA1(b528c0c82dc240dc34fe5b2fcb77b0e5f5701c7c) )
	ROM_LOAD( "ngal_07.bin",  0x0c000, 0x02000, CRC(de0e6f9b) SHA1(ac3428b0ba560e41f46dfb906da2c5e0f034d31c) )
	ROM_LOAD( "ngal_08.bin",  0x0e000, 0x02000, CRC(2c5cc9a0) SHA1(9ba797eb2fc549e9f16dfee442fd7de53a58d4f0) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "ngal_bp.bin", 0x00, 0x20, CRC(19255a7d) SHA1(4ac6316f7d8b575f28d33564b422b68993a4e484) )
ROM_END

/*

Night Bunny
(c)1984 Nichibutsu

CPU: Z80
Sound: AY-3-8910
OSC: 20.000MHz
Other: surface scrached DIP40 (NB1413M3?)

ROMs:
1.3A
2.3C
3.3D
4.3F
5.3M
6.3N
7.3P
8.3S (2764)
MB7051.6S


dumped by sayu
--- Team Japump!!! ---


*/
ROM_START( ngtbunny )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "6.3n", 0x00000, 0x02000, CRC(3e39c0ef) SHA1(56287fb19ff1a61ff606454315e433ecd2e9318a) )
	ROM_LOAD( "7.3p", 0x02000, 0x02000, CRC(34024380) SHA1(ba535e2b198f55e68a45ad7030b12c9aa1389aea) )
	ROM_LOAD( "8.3s", 0x04000, 0x02000, CRC(9bf96168) SHA1(f0e9302bc9577fe779b56cb72035672368c94481) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "5.3m",  0x0c000, 0x02000, CRC(b8a82966) SHA1(9f86b3208fb48f9735cfc4f8e62680f0cb4a92f0) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "1.3a",  0x00000, 0x02000, CRC(16776c5f) SHA1(a2925eaed938ae3985ea796658b62d6fafb6412b) )
	ROM_LOAD( "2.3c",  0x02000, 0x02000, CRC(dffd2cc6) SHA1(34f45b20596f69c44dc01c7aef765ab3ddaa076b) )
	ROM_LOAD( "3.3d",  0x04000, 0x02000, CRC(c532ca49) SHA1(b01b08e99e24649c45ce1833f830775d6f532f6b) )
	ROM_LOAD( "4.3f",  0x06000, 0x02000, CRC(ade1d0d8) SHA1(c3ad6bfeed878132d02770d97f6392daa509de5f) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "mb7051.6s", 0x00, 0x20, CRC(006b42d6) SHA1(ced119a299a9a7694d2fa0ef178b69d76abd0d6f) )
ROM_END

ROM_START( royalngt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rn6.3n", 0x00000, 0x02000, CRC(abbf38b9) SHA1(455dcec2ac2187b7216ff53fbbb8975b763fb981) )
	ROM_LOAD( "rn7.3p", 0x02000, 0x02000, CRC(ae9c082b) SHA1(ee3effea653f972fd732453e9ab72f48e75410f8) )
	ROM_LOAD( "rn8.3s", 0x04000, 0x02000, CRC(1371a83a) SHA1(c7107b62534837dd51bb4a93ba9a690f91393930) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "rn5.3l",  0x0c000, 0x02000, CRC(b8a82966) SHA1(9f86b3208fb48f9735cfc4f8e62680f0cb4a92f0) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "rn1.3a",  0x00000, 0x02000, CRC(16776c5f) SHA1(a2925eaed938ae3985ea796658b62d6fafb6412b) )
	ROM_LOAD( "rn2.3c",  0x02000, 0x02000, CRC(dffd2cc6) SHA1(34f45b20596f69c44dc01c7aef765ab3ddaa076b) )
	ROM_LOAD( "rn3.3d",  0x04000, 0x02000, CRC(31fb1d47) SHA1(41441bc2613c95dc810cad569cbaa0c023c819ba) )
	ROM_LOAD( "rn4.3e",  0x06000, 0x02000, CRC(ade1d0d8) SHA1(c3ad6bfeed878132d02770d97f6392daa509de5f) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "f5.6s", 0x00, 0x20, CRC(006b42d6) SHA1(ced119a299a9a7694d2fa0ef178b69d76abd0d6f) )
ROM_END

ROM_START( royalqn )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "b10.3s", 0x00000, 0x02000, CRC(67a4abfe) SHA1(1f408f7540185ce136507a8aca8d3beb234979d5) )
	ROM_LOAD( "a11.3t", 0x02000, 0x02000, CRC(e7c5395b) SHA1(5131ab9b0fbf1b7b4d410aa2a57eceaf47f8ec3a) )
	ROM_LOAD( "a12.3v", 0x04000, 0x02000, CRC(4e8efda4) SHA1(1959491fd899a4d85fd067d7674592ec25188a75) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "rq9.3p",  0x0c000, 0x02000, CRC(34b4cf82) SHA1(01f49ca11a695d41c181e92217e228bc1656ee57) )

	ROM_REGION( 0xc000, "samples", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "rq1.3a",  0x00000, 0x02000, CRC(066449dc) SHA1(34838f5e3569b313306ce465e481b934e938c837) )
	ROM_LOAD( "rq2.3c",  0x02000, 0x02000, CRC(c467adb5) SHA1(755ebde6229bbf0c7d9293e0becb7506d9aa9d49) )
	ROM_LOAD( "rq3.3d",  0x04000, 0x02000, CRC(7e5a7a2d) SHA1(5770cd832de59ff4f61ac40eca8c2238ff7b582d) )
	ROM_LOAD( "rq4.3f",  0x06000, 0x02000, CRC(afb3e333) SHA1(a3ddf800925df748db4f71a9dcb05ff0e838d767) )
	ROM_LOAD( "rq5.3j",  0x08000, 0x02000, CRC(1e81d0f6) SHA1(f38fbaf1f2cfabb5ba0e4a06964f9a2862b7569d) )
	ROM_LOAD( "rq6.3k",  0x0a000, 0x02000, CRC(45b2bb9c) SHA1(935e72d45585576b8f8c140ef2fdedfe6578d1c8) )
	ROM_LOAD( "rq7.3l",  0x0c000, 0x02000, CRC(c43ee2dd) SHA1(235e15d0a5e3ccbdf47960241faf747eaa2524f6) )
	ROM_LOAD( "rq8.3n",  0x0e000, 0x02000, CRC(3a79b3cc) SHA1(0b7b13cd1ee35ec3475d33c734c6d8f757dddd96) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "ng.6s", 0x00, 0x20, CRC(19255a7d) SHA1(4ac6316f7d8b575f28d33564b422b68993a4e484) )
ROM_END

/*

Sexy Gal
(c)1985 Nihon Bussan

XG-1B (main board)
SGP-A (sub board)

CPU:    Z80-A
SOUND:  YM2203C
    DAC
OSC:    20.000MHz
    10.000MHz
    6.000MHz (sub board)
Chips:  CPU? 40pin
    CPU? 40pin (sub board)


1.3A    prg?

2.3C    chr.
3.3D
4.3E
5.3F
6.3H
7.3JK
8.3KL
9.3M

10.3N   Z80 prg.
11.3PR

12.S8B  prg./samples?
13.S7B
14.S6B

SG.7E   color


*/

ROM_START( sexygal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10.3n",  0x00000, 0x04000, CRC(53425b74) SHA1(1239c0527d00d693313366b7e3da669565f99ffd) )
	ROM_LOAD( "11.3pr", 0x04000, 0x04000, CRC(a3138b42) SHA1(1bf7f6e2c4020251379cc72fa731c17795f35e2e) )
	ROM_LOAD( "12.s8b", 0x08000, 0x04000, CRC(7ac4a984) SHA1(7b41c522387938fe7625c9a6c62a385d6635cc5e) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "1.3a",   0x0c000, 0x04000, CRC(f814cf27) SHA1(ceba1f14a202d926380039d7cb4669eb8be58539) ) // has a big (16 byte wide) ASCII 'Y.M' art, written in YMs (!)

	ROM_REGION( 0xc000, "samples", 0 )
	ROM_LOAD( "13.s7b",  0x04000, 0x04000, CRC(5eb75f56) SHA1(b7d81d786d1ac8d65a6a122140954eb89d76e8b4) )
	ROM_LOAD( "14.s6b",  0x08000, 0x04000, CRC(b4a2497b) SHA1(7231f57b4548899c886625e883b9972c0f30e9f2) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "2.3c",  0x00000, 0x04000, CRC(f719e09d) SHA1(c78411b4f974b3dd261d51e522e086fc30a96fcb) )
	ROM_LOAD( "3.3d",  0x04000, 0x04000, CRC(a84d9a89) SHA1(91d5978e35ba4acf9353a13ec22c22aeb8a35f12) )
	ROM_LOAD( "4.3e",  0x08000, 0x04000, CRC(f1cdbedb) SHA1(caacf2887a3a05e498d57d570a1e9873f95a5d5f) )
	ROM_LOAD( "5.3f",  0x0c000, 0x04000, CRC(76569186) SHA1(79cb32c1f1a96f90d59f331a01ca548936933b87) )
	ROM_LOAD( "6.3h",  0x10000, 0x04000, CRC(8b6268e4) SHA1(c57bb7fe8f079d8f202f370cd7bdce1cf0596ede) )
	ROM_LOAD( "7.3jk", 0x14000, 0x04000, CRC(c88f68b8) SHA1(512019f465c298ba8fbf0f6c285a9b0d6c8f7411) )
	ROM_LOAD( "8.3kl", 0x18000, 0x04000, CRC(4631e092) SHA1(961b10b556defe9e4ba84180149bb2ef4042dbe9) )
	ROM_LOAD( "9.3m",  0x1c000, 0x04000, CRC(198df711) SHA1(adf9531ee7058db2314811aba7568bd332632947) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "sg.7e", 0x00, 0x20, CRC(5786a035) SHA1(29d95a6fb076d64ca217206fcadde51993830a88) )
ROM_END

ROM_START( sweetgal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10.3n", 0x00000, 0x04000, CRC(0f6c4bf0) SHA1(50e5c6f08e124641f5df8938ccfcdebde18f6a0f) ) // sldh
	ROM_LOAD( "11.3p", 0x04000, 0x04000, CRC(7388e9b3) SHA1(e318d2d3888679bbd43a0aab68252fd359b7969d) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "1.3a",  0x0e000, 0x2000, CRC(5342c757) SHA1(b4ff84c45bd2c6a6a468f1d0daaf5b19c4dbf8fe) ) // sldh

	ROM_REGION( 0xc000, "samples", 0 ) // sound samples
	ROM_LOAD( "v2_12.bin",  0x00000, 0x04000, CRC(66a35be2) SHA1(4f0d73d753387acacc5ccc90e91d848a5ecce55e) )
	ROM_LOAD( "v2_13.bin",  0x04000, 0x04000, CRC(60785a0d) SHA1(71eaec3512c0b18b93c083c1808eec51cfd4f520) )
	ROM_LOAD( "v2_14.bin",  0x08000, 0x04000, CRC(149e84c1) SHA1(5c4e18637bef2f31bc3578cae6525fb6280fbc06) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "2.3c",  0x00000, 0x04000, CRC(3a3d78f7) SHA1(71e35529f30c43ee8ec2363f85fe17042f1d304e) ) // sldh
	ROM_LOAD( "3.3d",  0x04000, 0x04000, CRC(c6f9b884) SHA1(32d6fe1906a3f1f528f30dbd3f89971b2ea1925b) ) // sldh
	// all roms below match sexygal
	ROM_LOAD( "4.3e",  0x08000, 0x04000, CRC(f1cdbedb) SHA1(caacf2887a3a05e498d57d570a1e9873f95a5d5f) )
	ROM_LOAD( "5.3f",  0x0c000, 0x04000, CRC(76569186) SHA1(79cb32c1f1a96f90d59f331a01ca548936933b87) )
	ROM_LOAD( "6.3h",  0x10000, 0x04000, CRC(8b6268e4) SHA1(c57bb7fe8f079d8f202f370cd7bdce1cf0596ede) )
	ROM_LOAD( "7.3jk", 0x14000, 0x04000, CRC(c88f68b8) SHA1(512019f465c298ba8fbf0f6c285a9b0d6c8f7411) )
	ROM_LOAD( "8.3kl", 0x18000, 0x04000, CRC(4631e092) SHA1(961b10b556defe9e4ba84180149bb2ef4042dbe9) )
	ROM_LOAD( "9.3m",  0x1c000, 0x04000, CRC(198df711) SHA1(adf9531ee7058db2314811aba7568bd332632947) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "sg.7e", 0x00, 0x20, CRC(5786a035) SHA1(29d95a6fb076d64ca217206fcadde51993830a88) )
ROM_END

/*

Night Gal Summer (JPN Ver.)
(c)1985 Nihon Bussan

CPU:    Z80
SOUND:  AY-3-8910
    DAC
OSC:    20.000MHz
    6.000MHz (sub board)

Chips:  NG138507 (CPU?)
    Unknown 40pin
    Unknown 40pin


1S.IC7  prg./samples?
2S.IC6
3S.IC5

1.3A    chr.
2.3C
3.3D
4.3F
5.3H
6.3L

7.3P    main prg.
8.3S
9.3T
10.3V

NG2.6U  color



*/

ROM_START( ngalsumr )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "8.3s", 0x00000, 0x02000, CRC(30f81b12) SHA1(e264b0cdc6ff400643cba56847344c270e96a204) )
	ROM_LOAD( "9.3t", 0x02000, 0x02000, CRC(879fc493) SHA1(ec7c6928b5d4e46dcc99271466e7eb801f601a70) )
	ROM_LOAD( "10.3v", 0x04000, 0x02000, CRC(31211088) SHA1(960b781c420602be3de66565a030cf5ebdcc2ffb) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "7.3p",  0x0c000, 0x02000, CRC(20c55a25) SHA1(9dc88cb6c016b594264f7272d4fd5f30567e7c5d) )

	ROM_REGION( 0xc000, "samples", 0 )
	ROM_LOAD( "1s.ic7", 0x00000, 0x04000, CRC(47ad8a0f) SHA1(e3b1e13f0a5c613bd205338683bef8d005b54830) )
	ROM_LOAD( "2s.ic6", 0x04000, 0x04000, CRC(ca2a735f) SHA1(5980525a67fb0ffbfa04b82d805eee2463236ce3) )
	ROM_LOAD( "3s.ic5", 0x08000, 0x04000, CRC(5cf15267) SHA1(72e4b2aa59a50af6b1b25d5279b3b125bfe06d86) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "1.3a",  0x00000, 0x04000, CRC(9626f812) SHA1(ca7162811a0ba05dfaa2aa8cc93a2e898b326e9e) )
	ROM_LOAD( "2.3c",  0x04000, 0x04000, CRC(0d59cf7a) SHA1(600bc70d29853fb936f8adaef048d925cbae0ce9) )
	ROM_LOAD( "3.3d",  0x08000, 0x04000, CRC(2fb2ec0b) SHA1(2f1735e33906783b8c0b283455a2a079431e6f11) )
	ROM_LOAD( "4.3f",  0x0c000, 0x04000, CRC(c7b85199) SHA1(1c4ed2faf82f45d8a23c168793b02969f1201df6) )
	ROM_LOAD( "5.3h",  0x10000, 0x04000, CRC(feaca6a3) SHA1(6658c01ac5769e8317a1c7eec6802e7c96885710) )
	ROM_LOAD( "6.3l",  0x14000, 0x04000, CRC(de9e05f8) SHA1(724468eade222b513b7f39f0a24515f343428130) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "ng2.6u", 0x00, 0x20, CRC(0162a24a) SHA1(f7e1623c5bca3725f2e59ae2096b9bc42e0363bf) )
ROM_END

DRIVER_INIT_MEMBER(nightgal_state,royalqn)
{
	UINT8 *ROM = memregion("sub")->base();

	/* patch open bus / protection */
	ROM[0xc27e] = 0x02;
	ROM[0xc27f] = 0x02;
}

DRIVER_INIT_MEMBER(nightgal_state,ngalsumr)
{
	UINT8 *ROM = memregion("sub")->base();

	/* patch protection */
	ROM[0xd6ce] = 0x02;
	ROM[0xd6cf] = 0x02;
}

/* Type 1 HW */
GAME( 1984, nightgal, 0,        royalqn, sexygal, driver_device,  0,       ROT0, "Nichibutsu",   "Night Gal (Japan 840920 AG 1-00)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, ngtbunny, 0,        royalqn, sexygal, driver_device,  0,       ROT0, "Nichibutsu",   "Night Bunny (Japan 840601 MRN 2-10)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, royalngt, ngtbunny, royalqn, sexygal, driver_device,  0,       ROT0, "Royal Denshi", "Royal Night [BET] (Japan 840220 RN 2-00)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, royalqn,  0,        royalqn, sexygal, nightgal_state,  royalqn, ROT0, "Royal Denshi", "Royal Queen [BET] (Japan 841010 RQ 0-07)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
/* Type 2 HW */
GAME( 1985, sexygal,  0,        sexygal, sexygal, driver_device,  0,       ROT0, "Nichibutsu",   "Sexy Gal (Japan 850501 SXG 1-00)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1985, sweetgal, sexygal,  sexygal, sexygal, driver_device,  0,       ROT0, "Nichibutsu",   "Sweet Gal (Japan 850510 SWG 1-02)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
/* Type 3 HW */
GAME( 1985, ngalsumr, 0,        ngalsumr,sexygal, nightgal_state,  ngalsumr,ROT0, "Nichibutsu",   "Night Gal Summer", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
