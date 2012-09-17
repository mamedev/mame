/***************************************************************************

    National Semiconductor INS8154

    N-Channel 128-by-8 Bit RAM Input/Output (RAM I/O)

                            _____   _____
                   PB6   1 |*    \_/     | 40  VCC
                   PB5   2 |             | 39  PB7
                   PB4   3 |             | 38  NWDS
                   PB3   4 |             | 37  NRDS
                   PB2   5 |             | 36  NRST
                   PB1   6 |             | 35  _CS0
                   PB0   7 |             | 34  CS1
                   DB7   8 |             | 33  M/_IO
                   DB6   9 |             | 32  AD6
                   DB5  10 |   INS8154   | 31  AD5
                   DB4  11 |             | 30  AD4
                   DB3  12 |             | 29  AD3
                   DB2  13 |             | 28  AD2
                   DB1  14 |             | 27  AD1
                   DB0  15 |             | 26  AD0
                   PA7  16 |             | 25  INTR
                   PA6  17 |             | 24  PA0
                   PA5  18 |             | 23  PA1
                   PA4  19 |             | 22  PA2
                   GND  20 |_____________| 21  PA3

***************************************************************************/

#pragma once

#ifndef __INS8154_H__
#define __INS8154_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_INS8154_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, INS8154, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> ins8154_interface

struct ins8154_interface
{
	devcb_read8			m_in_a_cb;
	devcb_write8		m_out_a_cb;
	devcb_read8			m_in_b_cb;
	devcb_write8		m_out_b_cb;
	devcb_write_line	m_out_irq_cb;
};



// ======================> ins8154_device

class ins8154_device :  public device_t,
                        public ins8154_interface
{
public:
    // construction/destruction
    ins8154_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	UINT8 ins8154_r(UINT32 offset);
	void ins8154_w(UINT32 offset, UINT8 data);

	void ins8154_porta_w(UINT32 offset, UINT8 data);
	void ins8154_portb_w(UINT32 offset, UINT8 data);

protected:
    // device-level overrides
    virtual void device_config_complete();
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

private:

	/* i/o lines */
	devcb_resolved_read8 m_in_a_func;
	devcb_resolved_write8 m_out_a_func;
	devcb_resolved_read8 m_in_b_func;
	devcb_resolved_write8 m_out_b_func;
	devcb_resolved_write_line m_out_irq_func;

	/* registers */
	UINT8 m_in_a;  /* Input Latch Port A */
	UINT8 m_in_b;  /* Input Latch Port B */
	UINT8 m_out_a; /* Output Latch Port A */
	UINT8 m_out_b; /* Output Latch Port B */
	UINT8 m_mdr;   /* Mode Definition Register */
	UINT8 m_odra;  /* Output Definition Register Port A */
	UINT8 m_odrb;  /* Output Definition Register Port B */
};


// device type definition
extern const device_type INS8154;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

DECLARE_READ8_DEVICE_HANDLER( ins8154_r );
DECLARE_WRITE8_DEVICE_HANDLER( ins8154_w );

DECLARE_WRITE8_DEVICE_HANDLER( ins8154_porta_w );
DECLARE_WRITE8_DEVICE_HANDLER( ins8154_portb_w );


#endif /* __INS8154_H__ */
