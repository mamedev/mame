// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    mc10_mcx128.cpp

    Code for emulating Darren Atkinson's MCX-128 cartridge

***************************************************************************/

#include "emu.h"
#include "mc10_mcx128.h"

// #include "machine/ram.h"

//#define VERBOSE (LOG_GENERAL )
#include "logmacro.h"


ROM_START(mc10_mcx128)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("mcx128bas.rom", 0x0000, 0x4000, CRC(11202e4b) SHA1(36c30d0f198a1bffee88ef29d92f2401447a91f4))
ROM_END

//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

namespace
{
	// ======================> mc10_pak_device

	class mc10_pak_mcx128_device :
			public device_t,
			public device_mc10cart_interface
	{
	public:
		// construction/destruction
		mc10_pak_mcx128_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
		virtual void device_post_load() override;

		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(mc10_mcx128);
		}

		u8 control_register_read(offs_t offset);
		void control_register_write(offs_t offset, u8 data);

		void view_map0(address_map &map);
		void view_map1(address_map &map);
		void view_map2(address_map &map);
		void view_map3(address_map &map);
		void view_map4(address_map &map);
		void view_map5(address_map &map);
		void view_map6(address_map &map);
		void view_map7(address_map &map);


	private:
		memory_share_creator<u8> m_share;
		memory_view m_view;
		memory_bank_creator m_bank0, m_bank1, m_bank2, m_bank3, m_bank4, m_bank5, m_bank6, m_bank7;
		uint8_t ram_bank_cr;
		uint8_t rom_map_cr;

		void update_banks();

	};
};

DEFINE_DEVICE_TYPE_PRIVATE(MC10_PAK_MCX128, device_mc10cart_interface, mc10_pak_mcx128_device, "mc10_mcx128", "Darren Atkinson's MCX-128 cartridge")

//-------------------------------------------------
//  mc10_pak_device - constructor
//-------------------------------------------------

mc10_pak_mcx128_device::mc10_pak_mcx128_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MC10_PAK_MCX128, tag, owner, clock)
	, device_mc10cart_interface(mconfig, *this)
	, m_share(*this, "ext_ram", 1024*128, ENDIANNESS_BIG)
	, m_view(*this, "mcx_view")
	, m_bank0(*this, "bank0")
	, m_bank1(*this, "bank1")
	, m_bank2(*this, "bank2")
	, m_bank3(*this, "bank3")
	, m_bank4(*this, "bank4")
	, m_bank5(*this, "bank5")
	, m_bank6(*this, "bank6")
	, m_bank7(*this, "bank7")
{
}

void mc10_pak_mcx128_device::view_map0(address_map &map)
{
 	map(0x0004, 0x0007).bankrw("bank0");
 	map(0x000f, 0x000f).bankrw("bank1");
 	map(0x0020, 0x3fff).bankrw("bank2");
//	0x4000, 0x4fff: internal RAM
 	map(0x5000, 0xbeff).bankrw("bank4");
 	map(0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	map(0xc000, 0xfeff).rom().region("eprom",0x0000).bankw("bank5");
 	map(0xff00, 0xffff).rom().region("eprom",0x3f00).bankw("bank7");
}

void mc10_pak_mcx128_device::view_map1(address_map &map)
{
 	map(0x0004, 0x0007).bankrw("bank0");
 	map(0x000f, 0x000f).bankrw("bank1");
 	map(0x0020, 0x3fff).bankrw("bank2");
//	0x4000, 0x4fff: internal RAM
 	map(0x5000, 0xbeff).bankrw("bank4");
 	map(0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	map(0xc000, 0xdfff).bankrw("bank5");
 	map(0xe000, 0xfeff).rom().region("eprom",0x2000).bankw("bank6");
 	map(0xff00, 0xffff).rom().region("eprom",0x3f00).bankw("bank7");
}

void mc10_pak_mcx128_device::view_map2(address_map &map)
{
 	map(0x0004, 0x0007).bankrw("bank0");
 	map(0x000f, 0x000f).bankrw("bank1");
 	map(0x0020, 0x3fff).bankrw("bank2");
//	0x4000, 0x4fff: internal RAM
 	map(0x5000, 0xbeff).bankrw("bank4");
 	map(0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	map(0xc000, 0xdfff).bankrw("bank5");
 	map(0xe000, 0xfeff).bankw("bank6");
 	map(0xff00, 0xffff).bankw("bank7");
// 	0xe000, 0xffff: read internal ROM
}

void mc10_pak_mcx128_device::view_map3(address_map &map)
{
 	map(0x0004, 0x0007).bankrw("bank0");
 	map(0x000f, 0x000f).bankrw("bank1");
 	map(0x0020, 0x3fff).bankrw("bank2");
//	0x4000, 0x4fff: internal RAM
 	map(0x5000, 0xbeff).bankrw("bank4");
 	map(0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	map(0xc000, 0xfeff).bankrw("bank5");
 	map(0xff00, 0xffff).bankrw("bank7");
}

void mc10_pak_mcx128_device::view_map4(address_map &map)
{
 	map(0x0004, 0x0007).bankrw("bank0");
 	map(0x000f, 0x000f).bankrw("bank1");
 	map(0x0020, 0x3fff).bankrw("bank2");
 	map(0x4000, 0xbeff).bankrw("bank3");
 	map(0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	map(0xc000, 0xfeff).rom().region("eprom",0x0000).bankw("bank5");
 	map(0xff00, 0xffff).rom().region("eprom",0x3f00).bankw("bank7");
}

void mc10_pak_mcx128_device::view_map5(address_map &map)
{
 	map(0x0004, 0x0007).bankrw("bank0");
 	map(0x000f, 0x000f).bankrw("bank1");
 	map(0x0020, 0x3fff).bankrw("bank2");
 	map(0x4000, 0xbeff).bankrw("bank3");
 	map(0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	map(0xc000, 0xdfff).bankrw("bank5");
 	map(0xe000, 0xfeff).rom().region("eprom",0x2000).bankw("bank6");
 	map(0xff00, 0xffff).rom().region("eprom",0x3f00).bankw("bank7");
}

void mc10_pak_mcx128_device::view_map6(address_map &map)
{
 	map(0x0004, 0x0007).bankrw("bank0");
 	map(0x000f, 0x000f).bankrw("bank1");
 	map(0x0020, 0x3fff).bankrw("bank2");
 	map(0x4000, 0xbeff).bankrw("bank3");
 	map(0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	map(0xc000, 0xdfff).bankrw("bank5");
 	map(0xe000, 0xfeff).bankw("bank6");
 	map(0xff00, 0xffff).bankw("bank7");
// 	0xe000, 0xffff: read internal ROM
}

void mc10_pak_mcx128_device::view_map7(address_map &map)
{
 	map(0x0004, 0x0007).bankrw("bank0");
 	map(0x000f, 0x000f).bankrw("bank1");
 	map(0x0020, 0x3fff).bankrw("bank2");
 	map(0x4000, 0xbeff).bankrw("bank3");
 	map(0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	map(0xc000, 0xfeff).bankrw("bank5");
 	map(0xff00, 0xffff).bankrw("bank7");
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc10_pak_mcx128_device::device_start()
{
	save_item(NAME(ram_bank_cr));
	save_item(NAME(rom_map_cr));

	// 0x0004 - 0x0007
	m_bank0->configure_entry(0, &m_share[0x00004]);
	m_bank0->configure_entry(1, &m_share[0x10004]);

	// 0x000f - 0x000f
	m_bank1->configure_entry(0, &m_share[0x0000f]);
	m_bank1->configure_entry(1, &m_share[0x1000f]);

	// 0x0000 - 0x3fff
	m_bank2->configure_entry(0, &m_share[0x00020]);
	m_bank2->configure_entry(1, &m_share[0x10020]);

	// 0x4000 - 0x4fff
	m_bank3->configure_entry(0, &m_share[0x08000]);
	m_bank3->configure_entry(1, &m_share[0x18000]);

	// 0x5000 - 0xbeff
	m_bank4->configure_entry(0, &m_share[0x09000]);
	m_bank4->configure_entry(1, &m_share[0x19000]);

	// 0xc000 - 0xdfff
	m_bank5->configure_entry(0, &m_share[0x04000]);
	m_bank5->configure_entry(1, &m_share[0x14000]);

	// 0xe000 - 0xffff
	m_bank6->configure_entry(0, &m_share[0x06000]);
	m_bank6->configure_entry(1, &m_share[0x16000]);

	// 0xff00 - 0xffff
	m_bank7->configure_entry(0, &m_share[0x07f00]);
	m_bank7->configure_entry(1, &m_share[0x07f00]);

 	owning_slot().memspace().install_view(0x0000, 0xffff, m_view);

	m_view[0].install_device(0x0000, 0xffff, *this, &mc10_pak_mcx128_device::view_map0);
	m_view[1].install_device(0x0000, 0xffff, *this, &mc10_pak_mcx128_device::view_map1);
	m_view[2].install_device(0x0000, 0xffff, *this, &mc10_pak_mcx128_device::view_map2);
	m_view[3].install_device(0x0000, 0xffff, *this, &mc10_pak_mcx128_device::view_map3);
	m_view[4].install_device(0x0000, 0xffff, *this, &mc10_pak_mcx128_device::view_map4);
	m_view[5].install_device(0x0000, 0xffff, *this, &mc10_pak_mcx128_device::view_map5);
	m_view[6].install_device(0x0000, 0xffff, *this, &mc10_pak_mcx128_device::view_map6);
	m_view[7].install_device(0x0000, 0xffff, *this, &mc10_pak_mcx128_device::view_map7);
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void mc10_pak_mcx128_device::device_reset()
{
	ram_bank_cr = 0;
	rom_map_cr = 0;
	update_banks();
}

void mc10_pak_mcx128_device::device_post_load()
{
	update_banks();
}

u8 mc10_pak_mcx128_device::control_register_read(offs_t offset)
{
	if (offset==0)
		return ram_bank_cr & 0x03;

	return rom_map_cr & 0x03;
}

void mc10_pak_mcx128_device::control_register_write(offs_t offset, u8 data)
{
	if (offset==0)
		ram_bank_cr = data;
	else
		rom_map_cr = data;

	update_banks();
}

void mc10_pak_mcx128_device::update_banks()
{
	int bank0 = ram_bank_cr & 0x01;
	int bank1 = (ram_bank_cr & 0x02) >> 1;

	m_bank0->set_entry(bank0);
	m_bank0->set_entry(bank0);
	m_bank2->set_entry(bank0);
	m_bank3->set_entry(bank1);
	m_bank4->set_entry(bank1);
	m_bank5->set_entry(bank0);
	m_bank6->set_entry(bank0);
	m_bank7->set_entry(bank0);

	m_view.select((bank1 << 2) | (rom_map_cr & 0x03));

	LOG("view select: %d, bank cr: %d\n", (bank1 << 2) | (rom_map_cr & 0x03), ram_bank_cr & 0x03 );
}
