// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 Casio SK-1 Sampling Keyboard
 Also sold as Realistic Concertmate-500

 Low-cost 32-key 4-voice digital sampling keyboard.
 Holds a single 8-bit 9.38kHz PCM in volatile RAM.

 LSI1   SoC     MSM6283-01GS SoC
 LSI2   ROM     µPD23C256EAC-011 ROM
 LSI3   DRAM    µPD4168C
 LSI4   DRAM    µPD4168C

 7.24MHz system clock generated using an L10-495 oscillator coil and capacitors

 SoC has onboard sampling and playback capability
 Analog filtering section is simple and can be netlisted once SoC is producing output

 SoC has onboard keyboard scanning - matrix is 10 rows by 8 columns (KO0-KO9 and KI1-KI8)
 Piano keys have N-key rollover diodes, other buttons/switches don't

 So-called program ROM actually mostly contains samples, which begin at offset 0x0b00;
 a table at 0x085c has their starting addresses. The real microcode is likely internal,
 since MSM6283s with different suffixes show up in other Casio keyboards.
 */

#include "emu.h"
#include "machine/bankdev.h"


namespace {

class sk1_state : public driver_device
{
public:
	sk1_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void sk1(machine_config &config);

	// make slide switches usable on a keyboard
	template <ioport_value V> DECLARE_INPUT_CHANGED_MEMBER(sw_function);
	template <ioport_value V> DECLARE_INPUT_CHANGED_MEMBER(sw_mode);

	ioport_value function_in() { return m_sw_function; }
	ioport_value mode_in() { return m_sw_mode; }

private:
	void sk1_memory(address_map &map) ATTR_COLD;

	virtual void driver_start() override;

	ioport_value    m_sw_function = 0xfe;
	ioport_value    m_sw_mode = 0xfe;
};


template <ioport_value V> INPUT_CHANGED_MEMBER(sk1_state::sw_function)
{
	if (!newval && oldval)
	{
		if (!(~m_sw_function & 0x03) && (~V & 0x03))
		{
			// TODO: generate CPU reset pulse
		}
		m_sw_function = V;
	}
}


template <ioport_value V> INPUT_CHANGED_MEMBER(sk1_state::sw_mode)
{
	if (!newval && oldval)
		m_sw_mode = V;
}


void sk1_state::driver_start()
{
	save_item(NAME(m_sw_function));
	save_item(NAME(m_sw_mode));
}


void sk1_state::sk1_memory(address_map &map)
{
	// chip selects are driven by decoding A13 and A15 with IC3 quad 2-input NOR gate
	map(0x0000, 0x7fff).rom().region("lsi2", 0x0000);
	map(0x8000, 0x83ff).mirror(0x4000).ram();
}


void sk1_state::sk1(machine_config &config)
{
	// just to attach the memory map to something until I can work out what the CPU core is
	ADDRESS_MAP_BANK(config, "dummy").set_map(&sk1_state::sk1_memory).set_data_width(8).set_addr_width(16);
}


INPUT_PORTS_START(sk1)
	PORT_START("KO0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("SAMPLING")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("LOOP SET")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("DEMO")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("ONE KEY PLAY R")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("ONE KEY PLAY L")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KO1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("PORTAMENTO")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("HARMONIC SYNTHESIZER")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("ENVELOPE SELECT")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("VIBRATO")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("F3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("F3#")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("G3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("G3#")

	PORT_START("KO2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("MEMORY PLAY")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("RESET")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("RHYTHM SELECT")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("FILL-IN")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("A3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("A3#")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("B3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("C4")

	PORT_START("KO3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("TEMPO+")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("TEMPO-")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("DELETE")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("CLEAR")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("C4#")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("D4")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("D4#")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("E4")

	PORT_START("KO4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("SAMPLING SOUND")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("HARMONICS SOUND")
	PORT_BIT(0x0c, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("F4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("F4#")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("G4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("G4#")

	PORT_START("KO5")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("A4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("A4#")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("B4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("C5")

	PORT_START("KO6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("JAZZ ORGAN")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("PIPE ORGAN")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("FLUTE")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("HUMAN VOICE")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("C5#")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("D5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("D5#")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("E5")

	PORT_START("KO7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("SYNTH DRUMS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("TRUMPET")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("BRASS ENSEMBLE")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("PIANO")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("F5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("F5#")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("G5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("G5#")

	PORT_START("KO8")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(sk1_state, mode_in)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("A5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("A5#")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("B5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("C6")

	PORT_START("KO9")
	PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(sk1_state, function_in)
	PORT_BIT(0x7c, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("TOGGLES")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("PLAY")      PORT_CHANGED_MEMBER(DEVICE_SELF, sk1_state, sw_function<0xfe>, 0) // three-position function switch
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("RECORD")    PORT_CHANGED_MEMBER(DEVICE_SELF, sk1_state, sw_function<0xfd>, 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("POWER OFF") PORT_CHANGED_MEMBER(DEVICE_SELF, sk1_state, sw_function<0x7f>, 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("NORMAL")    PORT_CHANGED_MEMBER(DEVICE_SELF, sk1_state, sw_mode<0xfe>, 0) // four-position mode switch
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("SOLO 1")    PORT_CHANGED_MEMBER(DEVICE_SELF, sk1_state, sw_mode<0xfd>, 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("SOLO 2")    PORT_CHANGED_MEMBER(DEVICE_SELF, sk1_state, sw_mode<0xfb>, 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("CHORD")     PORT_CHANGED_MEMBER(DEVICE_SELF, sk1_state, sw_mode<0xf7>, 0)
INPUT_PORTS_END


ROM_START(sk1)
	ROM_REGION(0x8000, "lsi2", 0) // µPD23C256EAC-011
	ROM_LOAD("sk1.lsi2", 0x0000, 0x8000, CRC(d615963c) SHA1(0dbf2d1c4c776f1a1c35dd2be4d6ca03882afd4c))
ROM_END

ROM_START(sk5)
	ROM_REGION(0x8000, "lsi2", 0) // µPD23C256EAC-038
	ROM_LOAD("casio_sk5.bin", 0x0000, 0x8000, CRC(1fda590b) SHA1(c77ccb5fa20275478bf271512633fa1561d6f07c))
ROM_END

ROM_START(sk10)
	ROM_REGION(0x8000, "lsi2", 0) // µPD23C256EAC-070
	ROM_LOAD("casio_sk10.bin", 0x0000, 0x8000, CRC(5945b619) SHA1(929e906bfa0fcd99a8398b37ec62d0512299065c))
ROM_END

ROM_START(sk2)
	ROM_REGION(0x8000, "lsi2", 0) // µPD23C256EAC-093
	ROM_LOAD("casio_sk2.bin", 0x0000, 0x8000, CRC(f47e421d) SHA1(50785ffd09bb2effcc9f48de466b624fd59f1317))
ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME  FLAGS
SYST( 1985, sk1,  0,      0,      sk1,     sk1,   sk1_state, empty_init, "Casio", "SK-1",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
SYST( 1987, sk5,  0,      0,      sk1,     sk1,   sk1_state, empty_init, "Casio", "SK-5",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
SYST( 1987, sk10, 0,      0,      sk1,     sk1,   sk1_state, empty_init, "Casio", "SK-10",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
SYST( 1988, sk2,  0,      0,      sk1,     sk1,   sk1_state, empty_init, "Casio", "SK-2",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
