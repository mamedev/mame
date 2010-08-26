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

#define ACIA6850_STATUS_RDRF	0x01
#define ACIA6850_STATUS_TDRE	0x02
#define ACIA6850_STATUS_DCD		0x04
#define ACIA6850_STATUS_CTS		0x08
#define ACIA6850_STATUS_FE		0x10
#define ACIA6850_STATUS_OVRN	0x20
#define ACIA6850_STATUS_PE		0x40
#define ACIA6850_STATUS_IRQ		0x80



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_ACIA6850_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ACIA6850, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define ACIA6850_INTERFACE(_name) \
	const acia6850_interface(_name) =


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> acia6850_interface

struct acia6850_interface
{
	int	m_tx_clock;
	int	m_rx_clock;

	devcb_read_line		m_in_rx_func;
	devcb_write_line	m_out_tx_func;

	devcb_read_line		m_in_cts_func;
	devcb_write_line	m_out_rts_func;
	devcb_read_line		m_in_dcd_func;

	devcb_write_line	m_out_irq_func;
};



// ======================> acia6850_device_config

class acia6850_device_config : public device_config,
                               public acia6850_interface
{
    friend class acia6850_device;

    // construction/destruction
    acia6850_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};



// ======================> acia6850_device

class acia6850_device :  public device_t
{
    friend class acia6850_device_config;

    // construction/destruction
    acia6850_device(running_machine &_machine, const acia6850_device_config &_config);

public:

	void acia6850_tx_clock_in();
	void acia6850_rx_clock_in();

	void acia6850_set_rx_clock(int clock);
	void acia6850_set_tx_clock(int clock);

	void acia6850_ctrl_w(UINT32 offset, UINT8 data);
	UINT8 acia6850_stat_r(UINT32 offset);
	void acia6850_data_w(UINT32 offset, UINT8 data);
	UINT8 acia6850_data_r(UINT32 offset);

	void tx_clock_in();
	void rx_clock_in();

	void set_rx_clock(int clock) { m_rx_clock = clock; }
	void set_tx_clock(int clock) { m_tx_clock = clock; }

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

	static TIMER_CALLBACK( transmit_event_callback );
	static TIMER_CALLBACK( receive_event_callback );

private:

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

	devcb_resolved_read_line	m_in_rx_func;
	devcb_resolved_write_line	m_out_tx_func;
	devcb_resolved_read_line	m_in_cts_func;
	devcb_resolved_write_line	m_out_rts_func;
	devcb_resolved_read_line	m_in_dcd_func;
	devcb_resolved_write_line	m_out_irq_func;

	UINT8		m_ctrl;
	UINT8		m_status;

	UINT8		m_tdr;
	UINT8		m_rdr;
	UINT8		m_rx_shift;
	UINT8		m_tx_shift;

	UINT8		m_rx_counter;
	UINT8		m_tx_counter;

	int			m_rx_clock;
	int			m_tx_clock;

	int			m_divide;

	/* Counters */
	int			m_tx_bits;
	int			m_rx_bits;
	int			m_tx_parity;
	int			m_rx_parity;

	/* TX/RX state */
	int			m_bits;
	parity_type	m_parity;
	int			m_stopbits;
	int			m_tx_int;

	/* Signals */
	int			m_overrun;
	int			m_reset;
	int			m_rts;
	int			m_brk;
	int			m_first_reset;
	int			m_status_read;
	serial_state m_rx_state;
	serial_state m_tx_state;
	int			m_irq;

	emu_timer	*m_rx_timer;
	emu_timer	*m_tx_timer;

    const acia6850_device_config &m_config;

	static const int ACIA6850_DIVIDE[3];
	static const int ACIA6850_WORD[8][3];
};


// device type definition
extern const device_type ACIA6850;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

void acia6850_tx_clock_in(running_device *device) ATTR_NONNULL(1);
void acia6850_rx_clock_in(running_device *device) ATTR_NONNULL(1);

void acia6850_set_rx_clock(running_device *device, int clock) ATTR_NONNULL(1);
void acia6850_set_tx_clock(running_device *device, int clock) ATTR_NONNULL(1);

WRITE8_DEVICE_HANDLER( acia6850_ctrl_w );
READ8_DEVICE_HANDLER( acia6850_stat_r );
WRITE8_DEVICE_HANDLER( acia6850_data_w );
READ8_DEVICE_HANDLER( acia6850_data_r );

#endif /* __ACIA6850_H__ */
