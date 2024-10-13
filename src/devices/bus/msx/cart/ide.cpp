// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

Sunrise ATA-IDE and CF ATA-IDE cartridges.

The difference between these two interface cartridges is in the connectors. The
ATA-IDE cartridge has a connector for an IDE cable. The CF ATA-IDE cartridge
has 2 slots for CF cards. The rest of the hardware and firmware on these
cartridges is the same.

******************************************************************************/
#include "emu.h"
#include "ide.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/hdd.h"
#include "machine/intelfsh.h"


namespace
{

class msx_cart_sunrise_ataide_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_sunrise_ataide_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_SUNRISE_ATAIDE, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_flash(*this, "flash")
		, m_ata(*this, "ata")
		, m_view(*this, "view")
		, m_activity_led(*this, "activity_led")
		, m_control(0)
		, m_data(0)
	{ }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<amd_29f010_device> m_flash;
	required_device<ata_interface_device> m_ata;
	memory_view m_view;
	output_finder<> m_activity_led;
	u8 m_control;
	u32 m_bank_base;
	u16 m_data;
};


void msx_cart_sunrise_ataide_device::device_add_mconfig(machine_config &config)
{
	AMD_29F010(config, m_flash);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
	m_ata->dasp_handler().set_output("activity_led");
}


ROM_START(sunrise_ataide)
	ROM_REGION(0x20000, "flash", ROMREGION_ERASEFF)
	// These are not dumped from a real unit but the result of flashing the firmware.
	// Since the full flash contents are written, the contents are expected to be the same.
	ROM_DEFAULT_BIOS("v2.50")
	ROM_SYSTEM_BIOS(0, "v2.50", "v2.50 - 29.01.2013")
	ROMX_LOAD("v2.50.u3", 0, 0x20000, CRC(1944bca9) SHA1(b4bd26c0caee3bdb4e2e2dd1ca6e5ed1e9a7d57e), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v2.40", "v2.40 - 23.03.2005")
	ROMX_LOAD("v2.40.u3", 0, 0x20000, CRC(c73f525b) SHA1(b43e709d62ce4c88072eec4369b4910e89b3fe7b), ROM_BIOS(1))

	// Reverse-patched from a patched variant
	ROM_SYSTEM_BIOS(2, "v2.21", "v2.21 - 26.03.2003")
	ROMX_LOAD("v2.21.u3", 0, 0x20000, BAD_DUMP CRC(fb99f89b) SHA1(a650f51310c2a02c893993e683b54850c278d60b), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "v2.10", "v2.10 - 14.06.2002")
	ROMX_LOAD("v2.10.u3", 0, 0x20000, NO_DUMP, ROM_BIOS(3))

	// Reverse-patched from a patched variant
	ROM_SYSTEM_BIOS(4, "v2.01", "v2.01 - 18.09.2000")
	ROMX_LOAD("v2.01.u3", 0, 0x20000, BAD_DUMP CRC(7fd06ee2) SHA1(bced64ee970e0d66be2634914e4aa58530202f7e), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(5, "v1.99", "v1.99 - 27.02.2000")
	ROMX_LOAD("v1.99.u3", 0, 0x20000, NO_DUMP, ROM_BIOS(5))

	ROM_SYSTEM_BIOS(6, "v1.97", "v1.97 - 20.10.1999")
	ROMX_LOAD("v1.97.u3", 0, 0x20000, NO_DUMP, ROM_BIOS(6))

	ROM_SYSTEM_BIOS(7, "v1.96", "v1.96 - 22.09.1999")
	ROMX_LOAD("v1.96.u3", 0, 0x20000, NO_DUMP, ROM_BIOS(7))

	ROM_REGION(0x1000, "gals", ROMREGION_ERASE00)
	ROM_LOAD("gal16v8d.gal1.jed.u1", 0x0000, 1452, CRC(db625e66) SHA1(5e0718066eea14b89d3801e5464b9a3db41baa2e))
	ROM_LOAD("gal16v8d.gal2.jed.u2", 0x0800, 1494, CRC(22e34ff1) SHA1(cbeb5830ee740747aa61ef5b896e2eb6d7acd7ab))
ROM_END


const tiny_rom_entry *msx_cart_sunrise_ataide_device::device_rom_region() const
{
	return ROM_NAME(sunrise_ataide);
}


void msx_cart_sunrise_ataide_device::device_start()
{
	m_activity_led.resolve();

	save_item(NAME(m_control));
	save_item(NAME(m_bank_base));
	save_item(NAME(m_data));

	page(1)->install_read_handler(0x4000, 0x7fff, read8sm_delegate(*this, [this] (offs_t offset)
		{
			u8 data = m_flash->read(m_bank_base | offset);
			return bitswap(data, 0, 1, 2, 3, 4, 5, 6, 7);
		}, "read")
	);
	page(1)->install_write_handler(0x4104, 0x4104, write8smo_delegate(*this, [this] (u8 data)
		{
			m_control = data;
			m_bank_base = bitswap(m_control, 5, 6, 7) * 0x4000;
			m_view.select(BIT(m_control, 0));
		}, "control")
	);
	page(1)->install_view(0x7c00, 0x7e0f, m_view);
	m_view[0];
	m_view[1].install_read_handler(0x7c00, 0x7dff, read8sm_delegate(*this, [this] (offs_t offset)
		{
			if (!machine().side_effects_disabled())
			{
				if (!BIT(offset, 0))
				{
					m_ata->write_dmack(1);
					m_data = m_ata->read_dma();
					m_ata->write_dmack(0);
					return m_data & 0xff;
				}
			}
			return m_data >> 8;
		}, "read_ide_data")
	);
	m_view[1].install_write_handler(0x7c00, 0x7dff, write8sm_delegate(*this, [this] (offs_t offset, u8 data)
		{
			if (BIT(offset, 0))
			{
				m_data = (m_data & 0x00ff) | (data << 8);
				m_ata->write_dmack(1);
				m_ata->write_dma(m_data);
				m_ata->write_dmack(0);
			}
			else
			{
				m_data = (m_data & 0xff00) | data;
			}
		}, "write_ide_data")
	);
	m_view[1].install_write_handler(0x7e00, 0x7e0f, write8sm_delegate(*this, [this] (offs_t offset, u8 data)
		{
			if (BIT(offset, 3))
				m_ata->cs1_w(offset & 0x07, data, 0xff);
			else
				m_ata->cs0_w(offset & 0x07, data, 0xff);
		}, "write_ide")
	);
	m_view[1].install_read_handler(0x7e00, 0x7e0f, read8sm_delegate(*this, [this] (offs_t offset)
		{
			if (BIT(offset, 3))
				return m_ata->cs1_r(offset & 0x07, 0xff);
			else
				return m_ata->cs0_r(offset & 0x07, 0xff);
		}, "read_ide")
	);

	page(2)->install_write_handler(0x8000, 0xbfff, write8sm_delegate(*this, [this] (offs_t offset, u8 data)
		{
			data = bitswap(data, 0, 1, 2, 3, 4, 5, 6, 7);
			m_flash->write(m_bank_base | (offset & 0x3fff), data);
		}, "write_flash")
	);
}


void msx_cart_sunrise_ataide_device::device_reset()
{
	m_control = 0;
	m_bank_base = 0;
	m_data = 0;
	m_view.select(0);
	m_activity_led = 0;
	// Pulling high breaks status reads
	//m_ata->write_dmack(1);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_SUNRISE_ATAIDE, msx_cart_interface, msx_cart_sunrise_ataide_device, "msx_cart_sunrise_ataide", "Sunrise ATA-IDE/CF ATA-IDE Interface")
