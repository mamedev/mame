// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

https://segaretro.org/Tekken_Special

Unidentified protection chip, developer not known

TODO:
- identify and verify on HW all the unused combinations;
- chip mirror, if any;

**************************************************************************************************/

#include "emu.h"
#include "tekkensp.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_TEKKENSP, megadrive_unl_tekkensp_device, "megadrive_unl_tekkensp", "Megadrive Tekken Special cart")

megadrive_unl_tekkensp_device::megadrive_unl_tekkensp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_TEKKENSP, tag, owner, clock)
{
}

void megadrive_unl_tekkensp_device::device_start()
{
	megadrive_rom_device::device_start();
	save_item(NAME(m_prot_latch));
}

void megadrive_unl_tekkensp_device::device_reset()
{
	megadrive_rom_device::device_reset();
	// undefined, initialized by game anyway
	m_prot_latch = 0;
}

template <unsigned N> void megadrive_unl_tekkensp_device::prot_shift_w(u8 data)
{
	if (BIT(data, 0))
	{
		m_prot_latch |= (1 << N);
	}
}

void megadrive_unl_tekkensp_device::cart_map(address_map &map)
{
	map(0x00'0000, m_rom_mask).mirror(m_rom_mirror).bankr(m_rom);
	// reset?
	map(0x40'0000, 0x40'0000).lw8(NAME([this] (offs_t offset, u8 data) { (void)data; m_prot_latch = 0; }));
	// -1 is likely coming from the mode
	map(0x40'0002, 0x40'0002).lr8(NAME([this] (offs_t offset) { return m_prot_latch - 1; }));
	map(0x40'0004, 0x40'0004).w(FUNC(megadrive_unl_tekkensp_device::prot_shift_w<0>));
	map(0x40'0006, 0x40'0006).w(FUNC(megadrive_unl_tekkensp_device::prot_shift_w<1>));
	map(0x40'0008, 0x40'0008).w(FUNC(megadrive_unl_tekkensp_device::prot_shift_w<2>));
	map(0x40'000a, 0x40'000a).w(FUNC(megadrive_unl_tekkensp_device::prot_shift_w<3>));
	map(0x40'000c, 0x40'000c).lw8(NAME([this] (offs_t offset, u8 data) {
		if (!BIT(data, 0))
			popmessage("tekkensp.cpp: data output mode bit 0 == 0");
	}));
	map(0x40'000e, 0x40'000e).lw8(NAME([this] (offs_t offset, u8 data) {
		popmessage("tekkensp.cpp: data output mode bit 1 == %d", BIT(data, 0));
	}));
}
