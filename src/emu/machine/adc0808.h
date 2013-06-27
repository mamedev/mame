/**********************************************************************

    National Semiconductor ADC0808/ADC0809 8-Bit A/D Converter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   IN3   1 |*    \_/     | 28  IN2
                   IN4   2 |             | 27  IN1
                   IN5   3 |             | 26  IN0
                   IN6   4 |             | 25  ADD A
                   IN7   5 |             | 24  ADD B
                 START   6 |             | 23  ADD C
                   EOC   7 |   ADC0808   | 22  ALE
                   2-5   8 |   ADC0809   | 21  2-1 MSB
         OUTPUT ENABLE   9 |             | 20  2-2
                 CLOCK  10 |             | 19  2-3
                   Vcc  11 |             | 18  2-4
                 Vref+  12 |             | 17  2-8 LSB
                   GND  13 |             | 16  Vref-
                   2-7  14 |_____________| 15  2-6

**********************************************************************/

#pragma once

#ifndef __ADC0808__
#define __ADC0808__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ADC0808_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, ADC0808, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define ADC0808_INTERFACE(name) \
	const adc0808_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adc0808_analog_read

typedef double (*adc0808_analog_read) (device_t *device);
#define ADC0808_ANALOG_READ(name) double name(device_t *device)


// ======================> adc0808_interface

struct adc0808_interface
{
	devcb_write_line        m_out_eoc_cb;

	adc0808_analog_read     m_in_vref_pos_func;
	adc0808_analog_read     m_in_vref_neg_func;

	adc0808_analog_read     m_in_in_func[8];
};


// ======================> adc0808_device

class adc0808_device :  public device_t,
						public adc0808_interface
{
public:
	// construction/destruction
	adc0808_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( ale_w );

	DECLARE_WRITE_LINE_MEMBER( start_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	devcb_resolved_write_line           m_out_eoc_func;

	int m_address;                      // analog channel address
	int m_start;                        // start conversion pin
	int m_eoc;                          // end of conversion pin
	int m_next_eoc;                     // next value end of conversion pin

	UINT8 m_sar;                        // successive approximation register

	int m_cycle;                        // clock cycle counter
	int m_bit;                          // bit counter

	// timers
	emu_timer *m_cycle_timer;
};


// device type definition
extern const device_type ADC0808;



#endif
