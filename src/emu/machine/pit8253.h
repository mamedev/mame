/***************************************************************************

    Intel 8253/8254

    Programmable Interval Timer

                            _____   _____
                    D7   1 |*    \_/     | 24  VCC
                    D6   2 |             | 23  _WR
                    D5   3 |             | 22  _RD
                    D4   4 |             | 21  _CS
                    D3   5 |             | 20  A1
                    D2   6 |    8253     | 19  A0
                    D1   7 |             | 18  CLK2
                    D0   8 |             | 17  OUT2
                  CLK0   9 |             | 16  GATE2
                  OUT0  10 |             | 15  CLK1
                 GATE0  11 |             | 14  GATE1
                   GND  12 |_____________| 13  OUT1

***************************************************************************/

#ifndef __PIT8253_H__
#define __PIT8253_H__


#define PIT8253_MAX_TIMER       3


/* device types */
enum
{
	TYPE_PIT8253 = 0,
	TYPE_PIT8254
};

struct pit8253_interface
{
	struct
	{
		double              clockin;        /* timer clock */
		devcb_read_line     in_gate_func;   /* gate signal */
		devcb_write_line    out_out_func;   /* out signal */
	} m_intf_timer[3];
};



struct pit8253_timer
{
	int index;                      /* index number of the timer */
	double clockin;                 /* input clock frequency in Hz */
	int clock;                      /* clock signal when clockin is 0 */

	devcb_resolved_read_line    in_gate_func;   /* callback for gate input */
	devcb_resolved_write_line   out_out_func;   /* callback function for when output changes */

	attotime last_updated;          /* time when last updated */

	emu_timer *updatetimer;         /* MAME timer to process updates */

	UINT16 value;                   /* current counter value ("CE" in Intel docs) */
	UINT16 latch;                   /* latched counter value ("OL" in Intel docs) */
	UINT16 count;                   /* new counter value ("CR" in Intel docs) */
	UINT8 control;                  /* 6-bit control byte */
	UINT8 status;                   /* status byte - 8254 only */
	UINT8 lowcount;                 /* LSB of new counter value for 16-bit writes */
	INT32 rmsb;                     /* 1 = Next read is MSB of 16-bit value */
	INT32 wmsb;                     /* 1 = Next write is MSB of 16-bit value */
	INT32 output;                       /* 0 = low, 1 = high */

	INT32 gate;                     /* gate input (0 = low, 1 = high) */
	INT32 latched_count;                /* number of bytes of count latched */
	INT32 latched_status;               /* 1 = status latched (8254 only) */
	INT32 null_count;                   /* 1 = mode control or count written, 0 = count loaded */
	INT32 phase;                        /* see phase definition tables in simulate2(), below */

	UINT32 cycles_to_output;        /* cycles until output callback called */
};

class pit8253_device : public device_t,
						public pit8253_interface
{
public:
	pit8253_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	pit8253_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~pit8253_device() {}


	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	WRITE_LINE_MEMBER(clk0_w);
	WRITE_LINE_MEMBER(clk1_w);
	WRITE_LINE_MEMBER(clk2_w);

	WRITE_LINE_MEMBER(gate0_w);
	WRITE_LINE_MEMBER(gate1_w);
	WRITE_LINE_MEMBER(gate2_w);


	/* In the 8253/8254 the CLKx input lines can be attached to a regular clock
	 signal. Another option is to use the output from one timer as the input
	 clock to another timer.

	 The functions below should supply both functionalities. If the signal is
	 a regular clock signal, use the pit8253_set_clockin function. If the
	 CLKx input signal is the output of the different source, set the new_clockin
	 to 0 with pit8253_set_clockin and call pit8253_clkX_w to change
	 the state of the input CLKx signal.
	 */
	int get_output(int timer);
	void set_clockin(int timer, double new_clockin);


protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();


	// internal state
	void common_start(int device_type);
	pit8253_timer *get_timer(int which);
	int pit8253_gate(pit8253_timer *timer);
	void decrease_counter_value(pit8253_timer *timer, UINT64 cycles);
	void load_counter_value(pit8253_timer *timer);
	void set_output(pit8253_timer *timer, int output);
	void simulate2(pit8253_timer *timer, INT64 elapsed_cycles);
	void simulate(pit8253_timer *timer, INT64 elapsed_cycles);
	void update(pit8253_timer *timer);
	UINT16 masked_value(pit8253_timer *timer);
	void load_count(pit8253_timer *timer, UINT16 newcount);
	void readback(pit8253_timer *timer, int command);
	void gate_w(int gate, int state);
	void set_clock_signal(int timerno, int state);

	TIMER_CALLBACK_MEMBER(update_timer_cb);

	int m_device_type;

	pit8253_timer m_timers[PIT8253_MAX_TIMER];
};



class pit8254_device : public pit8253_device
{
public:
	pit8254_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
};



extern const device_type PIT8253;
extern const device_type PIT8254;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_PIT8253_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, PIT8253, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#define MCFG_PIT8254_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, PIT8254, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#endif  /* __PIT8253_H__ */
