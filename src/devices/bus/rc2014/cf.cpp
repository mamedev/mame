// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Compact Flash Module

****************************************************************************/

#include "emu.h"
#include "cf.h"
#include "bus/ata/ataintf.h"

namespace {

//**************************************************************************
//  RC2014 Compact Flash module
//  Module author: Spencer Owen
//**************************************************************************

class compact_flash_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	compact_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void tx_w(int state) { m_bus->tx_w(state); }

	uint8_t ide_cs0_r(offs_t offset) { return m_ata->cs0_r(offset); }
	void ide_cs0_w(offs_t offset, uint8_t data) { m_ata->cs0_w(offset, data); }
private:
	required_device<ata_interface_device> m_ata;
};

compact_flash_device::compact_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_COMPACT_FLASH, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_ata(*this, "ata")
{
}

void compact_flash_device::device_start()
{
}

void compact_flash_device::device_reset()
{
	// A15-A8 and A7 not connected
	m_bus->installer(AS_IO)->install_readwrite_handler(0x10, 0x17, 0, 0xff80, 0, read8sm_delegate(*this, FUNC(compact_flash_device::ide_cs0_r)), write8sm_delegate(*this, FUNC(compact_flash_device::ide_cs0_w)));
}

void compact_flash_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, "cf", nullptr, true);
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_COMPACT_FLASH, device_rc2014_card_interface, compact_flash_device, "rc2014_cf", "RC2014 Compact Flash module")
