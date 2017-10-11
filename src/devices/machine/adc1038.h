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

	static void set_input_callback(device_t &device, input_delegate &&callback) { downcast<adc1038_device &>(device).m_input_cb = std::move(callback); }
	static void set_gti_club_hack(device_t &device, int hack) { downcast<adc1038_device &>(device).m_gticlub_hack = hack; }

	DECLARE_READ_LINE_MEMBER( do_read );
	DECLARE_READ_LINE_MEMBER( sars_read );
	DECLARE_WRITE_LINE_MEMBER( di_write );
	DECLARE_WRITE_LINE_MEMBER( clk_write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	int m_cycle;
	int m_clk;
	int m_adr;
	int m_data_in;
	int m_data_out;
	int m_adc_data;
	int m_sars;

	int m_gticlub_hack;
	input_delegate m_input_cb;
};

DECLARE_DEVICE_TYPE(ADC1038, adc1038_device)


#define MCFG_ADC1038_INPUT_CB(_class, _method) \
	adc1038_device::set_input_callback(*device, adc1038_device::input_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ADC1038_GTIHACK(_hack) \
	adc1038_device::set_gti_club_hack(*device, _hack);

#endif // MAME_MACHINE_ADC1038_H
