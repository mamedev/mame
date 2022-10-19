// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

TODO:
- Verify real contents of the kanji roms. The current dumps seem to have
  been read from inside a running MSX machine. The content retrieved that
  way may not be how they are actually stored in the actual rom.

**********************************************************************************/

#include "emu.h"
#include "kanji.h"

#include "speaker.h"


#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MSX_CART_HXM200, msx_cart_kanji_hxm200_device, "hx_m200", "Toshiba HX-M200 Cartridge")


msx_cart_kanji_hxm200_device::msx_cart_kanji_hxm200_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_HXM200, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_region_kanji(*this, "kanji")
	, m_kanji_address(0)
{
}

ROM_START(msx_cart_hxm200)
	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hx_m200.rom", 0x0000, 0x20000, BAD_DUMP CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

const tiny_rom_entry *msx_cart_kanji_hxm200_device::device_rom_region() const
{
	return ROM_NAME(msx_cart_hxm200);
}

void msx_cart_kanji_hxm200_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0xd8, 0xd9, write8sm_delegate(*this, FUNC(msx_cart_kanji_hxm200_device::kanji_w)));
	io_space().install_read_handler(0xd9, 0xd9, read8sm_delegate(*this, FUNC(msx_cart_kanji_hxm200_device::kanji_r)));

	save_item(NAME(m_kanji_address));
}

void msx_cart_kanji_hxm200_device::device_reset()
{
	m_kanji_address = 0;
}

u8 msx_cart_kanji_hxm200_device::kanji_r(offs_t offset)
{
	u32 latch = m_kanji_address;
	u8 result = m_region_kanji->as_u8(latch);

	if (!machine().side_effects_disabled())
	{
		m_kanji_address = (m_kanji_address & ~0x1f) | ((m_kanji_address + 1) & 0x1f);
	}
	return result;
}

void msx_cart_kanji_hxm200_device::kanji_w(offs_t offset, u8 data)
{
	if (offset)
		m_kanji_address = (m_kanji_address & 0x007e0) | ((data & 0x3f) << 11);
	else
		m_kanji_address = (m_kanji_address & 0x1f800) | ((data & 0x3f) << 5);
}
