// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Tesla PMI-80

        12/05/2009 Skeleton driver.
        20/06/2011 Made mostly working [Robbbert]


ToDo:
- cassette
- keyboard to be finished
- artwork


Notes:
- Keyboard consists of 16 black hex keys, and 9 blue function keys
- The hex keys are 0 through 9, A through F on our keyboard
- The function keys are mostly not worked out although I've guessed a few
- The function key labels are RE, I, EX, R, BR, M, L, S, =
- The letter M shows as an inverted U in the display
- Turn it on, it says ''PMI -80''
- Press any key, it shows a ? at the left
- Press the function key corresponding to what you want to do
- Press the numbers to select an address or whatever
- For example, press M then enter an address, press =, enter data,
   press =  to increment to next address or to scan through them.
- As for the rest, no idea
- In the layout, there is a 10th digit. It is disabled but if you
   enable it (by renaming 'digitx' to 'digit9'), it will display
   things. Perhaps it connected to discrete LEDs or output lines.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "pmi80.lh"

class pmi80_state : public driver_device
{
public:
	pmi80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ledready(0)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void pmi80(machine_config &config);

private:
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_WRITE8_MEMBER(leds_w);
	void pmi80_io(address_map &map);
	void pmi80_mem(address_map &map);

	uint8_t m_keyrow;
	bool m_ledready;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	required_device<cpu_device> m_maincpu;
	output_finder<10> m_digits;
};


READ8_MEMBER( pmi80_state::keyboard_r)
{
	char kbdrow[6];
	sprintf(kbdrow,"%X",m_keyrow);
	return ioport(kbdrow)->read();
}

WRITE8_MEMBER( pmi80_state::keyboard_w )
{
	m_keyrow = data;
	m_ledready = true;
}

WRITE8_MEMBER( pmi80_state::leds_w )
{
	if (m_ledready)
	{
		m_ledready = false;
		if (m_keyrow > 0xF5)  // to be sure to be sure
			m_digits[m_keyrow^0xff] = data^0xff;
	}
}

void pmi80_state::pmi80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).rom();
	map(0x0400, 0x1fff).ram();
}

void pmi80_state::pmi80_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf8, 0xf8).w(FUNC(pmi80_state::leds_w));
	map(0xfa, 0xfa).rw(FUNC(pmi80_state::keyboard_r), FUNC(pmi80_state::keyboard_w));
}

/* Input ports */
static INPUT_PORTS_START( pmi80 )
	PORT_START("F6")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED) // does nothing
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_Q) // unknown
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) // same as '2'
	PORT_BIT(0x8F, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("F7")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x8F, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("F8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_W) // unknown
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x8F, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("F9")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_R) // unknown
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B BC") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 HL") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x8F, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("FA")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) // memory mode
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x8F, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("FB")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BR") PORT_CODE(KEYCODE_B) // b mode?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D DE") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x8F, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("FC")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_T) // unknown
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_Y) // unknown
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EX") PORT_CODE(KEYCODE_G) // Go?
	PORT_BIT(0x8F, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("FD")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) // Load tape?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A AF") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 SP") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x8F, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("FE")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) // Save tape?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x8F, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("FF")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_U) // unknown
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x8F, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void pmi80_state::machine_reset()
{
}

void pmi80_state::machine_start()
{
	m_digits.resolve();
}


void pmi80_state::pmi80(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, XTAL(1'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &pmi80_state::pmi80_mem);
	m_maincpu->set_addrmap(AS_IO, &pmi80_state::pmi80_io);


	/* video hardware */
	config.set_default_layout(layout_pmi80);
}

/* ROM definition */
ROM_START( pmi80 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pmi80_monitor.rom", 0x0000, 0x0400, CRC(b93f4407) SHA1(43153441070ed0572f33d2815635eb7bae878e38))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
COMP( 1982, pmi80, 0,      0,      pmi80,   pmi80, pmi80_state, empty_init, "Tesla", "PMI-80", MACHINE_NO_SOUND_HW)
