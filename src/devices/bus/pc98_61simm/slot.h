// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_61SIMM_SLOT_H
#define MAME_BUS_PC98_61SIMM_SLOT_H

#pragma once


class device_pc9801_61_interface;

class pc9801_61_simm_device : public device_t,
								public device_memory_interface,
								public device_single_card_slot_interface<device_pc9801_61_interface>
{
	friend class device_pc9801_61_interface;
public:
	// construction/destruction
	template <typename T>
	pc9801_61_simm_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: pc9801_61_simm_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	pc9801_61_simm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~pc9801_61_simm_device();

	u32 read(offs_t offset, u32 mem_mask = ~0);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 read_ext(offs_t offset, u32 mem_mask = ~0);
	void write_ext(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 read_15m_ext(offs_t offset, u32 mem_mask = ~0);
	void write_15m_ext(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 read_16m_ext(offs_t offset, u32 mem_mask = ~0);
	void write_16m_ext(offs_t offset, u32 data, u32 mem_mask = ~0);

	void install_ram(offs_t addrstart, offs_t addrend, void *baseptr);
protected:
	virtual void device_start() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	device_pc9801_61_interface *m_bank;

	address_space_config m_space_mem_config;

	address_space *m_space_mem;
};

class device_pc9801_61_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_pc9801_61_interface();

protected:
	device_pc9801_61_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;

	pc9801_61_simm_device *m_slot;

	std::vector<u32> m_ram;
};

DECLARE_DEVICE_TYPE(PC9801_61_SIMM,  pc9801_61_simm_device)

#endif // MAME_BUS_PC98_61SIMM_SLOT_H
