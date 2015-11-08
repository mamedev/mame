// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/*
Sega LaserDisc Hardware
Driver by Andrew Gardner from schematics with help from Daphne Source

Todo:
    -Get LDV1000 games past the strange, pseudo-attract mode!
    -Implement ldv1000's unknown/undocumented command 0xf2.
    -Confirm DIPs do what's expected.  Also, is Joystick_down really Joystick_up?
    -Correct IC names/positions for the ROMs - Astron Belt schematics have conflicting data.
    -Add SEGA/Hitachi VIP9500SG laserdisc player to MAME core & fix all non ldv1000 games to use it.
    -Write Sprite drawing code.
    -Hook up character map color PROM.
    -Confirm palette code (inferred from Daphne source) once its actually used.
    -Hook up OUT writes.
    -Add samples.
    -Confirm banking works.
    -Character decoding may combine the lower 0x0800 with the upper 0x0800.
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ldv1000.h"

#define SCHEMATIC_CLOCK (20000000)

class segald_state : public driver_device
{
public:
	segald_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_laserdisc(*this, "laserdisc") ,
		m_obj_ram(*this, "obj_ram"),
		m_out_ram(*this, "out_ram"),
		m_color_ram(*this, "color_ram"),
		m_fix_ram(*this, "fix_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	UINT8 m_nmi_enable;

	required_device<pioneer_ldv1000_device> m_laserdisc;
	required_shared_ptr<UINT8> m_obj_ram;
	required_shared_ptr<UINT8> m_out_ram;
	required_shared_ptr<UINT8> m_color_ram;
	required_shared_ptr<UINT8> m_fix_ram;

	UINT8 m_ldv1000_input_latch;
	UINT8 m_ldv1000_output_latch;

	DECLARE_READ8_MEMBER(astron_DISC_read);
	DECLARE_READ8_MEMBER(astron_OUT_read);
	DECLARE_READ8_MEMBER(astron_OBJ_read);
	DECLARE_READ8_MEMBER(astron_COLOR_read);
	DECLARE_WRITE8_MEMBER(astron_DISC_write);
	DECLARE_WRITE8_MEMBER(astron_OUT_write);
	DECLARE_WRITE8_MEMBER(astron_OBJ_write);
	DECLARE_WRITE8_MEMBER(astron_COLOR_write);
	DECLARE_WRITE8_MEMBER(astron_FIX_write);
	DECLARE_WRITE8_MEMBER(astron_io_bankswitch_w);
	DECLARE_DRIVER_INIT(astron);
	virtual void machine_start();
	UINT32 screen_update_astron(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void astron_draw_characters(bitmap_rgb32 &bitmap,const rectangle &cliprect);
	void astron_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

/* VIDEO GOODS */
void segald_state::astron_draw_characters(bitmap_rgb32 &bitmap,const rectangle &cliprect)
{
	UINT8 characterX, characterY;

	for (characterX = 0; characterX < 32; characterX++)
	{
		for (characterY = 0; characterY < 32; characterY++)
		{
			int current_screen_character = (characterY*32) + characterX;
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, m_fix_ram[current_screen_character],
					1, 0, 0, characterX*8, characterY*8, 0);
		}
	}
}

void segald_state::astron_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* Heisted from Daphne */
	const UINT8 SPR_Y_TOP     = 0;
/*  const UINT8 SPR_Y_BOTTOM  = 1;*/
	const UINT8 SPR_X_LO      = 2;
/*  const UINT8 SPR_X_HI      = 3;*/
/*  const UINT8 SPR_SKIP_LO   = 4;*/
/*  const UINT8 SPR_SKIP_HI   = 5;*/
/*  const UINT8 SPR_GFXOFS_LO = 6;*/
/*  const UINT8 SPR_GFXOFS_HI = 7;*/

	int sx,sy;
	int spr_number;
	int spr_base;

	for (spr_number = 0; spr_number < 32; spr_number++)
	{
		spr_base = 0x10 * spr_number;
		sy = m_obj_ram[spr_base + SPR_Y_TOP];
		sx = m_obj_ram[spr_base + SPR_X_LO];

		if (sx != 0 || sy != 0)
			logerror("Hey!  A sprite's not at 0,0 : %d %d", sx, sy);
	}
}


UINT32 segald_state::screen_update_astron(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	astron_draw_characters(bitmap, cliprect);
	astron_draw_sprites(bitmap, cliprect);

	return 0;
}



/* MEMORY HANDLERS */
/* READS */
READ8_MEMBER(segald_state::astron_DISC_read)
{
	if (m_nmi_enable)
		m_ldv1000_input_latch = m_laserdisc->status_r();

	logerror("DISC read   (0x%04x) @ 0x%04x [0x%x]\n", m_ldv1000_input_latch, offset, space.device().safe_pc());

	return m_ldv1000_input_latch;
}

READ8_MEMBER(segald_state::astron_OUT_read)
{
	logerror("OUT read   (0x%04x) @ 0x%04x [0x%x]\n", m_out_ram[offset], offset, space.device().safe_pc());
	return m_out_ram[offset];
}

READ8_MEMBER(segald_state::astron_OBJ_read)
{
	logerror("OBJ read   (0x%04x) @ 0x%04x [0x%x]\n", m_obj_ram[offset], offset, space.device().safe_pc());
	return m_obj_ram[offset];
}

READ8_MEMBER(segald_state::astron_COLOR_read)
{
	logerror("COLOR read   (0x%04x) @ 0x%04x [0x%x]\n", m_color_ram[offset], offset, space.device().safe_pc());
	return m_color_ram[offset];
}


/* WRITES */
WRITE8_MEMBER(segald_state::astron_DISC_write)
{
	logerror("DISC write : 0x%04x @  0x%04x [0x%x]\n", data, offset, space.device().safe_pc());

	m_ldv1000_output_latch = data;

	if (m_nmi_enable)
		m_laserdisc->data_w(m_ldv1000_output_latch);
}

WRITE8_MEMBER(segald_state::astron_OUT_write)
{
	logerror("OUT write : 0x%04x @  0x%04x [0x%x]\n", data, offset, space.device().safe_pc());

	switch(offset)
	{
		case 0x00:
			/* PCB CN2 -> Audio board */
			/* (plays samples) */
			break;

		case 0x01:
			/* PCB CN1 -> JAMMA */
			/* data & 0x01 = Coin Meter 1  */
			/* data & 0x02 = Coin Meter 2  */
			/* data & 0x04 = Start Lamp 1p */
			/* data & 0x08 = Start Lamp 2p */
			/* data & 0x10 = Continue Lamp */

			/* data & 0x20 = CHC           */
			m_nmi_enable = data & 0x40;      /* NMIE */
			/* data & 0x80 = CN0 Pin 19    */
			break;

		case 0x02:
			/* PCB CN2 -> Audio board */
			break;

		case 0x03:
			/* ??? */
			break;
	}

	m_out_ram[offset] = data;
}

WRITE8_MEMBER(segald_state::astron_OBJ_write)
{
	m_obj_ram[offset] = data;
	logerror("OBJ write : 0x%04x @ 0x%04x [0x%x]\n", data, offset, space.device().safe_pc());
}

WRITE8_MEMBER(segald_state::astron_COLOR_write)
{
	UINT8 r, g, b, a;
	UINT8 highBits, lowBits;
	const UINT8 palIndex = offset >> 1;

	/* Combine */
	m_color_ram[offset] = data;

	/* Easy access */
	highBits = m_color_ram[(palIndex<<1)+1] & 0x0f;
	lowBits  = m_color_ram[(palIndex<<1)];

	/* 4-bit RGB */
	r = (lowBits  & 0x0f);
	g = (lowBits  & 0xf0) >> 4;
	b = (highBits & 0x0f);
	a = (highBits & 0x80) ? 0 : 255;

	m_palette->set_pen_color(palIndex, rgb_t(a, r, g, b));
	logerror("COLOR write : 0x%04x @   0x%04x [0x%x]\n", data, offset, space.device().safe_pc());
}

WRITE8_MEMBER(segald_state::astron_FIX_write)
{
	m_fix_ram[offset] = data;
	/* logerror("FIX write : 0x%04x @ 0x%04x [0x%x]\n", data, offset, space.device().safe_pc()); */
}

WRITE8_MEMBER(segald_state::astron_io_bankswitch_w)
{
	logerror("Banking 0x%x\n", data);
	membank("bank1")->set_entry(data & 0xff);
}




/* PROGRAM MAP */
static ADDRESS_MAP_START( mainmem, AS_PROGRAM, 8, segald_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")

	AM_RANGE(0xc000, 0xc7ff) AM_READWRITE(astron_OBJ_read, astron_OBJ_write) AM_SHARE("obj_ram")    /* OBJ according to the schematics (sprite) */
	AM_RANGE(0xc800, 0xcfff) AM_READWRITE(astron_DISC_read, astron_DISC_write)                  /* DISC interface according to schematics */
	AM_RANGE(0xd000, 0xd000) AM_READ_PORT("DSWA")                               /* SW bank 2 (DIPs) */
	AM_RANGE(0xd001, 0xd001) AM_READ_PORT("DSWB")                               /* SW bank 3 (DIPs) */
	AM_RANGE(0xd002, 0xd002) AM_READ_PORT("IN0")                                /* SW bank 0 (IO) */
	AM_RANGE(0xd003, 0xd003) AM_READ_PORT("IN1")                                /* SW bank 1 (IO) */
	AM_RANGE(0xd800, 0xd803) AM_READWRITE(astron_OUT_read, astron_OUT_write) AM_SHARE("out_ram")    /* OUT according to schematics (output port) */
	AM_RANGE(0xe000, 0xe1ff) AM_READWRITE(astron_COLOR_read, astron_COLOR_write) AM_SHARE("color_ram") /* COLOR according to the schematics */
	AM_RANGE(0xf000, 0xf7ff) AM_WRITE(astron_FIX_write) AM_SHARE("fix_ram")                     /* FIX according to schematics (characters) */
	AM_RANGE(0xf800, 0xffff) AM_RAM                                                             /* RAM according to schematics */
ADDRESS_MAP_END


/* I/O MAP */
static ADDRESS_MAP_START( mainport, AS_IO, 8, segald_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITE(astron_io_bankswitch_w)
ADDRESS_MAP_END


/* PORTS */
static INPUT_PORTS_START( astron )
	PORT_START("DSWA")
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xa0, "2C/1C 4C/2C 5C/3C 6C/4C" )
	PORT_DIPSETTING(    0x20, "2C/1C 4C/3C" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, "1C/1C 2C/1C 3C/3C 4C/4C 5C/6C" )
	PORT_DIPSETTING(    0x40, "1C/1C 2C/1C 3C/3C 4C/5C" )
	PORT_DIPSETTING(    0x80, "1C/1C 2C/3C" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, "1C/2C 2C/4C 3C/6C 4C/8C 5C/11C" )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0a, "2C/1C 4C/2C 5C/3C 6C/4C" )
	PORT_DIPSETTING(    0x02, "2C/1C 4C/3C" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, "1C/1C 2C/1C 3C/3C 4C/4C 5C/6C" )
	PORT_DIPSETTING(    0x04, "1C/1C 2C/1C 3C/3C 4C/5C" )
	PORT_DIPSETTING(    0x08, "1C/1C 2C/3C" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, "1C/2C 2C/4C 3C/6C 4C/8C 5C/11C" )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Play Timer" ) PORT_DIPLOCATION("SW2:3,2")
	PORT_DIPSETTING(    0x60, "60 SEC" )
	PORT_DIPSETTING(    0x20, "50 SEC" )
	PORT_DIPSETTING(    0x40, "40 SEC" )
	PORT_DIPSETTING(    0x00, "Infinite SEC" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Reserve Ships Score" ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x0c, "5000, 10000, 20000" )
	PORT_DIPSETTING(    0x04, "5000, 20000, 40000" )
	PORT_DIPSETTING(    0x08, "10000, 20000, 30000" )
	PORT_DIPSETTING(    0x00, "10000, 20000, 40000" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )                                           /* SW0 = nonJAMMA pin 15 = coin1 & coin2 (?) */
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                                          /* SW1 = nonJAMMA pin S  = unused (maybe coin2?) */
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("TEST") PORT_CODE(KEYCODE_F1) /* SW2 = nonJAMMA pin T  = test switch */
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )                                         /* SW3 = nonJAMMA pin 16 = service switch */
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_START1 )                                          /* SW4 = nonJAMMA pin 8  = start */
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                                          /* SW5 = nonJAMMA pin J  = unused? */
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("CONTINUE")                   /* SW6 = nonJAMMA pin 18 = continue */
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                                          /* SW7 = nonJAMMA pin 19 = unused? */

	PORT_START("IN1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )                                  /* SW8  = nonJAMMA pin 9  = right */
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )                                   /* SW9  = nonJAMMA pin 10 = left */
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )                                     /* SW10 = nonJAMMA pin 11 = up */
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )                                   /* SW11 = nonJAMMA pin 12 = down */
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )                                         /* SW12 = nonJAMMA pin 13 = fire */
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                                          /* SW13 = nonJAMMA pin 14 = unused? */
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )                                          /* SW14 = nonJAMMA pin V  = unused? */
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                                          /* SW15 = nonJAMMA pin W  = unused? */
INPUT_PORTS_END

static GFXDECODE_START( segald )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1,  0, 1 )      /* CHARACTERS */
	/* SPRITES are apparently non-uniform in width - not straightforward to decode */
GFXDECODE_END


void segald_state::machine_start()
{
}


/* DRIVER */
static MACHINE_CONFIG_START( astron, segald_state )

	/* main cpu */
	MCFG_CPU_ADD("maincpu", Z80, SCHEMATIC_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(mainmem)
	MCFG_CPU_IO_MAP(mainport)
	MCFG_CPU_PERIODIC_INT_DRIVER(segald_state, nmi_line_pulse,  1000.0/59.94)


	MCFG_LASERDISC_LDV1000_ADD("laserdisc")
	MCFG_LASERDISC_OVERLAY_DRIVER(256, 256, segald_state, screen_update_astron)
	MCFG_LASERDISC_OVERLAY_PALETTE("palette")

	/* video hardware */
	MCFG_LASERDISC_SCREEN_ADD_NTSC("screen", "laserdisc")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", segald)
	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardare */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_MODIFY("laserdisc")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


ROM_START( astron )
	/* Last two ROMs are banked at 0x8000 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5473c", 0x0000, 0x4000, CRC(0f24baaa) SHA1(54fa344ad86c469f976ce469e39b2a4286da5f50) )
	ROM_LOAD( "5474a", 0x4000, 0x4000, CRC(5d44603d) SHA1(e229ff14255a5a0d1e156745664d31418b36893c) )
	ROM_LOAD( "5284",  0x8000, 0x4000, CRC(eec6db27) SHA1(f4c72d9d4137244c0a0b7a1b8f7fb0e7b032b1c4) )
	ROM_LOAD( "5285",  0xc000, 0x4000, CRC(820e154e) SHA1(574c69a7ef07c31396e9b5669a093a1db6f7cc6f) )

	/* Characters */
	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5280", 0x0000, 0x0800, CRC(583af1ff) SHA1(64259e955a04ee44d716870864f86011a709f2db) )
	ROM_LOAD( "5281", 0x0800, 0x0800, CRC(7b5c820c) SHA1(ddac82cea4d795011bd48d15bc6e2a0ee864429f) )

	/* Sprites */
	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "5286", 0x0000, 0x4000, CRC(8eb1c28e) SHA1(2867d62c9844c3ad2676f9a0ad1597baa54762fa) )
	ROM_LOAD( "5338", 0x8000, 0x4000, CRC(94ca5f9a) SHA1(4ab4ed06b70611739f45e207e9f1c7714b9a34e8) )

	/* Color lookup PROM */
	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "5279", 0x000,  0x200,  CRC(8716aeb5) SHA1(9a8bf599d025d039b12bc616850386f280b4df11) )

	/* Currently unused PROMs */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "pr-5278.bin", 0x000, 0x100, CRC(e81613da) SHA1(fc32141f7c0c8c0c1ed623636af3862a4ef0e940) )
	ROM_LOAD( "pr-5277.bin", 0x100, 0x100, CRC(bf2c33ab) SHA1(4a83b3e9b74b900621e8f42edf94cc04b791cdd0) )
	ROM_LOAD( "pr-5276.bin", 0x200, 0x20,  CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
	ROM_LOAD( "pr-5275.bin", 0x220, 0x20,  CRC(0c872a9b) SHA1(eaabce5d867a4e896bd356abc94429f9a4eec372) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "astron", 0, NO_DUMP )
ROM_END

ROM_START( astronp )
	/* Last two ROMs are banked at 0x8000 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5282", 0x0000, 0x4000, CRC(fd0dcfc9) SHA1(d797269f53f8a9e30b4d59d4f4f6e9858c133bbe) )
	ROM_LOAD( "5283", 0x4000, 0x4000, CRC(a3746393) SHA1(97864967073f0425555748535d1aa68459bacfb2) )
	ROM_LOAD( "5284", 0x8000, 0x4000, CRC(eec6db27) SHA1(f4c72d9d4137244c0a0b7a1b8f7fb0e7b032b1c4) )
	ROM_LOAD( "5285", 0xc000, 0x4000, CRC(820e154e) SHA1(574c69a7ef07c31396e9b5669a093a1db6f7cc6f) )

	/* Characters */
	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5280", 0x0000, 0x0800, CRC(583af1ff) SHA1(64259e955a04ee44d716870864f86011a709f2db) )
	ROM_LOAD( "5281", 0x0800, 0x0800, CRC(7b5c820c) SHA1(ddac82cea4d795011bd48d15bc6e2a0ee864429f) )

	/* Sprites */
	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "5286", 0x0000, 0x4000, CRC(8eb1c28e) SHA1(2867d62c9844c3ad2676f9a0ad1597baa54762fa) )
	ROM_LOAD( "5338", 0x8000, 0x4000, CRC(94ca5f9a) SHA1(4ab4ed06b70611739f45e207e9f1c7714b9a34e8) )

	/* Color lookup PROM */
	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "5279", 0x000,  0x200,  CRC(8716aeb5) SHA1(9a8bf599d025d039b12bc616850386f280b4df11) )

	/* Currently unused PROMs */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "pr-5278.bin", 0x000, 0x100, CRC(e81613da) SHA1(fc32141f7c0c8c0c1ed623636af3862a4ef0e940) )
	ROM_LOAD( "pr-5277.bin", 0x100, 0x100, CRC(bf2c33ab) SHA1(4a83b3e9b74b900621e8f42edf94cc04b791cdd0) )
	ROM_LOAD( "pr-5276.bin", 0x200, 0x20,  CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
	ROM_LOAD( "pr-5275.bin", 0x220, 0x20,  CRC(0c872a9b) SHA1(eaabce5d867a4e896bd356abc94429f9a4eec372) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "astron", 0, NO_DUMP )
ROM_END

ROM_START( galaxyr )
	/* Last two ROMs are banked at 0x8000 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gr5633.bin", 0x0000, 0x4000, CRC(398f6b23) SHA1(10f4bac2face29c2fb2422b70ba1b100acd0e968) )
	ROM_LOAD( "gr5634.bin", 0x4000, 0x4000, CRC(2c5be1b7) SHA1(03375729aa00fece8b938fd1672a700157a7f710) )
	ROM_LOAD( "gr5592.bin", 0x8000, 0x4000, CRC(d13715f8) SHA1(72e2570a1fa437faac0c52e24f801020b6e5a110) )
	ROM_LOAD( "gr5593.bin", 0xc000, 0x4000, CRC(b0a557aa) SHA1(5baacbaa25cdc0ee414bafe24d50a256631601aa) )

	/* Characters */
	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5280", 0x0000, 0x0800, CRC(583af1ff) SHA1(64259e955a04ee44d716870864f86011a709f2db) )
	ROM_LOAD( "5281", 0x0800, 0x0800, CRC(7b5c820c) SHA1(ddac82cea4d795011bd48d15bc6e2a0ee864429f) )

	/* Sprites */
	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "gr5594.bin", 0x0000, 0x4000, CRC(4403ef5a) SHA1(f8c879ea22cc48bc06608ff7bad3408378ef5162) )
	ROM_LOAD( "gr5611.bin", 0x4000, 0x4000, CRC(b16bdfe4) SHA1(565725189d0b124f8de148f343e67a177ca056f6) )
	ROM_LOAD( "gr5595.bin", 0x8000, 0x4000, CRC(ccbaec4f) SHA1(b6750543a0875e7cdc71ecb06575783b7e0ba457) )
	ROM_LOAD( "gr5612.bin", 0xc000, 0x4000, CRC(e312d080) SHA1(24eea718aa1914b87145da9990cb833b9621576c) )

	/* Color lookup PROM */
	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "5279", 0x000,  0x200,  CRC(8716aeb5) SHA1(9a8bf599d025d039b12bc616850386f280b4df11) )

	/* Currently unused PROMs */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "pr-5278.bin", 0x000, 0x100, CRC(e81613da) SHA1(fc32141f7c0c8c0c1ed623636af3862a4ef0e940) )
	ROM_LOAD( "pr-5277.bin", 0x100, 0x100, CRC(bf2c33ab) SHA1(4a83b3e9b74b900621e8f42edf94cc04b791cdd0) )
	ROM_LOAD( "pr-5276.bin", 0x200, 0x20,  CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
	ROM_LOAD( "pr-5275.bin", 0x220, 0x20,  CRC(0c872a9b) SHA1(eaabce5d867a4e896bd356abc94429f9a4eec372) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "galaxyr", 0, NO_DUMP )
ROM_END

ROM_START( galaxyrp )
	/* Last two ROMs are banked at 0x8000 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5613.bin", 0x0000, 0x4000, CRC(6617e702) SHA1(9de760d0e5a654a6abaa1a8cd7b038638d61f523) )
	ROM_LOAD( "epr-5614.bin", 0x4000, 0x4000, CRC(73ad8932) SHA1(ab0ff5e14bb58aa1f874fd135f1a8bd96de3f25e) )
	ROM_LOAD( "gr5592.bin",   0x8000, 0x4000, CRC(d13715f8) SHA1(72e2570a1fa437faac0c52e24f801020b6e5a110) )
	ROM_LOAD( "gr5593.bin",   0xc000, 0x4000, CRC(b0a557aa) SHA1(5baacbaa25cdc0ee414bafe24d50a256631601aa) )

	/* Characters */
	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5280", 0x0000, 0x0800, CRC(583af1ff) SHA1(64259e955a04ee44d716870864f86011a709f2db) )
	ROM_LOAD( "5281", 0x0800, 0x0800, CRC(7b5c820c) SHA1(ddac82cea4d795011bd48d15bc6e2a0ee864429f) )

	/* Sprites */
	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "gr5594.bin", 0x0000, 0x4000, CRC(4403ef5a) SHA1(f8c879ea22cc48bc06608ff7bad3408378ef5162) )
	ROM_LOAD( "gr5611.bin", 0x4000, 0x4000, CRC(b16bdfe4) SHA1(565725189d0b124f8de148f343e67a177ca056f6) )
	ROM_LOAD( "gr5595.bin", 0x8000, 0x4000, CRC(ccbaec4f) SHA1(b6750543a0875e7cdc71ecb06575783b7e0ba457) )
	ROM_LOAD( "gr5612.bin", 0xc000, 0x4000, CRC(e312d080) SHA1(24eea718aa1914b87145da9990cb833b9621576c) )

	/* Color lookup PROM */
	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "5279", 0x000,  0x200,  CRC(8716aeb5) SHA1(9a8bf599d025d039b12bc616850386f280b4df11) )

	/* Currently unused PROMs */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "pr-5278.bin", 0x000, 0x100, CRC(e81613da) SHA1(fc32141f7c0c8c0c1ed623636af3862a4ef0e940) )
	ROM_LOAD( "pr-5277.bin", 0x100, 0x100, CRC(bf2c33ab) SHA1(4a83b3e9b74b900621e8f42edf94cc04b791cdd0) )
	ROM_LOAD( "pr-5276.bin", 0x200, 0x20,  CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
	ROM_LOAD( "pr-5275.bin", 0x220, 0x20,  CRC(0c872a9b) SHA1(eaabce5d867a4e896bd356abc94429f9a4eec372) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "galaxyr", 0, NO_DUMP )
ROM_END

ROM_START( sblazerp )
	/* Last two ROMs are banked at 0x8000 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5590.bin", 0x0000, 0x4000, CRC(c06e41bb) SHA1(b2d47f04fe1a81040bbb07bdcb8930107d350e38) )
	ROM_LOAD( "epr-5591.bin", 0x4000, 0x4000, CRC(b179d18c) SHA1(eccad49cfbff1101677b66c19c5d79d41b11b72e) )
	ROM_LOAD( "gr5592.bin",   0x8000, 0x4000, CRC(d13715f8) SHA1(72e2570a1fa437faac0c52e24f801020b6e5a110) )
	ROM_LOAD( "gr5593.bin",   0xc000, 0x4000, CRC(b0a557aa) SHA1(5baacbaa25cdc0ee414bafe24d50a256631601aa) )

	/* Characters */
	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5280", 0x0000, 0x0800, CRC(583af1ff) SHA1(64259e955a04ee44d716870864f86011a709f2db) )
	ROM_LOAD( "5281", 0x0800, 0x0800, CRC(7b5c820c) SHA1(ddac82cea4d795011bd48d15bc6e2a0ee864429f) )

	/* Sprites */
	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "gr5594.bin", 0x0000, 0x4000, CRC(4403ef5a) SHA1(f8c879ea22cc48bc06608ff7bad3408378ef5162) )
	ROM_LOAD( "gr5611.bin", 0x4000, 0x4000, CRC(b16bdfe4) SHA1(565725189d0b124f8de148f343e67a177ca056f6) )
	ROM_LOAD( "gr5595.bin", 0x8000, 0x4000, CRC(ccbaec4f) SHA1(b6750543a0875e7cdc71ecb06575783b7e0ba457) )
	ROM_LOAD( "gr5612.bin", 0xc000, 0x4000, CRC(e312d080) SHA1(24eea718aa1914b87145da9990cb833b9621576c) )

	/* Color lookup PROM */
	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "5279", 0x000,  0x200,  CRC(8716aeb5) SHA1(9a8bf599d025d039b12bc616850386f280b4df11) )

	/* Currently unused PROMs */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "pr-5278.bin", 0x000, 0x100, CRC(e81613da) SHA1(fc32141f7c0c8c0c1ed623636af3862a4ef0e940) )
	ROM_LOAD( "pr-5277.bin", 0x100, 0x100, CRC(bf2c33ab) SHA1(4a83b3e9b74b900621e8f42edf94cc04b791cdd0) )
	ROM_LOAD( "pr-5276.bin", 0x200, 0x20,  CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
	ROM_LOAD( "pr-5275.bin", 0x220, 0x20,  CRC(0c872a9b) SHA1(eaabce5d867a4e896bd356abc94429f9a4eec372) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "sblazer", 0, NO_DUMP )
ROM_END

ROM_START( cobraseg )
	/* Banked ROMs aren't present */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic-1.bin", 0x0000, 0x4000, CRC(079783c7) SHA1(69f7821d94d62c8981a3879d8718d984c960ed25) )
	ROM_LOAD( "ic-2.bin", 0x4000, 0x2000, CRC(40c0b825) SHA1(1bac33b90b5a9d4ea528c2e69ded2009f9d69285) )

	/* Characters */
	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ic-11.bin", 0x0000, 0x0800, CRC(5a2e8f4e) SHA1(839d7cbb7dac1d5bca388ff8e9e2f4f6e1a5ce43) )
	ROM_LOAD( "ic-12.bin", 0x0800, 0x0800, CRC(4b89d7ed) SHA1(33894bc3d3f9817a018288e750d02e0f9deeeda3) )

	/* Sprites */
	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "ic-7.bin", 0x0000, 0x2000, CRC(9e12b19c) SHA1(a05accb44981288f62ff7da0c75dc972bb6d0754) )
	ROM_LOAD( "ic-8.bin", 0x8000, 0x2000, CRC(201041c0) SHA1(d7eb1da02184d2330211eb1f2114dffc39dc6ffe) )

	/* Color lookup PROM */
	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic-13.bin", 0x000,  0x200,  CRC(3547a14c) SHA1(5b8e3ddac0f6fda940b69343fdce7d5caead7a35) )

	/* Currently unused PROMs */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "pr-5278.bin", 0x000, 0x100, CRC(e81613da) SHA1(fc32141f7c0c8c0c1ed623636af3862a4ef0e940) )
	ROM_LOAD( "pr-5277.bin", 0x100, 0x100, CRC(bf2c33ab) SHA1(4a83b3e9b74b900621e8f42edf94cc04b791cdd0) )
	ROM_LOAD( "pr-5276.bin", 0x200, 0x20,  CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
	ROM_LOAD( "pr-5275.bin", 0x220, 0x20,  CRC(0c872a9b) SHA1(eaabce5d867a4e896bd356abc94429f9a4eec372) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "cobracom", 0, NO_DUMP )
ROM_END


DRIVER_INIT_MEMBER(segald_state,astron)
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &ROM[0x8000], 0x4000);
}


//    YEAR, NAME,     PARENT,  MACHINE,INPUT,  INIT,   MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1983, astron,   0,       astron, astron, segald_state, astron, ROT0,   "Sega", "Astron Belt", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
GAME( 1983, astronp,  astron,  astron, astron, segald_state, astron, ROT0,   "Sega", "Astron Belt (Pioneer LDV1000)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
GAME( 1983, cobraseg, astron,  astron, astron, segald_state, astron, ROT0,   "Sega", "Cobra Command (Sega LaserDisc Hardware)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
GAME( 1983, galaxyr,  0,       astron, astron, segald_state, astron, ROT0,   "Sega", "Galaxy Ranger", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
GAME( 1983, galaxyrp, galaxyr, astron, astron, segald_state, astron, ROT0,   "Sega", "Galaxy Ranger (Pioneer LDV1000)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
GAME( 1983, sblazerp, galaxyr, astron, astron, segald_state, astron, ROT0,   "Sega", "Star Blazer (Pioneer LDV1000)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
