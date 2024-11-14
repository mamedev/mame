// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    National Semiconductor ADC1038

    10-Bit Serial I/O A/D Converters with Analog Multiplexer and
    Track/hold Function

***************************************************************************/
#ifndef MAME_MACHINE_ADC1038_H
#define MAME_MACHINE_ADC1038_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#define ADC1038_INPUT_CB(name)  int name(int input)

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class adc1038_device : public device_t
{
public:
	typedef device_delegate<int (int input)> input_delegate;

	adc1038_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T> void set_input_callback(T &&... args) { m_input_cb.set(std::forward<T>(args)...); }

	void set_gti_club_hack(bool hack) { m_gticlub_hack = hack; }

	int do_read();
	int sars_read();
	void di_write(int state);
	void clk_write(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	int m_cycle;
	int m_clk;
	int m_adr;
	int m_data_in;
	int m_data_out;
	int m_adc_data;
	int m_sars;

	bool m_gticlub_hack;
	input_delegate m_input_cb;
};

DECLARE_DEVICE_TYPE(ADC1038, adc1038_device)

#endif // MAME_MACHINE_ADC1038_H
