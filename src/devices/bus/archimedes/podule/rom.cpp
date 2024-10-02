// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn AKA05 ROM Podule

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AKA05_ROMpodule.html

**********************************************************************/

#include "emu.h"
#include "rom.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/generic/ram.h"
#include "bus/generic/rom.h"
#include "softlist_dev.h"


namespace {

class arc_rom_aka05_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_rom_aka05_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::ROM; }

protected:
	arc_rom_aka05_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;

private:
	required_memory_region m_podule_rom;
	required_device_array<generic_slot_device, 8> m_rom;

	u8 rom_r(offs_t offset);
	void rom_w(offs_t offset, u8 data);

	u8 m_rom_select;
	u8 m_rom_page;
};

// ======================> arc_rom_r225_device

class arc_rom_r225_device : public arc_rom_aka05_device
{
public:
	// construction/destruction
	arc_rom_r225_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


void arc_rom_aka05_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(arc_rom_aka05_device::rom_r), FUNC(arc_rom_aka05_device::rom_w)).umask32(0x000000ff);
	map(0x2000, 0x2fff).lw8(NAME([this](u8 data) { m_rom_page = data & 0x3f; m_rom_select = (m_rom_select & 0x04) | ((data >> 6) & 0x03); })).umask32(0x000000ff);
	map(0x3000, 0x37ff).lw8(NAME([this](u8 data) { m_rom_select &= ~0x04; })).umask32(0x000000ff);
	map(0x3800, 0x3fff).lw8(NAME([this](u8 data) { m_rom_select |= 0x04; })).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( rom )
//-------------------------------------------------

ROM_START( rom_aka05 )
	ROM_REGION(0x20000, "podule_rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "2", "Second release 18-Dec-87")
	ROMX_LOAD("0276,221-02_manager.bin", 0x0000, 0x4000, CRC(23ec4ddb) SHA1(e699157895433010b1ea110c78fbc80a4bc58f09), ROM_BIOS(0))
ROM_END

ROM_START( rom_r225 )
	ROM_REGION(0x40000, "podule_rom", ROMREGION_ERASEFF)
	ROM_LOAD("0270,751-01_r225_boot_rom1.bin", 0x00000, 0x20000, CRC(69117329) SHA1(3ce1a3630bc5d3475216cc403df348f55cad681a))
	ROM_LOAD("0270,752-01_r225_boot_rom2.bin", 0x20000, 0x20000, CRC(2756ee10) SHA1(bb0825670bde59c80a73895141b2d8ad6ea87a2f))
ROM_END

const tiny_rom_entry *arc_rom_aka05_device::device_rom_region() const
{
	return ROM_NAME( rom_aka05 );
}

const tiny_rom_entry *arc_rom_r225_device::device_rom_region() const
{
	return ROM_NAME( rom_r225 );
}


void arc_ram_devices(device_slot_interface &device)
{
	device.option_add("ram", GENERIC_RAM_32K_PLAIN);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_rom_aka05_device::device_add_mconfig(machine_config &config)
{
	// rom sockets 6 x 128K
	GENERIC_SOCKET(config, m_rom[0], generic_plain_slot, "bbc_rom", "bin,rom");
	GENERIC_SOCKET(config, m_rom[1], generic_plain_slot, "bbc_rom", "bin,rom");
	GENERIC_SOCKET(config, m_rom[2], generic_plain_slot, "bbc_rom", "bin,rom");
	GENERIC_SOCKET(config, m_rom[3], generic_plain_slot, "bbc_rom", "bin,rom");
	GENERIC_SOCKET(config, m_rom[4], generic_plain_slot, "bbc_rom", "bin,rom");
	GENERIC_SOCKET(config, m_rom[5], generic_plain_slot, "bbc_rom", "bin,rom");

	// ram sockets 2 x 32K
	GENERIC_SOCKET(config, m_rom[6], arc_ram_devices, "bbc_rom", "bin,rom").set_user_loadable(false);
	GENERIC_SOCKET(config, m_rom[7], arc_ram_devices, "bbc_rom", "bin,rom").set_user_loadable(false);

	SOFTWARE_LIST(config, "rom_ls").set_original("bbc_rom").set_filter("B");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_rom_aka05_device - constructor
//-------------------------------------------------

arc_rom_aka05_device::arc_rom_aka05_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom(*this, "rom%u", 1U)
	, m_rom_select(0)
	, m_rom_page(0)
{
}

arc_rom_aka05_device::arc_rom_aka05_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_rom_aka05_device(mconfig, ARC_ROM_AKA05, tag, owner, clock)
{
}

arc_rom_r225_device::arc_rom_r225_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_rom_aka05_device(mconfig, ARC_ROM_R225, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_rom_aka05_device::device_start()
{
	save_item(NAME(m_rom_select));
	save_item(NAME(m_rom_page));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_rom_aka05_device::device_reset()
{
	m_rom_select = 0x00;
	m_rom_page = 0x00;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

u8 arc_rom_aka05_device::rom_r(offs_t offset)
{
	u8 data = 0xff;
	u32 const addr = offset | (m_rom_page << 11);

	switch (m_rom_select)
	{
	case 0: case 1: case 2: case 3: case 4: case 5:
		if ((m_rom_select * 0x20000) < m_podule_rom->bytes())
			data = m_podule_rom->base()[(m_rom_select * 0x20000) | addr];
		else
			data = m_rom[m_rom_select]->read_rom(addr);
		break;

	case 6: case 7:
		data = m_rom[m_rom_select]->read_ram(addr);
		break;
	}

	return data;
}

void arc_rom_aka05_device::rom_w(offs_t offset, u8 data)
{
	u32 const addr = offset | (m_rom_page << 11);

	switch (m_rom_select)
	{
	case 6: case 7:
		m_rom[m_rom_select]->write_ram(addr, data);
		break;
	}
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_ROM_AKA05, device_archimedes_podule_interface, arc_rom_aka05_device, "arc_rom_aka05", "Acorn AKA05 ROM Podule")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_ROM_R225,  device_archimedes_podule_interface, arc_rom_r225_device,  "arc_rom_r225",  "Acorn AKA05 ROM (with DiscLess Bootstrap support)")
