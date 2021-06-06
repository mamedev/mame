// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    mc10_mcx128.cpp

    Code for emulating Darren Atkinson's MCX-128 cartridge

***************************************************************************/

#include "emu.h"
#include "mc10_mcx128.h"

// #include "machine/ram.h"

// #define VERBOSE (LOG_GENERAL )
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

		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(mc10_mcx128);
		}

	private:
		memory_share_creator<u8> m_share;
		memory_view m_view;
	};
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(MC10_PAK_MCX128, device_mc10cart_interface, mc10_pak_mcx128_device, "mc10_mcx128", "Darren Atkinson's MCX-128 cartridge")


//-------------------------------------------------
//  mc10_pak_device - constructor
//-------------------------------------------------

mc10_pak_mcx128_device::mc10_pak_mcx128_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MC10_PAK_MCX128, tag, owner, clock)
	, device_mc10cart_interface(mconfig, *this)
	, m_share(*this, "ext_ram", 1024*128, ENDIANNESS_BIG)
	, m_view(*this, "mcx_view")
{
}



//**************************************************************************
//  MACHINE FRAGMENTS AND ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc10_pak_mcx128_device::device_start()
{
	owning_slot().install_bank(0x5000, 0x8fff, &m_share[0]);
}
