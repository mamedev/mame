// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADC0844

    A/D Converter With Multiplexer Options

                ___ ___
       /RD   1 |*  u   | 20  VCC
       /CS   2 |       | 19  /WR
       CH1   3 |       | 18  /INTR
       CH2   4 |       | 17  DB0/MA0
       CH3   5 |       | 16  DB1/MA1
       CH4   6 |       | 15  DB2/MA2
      AGND   7 |       | 14  DB3/MA3
      VREF   8 |       | 13  DB4
       DB7   9 |       | 12  DB5
      DGND  10 |_______| 11  DB6

***************************************************************************/

#ifndef MAME_DEVICES_MACHINE_ADC0844_H
#define MAME_DEVICES_MACHINE_ADC0844_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ADC0844_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ADC0844, 0)

#define MCFG_ADC0844_INTR_CB(_devcb) \
	devcb = &adc0844_device::set_intr_callback(*device, DEVCB_##_devcb);

#define MCFG_ADC0844_CH1_CB(_devcb) \
	devcb = &adc0844_device::set_ch1_callback(*device, DEVCB_##_devcb);

#define MCFG_ADC0844_CH2_CB(_devcb) \
	devcb = &adc0844_device::set_ch2_callback(*device, DEVCB_##_devcb);

#define MCFG_ADC0844_CH3_CB(_devcb) \
	devcb = &adc0844_device::set_ch3_callback(*device, DEVCB_##_devcb);

#define MCFG_ADC0844_CH4_CB(_devcb) \
	devcb = &adc0844_device::set_ch4_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class adc0844_device : public device_t
{
public:
	// construction/destruction
	adc0844_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <class Object> static devcb_base &set_intr_callback(device_t &device, Object &&cb)
	{ return downcast<adc0844_device &>(device).m_intr_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> static devcb_base &set_ch1_callback(device_t &device, Object &&cb)
	{ return downcast<adc0844_device &>(device).m_channel_cb[0].set_callback(std::forward<Object>(cb)); }

	template <class Object> static devcb_base &set_ch2_callback(device_t &device, Object &&cb)
	{ return downcast<adc0844_device &>(device).m_channel_cb[1].set_callback(std::forward<Object>(cb)); }

	template <class Object> static devcb_base &set_ch3_callback(device_t &device, Object &&cb)
	{ return downcast<adc0844_device &>(device).m_channel_cb[2].set_callback(std::forward<Object>(cb)); }

	template <class Object> static devcb_base &set_ch4_callback(device_t &device, Object &&cb)
	{ return downcast<adc0844_device &>(device).m_channel_cb[3].set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint8_t clamp(int value);

	// callbacks
	devcb_write_line m_intr_cb;
	devcb_read8 m_channel_cb[4];

	emu_timer *m_conversion_timer;

	// state
	int m_channel;
	uint8_t m_result;
};

// device type definition
DECLARE_DEVICE_TYPE(ADC0844, adc0844_device)

#endif // MAME_DEVICES_MACHINE_ADC0844_H
