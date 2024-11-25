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
//  TYPE DEFINITIONS
//**************************************************************************

class adc0808_device : public device_t
{
public:
	// construction/destruction
	adc0808_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto eoc_callback() { return m_eoc_cb.bind(); }
	auto eoc_ff_callback() { return m_eoc_ff_cb.bind(); }
	template <std::size_t Bit> auto in_callback() { return m_in_cb[Bit].bind(); }

	u8 data_r();
	void address_w(u8 data);
	void start_w(int state);
	int eoc_r();

	// common hookups
	void address_offset_start_w(offs_t offset, u8 data); // start and ale connected, address to the address bus
	void address_data_start_w(u8 data); // start and ale connected, address to the data bus

protected:
	adc0808_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_state);

private:
	// callbacks
	devcb_write_line m_eoc_cb;
	devcb_write_line m_eoc_ff_cb;
	devcb_read8::array<8> m_in_cb;

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
