// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Interrupt controller for H89

****************************************************************************/

#include "emu.h"

#include "intr_cntrl.h"

DEFINE_DEVICE_TYPE(HEATH_INTR_CNTRL, heath_intr_cntrl, "heath_intr_cntrl", "Heath H/Z-89 Interrupt Controller");
DEFINE_DEVICE_TYPE(HEATH_Z37_INTR_CNTRL, z37_intr_cntrl, "heath_z37_intr_cntrl", "Heath H/Z-89 with Z-37 Interrupt Controller");

heath_intr_cntrl::heath_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: heath_intr_cntrl(mconfig, HEATH_INTR_CNTRL, tag, owner, clock)
{
}

heath_intr_cntrl::heath_intr_cntrl(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, 0)
	, m_intr_inst(*this)
{
}

void heath_intr_cntrl::device_start()
{

}

void heath_intr_cntrl::device_resolve_objects()
{
	m_intr_inst.resolve_safe();
}

void heath_intr_cntrl::raise_irq(uint8_t level)
{
	uint8_t inst = 0xc7 | ((level & 0x7) << 3);

	set_instr(inst);

}

void heath_intr_cntrl::set_instr(uint8_t data)
{
	m_intr_inst(data);
}

z37_intr_cntrl::z37_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: heath_intr_cntrl(mconfig, HEATH_Z37_INTR_CNTRL, tag, owner, clock)
{
}

void z37_intr_cntrl::raise_irq(uint8_t level)
{
	if (!interrupts_blocked)
	{
		heath_intr_cntrl::raise_irq(level);
	}
}

void z37_intr_cntrl::set_drq(uint8_t data)
{
	if (data == ASSERT_LINE)
	{
		// execute an EI
		set_instr(0xfb);
	}
}


void z37_intr_cntrl::set_intrq(uint8_t data)
{
	if (data == ASSERT_LINE)
	{
		// RST 20H (Interrupt 4)
		set_instr(0xe7);
	}
}

void z37_intr_cntrl::device_start()
{
	save_item(NAME(interrupts_blocked));

	interrupts_blocked = false;
}

void z37_intr_cntrl::block_interrupts(uint8_t block)
{
	interrupts_blocked = (block == ASSERT_LINE);
}
