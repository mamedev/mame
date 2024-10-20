// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_FP1000_EXP_H
#define MAME_BUS_FP1000_EXP_H

#pragma once

class device_fp1000_exp_interface;
class fp1000_exp_device;

class fp1000_exp_slot_device : public device_t, public device_single_card_slot_interface<device_fp1000_exp_interface>
{
	friend class device_fp1000_exp_interface;
public:
	// construction/destruction
	template <typename T>
	fp1000_exp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: fp1000_exp_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	fp1000_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~fp1000_exp_slot_device();

	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }

	address_space &iospace() const { return *m_iospace; }

	void select_w(bool enable);

	auto inta_callback() { return m_inta_cb.bind(); }
	auto intb_callback() { return m_intb_cb.bind(); }
	auto intc_callback() { return m_intc_cb.bind(); }
	auto intd_callback() { return m_intd_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

private:
	required_address_space m_iospace;

	void remap_cb();
	void main_cs_w(offs_t offset, u8 data);

	bool m_main_enable = false;
	device_fp1000_exp_interface *m_dev;

	devcb_write_line m_inta_cb;
	devcb_write_line m_intb_cb;
	devcb_write_line m_intc_cb;
	devcb_write_line m_intd_cb;
};

class device_fp1000_exp_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_fp1000_exp_interface();

	virtual void cs_w(offs_t offset, u8 data) = 0;
	virtual u8 id_r(offs_t offset) = 0;
	virtual void remap_cb() = 0;


protected:
	device_fp1000_exp_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;

	fp1000_exp_slot_device *m_slot;

	void inta_w(int state);
	void intb_w(int state);
	void intc_w(int state);
	void intd_w(int state);
};

class fp1000_exp_device : public device_t, public device_fp1000_exp_interface
{
public:
	// construction/destruction
	fp1000_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);


protected:
//  virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(FP1000_EXP_SLOT, fp1000_exp_slot_device)

void fp1000_exp_devices(device_slot_interface &device);

#endif // MAME_MACHINE_FP1000_EXP_H
