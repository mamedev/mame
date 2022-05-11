// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 ROM/RAM Module

****************************************************************************/

#include "emu.h"
#include "romram.h"

namespace {

//**************************************************************************
//  RC2014 512K RAM / 512K Flash
//  Module author: Spencer Owen
//**************************************************************************

class rom_ram_512k_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	rom_ram_512k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override { update_banks(); }
	virtual const tiny_rom_entry *device_rom_region() const override;

	void page_w(offs_t offset, uint8_t data) { m_page_reg[offset & 3] = data & 0x3f; update_banks(); }
	void page_en_w(offs_t, uint8_t data) { m_page_en = data & 1; update_banks(); }

	void update_banks();
private:
	uint8_t m_page_reg[4];
	uint8_t m_page_en;
	std::unique_ptr<u8[]> m_ram;
	required_memory_region m_romram;
	memory_bank_array_creator<4> m_bank;
};

rom_ram_512k_device::rom_ram_512k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_ROM_RAM_512, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_page_reg{0,0,0,0}
	, m_page_en(0)
	, m_ram(nullptr)
	, m_romram(*this, "romram")
	, m_bank(*this, "bank%u", 0U)
{
}

void rom_ram_512k_device::device_start()
{
	m_ram = std::make_unique<u8[]>(0x80000);
	std::fill_n(m_ram.get(), 0x80000, 0xff);
	save_pointer(NAME(m_ram), 0x80000);
	save_item(NAME(m_page_en));
	save_item(NAME(m_page_reg));

	// A3 not connected
	m_bus->installer(AS_IO)->install_write_handler(0x70, 0x73, 0, 0x08, 0, write8sm_delegate(*this, FUNC(rom_ram_512k_device::page_w)));
	// A3, A1 and A0 not connected
	m_bus->installer(AS_IO)->install_write_handler(0x74, 0x74, 0, 0x0b, 0, write8sm_delegate(*this, FUNC(rom_ram_512k_device::page_en_w)));

	m_bus->installer(AS_PROGRAM)->install_read_bank(0x0000, 0x3fff, m_bank[0]);
	m_bus->installer(AS_PROGRAM)->install_read_bank(0x4000, 0x7fff, m_bank[1]);
	m_bus->installer(AS_PROGRAM)->install_read_bank(0x8000, 0xbfff, m_bank[2]);
	m_bus->installer(AS_PROGRAM)->install_read_bank(0xc000, 0xffff, m_bank[3]);
}

void rom_ram_512k_device::device_reset()
{
	m_page_en = 0;
	m_page_reg[0] = 0;
	m_page_reg[1] = 0;
	m_page_reg[2] = 0;
	m_page_reg[3] = 0;
	update_banks();
}

void rom_ram_512k_device::update_banks()
{
	if (m_page_en)
	{
		if (m_page_reg[0] & 0x20) {
			m_bank[0]->set_base(m_ram.get() + ((m_page_reg[0] & 0x1f) << 14));
			m_bus->installer(AS_PROGRAM)->install_write_bank(0x0000, 0x3fff, m_bank[0]);
		} else {
			m_bank[0]->set_base(m_romram->base() + (m_page_reg[0] << 14));
			m_bus->installer(AS_PROGRAM)->unmap_write(0x0000, 0x3fff);
		}
		if (m_page_reg[1] & 0x20) {
			m_bank[1]->set_base(m_ram.get() + ((m_page_reg[1] & 0x1f) << 14));
			m_bus->installer(AS_PROGRAM)->install_write_bank(0x4000, 0x7fff, m_bank[1]);
		} else {
			m_bank[1]->set_base(m_romram->base() + (m_page_reg[1] << 14));
			m_bus->installer(AS_PROGRAM)->unmap_write(0x4000, 0x7fff);
		}
		if (m_page_reg[2] & 0x20) {
			m_bank[2]->set_base(m_ram.get() + ((m_page_reg[2] & 0x1f) << 14));
			m_bus->installer(AS_PROGRAM)->install_write_bank(0x8000, 0xbfff, m_bank[2]);
		} else {
			m_bank[2]->set_base(m_romram->base() + (m_page_reg[2] << 14));
			m_bus->installer(AS_PROGRAM)->unmap_write(0x8000, 0xbfff);
		}
		if (m_page_reg[3] & 0x20) {
			m_bank[3]->set_base(m_ram.get() + ((m_page_reg[3] & 0x1f) << 14));
			m_bus->installer(AS_PROGRAM)->install_write_bank(0xc000, 0xffff, m_bank[3]);
		} else {
			m_bank[3]->set_base(m_romram->base() + (m_page_reg[3] << 14));
			m_bus->installer(AS_PROGRAM)->unmap_write(0xc000, 0xffff);
		}
	}
	else
	{
		m_bank[0]->set_base(m_romram->base() + 0x0000);
		m_bank[1]->set_base(m_romram->base() + 0x4000);
		m_bank[2]->set_base(m_romram->base() + 0x8000);
		m_bank[3]->set_base(m_romram->base() + 0xc000);
	}
}

ROM_START(rc2014_rom_ram_512k)
	ROM_REGION( 0x80000, "romram", 0)
	ROM_DEFAULT_BIOS("1.512k")
	// Official ROMs distributed with kit
	ROM_SYSTEM_BIOS(0, "1.512k", "RomWBW RC_Std.ROM 2.9.1-pre5")
	ROMX_LOAD( "rc_1.512k.rom", 0x00000, 0x80000, CRC(f360d908) SHA1(e9c0c79f873eecff9184836025c13915630274c5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "2.512k", "RomWBW RC_Std.ROM 2.9.1-pre5 With PPIDE")
	ROMX_LOAD( "rc_2.512k.rom", 0x00000, 0x80000, CRC(c3aefb4e) SHA1(34541851dc781033b00cdfbe445e1d91811da5c2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "3.512k", "RomWBW RC_Std.ROM 2.9.1-pre5 With RTC")
	ROMX_LOAD( "rc_3.512k.rom", 0x00000, 0x80000, CRC(749f7973) SHA1(2b78bb5ad0595b63c95dc58d48c5144320237362), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "4.512k", "RomWBW RC_Std.ROM 2.9.1-pre5 With PPIDE and RTC")
	ROMX_LOAD( "rc_4.512k.rom", 0x00000, 0x80000, CRC(fbe44292) SHA1(ae0407f0a605e9b1262ce00acf393c30fd87d1e4), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "5.512k", "RomWBW RC_Std.ROM 2.9.1-pre5 With WDC Floppy")
	ROMX_LOAD( "rc_5.512k.rom", 0x00000, 0x80000, CRC(4e1d00dd) SHA1(57b21ce416a0a23e034c2efdb54f669f4b110878), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "6.512k", "RomWBW RC_Std.ROM 2.9.1-pre5 With WDC Floppy and PPIDE")
	ROMX_LOAD( "rc_6.512k.rom", 0x00000, 0x80000, CRC(f2f1535e) SHA1(9121ca4246c520169cca47fda07143de8e903b3e), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "7.512k", "RomWBW RC_Std.ROM 2.9.1-pre5 With WDC Floppy and RTC")
	ROMX_LOAD( "rc_7.512k.rom", 0x00000, 0x80000, CRC(57efe210) SHA1(b9ee93215d69c185d5af511a1105ee06e6474ff2), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "8.512k", "RomWBW RC_Std.ROM 2.9.1-pre5 With WDC Floppy, PPIDE and RTC")
	ROMX_LOAD( "rc_8.512k.rom", 0x00000, 0x80000, CRC(38cc2dfc) SHA1(6e740224276b9d0ae32bb28095c8a8ccb47c38b5), ROM_BIOS(7))
	// Official distribution of ROMWBW
	// Taken from https://github.com/wwarthen/RomWBW/releases
	ROM_SYSTEM_BIOS(8, "2.9.0", "Official RomWBW 2.9.0")
	ROMX_LOAD( "rc_std_2_9_0.rom", 0x00000, 0x80000, CRC(2045d238) SHA1(dd37c945fd531192b368d80b20b0154e6b2d2a75), ROM_BIOS(8))
	ROM_SYSTEM_BIOS(9, "2.9.1", "Official RomWBW 2.9.1")
	ROMX_LOAD( "rcz80_std_2_9_1.rom", 0x00000, 0x80000, CRC(f7c52c5f) SHA1(86a0dbbecfea118cf66f9f35a21f138ce64ae788), ROM_BIOS(9))
	ROM_SYSTEM_BIOS(10, "3.0.0", "Official RomWBW 3.0.0")
	ROMX_LOAD( "rcz80_std_3_0_0.rom", 0x00000, 0x80000, CRC(15b802f8) SHA1(0941c6b00ccdca460d64d16fb374c0380dd431ad), ROM_BIOS(10))
	ROM_SYSTEM_BIOS(11, "3.0.1", "Official RomWBW 3.0.1")
	ROMX_LOAD( "rcz80_std_3_0_1.rom", 0x00000, 0x80000, CRC(6d6b60c5) SHA1(5c642cb3113bc0a51562dc92c8b46bde414adb6c), ROM_BIOS(11))
ROM_END

const tiny_rom_entry *rom_ram_512k_device::device_rom_region() const
{
	return ROM_NAME( rc2014_rom_ram_512k );
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_ROM_RAM_512, device_rc2014_card_interface, rom_ram_512k_device, "rc2014_rom_ram_512k", "RC2014 512K RAM / 512K Flash")
