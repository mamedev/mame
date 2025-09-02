// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Master Extra User Port

**********************************************************************/

#include "emu.h"
#include "meup.h"

#include "bus/bbc/userport/userport.h"
#include "machine/6522via.h"


namespace {

class bbc_meup_device: public device_t, public device_bbc_modem_interface
{
public:
	bbc_meup_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock)
		: device_t(mconfig, BBC_MEUP, tag, owner, clock)
		, device_bbc_modem_interface(mconfig, *this)
		, m_via(*this, "via")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override
	{
		return m_via->read(offset);
	}

	virtual void write(offs_t offset, uint8_t data) override
	{
		m_via->write(offset, data);
	}

private:
	required_device<via6522_device> m_via;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_meup_device::device_add_mconfig(machine_config &config)
{
	MOS6522(config, m_via, DERIVED_CLOCK(1, 1));
	m_via->readpb_handler().set("userport2", FUNC(bbc_userport_slot_device::pb_r));
	m_via->writepb_handler().set("userport2", FUNC(bbc_userport_slot_device::pb_w));
	m_via->cb1_handler().set("userport2", FUNC(bbc_userport_slot_device::write_cb1));
	m_via->cb2_handler().set("userport2", FUNC(bbc_userport_slot_device::write_cb2));
	m_via->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_modem_slot_device::irq_w));

	bbc_userport_slot_device &userport2(BBC_USERPORT_SLOT(config, "userport2", bbc_userport_devices, nullptr));
	userport2.cb1_handler().set(m_via, FUNC(via6522_device::write_cb1));
	userport2.cb2_handler().set(m_via, FUNC(via6522_device::write_cb2));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_MEUP, device_bbc_modem_interface, bbc_meup_device, "bbc_meup", "Master Extra User Port");
