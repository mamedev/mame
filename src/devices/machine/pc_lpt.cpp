// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    IBM-PC printer interface

***************************************************************************/

#include "emu.h"
#include "pc_lpt.h"


DEFINE_DEVICE_TYPE(PC_LPT, pc_lpt_device, "pc_lpt", "PC LPT")

pc_lpt_device::pc_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PC_LPT, tag, owner, clock),
	m_irq(1),
	m_data(0xff), m_control(0),
	m_irq_enabled(1),
	m_centronics_ack(1),
	m_irq_handler(*this),
	m_cent_data_in(*this, "cent_data_in"),
	m_cent_data_out(*this, "cent_data_out"),
	m_cent_status_in(*this, "cent_status_in"),
	m_cent_ctrl_in(*this, "cent_ctrl_in"),
	m_cent_ctrl_out(*this, "cent_ctrl_out")
{
}

void pc_lpt_device::device_start()
{
	m_irq_handler.resolve_safe();

	save_item(NAME(m_irq));
	save_item(NAME(m_data));
	save_item(NAME(m_control));
	save_item(NAME(m_centronics_ack));
	save_item(NAME(m_irq_enabled));

	m_cent_data_out->write(m_data);
}

void pc_lpt_device::device_reset()
{
	m_control = ~(0 ^ CONTROL_INIT);
	m_cent_ctrl_out->write(m_control);
}

void pc_lpt_device::device_add_mconfig(machine_config &config)
{
	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.set_data_input_buffer(m_cent_data_in);
	centronics.fault_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit3));
	centronics.select_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit4));
	centronics.perror_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit5));
	centronics.ack_handler().set(FUNC(pc_lpt_device::write_centronics_ack));
	centronics.busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit7));

	centronics.strobe_handler().set(m_cent_ctrl_in, FUNC(input_buffer_device::write_bit0));
	centronics.autofd_handler().set(m_cent_ctrl_in, FUNC(input_buffer_device::write_bit1));
	centronics.init_handler().set(m_cent_ctrl_in, FUNC(input_buffer_device::write_bit2));
	centronics.select_in_handler().set(m_cent_ctrl_in, FUNC(input_buffer_device::write_bit3));

	INPUT_BUFFER(config, m_cent_data_in);
	INPUT_BUFFER(config, m_cent_ctrl_in);
	INPUT_BUFFER(config, m_cent_status_in);

	OUTPUT_LATCH(config, m_cent_data_out);
	centronics.set_output_latch(*m_cent_data_out);

	OUTPUT_LATCH(config, m_cent_ctrl_out);
	m_cent_ctrl_out->bit_handler<0>().set("centronics", FUNC(centronics_device::write_strobe));
	m_cent_ctrl_out->bit_handler<1>().set("centronics", FUNC(centronics_device::write_autofd));
	m_cent_ctrl_out->bit_handler<2>().set("centronics", FUNC(centronics_device::write_init));
	m_cent_ctrl_out->bit_handler<3>().set("centronics", FUNC(centronics_device::write_select_in));
	m_cent_ctrl_out->bit_handler<4>().set(FUNC(pc_lpt_device::write_irq_enabled));
}


READ8_MEMBER( pc_lpt_device::data_r )
{
	// pull up mechanism for input lines, zeros are provided by peripheral
	return m_data & m_cent_data_in->read();
}

WRITE8_MEMBER( pc_lpt_device::data_w )
{
	m_data = data;
	m_cent_data_out->write(m_data);
}

READ8_MEMBER( pc_lpt_device::status_r )
{
	return m_cent_status_in->read() ^ STATUS_BUSY;
}

READ8_MEMBER( pc_lpt_device::control_r )
{
	return ~((m_control & m_cent_ctrl_in->read() & 0x3f) ^ CONTROL_INIT);
}

WRITE8_MEMBER( pc_lpt_device::control_w )
{
	//  logerror("pc_lpt_control_w: 0x%02x\n", data);

	m_control = ~(data ^ CONTROL_INIT);
	m_cent_ctrl_out->write(m_control);
}

READ8_MEMBER( pc_lpt_device::read )
{
	switch (offset)
	{
	case 0: return data_r(space, 0);
	case 1: return status_r(space, 0);
	case 2: return control_r(space, 0);
	}

	/* if we reach this its an error */
	logerror("PC-LPT %s: Read from invalid offset %x\n", tag(), offset);

	return 0xff;
}

WRITE8_MEMBER( pc_lpt_device::write )
{
	switch (offset)
	{
	case 0: data_w(space, 0, data); break;
	case 1: break;
	case 2: control_w(space, 0, data); break;
	}
}

void pc_lpt_device::update_irq()
{
	int irq = 1; // high impedance
	if (!m_irq_enabled)
	{
		irq = m_centronics_ack;
	}

	if (m_irq != irq)
	{
		m_irq = irq;
		m_irq_handler(!irq);
	}
}

WRITE_LINE_MEMBER( pc_lpt_device::write_irq_enabled )
{
	m_irq_enabled = state;
	update_irq();
}

WRITE_LINE_MEMBER( pc_lpt_device::write_centronics_ack )
{
	m_centronics_ack = state;
	m_cent_status_in->write_bit6(state);
	update_irq();
}
