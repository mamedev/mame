// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    GI ER1400 1400 Bit Serial Electrically Alterable Read-Only Memory

****************************************************************************
                             ____   ____
                (0) Vss   1 |*   \_/    | 14  Vm (N.C.)
              (-35) Vgg   2 |           | 13  N.C.
                   N.C.   3 |           | 12  Data I/O
                   N.C.   4 |  ER 1400  | 11  N.C.
                   N.C.   5 |           | 10  N.C.
                  Clock   6 |           |  9  C3
                     C1   7 |___________|  8  C2

***************************************************************************/

#ifndef MAME_MACHINE_ER1400_H
#define MAME_MACHINE_ER1400_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> er1400_device

class er1400_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	er1400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// line handlers
	void data_w(int state);
	void c1_w(int state);
	void c2_w(int state);
	void c3_w(int state);
	void clock_w(int state);
	int data_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	TIMER_CALLBACK_MEMBER(propagate_data);

private:
	enum
	{
		PROPAGATION_TIMER
	};

	// logging helper
	std::string address_binary_format() const;

	// internal helpers
	void read_data();
	void write_data();
	void erase_data();

	// optional default data
	optional_region_ptr<u16> m_default_data;

	// nonvolatile data
	std::unique_ptr<u16[]> m_data_array;

	// input and output line states
	bool m_clock_input;
	u8 m_code_input;
	bool m_data_input;
	bool m_data_output;

	// timing
	attotime m_write_time;
	attotime m_erase_time;
	emu_timer *m_data_propagation_timer;

	// internal registers
	u16 m_data_register;
	u32 m_address_register;
};


// device type definition
DECLARE_DEVICE_TYPE(ER1400, er1400_device)

#endif // MAME_DEVICES_MACHINE_ER1400_H
