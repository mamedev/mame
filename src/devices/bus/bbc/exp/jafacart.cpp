// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    JAFA Systems Master Compact Cartridge Adaptor

**********************************************************************/

#include "emu.h"
#include "jafacart.h"

#include "bus/bbc/cart/slot.h"


namespace {

class bbc_jafacart_device : public device_t, public device_bbc_exp_interface
{
public:
	bbc_jafacart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_JAFACART, tag, owner, clock)
		, device_bbc_exp_interface(mconfig, *this)
		, m_cart(*this, "cartslot")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	virtual void device_add_mconfig(machine_config &config) override
	{
		BBCM_CARTSLOT(config, m_cart, clock()/2, bbcm_cart, nullptr);
		m_cart->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_exp_slot_device::irq_w));
		m_cart->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_exp_slot_device::nmi_w));
	}

	virtual uint8_t fred_r(offs_t offset) override
	{
		return m_cart->read(offset, 1, 0, 0, 0, 0);
	}

	virtual void fred_w(offs_t offset, uint8_t data) override
	{
		m_cart->write(offset, data, 1, 0, 0, 0, 0);
	}

	virtual uint8_t jim_r(offs_t offset) override
	{
		return m_cart->read(offset, 0, 1, 0, 0, 0);
	}

	virtual void jim_w(offs_t offset, uint8_t data) override
	{
		m_cart->write(offset, data, 0, 1, 0, 0, 0);
	}

	virtual uint8_t rom_r(offs_t offset) override
	{
		return m_cart->read(offset & 0x3fff, 0, 0, BIT(offset, 14), 1, 0);
	}

	virtual void rom_w(offs_t offset, uint8_t data) override
	{
		m_cart->write(offset & 0x3fff, data, 0, 0, BIT(offset, 14), 1, 0);
	}

private:
	required_device<bbc_cartslot_device> m_cart;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_JAFACART, device_bbc_exp_interface, bbc_jafacart_device, "bbc_jafacart", "JAFA Systems Master Compact Cartridge Adaptor");
