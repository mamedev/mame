// license:BSD-3-Clause
// copyright-holders:eziochiu

#ifndef MAME_MACHINE_LPC2132_VIC_H
#define MAME_MACHINE_LPC2132_VIC_H

#pragma once

DECLARE_DEVICE_TYPE(LPC2132_VIC, lpc2132_vic_device)

class lpc2132_vic_device : public device_t
{
public:
	// construction/destruction
	lpc2132_vic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// IRQ output callback
	auto irq_callback() { return m_irq_out.bind(); }

	// FIQ output callback
	auto fiq_callback() { return m_fiq_out.bind(); }

	// VICDefVectAddr write callback (for handshake detection)
	auto def_vect_addr_callback() { return m_def_vect_addr_cb.bind(); }

	// register address map
	void regs_map(address_map &map) ATTR_COLD;

	// external interrupt trigger interface
	void set_irq(int line, int state);

	// template method for interrupt line write
	template<unsigned IRQ>
	void irq_w(int state) { set_irq(IRQ, state); }

 protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

 private:
	// status registers
	uint32_t m_vic_irq_status;      // VICIRQStatus - IRQ status
	uint32_t m_vic_fiq_status;      // VICFIQStatus - FIQ status
	uint32_t m_vic_raw_intr;        // VICRawIntr - raw interrupt status
	uint32_t m_vic_int_select;      // VICIntSelect - interrupt select (IRQ/FIQ)
	uint32_t m_vic_int_enable;      // VICIntEnable - interrupt enable
	uint32_t m_vic_soft_int;        // VICSoftInt - software interrupt
	uint32_t m_vic_protection;      // VICProtection - protection mode

	// vector interrupt registers
	uint32_t m_vic_vect_addr[16];   // VICVectAddr0-15 - vector addresses
	uint32_t m_vic_vect_cntl[16];   // VICVectCntl0-15 - vector control

	uint32_t m_vic_vect_addr_cur;   // VICVectAddr - current vector address
	uint32_t m_vic_def_vect_addr;   // VICDefVectAddr - default vector address

	// IRQ output
	devcb_write_line m_irq_out;

	// FIQ output
	devcb_write_line m_fiq_out;

	// VICDefVectAddr write callback
	devcb_write32 m_def_vect_addr_cb;

	// internal functions
	void update_interrupt_lines();
	uint32_t read_vector_address();

	// register read functions
	uint32_t irq_status_r();
	uint32_t fiq_status_r();
	uint32_t raw_intr_r();
	uint32_t int_select_r();
	uint32_t int_enable_r();
	uint32_t soft_int_r();
	uint32_t protection_r();
	uint32_t vect_addr_r();
	uint32_t def_vect_addr_r();
	uint32_t vect_addr_slot_r(offs_t offset);
	uint32_t vect_cntl_r(offs_t offset);

	// register write functions
	void int_select_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void int_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void int_en_clear_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void soft_int_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void soft_int_clear_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void protection_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vect_addr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void def_vect_addr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vect_addr_slot_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vect_cntl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
};

#endif // MAME_MACHINE_LPC2132_VIC_H
