// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*******************************************************************************

    Block Hole (GX973) (c) 1989 Konami

    driver by Nicola Salmoria

    Notes:
    Quarth works, but Block Hole crashes when it reaches the title screen. An
    interrupt happens, and after rti the ROM bank is not the same as before so
    it jumps to garbage code.
    If you want to see this happen, place a breakpoint at 0x8612, and trace
    after that.
    The code is almost identical in the two versions, it looks like Quarth is
    working just because luckily the interrupt doesn't happen at that point.
    It seems that the interrupt handler trashes the selected ROM bank and forces
    it to 0. To prevent crashes, I only generate interrupts when the ROM bank is
    already 0. There might be another interrupt enable register, but I haven't
    found it.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/konami.h" /* for the callback and the firq irq definition */
#include "sound/2151intf.h"
#include "includes/konamipt.h"
#include "includes/blockhl.h"


INTERRUPT_GEN_MEMBER(blockhl_state::blockhl_interrupt)
{
	if (m_k052109->is_irq_enabled() && m_rombank == 0)    /* kludge to prevent crashes */
		device.execute().set_input_line(KONAMI_IRQ_LINE, HOLD_LINE);
}

READ8_MEMBER(blockhl_state::bankedram_r)
{
	if (m_palette_selected)
		return m_paletteram[offset];
	else
		return m_ram[offset];
}

WRITE8_MEMBER(blockhl_state::bankedram_w)
{
	if (m_palette_selected)
		m_palette->write(space, offset, data);
	else
		m_ram[offset] = data;
}

WRITE8_MEMBER(blockhl_state::blockhl_sh_irqtrigger_w)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}


/* special handlers to combine 052109 & 051960 */
READ8_MEMBER(blockhl_state::k052109_051960_r)
{
	if (m_k052109->get_rmrd_line() == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return m_k051960->k051937_r(space, offset - 0x3800);
		else if (offset < 0x3c00)
			return m_k052109->read(space, offset);
		else
			return m_k051960->k051960_r(space, offset - 0x3c00);
	}
	else
		return m_k052109->read(space, offset);
}

WRITE8_MEMBER(blockhl_state::k052109_051960_w)
{
	if (offset >= 0x3800 && offset < 0x3808)
		m_k051960->k051937_w(space, offset - 0x3800, data);
	else if (offset < 0x3c00)
		m_k052109->write(space, offset, data);
	else
		m_k051960->k051960_w(space, offset - 0x3c00, data);
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, blockhl_state )
	AM_RANGE(0x1f84, 0x1f84) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x1f88, 0x1f88) AM_WRITE(blockhl_sh_irqtrigger_w)
	AM_RANGE(0x1f8c, 0x1f8c) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x1f94, 0x1f94) AM_READ_PORT("DSW3")
	AM_RANGE(0x1f95, 0x1f95) AM_READ_PORT("P1")
	AM_RANGE(0x1f96, 0x1f96) AM_READ_PORT("P2")
	AM_RANGE(0x1f97, 0x1f97) AM_READ_PORT("DSW1")
	AM_RANGE(0x1f98, 0x1f98) AM_READ_PORT("DSW2")
	AM_RANGE(0x0000, 0x3fff) AM_READWRITE(k052109_051960_r, k052109_051960_w)
	AM_RANGE(0x4000, 0x57ff) AM_RAM
	AM_RANGE(0x5800, 0x5fff) AM_READWRITE(bankedram_r, bankedram_w) AM_SHARE("ram")
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, blockhl_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0xe00c, 0xe00d) AM_WRITENOP        /* leftover from missing 007232? */
ADDRESS_MAP_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( blockhl )
	PORT_START("P1")
	KONAMI8_B123_START(1)

	PORT_START("P2")
	KONAMI8_B123_START(2)

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:2" )        /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(0x40, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:4" )        /* Listed as "Unused" */
INPUT_PORTS_END


/***************************************************************************

    Machine Driver

***************************************************************************/

void blockhl_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x2000);

	m_paletteram.resize(m_palette->entries() * 2);
	m_palette->basemem().set(m_paletteram, ENDIANNESS_BIG, 2);

	save_item(NAME(m_paletteram));
	save_item(NAME(m_palette_selected));
	save_item(NAME(m_rombank));
}

void blockhl_state::machine_reset()
{
	m_palette_selected = 0;
	m_rombank = 0;
}

WRITE8_MEMBER( blockhl_state::banking_callback )
{
	/* bits 0-1 = ROM bank */
	m_rombank = data & 0x03;
	membank("bank1")->set_entry(m_rombank);

	/* bits 3/4 = coin counters */
	coin_counter_w(machine(), 0, data & 0x08);
	coin_counter_w(machine(), 1, data & 0x10);

	/* bit 5 = select palette RAM or work RAM at 5800-5fff */
	m_palette_selected = ~data & 0x20;

	/* bit 6 = enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 7 used but unknown */

	/* other bits unknown */

	if ((data & 0x84) != 0x80)
		logerror("%04x: setlines %02x\n", machine().device("maincpu")->safe_pc(), data);
}

static MACHINE_CONFIG_START( blockhl, blockhl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI,3000000)     /* Konami custom 052526 */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blockhl_state,  blockhl_interrupt)
	MCFG_KONAMICPU_LINE_CB(WRITE8(blockhl_state, banking_callback))

	MCFG_CPU_ADD("audiocpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(audio_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(blockhl_state, screen_update_blockhl)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("k052109", K052109, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K052109_CB(blockhl_state, tile_callback)

	MCFG_DEVICE_ADD("k051960", K051960, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051960_CB(blockhl_state, sprite_callback)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 3579545)
	MCFG_SOUND_ROUTE(0, "mono", 0.60)
	MCFG_SOUND_ROUTE(1, "mono", 0.60)
MACHINE_CONFIG_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( blockhl )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* code + banked roms + space for banked RAM */
	ROM_LOAD( "973l02.e21", 0x10000, 0x08000, CRC(e14f849a) SHA1(d44cf178cc98998b72ed32c6e20b6ebdf1f97579) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "973d01.g6",  0x0000, 0x8000, CRC(eeee9d92) SHA1(6c6c324b1f6f4fba0aa12e0d1fc5dbab133ef669) )

	ROM_REGION( 0x20000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "973f07.k15", 0x00000, 0x08000, CRC(1a8cd9b4) SHA1(7cb7944d24ac51fa6b610542d9dec68697cacf0f) )
	ROM_LOAD32_BYTE( "973f08.k18", 0x00001, 0x08000, CRC(952b51a6) SHA1(017575738d444b688b137cad5611638d53be84f2) )
	ROM_LOAD32_BYTE( "973f09.k20", 0x00002, 0x08000, CRC(77841594) SHA1(e1bfdc5bb598d865868d578ef7faba8078becd7a) )
	ROM_LOAD32_BYTE( "973f10.k23", 0x00003, 0x08000, CRC(09039fab) SHA1(a9dea17aacf4484d21ef3b16470263447b51b6b5) )

	ROM_REGION( 0x20000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_BYTE( "973f06.k12", 0x00000, 0x08000, CRC(51acfdb6) SHA1(94d243f341b490684f5297d95d4835bd522ece35) )
	ROM_LOAD32_BYTE( "973f05.k9",  0x00001, 0x08000, CRC(4cfea298) SHA1(4772b5b99f5fd8174d8884bd84173512e1edabf4) )
	ROM_LOAD32_BYTE( "973f04.k7",  0x00002, 0x08000, CRC(69ca41bd) SHA1(9b0b1c888efd2f2d5525f14778e18fb4a7353eb6) )
	ROM_LOAD32_BYTE( "973f03.k4",  0x00003, 0x08000, CRC(21e98472) SHA1(8c697d369a1f57be0825c33b4e9107ce1b02a130) )

	ROM_REGION( 0x0100, "proms", 0 )    /* PROMs */
	ROM_LOAD( "973a11.h10", 0x0000, 0x0100, CRC(46d28fe9) SHA1(9d0811a928c8907785ef483bfbee5445506b3ec8) )  /* priority encoder (not used) */
ROM_END

ROM_START( quarth )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* code + banked roms + space for banked RAM */
	ROM_LOAD( "973j02.e21", 0x10000, 0x08000, CRC(27a90118) SHA1(51309385b93db29b9277d14252166c4ea1746303) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "973d01.g6",  0x0000, 0x8000, CRC(eeee9d92) SHA1(6c6c324b1f6f4fba0aa12e0d1fc5dbab133ef669) )

	ROM_REGION( 0x20000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "973e07.k15", 0x00000, 0x08000, CRC(0bd6b0f8) SHA1(6c59cf637354fe2df424eaa89feb9c1bc1f66a92) )
	ROM_LOAD32_BYTE( "973e08.k18", 0x00001, 0x08000, CRC(104d0d5f) SHA1(595698911513113d01e5b565f5b073d1bd033d3f) )
	ROM_LOAD32_BYTE( "973e09.k20", 0x00002, 0x08000, CRC(bd3a6f24) SHA1(eb45db3a6a52bb2b25df8c2dace877e59b4130a6) )
	ROM_LOAD32_BYTE( "973e10.k23", 0x00003, 0x08000, CRC(cf5e4b86) SHA1(43348753894c1763b26dbfc70245dac92048db8f) )

	ROM_REGION( 0x20000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_BYTE( "973e06.k12", 0x00000, 0x08000, CRC(0d58af85) SHA1(2efd661d614fb305a14cfe1aa4fb17714f215d4f) )
	ROM_LOAD32_BYTE( "973e05.k9",  0x00001, 0x08000, CRC(15d822cb) SHA1(70ecad5e0a461df0da6e6eb23f43a7b643297f0d) )
	ROM_LOAD32_BYTE( "973e04.k7",  0x00002, 0x08000, CRC(d70f4a2c) SHA1(25f835a17bacf2b8debb2eb8a3cff90cab3f402a) )
	ROM_LOAD32_BYTE( "973e03.k4",  0x00003, 0x08000, CRC(2c5a4b4b) SHA1(e2991dd78b9cd96cf93ebd6de0d4e060d346ab9c) )

	ROM_REGION( 0x0100, "proms", 0 )    /* PROMs */
	ROM_LOAD( "973a11.h10", 0x0000, 0x0100, CRC(46d28fe9) SHA1(9d0811a928c8907785ef483bfbee5445506b3ec8) )  /* priority encoder (not used) */
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

GAME( 1989, blockhl, 0,       blockhl, blockhl, driver_device, 0, ROT0, "Konami", "Block Hole", GAME_SUPPORTS_SAVE )
GAME( 1989, quarth,  blockhl, blockhl, blockhl, driver_device, 0, ROT0, "Konami", "Quarth (Japan)", GAME_SUPPORTS_SAVE )
