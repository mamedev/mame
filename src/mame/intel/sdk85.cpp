// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/**************************************************************************************************************************************

        Intel MCS-85 System Design Kit (SDK-85)

        09/12/2009 Skeleton driver.

        22/06/2011 Working [Robbbert]

This is an evaluation kit for the 8085 cpu.

All onboard RAM and (P)ROM is contained within address-latched Intel memories with built-in I/O
(8155 and 8355/8755/8755A).

There is no speaker or storage facility in the standard kit.

Download the User Manual to get the operating procedures.

An example is Press SUBST key, enter an address, press NEXT key, enter data,
then press NEXT to increment the address.

Warning: the default cold start routine fails to initialize SP properly, which will cause the GO
command to fail. SP can be set to point to the end of onboard RAM by the following sequence of
button presses: EXAM REG, 4, 2, 0, EXEC, EXAM REG, 5, F, F, EXEC. In TTY mode, use the "XS"
command to change SP. Another option is to push the RESET button again, which will leave SP
wherever it was in the monitor's scratchpad area.

ToDo:
- Artwork


Notes on Mastermind bios.
Original author: Paolo Forlani
Ported to SDK85 by: Stefano Bodrato

Game instructions (from Stefano):
Start up, press 0 to begin. Game shows ----. You need to guess a number between 0000 and 9999.
Enter your guess, "computer" will answer showing on left with the number of found digits.
On the right you'll get a "clue", slightly different than on the standard game (making it a bit more tricky and more intriguing).
Once you find the number, you'll see it flashing.   Press the 2 key and you'll get your score (number of attempts before guessing).
Press 0 to restart.

*************************************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/sdk85/memexp.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "machine/i8355.h"
#include "machine/i8279.h"
#include "softlist_dev.h"
#include "sdk85.lh"


namespace {

class sdk85_state : public driver_device
{
public:
	sdk85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kdc(*this, "kdc")
		, m_romio(*this, "romio")
		, m_expromio(*this, "expromio")
		, m_ramio(*this, "ramio")
		, m_expramio(*this, "expramio")
		, m_tty(*this, "tty")
		, m_keyboard(*this, "X%u", 0U)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void sdk85(machine_config &config);

	void reset_w(int state);
	void vect_intr_w(int state);

private:
	int sid_r();

	void scanlines_w(u8 data);
	void digit_w(u8 data);
	u8 kbd_r();

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_digit = 0U;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8279_device> m_kdc;
	required_device<i8355_device> m_romio;
	required_device<sdk85_romexp_device> m_expromio;
	required_device<i8155_device> m_ramio;
	required_device<i8155_device> m_expramio;
	required_device<rs232_port_device> m_tty;
	required_ioport_array<3> m_keyboard;
	output_finder<6> m_digits;
};

void sdk85_state::machine_reset()
{
	// Prevent spurious TRAP when system is reset
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void sdk85_state::machine_start()
{
	m_digits.resolve();
	save_item(NAME(m_digit));
}

void sdk85_state::reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
	if (!state)
	{
		m_kdc->reset();
		m_romio->reset();
		m_expromio->reset();
		m_ramio->reset();
		m_expramio->reset();
	}
}

void sdk85_state::vect_intr_w(int state)
{
	m_maincpu->set_input_line(I8085_RST75_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

int sdk85_state::sid_r()
{
	// Actual HW has S25 switch to ground SID when using keyboard input instead of TTY RX
	if (m_tty->get_card_device() == nullptr)
		return 0;
	else
		return m_tty->rxd_r();
}

void sdk85_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).r(m_romio, FUNC(i8355_device::memory_r));
	map(0x0800, 0x0fff).rw(m_expromio, FUNC(sdk85_romexp_device::memory_r), FUNC(sdk85_romexp_device::memory_w));
	map(0x1800, 0x1800).mirror(0x06ff).rw(m_kdc, FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));
	map(0x1900, 0x1900).mirror(0x06ff).rw(m_kdc, FUNC(i8279_device::status_r), FUNC(i8279_device::cmd_w));
	map(0x2000, 0x20ff).mirror(0x0700).rw(m_ramio, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x2800, 0x28ff).mirror(0x0700).rw(m_expramio, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
}

void sdk85_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x03).mirror(0x04).rw(m_romio, FUNC(i8355_device::io_r), FUNC(i8355_device::io_w));
	map(0x08, 0x0b).mirror(0x04).rw(m_expromio, FUNC(sdk85_romexp_device::io_r), FUNC(sdk85_romexp_device::io_w));
	map(0x20, 0x27).rw(m_ramio, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x28, 0x2f).rw(m_expramio, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}

/* Input ports */
static INPUT_PORTS_START( sdk85 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("0")      PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1")      PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2")      PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3  I")   PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4  SPH") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5  SPL") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6  PCH") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7  PCL") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8  H") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9  L") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A")    PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B")    PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C")    PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D")    PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("E")    PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("F")    PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EXEC  .")     PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("NEXT  ,")     PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("GO")          PORT_CODE(KEYCODE_G)     PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("SUBST MEM")   PORT_CODE(KEYCODE_S)     PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EXAM REG")    PORT_CODE(KEYCODE_X)     PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("SINGLE STEP") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INTR") // buttons hardwired to 8085 inputs
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("VECT INTR") PORT_WRITE_LINE_MEMBER(sdk85_state, vect_intr_w) PORT_CODE(KEYCODE_F2)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("RESET")     PORT_WRITE_LINE_MEMBER(sdk85_state, reset_w)     PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END


void sdk85_state::scanlines_w(u8 data)
{
	m_digit = data & 7;
}

void sdk85_state::digit_w(u8 data)
{
	if (m_digit < 6)
		m_digits[m_digit] = bitswap<8>(~data, 3, 2, 1, 0, 7, 6, 5, 4);
}

u8 sdk85_state::kbd_r()
{
	u8 data = (m_digit < 3) ? m_keyboard[m_digit]->read() : 0xff;
	return data;
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_110 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_110 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void sdk85_state::sdk85(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sdk85_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sdk85_state::io_map);
	m_maincpu->in_sid_func().set(FUNC(sdk85_state::sid_r));
	m_maincpu->out_sod_func().set(m_tty, FUNC(rs232_port_device::write_txd)).invert();

	I8355(config, m_romio, 6.144_MHz_XTAL / 2); // Monitor ROM (A14)

	SDK85_ROMEXP(config, m_expromio, 6.144_MHz_XTAL / 2, sdk85_romexp_device::rom_options, nullptr); // Expansion ROM (A15)

	I8155(config, m_ramio, 6.144_MHz_XTAL / 2); // Basic RAM (A16)
	m_ramio->out_to_callback().set_inputline(m_maincpu, I8085_TRAP_LINE);

	I8155(config, m_expramio, 6.144_MHz_XTAL / 2); // Expansion RAM (A17)

	/* video hardware */
	config.set_default_layout(layout_sdk85);

	/* Devices */
	I8279(config, m_kdc, 6.144_MHz_XTAL / 2);                               // Keyboard/Display Controller (A13)
	m_kdc->out_irq_callback().set_inputline(m_maincpu, I8085_RST55_LINE);   // irq
	m_kdc->out_sl_callback().set(FUNC(sdk85_state::scanlines_w));           // scan SL lines
	m_kdc->out_disp_callback().set(FUNC(sdk85_state::digit_w));             // display A&B
	m_kdc->in_rl_callback().set(FUNC(sdk85_state::kbd_r));                  // kbd RL lines
	m_kdc->in_shift_callback().set_constant(1);                             // Shift key
	m_kdc->in_ctrl_callback().set_constant(1);

	RS232_PORT(config, m_tty, default_rs232_devices, nullptr); // actually a 20 mA current loop
	m_tty->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	SOFTWARE_LIST(config, "rom_list").set_original("sdk85");
}

/* ROM definition */
ROM_START( sdk85 )
	ROM_REGION( 0x800, "romio", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "default", "Default")
	ROMX_LOAD( "sdk85.a14", 0x0000, 0x0800, CRC(9d5a983f) SHA1(54e218560fbec009ac3de5cfb64b920241ef2eeb), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "mastermind", "Mastermind")
	ROMX_LOAD( "mastermind.a14", 0x0000, 0x0800, CRC(36b694ae) SHA1(4d8a5ae5d10e8f72a6e349c7eeaf1aa00c4e45e1), ROM_BIOS(1) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS */
COMP( 1977, sdk85, 0,      0,      sdk85,   sdk85, sdk85_state, empty_init, "Intel", "MCS-85 System Design Kit", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
