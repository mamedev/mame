// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_I2CHLE_H
#define MAME_MACHINE_I2CHLE_H

#pragma once

class i2c_hle_interface :  public device_interface
{
public:
	// construction/destruction
	i2c_hle_interface(const machine_config &mconfig, device_t &device, u16 address);
	virtual ~i2c_hle_interface();

	void set_address(u16 address) { m_address = address; }

	auto sda_callback() { return write_sda.bind(); }

	void sda_write(int state);
	void scl_write(int state);

	devcb_write_line write_sda;

protected:
	void interface_post_start() override ATTR_COLD;

	// override this to read out data
	virtual u8 read_data(u16 offset);
	virtual void write_data(u16 offset, u8 data);

	// override this to properly identify your device in logging
	virtual const char *get_tag();

	u16 m_data_offset;

private:
	u8 m_latch;
	u8 m_bit;
	u16 m_address;
	u16 m_last_address;
	int m_sda, m_scl;
	u32 m_state, m_state_next;
	bool m_just_acked;
};

#endif // MAME_MACHINE_I2CHLE_H
