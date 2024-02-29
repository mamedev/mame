// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_SEGAAI_EXP_H
#define MAME_BUS_SEGAAI_EXP_H

#pragma once


DECLARE_DEVICE_TYPE(SEGAAI_EXP_SLOT, segaai_exp_slot_device);


class segaai_exp_interface;

class segaai_exp_slot_device : public device_t,
								public device_single_card_slot_interface<segaai_exp_interface>
{
public:
	template <typename T>
	segaai_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&opts, const char *dflt)
		: segaai_exp_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	segaai_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~segaai_exp_slot_device();

	template <typename T> void set_mem_space(T &&tag, int no) { m_mem_space.set_tag(std::forward<T>(tag), no); }
	template <typename T> void set_io_space(T &&tag, int no) { m_io_space.set_tag(std::forward<T>(tag), no); }

	address_space& mem_space() { return *m_mem_space; }
	address_space& io_space() { return *m_io_space; }

protected:
	virtual void device_start() override { }

private:
	optional_address_space m_mem_space;
	optional_address_space m_io_space;
};


class segaai_exp_interface : public device_interface
{
public:
	segaai_exp_interface(const machine_config &mconfig, device_t &device);
	virtual ~segaai_exp_interface();

protected:
	address_space& mem_space() { return m_slot->mem_space(); }
	address_space& io_space() { return m_slot->io_space(); }

private:
	segaai_exp_slot_device *const m_slot;
};


// slot interfaces
void segaai_exp(device_slot_interface &device);

#endif
