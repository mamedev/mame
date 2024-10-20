// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_FP1060IO_EXP_H
#define MAME_BUS_FP1060IO_EXP_H

#pragma once

class device_fp1060io_exp_interface;
class fp1060io_exp_device;

class fp1060io_exp_slot_device : public device_t, public device_single_card_slot_interface<device_fp1060io_exp_interface>
{
	friend class device_fp1060io_exp_interface;
public:
	// construction/destruction
	template <typename T>
	fp1060io_exp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: fp1060io_exp_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	fp1060io_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~fp1060io_exp_slot_device();

	device_fp1060io_exp_interface *m_dev;

	auto inta_callback() { return m_inta_cb.bind(); }
	auto intb_callback() { return m_intb_cb.bind(); }
	auto intc_callback() { return m_intc_cb.bind(); }
	auto intd_callback() { return m_intd_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

	devcb_write_line m_inta_cb;
	devcb_write_line m_intb_cb;
	devcb_write_line m_intc_cb;
	devcb_write_line m_intd_cb;
};


class device_fp1060io_exp_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_fp1060io_exp_interface();

	virtual void io_map(address_map &map) = 0;
	virtual u8 get_id() = 0;

protected:
	device_fp1060io_exp_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;

	fp1060io_exp_slot_device *m_slot;

	void inta_w(int state);
	void intb_w(int state);
	void intc_w(int state);
	void intd_w(int state);
};

class fp1060io_exp_device : public device_t, public device_fp1060io_exp_interface
{
public:
	// construction/destruction
	fp1060io_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);


protected:
//  virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

void fp1060io_slot_devices(device_slot_interface &device);

DECLARE_DEVICE_TYPE(FP1060IO_EXP_SLOT, fp1060io_exp_slot_device)

#endif // MAME_BUS_FP1060IO_EXP_H

