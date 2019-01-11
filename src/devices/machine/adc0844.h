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
	devcb = &downcast<adc0844_device &>(*device).set_intr_callback(DEVCB_##_devcb);

#define MCFG_ADC0844_CH1_CB(_devcb) \
	devcb = &downcast<adc0844_device &>(*device).set_ch1_callback(DEVCB_##_devcb);

#define MCFG_ADC0844_CH2_CB(_devcb) \
	devcb = &downcast<adc0844_device &>(*device).set_ch2_callback(DEVCB_##_devcb);

#define MCFG_ADC0844_CH3_CB(_devcb) \
	devcb = &downcast<adc0844_device &>(*device).set_ch3_callback(DEVCB_##_devcb);

#define MCFG_ADC0844_CH4_CB(_devcb) \
	devcb = &downcast<adc0844_device &>(*device).set_ch4_callback(DEVCB_##_devcb);

#define MCFG_ADC0848_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ADC0848, 0)

#define MCFG_ADC0848_INTR_CB MCFG_ADC0844_INTR_CB
#define MCFG_ADC0848_CH1_CB  MCFG_ADC0844_CH1_CB
#define MCFG_ADC0848_CH2_CB  MCFG_ADC0844_CH2_CB
#define MCFG_ADC0848_CH3_CB  MCFG_ADC0844_CH3_CB
#define MCFG_ADC0848_CH4_CB  MCFG_ADC0844_CH4_CB

#define MCFG_ADC0848_CH5_CB(_devcb) \
	devcb = &downcast<adc0848_device &>(*device).set_ch5_callback(DEVCB_##_devcb);

#define MCFG_ADC0848_CH6_CB(_devcb) \
	devcb = &downcast<adc0848_device &>(*device).set_ch6_callback(DEVCB_##_devcb);

#define MCFG_ADC0848_CH7_CB(_devcb) \
	devcb = &downcast<adc0848_device &>(*device).set_ch7_callback(DEVCB_##_devcb);

#define MCFG_ADC0848_CH8_CB(_devcb) \
	devcb = &downcast<adc0848_device &>(*device).set_ch8_callback(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class adc0844_device : public device_t
{
public:
	// construction/destruction
	adc0844_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <class Object> devcb_base &set_intr_callback(Object &&cb) { return m_intr_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ch1_callback(Object &&cb) { return m_ch1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ch2_callback(Object &&cb) { return m_ch2_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ch3_callback(Object &&cb) { return m_ch3_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ch4_callback(Object &&cb) { return m_ch4_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

protected:
	adc0844_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	uint8_t clamp(int value);

	// callbacks
	devcb_write_line m_intr_cb;
	devcb_read8 m_ch1_cb, m_ch2_cb, m_ch3_cb, m_ch4_cb;

	emu_timer *m_conversion_timer;

	// state
	int m_channel;
	uint8_t m_result;
};

class adc0848_device : public adc0844_device
{
public:
	// construction/destruction
	adc0848_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <class Object> devcb_base &set_ch5_callback(Object &&cb) { return m_ch5_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ch6_callback(Object &&cb) { return m_ch6_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ch7_callback(Object &&cb) { return m_ch7_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ch8_callback(Object &&cb) { return m_ch8_cb.set_callback(std::forward<Object>(cb)); }

	virtual DECLARE_WRITE8_MEMBER(write) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	devcb_read8 m_ch5_cb, m_ch6_cb, m_ch7_cb, m_ch8_cb;
};

// device type definition
DECLARE_DEVICE_TYPE(ADC0844, adc0844_device)
DECLARE_DEVICE_TYPE(ADC0848, adc0848_device)

#endif // MAME_DEVICES_MACHINE_ADC0844_H
