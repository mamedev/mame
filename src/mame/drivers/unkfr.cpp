// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/****************************************************************************************

  Unknown french encrypted system.
  Driver by Roberto Fresca.

*****************************************************************************************

  Hardware Notes:
  ---------------

  Based on chip manufacturing should be manufactured in 1998


  Specs:
  ------

  1x Zilog Z0840004PSE (4MHz Z80 CPU).
  1x GI AY-3-8910 (sound).
  1x LM356N (Low Voltage Audio Power Amplifier).

  ROMs: 2x 27C256 Program ROMs (I, II). 
        3x 27C256 GFX ROMs (R, V, B).

  RAMs: 1x KM62256ALP-10 (near prg ROMs).
        2x CY6264-70SNC (Near GFX ROMs).

  1x oscillator 18.432 MHz.

  1x 8 DIP Switches bank (near ay8910).
  1x Volume Pot (betweeen the audio amp and ay8910).
  1x Motorola MCT1413 (High Current Darlington Transistor Array, same as ULN2003).

  1x 2x28 Edge connector (pins 1-2-27-28 from component side are GND).

*****************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


#define MASTER_CLOCK    XTAL_18_432MHz      // OK
#define CPU_CLOCK       MASTER_CLOCK/4      // Guess... However it's a 4MHz. Z80.
#define SND_CLOCK       MASTER_CLOCK/12     // Guess... Approx 1.5 MHz.


class unkfr_state : public driver_device
{
public:
	unkfr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_videoram(*this, "videoram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(unkfr_videoram_w);
	DECLARE_DRIVER_INIT(unkfr);

	tilemap_t *m_bg_tilemap;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(unkfr);
	uint32_t screen_update_unkfr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*********************************************
*               Video Hardware               *
*********************************************/

WRITE8_MEMBER( unkfr_state::unkfr_videoram_w )
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(unkfr_state::get_bg_tile_info)
{
	uint8_t *videoram = m_videoram;
/*  - bits -
    7654 3210
    xxxx xxxx   tiles code.

    only one color code
*/
	int code = videoram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}


void unkfr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(unkfr_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 28, 32);
}


uint32_t unkfr_state::screen_update_unkfr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


PALETTE_INIT_MEMBER(unkfr_state, unkfr)
{
}


/**********************************************
*            Read / Write Handlers            *
**********************************************/


/*********************************************
*           Memory Map Information           *
*********************************************/

static ADDRESS_MAP_START( unkfr_map, AS_PROGRAM, 8, unkfr_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM_WRITE(unkfr_videoram_w) AM_SHARE("videoram")  // placeholder
//	AM_RANGE(0x5000, 0x5fff) AM_RAM  // placeholder
ADDRESS_MAP_END

static ADDRESS_MAP_START( unkfr_portmap, AS_IO, 8, unkfr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x00, 0x00)  AM_READ_PORT("DSW1")
//  AM_RANGE(0x01, 0x01)  AM_READ_PORT("IN0")
//  AM_RANGE(0x02, 0x02)  AM_READ_PORT("IN1")
	AM_RANGE(0xf8, 0xf8) AM_DEVWRITE("aysnd", ay8910_device, data_w)     // (placeholder) AY8910 data
	AM_RANGE(0xfa, 0xfa) AM_DEVWRITE("aysnd", ay8910_device, address_w)  // (placeholder) AY8910 control
ADDRESS_MAP_END


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( unkfr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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


/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( unkfr )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
GFXDECODE_END


/*********************************************
*              Machine Drivers               *
*********************************************/

static MACHINE_CONFIG_START( unkfr )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK) /* guess */
	MCFG_CPU_PROGRAM_MAP(unkfr_map)
	MCFG_CPU_IO_MAP(unkfr_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", unkfr_state,  nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(unkfr_state, screen_update_unkfr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", unkfr)
	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(unkfr_state, unkfr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, SND_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( unkfr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "i.bin",  0x8000, 0x6000, CRC(902efc27) SHA1(5992cdc647acd622c73adefac1aa66e77b5ccc4f) )
	ROM_CONTINUE(       0x0000, 0x2000)
	ROM_LOAD( "ii.bin", 0x8000, 0x6000, CRC(855c1ea3) SHA1(80c89bfbdf3d0d69aed7333e9aa93db6aff7b704) )
	ROM_CONTINUE(       0x2000, 0x2000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "b.bin", 0x0000, 0x8000, CRC(56ac5874) SHA1(7ae63f930b07cb1b4989c8328fcc3627d8ff68f8) )
	ROM_LOAD( "v.bin", 0x8000, 0x8000, CRC(4c5a7e43) SHA1(17e52ed73f9e8822b53bebc31c9320f5589ef70a) )
	ROM_LOAD( "r.bin", 0x10000, 0x8000, CRC(f8a358fe) SHA1(5c4051de156014a5c2400f4934e2136b38bfed8c) )
ROM_END


/*********************************************
*                Driver Init                 *
*********************************************/

DRIVER_INIT_MEMBER(unkfr_state, unkfr)
{
//  Attempt to decrypt/descramble the data lines... 

	uint8_t *ROM = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	int start = 0x0000;
	int i;

	for (i = start; i < size; i++)
	{
//		ROM[i] = ROM[i] ^ 0xa0;
		ROM[i] = BITSWAP8(ROM[i], 7, 6, 5, 4, 3, 2, 0, 1);
	}
}


/*********************************************
*                Game Drivers                *
*********************************************/

/*    YEAR  NAME      PARENT    MACHINE   INPUT     STATE         INIT   ROT   COMPANY     FULLNAME                        FLAGS */
GAME( 198?, unkfr,    0,        unkfr,    unkfr,    unkfr_state,  unkfr, ROT0, "unknown", "Unknown French encrypted game", MACHINE_NOT_WORKING )
