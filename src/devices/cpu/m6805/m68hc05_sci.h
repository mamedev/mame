// license:BSD-3-Clause
// copyright-holders:Adam Di Carlo
/***************************************************************************

    Motorola M68HC05 serial communications interface (SCI)

    The asynchronous SCI of the MC68HC05C4/C8 and relatives: BAUD, SCCR1,
    SCCR2, SCSR and SCDR at $0D-$11 of the host MCU.  The rate is derived
    from the MCU's internal processor clock (E, half the oscillator
    frequency), which is this device's clock.

***************************************************************************/

#ifndef MAME_CPU_M6805_M68HC05_SCI_H
#define MAME_CPU_M6805_M68HC05_SCI_H

#pragma once

#include "diserial.h"


class m68hc05_sci_device : public device_t, public device_serial_interface
{
public:
	// implemented bits, for the host's debugger state
	static constexpr u8 BAUD_MASK = 0x37;
	static constexpr u8 SCCR1_MASK = 0xd8;
	static constexpr u8 SCSR_MASK = 0xfe;

	m68hc05_sci_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// TxD output pin (TDO) and the interrupt request towards the host MCU (level); the
	// RxD input pin (RDI) is rx_w from device_serial_interface
	auto tdo_cb() { return m_tdo_cb.bind(); }
	auto irq_cb() { return m_irq_cb.bind(); }

	void map(address_map &map) ATTR_COLD;

	// register access for the host's debugger state (no transmitter or receiver activity)
	u8 baud() const { return m_baud; }
	u8 sccr1() const { return m_sccr1; }
	u8 sccr2() const { return m_sccr2; }
	u8 scsr() const { return m_scsr; }
	u8 scdr() const { return m_rdr; }
	void set_baud(u8 data);
	void set_sccr1(u8 data) { m_sccr1 = data & SCCR1_MASK; }
	void set_sccr2(u8 data);
	void set_scsr(u8 data);
	void set_scdr(u8 data) { m_rdr = data; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	virtual void rcv_complete() override;
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	enum : u8
	{
		SCCR1_M     = 0x10,
		SCCR1_WAKE  = 0x08,

		SCCR2_TIE   = 0x80,
		SCCR2_TCIE  = 0x40,
		SCCR2_RIE   = 0x20,
		SCCR2_ILIE  = 0x10,
		SCCR2_TE    = 0x08,
		SCCR2_RE    = 0x04,
		SCCR2_RWU   = 0x02,
		SCCR2_SBK   = 0x01,

		SCSR_TDRE   = 0x80,
		SCSR_TC     = 0x40,
		SCSR_RDRF   = 0x20,
		SCSR_IDLE   = 0x10,
		SCSR_OR     = 0x08,
		SCSR_NF     = 0x04,
		SCSR_FE     = 0x02,

		// cleared by an SCSR read followed by an SCDR read
		SCSR_RX_FLAGS = SCSR_RDRF | SCSR_IDLE | SCSR_OR | SCSR_NF | SCSR_FE
	};

	// what the line timer is timing
	enum : int
	{
		LINE_PREAMBLE,
		LINE_STOP
	};

	u8 baud_r();
	void baud_w(u8 data);
	u8 sccr1_r();
	void sccr1_w(u8 data);
	u8 sccr2_r();
	void sccr2_w(u8 data);
	u8 scsr_r();
	u8 scdr_r();
	void scdr_w(u8 data);

	TIMER_CALLBACK_MEMBER(line_done);

	bool te() const { return m_sccr2 & SCCR2_TE; }
	bool re() const { return m_sccr2 & SCCR2_RE; }
	bool tx_busy() const { return !is_transmit_register_empty() || m_preamble_active; }

	void set_flags(u8 mask);
	void clear_flags(u8 mask);
	void update_irq();
	void update_rate();
	void next_frame();
	void start_frame();
	void start_preamble();

	devcb_write_line    m_tdo_cb;
	devcb_write_line    m_irq_cb;
	emu_timer           *m_line_timer;

	u8                  m_baud;
	u8                  m_sccr1;
	u8                  m_sccr2;
	u8                  m_scsr, m_scsr_seen;
	u8                  m_rdr;              // receive data register, read through SCDR
	u8                  m_tdr;              // transmit data register, written through SCDR
	bool                m_tdr_full;         // TDR holds a byte not yet moved to the shifter
	bool                m_preamble_active;  // idle frame in progress
	bool                m_preamble_pending; // TE rose while a frame was in the shifter
	bool                m_stop_active;      // stop bit of the last frame in progress
	bool                m_irq_state;
	u32                 m_divisor;          // E clocks per bit time
};

DECLARE_DEVICE_TYPE(M68HC05_SCI, m68hc05_sci_device)

#endif // MAME_CPU_M6805_M68HC05_SCI_H
