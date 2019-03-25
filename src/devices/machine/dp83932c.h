// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_DP83932C_H
#define MAME_MACHINE_DP83932C_H

#pragma once

class dp83932c_device :
	public device_t,
	public device_memory_interface,
	public device_network_interface
{
public:
	// callback configuration
	auto out_int_cb() { return m_out_int.bind(); }

	void map(address_map &map);

protected:
	dp83932c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endian);

	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_network_interface overrides
	virtual void send_complete_cb(int result) override;
	virtual int recv_start_cb(u8 *buf, int length) override;
	virtual void recv_complete_cb(int result) override;

private:
	// device_memory_interface members
	const address_space_config m_space_config;
	address_space *m_space;

	devcb_write_line m_out_int;
};

class dp83932c_be_device : public dp83932c_device
{
public:
	dp83932c_be_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class dp83932c_le_device : public dp83932c_device
{
public:
	dp83932c_le_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(DP83932C_BE, dp83932c_be_device)
DECLARE_DEVICE_TYPE(DP83932C_LE, dp83932c_le_device)

#endif // MAME_MACHINE_DP83932C_H
