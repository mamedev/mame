/**********************************************************************

    SMC KR2376 Keyboard Encoder emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   Vcc   1 |*    \_/     | 40  Frequency Control A
   Frequency Control B   2 |             | 39  X0
   Frequency Control C   3 |             | 38  X1
           Shift Input   4 |             | 37  X2
         Control Input   5 |             | 36  X3
   Parity Invert Input   6 |             | 35  X4
         Parity Output   7 |             | 34  X5
        Data Output B8   8 |             | 33  X6
        Data Output B7   9 |             | 32  X7
        Data Output B6  10 |   KR2376    | 31  Y0
        Data Output B5  11 |             | 30  Y1
        Data Output B4  12 |             | 29  Y2
        Data Output B3  13 |             | 28  Y3
        Data Output B2  14 |             | 27  Y4
        Data Output B1  15 |             | 26  Y5
         Strobe Output  16 |             | 25  Y6
                Ground  17 |             | 24  Y7
                   Vgg  18 |             | 23  Y8
  Strobe Control Input  19 |             | 22  Y9
          Invert Input  20 |_____________| 21  Y10

**********************************************************************/

#ifndef __KR2376__
#define __KR2376__

/*
 * Input pins
 */
enum kr2376_input_pin_t
{
	KR2376_DSII=20,         /* DSII  - Pin 20 - Data & Strobe Invert Input */
	KR2376_PII=6            /* PII   - Pin  6 - Parity Invert Input */
};

enum kr2376_output_pin_t
{
	KR2376_SO=16,           /* SO    - Pin 16 - Strobe Output */
	KR2376_PO=7         /* PO    - Pin  7 - Parity Output */
};

typedef void (*kr2376_on_strobe_changed_func) (device_t *device, int level);
#define KR2376_ON_STROBE_CHANGED(name) void name(device_t *device, int level)

/* interface */
struct kr2376_interface
{
	/* The clock of the chip (Typical 50 kHz) */
	int m_our_clock;

	/* This will be called for every change of the strobe pin (pin 16). Optional */
	devcb_write_line m_on_strobe_changed_cb;
};

class kr2376_device : public device_t,
								public kr2376_interface
{
public:
	kr2376_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~kr2376_device() {}
	
	/* keyboard data */
	DECLARE_READ8_MEMBER( data_r );

	/* Set an input pin */
	void set_input_pin( kr2376_input_pin_t pin, int data );

	/* Get an output pin */
	int get_output_pin( kr2376_output_pin_t pin );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual ioport_constructor device_input_ports() const;
	
private:
	// internal state
	int m_pins[41];

	int m_ring11;                     /* sense input scan counter */
	int m_ring8;                      /* drive output scan counter */
	int m_modifiers;                  /* modifier inputs */

	int m_strobe;                     /* strobe output */
	int m_strobe_old;
	int m_parity;
	int m_data;

	/* timers */
	emu_timer *m_scan_timer;          /* keyboard scan timer */
	devcb_resolved_write_line m_on_strobe_changed;
	
	enum
	{
		TIMER_SCAN_TICK
	};
	
	void change_output_lines();
	void clock_scan_counters();
	void detect_keypress();
};

extern const device_type KR2376;


#define MCFG_KR2376_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, KR2376, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define KR2376_INTERFACE(name) const kr2376_interface (name)=

#endif
