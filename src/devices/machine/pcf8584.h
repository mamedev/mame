// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Philips PCF8584 I²C Bus Controller

***********************************************************************
                             ____   ____
                    CLK   1 |*   \_/    | 20  Vdd
        (SDA OUT)   SDA   2 |           | 19  _RESET/_STROBE
        (SCL IN)    SCL   3 |           | 18  _WR  (R/_W)
        (SDA IN)  _IACK   4 |           | 17  _CS
        (SCL OUT)  _INT   5 |           | 16  _RD  (_DTACK)
                     A0   6 |  PCF8584  | 15  DB7
                    DB0   7 |           | 14  DB6
                    DB1   8 |           | 13  DB5
                    DB2   9 |           | 12  DB4
                    Vss  10 |___________| 11  DB3

**********************************************************************/

#ifndef MAME_MACHINE_PCF8584_H
#define MAME_MACHINE_PCF8584_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pcf8584_device

class pcf8584_device : public device_t
{
	static const u32 s_prescaler[4];
	static const u32 s_divider[8];
	static const char *const s_nominal_clock[8];
	static const char *const s_bus_function[4];

public:
	// device type constructor
	pcf8584_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto sda_cb() { return m_sda_callback.bind(); }
	auto sda_out_cb() { return m_sda_out_callback.bind(); }
	auto scl_cb() { return m_scl_callback.bind(); }
	auto int_cb() { return m_int_callback.bind(); }

	// bus type configuration
	void set_80xx_bus() { m_68k_bus = false; }
	void set_68000_bus() { m_68k_bus = true; }

	// CPU interface
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	IRQ_CALLBACK_MEMBER(iack);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal helpers
	bool eso() const { return BIT(m_s1_control, 6); }
	bool es1() const { return BIT(m_s1_control, 5); }
	bool es2() const { return BIT(m_s1_control, 4); }
	bool eni() const { return BIT(m_s1_control, 3); }

	// write helpers
	void set_shift_register(u8 data);
	void set_own_address(u8 data);
	void set_control(u8 data);
	void set_clock_frequency(u8 data);
	void set_vector(u8 data);

	// callback objects
	devcb_read_line m_sda_callback;
	devcb_write_line m_sda_out_callback;
	devcb_write_line m_scl_callback;
	devcb_write_line m_int_callback;

	// misc. parameters
	bool m_68k_bus;

	// internal registers
	u8 m_s0_data_buffer;
	u8 m_s0_shift_register;
	u8 m_s0_own_address;
	u8 m_s1_status;
	u8 m_s1_control;
	u8 m_s2_clock;
	u8 m_s3_vector;
};

// device type declaration
DECLARE_DEVICE_TYPE(PCF8584, pcf8584_device)

#endif // MAME_MACHINE_PCF8584_H
