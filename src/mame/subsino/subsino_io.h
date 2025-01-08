// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_SUBSINO_SUBSINO_IO_H
#define MAME_SUBSINO_SUBSINO_IO_H

#pragma once

class subsino_io_device : public device_t
{
public:
	// callback configuration
	template <int P> auto in_port_callback() { return m_in_port_callback[P].bind(); }
	template <int P> auto out_port_callback() { return m_out_port_callback[P].bind(); }

protected:
	subsino_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// port read/write helpers
	u8 read_port_data(unsigned port);
	void write_port_data(unsigned port, u8 data);
	void write_port_dir(unsigned port, u8 dir);
	void write_port_data_and_dir(unsigned port, u8 data, u8 dir);

	// callback objects
	devcb_read8::array<10> m_in_port_callback;
	devcb_write8::array<10> m_out_port_callback;

	// internal state
	u8 m_port_data[10];
	u8 m_port_dir[10];
};

class ss9602_device : public subsino_io_device
{
public:
	// device type constructor
	ss9602_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// CPU read/write handlers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
};

class ss9802_device : public subsino_io_device
{
public:
	// device type constructor
	ss9802_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// CPU read/write handlers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
};

// device type declarations
DECLARE_DEVICE_TYPE(SS9602, ss9602_device)
DECLARE_DEVICE_TYPE(SS9802, ss9802_device)

#endif // MAME_SUBSINO_SUBSINO_IO_H
