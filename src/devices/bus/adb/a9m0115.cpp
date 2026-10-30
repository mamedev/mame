// license:BSD-3-Clause
// copyright-holders: R. Belmont
/*********************************************************************

    a9m0115.cpp
	Apple A9M0115 Extended Keyboard
	Apple A9M3501 Extended Keyboard II
    by R. Belmont

    Port 1: bits 3-0: Key matrix row select.  This is a row *number* 0-15 fed
                      to a decoder, not a one-hot mask.  Row $10 is the
                      modifier row and is read from Port 2 instead.
            bit 4:    Num Lock LED (active high)
            bit 5:    Caps Lock LED
            bit 6:    Scroll Lock LED
            bit 7:    ADB data out, open collector.  Writing 1 pulls the bus
                      low, 0 releases it.  The firmware read-modify-writes
                      Port 1 to change rows, so reading it back has to return
                      the output latch.
    Port 2: Modifier key inputs, active low.  Sampled during the row $10 scan
            and debounced by requiring eight identical reads.
              bit 0: Command       bit 4: Power
              bit 1: Left Option   bit 5: Caps Lock
              bit 2: Left Shift    bit 6: Right Shift
              bit 3: Left Control  bit 7: Right Control
            Bit 4 is also reported as bit 6 of Talk Register 3, the ADB
            "exceptional event" bit.
    Bus:    Key matrix column inputs, active low.
    T0:     Right Option key, active low, edge detected.
    T1:     ADB data in.
    INT:    Extended Keyboard: unused, the vector at $003 simply resets as a
            watchdog.  Extended Keyboard II: sampled once at power-on ($021)
            to pick the handler ID reported in Talk Register 3 - high gives
            $02 (ANSI), low gives $05 (ISO).

    Handler ID 2 is reported by default.  A Listen Register 3 can switch the
    keyboard to handler 3, which makes the right-hand modifiers report unique
	scan codes.

*********************************************************************/

#include "emu.h"
#include "a9m0115.h"

#include "cpu/mcs48/mcs48.h"

namespace {

class a9m0115_device : public adb_device_interface, public adb_slot_card_interface
{
public:
	a9m0115_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	a9m0115_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void adb_w(int state) override;

	required_device<i8049_device> m_mcu;
	required_ioport_array<16> m_rows;
	required_ioport m_t0;
	output_finder<3> m_leds;

private:
	u8 bus_r();
	void p1_w(u8 data);
	int t0_r();
	int t1_r();

	int m_adb_state;
	u8 m_kbd_row;
	int m_our_last_adb_state;
};

ROM_START(a9m0115)
	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD( "a9m0115.bin",  0x000000, 0x000800, CRC(3480e571) SHA1(699c7bdce78ee0b3f5704ffab0848bd9f0ce80e9) )
ROM_END

static INPUT_PORTS_START(a9m0115)
	PORT_START("ROW0")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))  PORT_NAME("Keypad Enter")   // $4C
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))      PORT_NAME("Keypad 3")       // $55
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))  PORT_NAME("Keypad -")       // $4E
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))   PORT_NAME("Keypad *")       // $43
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))  PORT_NAME("Keypad /")       // $4B
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))      PORT_NAME("Keypad 9")       // $5C

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))    PORT_NAME("Keypad .")       // $41
	PORT_BIT(0x06, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))                                  // $3C
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD)) PORT_NAME("Keypad =")       // $51
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_NUMLOCK)    PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))    PORT_NAME("Keypad Clear")   // $47

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))   PORT_NAME("Keypad +")       // $45
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))      PORT_NAME("Keypad 4")       // $56
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))      PORT_NAME("Keypad 5")       // $57
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))      PORT_NAME("Keypad 6")       // $58
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))      PORT_NAME("Keypad 7")       // $59
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))      PORT_NAME("Keypad 8")       // $5B
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))                                   // $3D
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))                                   // $3B
	PORT_BIT(0x3c, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F15)        PORT_CHAR(UCHAR_MAMEKEY(F15))                                    // $71
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F14)        PORT_CHAR(UCHAR_MAMEKEY(F14))                                    // $6B

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))      PORT_NAME("Keypad 2")       // $54
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))                                     // $3E
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))        PORT_NAME("Forward Delete") // $75
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))                                    // $77
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))                                   // $79
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))                                   // $74
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F13)        PORT_CHAR(UCHAR_MAMEKEY(F13))                                    // $69

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))      PORT_NAME("Keypad 1")       // $53
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')   PORT_CHAR('>')                                  // $2F
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(0x0d)                      PORT_NAME("Return")         // $24
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))     PORT_NAME("Help")           // $72
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))                                   // $73
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\')  PORT_CHAR('|')                                  // $2A
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                      PORT_NAME("Delete")         // $33
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))                                    // $6F

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))      PORT_NAME("Keypad 0")       // $52
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')   PORT_CHAR('?')                                  // $2C
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'')  PORT_CHAR('"')                                  // $27
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')   PORT_CHAR('}')                                  // $1E
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')   PORT_CHAR('{')                                  // $21
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')   PORT_CHAR('+')                                  // $18
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))                                    // $67
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))                                    // $6D

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')   PORT_CHAR('M')                                 // $2E
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')   PORT_CHAR('<')                                 // $2B
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')   PORT_CHAR(':')                                 // $29
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')   PORT_CHAR('P')                                 // $23
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')   PORT_CHAR('_')                                 // $1B
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')   PORT_CHAR('O')                                 // $1F
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')   PORT_CHAR(')')                                 // $1D
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))                                    // $65

	PORT_START("ROW8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')   PORT_CHAR('N')                                 // $2D
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')   PORT_CHAR('J')                                 // $26
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')   PORT_CHAR('L')                                 // $25
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')   PORT_CHAR('K')                                 // $28
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')   PORT_CHAR('(')                                 // $19
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                                    // $64
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))                                    // $62

	PORT_START("ROW9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')   PORT_CHAR('H')                                 // $04
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')   PORT_CHAR('I')                                 // $22
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')   PORT_CHAR('*')                                 // $1C
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')   PORT_CHAR('&')                                 // $1A
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))                                    // $61

	PORT_START("ROW10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')                                                  // $31
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')   PORT_CHAR('B')                                 // $0B
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')   PORT_CHAR('U')                                 // $20
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')   PORT_CHAR('Y')                                 // $10
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')   PORT_CHAR('T')                                 // $11
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')   PORT_CHAR('^')                                 // $16
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))                                    // $60

	PORT_START("ROW11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')   PORT_CHAR('V')                                 // $09
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')   PORT_CHAR('F')                                 // $03
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')   PORT_CHAR('R')                                 // $0F
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')   PORT_CHAR('G')                                 // $05
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')   PORT_CHAR('%')                                 // $17
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')   PORT_CHAR('$')                                 // $15
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))                                    // $76

	PORT_START("ROW12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')   PORT_CHAR('C')                                 // $08
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')   PORT_CHAR('D')                                 // $02
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')   PORT_CHAR('E')                                 // $0E
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')   PORT_CHAR('#')                                 // $14
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))                                    // $63

	PORT_START("ROW13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')   PORT_CHAR('X')                                 // $07
	PORT_BIT(0x06, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')   PORT_CHAR('W')                                 // $0D
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')   PORT_CHAR('@')                                 // $13
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))                                    // $78

	PORT_START("ROW14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')   PORT_CHAR('Z')                                 // $06
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')   PORT_CHAR('S')                                 // $01
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')   PORT_CHAR('Q')                                 // $0C
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')   PORT_CHAR('!')                                 // $12
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))                                    // $7A

	PORT_START("ROW15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')   PORT_CHAR('A')                                 // $00
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR('\t')                                                 // $30
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)   // the Extended Keyboard II puts the ISO key here
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`')   PORT_CHAR('~')                                 // $32
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(0x1b)                      PORT_NAME("Escape")        // $35

	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)                          PORT_NAME("Command")                         // $37
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN)                          PORT_NAME("Left Option")                     // $3A
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Left Shift")                    // $38
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)                      PORT_NAME("Left Control")                    // $36
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F20)                           PORT_NAME("Power")                           // $7F
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE          PORT_NAME("Caps Lock")                       // $39
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)                        PORT_NAME("Right Shift")                     // $38 / $7B
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)                      PORT_NAME("Right Control")                   // $36 / $7D

	PORT_START("T0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)                          PORT_NAME("Right Option")                    // $3A / $7C
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor a9m0115_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a9m0115);
}

void a9m0115_device::device_add_mconfig(machine_config &config)
{
	I8049(config, m_mcu, 6_MHz_XTAL);
	m_mcu->bus_in_cb().set(FUNC(a9m0115_device::bus_r));
	m_mcu->p1_out_cb().set(FUNC(a9m0115_device::p1_w));
	m_mcu->p2_in_cb().set_ioport("P2");
	m_mcu->t0_in_cb().set(FUNC(a9m0115_device::t0_r));
	m_mcu->t1_in_cb().set(FUNC(a9m0115_device::t1_r));
	// The firmware read-modify-writes Port 1 to change rows, 
	// and leaving the callback unset makes "in a,p1" return
	// the latched value.
}

const tiny_rom_entry *a9m0115_device::device_rom_region() const
{
	return ROM_NAME(a9m0115);
}

a9m0115_device::a9m0115_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: adb_device_interface(mconfig, type, tag, owner, clock)
	, adb_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_mcu(*this, "mcu")
	, m_rows{ *this, "ROW%u", 0U }
	, m_t0(*this, "T0")
	, m_leds(*this, "led%u", 0U)
	, m_adb_state(1)
	, m_kbd_row(0)
	, m_our_last_adb_state(1)
{
}

a9m0115_device::a9m0115_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: a9m0115_device(mconfig, ADB_A9M0115, tag, owner, clock)
{
}

void a9m0115_device::device_start()
{
	adb_device_interface::device_start();

	save_item(NAME(m_adb_state));
	save_item(NAME(m_kbd_row));
	save_item(NAME(m_our_last_adb_state));
}

void a9m0115_device::device_reset()
{
	adb_device_interface::device_reset();

	m_our_last_adb_state = 1;
	m_adb_state = 1;
	m_kbd_row = 0x0f;
}

void a9m0115_device::adb_w(int state)
{
	m_adb_state = state;
}

u8 a9m0115_device::bus_r()
{
	return m_rows[m_kbd_row]->read();
}

void a9m0115_device::p1_w(u8 data)
{
	m_kbd_row = data & 0x0f;

	m_leds[0] = BIT(data, 4);   // Num Lock
	m_leds[1] = BIT(data, 5);   // Caps Lock
	m_leds[2] = BIT(data, 6);   // Scroll Lock

	// ADB drive is through an inverting transistor
	int adb_state = BIT(data, 7) ^ 1;
	if (adb_state != m_our_last_adb_state)
	{
		adb_drive_w(adb_state);
		m_our_last_adb_state = adb_state;
		m_mcu->yield();
	}
}

int a9m0115_device::t0_r()
{
	// Right Option, active low
	return m_t0->read() & 1;
}

int a9m0115_device::t1_r()
{
	return m_adb_state & 1;
}

// *****************************************************
// A9M3501 Apple Extended Keyboard II section
// *****************************************************

//  - INT is sampled once at power-up to pick the handler ID
//    reported in Talk Register 3: high gives $02 (ANSI),
//    low gives $05 (ISO).

class a9m3501_device : public a9m0115_device
{
public:
	a9m3501_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;

private:
	required_ioport m_layout;
};

ROM_START(a9m3501)
	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD("a9m3501.bin", 0x000000, 0x000800, CRC(6fef541e) SHA1(891dbb4728f81f6da1e19287e01bc3e6a24d1af5))
ROM_END

static INPUT_PORTS_START(a9m3501)
	PORT_INCLUDE(a9m0115)

	PORT_MODIFY("ROW15")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_NAME(u8"ISO § / ±")

	PORT_START("LAYOUT")
	PORT_CONFNAME(0x01, 0x00, "Keyboard Layout")    // sampled from INT at power-on
	PORT_CONFSETTING(0x00, "ANSI (handler ID 2)")
	PORT_CONFSETTING(0x01, "ISO (handler ID 5)")
INPUT_PORTS_END

const tiny_rom_entry *a9m3501_device::device_rom_region() const
{
	return ROM_NAME(a9m3501);
}

ioport_constructor a9m3501_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a9m3501);
}

a9m3501_device::a9m3501_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	a9m0115_device(mconfig, ADB_A9M3501, tag, owner, clock)
	, m_layout(*this, "LAYOUT")
{
}

void a9m3501_device::device_reset_after_children()
{
	m_mcu->set_input_line(MCS48_INPUT_IRQ, BIT(m_layout->read(), 0) ? ASSERT_LINE : CLEAR_LINE);
}

} // anonymous namespace end

DEFINE_DEVICE_TYPE_PRIVATE(ADB_A9M0115, adb_slot_card_interface, a9m0115_device, "a9m0115", "Apple Extended Keyboard");
DEFINE_DEVICE_TYPE_PRIVATE(ADB_A9M3501, adb_slot_card_interface, a9m3501_device, "a9m3501", "Apple Extended Keyboard II");
