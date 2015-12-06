// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca, David Haywood
/*******************************************************************************************

  Millennium Nuovo 4000 / Nuovo Millennium 4000
  (c) 2000 Sure Milano

  Driver by David Haywood and Angelo Salese.
  Additional work by Roberto Fresca.


  Notes:

  - At first start-up,an Italian msg pops up: "(translated) pcb has been hacked from external
    agent,it's advised to add an anti-spark device". Press F2 to enter Service Mode,then press
    Hold 5 to exit Service Mode.

  - This game is supposed to have 3 kind of graphics: billiard/pool balls, numbers and cans.
    If you go to the settings mode (F2), and then in "Impostazioni del Gioco" you disable all
    3 graphics (Simboli Biliardo, Simboli Numeri, and Simboli Barattoli --> "Non Abilitato"),
    The game will start using normal poker cards. A "illegal easter egg"... ;-)

  - HW name (stamped on the pcb) is "CHP4";


  TODO:

  - Add Touch Screen support;
  - H/V-blank bits emulation;
  - Protection PIC is unused?


============================================================================================

  Manufacturer: Sure
  Revision number: CHP4 1.5

  CPU
  1x PIC16C65B (u60)(read protected)
  1x MC68HC000FN12 (u61)
  1x U6295 (u53)(equivalent to M6295)
  1x resonator 1000j (close to 6295)
  1x oscillator 12.000MHz
  1x oscillator 14.31818MHz

  ROMs
  1x 27C020 (1)
  7x V29C51001T (2,3,4,5,6,27,28)
  1x PALCE22V10H (u74)(read protected)
  2x A40MX04-PL840010 (u2,u3)(read protected)

  RAM:
  4x CY62256L-70PC - 32K x 8 Static RAM.
  2x CY7C199-15PC  - 32K x 8 Static RAM

  Note
  1x 28x2 edge connector
  1x RS232 9pins connector
  1x trimmer (volume)
  1x 2positon jumper
  1x pushbutton (reset)

============================================================================================

  CHAMPION 4000 V 1.4
  (CMP4 1.3 on PCB)
  12.000000MHz
  14.31818MHz
  1000J
  MC68HC000FN12
  PIC16C74B
  PALCE22V10H
  U6295

============================================================================================

  Changes (2008-12-10, Roberto Fresca):

  - Completed normal Inputs/Outputs.
  - Added button-lamps calculation.
  - Created button-lamps layout.
  - Documented the PCB RAM.
  - Fixed NVRAM size based on PCB picture (2x CY62256L-70PC near the battery).
  - Added notes about the method to make appear the real poker cards.
  - Fixed the OKI 6295 frequency (1000 kHz resonator near). Now the game has more decent sounds.
  - Corrected CPU clock to 12 MHz. (main Xtal).

  Changes (2013-05-24, Roberto Fresca):

  - Added Top XXI (Version 1.2).
  - Added crystals and cpu clock through #define.
  - Added default NVRAM.
  - Button-lamps support.
  - Added technical notes.

  Changes (2014-02-24, Roberto Fresca):

  - Added Cherry Wheel (Version 1.7).
  - Created new memory map due to hardware differences.
  - Added default NVRAM. Otherwise the game checks the NVRAM,
     and get a division by 0 error, then resets itself.
  - Partial MCU simulation. Without it, only cherries appear
    and the player always wins.
  - Added proper button-lamps support.
  - Added technical notes.


*******************************************************************************************/

#define MAIN_CLOCK    XTAL_12MHz
#define SEC_CLOCK     XTAL_14_31818MHz

#define CPU_CLOCK     MAIN_CLOCK


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "mil4000.lh"


class mil4000_state : public driver_device
{
public:
	mil4000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sc0_vram(*this, "sc0_vram"),
		m_sc1_vram(*this, "sc1_vram"),
		m_sc2_vram(*this, "sc2_vram"),
		m_sc3_vram(*this, "sc3_vram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT16> m_sc0_vram;
	required_shared_ptr<UINT16> m_sc1_vram;
	required_shared_ptr<UINT16> m_sc2_vram;
	required_shared_ptr<UINT16> m_sc3_vram;
	tilemap_t *m_sc0_tilemap;
	tilemap_t *m_sc1_tilemap;
	tilemap_t *m_sc2_tilemap;
	tilemap_t *m_sc3_tilemap;
	UINT16 m_vblank;
	UINT16 m_hblank;
	UINT8 mcucomm;
	UINT8 mcudata;
	DECLARE_READ16_MEMBER(hvretrace_r);
	DECLARE_READ16_MEMBER(unk_r);
	DECLARE_READ16_MEMBER(chewheel_mcu_r);
	DECLARE_WRITE16_MEMBER(unk_w);
	DECLARE_WRITE16_MEMBER(chewheel_mcu_w);
	DECLARE_WRITE16_MEMBER(sc0_vram_w);
	DECLARE_WRITE16_MEMBER(sc1_vram_w);
	DECLARE_WRITE16_MEMBER(sc2_vram_w);
	DECLARE_WRITE16_MEMBER(sc3_vram_w);
	DECLARE_WRITE16_MEMBER(output_w);
	TILE_GET_INFO_MEMBER(get_sc0_tile_info);
	TILE_GET_INFO_MEMBER(get_sc1_tile_info);
	TILE_GET_INFO_MEMBER(get_sc2_tile_info);
	TILE_GET_INFO_MEMBER(get_sc3_tile_info);
	virtual void video_start() override;
	UINT32 screen_update_mil4000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


TILE_GET_INFO_MEMBER(mil4000_state::get_sc0_tile_info)
{
	UINT32 data = (m_sc0_vram[tile_index*2]<<16) | m_sc0_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (m_sc0_vram[tile_index*2+1] & 0x1f)+0;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(mil4000_state::get_sc1_tile_info)
{
	UINT32 data = (m_sc1_vram[tile_index*2]<<16) | m_sc1_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (m_sc1_vram[tile_index*2+1] & 0x1f)+0x10;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(mil4000_state::get_sc2_tile_info)
{
	UINT32 data = (m_sc2_vram[tile_index*2]<<16) | m_sc2_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (m_sc2_vram[tile_index*2+1] & 0x1f)+0x20;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(mil4000_state::get_sc3_tile_info)
{
	UINT32 data = (m_sc3_vram[tile_index*2]<<16) | m_sc3_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (m_sc3_vram[tile_index*2+1] & 0x1f)+0x30;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

void mil4000_state::video_start()
{
	m_sc0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mil4000_state::get_sc0_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_sc1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mil4000_state::get_sc1_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_sc2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mil4000_state::get_sc2_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_sc3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mil4000_state::get_sc3_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);

	m_sc1_tilemap->set_transparent_pen(0);
	m_sc2_tilemap->set_transparent_pen(0);
	m_sc3_tilemap->set_transparent_pen(0);
}

UINT32 mil4000_state::screen_update_mil4000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sc0_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_sc1_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_sc2_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_sc3_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

/*TODO*/
READ16_MEMBER(mil4000_state::hvretrace_r)
{
	UINT16 res;

	res = 0;

	m_vblank^=1;
	m_hblank^=1;

	/*V-Blank*/
	if (m_vblank)
		res|= 0x80;

	/*H-Blank*/
	if (m_hblank)
		res|= 0x40;

	return res;
}


WRITE16_MEMBER(mil4000_state::sc0_vram_w)
{
	m_sc0_vram[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(mil4000_state::sc1_vram_w)
{
	m_sc1_vram[offset] = data;
	m_sc1_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(mil4000_state::sc2_vram_w)
{
	m_sc2_vram[offset] = data;
	m_sc2_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(mil4000_state::sc3_vram_w)
{
	m_sc3_vram[offset] = data;
	m_sc3_tilemap->mark_tile_dirty(offset/2);
}

/*end of video stuff*/

/*
    --x- ---- ---- ---- Coin Counter
    ---- ---- -x-- ---- Prize
    ---- ---- --x- ---- Start
    ---- ---- ---x ---- Hold 5
    ---- ---- ---- x--- Hold 4
    ---- ---- ---- -x-- Hold 3
    ---- ---- ---- --x- Hold 2
    ---- ---- ---- ---x Hold 1
*/
WRITE16_MEMBER(mil4000_state::output_w)
{
	int i;

	for(i=0;i<3;i++)
		coin_counter_w(machine(), i, data & 0x2000);

	output_set_lamp_value(0, (data) & 1);       /* HOLD1 */
	output_set_lamp_value(1, (data >> 1) & 1);  /* HOLD2 */
	output_set_lamp_value(2, (data >> 2) & 1);  /* HOLD3 */
	output_set_lamp_value(3, (data >> 3) & 1);  /* HOLD4 */
	output_set_lamp_value(4, (data >> 4) & 1);  /* HOLD5 */
	output_set_lamp_value(5, (data >> 5) & 1);  /* START */
	output_set_lamp_value(6, (data >> 6) & 1);  /* PREMIO */

//  popmessage("%04x\n",data);
}

READ16_MEMBER(mil4000_state::chewheel_mcu_r)
{
/*  Damn thing!
    708010-708011 communicate with the MCU.
    The returned value is critical to the reels.

    Writes to MCU:

    0x11 --> seems quiet or null.
    0x1A --> command for reels state/gfx. Two bytes (command + parameter).
             The second value will be the parameter to feed.
             The MCU will respond with one value (see table below).

    bits:
    7654-3210
    ---- ----  = cherries.
    ---- ---x  = single bar.
    ---- --x-  = double bars.
    ---- --xx  = triple bars.
    ---- -x--  = unknown (combinations produce reset).
    ---- x---  = sevens.
    ---- x--x  = double plums.
    ---- x-x-  = plums.
    ---- x-xx  = bells.
    ---x ----  = blanks.
    -xx- ----  = some kind of gfx retrace.
    x--- ----  = reset the game

   There are other commands, like:
   0x1B (+4 parameters)
   0x1C (+1 parameter, normally 00)
   0x1D (+2 parameters)
   0x1E (+1 parameter)

   You can find a 1E-18 at start.

*/
	switch( mcucomm )   /* MCU command */
	{
		case 0x11:  /* Idle - Null */
		{
			logerror("Writes idle command 0x11 to MCU");
			return (machine().rand() & 0x0b);   // otherwise got corrupt gfx...
		}

		case 0x1a:  /* Reels state - Control */
		{
			logerror("MCU feedback to command 0x1a with data: %02x\n", mcudata);
			return (machine().rand() & 0x0b);
		}

		case 0x1b:  /* Unknown */
		{
			logerror("MCU feedback to command 0x1b with data: %02x\n", mcudata);
			return 0x00;
		}

		case 0x1c:  /* Unknown. Always 00's? */
		{
			logerror("MCU feedback to command 0x1c with data: %02x\n", mcudata);
			return 0x00;
		}

		case 0x1d:  /* Unknown */
		{
			logerror("MCU feedback to command 0x1d with data: %02x\n", mcudata);
			return 0x00;
		}

		case 0x1e:  /* Unknown, only one at boot (1e 18) */
		{
			logerror("MCU feedback to command 0x1e with data: %02x\n", mcudata);
			return 0x00;
		}
	}

	logerror("MCU feedback to unknown command: %02x\n", mcucomm);
	return (machine().rand() & 0x0b);   // otherwise got corrupt gfx...
}

WRITE16_MEMBER(mil4000_state::chewheel_mcu_w)
{
	if ((data == 0x11)||(data == 0x1a)||(data == 0x1b)||(data == 0x1c)||(data == 0x1d)||(data == 0x1e))
	{
		mcucomm = data;
		logerror("Writes command to MCU: %02x\n", data);
	}
	else
	{
		mcudata = data;
		logerror("Writes data to MCU: %02x\n", data);
	}
}


READ16_MEMBER(mil4000_state::unk_r)
{
//  reads:  51000C-0E. touch screen?
	return 0xff;
}

WRITE16_MEMBER(mil4000_state::unk_w)
{
//  writes: 510000-02-04-06-08-0A-0C-0E
//  logerror("unknown writes from address %04x\n", offset);
}


static ADDRESS_MAP_START( mil4000_map, AS_PROGRAM, 16, mil4000_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x500000, 0x503fff) AM_RAM_WRITE(sc0_vram_w) AM_SHARE("sc0_vram")  // CY62256L-70, U77
	AM_RANGE(0x504000, 0x507fff) AM_RAM_WRITE(sc1_vram_w) AM_SHARE("sc1_vram")  // CY62256L-70, U77
	AM_RANGE(0x508000, 0x50bfff) AM_RAM_WRITE(sc2_vram_w) AM_SHARE("sc2_vram")  // CY62256L-70, U78
	AM_RANGE(0x50c000, 0x50ffff) AM_RAM_WRITE(sc3_vram_w) AM_SHARE("sc3_vram")  // CY62256L-70, U78
	AM_RANGE(0x708000, 0x708001) AM_READ_PORT("IN0")
	AM_RANGE(0x708002, 0x708003) AM_READ_PORT("IN1")
	AM_RANGE(0x708004, 0x708005) AM_READ(hvretrace_r)
	AM_RANGE(0x708006, 0x708007) AM_READ_PORT("IN2")
	AM_RANGE(0x708008, 0x708009) AM_WRITE(output_w)
	AM_RANGE(0x708010, 0x708011) AM_NOP //touch screen
	AM_RANGE(0x70801e, 0x70801f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)

	AM_RANGE(0x780000, 0x780fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("nvram") // 2x CY62256L-70 (U7 & U8).

ADDRESS_MAP_END

static ADDRESS_MAP_START( chewheel_map, AS_PROGRAM, 16, mil4000_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x500000, 0x503fff) AM_RAM_WRITE(sc0_vram_w) AM_SHARE("sc0_vram")  // V62C518256L-35P (U7).
	AM_RANGE(0x504000, 0x507fff) AM_RAM_WRITE(sc1_vram_w) AM_SHARE("sc1_vram")  // V62C518256L-35P (U7).
	AM_RANGE(0x508000, 0x50bfff) AM_RAM_WRITE(sc2_vram_w) AM_SHARE("sc2_vram")  // V62C518256L-35P (U8).
	AM_RANGE(0x50c000, 0x50ffff) AM_RAM_WRITE(sc3_vram_w) AM_SHARE("sc3_vram")  // V62C518256L-35P (U8).

	AM_RANGE(0x51000c, 0x51000f) AM_READ(unk_r)     // no idea what's mapped here.
	AM_RANGE(0x510000, 0x51000f) AM_WRITE(unk_w)    // no idea what's mapped here.

	AM_RANGE(0x708000, 0x708001) AM_READ_PORT("IN0")
	AM_RANGE(0x708002, 0x708003) AM_READ_PORT("IN1")
	AM_RANGE(0x708004, 0x708005) AM_READ(hvretrace_r)
	AM_RANGE(0x708006, 0x708007) AM_READ_PORT("IN2")
	AM_RANGE(0x708008, 0x708009) AM_WRITE(output_w)
	AM_RANGE(0x708010, 0x708011) AM_READWRITE(chewheel_mcu_r, chewheel_mcu_w)
	AM_RANGE(0x70801e, 0x70801f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)

	AM_RANGE(0x780000, 0x780fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xff0000, 0xff3fff) AM_RAM AM_SHARE("nvram")   // V62C51864L-70P (U77).
	AM_RANGE(0xffc000, 0xffffff) AM_RAM                     // V62C51864L-70P (U78).

ADDRESS_MAP_END

static INPUT_PORTS_START( mil4000 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Premio") PORT_CODE(KEYCODE_T)   //premio / prize (ticket?)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout tilelayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,5),
	5,
	{  RGN_FRAC(0,5), RGN_FRAC(1,5), RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( mil4000 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,     0, 0x800/32 )
GFXDECODE_END


static MACHINE_CONFIG_START( mil4000, mil4000_state )
	MCFG_CPU_ADD("maincpu", M68000, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(mil4000_map)
	// irq 2/4/5 point to the same place, others invalid
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mil4000_state,  irq5_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(mil4000_state, screen_update_mil4000)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_INIT_BLACK("palette", 0x800)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mil4000)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // frequency from 1000 kHz resonator. pin 7 high not verified.
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( chewheel, mil4000 )
	MCFG_CPU_REPLACE("maincpu", M68000, CPU_CLOCK) /* 2MHz */
	MCFG_CPU_PROGRAM_MAP(chewheel_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mil4000_state,  irq5_line_hold)
MACHINE_CONFIG_END


ROM_START( mil4000 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "9.u75", 0x000001, 0x20000, CRC(e3e520df) SHA1(16ee86deb75bd711c846a647e3a0a4293b5685a8) )
	ROM_LOAD16_BYTE( "10.u76", 0x000000, 0x20000, CRC(9020e19a) SHA1(e9ba0b69e8cb1fc35d024ae702d4670d78bf5cc8) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) // 5bpp?
	ROM_LOAD( "2.u36",     0x000000, 0x20000, CRC(bb4fcfde) SHA1(7e19722ce42b9ec86faac32a526429b0e56639b5) )
	ROM_LOAD( "3.u35_alt", 0x020000, 0x20000, CRC(3fd93c2f) SHA1(5217e328e51a2e00dc85a662dab6e339bd7f336f) ) // one of these is probably bad
	ROM_LOAD( "4.u34",     0x040000, 0x20000, CRC(372a67a4) SHA1(c1c1352dd3152603827224d8970e6cb04aa1e858) )
	ROM_LOAD( "5.u33",     0x060000, 0x20000, CRC(8058882e) SHA1(2de7b1e6e39d89913b2d6c1290d3cf326d2527d4) )
	ROM_LOAD( "6.u32",     0x080000, 0x20000, CRC(7217a8c2) SHA1(275c2d5a128960dd6cd56d5e3647354b17129a12) )

	ROM_REGION( 0x40000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "1.u54",   0x000000, 0x40000, CRC(e4a89163) SHA1(c0622c4e97b23daf9775137a2754bf9c47a29385) )

	ROM_REGION( 0x4d4c, "mcu", 0 ) // MCU code
	ROM_LOAD( "pic16c65a.u60.bad.dump", 0x000, 0x4d4c, BAD_DUMP CRC(c5e260ec) SHA1(d6e41de8a7db27382757ed7edfd7985090896e39) )

// palce22v10h.u74.bad.dump= palce22v10h-ch-jin-u27.u27  Jingle Bell (Italy, V133I)
ROM_END

ROM_START( mil4000a )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "27.u75", 0x000001, 0x20000, CRC(2a090f82) SHA1(c70295de25a99ec78752f2bd63e6ef0714141c84) )
	ROM_LOAD16_BYTE( "28.u76", 0x000000, 0x20000, CRC(009e1f16) SHA1(33014ccd33abf2de8e83ec964192ebb9cbda8a08) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) // 5bpp?
	ROM_LOAD( "2.u36",   0x000000, 0x20000, CRC(bb4fcfde) SHA1(7e19722ce42b9ec86faac32a526429b0e56639b5) )
	ROM_LOAD( "3.u35",   0x020000, 0x20000, CRC(21c43d81) SHA1(a266b85378723ad8e219dd63a639add64624de13) )
	ROM_LOAD( "4.u34",   0x040000, 0x20000, CRC(372a67a4) SHA1(c1c1352dd3152603827224d8970e6cb04aa1e858) )
	ROM_LOAD( "5.u33",   0x060000, 0x20000, CRC(8058882e) SHA1(2de7b1e6e39d89913b2d6c1290d3cf326d2527d4) )
	ROM_LOAD( "6.u32",   0x080000, 0x20000, CRC(7217a8c2) SHA1(275c2d5a128960dd6cd56d5e3647354b17129a12) )

	ROM_REGION( 0x40000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "1.u54",   0x000000, 0x40000, CRC(e4a89163) SHA1(c0622c4e97b23daf9775137a2754bf9c47a29385) )

	ROM_REGION( 0x4d4c, "mcu", 0 ) // MCU code
	ROM_LOAD( "pic16c65b_millennium4000.u60", 0x000, 0x4d4c, BAD_DUMP CRC(4f3f7b90) SHA1(fdf689dda57960820315dcf0138d2ade28248681) )

// palce22v10h.u74.bad.dump= palce22v10h-ch-jin-u27.u27  Jingle Bell (Italy, V133I)
ROM_END

ROM_START( mil4000b )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "27.u75", 0x000001, 0x20000, CRC(a5ca8a1e) SHA1(c42244e27031175c37e83995f548d960708eabab) ) // sldh
	ROM_LOAD16_BYTE( "28.u76", 0x000000, 0x20000, CRC(5bf4e681) SHA1(818d0ec1b2cc544334b0349ae15fd53ff32ef8c1) ) // sldh

	ROM_REGION( 0xa0000, "gfx1", 0 ) // 5bpp?
	ROM_LOAD( "2.u36",   0x000000, 0x20000, CRC(bb4fcfde) SHA1(7e19722ce42b9ec86faac32a526429b0e56639b5) )
	ROM_LOAD( "3.u35",   0x020000, 0x20000, CRC(21c43d81) SHA1(a266b85378723ad8e219dd63a639add64624de13) )
	ROM_LOAD( "4.u34",   0x040000, 0x20000, CRC(372a67a4) SHA1(c1c1352dd3152603827224d8970e6cb04aa1e858) )
	ROM_LOAD( "5.u33",   0x060000, 0x20000, CRC(8058882e) SHA1(2de7b1e6e39d89913b2d6c1290d3cf326d2527d4) )
	ROM_LOAD( "6.u32",   0x080000, 0x20000, CRC(7217a8c2) SHA1(275c2d5a128960dd6cd56d5e3647354b17129a12) )

	ROM_REGION( 0x40000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "1.u54",   0x000000, 0x40000, CRC(e4a89163) SHA1(c0622c4e97b23daf9775137a2754bf9c47a29385) )

	ROM_REGION( 0x4d4c, "mcu", 0 ) // MCU code
	ROM_LOAD( "pic16c65b_millennium4000.u60", 0x000, 0x4d4c, BAD_DUMP CRC(4f3f7b90) SHA1(fdf689dda57960820315dcf0138d2ade28248681) )
ROM_END

ROM_START( mil4000c )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "0.u75", 0x000001, 0x20000, CRC(2f84883e) SHA1(8b016eb586db59517f6a5d7e98a53c2002a6ed0a) )
	ROM_LOAD16_BYTE( "e.u76", 0x000000, 0x20000, CRC(f162018f) SHA1(43ac82d5828e57fb7ae83d88e1ed287a2e2060a3) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) // 5bpp?
	ROM_LOAD( "5.u36",     0x000000, 0x20000, CRC(bb4fcfde) SHA1(7e19722ce42b9ec86faac32a526429b0e56639b5) )
	ROM_LOAD( "4.u35",     0x020000, 0x20000, CRC(3fd93c2f) SHA1(5217e328e51a2e00dc85a662dab6e339bd7f336f) )
	ROM_LOAD( "3.u34",     0x040000, 0x20000, CRC(372a67a4) SHA1(c1c1352dd3152603827224d8970e6cb04aa1e858) )
	ROM_LOAD( "2.u33",     0x060000, 0x20000, CRC(8058882e) SHA1(2de7b1e6e39d89913b2d6c1290d3cf326d2527d4) )
	ROM_LOAD( "1.u32",     0x080000, 0x20000, CRC(7217a8c2) SHA1(275c2d5a128960dd6cd56d5e3647354b17129a12) )

	ROM_REGION( 0x40000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "red.u54",   0x000000, 0x40000, CRC(e4a89163) SHA1(c0622c4e97b23daf9775137a2754bf9c47a29385) )

	ROM_REGION( 0x4d4c, "mcu", 0 ) // MCU code
	ROM_LOAD( "pic16c74b_ch4000.u60", 0x000, 0x4d4c, NO_DUMP )
ROM_END

/*
  TOP XXI
  -------

  CPU: 1x MC68HC000FN-12 (U81)
       1x PIC16C65B-20   (U60)

  Sound: 1x U6295 (U53)

  RAM:  2x 6116.
        2x 6264 (prg RAM)
        2x 62256 (near battery)

  FPGA: 2x ACTEL A40MX04 - PL84 (U2, U3)
  PLDs: 1x PALCE22V10H-25 (U74) (protected)

  Battery: 1x 3.6 V.

  Clocks: 1x Xtal @ 12 MHz.
          1x Xtal @ 14.31818 MHz.

  Both FPGAs have stickers with the game title pacman logo.

*/
ROM_START( top21 )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "odd_1-2.u75",  0x000001, 0x80000, CRC(ce4f2a74) SHA1(f9a9043da924ddba16f49d6856dbcfd8f066c824) )
	ROM_LOAD16_BYTE( "even_1-2.u76", 0x000000, 0x80000, CRC(8d645456) SHA1(06c59816f259168f15503b276fc28c947e17cc60) )
	ROM_COPY( "maincpu",            0x080000, 0x00000, 0x80000 )    // copying the second halves to the right offset

	ROM_REGION( 0xa0000, "gfx1", 0 )
	ROM_LOAD( "36.u36",     0x000000, 0x20000, CRC(071883f7) SHA1(16b5c251975394bb94c0d32277912ea99280c21c) )
	ROM_LOAD( "35.u35",     0x020000, 0x20000, CRC(cdc8cc44) SHA1(ce703e7f050465b1bc07800eb84eb7f127ebbddb) )   // double size. 2nd half empty
	ROM_IGNORE(                       0x20000)
	ROM_LOAD( "34.u34",     0x040000, 0x20000, CRC(bdbe7360) SHA1(3038f66d57a43afea9d6c05908bfb50167a881c2) )
	ROM_LOAD( "33.u33",     0x060000, 0x20000, CRC(670584b0) SHA1(23772404b5e5066828c59d9baa03b732a80db676) )
	ROM_LOAD( "32.u32",     0x080000, 0x20000, CRC(c5bc3950) SHA1(aebaae91ade0c221ba14186fde78206996cdec30) )

	ROM_REGION( 0x80000, "oki", 0 ) // 6295 samples (first half empty)
	ROM_LOAD( "audio.u64",  0x00000, 0x80000, CRC(4f70a9bc) SHA1(83f0664eadf923ed45e3e18bfcefafb85163c4a0) )
	ROM_COPY( "oki",        0x40000, 0x00000, 0x40000 ) // copying the second half to the right offset

	ROM_REGION( 0x4000, "mcu", 0 )  // MCU code
	ROM_LOAD( "pic16c65b_top21.u60", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x10000, "nvram", 0 )   // default NVRAM (2x 62256)
	ROM_LOAD( "top21_nvram.bin", 0x00000, 0x10000, CRC(638726ce) SHA1(c55c77df5fbddfb19acf50f1b4467c63c818d5e7) )
ROM_END

/*
Cherry Wheel.
Similar hardware to TOP XXI

But...
2x V62C518256L-35P.  32K x 8 Static RAM. (U7-U8)
2x V62C51864L-70P.    8K x 8 Static RAM. (U77-U78)

Only U77 is tied to the battery.

32.u32    1ST AND 2ND HALF IDENTICAL
33.u33    1xxxxxxxxxxxxxxxxx = 0xFF
34.u34    1ST AND 2ND HALF IDENTICAL
35.u35    1xxxxxxxxxxxxxxxxx = 0xFF
36.u36    1ST AND 2ND HALF IDENTICAL

The PIC16c65b @U60 is used for protection, controlling the reels state / graphics.
Also could be used to talk with the touch screen controller via RS-232.

*/
ROM_START( chewheel )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "even.u76", 0x000000, 0x20000, CRC(7790d480) SHA1(e9d4bf16c61d57840076cf3c8bc865f92caae44c) )
	ROM_LOAD16_BYTE( "odd.u77",  0x000001, 0x20000, CRC(08f1b4b6) SHA1(7fa424b3fe899b7e8596156af6c3dbfba43984d6) )

	ROM_REGION( 0xa0000, "gfx1", 0 )
	ROM_LOAD( "36.u36",     0x000000, 0x20000, CRC(64ce0eb8) SHA1(d6533d730ecd01385c75b03884bb9b001f963ceb) )   // double size. identical halves.
	ROM_IGNORE(                       0x20000)
	ROM_LOAD( "35.u35",     0x020000, 0x20000, CRC(27f95f6f) SHA1(bb84b08ec5df60814d1d6825f4377ab1e8a63a70) )   // double size. 2nd half empty.
	ROM_IGNORE(                       0x20000)
	ROM_LOAD( "34.u34",     0x040000, 0x20000, CRC(47cac442) SHA1(e98bec034d5ab532faf86e5a11cfa1f1157491c4) )   // double size. identical halves.
	ROM_IGNORE(                       0x20000)
	ROM_LOAD( "33.u33",     0x060000, 0x20000, CRC(dd55ce6c) SHA1(4f5a2358ec96ed4afb4881fc8d6aa74e9f1d6aec) )   // double size. 2nd half empty.
	ROM_IGNORE(                       0x20000)
	ROM_LOAD( "32.u32",     0x080000, 0x20000, CRC(179e512a) SHA1(ba02563df98015349bc6fb5ac233ce3b9d6ed42e) )   // double size. identical halves.
	ROM_IGNORE(                       0x20000)

	ROM_REGION( 0x80000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "v29c51002t.u54",  0x00000, 0x40000, CRC(3c37ec4d) SHA1(11045f9b3f6fb35befdb67c111218750a4f750a7) )

	ROM_REGION( 0x4000, "mcu", 0 )  // MCU code
	ROM_LOAD( "pic16c65b_chewheel.u60", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x4000, "nvram", 0 )   // default NVRAM (1x 6264 storing the odd bytes)
	ROM_LOAD( "chewheel_nvram.bin", 0x0000, 0x4000, CRC(af73a270) SHA1(3e3e2c0a629bf506830b34d4c5a45ddbece618c3) )
ROM_END


/*     YEAR  NAME      PARENT    MACHINE   INPUT    STATE          INIT    ROT     COMPANY              FULLNAME                              FLAGS                       LAYOUT  */
GAMEL( 2000, mil4000,  0,        mil4000,  mil4000, driver_device, 0,      ROT0,  "Sure Milano",       "Millennium Nuovo 4000 (Version 2.0)", 0,                          layout_mil4000 )
GAMEL( 2000, mil4000a, mil4000,  mil4000,  mil4000, driver_device, 0,      ROT0,  "Sure Milano",       "Millennium Nuovo 4000 (Version 1.8)", 0,                          layout_mil4000 )
GAMEL( 2000, mil4000b, mil4000,  mil4000,  mil4000, driver_device, 0,      ROT0,  "Sure Milano",       "Millennium Nuovo 4000 (Version 1.5)", 0,                          layout_mil4000 )
GAMEL( 2000, mil4000c, mil4000,  mil4000,  mil4000, driver_device, 0,      ROT0,  "Sure Milano",       "Millennium Nuovo 4000 (Version 1.6)", 0,                          layout_mil4000 )
GAMEL( 200?, top21,    0,        mil4000,  mil4000, driver_device, 0,      ROT0,  "Assogiochi Assago", "Top XXI (Version 1.2)",               0,                          layout_mil4000 )
GAMEL( 200?, chewheel, 0,        chewheel, mil4000, driver_device, 0,      ROT0,  "Assogiochi Assago", "Cherry Wheel (Version 1.7)",          MACHINE_UNEMULATED_PROTECTION, layout_mil4000 )
