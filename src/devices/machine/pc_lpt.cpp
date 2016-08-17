// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    IBM-PC printer interface

***************************************************************************/

#include "emu.h"
#include "pc_lpt.h"


const device_type PC_LPT = &device_creator<pc_lpt_device>;

pc_lpt_device::pc_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC_LPT, "PC-LPT", tag, owner, clock, "pc_lpt", __FILE__),
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

static MACHINE_CONFIG_FRAGMENT( pc_lpt )
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_DATA_INPUT_BUFFER("cent_data_in")
	MCFG_CENTRONICS_FAULT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit3))
	MCFG_CENTRONICS_SELECT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit4))
	MCFG_CENTRONICS_PERROR_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit5))
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(pc_lpt_device, write_centronics_ack))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit7))

	MCFG_CENTRONICS_STROBE_HANDLER(DEVWRITELINE("cent_ctrl_in", input_buffer_device, write_bit0))
	MCFG_CENTRONICS_AUTOFD_HANDLER(DEVWRITELINE("cent_ctrl_in", input_buffer_device, write_bit1))
	MCFG_CENTRONICS_INIT_HANDLER(DEVWRITELINE("cent_ctrl_in", input_buffer_device, write_bit2))
	MCFG_CENTRONICS_SELECT_IN_HANDLER(DEVWRITELINE("cent_ctrl_in", input_buffer_device, write_bit3))

	MCFG_DEVICE_ADD("cent_data_in", INPUT_BUFFER, 0)
	MCFG_DEVICE_ADD("cent_ctrl_in", INPUT_BUFFER, 0)
	MCFG_DEVICE_ADD("cent_status_in", INPUT_BUFFER, 0)

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_DEVICE_ADD("cent_ctrl_out", OUTPUT_LATCH, 0)
	MCFG_OUTPUT_LATCH_BIT0_HANDLER(DEVWRITELINE("centronics", centronics_device, write_strobe))
	MCFG_OUTPUT_LATCH_BIT1_HANDLER(DEVWRITELINE("centronics", centronics_device, write_autofd))
	MCFG_OUTPUT_LATCH_BIT2_HANDLER(DEVWRITELINE("centronics", centronics_device, write_init))
	MCFG_OUTPUT_LATCH_BIT3_HANDLER(DEVWRITELINE("centronics", centronics_device, write_select_in))
	MCFG_OUTPUT_LATCH_BIT4_HANDLER(WRITELINE(pc_lpt_device, write_irq_enabled))
MACHINE_CONFIG_END

machine_config_constructor pc_lpt_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pc_lpt  );
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
