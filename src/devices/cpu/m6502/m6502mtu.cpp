// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502mtu.cpp

	M6502 with MTU external banking hardware

***************************************************************************/

#include "emu.h"
#include "m6502mtu.h"

DEFINE_DEVICE_TYPE(M6502MTU, m6502mtu_device, "m6502mtu", "M6502 with MTU banking")

m6502mtu_device::m6502mtu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6502MTU, tag, owner, clock)
{
	program_config.m_addr_width = 18;
	sprogram_config.m_addr_width = 18;
}

void m6502mtu_device::device_start()
{
	mintf = nullptr;
	space(AS_PROGRAM).specific(cprogram);
	space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).specific(csprogram);

	save_item(NAME(low_instruction));
	save_item(NAME(interrupt_mode));
	save_item(NAME(pbank));
	save_item(NAME(dbank));

	init();
}

void m6502mtu_device::device_reset()
{
	m6502_device::device_reset();
	pbank = 0;
	dbank = 0;
	low_instruction = false;
	interrupt_mode = false;
}


void m6502mtu_device::prefetchd()
{
	sync = true;
	sync_w(ASSERT_LINE);
	NPC = PC;
	low_instruction = PC < 0x200;
	IR = csprogram.read_byte(low_instruction || interrupt_mode ? PC : PC | pbank);
	sync = false;
	sync_w(CLEAR_LINE);

	if((nmi_pending || ((irq_state || apu_irq_state) && !(P & F_I))) && !inhibit_interrupts) {
		irq_taken = true;
		IR = 0x00;
	} else
		PC++;
}

void m6502mtu_device::prefetchd_noirq()
{
	sync = true;
	sync_w(ASSERT_LINE);
	NPC = PC;
	low_instruction = PC < 0x200;
	IR = csprogram.read_byte(low_instruction || interrupt_mode ? PC : PC | pbank);
	sync = false;
	sync_w(CLEAR_LINE);
	PC++;
}

#include "cpu/m6502/m6502mtu.hxx"
