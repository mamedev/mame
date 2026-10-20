// license:BSD-3-Clause
// copyright-holders:

/*

ハローキティのフラワービンゴ (Hello Kitty Flower Bingo) by オービット (Orbit) / マルカ (Maruka) - 1998

OrBIT Co. BAZ-CPU1 PCB

TMPZ84C011BF-6 CPU
12.0MT resonator
TC5565APL-15 SRAM
NEC D71055C PPI
YM2413 sound chip
3.58MT resonator
bank of 4 switches
bank of 2 switches
2x 7-segment LED


Sankyo VP-200A sub PCB

NEC D78C10ACW CPU
12.0MT resonator
NEC D7759C ADPCM chip
640J resonator
bank of 4 switches


OrBIT Co. BIF-MA2 (BIF-LED) PCB
9x 7-segment LED

*/

#include "emu.h"

#include "cpu/upd7810/upd7810.h"
#include "cpu/z80/tmpz84c011.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/upd7759.h"
#include "sound/ymopl.h"

#include "screen.h"
#include "speaker.h"


namespace {

class hkfbingo_state : public driver_device
{
public:
	hkfbingo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void hkfbingo(machine_config &config) ATTR_COLD;

private:
	required_device<tmpz84c011_device> m_maincpu;

	void main_program_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void audio_program_map(address_map &map) ATTR_COLD;
};


void hkfbingo_state::main_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
}

void hkfbingo_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x20, 0x21).w("ym", FUNC(ym2413_device::write));
	map(0x80, 0x83).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void hkfbingo_state::audio_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}


static INPUT_PORTS_START( hkfbingo )
	// externally only coin chute and stop button are actionable
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(4)
INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	TMPZ84C011_DAISY_INTERNAL,
	{ nullptr }
};


void hkfbingo_state::hkfbingo(machine_config &config)
{
	// basic machine hardware
	TMPZ84C011(config, m_maincpu, 12_MHz_XTAL / 2); // divider not verified, but rated for 6 MHz
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &hkfbingo_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &hkfbingo_state::main_io_map);

	upd78c10_device &audiocpu(UPD78C10(config, "audiocpu", 12_MHz_XTAL / 2)); // divider not verified
	audiocpu.set_addrmap(AS_PROGRAM, &hkfbingo_state::audio_program_map);

	I8255(config, "ppi");

	// video hardware

	// TODO: layout for LEDs and lamps

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym", 3.58_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);

	UPD7759(config, "upd", 640_kHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);
}


ROM_START( hkfbingo )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "flower_bingo_v1_0.ic13", 0x0000, 0x8000, CRC(d4c5dc28) SHA1(940dfe486dac9a3f559331dbf276ff743dc51819) ) // 1xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "vp2-10_sankyo.ic6", 0x0000, 0x8000, CRC(eca05ff8) SHA1(d7100f2379a9081daa83270a39bdf345d0116c01) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "kitty_sound.ic7", 0x00000, 0x40000, CRC(3591d3be) SHA1(05e089f5ed45ad6e13667fa1b4d78725a67de180) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


GAME( 1998, hkfbingo, 0, hkfbingo, hkfbingo, hkfbingo_state, empty_init, ROT0, "Orbit / Maruka", "Hello Kitty Flower Bingo", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
