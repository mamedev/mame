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

#ifndef MAME_MACHINE_ADC0808_H
#define MAME_MACHINE_ADC0808_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adc0808_analog_read

#define ADC0808_ANALOG_READ_CB(name)  double name()


#define MCFG_ADC0808_OUT_EOC_CB(_devcb) \
	devcb = &adc0808_device::set_out_eoc_callback(*device, DEVCB_##_devcb);

#define MCFG_ADC0808_IN_VREF_POS_CB(_class, _method) \
	adc0808_device::set_in_vref_pos_callback(*device, adc0808_device::analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_VREF_NEG_CB(_class, _method) \
	adc0808_device::set_in_vref_neg_callback(*device, adc0808_device::analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_0_CB(_class, _method) \
	adc0808_device::set_in_in_0_callback(*device, adc0808_device::analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_1_CB(_class, _method) \
	adc0808_device::set_in_in_1_callback(*device, adc0808_device::analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_2_CB(_class, _method) \
	adc0808_device::set_in_in_2_callback(*device, adc0808_device::analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_3_CB(_class, _method) \
	adc0808_device::set_in_in_3_callback(*device, adc0808_device::analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_4_CB(_class, _method) \
	adc0808_device::set_in_in_4_callback(*device, adc0808_device::analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_5_CB(_class, _method) \
	adc0808_device::set_in_in_5_callback(*device, adc0808_device::analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_6_CB(_class, _method) \
	adc0808_device::set_in_in_6_callback(*device, adc0808_device::analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC0808_IN_IN_7_CB(_class, _method) \
	adc0808_device::set_in_in_7_callback(*device, adc0808_device::analog_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

// ======================> adc0808_device

class adc0808_device :  public device_t
{
public:
	typedef device_delegate<double ()> analog_read_delegate;

	// construction/destruction
	adc0808_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_out_eoc_callback(device_t &device, Object &&cb) { return downcast<adc0808_device &>(device).m_out_eoc_cb.set_callback(std::forward<Object>(cb)); }
	static void set_in_vref_pos_callback(device_t &device, analog_read_delegate &&cb) { downcast<adc0808_device &>(device).m_in_vref_pos_cb = std::move(cb); }
	static void set_in_vref_neg_callback(device_t &device, analog_read_delegate &&cb) { downcast<adc0808_device &>(device).m_in_vref_neg_cb = std::move(cb); }
	static void set_in_in_0_callback(device_t &device, analog_read_delegate &&cb) { downcast<adc0808_device &>(device).m_in_in_0_cb = std::move(cb); }
	static void set_in_in_1_callback(device_t &device, analog_read_delegate &&cb) { downcast<adc0808_device &>(device).m_in_in_1_cb = std::move(cb); }
	static void set_in_in_2_callback(device_t &device, analog_read_delegate &&cb) { downcast<adc0808_device &>(device).m_in_in_2_cb = std::move(cb); }
	static void set_in_in_3_callback(device_t &device, analog_read_delegate &&cb) { downcast<adc0808_device &>(device).m_in_in_3_cb = std::move(cb); }
	static void set_in_in_4_callback(device_t &device, analog_read_delegate &&cb) { downcast<adc0808_device &>(device).m_in_in_4_cb = std::move(cb); }
	static void set_in_in_5_callback(device_t &device, analog_read_delegate &&cb) { downcast<adc0808_device &>(device).m_in_in_5_cb = std::move(cb); }
	static void set_in_in_6_callback(device_t &device, analog_read_delegate &&cb) { downcast<adc0808_device &>(device).m_in_in_6_cb = std::move(cb); }
	static void set_in_in_7_callback(device_t &device, analog_read_delegate &&cb) { downcast<adc0808_device &>(device).m_in_in_7_cb = std::move(cb); }

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( ale_w );

	DECLARE_WRITE_LINE_MEMBER( start_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	devcb_write_line        m_out_eoc_cb;
	analog_read_delegate    m_in_vref_pos_cb;
	analog_read_delegate    m_in_vref_neg_cb;
	analog_read_delegate    m_in_in_0_cb;
	analog_read_delegate    m_in_in_1_cb;
	analog_read_delegate    m_in_in_2_cb;
	analog_read_delegate    m_in_in_3_cb;
	analog_read_delegate    m_in_in_4_cb;
	analog_read_delegate    m_in_in_5_cb;
	analog_read_delegate    m_in_in_6_cb;
	analog_read_delegate    m_in_in_7_cb;

	int m_address;                      // analog channel address
	int m_start;                        // start conversion pin
	int m_eoc;                          // end of conversion pin
	int m_next_eoc;                     // next value end of conversion pin

	uint8_t m_sar;                        // successive approximation register

	int m_cycle;                        // clock cycle counter
	int m_bit;                          // bit counter

	// timers
	emu_timer *m_cycle_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(ADC0808, adc0808_device)

#endif // MAME_MACHINE_ADC0808_H
