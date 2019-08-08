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

Notes on expansion rom with default content Mastermind.
Original authors: see above (Paolo Forlani and Stefano Bodrato)
Ported to option rom: NASZVADI Peter

The game can be started from monitor by defining SP as 20FFh and PC as 0800h and starting execution.
When setting register values in monitor, SPH, SPL, PCH and PCL values must be set to 20, FF, 08, 00 respectively before start!
Stefano's bios had been altered in order to use lower ram bank and the option rom slot.
When selecting "Empty" a14, which is basically 2kbytes of zeros, the default option rom will be launched directly.

*************************************************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "machine/i8355.h"
#include "machine/i8279.h"
#include "sdk85.lh"


class sdk85_state : public driver_device
{
public:
	sdk85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_keyboard(*this, "X%u", 0)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void sdk85(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(scanlines_w);
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_READ8_MEMBER(kbd_r);
	void sdk85_io(address_map &map);
	void sdk85_mem(address_map &map);

	u8 m_digit;
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cpu_device> m_maincpu;
	required_ioport_array<3> m_keyboard;
	output_finder<6> m_digits;
};

void sdk85_state::machine_reset()
{
	// Prevent spurious TRAP when system is reset
	m_maincpu->reset();
}

void sdk85_state::sdk85_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).r("romio", FUNC(i8355_device::memory_r));
	map(0x0800, 0x0fff).r("expromio", FUNC(i8355_device::memory_r));
	map(0x1800, 0x1800).mirror(0x06ff).rw("kdc", FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));
	map(0x1900, 0x1900).mirror(0x06ff).rw("kdc", FUNC(i8279_device::status_r), FUNC(i8279_device::cmd_w));
	map(0x2000, 0x20ff).mirror(0x0700).rw("ramio", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x2800, 0x28ff).mirror(0x0700).rw("expramio", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
}

void sdk85_state::sdk85_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x03).mirror(0x04).rw("romio", FUNC(i8355_device::io_r), FUNC(i8355_device::io_w));
	map(0x08, 0x0b).mirror(0x04).rw("expromio", FUNC(i8355_device::io_r), FUNC(i8355_device::io_w));
	map(0x20, 0x27).rw("ramio", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x28, 0x2f).rw("expramio", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}

/* Input ports */
static INPUT_PORTS_START( sdk85 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXEC")   PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NEXT")   PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO")     PORT_CODE(KEYCODE_R)  PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SUBST")  PORT_CODE(KEYCODE_T)  PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXAM")   PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SINGLE") PORT_CODE(KEYCODE_U)  PORT_CHAR('U')
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


WRITE8_MEMBER( sdk85_state::scanlines_w )
{
	m_digit = data;
}

WRITE8_MEMBER( sdk85_state::digit_w )
{
	if (m_digit < 6)
		m_digits[m_digit] = bitswap<8>(~data, 3, 2, 1, 0, 7, 6, 5, 4);
}

READ8_MEMBER( sdk85_state::kbd_r )
{
	u8 data = (m_digit < 3) ? m_keyboard[m_digit]->read() : 0xff;
	return data;
}

void sdk85_state::sdk85(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sdk85_state::sdk85_mem);
	m_maincpu->set_addrmap(AS_IO, &sdk85_state::sdk85_io);

	I8355(config, "romio", 6.144_MHz_XTAL / 2); // Monitor ROM (A14)

	I8355(config, "expromio", 6.144_MHz_XTAL / 2); // Expansion ROM (A15)

	i8155_device &i8155(I8155(config, "ramio", 6.144_MHz_XTAL / 2)); // Basic RAM (A16)
	i8155.out_to_callback().set_inputline(m_maincpu, I8085_TRAP_LINE);

	I8155(config, "expramio", 6.144_MHz_XTAL / 2); // Expansion RAM (A17)

	/* video hardware */
	config.set_default_layout(layout_sdk85);

	/* Devices */
	i8279_device &kdc(I8279(config, "kdc", 6.144_MHz_XTAL / 2));        // Keyboard/Display Controller (A13)
	kdc.out_irq_callback().set_inputline("maincpu", I8085_RST55_LINE);  // irq
	kdc.out_sl_callback().set(FUNC(sdk85_state::scanlines_w));          // scan SL lines
	kdc.out_disp_callback().set(FUNC(sdk85_state::digit_w));            // display A&B
	kdc.in_rl_callback().set(FUNC(sdk85_state::kbd_r));                 // kbd RL lines
	kdc.in_shift_callback().set_constant(1);                            // Shift key
	kdc.in_ctrl_callback().set_constant(1);
}

/* ROM definition */
ROM_START( sdk85 )
	ROM_REGION( 0x800, "romio", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "default", "Default")
	ROMX_LOAD( "sdk85.a14", 0x0000, 0x0800, CRC(9d5a983f) SHA1(54e218560fbec009ac3de5cfb64b920241ef2eeb), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "mastermind", "Mastermind")
	ROMX_LOAD( "mastermind.a14", 0x0000, 0x0800, CRC(36b694ae) SHA1(4d8a5ae5d10e8f72a6e349c7eeaf1aa00c4e45e1), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "empty", "Empty")
	ROMX_LOAD( "empty.a14", 0x0000, 0x0800, CRC(f1e8ba9e) SHA1(605db3fdbaff4ba13729371ad0c4fbab3889378e), ROM_BIOS(2) )

	ROM_REGION( 0x800, "expromio", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL( "mastermind.a15", 0x0000, 0x0800, CRC(0538e162) SHA1(c351975e2cf515cee29fcaeb04ef47189afe5250) )
ROM_END

/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS */
COMP( 1977, sdk85, 0,      0,      sdk85,   sdk85, sdk85_state, empty_init, "Intel", "MCS-85 System Design Kit", MACHINE_NO_SOUND_HW)
