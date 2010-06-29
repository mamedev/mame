/***************************************************************************

    Zilog Z80 Parallel Input/Output Controller implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************
                            _____   _____
                    D2   1 |*    \_/     | 40  D3
                    D7   2 |             | 39  D4
                    D6   3 |             | 38  D5
                   _CE   4 |             | 37  _M1
                  C/_D   5 |             | 36  _IORQ
                  B/_A   6 |             | 35  RD
                   PA7   7 |             | 34  PB7
                   PA6   8 |             | 33  PB6
                   PA5   9 |             | 32  PB5
                   PA4  10 |    Z8420    | 31  PB4
                   GND  11 |             | 30  PB3
                   PA3  12 |             | 29  PB2
                   PA2  13 |             | 28  PB1
                   PA1  14 |             | 27  PB0
                   PA0  15 |             | 26  +5V
                 _ASTB  16 |             | 25  CLK
                 _BSTB  17 |             | 24  IEI
                  ARDY  18 |             | 23  _INT
                    D0  19 |             | 22  IEO
                    D1  20 |_____________| 21  BRDY

***************************************************************************/

#ifndef __Z80PIO__
#define __Z80PIO__

#include "cpu/z80/z80daisy.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_Z80PIO_ADD(_tag, _clock, _intrf) \
	MDRV_DEVICE_ADD(_tag, Z80PIO, _clock) \
	MDRV_DEVICE_CONFIG(_intrf)

#define Z80PIO_INTERFACE(_name) \
	const z80pio_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80pio_interface

struct z80pio_interface
{
	devcb_write_line	m_out_int_func;

	devcb_read8			m_in_pa_func;
	devcb_write8		m_out_pa_func;
	devcb_write_line	m_out_ardy_func;

	devcb_read8			m_in_pb_func;
	devcb_write8		m_out_pb_func;
	devcb_write_line	m_out_brdy_func;
};



// ======================> z80pio_device_config

class z80pio_device_config :	public device_config,
								public device_config_z80daisy_interface,
								public z80pio_interface
{
	friend class z80pio_device;

	// construction/destruction
	z80pio_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

protected:
	// device_config overrides
	virtual void device_config_complete();
};



// ======================> z80pio_device

class z80pio_device :	public device_t,
						public device_z80daisy_interface
{
	friend class z80pio_device_config;

	// construction/destruction
	z80pio_device(running_machine &_machine, const z80pio_device_config &config);

public:
	enum
	{
		PORT_A = 0,
		PORT_B,
		PORT_COUNT
	};

	// I/O line access
	int rdy(int which) { return m_port[which].rdy(); }
	void strobe(int which, bool state) { m_port[which].strobe(state); }

	// control register I/O
	UINT8 control_read();
	void control_write(int offset, UINT8 data) { m_port[offset & 1].control_write(data); }

	// data register I/O
	UINT8 data_read(int offset) { return m_port[offset & 1].data_read(); }
	void data_write(int offset, UINT8 data) { m_port[offset & 1].data_write(data); }

	// port I/O
	UINT8 port_read(int offset) { return m_port[offset & 1].read(); }
	void port_write(int offset, UINT8 data) { m_port[offset & 1].write(data); }

private:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

	// internal helpers
	void check_interrupts();

	// a single PIO port
	class pio_port
	{
		friend class z80pio_device;

	public:
		pio_port();

		void start(z80pio_device *device, int index, const devcb_read8 &infunc, const devcb_write8 &outfunc, const devcb_write_line &rdyfunc);
		void reset();

		bool interrupt_signalled();
		void trigger_interrupt();

		int rdy() const { return m_rdy; }
		void set_rdy(bool state);
		void set_mode(int mode);
		void strobe(bool state);

		UINT8 read();
		void write(UINT8 data);

		void control_write(UINT8 data);

		UINT8 data_read();
		void data_write(UINT8 data);

	private:
		void check_interrupts() { m_device->check_interrupts(); }

		z80pio_device *				m_device;
		int							m_index;

		devcb_resolved_read8		m_in_p_func;
		devcb_resolved_write8		m_out_p_func;
		devcb_resolved_write_line	m_out_rdy_func;

		int m_mode;					// mode register
		int m_next_control_word;	// next control word
		UINT8 m_input;				// input latch
		UINT8 m_output;				// output latch
		UINT8 m_ior;				// input/output register
		bool m_rdy;					// ready
		bool m_stb;					// strobe

		// interrupts
		bool m_ie;					// interrupt enabled
		bool m_ip;					// interrupt pending
		bool m_ius;					// interrupt under service
		UINT8 m_icw;				// interrupt control word
		UINT8 m_vector;				// interrupt vector
		UINT8 m_mask;				// interrupt mask
		bool m_match;				// logic equation match
	};

	// internal state
	const z80pio_device_config &m_config;
	pio_port					m_port[2];
	devcb_resolved_write_line	m_out_int_func;
};


// device type definition
extern const device_type Z80PIO;



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// control register access
READ8_DEVICE_HANDLER( z80pio_c_r );
WRITE8_DEVICE_HANDLER( z80pio_c_w );

// data register access
READ8_DEVICE_HANDLER( z80pio_d_r );
WRITE8_DEVICE_HANDLER( z80pio_d_w );

// register access
READ8_DEVICE_HANDLER( z80pio_cd_ba_r );
WRITE8_DEVICE_HANDLER( z80pio_cd_ba_w );

READ8_DEVICE_HANDLER( z80pio_ba_cd_r );
WRITE8_DEVICE_HANDLER( z80pio_ba_cd_w );

// port access
READ8_DEVICE_HANDLER( z80pio_pa_r );
WRITE8_DEVICE_HANDLER( z80pio_pa_w );

READ8_DEVICE_HANDLER( z80pio_pb_r );
WRITE8_DEVICE_HANDLER( z80pio_pb_w );

// ready
READ_LINE_DEVICE_HANDLER( z80pio_ardy_r );
READ_LINE_DEVICE_HANDLER( z80pio_brdy_r );

// strobe
WRITE_LINE_DEVICE_HANDLER( z80pio_astb_w );
WRITE_LINE_DEVICE_HANDLER( z80pio_bstb_w );

#endif
