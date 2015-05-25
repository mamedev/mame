// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    MOS Technology 6551 Asynchronous Communication Interface Adapter

**********************************************************************
                            _____   _____
                   GND   1 |*    \_/     | 28  R/_W
                   CS0   2 |             | 27  phi2
                  _CS1   3 |             | 26  _IRQ
                  _RES   4 |             | 25  DB7
                   RxC   5 |             | 24  DB6
                 XTAL1   6 |             | 23  DB5
                 XTAL2   7 |   MOS6551   | 22  DB4
                  _RTS   8 |             | 21  DB3
                  _CTS   9 |             | 20  DB2
                   TxD  10 |             | 19  DB1
                  _DTR  11 |             | 18  DB0
                   RxD  12 |             | 17  _DBR
                   RS0  13 |             | 16  _DCD
                   RS1  14 |_____________| 15  Vcc

**********************************************************************/

#pragma once

#ifndef __MOS6551__
#define __MOS6551__

#include "emu.h"
#include "machine/clock.h"

#define MCFG_MOS6551_XTAL(_xtal) \
	mos6551_device::set_xtal(*device, _xtal);

#define MCFG_MOS6551_IRQ_HANDLER(_devcb) \
	devcb = &mos6551_device::set_irq_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6551_TXD_HANDLER(_devcb) \
	devcb = &mos6551_device::set_txd_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6551_RXC_HANDLER(_devcb) \
	devcb = &mos6551_device::set_rxc_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6551_RTS_HANDLER(_devcb) \
	devcb = &mos6551_device::set_rts_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6551_DTR_HANDLER(_devcb) \
	devcb = &mos6551_device::set_dtr_handler(*device, DEVCB_##_devcb);

class mos6551_device : public device_t
{
public:
	mos6551_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_xtal(device_t &device, UINT32 xtal) { downcast<mos6551_device &>(device).set_xtal(xtal); }
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<mos6551_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_txd_handler(device_t &device, _Object object) { return downcast<mos6551_device &>(device).m_txd_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_rxc_handler(device_t &device, _Object object) { return downcast<mos6551_device &>(device).m_rxc_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_rts_handler(device_t &device, _Object object) { return downcast<mos6551_device &>(device).m_rts_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_dtr_handler(device_t &device, _Object object) { return downcast<mos6551_device &>(device).m_dtr_handler.set_callback(object); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_WRITE_LINE_MEMBER(write_xtal1); // txc
	DECLARE_WRITE_LINE_MEMBER(write_rxd);
	DECLARE_WRITE_LINE_MEMBER(write_rxc);
	DECLARE_WRITE_LINE_MEMBER(write_cts);
	DECLARE_WRITE_LINE_MEMBER(write_dsr);
	DECLARE_WRITE_LINE_MEMBER(write_dcd);

	DECLARE_WRITE_LINE_MEMBER(internal_clock);

	void set_xtal(UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	enum
	{
		SR_PARITY_ERROR = 0x01,
		SR_FRAMING_ERROR = 0x02,
		SR_OVERRUN = 0x04,
		SR_RDRF = 0x08,
		SR_TDRE = 0x10,
		SR_DCD = 0x20,
		SR_DSR = 0x40,
		SR_IRQ = 0x80
	};

	enum
	{
		PARITY_NONE = 0,
		PARITY_ODD = 1,
		PARITY_EVEN = 3,
		PARITY_MARK = 5,
		PARITY_SPACE = 7
	};

	enum
	{
		IRQ_DCD = 1,
		IRQ_DSR = 2,
		IRQ_RDRF = 4,
		IRQ_TDRE = 8,
		IRQ_CTS = 16
	};

	enum
	{
		STATE_START,
		STATE_DATA,
		STATE_STOP
	};

	enum
	{
		OUTPUT_TXD,
		OUTPUT_MARK,
		OUTPUT_BREAK
	};

	void output_irq(int irq);
	void output_txd(int txd);
	void output_rxc(int rxc);
	void output_rts(int rts);
	void output_dtr(int dtr);

	void update_irq();
	void update_divider();

	UINT8 read_rdr();
	UINT8 read_status();
	UINT8 read_command();
	UINT8 read_control();

	void write_tdr(UINT8 data);
	void write_reset(UINT8 data);
	void write_command(UINT8 data);
	void write_control(UINT8 data);

	int stoplength();

	DECLARE_WRITE_LINE_MEMBER(receiver_clock);
	DECLARE_WRITE_LINE_MEMBER(transmitter_clock);

	static const int internal_divider[16];
	static const int transmitter_controls[4][3];

	required_device<clock_device> m_internal_clock;
	devcb_write_line m_irq_handler;
	devcb_write_line m_txd_handler;
	devcb_write_line m_rxc_handler;
	devcb_write_line m_rts_handler;
	devcb_write_line m_dtr_handler;

	UINT8 m_control;
	UINT8 m_command;
	UINT8 m_status;
	UINT8 m_tdr;
	UINT8 m_rdr;

	UINT8 m_irq_state;

	int m_irq;
	int m_txd;
	int m_rxc;
	int m_rts;
	int m_dtr;

	UINT32 m_xtal;
	int m_divide;
	int m_cts;
	int m_dsr;
	int m_dcd;
	int m_rxd;

	int m_wordlength;
	int m_extrastop;
	int m_brk;
	int m_echo_mode;
	int m_parity;

	int m_rx_state;
	int m_rx_clock;
	int m_rx_bits;
	int m_rx_shift;
	int m_rx_parity;
	int m_rx_counter;
	int m_rx_irq_enable;
	int m_rx_internal_clock;

	int m_tx_state;
	int m_tx_output;
	int m_tx_clock;
	int m_tx_bits;
	int m_tx_shift;
	int m_tx_parity;
	int m_tx_counter;
	int m_tx_enable;
	int m_tx_irq_enable;
	int m_tx_internal_clock;
};

extern const device_type MOS6551;

#endif
