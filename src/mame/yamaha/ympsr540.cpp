// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/sh/sh7042.h"

#include "debugger.h"
#include "speaker.h"

class psr540_state : public driver_device {
public:
	psr540_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_sustain(*this, "SUSTAIN"),
		  m_pitch_bend(*this, "PITCH_BEND")
	{ }

	void psr540(machine_config &config);

private:
	required_device<sh7042_device> m_maincpu;
	required_ioport m_sustain;
	required_ioport m_pitch_bend;

	void map(address_map &map);

	void machine_start() override;

	u16 adc_sustain_r();
	u16 adc_midisw_r();
	u16 adc_battery_r();
};

void psr540_state::machine_start()
{
}

u16 psr540_state::adc_sustain_r()
{
	return m_sustain->read() ? 0x3ff : 0;
}

u16 psr540_state::adc_midisw_r()
{
	return 0;
}

u16 psr540_state::adc_battery_r()
{
	return 0x3ff;
}

void psr540_state::psr540(machine_config &config)
{
	SH7042(config, m_maincpu, 7_MHz_XTAL*4); // internal mask rom active, pll x4
	m_maincpu->set_addrmap(AS_PROGRAM, &psr540_state::map);
	m_maincpu->read_adc<0>().set(FUNC(psr540_state::adc_midisw_r));
	m_maincpu->read_adc<1>().set(FUNC(psr540_state::adc_sustain_r));
	m_maincpu->read_adc<2>().set(FUNC(psr540_state::adc_battery_r));
	m_maincpu->read_adc<3>().set_constant(0);
	m_maincpu->read_adc<4>().set_ioport(m_pitch_bend);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

void psr540_state::map(address_map &map)
{
	map(0x0000000, 0x001ffff).rom().region("kernel", 0).mirror(0x1e0000);

	// 200000-3fffff: cs0 space
	// 200000 fdc
	map(0x0280000, 0x029ffff).ram(); // sram
	// 2a0000 leds
	// 2c0000 lcd

	// 400000-7fffff: cs1 space
	map(0x0400000, 0x07fffff).rom().region("program_rom", 0);

	// c00000-ffffff: cs3 space
	// c00000: swx00

	// Dedicated dram space
	map(0x1000000, 0x13fffff).ram(); // dram
}

static INPUT_PORTS_START( psr540 )
	PORT_START("SUSTAIN")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Sustain Pedal")

	PORT_START("PITCH_BEND")
	PORT_BIT(0x3ff, 0x200, IPT_PADDLE) PORT_NAME("Pitch Bend") PORT_SENSITIVITY(30)

INPUT_PORTS_END

ROM_START( psr540 )
	// Hitachi HI-7 RTOS, version 1.6.  Internal to the sh-2
	ROM_REGION32_BE( 0x400000, "kernel", 0 )
	ROM_LOAD( "hi_7_v1_6.bin", 0, 0x20000, CRC(1f1992d9) SHA1(4f037cc5d7928ace240e31c65dfdff7ce91bd33a))

	ROM_REGION32_BE( 0x400000, "program_rom", 0 )
	ROM_LOAD16_WORD_SWAP( "xw25320.ic310", 0, 0x400000, CRC(e8d29e49) SHA1(079e0ccf6cf5d5bd2d2d82076b09dd702fcd1421))

	ROM_REGION16_LE( 0x600000, "swx00", 0)
	ROM_LOAD16_WORD_SWAP( "xw25410.ic210", 0, 0x400000, CRC(c7c4736d) SHA1(ff1052eb076557071ed8652e6c2fc0925144fbd5))
	ROM_LOAD16_WORD_SWAP( "xw25520.ic220", 0x400000, 0x200000, CRC(9ef56c4e) SHA1(f26b588f9bcfd7bdbf1c0b38e4a1ea57e2f29f10))
ROM_END

SYST( 1999, psr540, 0, 0, psr540, psr540, psr540_state, empty_init, "Yamaha", "PSR540", MACHINE_IS_SKELETON )
