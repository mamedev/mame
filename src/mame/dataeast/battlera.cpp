// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Battle Rangers                  (c) 1988 Data East Corporation
    Bloody Wolf                     (c) 1988 Data East USA

    Emulation by Bryan McPhail, mish@tendril.co.uk

    This board is based on the Hudson HuC6280 and Huc6270 ICs used in
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
     YM2203C - Yamaha YM2203C FM Operator Type-N(OPN) 3-Channel Sound Chip. Clock input 1.5MHz [12/8]
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
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"
#include "video/huc6260.h"
#include "video/huc6270.h"

#include "screen.h"
#include "speaker.h"


namespace {

class battlera_state : public driver_device
{
public:
	battlera_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_msm(*this, "msm")
		, m_screen(*this, "screen")
		, m_huc6260(*this, "huc6260")
		, m_soundlatch(*this, "soundlatch")
		, m_inputs(*this, { "IN0", "IN1", "IN2", "DSW2", "DSW1" })
	{ }

	void battlera(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<h6280_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<screen_device> m_screen;
	required_device<huc6260_device> m_huc6260;
	required_device<generic_latch_8_device> m_soundlatch;
	required_ioport_array<5> m_inputs;

	uint8_t m_control_port_select = 0;
	uint8_t m_msm5205next = 0;
	uint8_t m_toggle = 0;

	void control_data_w(uint8_t data);
	uint8_t control_data_r();
	void adpcm_data_w(uint8_t data);
	void adpcm_reset_w(uint8_t data);
	void adpcm_int(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_prg_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void battlera_state::machine_start()
{
	save_item(NAME(m_control_port_select));
	save_item(NAME(m_msm5205next));
	save_item(NAME(m_toggle));
}

void battlera_state::machine_reset()
{
	m_control_port_select = 0xff;
	m_msm5205next = 0;
	m_toggle = 0;
}

/******************************************************************************/

void battlera_state::control_data_w(uint8_t data)
{
	m_control_port_select = data;
}

uint8_t battlera_state::control_data_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 5; i++)
		if (!BIT(m_control_port_select, i))
			data &= m_inputs[i]->read();

	return data;
}

/******************************************************************************/

void battlera_state::main_prg_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x1e0800, 0x1e0800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x1e1000, 0x1e13ff).rw(m_huc6260, FUNC(huc6260_device::palette_direct_read), FUNC(huc6260_device::palette_direct_write)).share("paletteram");
	map(0x1f0000, 0x1f1fff).ram(); // Main RAM
	map(0x1fe000, 0x1fe3ff).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
	map(0x1fe400, 0x1fe7ff).rw(m_huc6260, FUNC(huc6260_device::read), FUNC(huc6260_device::write));
}

void battlera_state::main_portmap(address_map &map)
{
	map(0x00, 0x03).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
}

/******************************************************************************/

void battlera_state::adpcm_int(int state)
{
	m_msm->data_w(m_msm5205next >> 4);
	m_msm5205next <<= 4;

	m_toggle ^= 1;
	if (m_toggle)
		m_audiocpu->set_input_line(1, HOLD_LINE);
}

void battlera_state::adpcm_data_w(uint8_t data)
{
	m_msm5205next = data;
}

void battlera_state::adpcm_reset_w(uint8_t data)
{
	m_msm->reset_w(0);
}

void battlera_state::sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x040000, 0x040001).w("ymsnd", FUNC(ym2203_device::write));
	map(0x080000, 0x080001).w(FUNC(battlera_state::adpcm_data_w));
	map(0x1f0000, 0x1f1fff).ram(); // Main RAM
}

/******************************************************************************/

static INPUT_PORTS_START( battlera )
	PORT_START("IN0")  // Player 1 controls
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")  // Player 2 controls
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")   // Coins
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
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )        // Listed as "Unused"
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )        // Listed as "Unused"

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
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        // Listed as "Unused"
INPUT_PORTS_END


/******************************************************************************/

uint32_t battlera_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_huc6260->video_update(bitmap, cliprect);
	return 0;
}


void battlera_state::battlera(machine_config &config)
{
	// basic machine hardware
	H6280(config, m_maincpu, 21.477272_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &battlera_state::main_prg_map);
	m_maincpu->set_addrmap(AS_IO, &battlera_state::main_portmap);
	m_maincpu->port_in_cb().set(FUNC(battlera_state::control_data_r));
	m_maincpu->port_out_cb().set(FUNC(battlera_state::control_data_w));
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	H6280(config, m_audiocpu, 21.477272_MHz_XTAL / 3);
	m_audiocpu->set_addrmap(AS_PROGRAM, &battlera_state::sound_map);
	m_audiocpu->port_in_cb().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	m_audiocpu->port_out_cb().set(FUNC(battlera_state::adpcm_reset_w));
	m_audiocpu->add_route(ALL_OUTPUTS, "mono", 0.60); // music data is stereo, but hardware isn't

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(21.477272_MHz_XTAL, huc6260_device::WPF, 64, 64 + 1024 + 64, huc6260_device::LPF, 18, 18 + 242);
	m_screen->set_screen_update(FUNC(battlera_state::screen_update));
	m_screen->set_palette(m_huc6260);

	HUC6260(config, m_huc6260, 21.477272_MHz_XTAL);
	m_huc6260->next_pixel_data().set("huc6270", FUNC(huc6270_device::next_pixel));
	m_huc6260->time_til_next_event().set("huc6270", FUNC(huc6270_device::time_until_next_event));
	m_huc6260->vsync_changed().set("huc6270", FUNC(huc6270_device::vsync_changed));
	m_huc6260->hsync_changed().set("huc6270", FUNC(huc6270_device::hsync_changed));

	huc6270_device &huc6270(HUC6270(config, "huc6270", 0));
	huc6270.set_vram_size(0x20000);
	huc6270.irq().set_inputline(m_maincpu, 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	YM2203(config, "ymsnd", 12_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.40);

	MSM5205(config, m_msm, 384_kHz_XTAL);
	m_msm->vck_legacy_callback().set(FUNC(battlera_state::adpcm_int));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B); // 8KHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.85);
}

/******************************************************************************/

ROM_START( battlera )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "00_e1.bin", 0x00000, 0x10000, CRC(aa1cbe69) SHA1(982530f3202bc7b8d94d2b818873b71f02c0e8de) ) // ET00
	ROM_LOAD( "es01.rom",  0x10000, 0x10000, CRC(9fea3189) SHA1(0692df6df533dfe55f61df8aa0c5c11944ba3ae3) ) // ET01
	ROM_LOAD( "02_e4.bin", 0x20000, 0x10000, CRC(cd72f580) SHA1(43b476c8f554348b02aa9558c0773f47cdb47fe0) ) // ET02, etc
	// Rom sockets 0x30000 - 0x70000 are unused
	ROM_LOAD( "es05.rom",  0x80000, 0x10000, CRC(551fa331) SHA1(a70c627c572ba1b8029f61eae6eaad9825c56339) )
	ROM_LOAD( "es06.rom",  0x90000, 0x10000, CRC(ab91aac8) SHA1(81d820c8b70281a4a52f7ec75a3c54377011d9d9) )
	ROM_LOAD( "es07.rom",  0xa0000, 0x10000, CRC(8d15a3d0) SHA1(afae081ee5e0de359cae6a7ea8401237c5ab7095) )
	ROM_LOAD( "es08.rom",  0xb0000, 0x10000, CRC(38f06039) SHA1(cc394f161b2c4423cd2da763701ceaad7d27f741) )
	ROM_LOAD( "es09.rom",  0xc0000, 0x10000, CRC(b718c47d) SHA1(1d5b2ec819b0848e5b883373887445a63ebddb06) )
	ROM_LOAD( "es10-1.rom",0xd0000, 0x10000, CRC(d3cddc02) SHA1(d212127a9d7aff384171d79c563f1516c0bd46ae) )
	// Rom sockets 0xe0000 - 0x100000 are unused

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "es11.rom",  0x00000, 0x10000, CRC(f5b29c9c) SHA1(44dcdf96f8deb9a29aa9d94a8b9cf91a0ed808d4) )
ROM_END

ROM_START( bldwolf )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "es00-1.rom", 0x00000, 0x10000, CRC(ff4aa252) SHA1(3c190e49020bb6923abb3f3c2632d3c86443c292) )
	ROM_LOAD( "es01.rom",   0x10000, 0x10000, CRC(9fea3189) SHA1(0692df6df533dfe55f61df8aa0c5c11944ba3ae3) )
	ROM_LOAD( "es02-1.rom", 0x20000, 0x10000, CRC(49792753) SHA1(4f3fb6912607d373fc0c1096ac0a8cc939e33617) )
	// Rom sockets 0x30000 - 0x70000 are unused
	ROM_LOAD( "es05.rom",   0x80000, 0x10000, CRC(551fa331) SHA1(a70c627c572ba1b8029f61eae6eaad9825c56339) )
	ROM_LOAD( "es06.rom",   0x90000, 0x10000, CRC(ab91aac8) SHA1(81d820c8b70281a4a52f7ec75a3c54377011d9d9) )
	ROM_LOAD( "es07.rom",   0xa0000, 0x10000, CRC(8d15a3d0) SHA1(afae081ee5e0de359cae6a7ea8401237c5ab7095) )
	ROM_LOAD( "es08.rom",   0xb0000, 0x10000, CRC(38f06039) SHA1(cc394f161b2c4423cd2da763701ceaad7d27f741) )
	ROM_LOAD( "es09.rom",   0xc0000, 0x10000, CRC(b718c47d) SHA1(1d5b2ec819b0848e5b883373887445a63ebddb06) )
	ROM_LOAD( "es10-1.rom", 0xd0000, 0x10000, CRC(d3cddc02) SHA1(d212127a9d7aff384171d79c563f1516c0bd46ae) )
	// Rom sockets 0xe0000 - 0x100000 are unused

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "es11.rom",   0x00000, 0x10000, CRC(f5b29c9c) SHA1(44dcdf96f8deb9a29aa9d94a8b9cf91a0ed808d4) )
ROM_END

ROM_START( bldwolfj ) // note, ROM codes are ER not ES even if the content of some ROMs is identical
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "er00-.0-0", 0x00000, 0x10000, CRC(3819a14e) SHA1(0222051e0b5ec87a18f2e6e9155034f91898c14f) )
	ROM_LOAD( "er01-.0-1", 0x10000, 0x10000, CRC(763cf206) SHA1(0f1c0f80a6aaad0c987c2ba3fdd01db1f5ceb7e6) )
	ROM_LOAD( "er02-.0-2", 0x20000, 0x10000, CRC(bcad8a0f) SHA1(e7c69d2c894eaedd10ce02f6bceaa43bb060afb9) )
	// Rom sockets 0x30000 - 0x70000 are unused
	ROM_LOAD( "er05-.1-0", 0x80000, 0x10000, CRC(551fa331) SHA1(a70c627c572ba1b8029f61eae6eaad9825c56339) )
	ROM_LOAD( "er06-.1-1", 0x90000, 0x10000, CRC(ab91aac8) SHA1(81d820c8b70281a4a52f7ec75a3c54377011d9d9) )
	ROM_LOAD( "er07-.1-2", 0xa0000, 0x10000, CRC(8d15a3d0) SHA1(afae081ee5e0de359cae6a7ea8401237c5ab7095) )
	ROM_LOAD( "er08-.1-3", 0xb0000, 0x10000, CRC(38f06039) SHA1(cc394f161b2c4423cd2da763701ceaad7d27f741) )
	ROM_LOAD( "er09-.1-4", 0xc0000, 0x10000, CRC(b718c47d) SHA1(1d5b2ec819b0848e5b883373887445a63ebddb06) )
	ROM_LOAD( "er10-.1-5", 0xd0000, 0x10000, CRC(d3cddc02) SHA1(d212127a9d7aff384171d79c563f1516c0bd46ae) )
	// Rom sockets 0xe0000 - 0x100000 are unused

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "er11-.tpg",   0x00000, 0x10000, CRC(f5b29c9c) SHA1(44dcdf96f8deb9a29aa9d94a8b9cf91a0ed808d4) )
ROM_END

} // Anonymous namespace


/******************************************************************************/

GAME( 1988, battlera, 0,        battlera, battlera, battlera_state, empty_init, ROT0, "Data East Corporation", "Battle Rangers (World)",                     MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1988, bldwolf,  battlera, battlera, battlera, battlera_state, empty_init, ROT0, "Data East USA",         "Bloody Wolf (US)",                           MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1988, bldwolfj, battlera, battlera, battlera, battlera_state, empty_init, ROT0, "Data East Corporation", "Narazumono Sentoubutai Bloody Wolf (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
