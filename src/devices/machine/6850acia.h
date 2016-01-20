// license:BSD-3-Clause
// copyright-holders:smf
/*********************************************************************

    6850acia.h

    6850 ACIA code

*********************************************************************/

#pragma once

#ifndef __ACIA6850_H__
#define __ACIA6850_H__

#include "emu.h"

#define MCFG_ACIA6850_TXD_HANDLER(_devcb) \
	devcb = &acia6850_device::set_txd_handler(*device, DEVCB_##_devcb);

#define MCFG_ACIA6850_RTS_HANDLER(_devcb) \
	devcb = &acia6850_device::set_rts_handler(*device, DEVCB_##_devcb);

#define MCFG_ACIA6850_IRQ_HANDLER(_devcb) \
	devcb = &acia6850_device::set_irq_handler(*device, DEVCB_##_devcb);

class acia6850_device :  public device_t
{
public:
	// construction/destruction
	acia6850_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_txd_handler(device_t &device, _Object object) { return downcast<acia6850_device &>(device).m_txd_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_rts_handler(device_t &device, _Object object) { return downcast<acia6850_device &>(device).m_rts_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<acia6850_device &>(device).m_irq_handler.set_callback(object); }

	DECLARE_WRITE8_MEMBER( control_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( data_r );

	DECLARE_WRITE_LINE_MEMBER( write_cts );
	DECLARE_WRITE_LINE_MEMBER( write_dcd );
	DECLARE_WRITE_LINE_MEMBER( write_rxd );
	DECLARE_WRITE_LINE_MEMBER( write_rxc );
	DECLARE_WRITE_LINE_MEMBER( write_txc );

protected:
	acia6850_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;

	virtual void update_irq();
	int calculate_txirq();
	int calculate_rxirq();

private:
	void output_txd(int txd);
	void output_rts(int txd);
	void output_irq(int irq);

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

	UINT8 m_status;
	UINT8 m_tdr;
	UINT8 m_rdr;

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
extern const device_type ACIA6850;

#endif /* __ACIA6850_H__ */
