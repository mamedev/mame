// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

Schachcomputer SC1

2009-05-12 Skeleton driver.

ToDo:
- speaker
- LEDs
- 7seg sometimes flashes


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "sc1.lh"


class sc1_state : public driver_device
{
public:
	sc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void sc1(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER( matrix_w );
	DECLARE_WRITE8_MEMBER( pio_port_a_w );
	DECLARE_READ8_MEMBER( pio_port_b_r );

	void sc1_io(address_map &map);
	void sc1_mem(address_map &map);

	uint8_t m_matrix;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	output_finder<4> m_digits;
};

/***************************************************************************

    Display

***************************************************************************/

WRITE8_MEMBER( sc1_state::pio_port_a_w )
{
	uint8_t digit = bitswap<8>( data,3,4,6,0,1,2,7,5 ) & 0x7f;

	if (m_matrix & 0x04)
		m_digits[3] = digit;
	if (m_matrix & 0x08)
		m_digits[2] = digit;
	if (m_matrix & 0x10)
		m_digits[1] = digit;
	if (m_matrix & 0x20)
		m_digits[0] = digit;
}


/***************************************************************************

    Keyboard

***************************************************************************/

WRITE8_MEMBER( sc1_state::matrix_w )
{
	m_matrix = data;
}

READ8_MEMBER( sc1_state::pio_port_b_r )
{
	uint8_t data = 0;

	if (m_matrix & 0x01)
		data |= ioport("LINE1")->read();
	if (m_matrix & 0x02)
		data |= ioport("LINE2")->read();
	if (m_matrix & 0x04)
		data |= ioport("LINE3")->read();
	if (m_matrix & 0x08)
		data |= ioport("LINE4")->read();
	if (m_matrix & 0x10)
		data |= ioport("LINE5")->read();
	if (m_matrix & 0x20)
		data |= ioport("LINE6")->read();
	if (m_matrix & 0x40)
		data |= ioport("LINE7")->read();
	if (m_matrix & 0x80)
		data |= ioport("LINE8")->read();

	return data;
}


void sc1_state::sc1_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x4000, 0x43ff).ram();
}

void sc1_state::sc1_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).rw("z80pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0xfc, 0xfc).w(FUNC(sc1_state::matrix_w));
}

/* Input ports */
static INPUT_PORTS_START( sc1 )
	PORT_START("LINE1")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_D)

	PORT_START("LINE2")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_B)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F)

	PORT_START("LINE3")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_C)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_G)

	PORT_START("LINE4")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_A)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_E)

	PORT_START("LINE5")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_H)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CE") PORT_CODE(KEYCODE_DEL)

	PORT_START("LINE6")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER)

	PORT_START("LINE7")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("P") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("W/S") PORT_CODE(KEYCODE_W)

	PORT_START("LINE8")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C") PORT_CODE(KEYCODE_M)
INPUT_PORTS_END


void sc1_state::sc1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &sc1_state::sc1_mem);
	m_maincpu->set_addrmap(AS_IO, &sc1_state::sc1_io);

	/* video hardware */
	config.set_default_layout(layout_sc1);

	/* devices */
	z80pio_device& pio(Z80PIO(config, "z80pio", XTAL(4'000'000)));
	pio.out_pa_callback().set(FUNC(sc1_state::pio_port_a_w));
	pio.in_pb_callback().set(FUNC(sc1_state::pio_port_b_r));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

/* ROM definition */
ROM_START( sc1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sc1.rom", 0x0000, 0x1000, CRC(26965b23) SHA1(01568911446eda9f05ec136df53da147b7c6f2bf))
ROM_END

/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                       FULLNAME              FLAGS
COMP( 1989, sc1,  0,      0,      sc1,     sc1,   sc1_state, empty_init, "VEB Mikroelektronik Erfurt", "Schachcomputer SC1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
