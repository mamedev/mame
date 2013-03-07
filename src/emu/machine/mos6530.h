/**********************************************************************

    MOS Technology 6530 Memory, I/O, Timer Array emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 40  PA1
                   PA0   2 |             | 39  PA2
                  phi2   3 |             | 38  PA3
                   RS0   4 |             | 37  PA4
                    A9   5 |             | 36  PA5
                    A8   6 |             | 35  PA6
                    A7   7 |             | 34  PA7
                    A6   8 |             | 33  DB0
                   R/W   9 |             | 32  DB1
                    A5  10 |   MCS6530   | 31  DB2
                    A4  11 |             | 30  DB3
                    A3  12 |             | 29  DB4
                    A2  13 |             | 28  DB5
                    A1  14 |             | 27  DB6
                    A0  15 |             | 26  DB7
                  _RES  16 |             | 25  PB0
               IRQ/PB7  17 |             | 24  PB1
               CS1/PB6  18 |             | 23  PB2
               CS2/PB5  19 |             | 22  PB3
                   Vcc  20 |_____________| 21  PB4

**********************************************************************/

#ifndef __MIOT6530_H__
#define __MIOT6530_H__

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class mos6530_device : public device_t
{
public:
	mos6530_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mos6530_device() { global_free(m_token); }

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

extern const device_type MOS6530;


#define MCFG_MOS6530_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD((_tag), MOS6530, _clock)    \
	MCFG_DEVICE_CONFIG(_config)

#define MOS6530_INTERFACE(name) \
	const mos6530_interface (name) =

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct mos6530_interface
{
	devcb_read8             in_pa_func;
	devcb_write8            out_pa_func;

	devcb_read8             in_pb_func;
	devcb_write8            out_pb_func;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

DECLARE_READ8_DEVICE_HANDLER( mos6530_r );
DECLARE_WRITE8_DEVICE_HANDLER( mos6530_w );

void mos6530_porta_in_set(device_t *device, UINT8 data, UINT8 mask);
void mos6530_portb_in_set(device_t *device, UINT8 data, UINT8 mask);

UINT8 mos6530_porta_in_get(device_t *device);
UINT8 mos6530_portb_in_get(device_t *device);

UINT8 mos6530_porta_out_get(device_t *device);
UINT8 mos6530_portb_out_get(device_t *device);

#endif
