/***************************************************************************

    Surprise Attack (Konami GX911) (c) 1990 Konami

    Very similar to Parodius

    driver by Nicola Salmoria

***************************************************************************/

#include "emu.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "video/konicdev.h"
#include "sound/2151intf.h"
#include "includes/konamipt.h"
#include "includes/surpratk.h"

/* prototypes */
static KONAMI_SETLINES_CALLBACK( surpratk_banking );

static INTERRUPT_GEN( surpratk_interrupt )
{
	surpratk_state *state = device->machine().driver_data<surpratk_state>();
	if (k052109_is_irq_enabled(state->m_k052109))
		device_set_input_line(device, 0, HOLD_LINE);
}

static READ8_HANDLER( bankedram_r )
{
	surpratk_state *state = space->machine().driver_data<surpratk_state>();

	if (state->m_videobank & 0x02)
	{
		if (state->m_videobank & 0x04)
			return space->machine().generic.paletteram.u8[offset + 0x0800];
		else
			return space->machine().generic.paletteram.u8[offset];
	}
	else if (state->m_videobank & 0x01)
		return k053245_r(state->m_k053244, offset);
	else
		return state->m_ram[offset];
}

static WRITE8_HANDLER( bankedram_w )
{
	surpratk_state *state = space->machine().driver_data<surpratk_state>();

	if (state->m_videobank & 0x02)
	{
		if (state->m_videobank & 0x04)
			paletteram_xBBBBBGGGGGRRRRR_be_w(space,offset + 0x0800,data);
		else
			paletteram_xBBBBBGGGGGRRRRR_be_w(space,offset,data);
	}
	else if (state->m_videobank & 0x01)
		k053245_w(state->m_k053244, offset, data);
	else
		state->m_ram[offset] = data;
}

static WRITE8_HANDLER( surpratk_videobank_w )
{
	surpratk_state *state = space->machine().driver_data<surpratk_state>();

	logerror("%04x: videobank = %02x\n",cpu_get_pc(&space->device()),data);
	/* bit 0 = select 053245 at 0000-07ff */
	/* bit 1 = select palette at 0000-07ff */
	/* bit 2 = select palette bank 0 or 1 */
	state->m_videobank = data;
}

static WRITE8_HANDLER( surpratk_5fc0_w )
{
	surpratk_state *state = space->machine().driver_data<surpratk_state>();

	if ((data & 0xf4) != 0x10)
		logerror("%04x: 3fc0 = %02x\n",cpu_get_pc(&space->device()),data);

	/* bit 0/1 = coin counters */
	coin_counter_w(space->machine(), 0, data & 0x01);
	coin_counter_w(space->machine(), 1, data & 0x02);

	/* bit 3 = enable char ROM reading through the video RAM */
	k052109_set_rmrd_line(state->m_k052109, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

	/* other bits unknown */
}


/********************************************/

static ADDRESS_MAP_START( surpratk_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_READWRITE(bankedram_r, bankedram_w) AM_BASE_MEMBER(surpratk_state, m_ram)
	AM_RANGE(0x0800, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")					/* banked ROM */
	AM_RANGE(0x5f8c, 0x5f8c) AM_READ_PORT("P1")
	AM_RANGE(0x5f8d, 0x5f8d) AM_READ_PORT("P2")
	AM_RANGE(0x5f8e, 0x5f8e) AM_READ_PORT("DSW3")
	AM_RANGE(0x5f8f, 0x5f8f) AM_READ_PORT("DSW1")
	AM_RANGE(0x5f90, 0x5f90) AM_READ_PORT("DSW2")
	AM_RANGE(0x5fa0, 0x5faf) AM_DEVREADWRITE("k053244", k053244_r, k053244_w)
	AM_RANGE(0x5fb0, 0x5fbf) AM_DEVWRITE("k053251", k053251_w)
	AM_RANGE(0x5fc0, 0x5fc0) AM_READWRITE(watchdog_reset_r, surpratk_5fc0_w)
	AM_RANGE(0x5fd0, 0x5fd1) AM_DEVWRITE("ymsnd", ym2151_w)
	AM_RANGE(0x5fc4, 0x5fc4) AM_WRITE(surpratk_videobank_w)
	AM_RANGE(0x4000, 0x7fff) AM_DEVREADWRITE("k052109", k052109_r, k052109_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM					/* ROM */
ADDRESS_MAP_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( surpratk )
	PORT_START("P1")
	KONAMI8_ALT_B12(1)

	PORT_START("P2")
	KONAMI8_ALT_B12(2)

	PORT_START("DSW1")
	KONAMI_COINAGE(DEF_STR( Free_Play ), DEF_STR( Free_Play ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Upright Controls" )
	PORT_DIPSETTING(    0x20, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static void irqhandler( device_t *device, int linestate )
{
	surpratk_state *state = device->machine().driver_data<surpratk_state>();
	device_set_input_line(state->m_maincpu, KONAMI_FIRQ_LINE, linestate);
}

static const ym2151_interface ym2151_config =
{
	irqhandler
};



static const k052109_interface surpratk_k052109_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	surpratk_tile_callback
};

static const k05324x_interface surpratk_k05324x_intf =
{
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	0, 0,
	KONAMI_ROM_DEINTERLEAVE_2,
	surpratk_sprite_callback
};

static MACHINE_START( surpratk )
{
	surpratk_state *state = machine.driver_data<surpratk_state>();
	UINT8 *ROM = machine.region("maincpu")->base();

	memory_configure_bank(machine, "bank1", 0, 28, &ROM[0x10000], 0x2000);
	memory_configure_bank(machine, "bank1", 28, 4, &ROM[0x08000], 0x2000);
	memory_set_bank(machine, "bank1", 0);

	machine.generic.paletteram.u8 = auto_alloc_array_clear(machine, UINT8, 0x1000);

	state->m_maincpu = machine.device("maincpu");
	state->m_k053244 = machine.device("k053244");
	state->m_k053251 = machine.device("k053251");
	state->m_k052109 = machine.device("k052109");

	state->save_item(NAME(state->m_videobank));
	state->save_item(NAME(state->m_sprite_colorbase));
	state->save_item(NAME(state->m_layer_colorbase));
	state->save_item(NAME(state->m_layerpri));
	state_save_register_global_pointer(machine, machine.generic.paletteram.u8, 0x1000);
}

static MACHINE_RESET( surpratk )
{
	surpratk_state *state = machine.driver_data<surpratk_state>();
	int i;

	konami_configure_set_lines(machine.device("maincpu"), surpratk_banking);

	for (i = 0; i < 3; i++)
	{
		state->m_layerpri[i] = 0;
		state->m_layer_colorbase[i] = 0;
	}

	state->m_sprite_colorbase = 0;
	state->m_videobank = 0;
}

static MACHINE_CONFIG_START( surpratk, surpratk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI, 3000000)	/* 053248 */
	MCFG_CPU_PROGRAM_MAP(surpratk_map)
	MCFG_CPU_VBLANK_INT("screen", surpratk_interrupt)

	MCFG_MACHINE_START(surpratk)
	MCFG_MACHINE_RESET(surpratk)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE(surpratk)

	MCFG_PALETTE_LENGTH(2048)

	MCFG_K052109_ADD("k052109", surpratk_k052109_intf)
	MCFG_K053244_ADD("k053244", surpratk_k05324x_intf)
	MCFG_K053251_ADD("k053251")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, 3579545)
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game ROMs

***************************************************************************/


ROM_START( suratk )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "911j01.f5", 0x10000, 0x20000, CRC(1e647881) SHA1(241e421d5599ebd9fcfb8be9c48dfd3b4c671958) )
	ROM_LOAD( "911k02.h5", 0x30000, 0x18000, CRC(ef10e7b6) SHA1(0b41a929c0c579d688653a8d90dd6b40db12cfb3) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) ) /* characters */
	ROM_LOAD( "911d06.bin", 0x040000, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) ) /* characters */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )	/* sprites */
	ROM_LOAD( "911d04.bin", 0x040000, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )	/* sprites */
ROM_END

ROM_START( suratka )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "911j01.f5", 0x10000, 0x20000, CRC(1e647881) SHA1(241e421d5599ebd9fcfb8be9c48dfd3b4c671958) )
	ROM_LOAD( "911l02.h5", 0x30000, 0x18000, CRC(11db8288) SHA1(09fe187855172ebf0c57f561cce7f41e47f53114) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) ) /* characters */
	ROM_LOAD( "911d06.bin", 0x040000, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) ) /* characters */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )	/* sprites */
	ROM_LOAD( "911d04.bin", 0x040000, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )	/* sprites */
ROM_END

ROM_START( suratkj )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "911m01.f5", 0x10000, 0x20000, CRC(ee5b2cc8) SHA1(4b05f7ba4e804a3bccb41fe9d3258cbcfe5324aa) )
	ROM_LOAD( "911m02.h5", 0x30000, 0x18000, CRC(5d4148a8) SHA1(4fa5947db777b4c742775d588dea38758812a916) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) ) /* characters */
	ROM_LOAD( "911d06.bin", 0x040000, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) ) /* characters */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )	/* sprites */
	ROM_LOAD( "911d04.bin", 0x040000, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )	/* sprites */
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

static KONAMI_SETLINES_CALLBACK( surpratk_banking )
{
	logerror("%04x: setlines %02x\n",cpu_get_pc(device), lines);
	memory_set_bank(device->machine(), "bank1", lines & 0x1f);
}


GAME( 1990, suratk,  0,      surpratk, surpratk, 0, ROT0, "Konami", "Surprise Attack (World ver. K)", GAME_SUPPORTS_SAVE )
GAME( 1990, suratka, suratk, surpratk, surpratk, 0, ROT0, "Konami", "Surprise Attack (Asia ver. L)", GAME_SUPPORTS_SAVE )
GAME( 1990, suratkj, suratk, surpratk, surpratk, 0, ROT0, "Konami", "Surprise Attack (Japan ver. M)", GAME_SUPPORTS_SAVE )
