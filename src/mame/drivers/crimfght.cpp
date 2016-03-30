// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Crime Fighters (Konami GX821) (c) 1989 Konami

    Preliminary driver by:
        Manuel Abadia <emumanu+mame@gmail.com>


    2008-08
    Dip locations verified with manual (US)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/konami.h" /* for the callback and the firq irq definition */
#include "sound/2151intf.h"
#include "includes/konamipt.h"
#include "includes/crimfght.h"


WRITE8_MEMBER(crimfght_state::crimfght_coin_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

READ8_MEMBER(crimfght_state::k052109_051960_r)
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

WRITE8_MEMBER(crimfght_state::k052109_051960_w)
{
	if (offset >= 0x3800 && offset < 0x3808)
		m_k051960->k051937_w(space, offset - 0x3800, data);
	else if (offset < 0x3c00)
		m_k052109->write(space, offset, data);
	else
		m_k051960->k051960_w(space, offset - 0x3c00, data);
}

WRITE8_MEMBER(crimfght_state::sound_w)
{
	// writing the latch asserts the irq line
	soundlatch_write(0, data);
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER( crimfght_state::audiocpu_irq_ack )
{
	// irq ack cycle clears irq via flip-flop u86
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return 0xff;
}

WRITE8_MEMBER(crimfght_state::ym2151_ct_w)
{
	// ne output from the 007232 is connected to a ls399 which
	// has inputs connected to the ct1 and ct2 outputs from
	// the ym2151 used to select the bank

	int bank_a = BIT(data, 1);
	int bank_b = BIT(data, 0);

	m_k007232->set_bank(bank_a, bank_b);
}

static ADDRESS_MAP_START( crimfght_map, AS_PROGRAM, 8, crimfght_state )
	AM_RANGE(0x0000, 0x03ff) AM_DEVICE("bank0000", address_map_bank_device, amap8)
	AM_RANGE(0x0400, 0x1fff) AM_RAM
	AM_RANGE(0x3f80, 0x3f80) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3f81, 0x3f81) AM_READ_PORT("P1")
	AM_RANGE(0x3f82, 0x3f82) AM_READ_PORT("P2")
	AM_RANGE(0x3f83, 0x3f83) AM_READ_PORT("DSW2")
	AM_RANGE(0x3f84, 0x3f84) AM_READ_PORT("DSW3")
	AM_RANGE(0x3f85, 0x3f85) AM_READ_PORT("P3")
	AM_RANGE(0x3f86, 0x3f86) AM_READ_PORT("P4")
	AM_RANGE(0x3f87, 0x3f87) AM_READ_PORT("DSW1")
	AM_RANGE(0x3f88, 0x3f88) AM_MIRROR(0x03) AM_READ(watchdog_reset_r) AM_WRITE(crimfght_coin_w) // 051550
	AM_RANGE(0x3f8c, 0x3f8c) AM_MIRROR(0x03) AM_WRITE(sound_w)
	AM_RANGE(0x2000, 0x5fff) AM_READWRITE(k052109_051960_r, k052109_051960_w)   /* video RAM + sprite RAM */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("rombank")                        /* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("maincpu", 0x18000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bank0000_map, AS_PROGRAM, 8, crimfght_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x0400, 0x07ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
ADDRESS_MAP_END

// full memory map derived from schematics
static ADDRESS_MAP_START( crimfght_sound_map, AS_PROGRAM, 8, crimfght_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0xa000, 0xa001) AM_MIRROR(0x1ffe) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x1fff) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe000, 0xe00f) AM_MIRROR(0x1ff0) AM_DEVREADWRITE("k007232", k007232_device, read, write)
ADDRESS_MAP_END

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( crimfght )
	PORT_START("DSW1")
	PORT_DIPNAME(0x0f, 0x0f, DEF_STR( Coin_A )) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(   0x02, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(   0x05, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 3C_2C ))
	PORT_DIPSETTING(   0x01, DEF_STR( 4C_3C ))
	PORT_DIPSETTING(   0x0f, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x03, DEF_STR( 3C_4C ))
	PORT_DIPSETTING(   0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(   0x0e, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x06, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(   0x0d, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(   0x0b, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(   0x0a, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(   0x09, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(   0x00, DEF_STR( Free_Play ))
	PORT_DIPNAME(0xf0, 0xf0, "Coin B") PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(   0x20, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(   0x50, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x80, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x40, DEF_STR( 3C_2C ))
	PORT_DIPSETTING(   0x10, DEF_STR( 4C_3C ))
	PORT_DIPSETTING(   0xf0, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x30, DEF_STR( 3C_4C ))
	PORT_DIPSETTING(   0x70, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(   0xe0, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x60, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(   0xd0, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0xc0, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(   0xb0, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(   0xa0, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(   0x90, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(   0x00, DEF_STR( Unused ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
		PORT_DIPSETTING(    0x03, "1" )
		PORT_DIPSETTING(    0x02, "2" )
		PORT_DIPSETTING(    0x01, "3" )
		PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNUSED_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPNAME(0x60, 0x40, DEF_STR( Difficulty ))  PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(   0x60, DEF_STR( Easy ))
	PORT_DIPSETTING(   0x40, DEF_STR( Normal ))
	PORT_DIPSETTING(   0x20, DEF_STR( Difficult ))
	PORT_DIPSETTING(   0x00, DEF_STR( Very_Difficult ))
	PORT_DIPNAME(0x80, 0x00, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Flip_Screen )) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPUNUSED_DIPLOC(0x02, IP_ACTIVE_LOW, "SW3:2")
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW3:3")
	PORT_DIPUNUSED_DIPLOC(0x08, IP_ACTIVE_LOW, "SW3:4")
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, crimfght_state, system_r, NULL)

	PORT_START("P1")
	KONAMI8_B123_START(1)

	PORT_START("P2")
	KONAMI8_B123_START(2)

	PORT_START("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_SERVICE2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

static INPUT_PORTS_START( crimfghtu )
	PORT_INCLUDE( crimfght )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME(0xf0, 0x00, "Coin B (Unused)") PORT_DIPLOCATION("SW1:5,6,7,8")
		PORT_DIPSETTING(   0x20, DEF_STR( 4C_1C ))
		PORT_DIPSETTING(   0x50, DEF_STR( 3C_1C ))
		PORT_DIPSETTING(   0x80, DEF_STR( 2C_1C ))
		PORT_DIPSETTING(   0x40, DEF_STR( 3C_2C ))
		PORT_DIPSETTING(   0x10, DEF_STR( 4C_3C ))
		PORT_DIPSETTING(   0xf0, DEF_STR( 1C_1C ))
		PORT_DIPSETTING(   0x30, DEF_STR( 3C_4C ))
		PORT_DIPSETTING(   0x70, DEF_STR( 2C_3C ))
		PORT_DIPSETTING(   0xe0, DEF_STR( 1C_2C ))
		PORT_DIPSETTING(   0x60, DEF_STR( 2C_5C ))
		PORT_DIPSETTING(   0xd0, DEF_STR( 1C_3C ))
		PORT_DIPSETTING(   0xc0, DEF_STR( 1C_4C ))
		PORT_DIPSETTING(   0xb0, DEF_STR( 1C_5C ))
		PORT_DIPSETTING(   0xa0, DEF_STR( 1C_6C ))
		PORT_DIPSETTING(   0x90, DEF_STR( 1C_7C ))
		PORT_DIPSETTING(   0x00, DEF_STR( Unused ))

	PORT_MODIFY("DSW2")
		PORT_DIPUNUSED_DIPLOC(0x01, 0x01, "SW2:1")
		PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "SW2:2")

		PORT_MODIFY("P1")
		KONAMI8_B12_UNK(1)

		PORT_MODIFY("P2")
		KONAMI8_B12_UNK(2)

		PORT_MODIFY("P3")
		KONAMI8_B12_UNK(3)

		PORT_MODIFY("P4")
		KONAMI8_B12_UNK(4)

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )
INPUT_PORTS_END


/***************************************************************************

    Machine Driver

***************************************************************************/

WRITE8_MEMBER(crimfght_state::volume_callback)
{
	m_k007232->set_volume(0, (data & 0x0f) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data >> 4) * 0x11);
}

void crimfght_state::machine_start()
{
	m_rombank->configure_entries(0, 16, memregion("maincpu")->base(), 0x2000);
	m_rombank->set_entry(0);
}

WRITE8_MEMBER( crimfght_state::banking_callback )
{
	m_rombank->set_entry(data & 0x0f);

	/* bit 5 = select work RAM or palette */
	m_woco = BIT(data, 5);
	m_bank0000->set_bank(m_woco);

	/* bit 6 = enable char ROM reading through the video RAM */
	m_rmrd = BIT(data, 6);
	m_k052109->set_rmrd_line(m_rmrd ? ASSERT_LINE : CLEAR_LINE);

	m_init = BIT(data, 7);
}

CUSTOM_INPUT_MEMBER( crimfght_state::system_r )
{
	UINT8 data = 0;

	data |= 1 << 4; // VCC
	data |= m_woco << 5;
	data |= m_rmrd << 6;
	data |= m_init << 7;

	return data >> 4;
}

static MACHINE_CONFIG_START( crimfght, crimfght_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI, XTAL_24MHz/8)       /* 052001 (verified on pcb) */
	MCFG_CPU_PROGRAM_MAP(crimfght_map)
	MCFG_KONAMICPU_LINE_CB(WRITE8(crimfght_state, banking_callback))

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz)     /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(crimfght_sound_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(DEVICE_SELF, crimfght_state, audiocpu_irq_ack)

	MCFG_DEVICE_ADD("bank0000", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bank0000_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(11)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x400)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_24MHz/3, 528, 96, 416, 256, 16, 240) // measured 59.17
//  6MHz dotclock is more realistic, however needs drawing updates. replace when ready
//  MCFG_SCREEN_RAW_PARAMS(XTAL_24MHz/4, 396, hbend, hbstart, 256, 16, 240)
	MCFG_SCREEN_UPDATE_DRIVER(crimfght_state, screen_update_crimfght)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("k052109", K052109, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K052109_CB(crimfght_state, tile_callback)

	MCFG_DEVICE_ADD("k051960", K051960, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051960_SCREEN_TAG("screen")
	MCFG_K051960_CB(crimfght_state, sprite_callback)
	MCFG_K051960_IRQ_HANDLER(INPUTLINE("maincpu", KONAMI_IRQ_LINE))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz)  /* verified on pcb */
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(crimfght_state, ym2151_ct_w))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("k007232", K007232, XTAL_3_579545MHz)    /* verified on pcb */
	MCFG_K007232_PORT_WRITE_HANDLER(WRITE8(crimfght_state, volume_callback))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.20)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.20)
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.20)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.20)
MACHINE_CONFIG_END

/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( crimfght )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "821r02.f24", 0x00000, 0x20000, CRC(4ecdd923) SHA1(78e5260c4bb9b18d7818fb6300d7e1d3a577fb63) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "821l01.h4",  0x0000, 0x8000, CRC(0faca89e) SHA1(21c9c6d736b398a29e8709e1187c5bf3cacdc99d) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "821k06.k13", 0x000000, 0x040000, CRC(a1eadb24) SHA1(ca305b904b34e03918ad07281fda86ad63caa44f) )
	ROM_LOAD32_WORD( "821k07.k19", 0x000002, 0x040000, CRC(060019fa) SHA1(c3bca007aaa5f1c534d2a75fe4f96d01a740dd58) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "821k04.k2",  0x000000, 0x080000, CRC(00e0291b) SHA1(39d5db6cf36826e47cdf5308eff9bfa8afc82050) )
	ROM_LOAD32_WORD( "821k05.k8",  0x000002, 0x080000, CRC(e09ea05d) SHA1(50ac9a2117ce63fe774c48d769ec445a83f1269e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.i15", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )  /* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232", 0 ) /* data for the 007232 */
	ROM_LOAD( "821k03.e5",  0x00000, 0x40000, CRC(fef8505a) SHA1(5c5121609f69001838963e961cb227d6b64e4f5f) )
ROM_END

ROM_START( crimfghtj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "821p02.f24", 0x00000, 0x20000, CRC(f33fa2e1) SHA1(00fc9e8250fa51386f3af2fca0f137bec9e1c220) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "821l01.h4",  0x0000, 0x8000, CRC(0faca89e) SHA1(21c9c6d736b398a29e8709e1187c5bf3cacdc99d) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "821k06.k13", 0x000000, 0x040000, CRC(a1eadb24) SHA1(ca305b904b34e03918ad07281fda86ad63caa44f) )
	ROM_LOAD32_WORD( "821k07.k19", 0x000002, 0x040000, CRC(060019fa) SHA1(c3bca007aaa5f1c534d2a75fe4f96d01a740dd58) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "821k04.k2",  0x000000, 0x080000, CRC(00e0291b) SHA1(39d5db6cf36826e47cdf5308eff9bfa8afc82050) )  /* sprites */
	ROM_LOAD32_WORD( "821k05.k8",  0x000002, 0x080000, CRC(e09ea05d) SHA1(50ac9a2117ce63fe774c48d769ec445a83f1269e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.i15", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )  /* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232", 0 ) /* data for the 007232 */
	ROM_LOAD( "821k03.e5",  0x00000, 0x40000, CRC(fef8505a) SHA1(5c5121609f69001838963e961cb227d6b64e4f5f) )
ROM_END

ROM_START( crimfghtu )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
		ROM_LOAD( "821l02.f24", 0x00000, 0x20000, CRC(588e7da6) SHA1(285febb3bcca31f82b34af3695a59eafae01cd30) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "821l01.h4",  0x0000, 0x8000, CRC(0faca89e) SHA1(21c9c6d736b398a29e8709e1187c5bf3cacdc99d) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "821k06.k13", 0x000000, 0x040000, CRC(a1eadb24) SHA1(ca305b904b34e03918ad07281fda86ad63caa44f) )
	ROM_LOAD32_WORD( "821k07.k19", 0x000002, 0x040000, CRC(060019fa) SHA1(c3bca007aaa5f1c534d2a75fe4f96d01a740dd58) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "821k04.k2",  0x000000, 0x080000, CRC(00e0291b) SHA1(39d5db6cf36826e47cdf5308eff9bfa8afc82050) )  /* sprites */
	ROM_LOAD32_WORD( "821k05.k8",  0x000002, 0x080000, CRC(e09ea05d) SHA1(50ac9a2117ce63fe774c48d769ec445a83f1269e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.i15", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )  /* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232", 0 ) /* data for the 007232 */
	ROM_LOAD( "821k03.e5",  0x00000, 0x40000, CRC(fef8505a) SHA1(5c5121609f69001838963e961cb227d6b64e4f5f) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

GAME( 1989, crimfght,  0,        crimfght, crimfght, driver_device, 0, ROT0, "Konami", "Crime Fighters (World 2 players)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, crimfghtu, crimfght, crimfght, crimfghtu, driver_device,0, ROT0, "Konami", "Crime Fighters (US 4 Players)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, crimfghtj, crimfght, crimfght, crimfght, driver_device,0, ROT0, "Konami", "Crime Fighters (Japan 2 Players)", MACHINE_SUPPORTS_SAVE )
