// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC8801_EXP_H
#define MAME_BUS_PC8801_EXP_H

#pragma once

class device_pc8801_exp_interface;
class pc8801_exp_device;

class pc8801_exp_slot_device : public device_t, public device_single_card_slot_interface<device_pc8801_exp_interface>
{
	friend class device_pc8801_exp_interface;
public:
	// construction/destruction
	template <typename T>
	pc8801_exp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: pc8801_exp_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	pc8801_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~pc8801_exp_slot_device();

	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }

	template<typename T> void install_io_device(T &device, void (T::*map)(class address_map &map))
	{
		m_iospace->install_device(0x00, 0xff, device, map);
	}

	auto int3_callback() { return m_int3_cb.bind(); }
	auto int4_callback() { return m_int4_cb.bind(); }
	auto int5_callback() { return m_int5_cb.bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	required_address_space m_iospace;

//  device_pc8801_exp_interface *m_exp;

	devcb_write_line m_int3_cb;
	devcb_write_line m_int4_cb;
	devcb_write_line m_int5_cb;
};

class device_pc8801_exp_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_pc8801_exp_interface();

	virtual void io_map(address_map &map) { }

protected:
	device_pc8801_exp_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;

	pc8801_exp_slot_device *m_slot;

	void int3_w(int state);
	virtual void int4_w(int state);
	void int5_w(int state);
};

class pc8801_exp_device : public device_t, public device_pc8801_exp_interface
{
public:
	// construction/destruction
	pc8801_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);


protected:
//  virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;


};

// device type definition
DECLARE_DEVICE_TYPE(PC8801_EXP_SLOT, pc8801_exp_slot_device)

void pc8801_exp_devices(device_slot_interface &device);

#endif // MAME_MACHINE_PC8801_EXP_H
