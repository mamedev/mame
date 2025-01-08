// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    National Semiconductor ADC0804 8-bit µP Compatible A/D Converter

***********************************************************************
                              ____   ____
                     /CS   1 |*   \_/    | 20  Vcc (or Vref)
                     /RD   2 |           | 19  CLK R
                     /WR   3 |           | 18  DB0
                  CLK IN   4 |  ADC0801  | 17  DB1
                   /INTR   5 |  ADC0802  | 16  DB2
                  Vin(+)   6 |  ADC0803  | 15  DB3
                  Vin(-)   7 |  ADC0804  | 14  DB4
                   A GND   8 |  ADC0805  | 13  DB5
                  Vref/2   9 |           | 12  DB6
                     GND  10 |___________| 11  DB7

**********************************************************************/

#ifndef MAME_MACHINE_ADC0804_H
#define MAME_MACHINE_ADC0804_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adc0804_device

class adc0804_device : public device_t
{
	static const int s_conversion_cycles;

public:
	enum read_mode {
		RD_STROBED = 0,
		RD_BITBANGED,
		RD_GROUNDED
	};

	// device type constructors
	adc0804_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	adc0804_device(const machine_config &mconfig, const char *tag, device_t *owner, double r, double c)
		: adc0804_device(mconfig, tag, owner, 0U)
	{
		set_rc(r, c);
	}

	// callback configuration
	auto vin_callback() { return m_vin_callback.bind(); }
	auto intr_callback() { return m_intr_callback.bind(); }

	// misc. configuration
	void set_rc(double res, double cap) { assert(!configured()); m_res = res; m_cap = cap; }
	void set_rd_mode(read_mode mode) { assert(!configured()); m_rd_mode = mode; }

	// data bus interface
	u8 read();
	u8 read_and_write();
	void write(u8 data = 0);

	// control line interface
	void rd_w(int state);
	void wr_w(int state);

	// status line interface
	int intr_r() { return m_intr_active ? 0 : 1; }

protected:
	adc0804_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	// internal helpers
	void set_interrupt(bool state);
	void conversion_start();
	TIMER_CALLBACK_MEMBER(conversion_done);

	// callback objects
	devcb_read8 m_vin_callback;
	devcb_write_line m_intr_callback;

	// timing parameters
	double m_res;
	double m_cap;
	attotime m_fclk_rc;

	// conversion timer
	emu_timer *m_timer;

	// inputs
	read_mode m_rd_mode;
	bool m_rd_active;
	bool m_wr_active;

	// internal state
	u8 m_result;
	bool m_intr_active;
};

// ======================> adc0803_device

class adc0803_device : public adc0804_device
{
public:
	// device type constructors
	adc0803_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	adc0803_device(const machine_config &mconfig, const char *tag, device_t *owner, double r, double c)
		: adc0803_device(mconfig, tag, owner, 0U)
	{
		set_rc(r, c);
	}
};

// device type declarations
DECLARE_DEVICE_TYPE(ADC0803, adc0803_device)
DECLARE_DEVICE_TYPE(ADC0804, adc0804_device)

#endif // MAME_MACHINE_ADC0804_H
