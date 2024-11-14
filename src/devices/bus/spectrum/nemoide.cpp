// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#include "emu.h"
#include "nemoide.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/atapicdr.h"
#include "bus/ata/hdd.h"

namespace bus::spectrum::zxbus {

namespace {

class nemoide_device : public device_t, public device_zxbus_card_interface
{
public:
	nemoide_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, ZXBUS_NEMOIDE, tag, owner, clock)
		, device_zxbus_card_interface(mconfig, *this)
		, m_ata(*this, "ata")
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void map_io(address_map &map) ATTR_COLD;

	u8 ata_r(offs_t offset);
	void ata_w(offs_t offset, u8 data);

	required_device<ata_interface_device> m_ata;

	u8 m_ata_data_latch;
};

u8 nemoide_device::ata_r(offs_t offset)
{
	const u16 data = m_ata->cs0_r((offset >> 5) & 7);
	if (!machine().side_effects_disabled())
		m_ata_data_latch = data >> 8;

	return data & 0xff;
}

void nemoide_device::ata_w(offs_t offset, u8 data)
{
	const u16 ata_data = (m_ata_data_latch << 8) | data;
	m_ata->cs0_w((offset >> 5) & 7, ata_data);
}

void nemoide_device::map_io(address_map &map)
{
	map(0x00011, 0x00011).mirror(0xff00).lrw8(NAME([this]() { return m_ata_data_latch; })
		, NAME([this](offs_t offset, u8 data) { m_ata_data_latch = data; }));
	map(0x00010, 0x00010).select(0xffe0).rw(FUNC(nemoide_device::ata_r), FUNC(nemoide_device::ata_w));
	map(0x000c8, 0x000c8).mirror(0xff00).lrw8(NAME([this]() { return m_ata->cs1_r(6); })
		, NAME([this](offs_t offset, u8 data) { m_ata->cs1_w(6, data); }));
}

static void nemoide_ata_devices(device_slot_interface &device)
{
	device.option_add("hdd", IDE_HARDDISK);
	device.option_add("cdrom", ATAPI_CDROM);
}

void nemoide_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(nemoide_ata_devices, "hdd", "hdd", false);
}

void nemoide_device::device_start()
{
	save_item(NAME(m_ata_data_latch));

	m_zxbus->install_device(0x00000, 0x1ffff, *this, &nemoide_device::map_io);
}

} // anonymous namespace

} // namespace bus::spectrum::zxbus

DEFINE_DEVICE_TYPE_PRIVATE(ZXBUS_NEMOIDE, device_zxbus_card_interface, bus::spectrum::zxbus::nemoide_device, "zxbus_nemoide", "Nemo IDE Controller")
