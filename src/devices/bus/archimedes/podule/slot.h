// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Archimedes Expansion Bus emulation

**********************************************************************/

#ifndef MAME_BUS_ARCHIMEDES_PODULE_SLOT_H
#define MAME_BUS_ARCHIMEDES_PODULE_SLOT_H

#pragma once


//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

class archimedes_exp_device;
class device_archimedes_podule_interface;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> archimedes_podule_slot_device

class archimedes_podule_slot_device : public device_t,
	public device_single_card_slot_interface<device_archimedes_podule_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	archimedes_podule_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&bus_tag, U &&opts, const char *dflt)
		: archimedes_podule_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1, 1))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_exp.set_tag(std::forward<T>(bus_tag));
	}
	archimedes_podule_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	required_device<archimedes_exp_device> m_exp;
};

// device type definition
DECLARE_DEVICE_TYPE(ARCHIMEDES_PODULE_SLOT, archimedes_podule_slot_device)



// ======================> archimedes_exp_device

class archimedes_exp_device : public device_t,
	public device_memory_interface
{
public:
	// construction/destruction
	archimedes_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// inline configuration
	auto out_irq_callback() { return m_out_pirq_cb.bind(); }
	auto out_fiq_callback() { return m_out_pfiq_cb.bind(); }

	u16 ps4_r(offs_t offset, u16 mem_mask = ~0);
	void ps4_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ps6_r(offs_t offset, u16 mem_mask = ~0);
	void ps6_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ms_r(offs_t offset, u16 mem_mask = ~0);
	void ms_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	template<typename T> void install_ioc_map(int slot, T &device, void (T::*map)(class address_map &map))
	{
		offs_t base = slot << 14;
		m_ioc->install_device(base, base + 0x3fff, device, map);
	}
	template<typename T> void install_memc_map(int slot, T &device, void (T::*map)(class address_map &map))
	{
		offs_t base = slot << 14;
		m_memc->install_device(base, base + 0x3fff, device, map);
	}

	void pirq_w(int state, int slot);
	void pfiq_w(int state) { m_out_pfiq_cb(state); }

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_ioc_config;
	address_space_config m_memc_config;

	devcb_write_line m_out_pirq_cb;
	devcb_write_line m_out_pfiq_cb;

	u8 m_pirq_state;
	u8 m_pirq_mask;

	address_space *m_ioc;
	address_space *m_memc;

	void ioc_map(address_map &map);
	void memc_map(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(ARCHIMEDES_EXPANSION_BUS, archimedes_exp_device)


// ======================> device_archimedes_podule_interface

class device_archimedes_podule_interface : public device_interface
{
	friend class archimedes_exp_device;

public:
	// construction/destruction
	virtual ~device_archimedes_podule_interface();

	// inline configuration
	void set_archimedes_exp(archimedes_exp_device *exp, const char *slottag) { m_exp = exp; m_exp_slottag = slottag; }

protected:
	void set_pirq(int state) { m_exp->pirq_w(state, m_slot); }
	void set_pfiq(int state) { m_exp->pfiq_w(state); }

	device_archimedes_podule_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;

	virtual void ioc_map(address_map &map) { }
	virtual void memc_map(address_map &map) { }

	archimedes_exp_device *m_exp;
	const char *m_exp_slottag;
	int m_slot;
};


void archimedes_exp_devices(device_slot_interface &device);
void archimedes_mini_exp_devices(device_slot_interface &device);


#endif // MAME_BUS_ARCHIMEDES_PODULE_SLOT_H
