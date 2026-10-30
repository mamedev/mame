// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "neomania_adapter.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEOMANIA_ADAPTER, neomania_adapter_device, "neomania_adapter", "Neo Mania Adapter JAMMA board")

neomania_adapter_device::neomania_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEOMANIA_ADAPTER, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_in(*this, "IN%u", 0U),
	m_select_in(0),
	m_init(0),
	m_strobe(0),
	m_autofd(0),
	m_has_inited(false)
{ }

// "Testing I/O board input bits" wants IN0~3 and IN7 to return low
static INPUT_PORTS_START( neomania_adapter )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Quit game")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("IN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN6")
	// untested by ppm.exe, but neoemu.exe wants it low otherwise it will enter in work RAM test
	// (probably something related to the BIOS hack that MAME had back then)
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN7")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END


ioport_constructor neomania_adapter_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( neomania_adapter );
}

void neomania_adapter_device::device_start()
{
	save_item(NAME(m_select_in));
	save_item(NAME(m_init));
	save_item(NAME(m_strobe));
	save_item(NAME(m_autofd));

	save_item(NAME(m_ddr));
}

void neomania_adapter_device::device_reset()
{
	m_select_in = m_init = 0;
	m_autofd = m_strobe = 0;
	m_has_inited = false;

	m_ddr = 0xff;
}

void neomania_adapter_device::input_strobe(int state)
{
	LOG("input_strobe %d\n", state);
	m_strobe = !state;
	update_ack();
}

void neomania_adapter_device::input_select_in(int state)
{
	m_select_in = state;
	LOG("input_select_in %d\n", state);
	update_ack();
}

void neomania_adapter_device::input_autofd(int state)
{
	m_autofd = !state;
	LOG("input_autofd %d\n", state);
	update_ack();
}

void neomania_adapter_device::input_init(int state)
{
	LOG("input_init %d\n", state);

	m_init = state;
	update_ack();

	// "Testing I/O board connection" at bp 100027e2
	// this is enough to make it output a 0x02 in the buffer, perhaps different board revs?
	if (state && !m_select_in)
	{
		output_fault(0);
		output_select(0);
		output_perror(0);
		output_busy(0);
		// avoid BIOS interfering with actual I/O reading
		m_has_inited = true;
	}
}

// "Testing I/O board communications"
// wants an encoded ACK depending on control writes
// 0xec -> 0xe0 -> 0xe1 -> ... -> 0xe7
void neomania_adapter_device::update_ack()
{
	const int id_table[8] = { 1, 1, 0, 0, 1, 0, 1, 0 };
	u8 id_ptr = (m_strobe | (m_autofd << 1) | (m_init << 2)) & 7;
	int result = id_table[id_ptr];
	LOG("ACK %d -> %d\n", id_ptr, result);
	output_ack(result);

	if (m_has_inited)
	{
		// "Testing I/O board input bits"
		// TODO: doesn't work, BIOS writes a 0 to pc_lpt data_w, which craps this out the window
		const u8 data_in = m_in[id_ptr]->read();

		LOG("read input @ %d -> %02x\n", id_ptr, data_in);

		output_data0(BIT(data_in, 0));
		output_data1(BIT(data_in, 1));
		output_data2(BIT(data_in, 2));
		output_data3(BIT(data_in, 3));
		output_data4(BIT(data_in, 4));
		output_data5(BIT(data_in, 5));
		output_data6(BIT(data_in, 6));
		output_data7(BIT(data_in, 7));
	}
}
