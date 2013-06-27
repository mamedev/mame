/*********************************************************************

    tms5501.h

    TMS 5501 code

*********************************************************************/

#ifndef __TMS5501_H__
#define __TMS5501_H__


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TMS5501_ADD(_tag, _clock, _intrf) \
	MCFG_DEVICE_ADD(_tag, TMS5501, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)

#define TMS5501_INTERFACE(name) \
	const tms5501_interface (name) =


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*tms5501_interrupt_cb)(device_t &device, int intreq, UINT8 vector);
#define TMS5501_IRQ_CALLBACK(name) void name(device_t &device, int intreq, UINT8 vector)


// ======================> tms5501_interface

struct tms5501_interface
{
	devcb_read8             m_pio_read_cb;       // PIO read
	devcb_write8            m_pio_write_cb;      // PIO write
	tms5501_interrupt_cb    m_interrupt_cb;      // interrupt callback
};

// ======================> tms5501_device

class tms5501_device :  public device_t,
						public tms5501_interface
{
public:
	// construction/destruction
	tms5501_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( set_pio_bit_7 );
	DECLARE_WRITE_LINE_MEMBER( set_sensor );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

public:
	int  find_first_bit(int value);
	void field_interrupts();
	void timer_decrementer(UINT8 mask);
	void timer_reload(int timer);

	static const device_timer_id TIMER_DECREMENTER = 0;

	// internal registers
	UINT8   m_status;                   // status register
	UINT8   m_command;                  // command register, bits 1-5 are latched

	UINT8   m_sio_rate;                 // SIO configuration register
	UINT8   m_sio_input_buffer;         // SIO input buffer
	UINT8   m_sio_output_buffer;        // SIO output buffer

	UINT8   m_pio_input_buffer;         // PIO input buffer
	UINT8   m_pio_output_buffer;        // PIO output buffer

	UINT8   m_interrupt_mask;           // interrupt mask register
	UINT8   m_pending_interrupts;       // pending interrupts register
	UINT8   m_interrupt_address;        // interrupt vector register

	int     m_sensor;                   // sensor input

	// internal timers
	UINT8       m_timer_counter[5];
	emu_timer * m_timer[5];

	// PIO callbacks
	devcb_resolved_read8    m_pio_read_func;
	devcb_resolved_write8   m_pio_write_func;
};


// device type definition
extern const device_type TMS5501;

#endif /* __TMS5501_H__ */
