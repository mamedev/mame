// license:BSD-3-Clause
// copyright-holders:Robbbert
// thanks-to:Manfred Schneider
/***************************************************************************

        Elektor Junior

2009-07-17 Skeleton driver.

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

NMI and IRQ:
- When NMI is activated, it jumps to the vector located at 1A7A.
- When IRQ is activated, it jumps to the vector located at 1A7E.
- The user needs to populate these vectors before use, or the system
  will crash when the debugging switches are used. Use RST to escape
  from this.


To Do:
- Single-Step switch (and associated LED) needs to be hooked up

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/mos6530.h"
#include "machine/timer.h"
#include "video/pwm.h"
#include "junior.lh"


namespace {

class junior_state : public driver_device
{
public:
	junior_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_riot(*this, "riot")
		, m_maincpu(*this, "maincpu")
		, m_display(*this, "display")
		, m_io_keyboard(*this, "LINE%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(junior_reset);
	DECLARE_INPUT_CHANGED_MEMBER(junior_stop);
	void junior(machine_config &config);

private:
	u8 riot_a_r();
	void riot_a_w(u8 data);
	void riot_b_w(u8 data);
	u8 m_digit = 0U;
	u8 m_seg = 0U;

	virtual void machine_start() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	required_device<mos6532_device> m_riot;
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_io_keyboard;
};




void junior_state::mem_map(address_map &map)
{
	map.global_mask(0x1FFF);
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram(); // 1K RAM
	map(0x1a00, 0x1a7f).m(m_riot, FUNC(mos6532_device::ram_map));
	map(0x1a80, 0x1a9f).m(m_riot, FUNC(mos6532_device::io_map));
	map(0x1c00, 0x1fff).rom().region("maincpu", 0); // Monitor
}


INPUT_CHANGED_MEMBER(junior_state::junior_reset)
{
	if (newval == 0)
		m_maincpu->reset();
}

INPUT_CHANGED_MEMBER(junior_state::junior_stop)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
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
	PORT_BIT( 0x40, 0x40, IPT_KEYBOARD ) PORT_NAME("S1: STOP") PORT_CODE(KEYCODE_F7) PORT_CHANGED_MEMBER(DEVICE_SELF, junior_state, junior_stop, 0)
	PORT_BIT( 0x20, 0x20, IPT_KEYBOARD ) PORT_NAME("S2: RST") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, junior_state, junior_reset, 0)
	PORT_DIPNAME(0x10, 0x10, "S24: SS (NumLock)") PORT_CODE(KEYCODE_NUMLOCK) PORT_TOGGLE
	PORT_DIPSETTING( 0x00, "single step")
	PORT_DIPSETTING( 0x10, "run")
	PORT_DIPNAME(0x08, 0x08, "S25: Display")
	PORT_DIPSETTING( 0x00, "Off")
	PORT_DIPSETTING( 0x08, "On")
	PORT_BIT( 0x04, 0x00, IPT_UNUSED )
	PORT_BIT( 0x02, 0x00, IPT_UNUSED )
	PORT_BIT( 0x01, 0x00, IPT_UNUSED )
INPUT_PORTS_END



u8 junior_state::riot_a_r()
{
	u8 data = 0xff;

	if (m_digit < 3)
		data &= m_io_keyboard[m_digit]->read();

	return data;

}

void junior_state::riot_a_w(u8 data)
{
	m_seg = ~data;
	if (BIT(m_io_keyboard[3]->read(), 3))
		m_display->matrix(1 << m_digit, m_seg);
	else
		m_display->matrix(1 << m_digit, 0);
}


void junior_state::riot_b_w(u8 data)
{
	m_digit = (data >> 1) & 15;
	if (BIT(m_io_keyboard[3]->read(), 3))
		m_display->matrix(1 << m_digit, m_seg);
	else
		m_display->matrix(1 << m_digit, 0);
}


void junior_state::machine_start()
{
	save_item(NAME(m_digit));
	save_item(NAME(m_seg));
}


void junior_state::junior(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &junior_state::mem_map);

	/* video hardware */
	config.set_default_layout(layout_junior);
	PWM_DISPLAY(config, m_display).set_size(10, 7);
	m_display->set_segmask(0x3f0, 0x7f);

	/* Devices */
	MOS6532(config, m_riot, 1_MHz_XTAL);
	m_riot->pa_rd_callback().set(FUNC(junior_state::riot_a_r));
	m_riot->pa_wr_callback().set(FUNC(junior_state::riot_a_w));
	m_riot->pb_wr_callback().set(FUNC(junior_state::riot_b_w));
	m_riot->irq_wr_callback().set_inputline(m_maincpu, M6502_IRQ_LINE);
}


/* ROM definition */
ROM_START( junior )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_DEFAULT_BIOS("orig")

	ROM_SYSTEM_BIOS( 0, "orig", "Original ESS503" )
	ROMX_LOAD( "ess503.ic2", 0x0000, 0x0400, CRC(9e804f8c) SHA1(181bdb69fb4711cb008e7966747d4775a5e3ef69), ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "mod-orig", "Mod-Original (2708)" )
	ROMX_LOAD( "junior-mod.ic2", 0x0000, 0x0400, CRC(ee8aa69d) SHA1(a132a51603f1a841c354815e6d868b335ac84364), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 2, "2732", "Just monitor (2732)" )
	ROMX_LOAD( "junior27321a.ic2", 0x0000, 0x0400, CRC(e22f24cc) SHA1(a6edb52a9eea5e99624c128065e748e5a3fb2e4c), ROM_BIOS(2))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY    FULLNAME           FLAGS */
COMP( 1980, junior, 0,      0,      junior,  junior, junior_state, empty_init, "Elektor", "Junior Computer", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
