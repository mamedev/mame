// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Fast Lane (GX752) (c) 1987 Konami

    Driver by Manuel Abadia <emumanu+mame@gmail.com>

    TODO:
        - verify that sound is correct (volume and bank switching)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/hd6309.h"
#include "includes/konamipt.h"
#include "includes/fastlane.h"

TIMER_DEVICE_CALLBACK_MEMBER(fastlane_state::fastlane_scanline)
{
	int scanline = param;

	address_space &space = generic_space();
	if(scanline == 240 && m_k007121->ctrlram_r(space, 7) & 0x02) // vblank irq
		m_maincpu->set_input_line(HD6309_IRQ_LINE, HOLD_LINE);
	else if(((scanline % 32) == 0) && m_k007121->ctrlram_r(space, 7) & 0x01) // timer irq
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


WRITE8_MEMBER(fastlane_state::k007121_registers_w)
{
	if (offset < 8)
		m_k007121->ctrl_w(space, offset, data);
	else    /* scroll registers */
		m_k007121_regs[offset] = data;
}

WRITE8_MEMBER(fastlane_state::fastlane_bankswitch_w)
{
	/* bits 0 & 1 coin counters */
	coin_counter_w(machine(), 0,data & 0x01);
	coin_counter_w(machine(), 1,data & 0x02);

	/* bits 2 & 3 = bank number */
	membank("bank1")->set_entry((data & 0x0c) >> 2);

	/* bit 4: bank # for the 007232 (chip 2) */
	m_k007232_2->set_bank(0 + ((data & 0x10) >> 4), 2 + ((data & 0x10) >> 4));

	/* other bits seems to be unused */
}

/* Read and write handlers for one K007232 chip:
   even and odd register are mapped swapped */

READ8_MEMBER(fastlane_state::fastlane_k1_k007232_r)
{
	return m_k007232_1->read(space, offset ^ 1);
}

WRITE8_MEMBER(fastlane_state::fastlane_k1_k007232_w)
{
	m_k007232_1->write(space, offset ^ 1, data);
}

READ8_MEMBER(fastlane_state::fastlane_k2_k007232_r)
{
	return m_k007232_2->read(space, offset ^ 1);
}

WRITE8_MEMBER(fastlane_state::fastlane_k2_k007232_w)
{
	m_k007232_2->write(space, offset ^ 1, data);
}
static ADDRESS_MAP_START( fastlane_map, AS_PROGRAM, 8, fastlane_state )
	AM_RANGE(0x0000, 0x005f) AM_RAM_WRITE(k007121_registers_w) AM_SHARE("k007121_regs") /* 007121 registers */
	AM_RANGE(0x0800, 0x0800) AM_READ_PORT("DSW3")
	AM_RANGE(0x0801, 0x0801) AM_READ_PORT("P2")
	AM_RANGE(0x0802, 0x0802) AM_READ_PORT("P1")
	AM_RANGE(0x0803, 0x0803) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0900, 0x0900) AM_READ_PORT("DSW1")
	AM_RANGE(0x0901, 0x0901) AM_READ_PORT("DSW2")
	AM_RANGE(0x0b00, 0x0b00) AM_WRITE(watchdog_reset_w)                                         /* watchdog reset */
	AM_RANGE(0x0c00, 0x0c00) AM_WRITE(fastlane_bankswitch_w)                                    /* bankswitch control */
	AM_RANGE(0x0d00, 0x0d0d) AM_READWRITE(fastlane_k1_k007232_r, fastlane_k1_k007232_w) /* 007232 registers (chip 1) */
	AM_RANGE(0x0e00, 0x0e0d) AM_READWRITE(fastlane_k2_k007232_r, fastlane_k2_k007232_w) /* 007232 registers (chip 2) */
	AM_RANGE(0x0f00, 0x0f1f) AM_DEVREADWRITE("k051733", k051733_device, read, write)                                    /* 051733 (protection) */
	AM_RANGE(0x1000, 0x17ff) AM_RAM_DEVWRITE("palette", palette_device, write_indirect) AM_SHARE("palette")
	AM_RANGE(0x1800, 0x1fff) AM_RAM                                                             /* Work RAM */
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(fastlane_vram1_w) AM_SHARE("videoram1")       /* Video RAM (chip 1) */
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(fastlane_vram2_w) AM_SHARE("videoram2")       /* Video RAM (chip 2) */
	AM_RANGE(0x3000, 0x3fff) AM_RAM AM_SHARE("spriteram")                                           /* Sprite RAM */
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")                                                        /* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM                                                             /* ROM */
ADDRESS_MAP_END

/***************************************************************************

    Input Ports

***************************************************************************/

/* verified from HD6309 code */
static INPUT_PORTS_START( fastlane )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	/* The bonus life affects the starting high score too, 20000 or 30000 */
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "20k 100k 200k 400k 800k" )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, "30k 150k 300k 600k" )
	PORT_DIPSETTING(    0x08, "20k only" )
	PORT_DIPSETTING(    0x00, "30k only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8") // seems it doesn't work (same on pcb)
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )          PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, "3 Times" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_B12_UNK(1)

	PORT_START("P2")
	KONAMI8_B12_UNK(2)
INPUT_PORTS_END

static const gfx_layout gfxlayout =
{
	8,8,
	0x80000/32,
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( fastlane )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout, 0, 64*16 )
GFXDECODE_END

/***************************************************************************

    Machine Driver

***************************************************************************/

WRITE8_MEMBER(fastlane_state::volume_callback0)
{
	m_k007232_1->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232_1->set_volume(1, 0, (data & 0x0f) * 0x11);
}

WRITE8_MEMBER(fastlane_state::volume_callback1)
{
	m_k007232_2->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232_2->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void fastlane_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);
}

static MACHINE_CONFIG_START( fastlane, fastlane_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309, XTAL_24MHz/2) // 3MHz(XTAL_24MHz/8) internally
	MCFG_CPU_PROGRAM_MAP(fastlane_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", fastlane_state, fastlane_scanline, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.17) // measured
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(37*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 35*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(fastlane_state, screen_update_fastlane)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fastlane)
	MCFG_PALETTE_ADD("palette", 1024*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(0x400)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)
	MCFG_PALETTE_INIT_OWNER(fastlane_state, fastlane)

	MCFG_K007121_ADD("k007121")
	MCFG_K007121_PALETTE("palette")
	MCFG_K051733_ADD("k051733")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("k007232_1", K007232, XTAL_3_579545MHz)
	MCFG_K007232_PORT_WRITE_HANDLER(WRITE8(fastlane_state, volume_callback0))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_SOUND_ADD("k007232_2", K007232, XTAL_3_579545MHz)
	MCFG_K007232_PORT_WRITE_HANDLER(WRITE8(fastlane_state, volume_callback1))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
MACHINE_CONFIG_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( fastlane )
	ROM_REGION( 0x21000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "752_m02.9h",  0x08000, 0x08000, CRC(e1004489) SHA1(615b608d22abc3611f1620503cd6a8c9a6218db8) )  /* fixed ROM */
	ROM_LOAD( "752_e01.10h", 0x10000, 0x10000, CRC(ff4d6029) SHA1(b5c5d8654ce728300d268628bd3dd878570ba7b8) )  /* banked ROM */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "752e04.2i",   0x00000, 0x80000, CRC(a126e82d) SHA1(6663230c2c36dec563969bccad8c62e3d454d240) )  /* tiles + sprites */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "752e03.6h",   0x0000, 0x0100, CRC(44300aeb) SHA1(580c6e88cbb3b6d8156ea0b9103834f199ec2747) )

	ROM_REGION( 0x20000, "k007232_1", 0 ) /* 007232 data */
	ROM_LOAD( "752e06.4c",   0x00000, 0x20000, CRC(85d691ed) SHA1(7f8d05562a68c75672141fc80ce7e7acb80588b9) ) /* chip 1 */

	ROM_REGION( 0x80000, "k007232_2", 0 ) /* 007232 data */
	ROM_LOAD( "752e05.12b",  0x00000, 0x80000, CRC(119e9cbf) SHA1(21e3def9ab10b210632df11b6df4699140c473db) ) /* chip 2 */
ROM_END


GAME( 1987, fastlane, 0, fastlane, fastlane, driver_device, 0, ROT90, "Konami", "Fast Lane", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
