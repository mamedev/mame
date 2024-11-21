// license:BSD-3-Clause
// copyright-holders:Golden Child
/**********************************************************************

    TK-10 Apple ][ Clone keyboard



to patch into the apple2 driver:


+#include "machine/tk10_keyboard.h"

+void tk10_data_write(u8 data);

+void apple2_state::tk10_data_write(u8 data)
+{
+ //    printf("tk10_code_sent_to_apple %x\n",data);
+       if (data & 0x80)
+       {
+                m_transchar = data & 0x7f;
+                m_strobe = 0x80;
+       }
+}
+

+       tk10_keyboard_device &tk10(TK10_KEYBOARD(config, "tk10", 0));
+       tk10.data_write_cb().set(FUNC(apple2_state::tk10_data_write));
+



**********************************************************************/

#include "emu.h"

#include "tk10_keyboard.h"

#include "utf8.h"


//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TK10_KEYBOARD, tk10_keyboard_device, "tk10_keyboard", "TK-10 Keyboard")


//-------------------------------------------------
//  tk10_keyboard_device - constructor
//-------------------------------------------------

tk10_keyboard_device::tk10_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TK10_KEYBOARD, tag, owner, clock)
	, m_data_write(*this)
	, m_reset_write(*this)
	, m_tk10cpu(*this, "tk10cpu")
	, m_keyboard(*this, "P2%u", 0U)
	, m_kbspecial(*this, "kbspecial")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tk10_keyboard_device::device_start()
{
}

//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------

void tk10_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8039(config, m_tk10cpu, 6_MHz_XTAL);
	m_tk10cpu->set_addrmap(AS_PROGRAM, &tk10_keyboard_device::tk10_mem_map);

	m_tk10cpu->t1_in_cb().set([this] () -> u8
	{
		u8 kbspecial = m_kbspecial->read();  // left and right shift keys
		return (BIT(kbspecial, 0) & BIT(kbspecial, 1));
	});

	m_tk10cpu->t0_in_cb().set([this] () -> u8
	{
		return (BIT(m_kbspecial->read(), 2));  // control key
	});

	m_tk10cpu->p1_in_cb().set([this] () -> u8
	{
		for (int i = 0; i < 8; i++)
		{
			if (!BIT(m_tk10cpu->p2_r(), i)) return m_keyboard[i]->read();
			// read key matrix:
			// a low value in p2 bit will drive bit of p1 low when switch contact made
		}
		return 0xff;
	});

	m_tk10cpu->bus_out_cb().set([this] (u8 data)
	{
		m_data_write(data); // callback with data in low 7 bits and strobe in msb
	});
}

void tk10_keyboard_device::tk10_mem_map(address_map &map)
{
	map(0x0000, 0x07ff).rom().region("tk10", 0);
}

INPUT_CHANGED_MEMBER(tk10_keyboard_device::tk10_func)
{
		m_tk10cpu->set_input_line(MCS48_INPUT_IRQ, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(tk10_keyboard_device::tk10_reset)
{
	// printf("TK10 RESET new=%x old=%x\n",newval,oldval);
	u8 key = m_kbspecial->read();
	if (!BIT(key, 3) && !BIT(key, 4)) m_reset_write(0);
	else m_reset_write(1);
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START(tk10_keyboard)

	PORT_START("P20")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Edit Function Key") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End Edit Function Key") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Insert Text") PORT_CODE(KEYCODE_INSERT)  // moves up, now inserts text, enter returns
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)   // CALL -151
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Pause Key") PORT_CODE(KEYCODE_UP)  // CTRL+S 0x13 PAUSE OUTPUT
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)   // UNDERSCORE   7F
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_8_PAD)   // CTRL+i 0x09  tab??

	PORT_START("P21")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)   //  _ SHIFT=^
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)   // ]
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  // [
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)        // @ SHIFT=^
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)   // /
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)

	PORT_START("P22")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) // BACKSLASH
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("P23")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)  PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('^') PORT_CHAR('[')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("P24")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')

	PORT_START("P25")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')

	PORT_START("P26")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)  PORT_CHAR('0')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('@')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')

	PORT_START("P27")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("kbspecial")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RESET") PORT_CODE(KEYCODE_F12) // RESET not currently hooked up
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tk10_keyboard_device::tk10_reset), 0)

	PORT_START("tk10_function_key")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Function Key") PORT_CODE(KEYCODE_LALT)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tk10_keyboard_device::tk10_func), 0)

INPUT_PORTS_END

//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------

ioport_constructor tk10_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tk10_keyboard);
}


//**************************************************************************
//  ROM DEFINITION
//**************************************************************************

ROM_START(tk10_keyboard)
	ROM_REGION(0x800, "tk10",0)
	ROM_LOAD( "tk10.bin", 0x0000, 0x0800, CRC(a06c5b78) SHA1(27c5160b913e0f62120f384026d24b9f1acb6970) )
ROM_END

//-------------------------------------------------
//  device_rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *tk10_keyboard_device::device_rom_region() const
{
	return ROM_NAME(tk10_keyboard);
}


