// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, Robbbert
/******************************************************************************************************

Protec Pro-80

2009-12-09 Skeleton driver.

TODO:
- Cassette load (last byte is bad)
- Use messram for optional ram
- Fix Step command (doesn't work due to missing interrupt emulation)

The cassette uses 2 bits for input, plus a D flipflop and a 74LS221 oneshot, with the pulse width set
  by 0.02uf and 24k (= 336 usec). The pulse is started by a H->L transistion of the input. At the end
  of the pulse, the current input is passed through the flipflop.

Cassette start address is always 1000. The end address must be entered into 13DC/DD (little-endian).
Then press W to save. To load, press L. If it says r at the end, it indicates a bad checksum.

******************************************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/z80pio.h"
#include "imagedev/cassette.h"
#include "speaker.h"
#include "video/pwm.h"

#include "pro80.lh"


namespace {

class pro80_state : public driver_device
{
public:
	pro80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_io_keyboard(*this, "LINE%u", 0U)
		, m_display(*this, "display")
	{ }

	void pro80(machine_config &config);

private:
	void digit_w(u8 data);
	void segment_w(u8 data);
	u8 kp_r();
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_digit_sel = 0U;
	u8 m_cass_in = 0U;
	u16 m_cass_data[4]{};
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_ioport_array<6> m_io_keyboard;
	required_device<pwm_display_device> m_display;
};

TIMER_DEVICE_CALLBACK_MEMBER( pro80_state::kansas_r )
{
	m_cass_data[1]++;
	u8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cass_in = ((m_cass_data[1] < 12) ? 0x10 : 0); // get data bit
		m_cass_data[1] = 0;
		if (!cass_ws) m_cass_in |= 0x20;  // set sync bit
	}
}

void pro80_state::digit_w(u8 data)
{
	// --xx xxxx digit select
	// -x-- ---- cassette out
	// x--- ---- ???
	m_digit_sel = data & 0x3f;
	m_cass->output( BIT(data, 6) ? -1.0 : +1.0);
}

void pro80_state::segment_w(u8 data)
{
	if (m_digit_sel)
	{
		for (u8 i = 0; i < 6; i++)
			if (!BIT(m_digit_sel, i))
				m_display->matrix(1<<i, data);

		m_digit_sel = 0;
	}
}

u8 pro80_state::kp_r()
{
	u8 data = 0x0f;

	for (u8 i = 0; i < 6; i++)
		if (!BIT(m_digit_sel, i))
			data &= m_io_keyboard[i]->read();

	data |= m_cass_in;
	data |= 0xc0;
	m_cass_in = 0;
	return data;
}

void pro80_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).rom();
	map(0x1000, 0x13ff).ram();
	map(0x1400, 0x17ff).ram(); // 2nd RAM is optional
}

void pro80_state::io_map(address_map &map)
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
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("NEx") PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_UP)
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

void pro80_state::machine_start()
{
	save_item(NAME(m_digit_sel));
	save_item(NAME(m_cass_in));
	save_item(NAME(m_cass_data));
}

void pro80_state::pro80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pro80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pro80_state::io_map);

	/* video hardware */
	config.set_default_layout(layout_pro80);
	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(0x3f, 0xff);

	Z80PIO(config, "pio", XTAL(4'000'000) / 2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	// Cassette
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_r").configure_periodic(FUNC(pro80_state::kansas_r), attotime::from_hz(40000)); // cass read
}

/* ROM definition */
ROM_START( pro80 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	// This rom dump is taken out of manual for this machine
	ROM_LOAD( "pro80.bin", 0x0000, 0x0400, CRC(1bf6e0a5) SHA1(eb45816337e08ed8c30b589fc24960dc98b94db2))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME  FLAGS
COMP( 1981, pro80, 0,      0,      pro80,   pro80, pro80_state, empty_init, "Protec", "Pro-80", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
