// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    IBM-PC printer interface

***************************************************************************/

#include "emu.h"
#include "lpt.h"
#include "machine/pc_lpt.h"

DEFINE_DEVICE_TYPE(ISA8_LPT, isa8_lpt_device, "isa_lpt", "Printer Adapter")

isa8_lpt_device::isa8_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_LPT, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_is_primary(false)
{
}

void isa8_lpt_device::device_add_mconfig(machine_config &config)
{
	pc_lpt_device &lpt(PC_LPT(config, "lpt"));
	lpt.irq_handler().set(FUNC(isa8_lpt_device::pc_cpu_line));
}

static INPUT_PORTS_START( lpt_dsw )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Base address")
	PORT_DIPSETTING(    0x00, "0x378" )
	PORT_DIPSETTING(    0x01, "0x278" )
INPUT_PORTS_END

ioport_constructor isa8_lpt_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( lpt_dsw );
}

void isa8_lpt_device::device_start()
{
	set_isa_device();
}

void isa8_lpt_device::device_reset()
{
	m_is_primary = (ioport("DSW")->read() & 1) ? false : true;
	pc_lpt_device &lpt(*subdevice<pc_lpt_device>("lpt"));
	if (m_is_primary)
		m_isa->install_device(0x0378, 0x037b, read8_delegate(lpt, FUNC(pc_lpt_device::read)), write8_delegate(lpt, FUNC(pc_lpt_device::write)));
	else
		m_isa->install_device(0x0278, 0x027b, read8_delegate(lpt, FUNC(pc_lpt_device::read)), write8_delegate(lpt, FUNC(pc_lpt_device::write)));
}

WRITE_LINE_MEMBER(isa8_lpt_device::pc_cpu_line)
{
	if (is_primary())
		m_isa->irq7_w(state);
	else
		m_isa->irq5_w(state);
}
