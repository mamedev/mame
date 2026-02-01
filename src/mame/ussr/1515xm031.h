// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    1515XM1-031 Gate Array emulation

**********************************************************************/

#ifndef MAME_USSR_1515XM031_H
#define MAME_USSR_1515XM031_H

#pragma once

#include "machine/keyboard.h"
#include "machine/pdp11.h"
#include "machine/z80daisy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> k1515xm031_device

class k1515xm031_pri_device : public device_t,
	public device_z80daisy_interface,
	protected device_matrix_keyboard_interface<12U>
{
public:
	// construction/destruction
	k1515xm031_pri_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto virq_wr_callback() { return m_write_virq.bind(); }

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;

private:
	static constexpr uint16_t KBDCSR_RD = CSR_DONE | CSR_IE;
	static constexpr uint16_t KBDCSR_WR = CSR_IE;

	devcb_write_line m_write_virq;
	line_state m_rxrdy;

	uint16_t m_csr;
	uint16_t m_buf;
};


class k1515xm031_sub_device : public device_t,
	public device_z80daisy_interface
{
public:
	// construction/destruction
	k1515xm031_sub_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto virq_wr_callback() { return m_write_virq.bind(); }

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

private:
	static constexpr uint16_t TMRCSR_MODE    = 0001;
	static constexpr uint16_t TMRCSR_FREQ    = 0002;
	static constexpr uint16_t TMRCSR_OVF     = 0010;
	static constexpr uint16_t TMRCSR_EIE     = 0020;
	static constexpr uint16_t TMRCSR_EVNT    = 0040;
	static constexpr uint16_t TMRCSR_RD      = 0377;
	static constexpr uint16_t TMRCSR_WR      = (CSR_IE | TMRCSR_EIE | TMRCSR_FREQ | TMRCSR_MODE);

	static constexpr uint16_t TMRBUF_RDWR    = 07777;

	devcb_write_line m_write_virq;

	line_state m_rxrdy;
	uint16_t m_csr;
	uint16_t m_buf;
	uint16_t m_value;
	uint16_t m_counter;

	emu_timer *m_timer;
	TIMER_CALLBACK_MEMBER(timer_tick);
};


// device type definition
DECLARE_DEVICE_TYPE(K1515XM031_PRI, k1515xm031_pri_device)
DECLARE_DEVICE_TYPE(K1515XM031_SUB, k1515xm031_sub_device)

#endif // MAME_USSR_1515XM031_H
