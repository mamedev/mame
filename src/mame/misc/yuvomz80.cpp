// license:BSD-3-Clause
// copyright-holders:

// Skeleton driver for Yuvo / Yubis Z80 based medal games.

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ymopn.h"
#include "sound/ymz280b.h"
#include "speaker.h"

/*
This tries to document the available info, but even game titles should be taken with a grain of salt.
YouTube video references:
Hexa President: http://www.youtube.com/watch?v=5Ea5HxH2zwM&t=8s
Golden Hexa: http://www.youtube.com/watch?v=3u1ccTo3SGI
*/

/*
Hexa President by Yuvo / Yubis

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


namespace {

class yuvomz80_state : public driver_device
{
public:
	yuvomz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void hexaprs(machine_config &config);
	void hexaprsz(machine_config &config);

private:
	void audio_mem_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
	void hexaprsz_audio_io_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void yuvomz80_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram();
}

void yuvomz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0c, 0x0f).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void yuvomz80_state::audio_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("audiocpu", 0);
	map(0x8000, 0x87ff).ram();
}

void yuvomz80_state::audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
}

void yuvomz80_state::hexaprsz_audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
}

static INPUT_PORTS_START( hexaprs )
INPUT_PORTS_END

void yuvomz80_state::hexaprs(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_disable();

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(8'000'000)));
	audiocpu.set_addrmap(AS_PROGRAM, &yuvomz80_state::audio_mem_map);
	audiocpu.set_addrmap(AS_IO, &yuvomz80_state::audio_io_map);

	SPEAKER(config, "mono").front_center();
	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8000000)); // type guessed
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);
}

void yuvomz80_state::hexaprsz(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &yuvomz80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &yuvomz80_state::io_map);

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(8'000'000)));
	audiocpu.set_addrmap(AS_PROGRAM, &yuvomz80_state::audio_mem_map);
	audiocpu.set_addrmap(AS_IO, &yuvomz80_state::hexaprsz_audio_io_map);

	I8255A(config, "ppi0", 0);
	I8255A(config, "ppi1", 0);
	I8255A(config, "ppi2", 0);
	I8255A(config, "ppi3", 0);

	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(16'934'400)));
	ymz.add_route(0, "speaker", 1.00, 0);
	ymz.add_route(1, "speaker", 1.00, 1);
}

ROM_START( hexaprs )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "hexaprs.bin",  0x0000, 0x10000, NO_DUMP )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD( "ghp_snd.bin",  0x0000, 0x10000, CRC(8933b6ea) SHA1(a66157f2b7407ab374db07bcda34f066740f14dc) )

	ROM_REGION(0x80000, "ymsnd:adpcma", 0)
	ROM_LOAD( "ghp_voia.bin",  0x00000, 0x80000, CRC(cf3e4c43) SHA1(6d348054704d1d0082d6166701ab84cb162b3a26) )

	ROM_REGION(0x80000, "ymsnd:adpcmb", 0)
	ROM_LOAD( "ghp_voib.bin",  0x00000, 0x80000, CRC(8be745fe) SHA1(840bbb212c8c519f2e4633f8db731fcf3f55635a) )
ROM_END

ROM_START( hexaprsz )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "ghp_program_ver.1.02.ic8",  0x0000, 0x10000, CRC(a21a8cfd) SHA1(324f54ca6e17373138df2854f8c5e77cf78e9434) ) // 1111xxxxxxxxxxxx = 0xFF

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD( "hp-sp_ver.1.01.ic7",  0x0000, 0x2000, CRC(05133b91) SHA1(2fe931e55c503f15aedfb1c1ea14c257f57c564b) ) // 1xxxxxxxxxxxx = 0xFF

	ROM_REGION(0x100000, "ymz", 0)
	ROM_LOAD( "ghp_pcm-a_ver.1.01.ic12",  0x00000, 0x80000, CRC(08de888b) SHA1(a6b68accb136481f45b65eab33e0bab5212a1daf) )
	ROM_LOAD( "ghp_pcm-b_ver.1.01.ic13",  0x80000, 0x80000, CRC(161838c9) SHA1(52b9c324b01702c1164a462af371d82e8c2eea43) )
ROM_END

} // anonymous namespace


GAME( 1995, hexaprs,  0,       hexaprs,  hexaprs, yuvomz80_state, empty_init, ROT0, "Yuvo / Yubis", "Hexa President (YM2610 set)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1995, hexaprsz, hexaprs, hexaprsz, hexaprs, yuvomz80_state, empty_init, ROT0, "Yuvo / Yubis", "Hexa President (YMZ280B set)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
