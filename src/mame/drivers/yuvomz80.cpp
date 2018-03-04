// license:BSD-3-Clause
// copyright-holders:

// Skeleton driver for Yuvo / Yubis Z80 based medal games.

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/2610intf.h"
#include "sound/ymz280b.h"
#include "speaker.h"

/*
This tries to document the available info, but even game titles should be taken with a grain of salt.
YouTube video references:
Hexa President: http://www.youtube.com/watch?v=5Ea5HxH2zwM&t=8s
Golden Hexa: http://www.youtube.com/watch?v=3u1ccTo3SGI
*/

/*
Golden Hexa by Yubis

PCBs:

Yuvo PCC116B - maincpu board
- TMPZ84C00AP-8
- 8 MHz XTAL
- maincpu ROM
- 2 x TMP82C55AN-2
- 2 x 8 dips bank

Yuvo PCO124B - sound board
- TMPZ84C00AP-8
- 8 MHz, 16.9344 MHz XTALs
- audiocpu ROM
- YMZ280B-F
- sample ROMs

Yuvo PCI117B - I/O board
- connectors and ttl chips
*/

class yuvomz80_state : public driver_device
{
public:
	yuvomz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void goldhexa(machine_config &config);
	void hexapres(machine_config &config);
	void audio_io_map(address_map &map);
	void audio_mem_map(address_map &map);
	void hexapres_audio_io_map(address_map &map);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};

ADDRESS_MAP_START(yuvomz80_state::mem_map)
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(yuvomz80_state::io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi0", i8255_device, read, write)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi1", i8255_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ppi2", i8255_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("ppi3", i8255_device, read, write)
ADDRESS_MAP_END

ADDRESS_MAP_START(yuvomz80_state::audio_mem_map)
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("audiocpu", 0)
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(yuvomz80_state::audio_io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymz", ymz280b_device, read, write)
ADDRESS_MAP_END

ADDRESS_MAP_START(yuvomz80_state::hexapres_audio_io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
ADDRESS_MAP_END

static INPUT_PORTS_START( goldhexa )
INPUT_PORTS_END

MACHINE_CONFIG_START(yuvomz80_state::goldhexa)
	MCFG_CPU_ADD("maincpu", Z80, XTAL(8'000'000))
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL(8'000'000))
	MCFG_CPU_PROGRAM_MAP(audio_mem_map)
	MCFG_CPU_IO_MAP(audio_io_map)

	MCFG_DEVICE_ADD("ppi0", I8255A, 0)
	MCFG_DEVICE_ADD("ppi1", I8255A, 0)
	MCFG_DEVICE_ADD("ppi2", I8255A, 0)
	MCFG_DEVICE_ADD("ppi3", I8255A, 0)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ymz", YMZ280B, XTAL(16'934'400))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(yuvomz80_state::hexapres)
	MCFG_CPU_ADD("maincpu", Z80, XTAL(8'000'000))
	MCFG_DEVICE_DISABLE()

	MCFG_CPU_ADD("audiocpu", Z80, XTAL(8'000'000))
	MCFG_CPU_PROGRAM_MAP(audio_mem_map)
	MCFG_CPU_IO_MAP(hexapres_audio_io_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2610, 8000000) // type guessed
	MCFG_YM2610_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( goldhexa )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "ghp_program_ver.1.02.ic8",  0x0000, 0x10000, CRC(a21a8cfd) SHA1(324f54ca6e17373138df2854f8c5e77cf78e9434) ) // 1111xxxxxxxxxxxx = 0xFF

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD( "hp-sp_ver.1.01.ic7",  0x0000, 0x2000, CRC(05133b91) SHA1(2fe931e55c503f15aedfb1c1ea14c257f57c564b) ) // 1xxxxxxxxxxxx = 0xFF

	ROM_REGION(0x100000, "ymz", 0)
	ROM_LOAD( "ghp_pcm-a_ver.1.01.ic12",  0x00000, 0x80000, CRC(08de888b) SHA1(a6b68accb136481f45b65eab33e0bab5212a1daf) )
	ROM_LOAD( "ghp_pcm-b_ver.1.01.ic13",  0x80000, 0x80000, CRC(161838c9) SHA1(52b9c324b01702c1164a462af371d82e8c2eea43) )
ROM_END

ROM_START( hexapres )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "hexapres.bin",  0x0000, 0x10000, NO_DUMP )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD( "ghp_snd.bin",  0x0000, 0x10000, CRC(8933b6ea) SHA1(a66157f2b7407ab374db07bcda34f066740f14dc) )

	ROM_REGION(0x80000, "ymsnd", 0)
	ROM_LOAD( "ghp_voia.bin",  0x00000, 0x80000, CRC(cf3e4c43) SHA1(6d348054704d1d0082d6166701ab84cb162b3a26) )

	ROM_REGION(0x80000, "ymsnd.deltat", 0)
	ROM_LOAD( "ghp_voib.bin",  0x00000, 0x80000, CRC(8be745fe) SHA1(840bbb212c8c519f2e4633f8db731fcf3f55635a) )
ROM_END

GAME( 200?, goldhexa, 0, goldhexa, goldhexa, yuvomz80_state, 0, ROT0, "Yubis", "Golden Hexa", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hexapres, 0, hexapres, goldhexa, yuvomz80_state, 0, ROT0, "Yubis", "Hexa President", MACHINE_IS_SKELETON_MECHANICAL )
