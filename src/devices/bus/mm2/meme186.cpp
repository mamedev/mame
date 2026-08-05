// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 MEME186 emulation

*********************************************************************/

#include "emu.h"
#include "meme186.h"

DEFINE_DEVICE_TYPE(NOKIA_MEME186, meme186_device, "nokia_meme186", "Nokia MikroMikko 2 MEME186")

meme186_device::meme186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NOKIA_MEME186, tag, owner, clock),
	device_mikromikko2_expansion_bus_card_interface(mconfig, *this),
	m_ram(*this, "ram", 0x80000, ENDIANNESS_LITTLE)
{
}

void meme186_device::device_start()
{
	m_bus->memspace().install_ram(0x40000, 0xbffff, m_ram);
}
