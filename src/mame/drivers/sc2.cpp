// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Schachcomputer SC2

        Note:
         The HW is very similar to Fidelity Chess Challenger series (see fidelz80.c)

        12/05/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/beep.h"
#include "machine/z80pio.h"
#include "speaker.h"

#include "sc2.lh"

class sc2_state : public driver_device
{
public:
	sc2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_beep(*this, "beeper"),
		m_maincpu(*this, "maincpu"),
		m_digits(*this, "digit%u", 0U),
		m_leds(*this, "led%u", 0U)
	{ }

	void sc2(machine_config &config);

private:
	DECLARE_READ8_MEMBER(pio_port_a_r);
	DECLARE_READ8_MEMBER(pio_port_b_r);
	DECLARE_WRITE8_MEMBER(pio_port_a_w);
	DECLARE_WRITE8_MEMBER(pio_port_b_w);
	DECLARE_READ8_MEMBER(sc2_beep);
	uint8_t m_kp_matrix;
	uint8_t m_led_7seg_data[4];
	uint8_t m_led_selected;
	uint8_t m_digit_data;
	uint8_t m_beep_state;
	void sc2_update_display();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void sc2_io(address_map &map);
	void sc2_mem(address_map &map);

	required_device<beep_device> m_beep;
	required_device<cpu_device> m_maincpu;
	output_finder<4> m_digits;
	output_finder<2> m_leds;
};

READ8_MEMBER( sc2_state::sc2_beep )
{
	//if (!machine().side_effects_disabled())
	{
		m_beep_state = ~m_beep_state;

		m_beep->set_state(m_beep_state);
	}

	return 0xff;
}

void sc2_state::sc2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x13ff).ram();
	map(0x2000, 0x33ff).rom();
	map(0x3c00, 0x3c00).r(FUNC(sc2_state::sc2_beep));
}

void sc2_state::sc2_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).mirror(0xfc).rw("z80pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

/* Input ports */
static INPUT_PORTS_START( sc2 )
	PORT_START("LINE1")
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)

	PORT_START("LINE2")
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4)

	PORT_START("LINE3")
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8)

	PORT_START("LINE4")
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("P") PORT_CODE(KEYCODE_O)
INPUT_PORTS_END

void sc2_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	save_item(NAME(m_led_7seg_data));
	save_item(NAME(m_kp_matrix));
	save_item(NAME(m_led_selected));
	save_item(NAME(m_digit_data));
	save_item(NAME(m_beep_state));
}

void sc2_state::machine_reset()
{
	m_kp_matrix = 0;
	m_led_selected = 0;
	m_digit_data = 0;
	m_beep_state = 0;
	memset(m_led_7seg_data, 0, ARRAY_LENGTH(m_led_7seg_data));
}

void sc2_state::sc2_update_display()
{
	uint8_t digit_data = bitswap<8>( m_digit_data,7,0,1,2,3,4,5,6 ) & 0x7f;

	if (!BIT(m_led_selected, 0))
	{
		m_digits[0] = digit_data;
		m_led_7seg_data[0] = digit_data;

		m_leds[0] = BIT(m_digit_data, 7);
	}

	if (!BIT(m_led_selected, 1))
	{
		m_digits[1] = digit_data;
		m_led_7seg_data[1] = digit_data;

		m_leds[1] = BIT(m_digit_data, 7);
	}

	if (!BIT(m_led_selected, 2))
	{
		m_digits[2] = digit_data;
		m_led_7seg_data[2] = digit_data;
	}

	if (!BIT(m_led_selected, 3))
	{
		m_digits[3] = digit_data;
		m_led_7seg_data[3] = digit_data;
	}
}

READ8_MEMBER( sc2_state::pio_port_a_r )
{
	return m_digit_data;
}

READ8_MEMBER( sc2_state::pio_port_b_r )
{
	uint8_t data = m_led_selected & 0x0f;

	if (BIT(m_kp_matrix, 0))
	{
		data |= ioport("LINE1")->read();
	}

	if (BIT(m_kp_matrix, 1))
	{
		data |= ioport("LINE2")->read();
	}

	if (BIT(m_kp_matrix, 2))
	{
		data |= ioport("LINE3")->read();
	}

	if (BIT(m_kp_matrix, 3))
	{
		data |= ioport("LINE4")->read();
	}

	return data;
}

WRITE8_MEMBER( sc2_state::pio_port_a_w )
{
	m_digit_data = data;
}

WRITE8_MEMBER( sc2_state::pio_port_b_w )
{
	if (data != 0xf1 && data != 0xf2 && data != 0xf4 && data != 0xf8)
	{
		m_led_selected = data;
		sc2_update_display();
	}
	else
		m_kp_matrix = data;
}

void sc2_state::sc2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &sc2_state::sc2_mem);
	m_maincpu->set_addrmap(AS_IO, &sc2_state::sc2_io);

	/* video hardware */
	config.set_default_layout(layout_sc2);

	/* devices */
	z80pio_device& pio(Z80PIO(config, "z80pio", XTAL(4'000'000)));
	pio.in_pa_callback().set(FUNC(sc2_state::pio_port_a_r));
	pio.out_pa_callback().set(FUNC(sc2_state::pio_port_a_w));
	pio.in_pb_callback().set(FUNC(sc2_state::pio_port_b_r));
	pio.out_pb_callback().set(FUNC(sc2_state::pio_port_b_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 3250).add_route(ALL_OUTPUTS, "mono", 0.50);
}

/* ROM definition */
ROM_START( sc2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bm008.bin", 0x0000, 0x0400, CRC(3023ea82) SHA1(07020d153d802c672c39e1af3c716dbe35e23f08))
	ROM_LOAD( "bm009.bin", 0x0400, 0x0400, CRC(6a34814e) SHA1(e58ae6615297b028db135a48a8f9e186a4220f4f))
	ROM_LOAD( "bm010.bin", 0x0800, 0x0400, CRC(deab0373) SHA1(81c9a7197eef8d9131e47ecd2ec35b943caee54e))
	ROM_LOAD( "bm011.bin", 0x0c00, 0x0400, CRC(c8282339) SHA1(8d6b8861281e967a77609b6d77e80afd47d28ed2))
	ROM_LOAD( "bm012.bin", 0x2000, 0x0400, CRC(2e6a4294) SHA1(7b9bd191c9ec73139a65c3a339ab88e1f3eb5ed2))
	ROM_LOAD( "bm013.bin", 0x2400, 0x0400, CRC(3e02eb42) SHA1(2e4a9a8fd04c202c9518550d7e8cf9bfea394153))
	ROM_LOAD( "bm014.bin", 0x2800, 0x0400, CRC(538d449e) SHA1(c4186995b69e97740e01eaff84a20d49d03d180f))
	ROM_LOAD( "bm015.bin", 0x2c00, 0x0400, CRC(b4991dca) SHA1(6a6cdddf5c4afa24773acf693f58c34b99c8d328))
	ROM_LOAD( "bm016.bin", 0x3000, 0x0400, CRC(4fe0853a) SHA1(c2253e320778b0ea468fb54f26ae83d07f9700e6))
ROM_END

/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                       FULLNAME              FLAGS
COMP( 1981, sc2,  0,      0,      sc2,     sc2,   sc2_state, empty_init, "VEB Mikroelektronik Erfurt", "Schachcomputer SC2", MACHINE_SUPPORTS_SAVE )
