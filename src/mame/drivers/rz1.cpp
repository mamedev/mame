// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Casio RZ-1

    Sampling drum machine

	Skeleton driver

	Sound ROM info:

	Each ROM chip holds 1.49s of sounds and can be opened as raw
	PCM data: signed 8-bit, mono, 20,000 Hz.

	* Sound A: Toms 1~3, Kick, Snare, Rimshot, Closed Hi-Hat, Open Hi-Hat,
	    and Metronome Click (in that order).

	* Sound B: Clap, Ride, Cowbell, and Crash (in that order).

***************************************************************************/

#include "emu.h"
#include "cpu/upd7810/upd7811.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class rz1_state : public driver_device
{
public:
	rz1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_keys(*this, "kc%u", 0), m_key_select(0)
	{ }

	void rz1(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<upd7811_device> m_maincpu;
	required_ioport_array<8> m_keys;

	void map(address_map &map);

	DECLARE_WRITE8_MEMBER(port_a_w);
	DECLARE_WRITE8_MEMBER(port_b_w);
	DECLARE_READ8_MEMBER(port_c_r);
	DECLARE_WRITE8_MEMBER(port_c_w);
	DECLARE_READ8_MEMBER(port_d_r);

	uint8_t m_key_select;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

ADDRESS_MAP_START( rz1_state::map )
//	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x2000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_ROM AM_REGION("program", 0)
ADDRESS_MAP_END


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( rz1 )
	PORT_START("kc0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COWBELL")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLAPS")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OPEN HH")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIM")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TOM3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TOM1")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CRASH")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIDE")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLOSED HH")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S D")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B D")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TOM2")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ACCENT")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MUTE")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SAMPLE 4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SAMPLE 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SAMPLE 2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SAMPLE 1")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RESET/COPY")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TEMPO \xe2\x96\xbd")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TEMPO \xe2\x96\xb3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SAMPLING")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CONTINUE START")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("START/STOP")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI CLOCK")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI CH")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MT LOAD")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MT SAVE")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 (1/16)")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 (1/12)")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 (1/8)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 (1/6)")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 (1/4)")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 (1/2)")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xe2\x96\xbd (NO)")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xe2\x96\xb3 (YES)")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 (1/96)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (1/48)")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 (1/32)")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 (1/24)")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

WRITE8_MEMBER( rz1_state::port_a_w )
{
	logerror("port_a_w: %02x\n", data);
	m_key_select = data & 0x07;
}

WRITE8_MEMBER( rz1_state::port_b_w )
{
	logerror("port_b_w: %02x\n", data);
}

READ8_MEMBER( rz1_state::port_c_r )
{
	return 0xff;
}

WRITE8_MEMBER( rz1_state::port_c_w )
{
	logerror("port_c_w: %02x\n", data);
}

READ8_MEMBER( rz1_state::port_d_r )
{
	return m_keys[m_key_select]->read();
}

void rz1_state::machine_start()
{
	// register for save states
	save_item(NAME(m_key_select));
}

void rz1_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

MACHINE_CONFIG_START( rz1_state::rz1 )
	MCFG_CPU_ADD("maincpu", UPD7811, 12_MHz_XTAL)
	MCFG_CPU_PROGRAM_MAP(map)
	MCFG_UPD7810_PORTA_WRITE_CB(WRITE8(rz1_state, port_a_w))
	MCFG_UPD7810_PORTB_WRITE_CB(WRITE8(rz1_state, port_b_w))
	MCFG_UPD7810_PORTC_READ_CB(READ8(rz1_state, port_c_r))
	MCFG_UPD7810_PORTC_WRITE_CB(WRITE8(rz1_state, port_c_w))
	MCFG_UPD7810_PORTD_READ_CB(READ8(rz1_state, port_d_r))
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( rz1 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("upd7811.bin", 0x0000, 0x1000, CRC(597ac04a) SHA1(96451a764296eaa22aaad3cba121226dcba865f4))

	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("program.bin", 0x0000, 0x4000, CRC(b44b2652) SHA1(b77f8daece9adb177b6ce1ef518fc3238b8c0a9c))

	ROM_REGION(0x10000, "samples", 0)
	ROM_LOAD("sound_a.cm5", 0x0000, 0x8000, CRC(c643ff24) SHA1(e886314d22a9a5473bfa2cb237ecafcf0daedfc1)) // HN613256P
	ROM_LOAD("sound_b.cm6", 0x8000, 0x8000, CRC(ee5b703e) SHA1(cbf2e92c68901f236678d704e9e695a5c84ff49e)) // HN613256P
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  MACHINE  INPUT  CLASS      INIT  ROTATION  COMPANY  FULLNAME  FLAGS
GAME( 1986, rz1,  0,      rz1,     rz1,   rz1_state, 0,    ROT0,    "Casio",  "RZ-1",   MACHINE_IS_SKELETON )
