// license:BSD-3-Clause
// copyright-holders:eziochiu
// LPC2132 Vectored Interrupt Controller (VIC)
// For ARM7TDMI core interrupt controller

#include "emu.h"
#include "lpc2132_vic.h"

#define LOG_IRQ    (1U << 1)
#define LOG_REG    (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(LPC2132_VIC, lpc2132_vic_device, "lpc2132_vic", "NXP LPC2132 VIC")

lpc2132_vic_device::lpc2132_vic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LPC2132_VIC, tag, owner, clock)
	, m_irq_out(*this)
	, m_fiq_out(*this)
	, m_def_vect_addr_cb(*this)
{
}

void lpc2132_vic_device::regs_map(address_map &map)
{
	map(0x000, 0x003).r(FUNC(lpc2132_vic_device::irq_status_r));       // VICIRQStatus
	map(0x004, 0x007).r(FUNC(lpc2132_vic_device::fiq_status_r));       // VICFIQStatus
	map(0x008, 0x00b).r(FUNC(lpc2132_vic_device::raw_intr_r));         // VICRawIntr
	map(0x00c, 0x00f).rw(FUNC(lpc2132_vic_device::int_select_r), FUNC(lpc2132_vic_device::int_select_w));   // VICIntSelect
	map(0x010, 0x013).rw(FUNC(lpc2132_vic_device::int_enable_r), FUNC(lpc2132_vic_device::int_enable_w));   // VICIntEnable
	map(0x014, 0x017).w(FUNC(lpc2132_vic_device::int_en_clear_w));     // VICIntEnClear
	map(0x018, 0x01b).rw(FUNC(lpc2132_vic_device::soft_int_r), FUNC(lpc2132_vic_device::soft_int_w));       // VICSoftInt
	map(0x01c, 0x01f).w(FUNC(lpc2132_vic_device::soft_int_clear_w));   // VICSoftIntClear
	map(0x020, 0x023).rw(FUNC(lpc2132_vic_device::protection_r), FUNC(lpc2132_vic_device::protection_w));   // VICProtection
	map(0x030, 0x033).rw(FUNC(lpc2132_vic_device::vect_addr_r), FUNC(lpc2132_vic_device::vect_addr_w));     // VICVectAddr
	map(0x034, 0x037).rw(FUNC(lpc2132_vic_device::def_vect_addr_r), FUNC(lpc2132_vic_device::def_vect_addr_w)); // VICDefVectAddr
	map(0x100, 0x13f).rw(FUNC(lpc2132_vic_device::vect_addr_slot_r), FUNC(lpc2132_vic_device::vect_addr_slot_w)); // VICVectAddr0-15
	map(0x200, 0x23f).rw(FUNC(lpc2132_vic_device::vect_cntl_r), FUNC(lpc2132_vic_device::vect_cntl_w));     // VICVectCntl0-15
}

void lpc2132_vic_device::device_start()
{
	save_item(NAME(m_vic_irq_status));
	save_item(NAME(m_vic_fiq_status));
	save_item(NAME(m_vic_raw_intr));
	save_item(NAME(m_vic_int_select));
	save_item(NAME(m_vic_int_enable));
	save_item(NAME(m_vic_soft_int));
	save_item(NAME(m_vic_protection));
	save_item(NAME(m_vic_vect_addr));
	save_item(NAME(m_vic_vect_cntl));
	save_item(NAME(m_vic_vect_addr_cur));
	save_item(NAME(m_vic_def_vect_addr));
}

void lpc2132_vic_device::device_reset()
{
	m_vic_irq_status = 0;
	m_vic_fiq_status = 0;
	m_vic_raw_intr = 0;
	m_vic_int_select = 0;
	m_vic_int_enable = 0;
	m_vic_soft_int = 0;
	m_vic_protection = 0;
	m_vic_vect_addr_cur = 0;
	m_vic_def_vect_addr = 0;

	for (auto &elem : m_vic_vect_addr) { elem = 0; }
	for (auto &elem : m_vic_vect_cntl) { elem = 0; }

	LOGMASKED(LOG_IRQ, "LPC2132 VIC: Reset complete\n");
}

void lpc2132_vic_device::update_interrupt_lines()
{
	// Calculate IRQ pending: enabled interrupts & raw interrupts & not FIQ selected
	uint32_t irq_pending = m_vic_int_enable & m_vic_raw_intr & ~m_vic_int_select;

	// Calculate FIQ pending: enabled interrupts & raw interrupts & FIQ selected
	uint32_t fiq_pending = m_vic_int_enable & m_vic_raw_intr & m_vic_int_select;

	m_vic_irq_status = irq_pending;

	m_vic_fiq_status = fiq_pending;

	// Output IRQ status
	m_irq_out(irq_pending ? ASSERT_LINE : CLEAR_LINE);

	// Output FIQ status
	m_fiq_out(fiq_pending ? ASSERT_LINE : CLEAR_LINE);
}

uint32_t lpc2132_vic_device::read_vector_address()
{
	uint32_t irq_pending = m_vic_int_enable & m_vic_raw_intr & ~m_vic_int_select;

	if (irq_pending == 0)
	{
		m_vic_vect_addr_cur = m_vic_def_vect_addr;
		return m_vic_vect_addr_cur;
	}

	// Iterate through 16 vector slots to find matching interrupt
	for (int i = 0; i < 16; i++)
	{
		if (m_vic_vect_cntl[i] & 0x20) // Check enable bit
		{
			uint32_t irq_source = m_vic_vect_cntl[i] & 0x1f;

			if (irq_pending & (1 << irq_source))
			{
				m_vic_vect_addr_cur = m_vic_vect_addr[i];
				return m_vic_vect_addr_cur;
			}
		}
	}

	// No matching vectored interrupt, return default address
	m_vic_vect_addr_cur = m_vic_def_vect_addr;
	return m_vic_vect_addr_cur;
}

void lpc2132_vic_device::set_irq(int line, int state)
{
	if (line < 0 || line >= 32)
		return;

	if (state == ASSERT_LINE)
		m_vic_raw_intr |= (1 << line);
	else
		m_vic_raw_intr &= ~(1 << line);

	update_interrupt_lines();
}

// Register read functions
uint32_t lpc2132_vic_device::irq_status_r()
{
	return m_vic_irq_status;
}

uint32_t lpc2132_vic_device::fiq_status_r()
{
	return m_vic_fiq_status;
}

uint32_t lpc2132_vic_device::raw_intr_r()
{
	return m_vic_raw_intr;
}

uint32_t lpc2132_vic_device::int_select_r()
{
	return m_vic_int_select;
}

uint32_t lpc2132_vic_device::int_enable_r()
{
	return m_vic_int_enable;
}

uint32_t lpc2132_vic_device::soft_int_r()
{
	return m_vic_soft_int;
}

uint32_t lpc2132_vic_device::protection_r()
{
	return m_vic_protection;
}

uint32_t lpc2132_vic_device::vect_addr_r()
{
	if (!machine().side_effects_disabled())
		return read_vector_address();

	// when side effects are disabled (e.g. debugger), return current address without updating state
	return m_vic_vect_addr_cur;
}

uint32_t lpc2132_vic_device::def_vect_addr_r()
{
	return m_vic_def_vect_addr;
}

uint32_t lpc2132_vic_device::vect_addr_slot_r(offs_t offset)
{
	return m_vic_vect_addr[offset];
}

uint32_t lpc2132_vic_device::vect_cntl_r(offs_t offset)
{
	return m_vic_vect_cntl[offset];
}

// Register write functions
void lpc2132_vic_device::int_select_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vic_int_select);
	update_interrupt_lines();
}

void lpc2132_vic_device::int_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_vic_int_enable |= (data & mem_mask);
	update_interrupt_lines();
}

void lpc2132_vic_device::int_en_clear_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_vic_int_enable &= ~(data & mem_mask);
	update_interrupt_lines();
}

void lpc2132_vic_device::soft_int_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_vic_soft_int |= (data & mem_mask);
	m_vic_raw_intr |= m_vic_soft_int;
	update_interrupt_lines();
}

void lpc2132_vic_device::soft_int_clear_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_vic_soft_int &= ~(data & mem_mask);
	m_vic_raw_intr &= ~(data & mem_mask);
	update_interrupt_lines();
}

void lpc2132_vic_device::protection_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vic_protection);
}

void lpc2132_vic_device::vect_addr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// Writing any value signals end of interrupt handling
	// Clear the highest priority interrupt currently being processed
	// We only manually clear IRQ 16 (EINT2) here as part of the handshake mechanism
	if (m_vic_raw_intr & (1 << 16))
	{
		m_vic_raw_intr &= ~(1 << 16);
		update_interrupt_lines();
	}
}

void lpc2132_vic_device::def_vect_addr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vic_def_vect_addr);
	m_def_vect_addr_cb(m_vic_def_vect_addr);
}

void lpc2132_vic_device::vect_addr_slot_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vic_vect_addr[offset]);
}

void lpc2132_vic_device::vect_cntl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vic_vect_cntl[offset]);
}
