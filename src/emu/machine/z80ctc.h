/***************************************************************************

    Z80 CTC (Z8430) implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
                            _____   _____
                    D4   1 |*    \_/     | 28  D3
                    D5   2 |             | 27  D2
                    D6   3 |             | 26  D1
                    D7   4 |             | 25  D0
                   GND   5 |             | 24  +5V
                   _RD   6 |             | 23  CLK/TRG0
                ZC/TOO   7 |   Z80-CTC   | 22  CLK/TRG1
                ZC/TO1   8 |             | 21  CLK/TRG2
                ZC/TO2   9 |             | 20  CLK/TRG3
                 _IORQ  10 |             | 19  CS1
                   IEO  11 |             | 18  CS0
                  _INT  12 |             | 17  _RESET
                   IEI  13 |             | 16  _CE
                   _M1  14 |_____________| 15  CLK

***************************************************************************/

#ifndef __Z80CTC_H__
#define __Z80CTC_H__

#include "cpu/z80/z80daisy.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int NOTIMER_0 = (1<<0);
const int NOTIMER_1 = (1<<1);
const int NOTIMER_2 = (1<<2);
const int NOTIMER_3 = (1<<3);



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define Z80CTC_INTERFACE(name) \
	const z80ctc_interface (name)=


#define MDRV_Z80CTC_ADD(_tag, _clock, _intrf) \
	MDRV_DEVICE_ADD(_tag, Z80CTC, _clock) \
	MDRV_DEVICE_CONFIG(_intrf)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80ctc_interface

struct z80ctc_interface
{
	UINT8				m_notimer;	// timer disabler mask
	devcb_write_line	m_intr;		// callback when change interrupt status
	devcb_write_line	m_zc0;		// ZC/TO0 callback
	devcb_write_line	m_zc1;		// ZC/TO1 callback
	devcb_write_line	m_zc2;		// ZC/TO2 callback
};



// ======================> z80ctc_device_config

class z80ctc_device_config :	public device_config,
								public device_config_z80daisy_interface,
								public z80ctc_interface
{
	friend class z80ctc_device;

	// construction/destruction
	z80ctc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

protected:
	// device_config overrides
	virtual void device_config_complete();
};



// ======================> z80ctc_device

class z80ctc_device :	public device_t,
						public device_z80daisy_interface
{
	friend class z80ctc_device_config;

	// construction/destruction
	z80ctc_device(running_machine &_machine, const z80ctc_device_config &_config);

public:
	// state getters
	attotime period(int ch) const { return m_channel[ch].period(); }

	// I/O operations
	UINT8 read(int ch) { return m_channel[ch].read(); }
	void write(int ch, UINT8 data) { m_channel[ch].write(data); }
	void trigger(int ch, UINT8 data) { m_channel[ch].trigger(data); }

private:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

	// internal helpers
	void interrupt_check();
	void timercallback(int chanindex);

	// a single channel within the CTC
	class ctc_channel
	{
	public:
		ctc_channel();

		void start(z80ctc_device *device, int index, bool notimer, const devcb_write_line *write_line);
		void reset();

		UINT8 read();
		void write(UINT8 data);

		attotime period() const;
		void trigger(UINT8 data);
		void timer_callback();

		z80ctc_device *	m_device;				// pointer back to our device
		int				m_index;				// our channel index
		devcb_resolved_write_line m_zc;			// zero crossing callbacks
		bool			m_notimer;				// timer disabled?
		UINT16			m_mode;					// current mode
		UINT16			m_tconst;				// time constant
		UINT16			m_down;					// down counter (clock mode only)
		UINT8			m_extclk;				// current signal from the external clock
		emu_timer *		m_timer;				// array of active timers
		UINT8			m_int_state;			// interrupt status (for daisy chain)

	private:
		static TIMER_CALLBACK( static_timer_callback ) { reinterpret_cast<z80ctc_device::ctc_channel *>(ptr)->timer_callback(); }
	};

	// internal state
	const z80ctc_device_config &m_config;
	devcb_resolved_write_line m_intr;			// interrupt callback

	UINT8				m_vector;				// interrupt vector
	attotime			m_period16;				// 16/system clock
	attotime			m_period256;			// 256/system clock
	ctc_channel			m_channel[4];			// data for each channel
};


// device type definition
extern const device_type Z80CTC;



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE8_DEVICE_HANDLER( z80ctc_w );
READ8_DEVICE_HANDLER( z80ctc_r );

WRITE_LINE_DEVICE_HANDLER( z80ctc_trg0_w );
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg1_w );
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg2_w );
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg3_w );


#endif
