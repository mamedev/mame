// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Battle Rangers                  (c) 1988 Data East Corporation
    Bloody Wolf                     (c) 1988 Data East USA

    Emulation by Bryan McPhail, mish@tendril.co.uk

    This board is based on the Hudson HuC6280 and Huc6270 IC's used in 
    the NEC PC-Engine.
    
    Differences from PC-Engine console:
    Input ports are different (2 dips, 2 joysticks, 1 coin port)
    _Interface_ to palette is different (Huc6260 isn't present), 
    palette data is the same.
    Extra sound chips (YM2203 & Oki M5205), and extra HuC6280 processor to drive them.
    Twice as much VRAM (128kb).

    Todo:
    - There seems to be a bug with a stuck note from the YM2203 FM channel
      at the start of scene 3 and near the ending when your characters are
      flying over a forest in a helicopter. 
      This is verified to NOT happen on real hardware - Guru

**********************************************************************

Battle Rangers / Bloody Wolf
Data East 1988

This game runs on custom Data East hardware using some of the ICs used in
the NEC PC Engine video game console made by NEC in 1987.
The PCB is NOT a modified PC Engine, it's a game-specific arcade PCB
manufactured by Data East.

PCB Layout
----------

DE-0314-2
  |-----------------------------------------------|
|-|                   |----|                      |
|                     |DEC-01           ET10-.L3  |
|J                    |----|              ET09-.L1|
|A     DSW2                                       |
|M        DSW1                                    |
|M RCDM-I1                        ET08-.J5        |
|A RCDM-I1                          ET07-.J4      |
|  RCDM-I1                             ET06-.J3   |
|  RCDM-I1                                ET05-.J1|
|-|RCDM-I1                                        |
  |         12MHz                 2063            |
|-|                       2018(1)         ET00-.E1|
|       21.4772MHz     ET11-.D10        ET01-.E3  |
|                                    ET02-.E4     |
|                           2018(2)               |
|  YM2203C                          62256   62256 |
|YM3014B              C1060C         62256   62256|
|UPC3403                                          |
| 384kHz              |----|         |----|       |
|VOL M5205            | 45 |         |6270|       |
|MB3730               |----|         |----|       |
|-------------------------------------------------|
Notes:
      DEC-01 - Hudson HuC6280 6502-based CPU with in-built Programmable Sound Generator
               used as the main CPU. Clock input is 21.4772MHz and is divided internally
               by 3 for the CPU (7.15906MHz) and by 6 for the PSG (3.579533MHz), although
               in this case the PSG isn't used. The Hudson markings have been scratch off 
               and the IC is labelled 'DEC-01'
          45 - Hudson HuC6280 6502-based CPU with in-built PSG used as the sound CPU
               Clock input is 21.4772MHz and is divided internally by 3 for the CPU
               and by 6 for the PSG. The Hudson markings have been scratch off and the IC
               is labelled '45'
        6270 - Hudson HuC6270 Video Display Controller. The Hudson markings have been
               scratch off. The chip was labelled by Data East as something else but the
               sticker is no longer present on top of the chip. Note the HuC6260 is NOT
               present on the PCB, some logic and RAM handle the color encoding
     2018(1) - Toshiba TMM2018 2kx8 SRAM used for color RAM
     2018(2) - Toshiba TMM2018 2kx8 SRAM used for sound program RAM
        2063 - Toshiba TMM2063 8kx8 SRAM used for main work RAM
       62256 - Hitachi HM62256 32kx8 SRAM used for video RAM
         ET* - EPROMs/MaskROMs
     YM2203C - Yahama YM2203C FM Operator Type-N(OPN) 3-Channel Sound Chip. Clock input 1.5MHz [12/8]
     YM3014B - Yamaha YM3014B Serial Input Floating D/A Converter
       M5205 - Oki M5205 ADPCM Speech Synthesis LSI. Clock input is via a 384kHz resonator
      MB3730 - Fujitsu MB3730 14W BTL Audio Power Amplifier. Audio output is mono via the JAMMA connector
        DSW* - 8-position DIP switch
     uPC3403 - NEC uPC3403 High Performance Quad Operational Amplifier
      C1060C - NEC C1060C High Precision Reference Voltage Circuit
     RCDM-I1 - Custom Ceramic Resistor Array
       VSync - 59.12246Hz   \
       HSync - 15.60838kHz  / measured on pins 25/26 of the HuC6270

**********************************************************************/

#include "emu.h"
#include "cpu/h6280/h6280.h"
#include "sound/2203intf.h"
#include "sound/c6280.h"
#include "includes/battlera.h"


void battlera_state::machine_start()
{
	save_item(NAME(m_control_port_select));
	save_item(NAME(m_msm5205next));
	save_item(NAME(m_toggle));
}

/******************************************************************************/

WRITE8_MEMBER(battlera_state::sound_w)
{
	if (offset == 0)
	{
		soundlatch_byte_w(space,0,data);
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

/******************************************************************************/

WRITE8_MEMBER(battlera_state::control_data_w)
{
	m_control_port_select=data;
}

READ8_MEMBER(battlera_state::control_data_r)
{
	switch (m_control_port_select)
	{
		case 0xfe: return ioport("IN0")->read(); /* Player 1 */
		case 0xfd: return ioport("IN1")->read(); /* Player 2 */
		case 0xfb: return ioport("IN2")->read(); /* Coins */
		case 0xf7: return ioport("DSW2")->read(); /* Dip 2 */
		case 0xef: return ioport("DSW1")->read(); /* Dip 1 */
	}

	return 0xff;
}

/******************************************************************************/

static ADDRESS_MAP_START( battlera_map, AS_PROGRAM, 8, battlera_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x1e0800, 0x1e0801) AM_WRITE(sound_w)
	AM_RANGE(0x1e1000, 0x1e13ff) AM_DEVREADWRITE( "huc6260", huc6260_device, palette_direct_read, palette_direct_write) AM_SHARE("paletteram")
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAMBANK("bank8") /* Main ram */
	AM_RANGE(0x1ff000, 0x1ff001) AM_READWRITE(control_data_r, control_data_w)

	AM_RANGE( 0x1FE000, 0x1FE3FF) AM_DEVREADWRITE( "huc6270", huc6270_device, read, write )
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_DEVREADWRITE( "huc6260", huc6260_device, read, write )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_DEVREADWRITE( "maincpu", h6280_device, timer_r, timer_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_DEVREADWRITE( "maincpu", h6280_device, irq_status_r, irq_status_w )

ADDRESS_MAP_END

static ADDRESS_MAP_START( battlera_portmap, AS_IO, 8, battlera_state )
	AM_RANGE( 0x00, 0x03) AM_DEVREADWRITE( "huc6270", huc6270_device, read, write )
ADDRESS_MAP_END

/******************************************************************************/


WRITE_LINE_MEMBER(battlera_state::adpcm_int)
{
	m_msm->data_w(m_msm5205next >> 4);
	m_msm5205next <<= 4;

	m_toggle = 1 - m_toggle;
	if (m_toggle)
		m_audiocpu->set_input_line(1, HOLD_LINE);
}

WRITE8_MEMBER(battlera_state::adpcm_data_w)
{
	m_msm5205next = data;
}

WRITE8_MEMBER(battlera_state::adpcm_reset_w)
{
	m_msm->reset_w(0);
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, battlera_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x040001) AM_DEVWRITE("ymsnd", ym2203_device, write)
	AM_RANGE(0x080000, 0x080001) AM_WRITE(adpcm_data_w)
	AM_RANGE(0x1fe800, 0x1fe80f) AM_DEVWRITE("c6280", c6280_device, c6280_w)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAMBANK("bank7") /* Main ram */
	AM_RANGE(0x1ff000, 0x1ff001) AM_READ(soundlatch_byte_r) AM_WRITE(adpcm_reset_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_DEVWRITE("audiocpu", h6280_device, irq_status_w)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( battlera )
	PORT_START("IN0")  /* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")  /* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")   /* Coins */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )        /* Listed as "Unused" */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */
INPUT_PORTS_END


/******************************************************************************/

UINT32 battlera_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_huc6260->video_update( bitmap, cliprect );
	return 0;
}


static MACHINE_CONFIG_START( battlera, battlera_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280,21477200/3)
	MCFG_CPU_PROGRAM_MAP(battlera_map)
	MCFG_CPU_IO_MAP(battlera_portmap)

	MCFG_CPU_ADD("audiocpu", H6280,21477200/3)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK, HUC6260_WPF, 64, 64 + 1024 + 64, HUC6260_LPF, 18, 18 + 242)
	MCFG_SCREEN_UPDATE_DRIVER( battlera_state, screen_update )
	MCFG_SCREEN_PALETTE("huc6260:palette")

	MCFG_DEVICE_ADD( "huc6260", HUC6260, MAIN_CLOCK )
	MCFG_HUC6260_NEXT_PIXEL_DATA_CB(DEVREAD16("huc6270", huc6270_device, next_pixel))
	MCFG_HUC6260_TIME_TIL_NEXT_EVENT_CB(DEVREAD16("huc6270", huc6270_device, time_until_next_event))
	MCFG_HUC6260_VSYNC_CHANGED_CB(DEVWRITELINE("huc6270", huc6270_device, vsync_changed))
	MCFG_HUC6260_HSYNC_CHANGED_CB(DEVWRITELINE("huc6270", huc6270_device, hsync_changed))

	MCFG_DEVICE_ADD( "huc6270", HUC6270, 0 )
	MCFG_HUC6270_VRAM_SIZE(0x20000)
	MCFG_HUC6270_IRQ_CHANGED_CB(INPUTLINE("maincpu", 0))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2203, 12000000 / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(battlera_state, adpcm_int)) /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8KHz            */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.85)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.85)

	MCFG_SOUND_ADD("c6280", C6280, 21477270/6)
	MCFG_C6280_CPU("audiocpu")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( battlera )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Main cpu code */
	ROM_LOAD( "00_e1.bin", 0x00000, 0x10000, CRC(aa1cbe69) SHA1(982530f3202bc7b8d94d2b818873b71f02c0e8de) ) /* ET00 */
	ROM_LOAD( "es01.rom",  0x10000, 0x10000, CRC(9fea3189) SHA1(0692df6df533dfe55f61df8aa0c5c11944ba3ae3) ) /* ET01 */
	ROM_LOAD( "02_e4.bin", 0x20000, 0x10000, CRC(cd72f580) SHA1(43b476c8f554348b02aa9558c0773f47cdb47fe0) ) /* ET02, etc */
	/* Rom sockets 0x30000 - 0x70000 are unused */
	ROM_LOAD( "es05.rom",  0x80000, 0x10000, CRC(551fa331) SHA1(a70c627c572ba1b8029f61eae6eaad9825c56339) )
	ROM_LOAD( "es06.rom",  0x90000, 0x10000, CRC(ab91aac8) SHA1(81d820c8b70281a4a52f7ec75a3c54377011d9d9) )
	ROM_LOAD( "es07.rom",  0xa0000, 0x10000, CRC(8d15a3d0) SHA1(afae081ee5e0de359cae6a7ea8401237c5ab7095) )
	ROM_LOAD( "es08.rom",  0xb0000, 0x10000, CRC(38f06039) SHA1(cc394f161b2c4423cd2da763701ceaad7d27f741) )
	ROM_LOAD( "es09.rom",  0xc0000, 0x10000, CRC(b718c47d) SHA1(1d5b2ec819b0848e5b883373887445a63ebddb06) )
	ROM_LOAD( "es10-1.rom",0xd0000, 0x10000, CRC(d3cddc02) SHA1(d212127a9d7aff384171d79c563f1516c0bd46ae) )
	/* Rom sockets 0xe0000 - 0x100000 are unused */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "es11.rom",  0x00000, 0x10000, CRC(f5b29c9c) SHA1(44dcdf96f8deb9a29aa9d94a8b9cf91a0ed808d4) )
ROM_END

ROM_START( bldwolf )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Main cpu code */
	ROM_LOAD( "es00-1.rom", 0x00000, 0x10000, CRC(ff4aa252) SHA1(3c190e49020bb6923abb3f3c2632d3c86443c292) )
	ROM_LOAD( "es01.rom",   0x10000, 0x10000, CRC(9fea3189) SHA1(0692df6df533dfe55f61df8aa0c5c11944ba3ae3) )
	ROM_LOAD( "es02-1.rom", 0x20000, 0x10000, CRC(49792753) SHA1(4f3fb6912607d373fc0c1096ac0a8cc939e33617) )
	/* Rom sockets 0x30000 - 0x70000 are unused */
	ROM_LOAD( "es05.rom",   0x80000, 0x10000, CRC(551fa331) SHA1(a70c627c572ba1b8029f61eae6eaad9825c56339) )
	ROM_LOAD( "es06.rom",   0x90000, 0x10000, CRC(ab91aac8) SHA1(81d820c8b70281a4a52f7ec75a3c54377011d9d9) )
	ROM_LOAD( "es07.rom",   0xa0000, 0x10000, CRC(8d15a3d0) SHA1(afae081ee5e0de359cae6a7ea8401237c5ab7095) )
	ROM_LOAD( "es08.rom",   0xb0000, 0x10000, CRC(38f06039) SHA1(cc394f161b2c4423cd2da763701ceaad7d27f741) )
	ROM_LOAD( "es09.rom",   0xc0000, 0x10000, CRC(b718c47d) SHA1(1d5b2ec819b0848e5b883373887445a63ebddb06) )
	ROM_LOAD( "es10-1.rom", 0xd0000, 0x10000, CRC(d3cddc02) SHA1(d212127a9d7aff384171d79c563f1516c0bd46ae) )
	/* Rom sockets 0xe0000 - 0x100000 are unused */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "es11.rom",   0x00000, 0x10000, CRC(f5b29c9c) SHA1(44dcdf96f8deb9a29aa9d94a8b9cf91a0ed808d4) )
ROM_END

ROM_START( bldwolfj ) /* note, rom codes are ER not ES even if the content of some roms is identical */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Main cpu code */
	ROM_LOAD( "er00-.0-0", 0x00000, 0x10000, CRC(3819a14e) SHA1(0222051e0b5ec87a18f2e6e9155034f91898c14f) )
	ROM_LOAD( "er01-.0-1", 0x10000, 0x10000, CRC(763cf206) SHA1(0f1c0f80a6aaad0c987c2ba3fdd01db1f5ceb7e6) )
	ROM_LOAD( "er02-.0-2", 0x20000, 0x10000, CRC(bcad8a0f) SHA1(e7c69d2c894eaedd10ce02f6bceaa43bb060afb9) )
	/* Rom sockets 0x30000 - 0x70000 are unused */
	ROM_LOAD( "er05-.1-0", 0x80000, 0x10000, CRC(551fa331) SHA1(a70c627c572ba1b8029f61eae6eaad9825c56339) )
	ROM_LOAD( "er06-.1-1", 0x90000, 0x10000, CRC(ab91aac8) SHA1(81d820c8b70281a4a52f7ec75a3c54377011d9d9) )
	ROM_LOAD( "er07-.1-2", 0xa0000, 0x10000, CRC(8d15a3d0) SHA1(afae081ee5e0de359cae6a7ea8401237c5ab7095) )
	ROM_LOAD( "er08-.1-3", 0xb0000, 0x10000, CRC(38f06039) SHA1(cc394f161b2c4423cd2da763701ceaad7d27f741) )
	ROM_LOAD( "er09-.1-4", 0xc0000, 0x10000, CRC(b718c47d) SHA1(1d5b2ec819b0848e5b883373887445a63ebddb06) )
	ROM_LOAD( "er10-.1-5", 0xd0000, 0x10000, CRC(d3cddc02) SHA1(d212127a9d7aff384171d79c563f1516c0bd46ae) )
	/* Rom sockets 0xe0000 - 0x100000 are unused */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "er11-.tpg",   0x00000, 0x10000, CRC(f5b29c9c) SHA1(44dcdf96f8deb9a29aa9d94a8b9cf91a0ed808d4) )
ROM_END



/******************************************************************************/

GAME( 1988, battlera, 0,        battlera, battlera, driver_device,  0,   ROT0, "Data East Corporation", "Battle Rangers (World)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1988, bldwolf,  battlera, battlera, battlera, driver_device,  0,   ROT0, "Data East USA", "Bloody Wolf (US)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1988, bldwolfj, battlera, battlera, battlera, driver_device,  0,   ROT0, "Data East Corporation", "Narazumono Sentoubutai Bloody Wolf (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
