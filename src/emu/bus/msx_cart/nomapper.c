
#include "emu.h"
#include "nomapper.h"

const device_type MSX_CART_NOMAPPER = &device_creator<msx_cart_nomapper>;


msx_cart_nomapper::msx_cart_nomapper(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_NOMAPPER, "MSX Cartridge - ROM", tag, owner, clock, "msx_cart_nomapper", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_start_address(0)
	, m_end_address(0)
{
}

void msx_cart_nomapper::device_start()
{
}

void msx_cart_nomapper::initialize_cartridge()
{
	UINT32 size = get_rom_size();

	m_start_address = 0x4000;

	if (size <= 0x4000)
	{
		// Check if this ROM should be in page 2

		UINT8 *rom = get_rom_base();

		if (rom[0] == 'A' && rom[1] == 'B' && (rom[3] & 0x80))
		{
			m_start_address = 0x8000;
		}
	}

	if (size == 0x10000)
	{
		m_start_address = 0;
	}

	m_end_address = MIN(m_start_address + size, 0x10000);
}

READ8_MEMBER(msx_cart_nomapper::read_cart)
{
	if ( offset >= m_start_address && offset < m_end_address )
	{
		return get_rom_base()[offset - m_start_address];
	}
	return 0xff;
}

