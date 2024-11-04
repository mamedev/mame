// license:BSD-3-Clause
// copyright-holders:David Haywood, Joakim Larsson Edstrom
#ifndef MAME_MACHINE_68340TMU_H
#define MAME_MACHINE_68340TMU_H

#pragma once

class m68340_cpu_device;

class mc68340_timer_module_device : public device_t
{
	friend class m68340_cpu_device;

public:
	mc68340_timer_module_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint16_t read(offs_t offset, uint16_t mem_mask = ~0);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tin_w(int state);
	void tgate_w(int state);

	uint8_t irq_level() const { return (m_sr & REG_SR_IRQ) ? (m_ir & REG_IR_INTLEV) >> 8 : 0; }
	uint8_t irq_vector() const { return m_ir & REG_IR_INTVEC; }
	uint8_t arbitrate(uint8_t level) const { return (irq_level() == level) ? (m_mcr & REG_MCR_ARBLV) : 0; }

	void module_reset();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	m68340_cpu_device *m_cpu;

	uint16_t m_mcr;
	uint16_t m_ir;
	uint16_t m_cr;
	uint16_t m_sr;
	uint16_t m_cntr;
	uint16_t m_cntr_reg;
	uint16_t m_prel1;
	uint16_t m_prel2;
	uint16_t m_com;
	uint16_t m_timer_counter;
	uint32_t m_tin;
	uint32_t m_tgate;
	emu_timer *m_timer;

	devcb_write_line    m_tout_out_cb;
	devcb_write_line    m_tgate_in_cb;
	void do_timer_irq();
	void do_timer_tick();
	void tout_set();
	void tout_clear();

	TIMER_CALLBACK_MEMBER(timer_callback);

	enum {
			REG_MCR   = 0x00,
			REG_IR    = 0x04,
			REG_CR    = 0x06,
			REG_SR    = 0x08,
			REG_CNTR  = 0x0a,
			REG_PREL1 = 0x0c,
			REG_PREL2 = 0x0e,
			REG_COM   = 0x10,
	};

	enum {
			REG_MCR_STP   = 0x8000,
			REG_MCR_FRZ1  = 0x4000,
			REG_MCR_FRZ2  = 0x2000,
			REG_MCR_SUPV  = 0x0080,
			REG_MCR_ARBLV = 0x000f,
	};

	enum {
			REG_IR_INTLEV = 0x0700,
			REG_IR_INTVEC = 0x00ff,
	};

	enum {
			REG_CR_SWR    = 0x8000,
			REG_CR_INTMSK = 0x7000,
			REG_CR_IE2    = 0x4000,
			REG_CR_IE1    = 0x2000,
			REG_CR_IE0    = 0x1000,
			REG_CR_TGE    = 0x0800,
			REG_CR_PCLK   = 0x0400,
			REG_CR_CPE    = 0x0200,
			REG_CR_CLK    = 0x0100,
			REG_CR_POT_MASK = 0x00e0,
			REG_CR_MODE_MASK   = 0x001c, // Mode mask
			REG_CR_MODE_ICOC   = 0x0000, // Input Capture Output Compare
			REG_CR_MODE_SQWG   = 0x0004, // Square Wave Generator
			REG_CR_MODE_VDCSW  = 0x0008, // Variable Duty Cycle Square Wave generator
			REG_CR_MODE_VWSSPG = 0x000c, // Variable Width Single Shot Pulse Generator
			REG_CR_MODE_PWM    = 0x0010, // Pulse Width Measurement
			REG_CR_MODE_PM     = 0x0014, // Period Measurement
			REG_CR_MODE_EC     = 0x0018, // Event Count
			REG_CR_MODE_TB     = 0x001c, // Timer Bypass
			REG_CR_OC_MASK = 0x0003,
			REG_CR_OC_DISABLED = 0x0000,
			REG_CR_OC_TOGGLE   = 0x0001,
			REG_CR_OC_ZERO     = 0x0002,
			REG_CR_OC_ONE      = 0x0003,
	};

	enum {
			REG_SR_IRQ      = 0x8000,
			REG_SR_TO       = 0x4000,
			REG_SR_TG       = 0x2000,
			REG_SR_TC       = 0x1000,
			REG_SR_TGL      = 0x0800,
			REG_SR_ON       = 0x0400,
			REG_SR_OUT      = 0x0200,
			REG_SR_COM      = 0x0100,
			REG_SR_PSC_OUT  = 0x00ff,
	};

};

DECLARE_DEVICE_TYPE(MC68340_TIMER_MODULE, mc68340_timer_module_device)

#endif // MAME_MACHINE_68340TMU_H
