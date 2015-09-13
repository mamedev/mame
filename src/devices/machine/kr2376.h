// license:???
// copyright-holders:Roberto Lavarone
/**********************************************************************

    SMC KR2376 Keyboard Encoder emulation

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


#define MCFG_KR2376_STROBE_CALLBACK(_write) \
	devcb = &kr2376_device::set_strobe_wr_callback(*device, DEVCB_##_write);

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

class kr2376_device : public device_t
{
public:
	kr2376_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~kr2376_device() {}

	template<class _Object> static devcb_base &set_strobe_wr_callback(device_t &device, _Object object) { return downcast<kr2376_device &>(device).m_write_strobe.set_callback(object); }

	/* keyboard data */
	DECLARE_READ8_MEMBER( data_r );

	/* Set an input pin */
	void set_input_pin( kr2376_input_pin_t pin, int data );

	/* Get an output pin */
	int get_output_pin( kr2376_output_pin_t pin );

protected:
	// device-level overrides
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
	devcb_write_line m_write_strobe;

	enum
	{
		TIMER_SCAN_TICK
	};

	void change_output_lines();
	void clock_scan_counters();
	void detect_keypress();
};

extern const device_type KR2376;

#endif
