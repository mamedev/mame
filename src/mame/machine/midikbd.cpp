// license:BSD-3-Clause
// copyright-holders:Carl
#include "machine/midikbd.h"

const device_type MIDI_KBD = &device_creator<midi_keyboard_device>;

midi_keyboard_device::midi_keyboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MIDI_KBD, "Generic MIDI Keyboard", tag, owner, clock, "midi_kbd", __FILE__),
	device_serial_interface(mconfig, *this),
	m_out_tx_func(*this),
	m_keyboard(*this, "KEYBOARD")
{
}

void midi_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(id)
	{
		device_serial_interface::device_timer(timer, id, param, ptr);
	}
	else
	{
		const int keyboard_notes[24] =
		{
			0x3c,   // C1
			0x3d,   // C1#
			0x3e,   // D1
			0x3f,   // D1#
			0x40,   // E1
			0x41,   // F1
			0x42,   // F1#
			0x43,   // G1
			0x44,   // G1#
			0x45,   // A1
			0x46,   // A1#
			0x47,   // B1
			0x48,   // C2
			0x49,   // C2#
			0x4a,   // D2
			0x4b,   // D2#
			0x4c,   // E2
			0x4d,   // F2
			0x4e,   // F2#
			0x4f,   // G2
			0x50,   // G2#
			0x51,   // A2
			0x52,   // A2#
			0x53,   // B2
		};

		int i;

		UINT32 kbstate = m_keyboard->read();
		if(kbstate != m_keyboard_state)
		{
			for (i=0; i < 24; i++)
			{
				int kbnote = keyboard_notes[i];

				if ((m_keyboard_state & (1 << i)) != 0 && (kbstate & (1 << i)) == 0)
				{
					// key was on, now off -> send Note Off message
					push_tx(0x80);
					push_tx(kbnote);
					push_tx(0x7f);
				}
				else if ((m_keyboard_state & (1 << i)) == 0 && (kbstate & (1 << i)) != 0)
				{
					// key was off, now on -> send Note On message
					push_tx(0x90);
					push_tx(kbnote);
					push_tx(0x7f);
				}
			}
		}
		else
			// no messages, send Active Sense message instead
			push_tx(0xfe);

		m_keyboard_state = kbstate;
		if(is_transmit_register_empty())
			tra_complete();
	}
}

void midi_keyboard_device::device_start()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1); //8N1?
	set_tra_rate(clock());
	m_out_tx_func.resolve_safe();
	m_head = m_tail = 0;
	m_keyboard_timer = timer_alloc();
	m_keyboard_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

void midi_keyboard_device::tra_callback()
{
	m_out_tx_func(transmit_register_get_data_bit());
}

void midi_keyboard_device::tra_complete()
{
	if(m_head != m_tail)
	{
		transmit_register_setup(m_buffer[m_tail]);
		++m_tail %= 16;
	}
}

INPUT_PORTS_START(midi_keyboard)
	PORT_START("KEYBOARD")
	PORT_BIT( 0x000001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x000002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C1#") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D1") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D1#") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 E1") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F1") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F1#") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G1") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G1#") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A1#") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 B1") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C2") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C2#") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D2") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D2#") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 E2") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F2") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F2#") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G2#") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A2") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x400000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A2#") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x800000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 B2") PORT_CODE(KEYCODE_N)
INPUT_PORTS_END

ioport_constructor midi_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(midi_keyboard);
}
