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

#pragma once

#ifndef __ZX8302__
#define __ZX8302__

#include "emu.h"


///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_ZX8302_RTC_CLOCK(_clk) \
	zx8302_device::set_rtc_clock(*device, _clk);

#define MCFG_ZX8302_OUT_IPL1L_CB(_devcb) \
	devcb = &zx8302_device::set_out_ipl1l_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_BAUDX4_CB(_devcb) \
	devcb = &zx8302_device::set_out_baudx4_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_COMDATA_CB(_devcb) \
	devcb = &zx8302_device::set_out_comdata_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_TXD1_CB(_devcb) \
	devcb = &zx8302_device::set_out_txd1_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_TXD2_CB(_devcb) \
	devcb = &zx8302_device::set_out_txd2_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_NETOUT_CB(_devcb) \
	devcb = &zx8302_device::set_out_netout_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_MDSELCK_CB(_devcb) \
	devcb = &zx8302_device::set_out_mdselck_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_MDSELD_CB(_devcb) \
	devcb = &zx8302_device::set_out_mdseld_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_MDRDW_CB(_devcb) \
	devcb = &zx8302_device::set_out_mdrdw_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_ERASE_CB(_devcb) \
	devcb = &zx8302_device::set_out_erase_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_RAW1_CB(_devcb) \
	devcb = &zx8302_device::set_out_raw1_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_IN_RAW1_CB(_devcb) \
	devcb = &zx8302_device::set_in_raw1_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_OUT_RAW2_CB(_devcb) \
	devcb = &zx8302_device::set_out_raw2_callback(*device, DEVCB_##_devcb);

#define MCFG_ZX8302_IN_RAW2_CB(_devcb) \
	devcb = &zx8302_device::set_in_raw2_callback(*device, DEVCB_##_devcb);


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> zx8302_device

class zx8302_device :  public device_t,
						public device_serial_interface
{
public:
	// construction/destruction
	zx8302_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void set_rtc_clock(device_t &device, int rtc_clock) { downcast<zx8302_device &>(device).m_rtc_clock = rtc_clock; }
	template<class _Object> static devcb_base &set_out_ipl1l_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_ipl1l_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_baudx4_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_baudx4_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_comdata_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_comdata_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txd1_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_txd1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txd2_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_txd2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_netout_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_netout_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_mdselck_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_mdselck_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_mdseld_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_mdseld_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_mdrdw_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_mdrdw_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_erase_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_erase_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_raw1_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_raw1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_raw1_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_in_raw1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_raw2_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_out_raw2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_raw2_callback(device_t &device, _Object object) { return downcast<zx8302_device &>(device).m_in_raw2_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( rtc_r );
	DECLARE_WRITE8_MEMBER( rtc_w );
	DECLARE_WRITE8_MEMBER( control_w );
	DECLARE_READ8_MEMBER( mdv_track_r );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( ipc_command_w );
	DECLARE_WRITE8_MEMBER( mdv_control_w );
	DECLARE_READ8_MEMBER( irq_status_r );
	DECLARE_WRITE8_MEMBER( irq_acknowledge_w );
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_WRITE_LINE_MEMBER( vsync_w );
	DECLARE_WRITE_LINE_MEMBER( comctl_w );
	DECLARE_WRITE_LINE_MEMBER( comdata_w );
	DECLARE_WRITE_LINE_MEMBER( extint_w );

	DECLARE_WRITE_LINE_MEMBER( write_netin );
	DECLARE_WRITE_LINE_MEMBER( write_dtr1 );
	DECLARE_WRITE_LINE_MEMBER( write_cts2 );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

	inline void trigger_interrupt(UINT8 line);
	inline void transmit_ipc_data();

private:
	enum
	{
		TIMER_BAUDX4 = 0,
		TIMER_RTC,
		TIMER_GAP
	};

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
	UINT8 m_idr;                    // IPC data register
	UINT8 m_tcr;                    // transfer control register
	UINT8 m_tdr;                    // transfer data register
	UINT8 m_irq;                    // interrupt register
	UINT32 m_ctr;                   // counter register
	UINT8 m_status;                 // status register

	// IPC communication state
	int m_comdata_from_ipc;         // pending data from IPC->68000
	int m_comdata_to_cpu;           // communication data IPC->68000
	int m_comdata_to_ipc;           // communication data 68000->IPC
	int m_comctl;                   // communication control
	int m_ipc_state;                // communication state
	int m_ipc_busy;                 // IPC busy
	int m_baudx4;                   // IPC baud x4

	// microdrive state
	UINT8 m_mdv_data[2];            // track data register
	int m_track;                    // current track

	// timers
	emu_timer *m_baudx4_timer;      // baud x4 timer
	emu_timer *m_rtc_timer;         // real time clock timer
	emu_timer *m_gap_timer;         // microdrive gap timer
};


// device type definition
extern const device_type ZX8302;



#endif
