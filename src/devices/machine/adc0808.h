// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor ADC0808/ADC0809 8-Bit A/D Converter emulation

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
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adc0808_analog_read

typedef device_delegate<double ()> adc0808_analog_read_delegate;
#define ADC0808_ANALOG_READ_CB(name)  double name()


#define MCFG_ADC0808_OUT_EOC_CB(_devcb) \
	devcb = &adc0808_device::set_out_eoc_callback(*device, DEVCB_##_devcb);

#define MCFG_ADC0808_IN_VREF_POS_CB(_class, _method) \
	adc0808_device::set_in_vref_pos_callback(*device, adc0808_analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_VREF_NEG_CB(_class, _method) \
	adc0808_device::set_in_vref_neg_callback(*device, adc0808_analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_0_CB(_class, _method) \
	adc0808_device::set_in_in_0_callback(*device, adc0808_analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_1_CB(_class, _method) \
	adc0808_device::set_in_in_1_callback(*device, adc0808_analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_2_CB(_class, _method) \
	adc0808_device::set_in_in_2_callback(*device, adc0808_analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_3_CB(_class, _method) \
	adc0808_device::set_in_in_3_callback(*device, adc0808_analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_4_CB(_class, _method) \
	adc0808_device::set_in_in_4_callback(*device, adc0808_analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_5_CB(_class, _method) \
	adc0808_device::set_in_in_5_callback(*device, adc0808_analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_6_CB(_class, _method) \
	adc0808_device::set_in_in_6_callback(*device, adc0808_analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_7_CB(_class, _method) \
	adc0808_device::set_in_in_7_callback(*device, adc0808_analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

// ======================> adc0808_device

class adc0808_device :  public device_t
{
public:
	// construction/destruction
	adc0808_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_eoc_callback(device_t &device, _Object object) { return downcast<adc0808_device &>(device).m_out_eoc_cb.set_callback(object); }
	static void set_in_vref_pos_callback(device_t &device, adc0808_analog_read_delegate callback) { downcast<adc0808_device &>(device).m_in_vref_pos_cb = callback; }
	static void set_in_vref_neg_callback(device_t &device, adc0808_analog_read_delegate callback) { downcast<adc0808_device &>(device).m_in_vref_neg_cb = callback; }
	static void set_in_in_0_callback(device_t &device, adc0808_analog_read_delegate callback) { downcast<adc0808_device &>(device).m_in_in_0_cb = callback; }
	static void set_in_in_1_callback(device_t &device, adc0808_analog_read_delegate callback) { downcast<adc0808_device &>(device).m_in_in_1_cb = callback; }
	static void set_in_in_2_callback(device_t &device, adc0808_analog_read_delegate callback) { downcast<adc0808_device &>(device).m_in_in_2_cb = callback; }
	static void set_in_in_3_callback(device_t &device, adc0808_analog_read_delegate callback) { downcast<adc0808_device &>(device).m_in_in_3_cb = callback; }
	static void set_in_in_4_callback(device_t &device, adc0808_analog_read_delegate callback) { downcast<adc0808_device &>(device).m_in_in_4_cb = callback; }
	static void set_in_in_5_callback(device_t &device, adc0808_analog_read_delegate callback) { downcast<adc0808_device &>(device).m_in_in_5_cb = callback; }
	static void set_in_in_6_callback(device_t &device, adc0808_analog_read_delegate callback) { downcast<adc0808_device &>(device).m_in_in_6_cb = callback; }
	static void set_in_in_7_callback(device_t &device, adc0808_analog_read_delegate callback) { downcast<adc0808_device &>(device).m_in_in_7_cb = callback; }

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( ale_w );

	DECLARE_WRITE_LINE_MEMBER( start_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	devcb_write_line           m_out_eoc_cb;
	adc0808_analog_read_delegate     m_in_vref_pos_cb;
	adc0808_analog_read_delegate     m_in_vref_neg_cb;
	adc0808_analog_read_delegate     m_in_in_0_cb;
	adc0808_analog_read_delegate     m_in_in_1_cb;
	adc0808_analog_read_delegate     m_in_in_2_cb;
	adc0808_analog_read_delegate     m_in_in_3_cb;
	adc0808_analog_read_delegate     m_in_in_4_cb;
	adc0808_analog_read_delegate     m_in_in_5_cb;
	adc0808_analog_read_delegate     m_in_in_6_cb;
	adc0808_analog_read_delegate     m_in_in_7_cb;

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
