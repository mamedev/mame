// license:BSD-3-Clause
// copyright-holders:smf
/*********************************************************************

    6850acia.h

    6850 ACIA code

*********************************************************************/

#ifndef MAME_MACHINE_6850ACIA_H
#define MAME_MACHINE_6850ACIA_H

#pragma once


class acia6850_device :  public device_t
{
public:
	// construction/destruction
	acia6850_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto txd_handler() { return m_txd_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }
	auto irq_handler() { return m_irq_handler.bind(); }

	void control_w(uint8_t data);
	uint8_t status_r();
	void data_w(uint8_t data);
	uint8_t data_r();
	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

	void write_cts(int state);
	void write_dcd(int state);
	void write_rxd(int state);
	void write_rxc(int state);
	void write_txc(int state);

protected:
	acia6850_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void update_irq();
	int calculate_txirq();
	int calculate_rxirq();

private:
	void output_txd(int txd);
	void output_rts(int txd);
	void output_irq(int irq);

	TIMER_CALLBACK_MEMBER(delayed_output_irq);

	enum
	{
		SR_RDRF = 0x01,
		SR_TDRE = 0x02,
		SR_DCD = 0x04,
		SR_CTS = 0x08,
		SR_FE = 0x10,
		SR_OVRN = 0x20,
		SR_PE = 0x40,
		SR_IRQ = 0x80
	};

	enum serial_state
	{
		STATE_START,
		STATE_DATA,
		STATE_STOP
	};

	enum parity_type
	{
		PARITY_NONE,
		PARITY_ODD,
		PARITY_EVEN
	};

	enum dcd_irq_state
	{
		DCD_IRQ_NONE = 0,
		DCD_IRQ_READ_DATA,
		DCD_IRQ_READ_STATUS
	};

	devcb_write_line m_txd_handler;
	devcb_write_line m_rts_handler;
	devcb_write_line m_irq_handler;

	uint8_t m_status;
	uint8_t m_tdr;
	uint8_t m_rdr;

	bool m_first_master_reset;
	int m_dcd_irq_pending;
	bool m_overrun_pending;

	int m_divide;
	int m_bits;
	int m_stopbits;
	int m_parity;
	int m_brk;

	int m_rts;
	int m_dcd;
	int m_irq;

	int m_txc;
	int m_txd;
	int m_tx_state;
	int m_tx_bits;
	int m_tx_shift;
	int m_tx_parity;
	int m_tx_counter;
	int m_tx_irq_enable;

	int m_rxc;
	int m_rxd;
	int m_rx_state;
	int m_rx_bits;
	int m_rx_shift;
	int m_rx_parity;
	int m_rx_counter;
	int m_rx_irq_enable;

	static const int counter_divide_select[4];
	static const int word_select[8][3];
	static const int transmitter_control[4][3];
};

// device type definition
DECLARE_DEVICE_TYPE(ACIA6850, acia6850_device)

#endif // MAME_MACHINE_6850ACIA_H
