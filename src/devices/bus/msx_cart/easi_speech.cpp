// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Easi-Speech cartridge (R.Amy, 1987)

The program adds a hook to 0xfd29, usage appears to be something like this:
defusr0=&hfd29
x=usr0("hello")

TODO:
- Speech doesn't work. At boot, it's supposed to say "M S X easy speech".
  Maybe ald_w data is scrambled a bit? The expected usage in notes above
  also does not work.

******************************************************************************/

#include "emu.h"
#include "easi_speech.h"


DEFINE_DEVICE_TYPE(MSX_CART_EASISPEECH, msx_cart_easispeech_device, "msx_cart_easispeech", "MSX Cartridge - Easi-Speech")


msx_cart_easispeech_device::msx_cart_easispeech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_EASISPEECH, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_speech(*this, "speech")
{
}

ROM_START( msx_cart_easispeech )
	ROM_REGION( 0x10000, "speech", 0 )
	ROM_LOAD( "sp0256a-al2", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

const tiny_rom_entry *msx_cart_easispeech_device::device_rom_region() const
{
	return ROM_NAME( msx_cart_easispeech );
}

void msx_cart_easispeech_device::device_add_mconfig(machine_config &config)
{
	SP0256(config, m_speech, 3120000); // frequency unknown
	m_speech->add_route(ALL_OUTPUTS, ":speaker", 1.00);
}

uint8_t msx_cart_easispeech_device::read_cart(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0x8000)
		return get_rom_base()[offset & 0x1fff];
	if (offset >= 0x8000 && offset < 0xc000)
		return m_speech->lrq_r() << 7;

	return 0xff;
}

void msx_cart_easispeech_device::write_cart(offs_t offset, uint8_t data)
{
	if (offset >= 0x8000 && offset < 0xc000)
		m_speech->ald_w(data >> 2);
}
