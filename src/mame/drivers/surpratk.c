// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Surprise Attack (Konami GX911) (c) 1990 Konami

    Very similar to Parodius

    driver by Nicola Salmoria

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/konami.h" /* for the callback and the firq irq definition */
#include "sound/2151intf.h"
#include "includes/konamipt.h"
#include "includes/surpratk.h"

INTERRUPT_GEN_MEMBER(surpratk_state::surpratk_interrupt)
{
	if (m_k052109->is_irq_enabled())
		device.execute().set_input_line(0, HOLD_LINE);
}

WRITE8_MEMBER(surpratk_state::surpratk_videobank_w)
{
	if (data & 0xf8)
		logerror("%04x: videobank = %02x\n",space.device().safe_pc(),data);

	/* bit 0 = select 053245 at 0000-07ff */
	/* bit 1 = select palette at 0000-07ff */
	/* bit 2 = select palette bank 0 or 1 */
	if (BIT(data, 1))
		m_bank0000->set_bank(2 + BIT(data, 2));
	else
		m_bank0000->set_bank(BIT(data, 0));
}

WRITE8_MEMBER(surpratk_state::surpratk_5fc0_w)
{
	if ((data & 0xf4) != 0x10)
		logerror("%04x: 3fc0 = %02x\n",space.device().safe_pc(),data);

	/* bit 0/1 = coin counters */
	coin_counter_w(machine(), 0, data & 0x01);
	coin_counter_w(machine(), 1, data & 0x02);

	/* bit 3 = enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

	/* other bits unknown */
}


/********************************************/

static ADDRESS_MAP_START( surpratk_map, AS_PROGRAM, 8, surpratk_state )
	AM_RANGE(0x0000, 0x07ff) AM_DEVICE("bank0000", address_map_bank_device, amap8)
	AM_RANGE(0x0800, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")                    /* banked ROM */
	AM_RANGE(0x5f8c, 0x5f8c) AM_READ_PORT("P1")
	AM_RANGE(0x5f8d, 0x5f8d) AM_READ_PORT("P2")
	AM_RANGE(0x5f8e, 0x5f8e) AM_READ_PORT("DSW3")
	AM_RANGE(0x5f8f, 0x5f8f) AM_READ_PORT("DSW1")
	AM_RANGE(0x5f90, 0x5f90) AM_READ_PORT("DSW2")
	AM_RANGE(0x5fa0, 0x5faf) AM_DEVREADWRITE("k053244", k05324x_device, k053244_r, k053244_w)
	AM_RANGE(0x5fb0, 0x5fbf) AM_DEVWRITE("k053251", k053251_device, write)
	AM_RANGE(0x5fc0, 0x5fc0) AM_READ(watchdog_reset_r) AM_WRITE(surpratk_5fc0_w)
	AM_RANGE(0x5fd0, 0x5fd1) AM_DEVWRITE("ymsnd", ym2151_device, write)
	AM_RANGE(0x5fc4, 0x5fc4) AM_WRITE(surpratk_videobank_w)
	AM_RANGE(0x4000, 0x7fff) AM_DEVREADWRITE("k052109", k052109_device, read, write)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("maincpu", 0x38000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bank0000_map, AS_PROGRAM, 8, surpratk_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0fff) AM_DEVREADWRITE("k053244", k05324x_device, k053245_r, k053245_w)
	AM_RANGE(0x1000, 0x1fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
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
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( Free_Play ), SW1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x80, 0x80, "Bonus Quiz" )            PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
INPUT_PORTS_END


void surpratk_state::machine_start()
{
	membank("bank1")->configure_entries(0, 32, memregion("maincpu")->base(), 0x2000);
	membank("bank1")->set_entry(0);

	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));
}

void surpratk_state::machine_reset()
{
	m_bank0000->set_bank(0);

	for (int i = 0; i < 3; i++)
	{
		m_layerpri[i] = 0;
		m_layer_colorbase[i] = 0;
	}

	m_sprite_colorbase = 0;
}

WRITE8_MEMBER( surpratk_state::banking_callback )
{
//  logerror("%04x: setlines %02x\n", machine().device("maincpu")->safe_pc(), data);
	membank("bank1")->set_entry(data & 0x1f);
}

static MACHINE_CONFIG_START( surpratk, surpratk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI, XTAL_24MHz/2/4) /* 053248, the clock input is 12MHz, and internal CPU divider of 4 */
	MCFG_CPU_PROGRAM_MAP(surpratk_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", surpratk_state,  surpratk_interrupt)
	MCFG_KONAMICPU_LINE_CB(WRITE8(surpratk_state, banking_callback))

	MCFG_DEVICE_ADD("bank0000", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bank0000_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(13)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x800)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(surpratk_state, screen_update_surpratk)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("k052109", K052109, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K052109_CB(surpratk_state, tile_callback)

	MCFG_DEVICE_ADD("k053244", K053244, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K05324X_OFFSETS(0, 0)
	MCFG_K05324X_CB(surpratk_state, sprite_callback)

	MCFG_K053251_ADD("k053251")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("maincpu", KONAMI_FIRQ_LINE))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game ROMs

***************************************************************************/


ROM_START( suratk )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "911j01.f5", 0x00000, 0x20000, CRC(1e647881) SHA1(241e421d5599ebd9fcfb8be9c48dfd3b4c671958) )
	ROM_LOAD( "911k02.h5", 0x20000, 0x20000, CRC(ef10e7b6) SHA1(0b41a929c0c579d688653a8d90dd6b40db12cfb3) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) )
	ROM_LOAD32_WORD( "911d06.bin", 0x000002, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) )

	ROM_REGION( 0x080000, "k053244", 0 ) /* graphics */
	ROM_LOAD32_WORD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )  /* sprites */
	ROM_LOAD32_WORD( "911d04.bin", 0x000002, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )  /* sprites */
ROM_END

ROM_START( suratka )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "911j01.f5", 0x00000, 0x20000, CRC(1e647881) SHA1(241e421d5599ebd9fcfb8be9c48dfd3b4c671958) )
	ROM_LOAD( "911l02.h5", 0x20000, 0x20000, CRC(11db8288) SHA1(09fe187855172ebf0c57f561cce7f41e47f53114) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) )
	ROM_LOAD32_WORD( "911d06.bin", 0x000002, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) )

	ROM_REGION( 0x080000, "k053244", 0 ) /* graphics */
	ROM_LOAD32_WORD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )  /* sprites */
	ROM_LOAD32_WORD( "911d04.bin", 0x000002, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )  /* sprites */
ROM_END

ROM_START( suratkj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "911m01.f5", 0x00000, 0x20000, CRC(ee5b2cc8) SHA1(4b05f7ba4e804a3bccb41fe9d3258cbcfe5324aa) )
	ROM_LOAD( "911m02.h5", 0x20000, 0x20000, CRC(5d4148a8) SHA1(4fa5947db777b4c742775d588dea38758812a916) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) )
	ROM_LOAD32_WORD( "911d06.bin", 0x000002, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) )

	ROM_REGION( 0x080000, "k053244", 0 ) /* graphics */
	ROM_LOAD32_WORD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )  /* sprites */
	ROM_LOAD32_WORD( "911d04.bin", 0x000002, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )  /* sprites */
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

GAME( 1990, suratk,  0,      surpratk, surpratk, driver_device, 0, ROT0, "Konami", "Surprise Attack (World ver. K)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, suratka, suratk, surpratk, surpratk, driver_device, 0, ROT0, "Konami", "Surprise Attack (Asia ver. L)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, suratkj, suratk, surpratk, surpratk, driver_device, 0, ROT0, "Konami", "Surprise Attack (Japan ver. M)", MACHINE_SUPPORTS_SAVE )
