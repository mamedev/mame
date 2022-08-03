// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI Music Keyboard
    - CMI-10 Music Keyboard Controller
    - CMI-11 Keyboard Switch Module
    - CMI-12 Keyboard Display and Keypad Module
    - CMI-14 Slave Keyboard Interface

****************************************************************************

        The Master Keyboard
        -------------------

        The master keyboard has the following features:
            - A serial connector for communicating with the CMI mainframe
            - A connector for a slave keyboard
            - A connector for the alphanumeric keyboard
            - Connectors for pedal controls
            - Three slider-type analog controls
            - Two switch controls (one momentary, one toggle on/off)
            - Two lamp indicators for the switches with software-defined
              control
            - A 12-character LED alphanumeric display
            - A 16-switch keypad

        All communications with all peripherals and controls on the master
        keyboard is handled via the master keyboard's controller, and as
        such there is one single serial link to the "CMI mainframe" box
        itself.

***************************************************************************/

#include "emu.h"
#include "cmi_mkbd.h"

#include "cpu/m6800/m6800.h"
#include "machine/clock.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(CMI_MUSIC_KEYBOARD, cmi_music_keyboard_device, "cmi_mkbd", "Fairlight CMI Music Keyboard")

cmi_music_keyboard_device::cmi_music_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CMI_MUSIC_KEYBOARD, tag, owner, clock)
	, m_cmi_txd(*this)
	, m_cmi_rts(*this)
	, m_kbd_txd(*this)
	, m_kbd_rts(*this)
	, m_cpu(*this, "kbdcpu")
	, m_acia_kbd(*this, "acia_kbd")
	, m_acia_cmi(*this, "acia_cmi")
	, m_cmi10_pia_u20(*this, "cmi10_pia_u20")
	, m_cmi10_pia_u21(*this, "cmi10_pia_u21")
	, m_dp1(*this, "dp1")
	, m_dp2(*this, "dp2")
	, m_dp3(*this, "dp3")
	, m_keypad_a_port(*this, "KEYPAD_A")
	, m_keypad_b_port(*this, "KEYPAD_B")
	, m_analog(*this, "ANALOG")
	, m_key_mux_ports{ { *this, "KEY_%u_0", 0 }, { *this, "KEY_%u_1", 0 }, { *this, "KEY_%u_2", 0 }, { *this, "KEY_%u_3", 0 } }
	, m_digit(*this, "digit%u", 0U)
{
}

void cmi_music_keyboard_device::device_resolve_objects()
{
	m_cmi_txd.resolve_safe();
	m_cmi_rts.resolve_safe();
	m_kbd_txd.resolve_safe();
	m_kbd_rts.resolve_safe();
	m_digit.resolve();
}

void cmi_music_keyboard_device::device_start()
{
	m_cmi10_scnd_timer = timer_alloc(FUNC(cmi_music_keyboard_device::scnd_update), this);
	m_cmi10_scnd_timer->adjust(attotime::from_hz(4000000 / 4 / 2048 / 2), 0, attotime::from_hz(4000000 / 4 / 2048 / 2));

	m_scnd = 0;

	for (u32 i = 0; i < KEY_COUNT; i++)
	{
		m_velocity_timers[i] = timer_alloc(FUNC(cmi_music_keyboard_device::velkey_down), this);
		m_key_held[i] = false;
	}
}

TIMER_CALLBACK_MEMBER(cmi_music_keyboard_device::scnd_update)
{
	m_cmi10_pia_u20->ca1_w(m_scnd);
	m_scnd ^= 1;
	m_cmi10_pia_u21->ca1_w(m_scnd);
}

TIMER_CALLBACK_MEMBER(cmi_music_keyboard_device::velkey_down)
{
	m_key_held[param] = true;
}

/*
    PA0-7 = BKA0-7 (display)

    PB0 = DA1
    PB1 = DA0
    PB2 = CS2
    PB3 = CU2
    PB4 = CS1
    PB5 = CU1
    PB6 = CS0
    PB7 = CU0

    CB1 = /KPAD
    CB2 = /DWS
*/

void cmi_music_keyboard_device::cmi10_u20_a_w(u8 data)
{
	// low 7 bits connected to alphanumeric display data lines
	m_dp1->data_w(data & 0x7f);
	m_dp2->data_w(data & 0x7f);
	m_dp3->data_w(data & 0x7f);

	/*
	int bk = data;
	int bit = 0;

	if (BIT(bk, 3))
	    bit = BIT(input_port_read(device->machine, "KEYPAD_A"), bk & 7);
	else if (!BIT(bk, 4))
	    bit = BIT(input_port_read(device->machine, "KEYPAD_B"), bk & 7);

	pia6821_cb1_w(m_cmi10_pia_u20, 0, !bit);
	*/
}

void cmi_music_keyboard_device::cmi10_u20_b_w(u8 data)
{
}

READ_LINE_MEMBER( cmi_music_keyboard_device::cmi10_u20_cb1_r )
{
	int bk = m_cmi10_pia_u20->a_output();
	int bit = 0;

	if (BIT(bk, 3))
		bit = BIT(m_keypad_a_port->read(), bk & 7);
	else if (!BIT(bk, 4))
		bit = BIT(m_keypad_b_port->read(), bk & 7);

	return !bit;
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::cmi10_u20_cb2_w )
{
	uint8_t data = m_cmi10_pia_u20->a_output() & 0x7f;
	uint8_t b_port = m_cmi10_pia_u20->b_output();
	int addr = (BIT(b_port, 0) << 1) | BIT(b_port, 1);

	/* DP1 */
	m_dp1->ce_w(BIT(b_port, 6));
	m_dp1->cu_w(BIT(b_port, 7));
	m_dp1->wr_w(state);
	m_dp1->addr_w(addr);
	m_dp1->data_w(data);

	/* DP2 */
	m_dp2->ce_w(BIT(b_port, 4));
	m_dp2->cu_w(BIT(b_port, 5));
	m_dp2->wr_w(state);
	m_dp2->addr_w(addr);
	m_dp2->data_w(data);

	/* DP3 */
	m_dp3->ce_w(BIT(b_port, 2));
	m_dp3->cu_w(BIT(b_port, 3));
	m_dp3->wr_w(state);
	m_dp3->addr_w(addr);
	m_dp3->data_w(data);
}

template <unsigned N> void cmi_music_keyboard_device::update_dp(offs_t offset, u16 data)
{
	m_digit[(N << 2) | ((offset ^ 3) & 3)] = data;
}

/* Begin Conversion */
WRITE_LINE_MEMBER( cmi_music_keyboard_device::cmi10_u21_cb2_w )
{
	// if 0
//  state = state;
}

u32 cmi_music_keyboard_device::get_key_for_indices(int mux, int module, int key)
{
	if (mux == 3)
	{
		if (module == 2)
		{
			return KEY_F6;
		}
		return KEY_COUNT;
	}
	return KEY_F0 + (module * 24) + (mux * 8) + key;
}

u8 cmi_music_keyboard_device::cmi10_u21_a_r()
{
#if 0
//  int thld = m_cmi10_pia_u21->ca2_output();
	int sel = m_cmi10_pia_u20->a_output();
	int key = sel & 7;
	int mux = (sel >> 3) & 3;
	u8 data = 0x38; // slave keyboard not used


	for (int module = 0; module < 3; ++module)
	{
//      char keyname[16];
		u8 keyval;
		int state = 1;

		if (module == 1 && mux == 2 && key == 3)
		{
			keyval = m_analog->read();

			/* Unpressed */
			if (keyval <= 0)
				state = 1;
			/* In flight */

	#if 0
			else if (keyval <= 80)
			{
				if (thld == 1)
					state = 0;
				else
					state = 1;
			}
			/* Fully depressed */
	#endif
			else
				state = 0;

		}

		data |= state << module;
	}

	return data;
#else
	int thld = m_cmi10_pia_u21->ca2_output();
	int sel = m_cmi10_pia_u20->a_output();
	int key = sel & 7;
	int mux = (sel >> 3) & 3;
	u8 data = 0xff; // slave keyboard not used

	for (int module = 0; module < 3; ++module)
	{
		u8 keyval = m_key_mux_ports[mux][module]->read();
		u32 keyidx = get_key_for_indices(mux, module, key);
		if (keyidx < KEY_COUNT)
		{
			if (m_key_held[keyidx])
			{
				// Key is fully pressed
				data &= ~(1 << module);
			}
			else if (!BIT(keyval, key))
			{
				// User has started pressing the key
				if (m_velocity_timers[keyidx]->remaining().is_never())
				{
					data &= ~(1 << module);
					m_velocity_timers[keyidx]->adjust(attotime::from_hz(10), keyidx);
				}
				else if (thld == 0)
				{
					data &= ~(1 << module);
				}
			}
		}
	}

	/* Now do KD7 */
	{
		int bit = 0;

		if (BIT(sel, 3))
			bit = BIT(m_keypad_a_port->read(), sel & 7);
		else if (!BIT(sel, 4))
			bit = BIT(m_keypad_b_port->read(), sel & 7);

		data &= ~((bit && BIT(sel, 7)) << 7);
	}

	return ~data;
#endif
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::kbd_acia_int )
{
	m_kbd_acia_irq = state;

	if (m_kbd_acia_irq)
		m_cpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	else if (!m_cmi_acia_irq)
		m_cpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::cmi_acia_int )
{
	m_cmi_acia_irq = state;

	if (m_cmi_acia_irq)
		m_cpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	else if (!m_kbd_acia_irq)
		m_cpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::cmi_txd_w )
{
	m_cmi_txd(state);
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::cmi_rts_w )
{
	m_cmi_rts(state);
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::cmi_rxd_w )
{
	m_acia_cmi->write_rxd(state);
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::cmi_cts_w )
{
	m_acia_cmi->write_cts(state);
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::kbd_txd_w )
{
	m_kbd_txd(state);
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::kbd_rts_w )
{
	m_kbd_rts(state);
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::kbd_rxd_w )
{
	m_acia_kbd->write_rxd(state);
}

WRITE_LINE_MEMBER( cmi_music_keyboard_device::kbd_cts_w )
{
	m_acia_kbd->write_cts(state);
}

void cmi_music_keyboard_device::kbd_acia_w(offs_t offset, u8 data)
{
	m_acia_kbd->write(offset, data);
	LOG("%s: music keyboard ACIA write: %02x = %02x\n", machine().describe_context(), offset, data);
}

u8 cmi_music_keyboard_device::kbd_acia_r(offs_t offset)
{
	u8 data = m_acia_kbd->read(offset);
	LOG("%s: music keyboard ACIA read: %02x: %02x\n", machine().describe_context(), offset, data);
	return data;
}

INPUT_CHANGED_MEMBER(cmi_music_keyboard_device::key_changed)
{
	if (newval)
	{
		// Key released
		m_velocity_timers[param]->adjust(attotime::never);
	}
	m_key_held[param] = false;
}

void cmi_music_keyboard_device::muskeys_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0080, 0x0083).rw(m_cmi10_pia_u21, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(m_cmi10_pia_u20, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x00a0, 0x00a1).rw(FUNC(cmi_music_keyboard_device::kbd_acia_r), FUNC(cmi_music_keyboard_device::kbd_acia_w));
	map(0x00b0, 0x00b1).rw(m_acia_cmi, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x4000, 0x47ff).ram();
	map(0xb000, 0xb400).rom();
	map(0xf000, 0xffff).rom();
}

static INPUT_PORTS_START(cmi_music_keyboard)
	/* Keypad */
	PORT_START("KEYPAD_A")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("KEYPAD_B")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD)

	/* Master musical keyboard */
	PORT_START("KEY_0_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F0")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F0 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F0S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G0")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G0 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G0S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A1")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A1 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A1S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B1")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_B1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C1")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C1)

	PORT_START("KEY_0_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C1 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C1S)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D1")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D1 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D1S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E1")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_E1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F1")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F1 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F1S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G1")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G1 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G1S)

	PORT_START("KEY_0_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A2")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A2 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A2S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B2")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_B2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C2")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C2 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C2S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D2")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D2 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D2S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E2")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_E2)

	PORT_START("KEY_0_3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY_1_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F2")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F2 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F2S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G2")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G2 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G2S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A3")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A3 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A3S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B3")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_B3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C3")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C3)

	PORT_START("KEY_1_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C3 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C3S)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D3")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D3 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D3S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E3")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_E3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F3")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F3 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F3S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G3")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G3 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G3S)

	PORT_START("KEY_1_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A4")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A4 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A4S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B4")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_B4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C4")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C4 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C4S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D4")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D4 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D4S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E4")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_E4)

	PORT_START("KEY_1_3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY_2_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F4")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F4 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F4S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G4")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G4 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G4S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A5")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A5 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A5S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B5")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_B5)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C5")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C5)

	PORT_START("KEY_2_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C5 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C5S)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D5")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D5 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D5S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E5")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_E5)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F5")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F5 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F5S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G5")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G5)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G5 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_G5S)

	PORT_START("KEY_2_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A6")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A6 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_A6S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B6")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_B6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C6")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C6)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C6 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_C6S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D6")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D6 #")      PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_D6S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E6")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_E6)

	PORT_START("KEY_2_3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F6")        PORT_CHANGED_MEMBER(DEVICE_SELF, cmi_music_keyboard_device, key_changed, cmi_music_keyboard_device::KEY_F6)

	PORT_START("ANALOG")
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_MINMAX(0, 128) PORT_SENSITIVITY(100) PORT_KEYDELTA(50)
INPUT_PORTS_END

ioport_constructor cmi_music_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cmi_music_keyboard);
}

void cmi_music_keyboard_device::device_add_mconfig(machine_config &config)
{
	M6802(config, m_cpu, 4_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &cmi_music_keyboard_device::muskeys_map);

	PIA6821(config, m_cmi10_pia_u20);
	m_cmi10_pia_u20->readcb1_handler().set(FUNC(cmi_music_keyboard_device::cmi10_u20_cb1_r));
	m_cmi10_pia_u20->writepa_handler().set(FUNC(cmi_music_keyboard_device::cmi10_u20_a_w));
	m_cmi10_pia_u20->writepb_handler().set(FUNC(cmi_music_keyboard_device::cmi10_u20_b_w));
	m_cmi10_pia_u20->cb2_handler().set(FUNC(cmi_music_keyboard_device::cmi10_u20_cb2_w));

	PIA6821(config, m_cmi10_pia_u21);
	m_cmi10_pia_u21->readpa_handler().set(FUNC(cmi_music_keyboard_device::cmi10_u21_a_r));
	m_cmi10_pia_u21->cb2_handler().set(FUNC(cmi_music_keyboard_device::cmi10_u21_cb2_w));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 1.8432_MHz_XTAL / 12));
	acia_clock.signal_handler().set(m_acia_kbd, FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia_kbd, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia_cmi, FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia_cmi, FUNC(acia6850_device::write_txc));

	ACIA6850(config, m_acia_cmi);
	m_acia_cmi->txd_handler().set(FUNC(cmi_music_keyboard_device::cmi_txd_w));
	m_acia_cmi->rts_handler().set(FUNC(cmi_music_keyboard_device::cmi_rts_w));
	m_acia_cmi->irq_handler().set(FUNC(cmi_music_keyboard_device::cmi_acia_int));

	ACIA6850(config, m_acia_kbd);
	m_acia_kbd->txd_handler().set(FUNC(cmi_music_keyboard_device::kbd_txd_w));
	m_acia_kbd->rts_handler().set(FUNC(cmi_music_keyboard_device::kbd_rts_w));
	m_acia_kbd->irq_handler().set(FUNC(cmi_music_keyboard_device::kbd_acia_int));

	/* alpha-numeric display */
	DL1416T(config, m_dp1, u32(0));
	m_dp1->update().set(FUNC(cmi_music_keyboard_device::update_dp<0>));
	DL1416T(config, m_dp2, u32(0));
	m_dp2->update().set(FUNC(cmi_music_keyboard_device::update_dp<1>));
	DL1416T(config, m_dp3, u32(0));
	m_dp3->update().set(FUNC(cmi_music_keyboard_device::update_dp<2>));
}

ROM_START( cmi_mkbd )
	// Both of these dumps have been trimmed to size from within a roughly 2x-bigger file.
	// The actual size is known based on the format apparently used by the dumping device.
	ROM_REGION( 0x10000, "kbdcpu", 0 )
	ROM_LOAD( "velkeysd.bin", 0xb000, 0x0400, CRC(9b636781) SHA1(be29a72a1d6d313dafe0b63951b5e3e18ddb9a21) )
	ROM_LOAD( "kbdioa.bin",   0xfc00, 0x0400, CRC(a5cbe218) SHA1(bc6784aaa5697c28eab126e20500139b8d0c1f50) )
ROM_END

const tiny_rom_entry *cmi_music_keyboard_device::device_rom_region() const
{
	return ROM_NAME(cmi_mkbd);
}
