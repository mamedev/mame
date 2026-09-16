// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    National Semiconductor ADC0831 / ADC0832 / ADC0834 / ADC0838

    8-Bit serial I/O A/D Converters with Muliplexer Options

***************************************************************************/

#ifndef MAME_MACHINE_ADC083X_H
#define MAME_MACHINE_ADC083X_H

#pragma once


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ADC083X_CH0     0
#define ADC083X_CH1     1
#define ADC083X_CH2     2
#define ADC083X_CH3     3
#define ADC083X_CH4     4
#define ADC083X_CH5     5
#define ADC083X_CH6     6
#define ADC083X_CH7     7
#define ADC083X_COM     8
#define ADC083X_AGND    9
#define ADC083X_VREF    10

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class adc083x_device : public device_t
{
public:
	typedef device_delegate<double (uint8_t input)> input_delegate;

	// configuration helpers
	template <typename... T> void set_input_callback(T &&... args) { m_input_callback.set(std::forward<T>(args)...); }

	void cs_write(int state);
	void clk_write(int state);
	void di_write(int state);
	void se_write(int state);
	int sars_read();
	int do_read();

protected:
	adc083x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t mux_bits);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	const int32_t m_mux_bits;

private:
	uint8_t conversion();

	void clear_sars();

	// internal state
	int32_t m_cs;
	int32_t m_clk;
	int32_t m_di;
	int32_t m_se;
	int32_t m_sars;
	int32_t m_do;
	int32_t m_sgl;
	int32_t m_odd;
	int32_t m_sel1;
	int32_t m_sel0;
	int32_t m_state;
	int32_t m_bit;
	int32_t m_output;

	input_delegate m_input_callback;
};


class adc0831_device : public adc083x_device
{
public:
	adc0831_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


class adc0832_device : public adc083x_device
{
public:
	adc0832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


class adc0834_device : public adc083x_device
{
public:
	adc0834_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


class adc0838_device : public adc083x_device
{
public:
	adc0838_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


DECLARE_DEVICE_TYPE(ADC0831, adc0831_device)
DECLARE_DEVICE_TYPE(ADC0832, adc0832_device)
DECLARE_DEVICE_TYPE(ADC0834, adc0834_device)
DECLARE_DEVICE_TYPE(ADC0838, adc0838_device)

#endif // MAME_MACHINE_ADC083X_H
