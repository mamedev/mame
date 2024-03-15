// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_intc.cpp

    SH interrupt controllers family

***************************************************************************/

#include "emu.h"
#include "sh_intc.h"

#include "sh7042.h"

DEFINE_DEVICE_TYPE(SH_INTC, sh_intc_device, "sh_intc", "SH interrupt controller")

sh_intc_device::sh_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SH_INTC, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG)
{
}

void sh_intc_device::device_start()
{
}

void sh_intc_device::device_reset()
{
}

int sh_intc_device::interrupt_taken(int vector)
{
	return 0;
}

void sh_intc_device::internal_interrupt(int vector)
{
	logerror("Internal interrupt %d\n", vector);
}

void sh_intc_device::set_input(int inputnum, int state)
{
}

u16 sh_intc_device::icr_r()
{
	return 0;
}

void sh_intc_device::icr_w(offs_t, u16 data, u16 mem_mask)
{
	logerror("icr_w %04x @ %04x\n", data, mem_mask);
}

u16 sh_intc_device::isr_r()
{
	return 0;
}

void sh_intc_device::isr_w(offs_t, u16 data, u16 mem_mask)
{
	logerror("isr_w %04x @ %04x\n", data, mem_mask);
}

u16 sh_intc_device::ipr_r(offs_t offset)
{
	return 0;
}

void sh_intc_device::ipr_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("ipr_w %x, %04x @ %04x\n", offset, data, mem_mask);
}

