// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    National Semiconductor ADC0831 / ADC0832 / ADC0834 / ADC0838

    8-Bit serial I/O A/D Converters with Muliplexer Options

***************************************************************************/

#ifndef __ADC083X_H__
#define __ADC083X_H__

#include "emu.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef device_delegate<double (UINT8 input)> adc083x_input_delegate;
#define ADC083X_INPUT_CB(name)  double name(UINT8 input)

#define MCFG_ADC083X_INPUT_CB(_class, _method) \
	adc083x_device::set_input_callback(*device, adc083x_input_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

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
	adc083x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// static configuration helpers
	static void set_input_callback(device_t &device, adc083x_input_delegate input_callback) { downcast<adc083x_device &>(device).m_input_callback = input_callback; }

	DECLARE_WRITE_LINE_MEMBER( cs_write );
	DECLARE_WRITE_LINE_MEMBER( clk_write );
	DECLARE_WRITE_LINE_MEMBER( di_write );
	DECLARE_WRITE_LINE_MEMBER( se_write );
	DECLARE_READ_LINE_MEMBER( sars_read );
	DECLARE_READ_LINE_MEMBER( do_read );

protected:
	// device-level overrides
	virtual void device_start() override;

	INT32 m_mux_bits;

private:
	UINT8 conversion();

	void clear_sars();

	// internal state
	INT32 m_cs;
	INT32 m_clk;
	INT32 m_di;
	INT32 m_se;
	INT32 m_sars;
	INT32 m_do;
	INT32 m_sgl;
	INT32 m_odd;
	INT32 m_sel1;
	INT32 m_sel0;
	INT32 m_state;
	INT32 m_bit;
	INT32 m_output;

	adc083x_input_delegate m_input_callback;
};

class adc0831_device : public adc083x_device
{
public:
	adc0831_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ADC0831;


class adc0832_device : public adc083x_device
{
public:
	adc0832_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ADC0832;


class adc0834_device : public adc083x_device
{
public:
	adc0834_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ADC0834;


class adc0838_device : public adc083x_device
{
public:
	adc0838_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ADC0838;


#endif  /* __ADC083X_H__ */
