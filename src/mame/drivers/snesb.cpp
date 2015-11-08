// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina, Peter Ferrie
/***************************************************************************

 Arcade games (hacks of console games) running on SNES harware.

 Driver (based on nss.c) by Tomasz Slanina  analog[at]op.pl

    Supported games:
    - Killer Instinct
    - Final Fight 2
    - Iron (bootleg of Iron Commando)
    - Ghost Chaser Densei
    - Sonic Blast Man 2
    - Gundam Wing: Endless Duel

    Not dumped:
    - Final Fight 3

TODO:

 - all games : (re)add PORT_DIPLOCATION
 - kiinstb  : fix gfx glitches, missing texts
 - ffight2b : remove hack for starting credits (RAM - mainly 0x7eadce where credits are stored - is filled with 0x55,
   so you are awarded 55 credits on a hard reset)
 - sblast2b : dipswicthes
 - sblast2b : pressing start during gameplay changes the character used. Intentional?
 - denseib  :  fix gfx glitches, missing texts

***************************************************************************

  Killer Instinct PCB Info:
  --------------------------

    PQFP 100(?)pin chip marked "SP-BE0"
    PQFP 100(?)pin chip marked "SP-BH0"
    PQFP 100(?)pin chip marked "SP-AF0"
    Lattice pLSI 1024-60LJ B604S03
    6116 SRAM    x2
    AS7C256 SRAM x8
    jumper pack (12)
    dsw8         x2
    Xtal 24.576 MHz
    Xtal 21.47727 MHz
    volume pot
    27c801       x4
    two empty eprom sockets

    It's SNES version of KI with few mods (removed copyright messages,
    extra code for coin input, etc).

    256 bytes of RAM (mapped to reserved area) are shared with some
    device (probably Lattice PLD) used for handle coin inputs and dips

    Data lines of eproms are bitswapped.

***************************************************************************

  Final Fight 2 PCB layout:
  ------------------------

 |----------------------------------------------------------------------------|
 | |-----------|                                                              |
 | |           |        21.47727 MHz      24.576 MHz                          |
 | | Lattice   |                                                              |
 | | pLSI      |               |--------|   |--------|          HM65256       |
 | | 1024-60LJ |  |--------|   |        |   |        |                        |
 | |           |  |        |   | 86A621 |   | 86A537 |                        |
 | |-----------|  | 86A623 |   |  JDCF  |   |  JDCF  |                        |
 |                |  JDCF  |   |        |   |        |          D42832C       |
 |    ff2_1.u8    |        |   |--------|   |--------|                        |
 |                |--------|                                                  |
 |                             |--------|   |--------|                        |
 |    ff2_2.u7    |--------|   |        |   |        |          KM62256       |
 |                |        |   | 86A617 |   | 86A618 |                        |
 |                | 86A540 |   |  JDCF  |   |  JDKF  |                        |
 |    ff2_3.u6    |  JDKF  |   |        |   |        |                        |
 |                |        |   |--------|   |--------|          KM62256       |
 |                |--------|                                                  |
 |     GL324                  D41464C     D41464C                             |
 |                                                                            |
 |                            D41464C     D41464C         DSW2      DSW1      |
 |                                                                            |
 |                              7414        74245        74245     74245      |
 |                                                                            |
 |    uPC1242H       VR1       GD4021B     GD4021B      GD4021B   GD4021B     |
 |                                                                            |
 |                                                                            |
 |               |---|              JAMMA                 |---|               |
 |---------------|   |------------------------------------|   |---------------|

***************************************************************************

Iron PCB (same as Final Fight 2?)
 ______________________________________________________________________________________________
|                                                                                              |
|     _____________              XTAL1                    XTAL2                                |
|    |             |             21.47727Mhz              24.576Mhz          _______           |
|    |             |                                                        |86A619 |          |
|    |   LATTICE   |                                                        |_______|          |
|    |pLSL1024_60LJ|                                                                           |
|    |   B611S01   |                                                         _______________   |
|    |             |                       _________        _________       |               |  |
|    |             |         ______       | 86A621  |      | 86A537  |      |HM65256BLP_12  |  |
|    |             |        |      |      |  JDCF   |      |  JDCF   |      |   01002990    |  |
|    |_____________|        |86A623|      |         |      |         |      |_______________|  |
|                           | JDCF |      |_________|      |_________|       _______________   |
|  ___________________      |      |                                        |               |  |
| |4.C11              |     |      |                                        |HM65256BLP_12  |  |
| |                   |     |______|                                        |   01002990    |  |
| |AM27C020           |                                                     |_______________|  |
| |___________________|                      ______           ______                           |
|  ___________________                      |      |         |      |        _______________   |
| |5.C10              |      ______         |86A617|         |86A618|       |               |  |
| |                   |     |      |        | JDCF |         | JDCF |       | KM62256BLP_10 |  |
| |27C4001            |     |86A540|        |      |         |      |       |  210Y  KOREA  |  |
| |___________________|     | JDKF |        |      |         |      |       |_______________|  |
|  ___________________      |      |        |______|         |______|        _______________   |
| |6.C09              |     |      |                                        |               |  |
| |                   |     |______|                                        | KM62256BLP_10 |  |
| |27C4001            |                                                     |  210Y  KOREA  |  |
| |___________________|                  ________     ________              |_______________|  |
|                                       |D41464C |   |D41464C |                                |
|  _______                              |________|   |________|                                |
| | GL324 |                                                        ________    ________        |
| |_______|                              ________     ________    |  DIP1  |  |  DIP2  |       |
|                                       |D41464C |   |D41464C |   |1      8|  |1      8|       |
|                                       |________|   |________|   |________|  |________|       |
|                                                                                              |
|                                           ______   _________    _________    _________       |
|                                          |74LS14| |74LS245N |  |74LS245N |  |74LS245B |      |
|                                          |______| |_________|  |_________|  |_________|      |
|                                                                                              |
|                                           ______     ______      ______       ______         |
|                                          |GD4021|   |CD4021|    |CD4021|     |CD4021|        |
|                                          |______|   |______|    |______|     |______|        |
|                                                                                              |
|                  _____ 1                                           28 _____                  |
|                 |     || | | | | | | | | | | | | | | | | | | | | | | |     |                 |
|                 |     || | | | | | | | | | | | | | | | | | | | | | | |     |                 |
|_________________|     |______________________________________________|     |_________________|

***************************************************************************/




#include "emu.h"
#include "includes/snes.h"


class snesb_state : public snes_state
{
public:
	snesb_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag) { }

	INT8 *m_shared_ram;
	UINT8 m_cnt;
	INT8 *m_shared_ram2;
	DECLARE_READ8_MEMBER(sharedram_r);
	DECLARE_WRITE8_MEMBER(sharedram_w);
	DECLARE_READ8_MEMBER(sb2b_75bd37_r);
	DECLARE_READ8_MEMBER(sb2b_6a6xxx_r);
	DECLARE_READ8_MEMBER(sb2b_7xxx_r);
	DECLARE_READ8_MEMBER(endless_580xxx_r);
	DECLARE_READ8_MEMBER(endless_624b7f_r);
	DECLARE_READ8_MEMBER(endless_800b_r);
	DECLARE_READ8_MEMBER(sharedram2_r);
	DECLARE_WRITE8_MEMBER(sharedram2_w);
	DECLARE_READ8_MEMBER(snesb_dsw1_r);
	DECLARE_READ8_MEMBER(snesb_dsw2_r);
	DECLARE_READ8_MEMBER(snesb_coin_r);
	DECLARE_READ8_MEMBER(spc_ram_100_r);
	DECLARE_WRITE8_MEMBER(spc_ram_100_w);
	DECLARE_DRIVER_INIT(iron);
	DECLARE_DRIVER_INIT(denseib);
	DECLARE_DRIVER_INIT(kinstb);
	DECLARE_DRIVER_INIT(sblast2b);
	DECLARE_DRIVER_INIT(ffight2b);
	DECLARE_DRIVER_INIT(endless);
	DECLARE_MACHINE_RESET(ffight2b);
};


/* Killer Instinct */
READ8_MEMBER(snesb_state::sharedram_r)
{
	return m_shared_ram[offset];
}

WRITE8_MEMBER(snesb_state::sharedram_w)
{
	m_shared_ram[offset]=data;
}

/* Sonic Blast Man 2 Special Turbo */
READ8_MEMBER(snesb_state::sb2b_75bd37_r)
{
	/* protection check */
	return ++m_cnt;
}

READ8_MEMBER(snesb_state::sb2b_6a6xxx_r)
{
	/* protection checks */
	switch(offset)
	{
		case 0x26f: return 0xb1;
		case 0x3e0: return 0x9e;
		case 0x5c8: return 0xf4;
		case 0x94b: return 0x3a;
		case 0xd1a: return 0xc5;
		case 0xfb7: return 0x47;
	}

	logerror("Unknown protection read read %x @ %x\n",offset, space.device().safe_pc());

	return 0;
}

READ8_MEMBER(snesb_state::sb2b_7xxx_r)
{
	return space.read_byte(0xc07000 + offset);
}


/* Endless Duel */
READ8_MEMBER(snesb_state::endless_580xxx_r)
{
	/* protection checks */
	switch(offset)
	{
		case 0x2bc: return 0xb4;
		case 0x36a: return 0x8a;
		case 0x7c1: return 0xd9;
		case 0x956: return 0xa5;
		case 0xe83: return 0x6b;
	}

	logerror("Unknown protection read read %x @ %x\n",offset, space.device().safe_pc());

	return 0;
}

READ8_MEMBER(snesb_state::endless_624b7f_r)
{
	/* protection check */
	return ++m_cnt;
}

READ8_MEMBER(snesb_state::endless_800b_r)
{
	if (!offset)
	{
		return 0x50;
	}

	return 0xe8;
}

READ8_MEMBER(snesb_state::sharedram2_r)
{
	return m_shared_ram2[offset];
}

WRITE8_MEMBER(snesb_state::sharedram2_w)
{
	m_shared_ram2[offset]=data;
}

/* Generic read handlers for Dip Switches and coins inputs */
READ8_MEMBER(snesb_state::snesb_dsw1_r)
{
	return ioport("DSW1")->read();
}

READ8_MEMBER(snesb_state::snesb_dsw2_r)
{
	return ioport("DSW2")->read();
}

READ8_MEMBER(snesb_state::snesb_coin_r)
{
	return ioport("COIN")->read();
}


static ADDRESS_MAP_START( snesb_map, AS_PROGRAM, 8, snesb_state )
	AM_RANGE(0x000000, 0x7dffff) AM_READWRITE(snes_r_bank1, snes_w_bank1)
	AM_RANGE(0x7e0000, 0x7fffff) AM_RAM                 /* 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM */
	AM_RANGE(0x800000, 0xffffff) AM_READWRITE(snes_r_bank2, snes_w_bank2)    /* Mirror and ROM */
ADDRESS_MAP_END

READ8_MEMBER(snesb_state::spc_ram_100_r)
{
	return m_spc700->spc_ram_r(space, offset + 0x100);
}

WRITE8_MEMBER(snesb_state::spc_ram_100_w)
{
	m_spc700->spc_ram_w(space, offset + 0x100, data);
}

static ADDRESS_MAP_START( spc_mem, AS_PROGRAM, 8, snesb_state )
	AM_RANGE(0x0000, 0x00ef) AM_DEVREADWRITE("spc700", snes_sound_device, spc_ram_r, spc_ram_w) /* lower 32k ram */
	AM_RANGE(0x00f0, 0x00ff) AM_DEVREADWRITE("spc700", snes_sound_device, spc_io_r, spc_io_w)   /* spc io */
	AM_RANGE(0x0100, 0xffff) AM_READWRITE(spc_ram_100_r, spc_ram_100_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( snes_common )

	PORT_START("SERIAL1_DATA1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button R") PORT_PLAYER(1)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("P2 Select")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL1_DATA2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

#if SNES_LAYER_DEBUG
	PORT_START("DEBUG1")
	PORT_CONFNAME( 0x03, 0x00, "Select BG1 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x01, "BG1B (lower) only" )
	PORT_CONFSETTING(    0x02, "BG1A (higher) only" )
	PORT_CONFNAME( 0x0c, 0x00, "Select BG2 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x04, "BG2B (lower) only" )
	PORT_CONFSETTING(    0x08, "BG2A (higher) only" )
	PORT_CONFNAME( 0x30, 0x00, "Select BG3 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "BG3B (lower) only" )
	PORT_CONFSETTING(    0x20, "BG3A (higher) only" )
	PORT_CONFNAME( 0xc0, 0x00, "Select BG4 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x40, "BG4B (lower) only" )
	PORT_CONFSETTING(    0x80, "BG4A (higher) only" )

	PORT_START("DEBUG2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 1") PORT_CODE(KEYCODE_1_PAD) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 2") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 3") PORT_CODE(KEYCODE_3_PAD) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 4") PORT_CODE(KEYCODE_4_PAD) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Objects") PORT_CODE(KEYCODE_5_PAD) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Main/Sub") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Color Math") PORT_CODE(KEYCODE_7_PAD) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Windows") PORT_CODE(KEYCODE_8_PAD) PORT_TOGGLE

	PORT_START("DEBUG3")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mosaic") PORT_CODE(KEYCODE_9_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x70, 0x00, "Select OAM priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "OAM0 only" )
	PORT_CONFSETTING(    0x20, "OAM1 only" )
	PORT_CONFSETTING(    0x30, "OAM2 only" )
	PORT_CONFSETTING(    0x40, "OAM3 only" )
	PORT_CONFNAME( 0x80, 0x00, "Draw sprite in reverse order" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DEBUG4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 0 draw") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 1 draw") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 2 draw") PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 3 draw") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 4 draw") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 5 draw") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 6 draw") PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 7 draw") PORT_TOGGLE
#endif
INPUT_PORTS_END


/* verified from 5A22 code */
static INPUT_PORTS_START( kinstb )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "0 (Easiest)" )               /* "EASY" (0 star) */
	PORT_DIPSETTING(    0x01, "1" )                         /* (1 star) */
	PORT_DIPSETTING(    0x02, "2" )                         /* (2 stars) */
	PORT_DIPSETTING(    0x03, "3" )                         /* (3 stars) */
	PORT_DIPSETTING(    0x04, "4" )                         /* (4 stars) */
	PORT_DIPSETTING(    0x05, "5" )                         /* "HARD" (5 stars) */
	PORT_DIPSETTING(    0x06, "6" )                         /* undefined */
	PORT_DIPSETTING(    0x07, "7" )                         /* undefined */
	PORT_DIPSETTING(    0x08, "8" )                         /* undefined */
	PORT_DIPSETTING(    0x09, "9" )                         /* undefined */
	PORT_DIPSETTING(    0x0a, "10" )                        /* undefined */
	PORT_DIPSETTING(    0x0b, "11" )                        /* undefined */
	PORT_DIPSETTING(    0x0c, "12" )                        /* undefined */
	PORT_DIPSETTING(    0x0d, "13" )                        /* undefined */
	PORT_DIPSETTING(    0x0e, "14" )                        /* undefined */
	PORT_DIPSETTING(    0x0f, "15 (Hardest)" )              /* undefined */
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0f, "15 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0e, "14 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0d, "13 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, "12 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0b, "11 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0a, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x09, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

/* verified from 5A22 code */
static INPUT_PORTS_START( ffight2b )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            /* duplicate setting */
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )       /* "GAME LEVEL" */
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )          /* "EXPERT" */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "100k 300k 200k+" )
	PORT_DIPSETTING(    0x80, DEF_STR( None ) )

	PORT_START("DSW2")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

/* verified from 5A22 code */
static INPUT_PORTS_START( iron )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            /* duplicate setting */
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       /* "LEVEL" */
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )           /* "MEDIUM" */
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )             /* duplicate setting */
	PORT_DIPNAME( 0x04, 0x04, "Suffered Damages" )          /* code at 0x(8)082d0 */
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "More" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )            /* table at 0x(8)3ffda (4 * 1 word) gives 02 03 04 05 (add 1) but extra LSRA before TAY at 0x(8)3ffcf */
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "769 (Bug)" )
	PORT_DIPSETTING(    0x00, "1025 (Bug)" )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

/* verified from 5A22 code */
static INPUT_PORTS_START( denseib )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            /* duplicate setting */
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Difficulty ) )       /* "RANK" */
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )             /* duplicate setting */
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x00, "Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "Battle" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Suffered Damages" )          /* code at 0x(8)0f810 */
	PORT_DIPSETTING(    0x07, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x06, "x1.5" )
	PORT_DIPSETTING(    0x05, "x2.5" )
	PORT_DIPSETTING(    0x04, "x3.5" )
	PORT_DIPSETTING(    0x03, "x4.5" )
	PORT_DIPSETTING(    0x02, "x5.5" )
	PORT_DIPSETTING(    0x01, "x6.5" )
	PORT_DIPSETTING(    0x00, "x7.5" )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

/* verified from 5A22 code */
static INPUT_PORTS_START( sblast2b )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            /* duplicate setting */
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Difficulty ) )       /* "LEVEL" */
	PORT_DIPSETTING(    0x38, "0 (Easiest)" )               /* "NORMAL" */
	PORT_DIPSETTING(    0x30, "1" )                         /* "HARD" */
	PORT_DIPSETTING(    0x28, "2" )                         /* undefined */
	PORT_DIPSETTING(    0x20, "3" )                         /* undefined */
	PORT_DIPSETTING(    0x18, "4" )                         /* undefined */
	PORT_DIPSETTING(    0x10, "5" )                         /* undefined */
	PORT_DIPSETTING(    0x08, "6" )                         /* undefined */
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )               /* undefined */
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x04, "Power" )
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END


static INPUT_PORTS_START( endless )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            /* duplicate setting */
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Difficulty ) )       /* "LEVEL" */
	PORT_DIPSETTING(    0x38, "0 (Easiest)" )               /* "EASY" */
	PORT_DIPSETTING(    0x30, "1" )                         /* "NORMAL" */
	PORT_DIPSETTING(    0x28, "2" )                         /* "HARD" */
	PORT_DIPSETTING(    0x20, "3" )                         /* undefined */
	PORT_DIPSETTING(    0x18, "4" )                         /* undefined */
	PORT_DIPSETTING(    0x10, "5" )                         /* undefined */
	PORT_DIPSETTING(    0x08, "6" )                         /* undefined */
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )               /* undefined */
	PORT_DIPNAME( 0xc0, 0xc0, "Time" )                      /* "TIME" */
	PORT_DIPSETTING(    0xc0, "99" )                        /* "LIMIT" */
	PORT_DIPSETTING(    0x80, "60" )                        /* undefined */
	PORT_DIPSETTING(    0x40, "30" )                        /* undefined */
	PORT_DIPSETTING(    0x00, "Infinite" )                  /* "NO LIMIT" */

	PORT_START("DSW2")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END


static MACHINE_CONFIG_START( kinstb, snesb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", _5A22, 3580000*6)   /* 2.68Mhz, also 3.58Mhz */
	MCFG_CPU_PROGRAM_MAP(snesb_map)

	/* audio CPU */
	MCFG_CPU_ADD("soundcpu", SPC700, 2048000/2) /* 2.048 Mhz, but internal divider */
	MCFG_CPU_PROGRAM_MAP(spc_mem)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(DOTCLK_NTSC, SNES_HTOTAL, 0, SNES_SCR_WIDTH, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC)
	MCFG_SCREEN_UPDATE_DRIVER( snes_state, screen_update )

	MCFG_DEVICE_ADD("ppu", SNES_PPU, 0)
	MCFG_SNES_PPU_OPENBUS_CB(READ8(snes_state, snes_open_bus_r))
	MCFG_VIDEO_SET_SCREEN("screen")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("spc700", SNES, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)
MACHINE_CONFIG_END

MACHINE_RESET_MEMBER( snesb_state, ffight2b )
{
	address_space &cpu0space = m_maincpu->space(AS_PROGRAM);
	snes_state::machine_reset();

	/* Hack: avoid starting with 55 credits. It's either a work RAM init fault or MCU clears it by his own, hard to tell ... */
	cpu0space.write_byte(0x7eadce, 0x00);
}

static MACHINE_CONFIG_DERIVED( ffight2b, kinstb )
	MCFG_MACHINE_RESET_OVERRIDE( snesb_state, ffight2b )
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(snesb_state,kinstb)
{
	INT32 i;
	UINT8 *rom = memregion("user3")->base();

	for (i = 0; i < 0x400000; i++)
	{
		rom[i] = BITSWAP8(rom[i], 5, 0, 6, 1, 7, 4, 3, 2);
	}

	m_shared_ram = auto_alloc_array(machine(), INT8, 0x100);
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x781000, 0x7810ff, read8_delegate(FUNC(snesb_state::sharedram_r),this), write8_delegate(FUNC(snesb_state::sharedram_w),this));

	/* extra inputs */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770071, 0x770071, read8_delegate(FUNC(snesb_state::snesb_dsw1_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770073, 0x770073, read8_delegate(FUNC(snesb_state::snesb_dsw2_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770079, 0x770079, read8_delegate(FUNC(snesb_state::snesb_coin_r),this));

	DRIVER_INIT_CALL(snes_hirom);
}

DRIVER_INIT_MEMBER(snesb_state,ffight2b)
{
	INT32 i;
	UINT8 *rom = memregion("user3")->base();

	for(i = 0; i < 0x200000; i++)
	{
		rom[i] = rom[i] ^ 0xff;

		if (i < 0x10000) /* 0x00000 - 0x0ffff */
		{
			rom[i] = BITSWAP8(rom[i],3,1,6,4,7,0,2,5);
		}
		else if (i < 0x20000) /* 0x10000 - 0x1ffff */
		{
			rom[i] = BITSWAP8(rom[i],3,7,0,5,1,6,2,4);
		}
		else if (i < 0x30000) /* 0x20000 - 0x2ffff */
		{
			rom[i] = BITSWAP8(rom[i],1,7,6,4,5,2,3,0);
		}
		else if (i < 0x40000) /* 0x30000 - 0x3ffff */
		{
			rom[i] = BITSWAP8(rom[i],0,3,2,5,4,6,7,1);
		}
		else if (i < 0x150000)
		{
			rom[i] = BITSWAP8(rom[i],6,4,0,5,1,3,2,7);
		}
	}

	/* boot vector */
	rom[0x7ffd] = 0x89;
	rom[0x7ffc] = 0x54;

	/* extra inputs */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770071, 0x770071, read8_delegate(FUNC(snesb_state::snesb_dsw1_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770073, 0x770073, read8_delegate(FUNC(snesb_state::snesb_dsw2_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770079, 0x770079, read8_delegate(FUNC(snesb_state::snesb_coin_r),this));

	DRIVER_INIT_CALL(snes);
}

DRIVER_INIT_MEMBER(snesb_state,iron)
{
	INT32 i;
	UINT8 *rom = memregion("user3")->base();

	for (i = 0; i < 0x140000; i++)
	{
		if(i < 0x80000)
		{
			rom[i] = BITSWAP8(rom[i]^0xff,2,7,1,6,3,0,5,4);
		}
		else
		{
			rom[i] = BITSWAP8(rom[i],6,3,0,5,1,4,7,2);
		}
	}

	/* extra inputs */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770071, 0x770071, read8_delegate(FUNC(snesb_state::snesb_dsw1_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770073, 0x770073, read8_delegate(FUNC(snesb_state::snesb_dsw2_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770079, 0x770079, read8_delegate(FUNC(snesb_state::snesb_coin_r),this));

	DRIVER_INIT_CALL(snes);
}

DRIVER_INIT_MEMBER(snesb_state,denseib)
{
	UINT8 *rom = memregion("user3")->base();
	INT32 i;

	for (i = 0; i < 0x200000; i++)
	{
		rom[i] = rom[i] ^ 0xff;
		switch (i >> 16)
		{
			case 0x00: rom[i] = BITSWAP8(rom[i],1,7,0,6,3,4,5,2); break;
			case 0x01: rom[i] = BITSWAP8(rom[i],3,4,7,2,0,6,5,1); break;
			case 0x02: rom[i] = BITSWAP8(rom[i],5,4,2,1,7,0,6,3); break;
			case 0x03: rom[i] = BITSWAP8(rom[i],0,1,3,7,2,6,5,4); break;

			default:   rom[i] = BITSWAP8(rom[i],4,5,1,0,2,3,7,6); break;
		}
	}

	/* boot vector */
	rom[0xfffc] = 0x40;
	rom[0xfffd] = 0xf7;

	/* extra inputs */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770071, 0x770071, read8_delegate(FUNC(snesb_state::snesb_dsw1_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770073, 0x770073, read8_delegate(FUNC(snesb_state::snesb_dsw2_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770079, 0x770079, read8_delegate(FUNC(snesb_state::snesb_coin_r),this));

	DRIVER_INIT_CALL(snes_hirom);
}

static const UINT8 data_substitution0[] = {0x88,0x02,0x2a,0x08,0x28,0xaa,0x8a,0x0a,0xa2,0x00,0x80,0xa0,0x22,0xa8,0x82,0x20,};
static const UINT8 data_substitution1[] = {0x44,0x01,0x51,0x40,0x50,0x55,0x45,0x41,0x15,0x00,0x04,0x14,0x11,0x54,0x05,0x10,};
static const UINT8 address_substitution_low[] =
{
	0x32,0x35,0x3a,0x28,0x2a,0x0c,0x36,0x38,0x29,0x39,0x04,0x2c,0x21,0x23,0x3d,0x2d,
	0x3c,0x02,0x17,0x31,0x00,0x2e,0x0a,0x2f,0x25,0x26,0x27,0x30,0x33,0x01,0x18,0x19,
	0x10,0x11,0x24,0x16,0x1b,0x0d,0x0e,0x12,0x13,0x05,0x22,0x34,0x1c,0x06,0x07,0x37,
	0x08,0x3b,0x09,0x14,0x15,0x1d,0x0b,0x0f,0x1e,0x1f,0x2b,0x1a,0x03,0x20,0x3f,0x3e,
};

static const UINT8  address_substitution_high[] =
{
	0x1b,0x15,0x08,0x1f,0x06,0x02,0x13,0x0a,0x1d,0x04,0x0e,0x00,0x17,0x0c,0x11,0x19,
	0x16,0x0d,0x1c,0x07,0x10,0x03,0x1a,0x0b,0x12,0x05,0x0f,0x18,0x1e,0x01,0x14,0x09,
	0x2b,0x25,0x28,0x2f,0x26,0x22,0x23,0x2a,0x2d,0x24,0x2e,0x20,0x27,0x2c,0x21,0x29
};

DRIVER_INIT_MEMBER(snesb_state,sblast2b)
{
	int i, cipherText, plainText, newAddress;
	UINT8 *src = memregion("user7")->base();
	UINT8 *dst = memregion("user3")->base();

	for (i =0; i < 0x80000 * 3; i++)
	{
		cipherText = src[i];
		plainText = data_substitution0[cipherText & 0xf] | data_substitution1[cipherText >> 4];
		newAddress = (address_substitution_high[i >> 15] << 15) | (i & 0x7fc0) | (address_substitution_low[i & 0x3f]);

		if (newAddress < 0x10000)
		{
			plainText = BITSWAP8(plainText, 6,3,5,4,2,0,7,1) ^ 0xff;
		}
		else if (newAddress < 0x20000)
		{
			plainText = BITSWAP8(plainText, 4,0,7,6,3,1,2,5) ^ 0xff;
		}
		else if (newAddress < 0x30000)
		{
			plainText = BITSWAP8(plainText, 5,7,6,1,4,3,0,2);
		}
		else if (newAddress < 0x40000)
		{
			plainText = BITSWAP8(plainText, 3,1,2,0,5,6,4,7) ^ 0xff;
		}
		dst[newAddress] = plainText;
	}

	/*  boot vector */
	dst[0xfffc] = 0xc0;
	dst[0xfffd] = 0x7a;

	/*  protection checks */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x75bd37, 0x75bd37, read8_delegate(FUNC(snesb_state::sb2b_75bd37_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x6a6000, 0x6a6fff, read8_delegate(FUNC(snesb_state::sb2b_6a6xxx_r),this));

	/* handler to read boot code */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x007000, 0x007fff, read8_delegate(FUNC(snesb_state::sb2b_7xxx_r),this));

	/* extra inputs */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770071, 0x770071, read8_delegate(FUNC(snesb_state::snesb_dsw1_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770073, 0x770073, read8_delegate(FUNC(snesb_state::snesb_dsw2_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770079, 0x770079, read8_delegate(FUNC(snesb_state::snesb_coin_r),this));

	DRIVER_INIT_CALL(snes_hirom);
}

DRIVER_INIT_MEMBER(snesb_state,endless)
{
	int i;
	UINT8 *src = memregion("user7")->base();
	UINT8 *dst = memregion("user3")->base();

	static const UINT8 address_tab_high[0x40] = {
		0x3b, 0x1d, 0x35, 0x15, 0x39, 0x19, 0x34, 0x13, 0x32, 0x1f, 0x37, 0x17, 0x3d, 0x11, 0x3a, 0x1a,
		0x14, 0x3e, 0x18, 0x36, 0x1e, 0x31, 0x10, 0x3c, 0x1b, 0x3f, 0x16, 0x30, 0x12, 0x38, 0x1c, 0x33,
		0x2b, 0x0d, 0x25, 0x05, 0x29, 0x09, 0x24, 0x03, 0x22, 0x0f, 0x27, 0x07, 0x2d, 0x01, 0x2a, 0x0a,
		0x04, 0x2e, 0x08, 0x26, 0x0e, 0x21, 0x00, 0x2c, 0x0b, 0x2f, 0x06, 0x20, 0x02, 0x28, 0x0c, 0x23
	};

	static const UINT8 address_tab_low[0x40] = {
		0x14, 0x1d, 0x11, 0x3c, 0x0a, 0x29, 0x2d, 0x2e, 0x30, 0x32, 0x16, 0x36, 0x05, 0x25, 0x26, 0x37,
		0x20, 0x21, 0x27, 0x28, 0x33, 0x34, 0x23, 0x12, 0x1e, 0x1f, 0x3b, 0x24, 0x2c, 0x35, 0x38, 0x39,
		0x3d, 0x0c, 0x2a, 0x0d, 0x22, 0x18, 0x19, 0x1a, 0x03, 0x08, 0x04, 0x3a, 0x0b, 0x0f, 0x15, 0x17,
		0x1b, 0x13, 0x00, 0x1c, 0x2b, 0x01, 0x06, 0x2f, 0x07, 0x09, 0x02, 0x31, 0x10, 0x0e, 0x3f, 0x3e
	};

	static const UINT8 data_high[16] = {
		0x88, 0x38, 0x10, 0x98, 0x90, 0x00, 0x08, 0x18, 0x20, 0xb8, 0xa8, 0xa0, 0x30, 0x80, 0x28, 0xb0
	};

	static const UINT8 data_low[16] = {
		0x41, 0x46, 0x02, 0x43, 0x03, 0x00, 0x40, 0x42, 0x04, 0x47, 0x45, 0x05, 0x06, 0x01, 0x44, 0x07
	};

	for (i = 0; i < 0x200000; i++) {
		int j = (address_tab_high[i >> 15] << 15) + (i & 0x7fc0) + address_tab_low[i & 0x3f];

		dst[i] = data_high[src[j]>>4] | data_low[src[j]&0xf];

		if (i >= 0x00000 && i < 0x10000) {
			dst[i] = BITSWAP8(dst[i],2,3,4,1,7,0,6,5);
		}

		if (i >= 0x10000 && i < 0x20000) {
			dst[i] = BITSWAP8(dst[i],1,5,6,0,2,4,7,3) ^ 0xff;
		}

		if (i >= 0x20000 && i < 0x30000) {
			dst[i] = BITSWAP8(dst[i],3,0,1,6,4,5,2,7);
		}

		if (i >= 0x30000 && i < 0x40000) {
			dst[i] = BITSWAP8(dst[i],0,4,2,3,5,6,7,1) ^ 0xff;
		}
	}

	/*  boot vector */
	dst[0x7ffc] = 0x00;
	dst[0x7ffd] = 0x80;

	/* protection checks */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x580000, 0x580fff, read8_delegate(FUNC(snesb_state::endless_580xxx_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x624b7f, 0x624b7f, read8_delegate(FUNC(snesb_state::endless_624b7f_r),this));

	/* work around missing content */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x800b, 0x800c, read8_delegate(FUNC(snesb_state::endless_800b_r),this));

	m_shared_ram = auto_alloc_array(machine(), INT8, 0x22);
	m_shared_ram2 = auto_alloc_array(machine(), INT8, 0x22);
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x781000, 0x781021, read8_delegate(FUNC(snesb_state::sharedram_r),this), write8_delegate(FUNC(snesb_state::sharedram_w),this));
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x781200, 0x781221, read8_delegate(FUNC(snesb_state::sharedram2_r),this), write8_delegate(FUNC(snesb_state::sharedram2_w),this));

	/* extra inputs */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770071, 0x770071, read8_delegate(FUNC(snesb_state::snesb_dsw1_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770073, 0x770073, read8_delegate(FUNC(snesb_state::snesb_dsw2_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770079, 0x770079, read8_delegate(FUNC(snesb_state::snesb_coin_r),this));

	DRIVER_INIT_CALL(snes);
}


ROM_START( kinstb )
	ROM_REGION( 0x400000, "user3", 0 )
	ROM_LOAD( "1.u14", 0x000000, 0x100000, CRC(70889919) SHA1(1451714cbdacb7f6ced2bc7afa478ad7264cf3b7) )
	ROM_LOAD( "2.u15", 0x100000, 0x100000, CRC(e4a5d1da) SHA1(6ae566bd2f740a251d7a81b8ebb92a651cfaac8d) )
	ROM_LOAD( "3.u16", 0x200000, 0x100000, CRC(7a40f7dd) SHA1(cebe632e8d2d68d0619077cc1e931af73c9a723b) )
	ROM_LOAD( "4.u17", 0x300000, 0x100000, CRC(3d7564c1) SHA1(392b513991897668d5dd469ac84a34f785895774) )

	ROM_REGION(0x100,           "sound_ipl", 0)
	ROM_LOAD("spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )

	ROM_REGION(0x800,           "user6", ROMREGION_ERASEFF)
ROM_END

ROM_START( ffight2b )
	ROM_REGION( 0x400000, "user3", 0 )
	ROM_LOAD( "ff2_3.u6", 0x000000, 0x008000, CRC(343bf582) SHA1(cc6b7219bb2fe61f0b377b606ad28b0e5a78be0b) )
	ROM_CONTINUE(          0x088000, 0x008000 )
	ROM_CONTINUE(          0x010000, 0x008000 )
	ROM_CONTINUE(          0x098000, 0x008000 )
	ROM_CONTINUE(          0x020000, 0x008000 )
	ROM_CONTINUE(          0x0a8000, 0x008000 )
	ROM_CONTINUE(          0x030000, 0x008000 )
	ROM_CONTINUE(          0x0b8000, 0x008000 )
	ROM_CONTINUE(          0x040000, 0x008000 )
	ROM_CONTINUE(          0x0c8000, 0x008000 )
	ROM_CONTINUE(          0x050000, 0x008000 )
	ROM_CONTINUE(          0x0d8000, 0x008000 )
	ROM_CONTINUE(          0x060000, 0x008000 )
	ROM_CONTINUE(          0x0e8000, 0x008000 )
	ROM_CONTINUE(          0x070000, 0x008000 )
	ROM_CONTINUE(          0x0f8000, 0x008000 )
	ROM_LOAD( "ff2_2.u7", 0x080000, 0x008000, CRC(b2078ae5) SHA1(e7bc3ad26ed672707d0dcfcaff238aad74986532) )
	ROM_CONTINUE(          0x008000, 0x008000 )
	ROM_CONTINUE(          0x090000, 0x008000 )
	ROM_CONTINUE(          0x018000, 0x008000 )
	ROM_CONTINUE(          0x0a0000, 0x008000 )
	ROM_CONTINUE(          0x028000, 0x008000 )
	ROM_CONTINUE(          0x0b0000, 0x008000 )
	ROM_CONTINUE(          0x038000, 0x008000 )
	ROM_CONTINUE(          0x0c0000, 0x008000 )
	ROM_CONTINUE(          0x048000, 0x008000 )
	ROM_CONTINUE(          0x0d0000, 0x008000 )
	ROM_CONTINUE(          0x058000, 0x008000 )
	ROM_CONTINUE(          0x0e0000, 0x008000 )
	ROM_CONTINUE(          0x068000, 0x008000 )
	ROM_CONTINUE(          0x0f0000, 0x008000 )
	ROM_CONTINUE(          0x078000, 0x008000 )
	ROM_LOAD( "ff2_1.u8", 0x100000, 0x040000, CRC(ea315ac1) SHA1(a85de091882d35bc77dc99677511828ff7c20350) )

	ROM_REGION(0x100,           "sound_ipl", 0)
	ROM_LOAD("spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )

	ROM_REGION(0x800,           "user6", ROMREGION_ERASEFF)
ROM_END

ROM_START( iron )
	ROM_REGION( 0x400000, "user3", 0 )
	ROM_LOAD( "6.c09.bin", 0x000000, 0x080000, CRC(50ea1457) SHA1(092f9a0e34deeb090b8c88553be3b1596ded60ef) )
	ROM_LOAD( "5.c10.bin", 0x080000, 0x080000, CRC(0c3a0b5b) SHA1(1e8ab860689137e0e94731f1af2cfc561492b5bd) )
	ROM_LOAD( "4.c11.bin", 0x100000, 0x040000, CRC(2aa417c7) SHA1(24b375e5bbd4be5dcd31b63ea98fbbadd53d543e) )

	ROM_REGION(0x100,           "sound_ipl", 0)
	ROM_LOAD("spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )

	ROM_REGION(0x800,           "user6", ROMREGION_ERASEFF)
ROM_END

ROM_START( denseib )
	ROM_REGION( 0x200000, "user3", ROMREGION_ERASEFF )
	ROM_LOAD( "dj.u14", 0x000000, 0x0080000, CRC(487ded13) SHA1(624edce30fe2f2d750bcb49c609ceb511b2279b1) )
	ROM_LOAD( "dj.u15", 0x080000, 0x0080000, CRC(5932a440) SHA1(6048372268a097b08d9f56ad30f083267d798165) )
	ROM_LOAD( "dj.u16", 0x100000, 0x0080000, CRC(7cb71fd7) SHA1(7673e9dcaabe804e2d637e67eabca1683dad4245) )
	ROM_LOAD( "dj.u17", 0x180000, 0x0080000, CRC(de29dd89) SHA1(441aefbc7ee64515ee66431ef504e76dc8dc5ca3) )

	ROM_REGION(0x100,           "sound_ipl", 0)
	ROM_LOAD("spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )

	ROM_REGION(0x800,           "user6", ROMREGION_ERASEFF)
ROM_END

ROM_START( sblast2b )
	ROM_REGION( 0x180000, "user3", ROMREGION_ERASEFF )

	ROM_REGION(0x100,           "sound_ipl", 0)
	ROM_LOAD("spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )

	ROM_REGION(0x800,           "user6", ROMREGION_ERASEFF)

	ROM_REGION( 0x180000, "user7", 0 )
	ROM_LOAD( "1.bin", 0x000000, 0x0080000, CRC(bea10c40) SHA1(d9cc65267b9b57145d714f2c17b436c1fb21513f) )
	ROM_LOAD( "2.bin", 0x080000, 0x0080000, CRC(57d2b6e9) SHA1(1a7b347101f67b254e2f86294d501b0669431644) )
	ROM_LOAD( "3.bin", 0x100000, 0x0080000, CRC(9e63a5ce) SHA1(1d18606fbb28b55a921fc37e1af1aff4caae9003) )

ROM_END


ROM_START( endless )
	ROM_REGION( 0x200000, "user3", ROMREGION_ERASEFF )

	ROM_REGION(0x100,           "sound_ipl", 0)
	ROM_LOAD("spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )

	ROM_REGION(0x800,           "user6", ROMREGION_ERASEFF)

	ROM_REGION( 0x200000, "user7", 0 )
	ROM_LOAD( "endlessduel.unknownposition1", 0x000000, 0x80000, CRC(e49acd29) SHA1(ac137261fe7a7691738ac812bea9591256eb9038) )
	ROM_LOAD( "endlessduel.unknownposition2", 0x080000, 0x80000, CRC(ad2052f9) SHA1(d61382e3d93eb0bff45fb534cec0ce5ae3626165) )
	ROM_LOAD( "endlessduel.unknownposition3", 0x100000, 0x80000, CRC(30d06d7a) SHA1(17c617d94abb10c3bdf9d51013b116f4ef4debe8) )
	ROM_LOAD( "endlessduel.unknownposition4", 0x180000, 0x80000, CRC(9a9493ad) SHA1(82ee4fce9cc2014cb8404fd43eebb7941cdb9ac1) )
ROM_END



GAME( 199?, kinstb,       0,     kinstb,         kinstb, snesb_state,    kinstb,       ROT0, "bootleg",  "Killer Instinct (SNES bootleg)",                 MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, ffight2b,     0,     ffight2b,       ffight2b, snesb_state,  ffight2b,     ROT0, "bootleg",  "Final Fight 2 (SNES bootleg)",                   MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, iron,         0,     kinstb,         iron, snesb_state,      iron,         ROT0, "bootleg",  "Iron (SNES bootleg)",                            MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, denseib,      0,     kinstb,         denseib, snesb_state,   denseib,      ROT0, "bootleg",  "Ghost Chaser Densei (SNES bootleg)",             MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sblast2b,     0,     kinstb,         sblast2b, snesb_state,  sblast2b,     ROT0, "bootleg",  "Sonic Blast Man 2 Special Turbo (SNES bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS)
GAME( 1996, endless,      0,     kinstb,         endless, snesb_state,   endless,      ROT0, "bootleg",  "Gundam Wing: Endless Duel (SNES bootleg)",       MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
