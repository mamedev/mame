// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    ram.cpp

    Code for emulating the Radio Shack 16K RAM Cartridge

***************************************************************************/

#include "emu.h"
#include "ram.h"

// #include "machine/ram.h"

// #define VERBOSE (LOG_GENERAL )
#include "logmacro.h"


//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

namespace
{
	// ======================> mc10_pak_device

	class mc10_pak_ram_device :
			public device_t,
			public device_mc10cart_interface
	{
	public:
		// construction/destruction
		mc10_pak_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	protected:
		// device_t implementation
		virtual void device_start() override ATTR_COLD;
	private:
		memory_share_creator<u8> m_share;
	};
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(MC10_PAK_RAM, device_mc10cart_interface, mc10_pak_ram_device, "mc10pakram", "Radio Shack 16K RAM Cartridge")



//-------------------------------------------------
//  mc10_pak_device - constructor
//-------------------------------------------------

mc10_pak_ram_device::mc10_pak_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MC10_PAK_RAM, tag, owner, clock)
	, device_mc10cart_interface(mconfig, *this)
	, m_share(*this, "ext_ram", 1024*16, ENDIANNESS_BIG)
{
}



//**************************************************************************
//  MACHINE FRAGMENTS AND ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc10_pak_ram_device::device_start()
{
	owning_slot().memspace().install_ram(0x5000, 0x8fff, &m_share[0]);
}
