// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADC0808/ADC0809

    A/D Converter with 8 Channel-Multiplexer

                ___ ___
       IN3   1 |*  u   | 28  IN2
       IN4   2 |       | 27  IN1
       IN5   3 |       | 26  IN0
       IN6   4 |       | 25  ADD A
       IN7   5 |       | 24  ADD B
     START   6 |       | 23  ADD C
       EOC   7 |       | 22  ALE
        D4   8 |       | 21  D0
        OE   9 |       | 20  D1
     CLOCK  10 |       | 19  D2
       VCC  11 |       | 18  D3
     VREF+  12 |       | 17  D7
       DND  13 |       | 16  VREF-
        D6  14 |_______| 15  D5

    Notes:
    * The difference between the two devices is the total adjusted
      error: ADC0808 ±½ LSB, ADC0809 ±1 LSB
    * MM74C949 and M58990P are equivalent to ADC0808
    * MM74C949-1 and M58990P-1 are equivalent to ADC0809
    * ADC0816 and ADC0817 are 16 channel equivalents

***************************************************************************/

#ifndef MAME_DEVICES_MACHINE_ADC0808_H
#define MAME_DEVICES_MACHINE_ADC0808_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ADC0808_EOC_CB(_devcb) \
	devcb = &downcast<adc0808_device &>(*device).set_eoc_callback(DEVCB_##_devcb);

// common hookup where the eoc output is connected to a flip-flop
#define MCFG_ADC0808_EOC_FF_CB(_devcb) \
	devcb = &downcast<adc0808_device &>(*device).set_eoc_ff_callback(DEVCB_##_devcb);

#define MCFG_ADC0808_IN0_CB(_devcb) \
	devcb = &downcast<adc0808_device &>(*device).set_in_callback(DEVCB_##_devcb, 0);

#define MCFG_ADC0808_IN1_CB(_devcb) \
	devcb = &downcast<adc0808_device &>(*device).set_in_callback(DEVCB_##_devcb, 1);

#define MCFG_ADC0808_IN2_CB(_devcb) \
	devcb = &downcast<adc0808_device &>(*device).set_in_callback(DEVCB_##_devcb, 2);

#define MCFG_ADC0808_IN3_CB(_devcb) \
	devcb = &downcast<adc0808_device &>(*device).set_in_callback(DEVCB_##_devcb, 3);

#define MCFG_ADC0808_IN4_CB(_devcb) \
	devcb = &downcast<adc0808_device &>(*device).set_in_callback(DEVCB_##_devcb, 4);

#define MCFG_ADC0808_IN5_CB(_devcb) \
	devcb = &downcast<adc0808_device &>(*device).set_in_callback(DEVCB_##_devcb, 5);

#define MCFG_ADC0808_IN6_CB(_devcb) \
	devcb = &downcast<adc0808_device &>(*device).set_in_callback(DEVCB_##_devcb, 6);

#define MCFG_ADC0808_IN7_CB(_devcb) \
	devcb = &downcast<adc0808_device &>(*device).set_in_callback(DEVCB_##_devcb, 7);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class adc0808_device : public device_t
{
public:
	// construction/destruction
	adc0808_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <class Object> devcb_base &set_eoc_callback(Object &&cb)
	{ return m_eoc_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_eoc_ff_callback(Object &&cb)
	{ return m_eoc_ff_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_in_callback(Object &&cb, int index)
	{ return m_in_cb[index].set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(address_w);
	DECLARE_WRITE_LINE_MEMBER(start_w);
	DECLARE_READ_LINE_MEMBER(eoc_r);

	// common hookups
	DECLARE_WRITE8_MEMBER(address_offset_start_w); // start and ale connected, address to the address bus
	DECLARE_WRITE8_MEMBER(address_data_start_w); // start and ale connected, address to the data bus

protected:
	adc0808_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// callbacks
	devcb_write_line m_eoc_cb;
	devcb_write_line m_eoc_ff_cb;
	devcb_read8 m_in_cb[8];

	enum state : int
	{
		STATE_IDLE,
		STATE_CONVERSION_START,
		STATE_CONVERSION_READY,
		STATE_CONVERSION_RUNNING
	};
	state m_state;

	emu_timer *m_cycle_timer;

	// state
	int m_start;
	int m_address;
	uint8_t m_sar;
	bool m_eoc;
};

class adc0809_device : public adc0808_device
{
public:
	adc0809_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class m58990_device : public adc0808_device
{
public:
	m58990_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(ADC0808, adc0808_device)
DECLARE_DEVICE_TYPE(ADC0809, adc0809_device)
DECLARE_DEVICE_TYPE(M58990, m58990_device)

#endif // MAME_DEVICES_MACHINE_ADC0808_H
