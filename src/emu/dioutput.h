// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    dioutput.h

    Device Output interfaces.

***************************************************************************/

#ifndef MAME_EMU_DIOUTPUT_H
#define MAME_EMU_DIOUTPUT_H

#pragma once


//**************************************************************************
//  MACROS
//**************************************************************************

#define MCFG_OUTPUT_INDEX(_index) \
	dynamic_cast<device_output_interface &>(*device).set_output_index(_index);

#define MCFG_OUTPUT_NAME(_name) \
	dynamic_cast<device_output_interface &>(*device).set_output_name(_name);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_output_interface

class device_output_interface : public device_interface
{
public:
	// construction/destruction
	device_output_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_output_interface();

	void set_output_index(int index) { m_output_index = index; }
	void set_output_name(const char *name) { m_output_name = name; }

	void set_output_value(int value) const;
	void set_led_value(int value) const;
	void set_lamp_value(int value) const;
	void set_digit_value(int value) const;

protected:
	int m_output_index;
	const char *m_output_name;
};


#endif // MAME_EMU_DIOUTPUT_H
