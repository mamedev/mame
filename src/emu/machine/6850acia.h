/*********************************************************************

    6850acia.h

    6850 ACIA code

*********************************************************************/

#pragma once

#ifndef __ACIA6850_H__
#define __ACIA6850_H__

#include "emu.h"



/***************************************************************************
    EXTERNAL MACROS
***************************************************************************/

#define ACIA6850_STATUS_RDRF    0x01
#define ACIA6850_STATUS_TDRE    0x02
#define ACIA6850_STATUS_DCD     0x04
#define ACIA6850_STATUS_CTS     0x08
#define ACIA6850_STATUS_FE      0x10
#define ACIA6850_STATUS_OVRN    0x20
#define ACIA6850_STATUS_PE      0x40
#define ACIA6850_STATUS_IRQ     0x80



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_ACIA6850_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, ACIA6850, 0) \
	acia6850_device::static_set_interface(*device, _interface);

#define ACIA6850_INTERFACE(_name) \
	const acia6850_interface(_name) =


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> acia6850_interface

struct acia6850_interface
{
	int m_tx_clock;
	int m_rx_clock;

	devcb_read_line     m_in_rx_cb;
	devcb_write_line    m_out_tx_cb;

	devcb_read_line     m_in_cts_cb;
	devcb_write_line    m_out_rts_cb;
	devcb_read_line     m_in_dcd_cb;

	devcb_write_line    m_out_irq_cb;
};



// ======================> acia6850_device

class acia6850_device :  public device_t,
							public acia6850_interface
{
public:
	// construction/destruction
	acia6850_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_interface(device_t &device, const acia6850_interface &interface);

	DECLARE_WRITE8_MEMBER( control_write );
	DECLARE_READ8_MEMBER( status_read );
	DECLARE_WRITE8_MEMBER( data_write );
	DECLARE_READ8_MEMBER( data_read );

	void tx_clock_in();
	void rx_clock_in();

	void set_rx_clock(int clock);
	void set_tx_clock(int clock);

	void receive_data(UINT8 data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	enum
	{
		TIMER_ID_TRANSMIT,
		TIMER_ID_RECEIVE
	};

	void check_interrupts();

	void tx_tick();
	void transmit_event();

	void rx_tick();
	void receive_event();

	enum serial_state
	{
		START,
		DATA,
		PARITY,
		STOP,
		STOP2,
	};

	enum parity_type
	{
		NONE,
		ODD,
		EVEN
	};

	devcb_resolved_read_line    m_in_rx_func;
	devcb_resolved_write_line   m_out_tx_func;
	devcb_resolved_read_line    m_in_cts_func;
	devcb_resolved_write_line   m_out_rts_func;
	devcb_resolved_read_line    m_in_dcd_func;
	devcb_resolved_write_line   m_out_irq_func;

	UINT8       m_ctrl;
	UINT8       m_status;

	UINT8       m_tdr;
	UINT8       m_rdr;
	UINT8       m_rx_shift;
	UINT8       m_tx_shift;

	UINT8       m_rx_counter;
	UINT8       m_tx_counter;

	int         m_divide;

	// Counters
	int         m_tx_bits;
	int         m_rx_bits;
	int         m_tx_parity;
	int         m_rx_parity;

	// TX/RX state
	int         m_bits;
	parity_type m_parity;
	int         m_stopbits;
	int         m_tx_int;

	// Signals
	int         m_overrun;
	int         m_reset;
	int         m_rts;
	int         m_brk;
	int         m_first_reset;
	int         m_status_read;
	serial_state m_rx_state;
	serial_state m_tx_state;
	int         m_irq;

	emu_timer   *m_rx_timer;
	emu_timer   *m_tx_timer;

	static const int ACIA6850_DIVIDE[3];
	static const int ACIA6850_WORD[8][3];
};


// device type definition
extern const device_type ACIA6850;



#endif /* __ACIA6850_H__ */
