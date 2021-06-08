// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    mc10_mcx128.cpp

    Code for emulating Darren Atkinson's MCX-128 cartridge

***************************************************************************/

#include "emu.h"
#include "mc10_mcx128.h"

// #include "machine/ram.h"

#define VERBOSE (LOG_GENERAL )
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

	private:
		memory_share_creator<u8> m_share;
		memory_view m_view;
		memory_bank_creator m_bank0, m_bank1, m_bank2, m_bank3;
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
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc10_pak_mcx128_device::device_start()
{
	save_item(NAME(ram_bank_cr));
	save_item(NAME(rom_map_cr));

	m_bank0->configure_entry(0, &m_share[0x00000]);
	m_bank0->configure_entry(1, &m_share[0x10000]);

	m_bank1->configure_entry(0, &m_share[0x08000]);
	m_bank1->configure_entry(1, &m_share[0x18000]);

	m_bank2->configure_entry(0, &m_share[0x04000]);
	m_bank2->configure_entry(1, &m_share[0x14000]);

	m_bank3->configure_entry(0, &m_share[0x06000]);
	m_bank3->configure_entry(1, &m_share[0x16000]);

//  	owning_slot().memspace().install_view(0x0000, 0xffff, m_view);
	memory_region *memregion("eprom");
	owning_slot().install_rom(0xc000, 0xffff, memregion->base())

 	m_view[0](0x0000, 0x3fff).bankrw("bank0");
//	0x4000, 0xbeff: internal RAM
 	m_view[0](0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	m_view[0](0xc000, 0xffff).rom().region("eprom",0).bankw("bank2");

 	m_view[1](0x0000, 0x3fff).bankrw("bank0");
//	0x4000, 0xbeff: internal RAM
 	m_view[1](0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	m_view[1](0xc000, 0xdfff).bankrw("bank2");
 	m_view[1](0xe000, 0xffff).rom().region("eprom",0x2000).bankw("bank3");

 	m_view[2](0x0000, 0x3fff).bankrw("bank0");
//	0x4000, 0xbeff: internal RAM
 	m_view[2](0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	m_view[2](0xc000, 0xdfff).bankrw("bank2");
// 	0xe000, 0xffff: internal ROM

 	m_view[3](0x0000, 0x3fff).bankrw("bank0");
//	0x4000, 0xbeff: internal RAM
 	m_view[3](0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	m_view[3](0xc000, 0xffff).bankrw("bank2");

 	m_view[4](0x0000, 0x3fff).bankrw("bank0");
 	m_view[4](0x4000, 0xbeff).bankrw("bank1");
 	m_view[4](0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	m_view[4](0xc000, 0xffff).rom().region("eprom",0).bankw("bank2");

 	m_view[5](0x0000, 0x3fff).bankrw("bank0");
 	m_view[5](0x4000, 0xbeff).bankrw("bank1");
 	m_view[5](0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	m_view[5](0xc000, 0xdfff).bankrw("bank2");
 	m_view[5](0xe000, 0xffff).rom().region("eprom",0x2000).bankw("bank3");

 	m_view[6](0x0000, 0x3fff).bankrw("bank0");
 	m_view[6](0x4000, 0xbeff).bankrw("bank1");
 	m_view[6](0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	m_view[6](0xc000, 0xdfff).bankrw("bank2");
// 	0xe000, 0xffff: internal ROM

 	m_view[7](0x0000, 0x3fff).bankrw("bank0");
 	m_view[7](0x4000, 0xbeff).bankrw("bank1");
 	m_view[7](0xbf00, 0xbf01).rw(FUNC(mc10_pak_mcx128_device::control_register_read), FUNC(mc10_pak_mcx128_device::control_register_write));
 	m_view[7](0xc000, 0xffff).bankrw("bank2");
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
	m_bank0->set_entry(ram_bank_cr & 0x01);
	m_bank1->set_entry((ram_bank_cr & 0x02) >> 1);
	m_bank2->set_entry(ram_bank_cr & 0x01);
	m_bank3->set_entry(ram_bank_cr & 0x01);

	m_view.select(((ram_bank_cr & 0x02) << 1) | (rom_map_cr & 0x03));

	LOG("view select: %d, bank cr: %d\n", ((ram_bank_cr & 0x02) << 1) | (rom_map_cr & 0x03), ram_bank_cr & 0x03 );
}
