// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Ben Bruscella, Sean Young, Frank Palazzolo
/*******************************************************************************************************

  coleco.c

  Driver file to handle emulation of the ColecoVision.

  Marat Fayzullin (ColEm source)
  Marcel de Kogel (AdamEm source)
  Mike Balfour
  Ben Bruscella
  Sean Young

  NEWS:
    - Modified memory map, now it has only 1k of RAM mapped on 8k Slot
    - Modified I/O map, now it is handled as on a real ColecoVision:
        The I/O map is broken into 4 write and 4 read ports:
            80-9F (W) = Set both controllers to keypad mode
            80-9F (R) = Not Connected

            A0-BF (W) = Video Chip (TMS9928A), A0=0 -> Write Register 0 , A0=1 -> Write Register 1
            A0-BF (R) = Video Chip (TMS9928A), A0=0 -> Read Register 0 , A0=1 -> Read Register 1

            C0-DF (W) = Set both controllers to joystick mode
            C0-DF (R) = Not Connected

            E0-FF (W) = Sound Chip (SN76489A)
            E0-FF (R) = Read Controller data, A1=0 -> read controller 1, A1=1 -> read controller 2

    - Modified paddle handler, now it is handled as on a real ColecoVision
    - Added support for a Roller Controller (Trackball), enabled via category
    - Added support for two Super Action Controller, enabled via category

    EXTRA CONTROLLERS INFO:

    -Driving Controller (Expansion Module #2). It consist of a steering wheel and a gas pedal. Only one
    can be used on a real ColecoVision. The gas pedal is not analog, internally it is just a switch.
    On a real ColecoVision, when the Driving Controller is enabled, the controller 1 do not work because
    have been replaced by the Driving Controller, and controller 2 have to be used to start game, gear
    shift, etc.
    Driving Controller is just a spinner on controller 1 socket similar to the one on Roller Controller
    and Super Action Controllers so you can use Roller Controller or Super Action Controllers to play
    games requiring Driving Controller.

    -Roller Controller. Basically a trackball with four buttons (the two fire buttons from player 1 and
    the two fire buttons from player 2). Only one Roller Controller can be used on a real ColecoVision.
    Roller Controller is connected to both controller sockets and both controllers are conected to the Roller
    Controller, it uses the spinner pins of both sockets to generate the X and Y signals (X from controller 1
    and the Y from controller 2)

    -Super Action Controllers. It is a hand controller with a keypad, four buttons (the two from
    the player pad and two more), and a spinner. This was made primarily for two player sport games, but
    will work for every other ColecoVision game.

*******************************************************************************************************/

/*

    TODO:

    - Dina SG-1000 mode

    - Bit90:
        Add support for external memory handling (documented)
        Add support for printer interface (documented)
        Add tape support

*/

#include "emu.h"

#include "coleco.h"

#include "bus/coleco/expansion/expansion.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "logmacro.h"

/* Read/Write Handlers */

uint8_t coleco_state::paddle_1_r()
{
	return m_joy_d7_state[0] | coleco_paddle_read(0, m_joy_mode, m_joy_analog_state[0]);
}

uint8_t coleco_state::paddle_2_r()
{
	// Tape notes:
	//     Signal is averaged to set the threshold voltage for a comparator
	//     Output of the comparator goes to bit 7
	return m_joy_d7_state[1] | coleco_paddle_read(1, m_joy_mode, m_joy_analog_state[1]);
}

void coleco_state::paddle_off_w(uint8_t data)
{
	m_joy_mode = 0;
}

void coleco_state::paddle_on_w(uint8_t data)
{
	m_joy_mode = 1;
}

void bit90_state::init()
{
	auto pgm = &m_maincpu->space(AS_PROGRAM);

	if(m_ram->size() == 32768)
		return;
	else if(m_ram->size() == 16384)
		pgm->unmap_readwrite(0xc000, 0xffff);
	else if(m_ram->size() == 1024)
		pgm->unmap_readwrite(0x8000, 0xffff);
}

uint8_t bit90_state::bankswitch_u4_r(address_space &space)
{
	if (!machine().side_effects_disabled()) {
		LOG("Bankswitch to u4\n");
		m_bank->set_entry(0);
	}
	return space.unmap();
}

uint8_t bit90_state::bankswitch_u3_r(address_space &space)
{
	if (!machine().side_effects_disabled()) {
		LOG("Bankswitch to u3\n");
		m_bank->set_entry(1);
	}
	return space.unmap();
}

uint8_t bit90_state::keyboard_r(address_space &space)
{
	if (m_keyselect < 8) {
		return m_io_keyboard[m_keyselect]->read();
	}
	return space.unmap();
}

void bit90_state::u32_w(uint8_t data)
{
	// Write to a 74LS174 at u32
	// Bits 4-7 are connected for keyboard scanning (actually only 4-6 are used)
	// Bits 0-1 Tape Write
	//     Notes: Tape waveform is generated using bits 0 and 1 connected as 2-bit DAC, with four output levels
	//            The analog output is put through an RC filter to approximate a sine wave
	//            Tape waveforms appear to be full cycles of one of 2 frequencies
	m_keyselect = (data >> 4) & 0x0f;
	uint8_t temp = data & 0x03;
	if (m_unknown != temp) {
		m_unknown = temp;
		LOG("m_unknown -> 0x%02x\n",m_unknown);
	}
}

/* Memory Maps */

void coleco_state::coleco_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x6000, 0x63ff).ram().mirror(0x1c00);
	map(0x8000, 0xffff).rw(FUNC(coleco_state::cart_r), FUNC(coleco_state::cart_w));
}

void bit90_state::bit90_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr("bank");
	map(0x2000, 0x3fff).rom();
	map(0x4000, 0x5fff).rom();  // Decoded through pin 5 of the Bit90 expansion port
	map(0x6000, 0x67ff).ram().mirror(0x1800);
	map(0x8000, 0xffff).rw(FUNC(coleco_state::cart_r), FUNC(coleco_state::cart_w));
}

void coleco_state::coleco_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x80).mirror(0x1f).w(FUNC(coleco_state::paddle_off_w));
	map(0xa0, 0xa1).mirror(0x1e).rw("tms9928a", FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(0xc0, 0xc0).mirror(0x1f).w(FUNC(coleco_state::paddle_on_w));
	map(0xe0, 0xe0).mirror(0x1f).w("sn76489a", FUNC(sn76489a_device::write));
	map(0xe0, 0xe0).mirror(0x1d).r(FUNC(coleco_state::paddle_1_r));
	map(0xe2, 0xe2).mirror(0x1d).r(FUNC(coleco_state::paddle_2_r));
}

void bit90_state::bit90_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x80).mirror(0x17).r(FUNC(bit90_state::bankswitch_u4_r));
	map(0x88, 0x88).mirror(0x17).r(FUNC(bit90_state::bankswitch_u3_r));
	map(0x80, 0x80).mirror(0x1f).w(FUNC(coleco_state::paddle_off_w));
	map(0xa0, 0xa1).mirror(0x1e).rw("tms9928a", FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(0xc0, 0xc0).mirror(0x1f).r(FUNC(bit90_state::keyboard_r));
	map(0xc0, 0xc0).mirror(0x1f).w(FUNC(coleco_state::paddle_on_w));
	map(0xe0, 0xe0).mirror(0x1d).r(FUNC(coleco_state::paddle_1_r));
	map(0xe0, 0xe0).mirror(0x1b).w(FUNC(bit90_state::u32_w));        // bits7-4 for keyscan, (to bcd decoder) and bits1-0 tape out
	map(0xe2, 0xe2).mirror(0x1d).r(FUNC(coleco_state::paddle_2_r));  // also, bit7 is tape in
	map(0xe4, 0xe4).mirror(0x1b).w("sn76489a", FUNC(sn76489a_device::write));

	// IORQ goes to pin 55 of the Bit90 expansion port,
	// So ports < 0x80 can be decoded there

	// External Printer Interface
	//map(0x48, 0x48).w(FUNC(bit90_state::printer_data_w));  // data to latch here
	//map(0x4a, 0x4a).r(FUNC(bit90_state::printer_control_r));  // bit0 is busy bit
	//map(0x4a, 0x4a).w(FUNC(bit90_state::printer_control_w));  // bit0 is reset (active low), bit1 is latch (active low)

	// External/(Internal?) RAM Interface
	//map(0x4e, 0x4f).w(FUNC(bit90_state::external_ram_control_w)); // 0x4e enable, 0x4f disable
	// RAM can appear here, starting at 0x8000 up to 0xffff
}

void coleco_state::czz50_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6000, 0x63ff).ram().mirror(0x1c00);
	map(0x8000, 0xffff).rom();
}

/* Input Ports */

static INPUT_PORTS_START( czz50 )
	PORT_START("STD_KEYPAD1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("#") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('#')
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("*") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('*')
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xb000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STD_JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xb0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STD_KEYPAD2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xb000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STD_JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xb0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( bit90 )
	PORT_INCLUDE( coleco )

	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUBOUT") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("META") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BASIC") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FCTN") PORT_CODE(KEYCODE_RCONTROL)
INPUT_PORTS_END

/* Interrupts */

void coleco_state::coleco_vdp_interrupt(int state)
{
	// NMI on rising edge
	if (state && !m_last_nmi_state)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	m_last_nmi_state = state;
}

TIMER_CALLBACK_MEMBER(coleco_state::paddle_d7reset_callback)
{
	m_joy_d7_state[param] = 0;
	m_joy_analog_state[param] = 0;
}

TIMER_CALLBACK_MEMBER(coleco_state::paddle_irqreset_callback)
{
	m_joy_irq_state[param] = 0;

	if (!m_joy_irq_state[param ^ 1])
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(coleco_state::paddle_pulse_callback)
{
	if (m_joy_analog_reload[param])
	{
		m_joy_analog_state[param] = m_joy_analog_reload[param];

		// on movement, controller port d7 is set for a short period and an irq is fired on d7 rising edge
		m_joy_d7_state[param] = 0x80;
		m_joy_d7_timer[param]->adjust(attotime::from_usec(500), param); // TODO: measure duration

		// irq on rising edge, PULSE_LINE is not supported in this case, so clear it manually
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_joy_irq_timer[param]->adjust(attotime::from_usec(11), param); // TODO: measure duration
		m_joy_irq_state[param] = 1;

		// reload timer
		m_joy_pulse_timer[param]->adjust(m_joy_pulse_reload[param], param);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(coleco_state::paddle_update_callback)
{
	// arbitrary timer for reading analog controls
	coleco_scan_paddles(&m_joy_analog_reload[0], &m_joy_analog_reload[1]);

	for (int port = 0; port < 2; port++)
	{
		if (m_joy_analog_reload[port])
		{
			const int sensitivity = 500;
			int ipt = m_joy_analog_reload[port];
			if (ipt & 0x80) ipt = 0x100 - ipt;
			attotime freq = attotime::from_msec(sensitivity / ipt);

			// change pulse intervals relative to spinner/trackball speed
			m_joy_pulse_reload[port] = freq;
			m_joy_pulse_timer[port]->adjust(std::min(freq, m_joy_pulse_timer[port]->remaining()), port);
		}
	}
}

uint8_t coleco_state::cart_r(offs_t offset)
{
	return m_cart->read(offset, 0, 0, 0, 0);
}

void coleco_state::cart_w(offs_t offset, uint8_t data)
{
	m_cart->write(offset, data, 0, 0, 0, 0);
}

uint8_t coleco_state::coleco_scan_paddles(uint8_t *joy_status0, uint8_t *joy_status1)
{
	uint8_t ctrl_sel = m_ctrlsel.read_safe(0);

	/* which controller shall we read? */
	if ((ctrl_sel & 0x07) == 0x02)          // Super Action Controller P1
		*joy_status0 = m_sac_slide1.read_safe(0);
	else if ((ctrl_sel & 0x07) == 0x03)     // Driving Controller P1
		*joy_status0 = m_driv_wheel1.read_safe(0);

	if ((ctrl_sel & 0x70) == 0x20)          // Super Action Controller P2
		*joy_status1 = m_sac_slide2.read_safe(0);
	else if ((ctrl_sel & 0x70) == 0x30)     // Driving Controller P2
		*joy_status1 = m_driv_wheel2.read_safe(0);

	/* In principle, even if not supported by any game, I guess we could have two Super
	   Action Controllers plugged into the Roller controller ports. Since I found no info
	   about the behavior of sliders in such a configuration, we overwrite SAC sliders with
	   the Roller trackball inputs and actually use the latter ones, when both are selected. */
	if (ctrl_sel & 0x80)                    // Roller controller
	{
		*joy_status0 = m_roller_x.read_safe(0);
		*joy_status1 = m_roller_y.read_safe(0);
	}

	return *joy_status0 | *joy_status1;
}


uint8_t coleco_state::coleco_paddle_read(int port, int joy_mode, uint8_t joy_status)
{
	uint8_t ctrl_sel = m_ctrlsel.read_safe(0);
	uint8_t ctrl_extra = ctrl_sel & 0x80;
	ctrl_sel = ctrl_sel >> (port*4) & 7;

	/* Keypad and fire 1 (SAC Yellow Button) */
	if (joy_mode == 0)
	{
		/* No key pressed by default */
		uint8_t data = 0x0f;
		uint16_t ipt = 0xffff;

		if (ctrl_sel == 0)          // ColecoVision Controller
			ipt = port ? m_std_keypad2->read() : m_std_keypad1->read();
		else if (ctrl_sel == 2)     // Super Action Controller
			ipt = port ? m_sac_keypad2->read() : m_sac_keypad1->read();

		/* Numeric pad buttons are not independent on a real ColecoVision, if you push more
		   than one, a real ColecoVision think that it is a third button, so we are going to emulate
		   the right behaviour */
		/* Super Action Controller additional buttons are read in the same way */
		if (!(ipt & 0x0001)) data &= 0x0a; /* 0 */
		if (!(ipt & 0x0002)) data &= 0x0d; /* 1 */
		if (!(ipt & 0x0004)) data &= 0x07; /* 2 */
		if (!(ipt & 0x0008)) data &= 0x0c; /* 3 */
		if (!(ipt & 0x0010)) data &= 0x02; /* 4 */
		if (!(ipt & 0x0020)) data &= 0x03; /* 5 */
		if (!(ipt & 0x0040)) data &= 0x0e; /* 6 */
		if (!(ipt & 0x0080)) data &= 0x05; /* 7 */
		if (!(ipt & 0x0100)) data &= 0x01; /* 8 */
		if (!(ipt & 0x0200)) data &= 0x0b; /* 9 */
		if (!(ipt & 0x0400)) data &= 0x06; /* # */
		if (!(ipt & 0x0800)) data &= 0x09; /* * */
		if (!(ipt & 0x1000)) data &= 0x04; /* Blue Action Button */
		if (!(ipt & 0x2000)) data &= 0x08; /* Purple Action Button */

		return ((ipt & 0x4000) >> 8) | 0x30 | data;
	}
	/* Joystick and fire 2 (SAC Red Button) */
	else
	{
		uint8_t data = 0x7f;

		if (ctrl_sel == 0)          // ColecoVision Controller
			data = port ? m_std_joy2->read() : m_std_joy1->read();
		else if (ctrl_sel == 2)     // Super Action Controller
			data = port ? m_sac_joy2->read() : m_sac_joy1->read();
		else if (ctrl_sel == 3)     // Driving Controller
			data = port ? m_driv_pedal2->read() : m_driv_pedal1->read();

		/* If any extra analog contoller enabled */
		if (ctrl_extra || ctrl_sel == 2 || ctrl_sel == 3)
		{
			if (joy_status & 0x80) data ^= 0x30;
			else if (joy_status) data ^= 0x10;
		}

		return data & 0x7f;
	}
}

void coleco_state::machine_start()
{
	// init paddles
	for (int port = 0; port < 2; port++)
	{
		m_joy_pulse_timer[port] = timer_alloc(FUNC(coleco_state::paddle_pulse_callback), this);
		m_joy_d7_timer[port] = timer_alloc(FUNC(coleco_state::paddle_d7reset_callback), this);
		m_joy_irq_timer[port] = timer_alloc(FUNC(coleco_state::paddle_irqreset_callback), this);

		m_joy_irq_state[port] = 0;
		m_joy_d7_state[port] = 0;
		m_joy_analog_state[port] = 0;
	}

	save_item(NAME(m_joy_mode));
	save_item(NAME(m_last_nmi_state));
	save_item(NAME(m_joy_irq_state));
	save_item(NAME(m_joy_d7_state));
	save_item(NAME(m_joy_analog_state));
	save_item(NAME(m_joy_analog_reload));
}

void bit90_state::machine_start()
{
	coleco_state::machine_start();
	uint8_t *banked = memregion("banked")->base();
	m_bank->configure_entries(0, 0x02, banked, 0x2000);
}

void coleco_state::machine_reset()
{
	m_last_nmi_state = 0;
}

void bit90_state::machine_reset()
{
	coleco_state::machine_reset();
	m_bank->set_entry(0);
}

//static std::error_condition coleco_cart_verify(const uint8_t *cartdata, size_t size)
//{
//  std::error_condition retval = image_error::INVALIDIMAGE;
//
//  /* Verify the file is in Colecovision format */
//  if ((cartdata[0] == 0xAA) && (cartdata[1] == 0x55)) /* Production Cartridge */
//      retval = std::error_condition();
//  if ((cartdata[0] == 0x55) && (cartdata[1] == 0xAA)) /* "Test" Cartridge. Some games use this method to skip ColecoVision title screen and delay */
//      retval = std::error_condition();
//
//  return retval;
//}


/* Machine Drivers */

void coleco_state::coleco(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(7'159'090)/2); // 3.579545 MHz
	m_maincpu->z80_set_m1_cycles(4+1); // 1 WAIT CLK per M1
	m_maincpu->set_addrmap(AS_PROGRAM, &coleco_state::coleco_map);
	m_maincpu->set_addrmap(AS_IO, &coleco_state::coleco_io_map);

	/* video hardware */
	tms9928a_device &vdp(TMS9928A(config, "tms9928a", XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set(FUNC(coleco_state::coleco_vdp_interrupt));
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	sn76489a_device &psg(SN76489A(config, "sn76489a", XTAL(7'159'090)/2)); // 3.579545 MHz
	psg.add_route(ALL_OUTPUTS, "mono", 1.00);
	psg.ready_cb().set_inputline("maincpu", Z80_INPUT_LINE_WAIT).invert();

	/* cartridge */
	COLECOVISION_CARTRIDGE_SLOT(config, m_cart, colecovision_cartridges, nullptr);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("coleco");
	SOFTWARE_LIST(config, "homebrew_list").set_original("coleco_homebrew");

	TIMER(config, "paddle_timer").configure_periodic(FUNC(coleco_state::paddle_update_callback), attotime::from_msec(20));

	coleco_expansion_device &exp(COLECO_EXPANSION(config, "exp", nullptr));
	exp.set_program_space(m_maincpu, AS_PROGRAM);
	exp.set_io_space(m_maincpu, AS_IO);
	exp.int_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0); // TODO: Merge with other IRQs?
	exp.nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	exp.add_route(ALL_OUTPUTS, "mono", 1.00);
}

void coleco_state::colecop(machine_config &config)
{
	coleco(config);

	/* video hardware */
	tms9929a_device &vdp(TMS9929A(config.replace(), "tms9928a", XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set(FUNC(coleco_state::coleco_vdp_interrupt));
}

void bit90_state::bit90(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(7'159'090)/2); // 3.579545 MHz
	m_maincpu->z80_set_m1_cycles(4+1); // 1 WAIT CLK per M1
	m_maincpu->set_addrmap(AS_PROGRAM, &bit90_state::bit90_map);
	m_maincpu->set_addrmap(AS_IO, &bit90_state::bit90_io_map);

	/* video hardware */
	tms9929a_device &vdp(TMS9929A(config, "tms9928a", XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set(FUNC(coleco_state::coleco_vdp_interrupt));
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	sn76489a_device &psg(SN76489A(config, "sn76489a", XTAL(7'159'090)/2)); // 3.579545 MHz
	psg.add_route(ALL_OUTPUTS, "mono", 1.00);
	psg.ready_cb().set_inputline("maincpu", Z80_INPUT_LINE_WAIT).invert();

	/* cartridge */
	COLECOVISION_CARTRIDGE_SLOT(config, m_cart, colecovision_cartridges, nullptr);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("coleco");
	SOFTWARE_LIST(config, "homebrew_list").set_original("coleco_homebrew");

	/* internal ram */
	RAM(config, m_ram).set_default_size("32K").set_extra_options("1K,16K");

	TIMER(config, "paddle_timer").configure_periodic(FUNC(coleco_state::paddle_update_callback), attotime::from_msec(20));
}

void coleco_state::czz50(machine_config &config)
{
	coleco(config);

	config.device_remove("exp"); // this system has a different expansion port

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &coleco_state::czz50_map); // note: cpu speed unverified, assume it's the same as ColecoVision
}

void coleco_state::dina(machine_config &config)
{
	czz50(config);

	/* video hardware */
	tms9929a_device &vdp(TMS9929A(config.replace(), "tms9928a", XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set(FUNC(coleco_state::coleco_vdp_interrupt));
}


/* ROMs */

ROM_START (coleco)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "313_10031-4005_73108a.u2", 0x0000, 0x2000, CRC(3aa93ef3) SHA1(45bedc4cbdeac66c7df59e9e599195c778d86a92) )
ROM_END

/*  ONYX (Prototype)
    Unreleased Brazilian Colecovision clone by Microdigital.

    It was never released and the only known prototypes were uncovered by an ex-employee of Microdigital
    called Cl??udio Cassens who donated it to collectors (Eduardo Luccas) in June 2015.
    -- Felipe Sanches
*/
ROM_START (onyx)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "onyx.rom", 0x0000, 0x2000, CRC(011c32e7) SHA1(f44263221e330b2590dffc1a6f43ed2591fe19be) )
ROM_END

/*  PAL Colecovision BIOS

Country: Italy
Serial number: C0039036
Model number: 240020
Circuit board: (C) 1983 91162 rev D

Information about the chip

Motorola logo
(C)1983 COLECO
R72114A
8317   */

ROM_START (colecop)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r72114a_8317.u2", 0x0000, 0x2000, CRC(d393c0cc) SHA1(160077afb139943725c634d6539898db59f33657) )
ROM_END

ROM_START( czz50 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "czz50.rom", 0x0000, 0x2000, CRC(4999abc6) SHA1(96aecec3712c94517103d894405bc98a7dafa440) )
	ROM_CONTINUE( 0x8000, 0x2000 )
ROM_END

#define rom_dina rom_czz50
#define rom_prsarcde rom_czz50

/* Bit Corporation - BIT90

Circuit board is labelled: BIT90C-PAL-90002 or BIT90C-PAL-90003

BIT90C-PAL-90002 has 2K Internal RAM (<1K Usable from BASIC)
    Extra RAM can only be accessed via expansion port

BIT90C-PAL-90003 has sockets for additional internal 16K or 32K internal RAM

Units have 2764-compatible pinouts at U2,U3, and U4
Some units have 2764 EPROMS, Mask ROMs, or a combination

Mask ROMs are labelled:

@U4:
D32351E
BIT-99C1

@U3:
D32521E
MONITOR2

@U2:
D32522E
MONITOR3

*/

ROM_START( bit90 )
	ROM_DEFAULT_BIOS( "3.1" )
	ROM_SYSTEM_BIOS( 0, "3.0", "BASIC 3.0" )
	ROM_SYSTEM_BIOS( 1, "3.1", "BASIC 3.1" )

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROMX_LOAD("bit90b3.u2",  0x2000, 0x2000, CRC(b992b940) SHA1(c7dd96a1944fac40cbae20630f303a69de7e6313), ROM_BIOS(0))
	ROMX_LOAD("d32522e.u2",  0x2000, 0x2000, CRC(66fc66b0) SHA1(6644c217860aa9940bef7c6aeb50768810d9035b), ROM_BIOS(1)) // MONITOR3

	ROM_REGION( 0x4000, "banked", 0 )
	ROMX_LOAD("bit90b3.u4", 0x0000, 0x2000, CRC(06d21fc2) SHA1(6d296b09b661babd4c2ef6993f8e768a67932388), ROM_BIOS(0))
	ROMX_LOAD("bit90b3.u3", 0x2000, 0x2000, CRC(61fdccbb) SHA1(25cac13627c0916d3ed2b92f0b2218b405de5be4), ROM_BIOS(0))
	ROMX_LOAD("d32351e.u4", 0x0000, 0x2000, CRC(d00c7137) SHA1(43328257136aff5a4984cceafdb5601200ac24b4), ROM_BIOS(1)) // BIT-99C1
	ROMX_LOAD("d32521e.u3", 0x2000, 0x2000, CRC(f6401dd8) SHA1(78bc7f0fe4f5eb114773d654c92598512481abec), ROM_BIOS(1)) // MONITOR2
ROM_END

/* System Drivers */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS         INIT        COMPANY             FULLNAME                            FLAGS
CONS( 1982, coleco,   0,      0,      coleco,   coleco, coleco_state, empty_init, "Coleco",           "ColecoVision (NTSC)",              0 )
CONS( 1982, onyx,     coleco, 0,      coleco,   coleco, coleco_state, empty_init, "Microdigital",     "Onyx (Brazil/Prototype)",          0 )
CONS( 1983, colecop,  coleco, 0,      colecop,  coleco, coleco_state, empty_init, "Coleco",           "ColecoVision (PAL)",               0 )
CONS( 1986, czz50,    0,      coleco, czz50,    czz50,  coleco_state, empty_init, "Bit Corporation",  "Chuang Zao Zhe 50",                0 )
CONS( 1988, dina,     czz50,  0,      dina,     czz50,  coleco_state, empty_init, "Telegames",        "Dina",                             0 )
CONS( 1988, prsarcde, czz50,  0,      czz50,    czz50,  coleco_state, empty_init, "Telegames",        "Personal Arcade",                  0 )
COMP( 1983, bit90,    0,      coleco, bit90,    bit90,  bit90_state,  init,       "Bit Corporation",  "Bit90",                            0 )
