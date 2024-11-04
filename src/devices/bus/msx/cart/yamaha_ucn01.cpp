// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "yamaha_ucn01.h"
#include "bus/msx/module/module.h"

/*
Emulation of Yamaha UCN-01.

Converts a regular MSX cartridge slot into a Yamaha module slot.

The extra pins are not connected so modules that make use of the
video out or sound out pins would not work. This limitation is
currently not emulated.

*/

namespace {

class msx_cart_ucn01_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_ucn01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_UCN01, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_module(*this, "module")
	{ }

protected:
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_config_complete() override;

private:
	required_device<msx_slot_yamaha_expansion_device> m_module;
};

void msx_cart_ucn01_device::device_add_mconfig(machine_config &mconfig)
{
	MSX_SLOT_YAMAHA_EXPANSION(mconfig, m_module, DERIVED_CLOCK(1, 1));
	m_module->option_reset();
	msx_yamaha_60pin(*m_module, false);
	m_module->set_default_option(nullptr);
	m_module->set_fixed(false);
	m_module->irq_handler().set(*this, FUNC(msx_cart_ucn01_device::irq_out));
}

void msx_cart_ucn01_device::device_config_complete()
{
	if (parent_slot())
	{
		auto target = m_module.finder_target();
		parent_slot()->configure_subslot(*target.first.subdevice<msx_slot_yamaha_expansion_device>(target.second));
	}
}

void msx_cart_ucn01_device::device_resolve_objects()
{
	m_module->install(page(0), page(1), page(2), page(3));
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_UCN01, msx_cart_interface, msx_cart_ucn01_device, "msx_cart_ucn01", "Yamaha UCN-01")
