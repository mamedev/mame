// license:GPL-2.0+
// copyright-holders:Dirk Best
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

#define MCFG_INS8154_IN_A_CB(_devcb) \
	devcb = &ins8154_device::set_in_a_callback(*device, DEVCB_##_devcb);

#define MCFG_INS8154_OUT_A_CB(_devcb) \
	devcb = &ins8154_device::set_out_a_callback(*device, DEVCB_##_devcb);

#define MCFG_INS8154_IN_B_CB(_devcb) \
	devcb = &ins8154_device::set_in_b_callback(*device, DEVCB_##_devcb);

#define MCFG_INS8154_OUT_B_CB(_devcb) \
	devcb = &ins8154_device::set_out_b_callback(*device, DEVCB_##_devcb);

#define MCFG_INS8154_OUT_IRQ_CB(_devcb) \
	devcb = &ins8154_device::set_out_irq_callback(*device, DEVCB_##_devcb); //currently unused

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> ins8154_device

class ins8154_device :  public device_t
{
public:
	// construction/destruction
	ins8154_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_in_a_callback(device_t &device, _Object object) { return downcast<ins8154_device &>(device).m_in_a_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_a_callback(device_t &device, _Object object) { return downcast<ins8154_device &>(device).m_out_a_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_b_callback(device_t &device, _Object object) { return downcast<ins8154_device &>(device).m_in_b_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_b_callback(device_t &device, _Object object) { return downcast<ins8154_device &>(device).m_out_b_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_irq_callback(device_t &device, _Object object) { return downcast<ins8154_device &>(device).m_out_irq_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( ins8154_r );
	DECLARE_WRITE8_MEMBER( ins8154_w );

	DECLARE_WRITE8_MEMBER( ins8154_porta_w );
	DECLARE_WRITE8_MEMBER( ins8154_portb_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

private:

	/* i/o lines */
	devcb_read8         m_in_a_cb;
	devcb_write8        m_out_a_cb;
	devcb_read8         m_in_b_cb;
	devcb_write8        m_out_b_cb;
	devcb_write_line    m_out_irq_cb;

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

#endif /* __INS8154_H__ */
