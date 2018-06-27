// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Taito Captain Zodiac, pirate-theme punching bag game with red dot matrix display

****************************************************************************

Hardware summary:
Main PCB:
- 2*Z80 TMPZ84C00AP-6, 12MHz XTAL
- Z80 CTC TMPZ84C30AP-6
- 27C512 ROM, TMS27C010A ROM
- 3*5563-100 (8KB RAM?)
- YM2610B, 16MHz XTAL
- TE7750, TC0140SYT
- SED1351F LCD controller

Display PCB:
- Toshiba TD62C962LF
- 4 16*16 LED matrix boards plugged in

****************************************************************************

TODO:
- everything

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/taitosnd.h"
#include "machine/te7750.h"
#include "machine/z80ctc.h"
#include "sound/2610intf.h"
#include "speaker.h"


class cpzodiac_state : public driver_device
{
public:
	cpzodiac_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu")
	{ }

	void cpzodiac(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;

	void main_map(address_map &map);
	void main_io_map(address_map &map);
	void sound_map(address_map &map);
};


/***************************************************************************

  Memory Maps, I/O

***************************************************************************/

void cpzodiac_state::main_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdfff).ram(); // video?
	map(0xe000, 0xe00f).rw("io", FUNC(te7750_device::read), FUNC(te7750_device::write));
	map(0xe020, 0xe020).w("syt", FUNC(tc0140syt_device::master_port_w));
	map(0xe021, 0xe021).rw("syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}

void cpzodiac_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void cpzodiac_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xe200, 0xe200).w("syt", FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw("syt", FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xf200, 0xf200).nopw();
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( cpzodiac )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

MACHINE_CONFIG_START(cpzodiac_state::cpzodiac)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, 12_MHz_XTAL/2)
	MCFG_DEVICE_PROGRAM_MAP(main_map)
	MCFG_DEVICE_IO_MAP(main_io_map)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)

	MCFG_DEVICE_ADD("io", TE7750, 0)
	MCFG_TE7750_IOS_CB(CONSTANT(2))

	MCFG_DEVICE_ADD("ctc", Z80CTC, 12_MHz_XTAL/2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", 0))

	MCFG_DEVICE_ADD("audiocpu", Z80, 12_MHz_XTAL/2)
	MCFG_DEVICE_PROGRAM_MAP(sound_map)

	/* video hardware */
	// TODO

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("ymsnd", YM2610B, 16_MHz_XTAL/2)
	MCFG_YM2610_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( cpzodiac )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "d52_03-1.ic16", 0x00000, 0x20000, CRC(129b8f44) SHA1(2789cd6f1322176c1956668f024b8bc9d4b3a816) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "d52_04.ic14", 0x00000, 0x10000, CRC(804b45d4) SHA1(db3296558077c7c4eea968417d3edf2509d3742b) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "adpcm.bin", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x1000, "pals", 0 )
	ROM_LOAD( "16l8bcn.ic38", 0x0000, 0x0aee, CRC(6be9b935) SHA1(d36af591b03873aee3098b7c74b53ac6370ca064) )
ROM_END


GAME( 1993, cpzodiac, 0, cpzodiac, cpzodiac, cpzodiac_state, empty_init, ROT0, "Taito Corporation", "Captain Zodiac", MACHINE_SUPPORTS_SAVE | MACHINE_IS_SKELETON_MECHANICAL )
