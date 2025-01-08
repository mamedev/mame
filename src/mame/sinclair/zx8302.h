// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair ZX8302 emulation

**********************************************************************
                            _____   _____
                 ERASE   1 |*    \_/     | 40  Vdd
               EXTINTL   2 |             | 39  DB4
                MDRDWL   3 |             | 38  DB5
                NETOUT   4 |             | 37  DB0
                BAUDX4   5 |             | 36  DB1
                 DTR1L   6 |             | 35  COMDATA
                 CTS2L   7 |             | 34  MDSELDH
                 DCSML   8 |             | 33  MDSELCKN
                  ROWL   9 |             | 32  VSYNCH
                 PCENL  10 |    ZX8302   | 31  XTAL2
                   Vdd  11 |     ULA     | 30  XTAL1
                   DB2  12 |             | 29
                  TXD1  13 |             | 28  RESETOUTL
                  TXD2  14 |             | 27  DB7
                    A5  15 |             | 26  IPL1L
                 NETIN  16 |             | 25  CLKCPU
                    A1  17 |             | 24  DB3
                    A0  18 |             | 23  DB6
                  RAW2  19 |             | 22  COMCTL
                   GND  20 |_____________| 21  RAW1

**********************************************************************/

#ifndef MAME_SINCLAIR_ZX8302_H
#define MAME_SINCLAIR_ZX8302_H

#pragma once

#include "diserial.h"


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> zx8302_device

class zx8302_device :  public device_t,
						public device_serial_interface
{
public:
	// construction/destruction
	zx8302_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_rtc_clock(int rtc_clock) { m_rtc_clock = rtc_clock; }
	void set_rtc_clock(const XTAL &rtc_clock) { set_rtc_clock(rtc_clock.value()); }
	auto out_ipl1l_callback() { return m_out_ipl1l_cb.bind(); }
	auto out_baudx4_callback() { return m_out_baudx4_cb.bind(); }
	auto out_comdata_callback() { return m_out_comdata_cb.bind(); }
	auto out_txd1_callback() { return m_out_txd1_cb.bind(); }
	auto out_txd2_callback() { return m_out_txd2_cb.bind(); }
	auto out_netout_callback() { return m_out_netout_cb.bind(); }
	auto out_mdselck_callback() { return m_out_mdselck_cb.bind(); }
	auto out_mdseld_callback() { return m_out_mdseld_cb.bind(); }
	auto out_mdrdw_callback() { return m_out_mdrdw_cb.bind(); }
	auto out_erase_callback() { return m_out_erase_cb.bind(); }
	auto out_raw1_callback() { return m_out_raw1_cb.bind(); }
	auto in_raw1_callback() { return m_in_raw1_cb.bind(); }
	auto out_raw2_callback() { return m_out_raw2_cb.bind(); }
	auto in_raw2_callback() { return m_in_raw2_cb.bind(); }

	uint8_t rtc_r(offs_t offset);
	void rtc_w(uint8_t data);
	void control_w(uint8_t data);
	uint8_t mdv_track_r();
	uint8_t status_r();
	void ipc_command_w(uint8_t data);
	void mdv_control_w(uint8_t data);
	uint8_t irq_status_r();
	void irq_acknowledge_w(uint8_t data);
	void data_w(uint8_t data);
	void vsync_w(int state);
	void comctl_w(int state);
	void comdata_w(int state);
	void extint_w(int state);

	void write_netin(int state);
	void write_dtr1(int state);
	void write_cts2(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

	TIMER_CALLBACK_MEMBER(baudx4_tick);
	TIMER_CALLBACK_MEMBER(rtc_tick);
	TIMER_CALLBACK_MEMBER(trigger_gap_int);

	inline void trigger_interrupt(uint8_t line);
	inline void transmit_ipc_data();

private:
	enum
	{
		IPC_START,
		IPC_DATA,
		IPC_STOP
	};

	enum
	{
		BAUD_19200 = 0,
		BAUD_9600,
		BAUD_4800,
		BAUD_2400,
		BAUD_1200,
		BAUD_600,
		BAUD_300,
		BAUD_75,
		BAUD_MASK = 0x07
	};

	enum
	{
		MODE_SER1               = 0x00,
		MODE_SER2               = 0x08,
		MODE_MDV                = 0x10,
		MODE_NET                = 0x18,
		MODE_MASK               = 0x18
	};

	enum
	{
		INT_GAP                 = 0x01,
		INT_INTERFACE           = 0x02,
		INT_TRANSMIT            = 0x04,
		INT_FRAME               = 0x08,
		INT_EXTERNAL            = 0x10
	};

	enum
	{
		STATUS_NETWORK_PORT     = 0x01,
		STATUS_TX_BUFFER_FULL   = 0x02,
		STATUS_RX_BUFFER_FULL   = 0x04,
		STATUS_MICRODRIVE_GAP   = 0x08
	};

	int m_rtc_clock;              // the RTC clock (pin 30) of the chip

	// serial
	devcb_write_line    m_out_ipl1l_cb;
	devcb_write_line    m_out_baudx4_cb;
	devcb_write_line    m_out_comdata_cb;
	devcb_write_line    m_out_txd1_cb;
	devcb_write_line    m_out_txd2_cb;
	devcb_write_line    m_out_netout_cb;

	// microdrive
	devcb_write_line    m_out_mdselck_cb;
	devcb_write_line    m_out_mdseld_cb;
	devcb_write_line    m_out_mdrdw_cb;
	devcb_write_line    m_out_erase_cb;
	devcb_write_line    m_out_raw1_cb;
	devcb_read_line     m_in_raw1_cb;
	devcb_write_line    m_out_raw2_cb;
	devcb_read_line     m_in_raw2_cb;

	int m_rs232_rx;
	int m_dtr1;
	int m_cts2;

	// registers
	uint8_t m_idr;                    // IPC data register
	uint8_t m_tcr;                    // transfer control register
	uint8_t m_tdr;                    // transfer data register
	uint8_t m_irq;                    // interrupt register
	uint32_t m_ctr;                   // counter register
	uint8_t m_status;                 // status register

	// IPC communication state
	int m_comdata_from_ipc;         // pending data from IPC->68000
	int m_comdata_to_cpu;           // communication data IPC->68000
	int m_comdata_to_ipc;           // communication data 68000->IPC
	int m_comctl;                   // communication control
	int m_ipc_state;                // communication state
	int m_ipc_busy;                 // IPC busy
	int m_baudx4;                   // IPC baud x4

	// microdrive state
	uint8_t m_mdv_data[2];            // track data register
	int m_track;                    // current track

	// timers
	emu_timer *m_baudx4_timer;      // baud x4 timer
	emu_timer *m_rtc_timer;         // real time clock timer
	emu_timer *m_gap_timer;         // microdrive gap timer
};


// device type definition
DECLARE_DEVICE_TYPE(ZX8302, zx8302_device)



#endif // MAME_SINCLAIR_ZX8302_H
