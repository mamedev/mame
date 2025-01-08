// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#include "emu.h"
#include "smuc.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/atapicdr.h"
#include "bus/ata/hdd.h"
#include "machine/ds17x85.h"
#include "machine/i2cmem.h"

namespace bus::spectrum::zxbus {

namespace {

class smuc_device : public device_t, public device_zxbus_card_interface
{
public:
	smuc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, ZXBUS_SMUC, tag, owner, clock)
		, device_zxbus_card_interface(mconfig, *this)
		, m_rtc(*this, "rtc")
		, m_eeprom(*this, "eeprom")
		, m_ata(*this, "ata")
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void map_io(address_map &map) ATTR_COLD;

	void port_ffba_w(offs_t offset, u8 data);
	u8 ata_r(offs_t offset);
	void ata_w(offs_t offset, u8 data);

	required_device<ds17x85_device> m_rtc;   //cmos
	required_device<i2cmem_device> m_eeprom; //nvram
	required_device<ata_interface_device> m_ata;

	u8 m_port_ffba_data;
	u8 m_port_7fba_data;
	u8 m_ide_hi;
};

void smuc_device::port_ffba_w(offs_t offset, u8 data)
{
	if (!BIT(data, 0))
		m_ata->reset();

	m_eeprom->write_sda(BIT(data, 4));
	m_eeprom->write_wc(BIT(data, 5));
	m_eeprom->write_scl(BIT(data, 6));

	m_port_ffba_data = data;
}

u8 smuc_device::ata_r(offs_t offset)
{
	const u8 ata_offset = BIT(offset, 8, 3);
	const u16 data = BIT(m_port_ffba_data, 7)
		? m_ata->cs1_r(ata_offset)
		: m_ata->cs0_r(ata_offset);

	if (!machine().side_effects_disabled())
		m_ide_hi = data >> 8;

	return data & 0xff;
}

void smuc_device::ata_w(offs_t offset, u8 data)
{
	const u8 ata_offset = BIT(offset, 8, 3);
	const u16 ata_data = (m_ide_hi << 8) | data;
	if (BIT(m_port_ffba_data, 7))
		m_ata->cs1_w(ata_offset, ata_data);
	else
		m_ata->cs0_w(ata_offset, ata_data);
}


void smuc_device::map_io(address_map &map)
{
	map(0x118a2, 0x118a2).mirror(0x4718) //     5fba | 0x011xxx101xx010 | Version: 2
		.lr8(NAME([]() { return 0x40; }));
	map(0x118a6, 0x118a6).mirror(0x4718) //     5fbe | 0x011xxx101xx110 | Revision: 0
		.lr8(NAME([]() { return 0x28; }));
	map(0x138a2, 0x138a2).mirror(0x4718) //     7fba | 0x111xxx101xx010 | VirtualFDD
		.lrw8(NAME([this]() { return m_port_7fba_data | 0x37; })
			, NAME([this](u8 data) { m_port_7fba_data = data; }));
	map(0x138a6, 0x138a6).mirror(0x4718) //  7[ef]be | 0x111xxN101xx110 | i8259 - absent in 2.0
		.lr8(NAME([]() { return 0x57; })).nopw();
	map(0x198a2, 0x198a2).mirror(0x4718) //     dfba | 1x011xxx101xx010 | DS1685RTC
		.lrw8(NAME([this]() { return m_rtc->data_r(); })
			, NAME([this](u8 data) { if (BIT(m_port_ffba_data, 7)) m_rtc->data_w(data); else m_rtc->address_w(data); }));
	map(0x198a6, 0x198a6).mirror(0x4718) //     d8be | 1x011xxx101xx110 | IDE-Hi
		.lrw8(NAME([this]() { return m_ide_hi; }), NAME([this](u8 data) { m_ide_hi = data; }));
	map(0x1b8a2, 0x1b8a2).mirror(0x4718) //     ffba | 1x111xxx101xx010 | SYS
		.lr8(NAME([this]() { return m_eeprom->read_sda() ? 0xff : 0xbf; })).w(FUNC(smuc_device::port_ffba_w));
	map(0x1b8a6, 0x1b8a6).select(0x4718) // f[8-f]be | 1x111NNN101xx110  | IDE#1Fx/#3F6
		.rw(FUNC(smuc_device::ata_r), FUNC(smuc_device::ata_w));
}

static void smuc_ata_devices(device_slot_interface &device)
{
	device.option_add("hdd", IDE_HARDDISK);
	device.option_add("cdrom", ATAPI_CDROM);
}

void smuc_device::device_add_mconfig(machine_config &config)
{
	DS1685(config, m_rtc, XTAL(32'768));
	I2C_24C16(config, m_eeprom);
	ATA_INTERFACE(config, m_ata).options(smuc_ata_devices, "hdd", "hdd", false);
}

void smuc_device::device_start()
{
	m_port_7fba_data = 0;

	save_item(NAME(m_port_7fba_data));
	save_item(NAME(m_port_ffba_data));
	save_item(NAME(m_ide_hi));

	m_zxbus->install_device(0x00000, 0x1ffff, *this, &smuc_device::map_io);
}

void smuc_device::device_reset()
{
	m_port_ffba_data = 0;
}

} // anonymous namespace

} // namespace bus::spectrum::zxbus

DEFINE_DEVICE_TYPE_PRIVATE(ZXBUS_SMUC, device_zxbus_card_interface, bus::spectrum::zxbus::smuc_device, "zxbus_smuc", "SMUC")
