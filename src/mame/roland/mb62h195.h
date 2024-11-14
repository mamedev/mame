// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Roland MB62H195 gate array

***************************************************************************/

#ifndef MAME_ROLAND_MB62H195_H
#define MAME_ROLAND_MB62H195_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb62h195_device

class mb62h195_device : public device_t
{
public:
	// device type constructor
	mb62h195_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration
	auto lc_callback() { return m_lc_callback.bind(); }
	auto r_callback() { return m_r_callback.bind(); }
	auto t_callback() { return m_t_callback.bind(); }
	auto da_callback() { return m_da_callback.bind(); }
	auto dc_callback() { return m_dc_callback.bind(); }
	auto sout_callback() { return m_sout_callback.bind(); }
	auto sck_callback() { return m_sck_callback.bind(); }
	auto sin_callback() { return m_sin_callback.bind(); }
	auto adc_callback() { return m_adc_callback.bind(); }

	// CPU read/write handlers (TODO)

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	// callback objects
	devcb_write8 m_lc_callback;
	devcb_read8 m_r_callback;
	devcb_write8 m_t_callback;
	devcb_write16 m_da_callback;
	devcb_write8 m_dc_callback;
	devcb_write_line m_sout_callback;
	devcb_write_line m_sck_callback;
	devcb_read_line m_sin_callback;
	devcb_write_line m_adc_callback;
};


// device type declaration
DECLARE_DEVICE_TYPE(MB62H195, mb62h195_device)

#endif // MAME_ROLAND_MB62H195_H
