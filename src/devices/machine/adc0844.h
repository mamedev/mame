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
//  TYPE DEFINITIONS
//**************************************************************************

class adc0844_device : public device_t
{
public:
	// construction/destruction
	adc0844_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	auto intr_callback() { return m_intr_cb.bind(); }
	auto ch1_callback() { return m_ch1_cb.bind(); }
	auto ch2_callback() { return m_ch2_cb.bind(); }
	auto ch3_callback() { return m_ch3_cb.bind(); }
	auto ch4_callback() { return m_ch4_cb.bind(); }

	u8 read();
	virtual void write(u8 data);

protected:
	adc0844_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	virtual TIMER_CALLBACK_MEMBER(conversion_complete);

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
	adc0848_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	auto ch5_callback() { return m_ch5_cb.bind(); }
	auto ch6_callback() { return m_ch6_cb.bind(); }
	auto ch7_callback() { return m_ch7_cb.bind(); }
	auto ch8_callback() { return m_ch8_cb.bind(); }

	virtual void write(u8 data) override;

protected:
	virtual TIMER_CALLBACK_MEMBER(conversion_complete) override;

private:
	devcb_read8 m_ch5_cb, m_ch6_cb, m_ch7_cb, m_ch8_cb;
};

// device type definition
DECLARE_DEVICE_TYPE(ADC0844, adc0844_device)
DECLARE_DEVICE_TYPE(ADC0848, adc0848_device)

#endif // MAME_DEVICES_MACHINE_ADC0844_H
