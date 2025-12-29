// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_MZ80_MZ80_EXP_H
#define MAME_BUS_MZ80_MZ80_EXP_H

#pragma once

class device_mz80_exp_interface;
class mz80_exp_device;

class mz80_exp_slot_device : public device_t, public device_single_card_slot_interface<device_mz80_exp_interface>
{
	friend class device_mz80_exp_interface;
public:
	// construction/destruction
	template <typename T>
	mz80_exp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: mz80_exp_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	mz80_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~mz80_exp_slot_device();

	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }

	template<typename T> void install_io_device(T &device, void (T::*map)(class address_map &map))
	{
		m_iospace->install_device(0x0000, 0xffff, device, map);
	}

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	required_address_space m_iospace;

//  device_mz80_exp_interface *m_exp;
};

class device_mz80_exp_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_mz80_exp_interface();

	virtual void io_map(address_map &map) { }

protected:
	device_mz80_exp_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;

	mz80_exp_slot_device *m_slot;
};

class mz80_exp_device : public device_t, public device_mz80_exp_interface
{
public:
	// construction/destruction
	mz80_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);


protected:
//  virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;


};

// device type definition
DECLARE_DEVICE_TYPE(MZ80_EXP_SLOT, mz80_exp_slot_device)

void mz800_exp_devices(device_slot_interface &device);
void mz2500_exp_devices(device_slot_interface &device);

#endif // MAME_BUS_MZ80_MZ80_EXP_H
