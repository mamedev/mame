// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Curt Coder
/***************************************************************************

    A.S.EL. Amico 2000

    07/2009 Skeleton driver.

    IC9 - Monitor PROM, handwritten from book listing by Davide
    IC10 - Recorder PROM, yet to be found
    IC6/IC7 - PROMs reconstructed by Luigi Serrantoni

    To Do:
     * Basically everything, in particular implement PROM (described in details
       at the link below) and i8255

    http://www.computerhistory.it/index.php?option=com_content&task=view&id=85&Itemid=117

    Pasting:
        0-F : as is
        ^ (inc) : ^
        AD : -
        DA : =
        GO : X

    Test Paste:
        =11^22^33^44^55^66^77^88^99^-0000
        Now press up-arrow to confirm the data has been entered.

****************************************************************************/


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/i8255.h"
#include "utf8.h"

#include "amico2k.lh"


namespace {

class amico2k_state : public driver_device
{
public:
	amico2k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void amico2k(machine_config &config);

private:
	void machine_start() override ATTR_COLD;

	uint8_t ppi_pa_r();
	void ppi_pa_w(uint8_t data);
	uint8_t ppi_pb_r();
	void ppi_pb_w(uint8_t data);

	// timers
	emu_timer *m_led_refresh_timer = nullptr;
	TIMER_CALLBACK_MEMBER(led_refresh);
	void amico2k_mem(address_map &map) ATTR_COLD;

	int m_ls145_p = 0;
	uint8_t m_segment = 0U;
	required_device<cpu_device> m_maincpu;
	output_finder<6> m_digits;
};


void amico2k_state::amico2k_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram();
//  map(0x0400, 0x07ff).ram(); // optional expansion RAM
	map(0xfb00, 0xfcff).rom();
	map(0xfd00, 0xfd03).rw("i8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xfe00, 0xffff).rom();
}

/* Input ports */
static INPUT_PORTS_START( amico2k )
	PORT_START("Q0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Q1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Q2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PC") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DA") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("AD") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(amico2k_state::led_refresh)
{
	if (m_ls145_p > 3)
	{
		m_digits[m_ls145_p - 4] = m_segment;
	}
}

uint8_t amico2k_state::ppi_pa_r()
{
	/*

	    bit     description

	    PA0     keyboard data 0
	    PA1     keyboard data 1
	    PA2     keyboard data 2
	    PA3     keyboard data 3
	    PA4     keyboard data 4
	    PA5     keyboard data 5
	    PA6     keyboard data 6
	    PA7     reg out

	*/

	switch (m_ls145_p)
	{
	case 0:     return ioport("Q0")->read();
	case 1:     return ioport("Q1")->read();
	case 2:     return ioport("Q2")->read();
	default:    return 0xff;
	}
}

void amico2k_state::ppi_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     LED segment A
	    PA1     LED segment B
	    PA2     LED segment C
	    PA3     LED segment D
	    PA4     LED segment E
	    PA5     LED segment F
	    PA6     LED segment G
	    PA7

	*/

	m_segment = data;
	m_led_refresh_timer->adjust(attotime::from_usec(70));
}

uint8_t amico2k_state::ppi_pb_r()
{
	/*

	    bit     description

	    PB0     reg out
	    PB1
	    PB2
	    PB3
	    PB4
	    PB5
	    PB6
	    PB7

	*/

	return 0;
}

void amico2k_state::ppi_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0
	    PB1     LS145 P0
	    PB2     LS145 P1
	    PB3     LS145 P2
	    PB4     LS145 P3
	    PB5     reg in
	    PB6     reg in
	    PB7     led output enable

	*/

	m_ls145_p = (data >> 1) & 0x0f;
}

void amico2k_state::machine_start()
{
	m_digits.resolve();
	m_led_refresh_timer = timer_alloc(FUNC(amico2k_state::led_refresh), this);

	// state saving
	save_item(NAME(m_ls145_p));
	save_item(NAME(m_segment));
}

void amico2k_state::amico2k(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1000000); /* 1MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &amico2k_state::amico2k_mem);

	/* video hardware */
	config.set_default_layout(layout_amico2k);

	i8255_device &ppi(I8255(config, "i8255"));
	ppi.in_pa_callback().set(FUNC(amico2k_state::ppi_pa_r));
	ppi.out_pa_callback().set(FUNC(amico2k_state::ppi_pa_w));
	ppi.in_pb_callback().set(FUNC(amico2k_state::ppi_pb_r));
	ppi.out_pb_callback().set(FUNC(amico2k_state::ppi_pb_w));
}


/* ROM definition */
// not sure the ROMs are loaded correctly
ROM_START( amico2k )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "prom.ic10", 0xfb00, 0x200, NO_DUMP )     // cassette recorder ROM, not published anywhere. a board is needed!
	ROM_LOAD( "prom.ic9",  0xfe00, 0x200, CRC(86449f7c) SHA1(fe7deca86e90ab89aae23f11e9dbaf343b4242dc) )

	ROM_REGION( 0x200, "proms", ROMREGION_ERASEFF )
	ROM_LOAD( "prom.ic6",  0x000, 0x100, CRC(4005f760) SHA1(7edcd85feb5a576f6da1bbb723b3cf668cf3df45) )
	ROM_LOAD( "prom.ic7",  0x100, 0x100, CRC(8785d864) SHA1(d169c3b5f5690664083030948db9f33571b08656) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY     FULLNAME      FLAGS
COMP( 1978, amico2k, 0,      0,      amico2k, amico2k, amico2k_state, empty_init, "A.S.E.L.", "Amico 2000", MACHINE_NO_SOUND_HW)
