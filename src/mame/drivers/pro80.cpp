// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, Robbbert
/******************************************************************************************************

Protec Pro-80

2009-12-09 Skeleton driver.

TODO:
- Cassette load
- Use messram for optional ram
- Fix Step command (don't works due of missing interrupt emulation)

The cassette uses 2 bits for input, plus a D flipflop and a 74LS221 oneshot, with the pulse width set
  by 0.02uf and 24k (= 336 usec). The pulse is started by a H->L transistion of the input. At the end
  of the pulse, the current input is passed through the flipflop.

******************************************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/z80pio.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"

#include "speaker.h"

#include "pro80.lh"


class pro80_state : public driver_device
{
public:
	pro80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void pro80(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_WRITE8_MEMBER(segment_w);
	DECLARE_READ8_MEMBER(kp_r);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_p);

	void pro80_io(address_map &map);
	void pro80_mem(address_map &map);

	uint8_t m_digit_sel;
	uint8_t m_cass_in;
	uint16_t m_cass_data[4];
	void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	output_finder<6> m_digits;
};

// This can read the first few bytes correctly, but after that bit slippage occurs.
// Needs to be reworked
TIMER_DEVICE_CALLBACK_MEMBER( pro80_state::timer_p )
{
	m_cass_data[1]++;
	uint8_t cass_ws = ((m_cass)->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cass_in = ((m_cass_data[1] < 0x0d) ? 0x10 : 0); // data bit
		m_cass_data[1] = 0;
	}
	if (m_cass_data[1] < 5)
		m_cass_in |= 0x20; // sync bit
}

WRITE8_MEMBER( pro80_state::digit_w )
{
	// --xx xxxx digit select
	// -x-- ---- cassette out
	// x--- ---- ???
	m_digit_sel = data & 0x3f;
	m_cass->output( BIT(data, 6) ? -1.0 : +1.0);
}

WRITE8_MEMBER( pro80_state::segment_w )
{
	if (m_digit_sel)
	{
		if (!BIT(m_digit_sel, 0)) m_digits[0] = data;
		if (!BIT(m_digit_sel, 1)) m_digits[1] = data;
		if (!BIT(m_digit_sel, 2)) m_digits[2] = data;
		if (!BIT(m_digit_sel, 3)) m_digits[3] = data;
		if (!BIT(m_digit_sel, 4)) m_digits[4] = data;
		if (!BIT(m_digit_sel, 5)) m_digits[5] = data;

		m_digit_sel = 0;
	}
}

READ8_MEMBER( pro80_state::kp_r )
{
	uint8_t data = 0x0f;

	if (!BIT(m_digit_sel, 0)) data &= ioport("LINE0")->read();
	if (!BIT(m_digit_sel, 1)) data &= ioport("LINE1")->read();
	if (!BIT(m_digit_sel, 2)) data &= ioport("LINE2")->read();
	if (!BIT(m_digit_sel, 3)) data &= ioport("LINE3")->read();
	if (!BIT(m_digit_sel, 4)) data &= ioport("LINE4")->read();
	if (!BIT(m_digit_sel, 5)) data &= ioport("LINE5")->read();

	return data | m_cass_in | 0xc0;
}

void pro80_state::pro80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).rom();
	map(0x1000, 0x13ff).ram();
	map(0x1400, 0x17ff).ram(); // 2nd RAM is optional
}

void pro80_state::pro80_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x40, 0x43).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x44, 0x47).r(FUNC(pro80_state::kp_r));
	map(0x48, 0x4b).w(FUNC(pro80_state::digit_w));
	map(0x4c, 0x4f).w(FUNC(pro80_state::segment_w));
}

/* Input ports */
static INPUT_PORTS_START( pro80 )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CR") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CW") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("SSt") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Res") PORT_CODE(KEYCODE_EQUALS)
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EXE") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("NEx") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("REx") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MEx") PORT_CODE(KEYCODE_M)
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7 [IY]") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6 [IX]") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5 [PC]") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9 [H]") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4 [SP]") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8 [L]") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
INPUT_PORTS_END

void pro80_state::machine_reset()
{
	m_digit_sel = 0;
	m_cass_in = 0;
}

void pro80_state::pro80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pro80_state::pro80_mem);
	m_maincpu->set_addrmap(AS_IO, &pro80_state::pro80_io);

	/* video hardware */
	config.set_default_layout(layout_pro80);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.05);

	/* Devices */
	CASSETTE(config, m_cass);
	Z80PIO(config, "pio", XTAL(4'000'000) / 2);
	TIMER(config, "timer_p").configure_periodic(FUNC(pro80_state::timer_p), attotime::from_hz(40000)); // cass read
}

/* ROM definition */
ROM_START( pro80 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// This rom dump is taken out of manual for this machine
	ROM_LOAD( "pro80.bin", 0x0000, 0x0400, CRC(1bf6e0a5) SHA1(eb45816337e08ed8c30b589fc24960dc98b94db2))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME  FLAGS
COMP( 1981, pro80, 0,      0,      pro80,   pro80, pro80_state, empty_init, "Protec", "Pro-80", MACHINE_NOT_WORKING )
