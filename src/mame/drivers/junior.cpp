// license:BSD-3-Clause
// copyright-holders:Robbbert
// thanks-to:Manfred Schneider
/***************************************************************************

        Elektor Junior

        17/07/2009 Skeleton driver.

This is heavily based on the KIM-1, the keycodes and operation being
identical.

Pasting:
        0-F : as is
        + (inc) : ^
        AD : -
        DA : =
        GO : X

(note: DA only works when addressing RAM)

Test Paste:
        =11^22^33^44^55^66^77^88^99^-0000
        Now press up-arrow to confirm the data has been entered.

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/mos6530n.h"
#include "machine/timer.h"
#include "junior.lh"


class junior_state : public driver_device
{
public:
	junior_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_riot(*this, "riot")
		, m_maincpu(*this, "maincpu")
		, m_line(*this, "LINE%u", 0U)
		, m_digit(*this, "digit%u", 0U)
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(junior_reset);
	void junior(machine_config &config);

private:
	DECLARE_READ8_MEMBER(junior_riot_a_r);
	DECLARE_READ8_MEMBER(junior_riot_b_r);
	DECLARE_WRITE8_MEMBER(junior_riot_a_w);
	DECLARE_WRITE8_MEMBER(junior_riot_b_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	TIMER_DEVICE_CALLBACK_MEMBER(junior_update_leds);

	void junior_mem(address_map &map);

	required_device<mos6532_new_device> m_riot;
	uint8_t m_port_a;
	uint8_t m_port_b;
	uint8_t m_led_time[6];
	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_line;
	output_finder<6> m_digit;
};




void junior_state::junior_mem(address_map &map)
{
	map.global_mask(0x1FFF);
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram(); // 1K RAM
	map(0x1a00, 0x1a7f).m(m_riot, FUNC(mos6532_new_device::ram_map));
	map(0x1a80, 0x1a9f).m(m_riot, FUNC(mos6532_new_device::io_map));
	map(0x1c00, 0x1fff).rom(); // Monitor
}


INPUT_CHANGED_MEMBER(junior_state::junior_reset)
{
	if (newval == 0)
		m_maincpu->reset();
}


/* Input ports */
static INPUT_PORTS_START( junior )
PORT_START("LINE0")         /* IN0 keys row 0 */
	PORT_BIT( 0x80, 0x00, IPT_UNUSED )
	PORT_BIT( 0x40, 0x40, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x20, 0x20, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x10, 0x10, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, 0x08, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x04, 0x04, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, 0x02, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x01, 0x01, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')

	PORT_START("LINE1")         /* IN1 keys row 1 */
	PORT_BIT( 0x80, 0x00, IPT_UNUSED )
	PORT_BIT( 0x40, 0x40, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x20, 0x20, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x10, 0x10, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, 0x08, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, 0x04, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, 0x02, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x01, 0x01, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("LINE2")         /* IN2 keys row 2 */
	PORT_BIT( 0x80, 0x00, IPT_UNUSED )
	PORT_BIT( 0x40, 0x40, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x20, 0x20, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, 0x10, IPT_KEYBOARD ) PORT_NAME("AD") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x08, 0x08, IPT_KEYBOARD ) PORT_NAME("DA") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x04, 0x04, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x02, 0x02, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x01, 0x01, IPT_KEYBOARD ) PORT_NAME("PC") PORT_CODE(KEYCODE_F6)

	PORT_START("LINE3")         /* IN3 STEP and RESET keys, MODE switch */
	PORT_BIT( 0x80, 0x00, IPT_UNUSED )
	PORT_BIT( 0x40, 0x40, IPT_KEYBOARD ) PORT_NAME("sw1: ST") PORT_CODE(KEYCODE_F7)
	PORT_BIT( 0x20, 0x20, IPT_KEYBOARD ) PORT_NAME("sw2: RST") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, junior_state, junior_reset, 0)
	PORT_DIPNAME(0x10, 0x10, "sw3: SS (NumLock)") PORT_CODE(KEYCODE_NUMLOCK) PORT_TOGGLE
	PORT_DIPSETTING( 0x00, "single step")
	PORT_DIPSETTING( 0x10, "run")
	PORT_BIT( 0x08, 0x00, IPT_UNUSED )
	PORT_BIT( 0x04, 0x00, IPT_UNUSED )
	PORT_BIT( 0x02, 0x00, IPT_UNUSED )
	PORT_BIT( 0x01, 0x00, IPT_UNUSED )
INPUT_PORTS_END



READ8_MEMBER( junior_state::junior_riot_a_r )
{
	uint8_t data = 0xff;

	uint8_t const sel = ( m_port_b >> 1) & 0x0f;
	switch ( sel )
	{
	case 0:
	case 1:
	case 2:
		data = m_line[sel]->read();
		break;
	}
	return data;

}


READ8_MEMBER( junior_state::junior_riot_b_r )
{
	if ( m_port_b & 0x20 )
		return 0xFF;

	return 0x7F;

}


WRITE8_MEMBER( junior_state::junior_riot_a_w )
{
	uint8_t idx = ( m_port_b >> 1 ) & 0x0f;

	m_port_a = data;

	if ((idx >= 4 && idx < 10) & ( m_port_a != 0xff ))
	{
		m_digit[idx - 4] = m_port_a ^ 0x7f;
		m_led_time[idx - 4] = 10;
	}
}


WRITE8_MEMBER( junior_state::junior_riot_b_w )
{
	uint8_t idx = ( data >> 1 ) & 0x0f;

	m_port_b = data;

	if ((idx >= 4 && idx < 10) & ( m_port_a != 0xff ))
	{
		m_digit[idx - 4] = m_port_a ^ 0x7f;
		m_led_time[idx - 4] = 10;
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(junior_state::junior_update_leds)
{
	int i;

	for ( i = 0; i < 6; i++ )
	{
		if ( m_led_time[i] )
			m_led_time[i]--;
		else
			m_digit[i] = 0;
	}
}


void junior_state::machine_start()
{
	m_digit.resolve();

	save_item(NAME(m_port_a));
	save_item(NAME(m_port_b));
	save_item(NAME(m_led_time));
}


void junior_state::machine_reset()
{
	int i;

	for ( i = 0; i < 6; i++ )
		m_led_time[i] = 0;
}


void junior_state::junior(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &junior_state::junior_mem);
	config.m_minimum_quantum = attotime::from_hz(50);

	/* video hardware */
	config.set_default_layout(layout_junior);

	/* Devices */
	MOS6532_NEW(config, m_riot, 1_MHz_XTAL);
	m_riot->pa_rd_callback().set(FUNC(junior_state::junior_riot_a_r));
	m_riot->pa_wr_callback().set(FUNC(junior_state::junior_riot_a_w));
	m_riot->pb_rd_callback().set(FUNC(junior_state::junior_riot_b_r));
	m_riot->pb_wr_callback().set(FUNC(junior_state::junior_riot_b_w));
	m_riot->irq_wr_callback().set_inputline(m_maincpu, M6502_IRQ_LINE);

	TIMER(config, "led_timer").configure_periodic(FUNC(junior_state::junior_update_leds), attotime::from_hz(50));
}


/* ROM definition */
ROM_START( junior )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("orig")

	ROM_SYSTEM_BIOS( 0, "orig", "Original ESS503" )
	ROMX_LOAD( "ess503.ic2", 0x1c00, 0x0400, CRC(9e804f8c) SHA1(181bdb69fb4711cb008e7966747d4775a5e3ef69), ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "mod-orig", "Mod-Original (2708)" )
	ROMX_LOAD( "junior-mod.ic2", 0x1c00, 0x0400, CRC(ee8aa69d) SHA1(a132a51603f1a841c354815e6d868b335ac84364), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 2, "2732", "Just monitor (2732)" )
	ROMX_LOAD( "junior27321a.ic2", 0x1c00, 0x0400, CRC(e22f24cc) SHA1(a6edb52a9eea5e99624c128065e748e5a3fb2e4c), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                FULLNAME           FLAGS */
COMP( 1980, junior, 0,      0,      junior,  junior, junior_state, empty_init, "Elektor Electronics", "Junior Computer", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW)
