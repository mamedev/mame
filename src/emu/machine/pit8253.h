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

#include "devlegcy.h"
#include "devcb.h"


struct pit8253_config
{
	struct
	{
		double				clockin;		/* timer clock */
		devcb_read_line		in_gate_func;	/* gate signal */
		devcb_write_line	out_out_func;	/* out signal */
	} timer[3];
};

class pit8253_device : public device_t
{
public:
	pit8253_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	pit8253_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~pit8253_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type PIT8253;

class pit8254_device : public pit8253_device
{
public:
	pit8254_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

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


DECLARE_READ8_DEVICE_HANDLER( pit8253_r );
DECLARE_WRITE8_DEVICE_HANDLER( pit8253_w );

WRITE_LINE_DEVICE_HANDLER( pit8253_clk0_w );
WRITE_LINE_DEVICE_HANDLER( pit8253_clk1_w );
WRITE_LINE_DEVICE_HANDLER( pit8253_clk2_w );

WRITE_LINE_DEVICE_HANDLER( pit8253_gate0_w );
WRITE_LINE_DEVICE_HANDLER( pit8253_gate1_w );
WRITE_LINE_DEVICE_HANDLER( pit8253_gate2_w );

/* In the 8253/8254 the CLKx input lines can be attached to a regular clock
   signal. Another option is to use the output from one timer as the input
   clock to another timer.

   The functions below should supply both functionalities. If the signal is
   a regular clock signal, use the pit8253_set_clockin function. If the
   CLKx input signal is the output of the different source, set the new_clockin
   to 0 with pit8253_set_clockin and call pit8253_clkX_w to change
   the state of the input CLKx signal.
 */
int pit8253_get_output(device_t *device, int timer);
void pit8253_set_clockin(device_t *device, int timer, double new_clockin);


#endif	/* __PIT8253_H__ */
