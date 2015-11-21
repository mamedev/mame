// license:???
// copyright-holders:Robbbert, Manfred Schneider
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
#include "junior.lh"


class junior_state : public driver_device
{
public:
	junior_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_riot(*this, "riot")
	,
		m_maincpu(*this, "maincpu") { }

	required_device<mos6532_t> m_riot;
	DECLARE_READ8_MEMBER(junior_riot_a_r);
	DECLARE_READ8_MEMBER(junior_riot_b_r);
	DECLARE_WRITE8_MEMBER(junior_riot_a_w);
	DECLARE_WRITE8_MEMBER(junior_riot_b_w);
	DECLARE_WRITE_LINE_MEMBER(junior_riot_irq);
	UINT8 m_port_a;
	UINT8 m_port_b;
	UINT8 m_led_time[6];
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_INPUT_CHANGED_MEMBER(junior_reset);
	TIMER_DEVICE_CALLBACK_MEMBER(junior_update_leds);
	required_device<cpu_device> m_maincpu;
};




static ADDRESS_MAP_START(junior_mem, AS_PROGRAM, 8, junior_state)
	ADDRESS_MAP_GLOBAL_MASK(0x1FFF)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_RAM // 1K RAM
	AM_RANGE(0x1a00, 0x1a7f) AM_DEVICE("riot", mos6532_t, ram_map)
	AM_RANGE(0x1a80, 0x1a9f) AM_DEVICE("riot", mos6532_t, io_map)
	AM_RANGE(0x1c00, 0x1fff) AM_ROM // Monitor
ADDRESS_MAP_END


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
	PORT_BIT( 0x20, 0x20, IPT_KEYBOARD ) PORT_NAME("sw2: RST") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, junior_state, junior_reset, NULL)
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
	UINT8 data = 0xff;

	switch( ( m_port_b >> 1 ) & 0x0f )
	{
	case 0:
		data = ioport("LINE0")->read();
		break;
	case 1:
		data = ioport("LINE1")->read();
		break;
	case 2:
		data = ioport("LINE2")->read();
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
	UINT8 idx = ( m_port_b >> 1 ) & 0x0f;

	m_port_a = data;

	if ((idx >= 4 && idx < 10) & ( m_port_a != 0xff ))
	{
		output_set_digit_value( idx-4, m_port_a ^ 0x7f );
		m_led_time[idx - 4] = 10;
	}
}


WRITE8_MEMBER( junior_state::junior_riot_b_w )
{
	UINT8 idx = ( data >> 1 ) & 0x0f;

	m_port_b = data;

	if ((idx >= 4 && idx < 10) & ( m_port_a != 0xff ))
	{
		output_set_digit_value( idx-4, m_port_a ^ 0x7f );
		m_led_time[idx - 4] = 10;
	}
}


WRITE_LINE_MEMBER( junior_state::junior_riot_irq )
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, state ? HOLD_LINE : CLEAR_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(junior_state::junior_update_leds)
{
	int i;

	for ( i = 0; i < 6; i++ )
	{
		if ( m_led_time[i] )
			m_led_time[i]--;
		else
			output_set_digit_value( i, 0 );
	}
}


void junior_state::machine_start()
{
	save_item(NAME(m_port_a));
	save_item(NAME(m_port_b));
}


void junior_state::machine_reset()
{
	int i;

	for ( i = 0; i < 6; i++ )
		m_led_time[i] = 0;
}


static MACHINE_CONFIG_START( junior, junior_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(junior_mem)
	MCFG_QUANTUM_TIME(attotime::from_hz(50))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_junior )

	/* Devices */
	MCFG_DEVICE_ADD("riot", MOS6532n, XTAL_1MHz)
	MCFG_MOS6530n_IN_PA_CB(READ8(junior_state, junior_riot_a_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(junior_state, junior_riot_a_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(junior_state, junior_riot_b_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(junior_state, junior_riot_b_w))
	MCFG_MOS6530n_IRQ_CB(WRITELINE(junior_state, junior_riot_irq))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("led_timer", junior_state, junior_update_leds, attotime::from_hz(50))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( junior )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("orig")

	ROM_SYSTEM_BIOS( 0, "orig", "Original ESS503" )
	ROMX_LOAD( "ess503.ic2", 0x1c00, 0x0400, CRC(9e804f8c) SHA1(181bdb69fb4711cb008e7966747d4775a5e3ef69), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 1, "mod-orig", "Mod-Original (2708)" )
	ROMX_LOAD( "junior-mod.ic2", 0x1c00, 0x0400, CRC(ee8aa69d) SHA1(a132a51603f1a841c354815e6d868b335ac84364), ROM_BIOS(2))

	ROM_SYSTEM_BIOS( 2, "2732", "Just monitor (2732)" )
	ROMX_LOAD( "junior27321a.ic2", 0x1c00, 0x0400, CRC(e22f24cc) SHA1(a6edb52a9eea5e99624c128065e748e5a3fb2e4c), ROM_BIOS(3))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY                     FULLNAME       FLAGS */
COMP( 1980, junior, 0,      0,       junior,    junior, driver_device,   0,     "Elektor Electronics", "Junior Computer", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW)
