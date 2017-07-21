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
DEFINE_DEVICE_TYPE(BUFFERED_SPRITERAM8,  buffered_spriteram8_device,  "buffered_spriteram8",  "Buffered 8-bit Sprite RAM")
DEFINE_DEVICE_TYPE(BUFFERED_SPRITERAM16, buffered_spriteram16_device, "buffered_spriteram16", "Buffered 16-bit Sprite RAM")
DEFINE_DEVICE_TYPE(BUFFERED_SPRITERAM32, buffered_spriteram32_device, "buffered_spriteram32", "Buffered 32-bit Sprite RAM")
DEFINE_DEVICE_TYPE(BUFFERED_SPRITERAM64, buffered_spriteram64_device, "buffered_spriteram64", "Buffered 64-bit Sprite RAM")


template <typename Type>
buffered_spriteram_device<Type>::buffered_spriteram_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_spriteram(*this, DEVICE_SELF)
{
}


template <typename Type>
void buffered_spriteram_device<Type>::device_start()
{
	if (m_spriteram)
	{
		m_buffered.resize(m_spriteram.bytes() / sizeof(Type));
		save_item(NAME(m_buffered));
	}
}



buffered_spriteram8_device::buffered_spriteram8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: buffered_spriteram_device<u8>(mconfig, BUFFERED_SPRITERAM8, tag, owner, clock)
{
}



buffered_spriteram16_device::buffered_spriteram16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: buffered_spriteram_device<u16>(mconfig, BUFFERED_SPRITERAM16, tag, owner, clock)
{
}



buffered_spriteram32_device::buffered_spriteram32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: buffered_spriteram_device<u32>(mconfig, BUFFERED_SPRITERAM32, tag, owner, clock)
{
}



buffered_spriteram64_device::buffered_spriteram64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: buffered_spriteram_device<u64>(mconfig, BUFFERED_SPRITERAM64, tag, owner, clock)
{
}
