// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    bufsprite.h

    Buffered Sprite RAM device.

*********************************************************************/

#include "emu.h"
#include "bufsprite.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type BUFFERED_SPRITERAM8 = device_creator<buffered_spriteram8_device>;
const device_type BUFFERED_SPRITERAM16 = device_creator<buffered_spriteram16_device>;
const device_type BUFFERED_SPRITERAM32 = device_creator<buffered_spriteram32_device>;
const device_type BUFFERED_SPRITERAM64 = device_creator<buffered_spriteram64_device>;


template <typename _Type>
buffered_spriteram_device<_Type>::buffered_spriteram_device(
		const machine_config &mconfig,
		device_type type,
		const char *name,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		const char *shortname,
		const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_spriteram(*this, DEVICE_SELF)
{
}


template <typename _Type>
void buffered_spriteram_device<_Type>::device_start()
{
	if (m_spriteram != nullptr)
	{
		m_buffered.resize(m_spriteram.bytes() / sizeof(_Type));
		save_item(NAME(m_buffered));
	}
}



buffered_spriteram8_device::buffered_spriteram8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: buffered_spriteram_device<u8>(mconfig, BUFFERED_SPRITERAM8, "Buffered 8-bit Sprite RAM", tag, owner, clock, "buffered_spriteram8", __FILE__)
{
}



buffered_spriteram16_device::buffered_spriteram16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: buffered_spriteram_device<u16>(mconfig, BUFFERED_SPRITERAM16, "Buffered 16-bit Sprite RAM", tag, owner, clock, "buffered_spriteram16", __FILE__)
{
}



buffered_spriteram32_device::buffered_spriteram32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: buffered_spriteram_device<u32>(mconfig, BUFFERED_SPRITERAM32, "Buffered 32-bit Sprite RAM", tag, owner, clock, "buffered_spriteram32", __FILE__)
{
}



buffered_spriteram64_device::buffered_spriteram64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: buffered_spriteram_device<u64>(mconfig, BUFFERED_SPRITERAM64, "Buffered 64-bit Sprite RAM", tag, owner, clock, "buffered_spriteram64", __FILE__)
{
}
