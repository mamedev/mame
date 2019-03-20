// license:BSD-3-Clause
// copyright-holders:Robbbert
/**********************************************************************************************

Gammatron Datum (1982)

2016-10-28.

Described in Electronics Australia magazine in 1982/1983.

- The ROM was keyed in by hand from a printout, so is therefore considered a bad dump.
- It has routines for adc, dac, cassette, but there's no hardware to support these.
- The basic machine is fully emulated as per the schematic.

Memory Map:
0000-007F = RAM inside the cpu
1000-13FF = RAM (2x 2114)
3E00-3E01 = ADC (in monitor as EQU but not further referenced) not in schematic
3F00-3F01 = 10-bit DAC (supported in monitor with routines for the user) not in schematic
4000-4001 = ACIA (in monitor for cassette usage, but schematic shows it for rs232)
            Schematic has no provision of baud clocks
5000-5003 = PIA2 for expansion, monitor does not use it at all.
6000-6003 = PIA1 for keyboard and display
7000-77FF = ROM


Sample program to paste in: Pick-up-sticks (Nim) All inputs are in HEX.
1. Paste this in
2. Enter number of sticks to begin with (2 digits)
3. Enter max number of sticks to be removed per turn (1 digit)
4. Your turn - remove some sticks
5. After a pause, player's turn.
6. Keep doing steps 4 and 5 till you lose (it says 10b3 3F)
7. When tested, the game seems buggy - prepare to lose sooner than expected

1000M
BD^71^07^CE^00^10^86^7E^A7^01^A7^05^BD^71^5E^B7^12^00^BD^72^7D^97^10^BD^71^5E^
B7^12^01^BD^72^7D^97^11^CE^12^00^BD^72^4D^B7^12^00^BD^71^5E^B7^12^02^BD^72^7D^
97^15^BD^71^5E^B7^12^03^27^08^7C^12^02^B1^12^02^2B^18^CE^00^10^86^06^97^12^
86^15^97^13^8D^61^CE^00^10^86^7F^A7^02^A7^03^20^D8^B6^12^00^B0^12^03^B7^12^00^
BD^72^6E^D7^10^97^11^8D^44^B6^12^00^4A^27^3C^7F^12^05^7C^12^02^7C^12^05^
B0^12^02^2E^F8^7A^12^05^5F^FB^12^02^7A^12^05^26^F8^B6^12^00^10^16^5A^
26^02^C6^01^B6^12^00^10^B7^12^00^BD^72^6E^D7^10^97^11^B6^12^00^81^01^27^04^
7E^10^36^3F^3F^86^80^B7^12^04^BD^71^DD^7A^12^04^26^F8^39^
Z1000G


*******************************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "datum.lh"


class datum_state : public driver_device
{
public:
	datum_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_pia1(*this, "pia1")
		, m_keyboard(*this, "X%u", 0)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void datum(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_nmi);

private:
	DECLARE_READ8_MEMBER(pa_r);
	DECLARE_WRITE8_MEMBER(pa_w);
	DECLARE_WRITE8_MEMBER(pb_w);
	void datum_mem(address_map &map);
	uint8_t m_keydata;
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<pia6821_device> m_pia1;
	required_ioport_array<4> m_keyboard;
	required_device<cpu_device> m_maincpu;
	output_finder<16> m_digits;
};


void datum_state::datum_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff); // A15 not used
	map(0x0000, 0x007f).ram(); // inside CPU
	map(0x1000, 0x13ff).mirror(0x0c00).ram(); // main ram 2x 2114
	map(0x4000, 0x4001).mirror(0x0ffe).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x5000, 0x5003).mirror(0x0ffc).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x6000, 0x6003).mirror(0x0ffc).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x7000, 0x77ff).mirror(0x0800).rom().region("roms", 0);
}


/* Input ports */
static INPUT_PORTS_START( datum )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2F") PORT_CODE(KEYCODE_S) PORT_CHAR('S') // "Second Function" i.e Shift-lock
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M/R") PORT_CODE(KEYCODE_M) PORT_CHAR('M') // "Memory / Registers"
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("I/D") PORT_CODE(KEYCODE_UP) PORT_CHAR('^') // "Increment / Decrement"
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G/S") PORT_CODE(KEYCODE_G) PORT_CHAR('G') // "Goto / Single-step"
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, datum_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC)  PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, datum_state, trigger_nmi, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( datum_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( datum_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


void datum_state::machine_reset()
{
	m_keydata = 0;
}

// read keyboard
READ8_MEMBER( datum_state::pa_r )
{
	if (m_keydata < 4)
		return m_keyboard[m_keydata]->read();

	return 0xff;
}

// write display segments
WRITE8_MEMBER( datum_state::pa_w )
{
	data ^= 0xff;
	if (m_keydata > 3)
	{
		m_digits[m_keydata] = bitswap<8>(data & 0x7f, 7, 0, 5, 6, 4, 2, 1, 3);
		m_keydata = 0;
	}

	return;
}

// select keyboard row, select a digit
WRITE8_MEMBER( datum_state::pb_w )
{
	m_keydata = bitswap<8>(data, 7, 6, 5, 4, 0, 1, 2, 3) & 15;
	return;
}


void datum_state::datum(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, XTAL(4'000'000)); // internally divided to 1 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &datum_state::datum_mem);

	/* video hardware */
	config.set_default_layout(layout_datum);

	/* Devices */
	PIA6821(config, m_pia1, 0); // keyboard & display
	m_pia1->readpa_handler().set(FUNC(datum_state::pa_r));
	m_pia1->writepa_handler().set(FUNC(datum_state::pa_w));
	m_pia1->writepb_handler().set(FUNC(datum_state::pb_w));
	m_pia1->irqa_handler().set_inputline("maincpu", M6802_IRQ_LINE);
	m_pia1->irqb_handler().set_inputline("maincpu", M6802_IRQ_LINE);

	pia6821_device &pia2(PIA6821(config, "pia2", 0)); // expansion
	pia2.irqa_handler().set_inputline("maincpu", M6802_IRQ_LINE);
	pia2.irqb_handler().set_inputline("maincpu", M6802_IRQ_LINE);

	ACIA6850(config, "acia", 0); // rs232
}


/* ROM definition */
ROM_START( datum )
	ROM_REGION( 0x800, "roms", 0 )
	ROM_LOAD( "datum.bin", 0x0000, 0x0800, BAD_DUMP CRC(6fb11628) SHA1(8a77a846b62eee0d12848da76e16b4c66ef445d8) )
ROM_END

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME  FLAGS
COMP( 1982, datum, 0,      0,      datum,   datum, datum_state, empty_init, "Gammatron", "Datum",  MACHINE_NO_SOUND_HW )
