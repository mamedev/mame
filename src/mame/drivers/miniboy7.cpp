// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

  MINI-BOY 7

  Driver by Roberto Fresca.

  Games running on this hardware:

  * Mini-Boy 7 - 1983, Bonanza Enterprises, Ltd.


*******************************************************************************

  Game Notes:

  Mini-Boy 7. Seven games in one, plus Ad message support.
  http://www.arcadeflyers.com/?page=thumbs&db=videodb&id=4275

  - Draw Poker.
  - 7-Stud Poker.
  - Black Jack.
  - Baccarat.
  - Hi-Lo.
  - Double-Up.
  - Craps.

  During attract mode display, pressing the service menu will allow you to
  add a custom ad to scroll during attract mode display. Up to 120 characters

*******************************************************************************

  Hardware Notes:
  --------------

  Board silkscreened on top:
  be MVX-001-01  ('be' is a Bonanza Enterprises logo).

  - CPU:            1x R6502P.
  - Sound:          1x AY-3-8910.
  - Video:          1x HD46505 HD6845SP.
  - RAM:            4x 6116
  - I/O             1x MC6821 PIA.
  - PRG ROMs:       6x 2764 (8Kb).
  - GFX ROMs:       1x 2732 (4Kb) for text layer.
                    3x 2764 (8Kb) for gfx tiles.

  - Clock:          1x 12.4725 MHz. Crystal.

  - Battery backup: (unknown)

  - 1x normal switch (SW1)
  - 1x 8 DIP switches bank (SW2)
  - 1x 4 DIP switches bank (SW3)

  - 1x 2x28 pins edge connector.
  - 1x 2x20 pins female connector.

  - 2x pots to handle the B-G background color/intensity.


*******************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  $0000 - $00FF   RAM     ; Zero Page (pointers and registers)
  $0100 - $01FF   RAM     ; 6502 Stack Pointer.
  $0200 - $07FF   RAM     ; R/W. (settings)

  $0800 - $0FFF   Video RAM A
  $1000 - $17FF   Color RAM A
  $1800 - $1FFF   Video RAM B
  $2000 - $27FF   Color RAM B

  $2800 - $2801   MC6845  ; MC6845 use $2800 for register addressing and $2801 for register values.

  $3000 - $3001   ?????   ; R/W. AY8910?
  $3080 - $3083   MC6821  ; R/W. PIA
  $3800 - $3800   ?????   ; R.

  $4000 - $FFFF   ROM     ; ROM space.


  *** mc6845 init ***
  register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
  value:     0x2F  0x25  0x28  0x44  0x27  0x06  0x25  0x25  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.


*******************************************************************************


  DRIVER UPDATES:


  [2007-06-19]

  - Initial release. Just a skeleton driver.


  [2007-06-20]

  - Confirmed the CPU as 6502.
  - Confirmed the CRT controller as 6845.
  - Corrected the total & visible area analyzing the 6845 registers.
  - Crystal documented via #define.
  - CPU clock derived from #defined crystal value.
  - Decoded all gfx properly.
  - Partially worked the GFX banks:
      - 2 bank (1bpp) for text layers and minor graphics.
      - 1 bank (3bpp) for cards, jokers, dices and big text graphics.


  [2010-07-30]

  - Added a new complete set. Now set as parent.
  - Corrected Xtal frequency.
  - Mapped the PIA MC6821 (not wired since is not totally understood).
  - Preliminary attempt to decode the color PROM.
  - Mapped the AY-3-8910, but still needs ports and some checks.
  - Added debug and technical notes.


  TODO:

  - Find the way to clean the lamps writes.
    (there are alternate writes that mess the lamps)

  - Implement fake pots for B-G background color

*******************************************************************************/


#define MASTER_CLOCK    XTAL_12_4725MHz    /* 12.4725 MHz */

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "miniboy7.lh"


class miniboy7_state : public driver_device
{
public:
	miniboy7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram_a(*this, "videoram_a"),
		m_colorram_a(*this, "colorram_a"),
		m_videoram_b(*this, "videoram_b"),
		m_colorram_b(*this, "colorram_b"),
		m_gfx1(*this, "gfx1"),
		m_gfx2(*this, "gfx2"),
		m_proms(*this, "proms"),
		m_input2(*this, "INPUT2"),
		m_dsw2(*this, "DSW2"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_videoram_a;
	required_shared_ptr<UINT8> m_colorram_a;
	required_shared_ptr<UINT8> m_videoram_b;
	required_shared_ptr<UINT8> m_colorram_b;
	required_region_ptr<UINT8> m_gfx1;
	required_region_ptr<UINT8> m_gfx2;
	required_region_ptr<UINT8> m_proms;
	required_ioport m_input2;
	required_ioport m_dsw2;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	DECLARE_WRITE8_MEMBER(ay_pa_w);
	DECLARE_WRITE8_MEMBER(ay_pb_w);
	DECLARE_READ8_MEMBER(pia_pb_r);
	DECLARE_WRITE_LINE_MEMBER(pia_ca2_w);

	void machine_reset();

	int get_color_offset(UINT8 tile, UINT8 attr, int ra, int px);
	MC6845_UPDATE_ROW(crtc_update_row);
	DECLARE_PALETTE_INIT(miniboy7);

private:
	UINT8 m_ay_pb;
	int m_gpri;
};


/***********************************
*          Video Hardware          *
***********************************/

int miniboy7_state::get_color_offset(UINT8 tile, UINT8 attr, int ra, int px)
{
/*  - bits -
    7654 3210
    xxxx ----   tiles color.
    ---- -xxx   tiles bank.
    ---- x---   seems unused. */

	int color = (attr >> 4) & 0x0f;

	if (attr & 0x04)
	{
		int bank = (attr & 0x03) << 8;
		UINT8 bitplane0 = m_gfx2[0x0000 + ((tile + bank) << 3) + ra];
		UINT8 bitplane1 = m_gfx2[0x2000 + ((tile + bank) << 3) + ra];
		UINT8 bitplane2 = m_gfx2[0x4000 + ((tile + bank) << 3) + ra];

		return (color << 3) + ((BIT(bitplane0 << px, 7) << 0) | (BIT(bitplane1 << px, 7) << 1) | (BIT(bitplane2 << px, 7) << 2));
	}
	else
	{
		int bank = (attr & 0x01) << 8;
		UINT8 bitplane0 = m_gfx1[((tile + bank) << 3) + ra];
		return (color << 3) + BIT(bitplane0 << px, 7);
	}
}

MC6845_UPDATE_ROW( miniboy7_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	for (UINT8 cx = 0; cx < x_count; cx+=1)
	{
		for (int px = 0; px < 8; px++)
		{
			int offset_a = (m_gpri ? 0x80 : 0) + get_color_offset(m_videoram_a[ma + cx], m_colorram_a[ma + cx], ra, px);
			int offset_b = (m_gpri ? 0 : 0x80) + get_color_offset(m_videoram_b[ma + cx], m_colorram_b[ma + cx], ra, px);
			UINT8 color_a = m_proms[offset_a] & 0x0f;
			UINT8 color_b = m_proms[offset_b] & 0x0f;

			if (color_a && (m_gpri || !color_b))          // videoram A has priority
				bitmap.pix32(y, (cx << 3) + px) = palette[offset_a];
			else if (color_b && (!m_gpri || !color_a))    // videoram B has priority
				bitmap.pix32(y, (cx << 3) + px) = palette[offset_b];
		}
	}
}

PALETTE_INIT_MEMBER(miniboy7_state, miniboy7)
{
/*
    prom bits
    7654 3210
    ---- ---x   red component?.
    ---- --x-   green component?.
    ---- -x--   blue component?.
    ---- x---   intensity?.
    xxxx ----   unused.
*/
	int i;

	/* 0000IBGR */
	if (m_proms == nullptr) return;

	for (i = 0;i < palette.entries();i++)
	{
		int bit0, bit1, bit2, r, g, b, inten, intenmin, intenmax;

		intenmin = 0xe0;
//      intenmin = 0xc2;
		intenmax = 0xff;

		/* intensity component */
		inten = (m_proms[i] >> 3) & 0x01;

		/* red component */
		bit0 = (m_proms[i] >> 0) & 0x01;
		r = (bit0 * intenmin) + (inten * (bit0 * (intenmax - intenmin)));

		/* green component */
		bit1 = (m_proms[i] >> 1) & 0x01;
		g = (bit1 * intenmin) + (inten * (bit1 * (intenmax - intenmin)));

		/* blue component */
		bit2 = (m_proms[i] >> 2) & 0x01;
		b = (bit2 * intenmin) + (inten * (bit2 * (intenmax - intenmin)));


		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void miniboy7_state::machine_reset()
{
	m_ay_pb = 0;
	m_gpri = 0;
}

WRITE8_MEMBER(miniboy7_state::ay_pa_w)
{
	// ---x xxxx    lamps
	// --x- ----    coins lockout
	// -x-- ----    coins meter
	// x--- ----    unused

	data = data ^ 0xff;

//    output_set_lamp_value(0, (data) & 1);         // [----x]
//    output_set_lamp_value(1, (data >> 1) & 1);    // [---x-]
//    output_set_lamp_value(2, (data >> 2) & 1);    // [--x--]
//    output_set_lamp_value(3, (data >> 3) & 1);    // [-x---]
//    output_set_lamp_value(4, (data >> 4) & 1);    // [x----]

	coin_counter_w(machine(), 0, data & 0x40);    // counter

//  popmessage("Out Lamps: %02x", data);
//  logerror("Out Lamps: %02x\n", data);

}

WRITE8_MEMBER(miniboy7_state::ay_pb_w)
{
	// ---- xxxx    unused
	// -xxx ----    HCD
	// x--- ----    DSW2 select

	m_ay_pb = data;
}

READ8_MEMBER(miniboy7_state::pia_pb_r)
{
	return (m_input2->read() & 0x0f) | ((m_dsw2->read() << (BIT(m_ay_pb, 7) ? 0 : 4)) & 0xf0);
}

WRITE_LINE_MEMBER(miniboy7_state::pia_ca2_w)
{
	m_gpri = state;
}


/***********************************
*      Memory Map Information      *
***********************************/

static ADDRESS_MAP_START( miniboy7_map, AS_PROGRAM, 8, miniboy7_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram") /* battery backed RAM? */
	AM_RANGE(0x0800, 0x0fff) AM_RAM AM_SHARE("videoram_a")
	AM_RANGE(0x1000, 0x17ff) AM_RAM AM_SHARE("colorram_a")
	AM_RANGE(0x1800, 0x1fff) AM_RAM AM_SHARE("videoram_b")
	AM_RANGE(0x2000, 0x27ff) AM_RAM AM_SHARE("colorram_b")
	AM_RANGE(0x2800, 0x2800) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x2801, 0x2801) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x3000, 0x3001) AM_DEVREADWRITE("ay8910", ay8910_device, data_r, address_data_w)  // FIXME
	AM_RANGE(0x3080, 0x3083) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x3800, 0x3800) AM_READNOP // R (right after each read, another value is loaded to the ACCU, so it lacks of sense)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*

'maincpu' (E190): unmapped program memory byte read from 3800
'maincpu' (E190): unmapped program memory byte read from 3800
'maincpu' (E190): unmapped program memory byte read from 3800
'maincpu' (E190): unmapped program memory byte read from 3800

'maincpu' (CF41): unmapped program memory byte read from 3081
'maincpu' (CF41): unmapped program memory byte write to 3081 = 00
'maincpu' (CF41): unmapped program memory byte read from 3083
'maincpu' (CF41): unmapped program memory byte write to 3083 = 00
'maincpu' (CF41): unmapped program memory byte read from 3080
'maincpu' (CF41): unmapped program memory byte write to 3080 = 00
'maincpu' (CF41): unmapped program memory byte read from 3082
'maincpu' (CF41): unmapped program memory byte write to 3082 = 00
'maincpu' (CF41): unmapped program memory byte read from 3081
'maincpu' (CF41): unmapped program memory byte write to 3081 = 3F
'maincpu' (CF41): unmapped program memory byte read from 3083
'maincpu' (CF41): unmapped program memory byte write to 3083 = 34

'maincpu' (CF5A): unmapped program memory byte write to 3000 = 0E
'maincpu' (CF61): unmapped program memory byte write to 3001 = FF
'maincpu' (CF5A): unmapped program memory byte write to 3000 = 0F
'maincpu' (CF61): unmapped program memory byte write to 3001 = FF
'maincpu' (CF5A): unmapped program memory byte write to 3000 = 07
'maincpu' (CF61): unmapped program memory byte write to 3001 = FF
'maincpu' (CF5A): unmapped program memory byte write to 3000 = 0E
'maincpu' (CF61): unmapped program memory byte write to 3001 = FF
'maincpu' (CF5A): unmapped program memory byte write to 3000 = 0F
'maincpu' (CF61): unmapped program memory byte write to 3001 = FF

  ... CRTC init (snap) --> $CF2D: JSR $CF76

'maincpu' (E189): unmapped program memory byte read from 3800

'maincpu' (E1A0): unmapped program memory byte write to 3000 = 0E
'maincpu' (E1A3): unmapped program memory byte read from 3000
'maincpu' (E1A8): unmapped program memory byte write to 3001 = 00
'maincpu' (E1BF): unmapped program memory byte write to 3000 = 0E
'maincpu' (E1C2): unmapped program memory byte read from 3000
'maincpu' (E1CA): unmapped program memory byte write to 3001 = 1F

'maincpu' (E189): unmapped program memory byte read from 3800
'maincpu' (E189): unmapped program memory byte read from 3800
'maincpu' (E189): unmapped program memory byte read from 3800
'maincpu' (E189): unmapped program memory byte read from 3800
'maincpu' (E189): unmapped program memory byte read from 3800
'maincpu' (E189): unmapped program memory byte read from 3800

*/

/***********************************
*           Input Ports            *
***********************************/

static INPUT_PORTS_START( miniboy7 )
	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("INPUT2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("RAM Reset")
	PORT_BIT( 0xfb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x06, 0x06, "Turns per Coin" )    PORT_DIPLOCATION("DSW2:2,3")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_DIPNAME( 0x18, 0x18, "Bonus Turns" )       PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "50000 100000" )
	PORT_DIPSETTING(    0x10, "100000 200000" )
	PORT_DIPSETTING(    0x08, "100000 300000" )
	PORT_DIPSETTING(    0x00, "200000 300000" )

	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, "Bartop" )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW2-6" )            PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW2-7" )            PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW2-8" )            PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***********************************
*         Graphics Layouts         *
***********************************/

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), 0 },    /* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/****************************************
*      Graphics Decode Information      *
****************************************/

static GFXDECODE_START( miniboy7 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 128 ) /* text layer */

	/* 0x000 cards
	   0x100 joker
	   0x200 dices
	   0x300 bigtxt */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 32 )

GFXDECODE_END

/***********************************
*         Machine Drivers          *
***********************************/

static MACHINE_CONFIG_START( miniboy7, miniboy7_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK/16) /* guess */
	MCFG_CPU_PROGRAM_MAP(miniboy7_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("INPUT1"))
	MCFG_PIA_READPB_HANDLER(READ8(miniboy7_state, pia_pb_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(miniboy7_state, pia_ca2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", 0))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", 0))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE((47+1)*8, (39+1)*8)                  /* Taken from MC6845, registers 00 & 04. Normally programmed with (value-1) */
	MCFG_SCREEN_VISIBLE_AREA(0*8, 37*8-1, 0*8, 37*8-1)    /* Taken from MC6845, registers 01 & 06 */
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", miniboy7)

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(miniboy7_state, miniboy7)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", MASTER_CLOCK/12) /* guess */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(miniboy7_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE("pia0", pia6821_device, ca1_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, MASTER_CLOCK/8)    /* guess */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(miniboy7_state, ay_pa_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(miniboy7_state, ay_pb_w))

MACHINE_CONFIG_END


/***********************************
*             Rom Load             *
***********************************/

/*

  Board silkscreened on top:
  be MVX-001-01  ('be' is a Bonanza Enterprises logo).

  1x 6502.
  1x AY-3-8910.
  1x MC6821.
  1x HD46505 HD6845SP (Handwritten sticker '107040').
  1x 12.4725 Crystal.

  .a1    2764    No sticker.
  .a3    2764    Stickered 'MB7 5-4'
  .a4    2764    Stickered 'MB7 4-4'
  .a6    2764    Stickered 'MB7 3-4'
  .a7    2764    Stickered 'MB7 2-4'
  .a8    2764    Stickered 'MB7 6-4'
  .d11   2732    Stickered 'MB7 ASC CG'
  .d12   2764    Stickered 'MB7 CG1'
  .d13   2764    Stickered 'MB7 CG2'
  .d14   2764    Stickered 'MB7 CG3'

  .e7    82s10 read as 82s129, stickered 'J'
  .f10   82s10 read as 82s129, stickered 'J'

*/
ROM_START( miniboy7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mb7_6-4.a8",   0x4000, 0x2000, CRC(a3fdea08) SHA1(2f1a74274005b8c77eb4254d0220206ae4175834) )
	ROM_LOAD( "mb7_2-4.a7",   0x6000, 0x2000, CRC(396e7250) SHA1(8f8c86cc412269157b16ad883638b38bb21345d7) )
	ROM_LOAD( "mb7_3-4.a6",   0x8000, 0x2000, CRC(360a7f7c) SHA1(d98bcfd320680e88b07182d78b4e56fc5579874d) )
	ROM_LOAD( "mb7_4-4.a4",   0xa000, 0x2000, CRC(bff8e334) SHA1(1d09a86b4dbfec6522b326683febaf7426f723e0) )
	ROM_LOAD( "mb7_5-4.a3",   0xc000, 0x2000, CRC(d610bed3) SHA1(67e44ce2345d5429d6ccf4833de207ff6518c534) )
	ROM_LOAD( "nosticker.a1", 0xe000, 0x2000, CRC(5f715a12) SHA1(eabe0e4ee2e110c6ce4fd58c9d36ba80a612d4b5) )    /* ROM 1-4? */

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mb7_asc_cg.d11", 0x0000, 0x1000, CRC(84f78ee2) SHA1(c434e8a9b19ef1394b1dac67455f859eef299f95) )  /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "mb7_cg1.d12",    0x0000, 0x2000, CRC(5f3e3b93) SHA1(41ab6a42a41ddeb8b6b76f4d790bf9fb9e7c32a3) )  /* bitplane 1 */
	ROM_LOAD( "mb7_cg2.d13",    0x2000, 0x2000, CRC(b3362650) SHA1(603907fd3a0049c0a3e1858c4329bf9fd58137f6) )  /* bitplane 2 */
	ROM_LOAD( "mb7_cg3.d14",    0x4000, 0x2000, CRC(10c2bf71) SHA1(23a01625b0fc0b772054ee4bc026d2257df46a03) )  /* bitplane 3 */

	ROM_REGION( 0x0200, "proms", ROMREGION_INVERT )    /* both bipolar PROMs are identical */
	ROM_LOAD( "j.e7",   0x0000, 0x0100, CRC(4b66215e) SHA1(de4a8f1ee7b9bea02f3a5fc962358d19c7a871a0) ) /* N82S129N BPROM simply labeled J */
	ROM_LOAD( "j.f10",  0x0100, 0x0100, CRC(4b66215e) SHA1(de4a8f1ee7b9bea02f3a5fc962358d19c7a871a0) ) /* N82S129N BPROM simply labeled J */
ROM_END

ROM_START( miniboy7a ) /* The term CREDIT has been changed to POINT is this version, other changes?? */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mb7_1-11.a8",  0x4000, 0x2000, CRC(e1c0f8f2) SHA1(0790dc37374cf12313ae13adaea2c6e7338e0dbc) )
	ROM_LOAD( "mb7_2-11.a7",  0x6000, 0x2000, CRC(596040a3) SHA1(bb68b9fd12fba09c3d7c9dec70cf4770d31f911b) )
	ROM_LOAD( "mb7_3-11.a5",  0x8000, 0x2000, CRC(41a9816f) SHA1(9f7853498fcc6ead7cba619421a60335a48dfe57) )
	ROM_LOAD( "mb7_4-11.a4",  0xa000, 0x2000, CRC(bafb08fa) SHA1(004e95a81c94ee40701a604cca4023e6fdece54f) )
	ROM_LOAD( "mb7_5-11.a3",  0xc000, 0x2000, CRC(a8334503) SHA1(ab63f0f602e385445a322663e2e0d6008a25bf5c) )
	ROM_LOAD( "mb7_6-11.a1",  0xe000, 0x2000, CRC(ca9b9b20) SHA1(c6cd793a15948601faa051a4643b14fd3d8bda0b) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mb7_0.11d",   0x0000, 0x1000, CRC(84f78ee2) SHA1(c434e8a9b19ef1394b1dac67455f859eef299f95) )    /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "mb7_1.12d",   0x0000, 0x2000, CRC(5f3e3b93) SHA1(41ab6a42a41ddeb8b6b76f4d790bf9fb9e7c32a3) )  /* bitplane 1 */
	ROM_LOAD( "mb7_2.13d",   0x2000, 0x2000, CRC(b3362650) SHA1(603907fd3a0049c0a3e1858c4329bf9fd58137f6) )  /* bitplane 2 */
	ROM_LOAD( "mb7_3.14d",   0x4000, 0x2000, CRC(10c2bf71) SHA1(23a01625b0fc0b772054ee4bc026d2257df46a03) )  /* bitplane 3 */

	ROM_REGION( 0x0200, "proms", ROMREGION_INVERT )    /* both bipolar PROMs are identical */
	ROM_LOAD( "j.e7",   0x0000, 0x0100, CRC(4b66215e) SHA1(de4a8f1ee7b9bea02f3a5fc962358d19c7a871a0) ) /* N82S129N BPROM simply labeled J */
	ROM_LOAD( "j.f10",  0x0100, 0x0100, CRC(4b66215e) SHA1(de4a8f1ee7b9bea02f3a5fc962358d19c7a871a0) ) /* N82S129N BPROM simply labeled J */
ROM_END


/***********************************
*           Game Drivers           *
***********************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     STATE          INIT   ROT    COMPANY                     FULLNAME             FLAGS             LAYOUT
GAMEL( 1983, miniboy7,  0,        miniboy7, miniboy7, driver_device, 0,     ROT0, "Bonanza Enterprises, Ltd", "Mini-Boy 7 (set 1)", MACHINE_NO_COCKTAIL, layout_miniboy7 )
GAMEL( 1983, miniboy7a, miniboy7, miniboy7, miniboy7, driver_device, 0,     ROT0, "Bonanza Enterprises, Ltd", "Mini-Boy 7 (set 2)", MACHINE_NO_COCKTAIL, layout_miniboy7 )
