// license:BSD-3-Clause
// copyright-holders:Devin Acker
/**********************************************************************

	Skeleton for NEC PC-6022 Color Plotter Printer

	User manual:
	https://archive.org/details/nec-pc-6022-color-plotter-printer-users-manual

	Page width is 96mm / 480px.
	CPU port A controls the two steppers, and port C controls raising/lowering the pen.
	Pen/color selection happens by moving the pen to the left of the print area, then
	moving it back and forth 3 times per color change.

**********************************************************************/

#include "emu.h"
#include "pc6022.h"

//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PC6022, pc6022_device, "pc6022", "NEC PC-6022 Color Plotter Printer")


ROM_START( pc6022 )
	ROM_REGION(0x1000, "cpu", 0)
	ROM_LOAD("upd7801g-138.bin", 0x0000, 0x1000, CRC(0a294989) SHA1(fab85e88566ffa10923dec3a1a6b0d8346bc69ec))
ROM_END

const tiny_rom_entry *pc6022_device::device_rom_region() const
{
	return ROM_NAME( pc6022 );
}


INPUT_PORTS_START( pc6022 )
	PORT_START("PA")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_OUTPUT ) // X stepper (pattern is 5, 6, A, 9 for positive/rightward movement)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_OUTPUT ) // Y stepper (page feed)

	PORT_START("PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pen Right");
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pen Left");
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pen Down / Paper Feed");
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pen Up");
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_TOGGLE PORT_NAME("Step");
	PORT_DIPSETTING( 0x00, "1" )
	PORT_DIPSETTING( 0x10, "2" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_TOGGLE PORT_NAME("Character Set");
	PORT_DIPSETTING( 0x00, "Hiragana" )
	PORT_DIPSETTING( 0x20, "Katakana" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Color Change")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pen Change")

	PORT_START("PC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT )  // TODO (stop/online LED?)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_KEYPAD )  PORT_NAME("Stop")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // TODO
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_DEVICE_MEMBER("busy", FUNC(input_merger_device::in_w<1>))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_MEMBER(FUNC(pc6022_device::ack_w));
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_MEMBER(FUNC(pc6022_device::pen_ctrl_w));
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // TODO
INPUT_PORTS_END

ioport_constructor pc6022_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc6022 );
}


pc6022_device::pc6022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PC6022, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_cpu(*this, "cpu"),
	m_busy(*this, "busy")
{
}

void pc6022_device::io_map(address_map &map)
{
	map.global_mask(0x00ff);
	// TODO: does the address actually matter?
	map(0x00, 0x00).r(FUNC(pc6022_device::data_r));
}

void pc6022_device::device_add_mconfig(machine_config &config)
{
	UPD7801(config, m_cpu, 4'000'000); // actual clock unknown
	m_cpu->set_addrmap(AS_IO, &pc6022_device::io_map);
	m_cpu->pa_out_cb().set_ioport("PA");
	m_cpu->pb_in_cb().set_ioport("PB");
	m_cpu->pc_in_cb().set_ioport("PC");
	m_cpu->pc_out_cb().set_ioport("PC");

	// firmware only manually polls INT1, but BUSY needs to go high faster than that
	INPUT_MERGER_ANY_HIGH(config, m_busy);
	m_busy->output_handler().set(FUNC(pc6022_device::output_busy));
}

void pc6022_device::device_start()
{
	m_data = 0;
	m_pen_ctrl = 0;
	m_pen_down = 0;
	m_ack = 0;
	m_strobe = 0;

	save_item(NAME(m_data));
	save_item(NAME(m_pen_ctrl));
	save_item(NAME(m_pen_down));
	save_item(NAME(m_ack));
	save_item(NAME(m_strobe));
}

void pc6022_device::device_reset()
{
	output_select(1);
	output_busy(0);
}

void pc6022_device::input_strobe(int state)
{
	if (!state && m_strobe)
	{
		m_busy->in_set<0>();
		m_cpu->set_input_line(UPD7810_INTF1, ASSERT_LINE);
	}

	m_strobe = state;
}

void pc6022_device::ack_w(int state)
{
	if (!state && m_ack)
	{
		m_busy->in_clear<0>();
		m_cpu->set_input_line(UPD7810_INTF1, CLEAR_LINE);
	}

	m_ack = state;
}

void pc6022_device::pen_ctrl_w(int state)
{
	if (BIT(state & ~m_pen_ctrl, 0))
	{
		m_pen_down = 0;
		LOG("pen up\n");
	}
	else if (BIT(state & ~m_pen_ctrl, 1))
	{
		m_pen_down = 1;
		LOG("pen down\n");
	}

	m_pen_ctrl = state;
}

u8 pc6022_device::data_r()
{
	return m_data;
}
