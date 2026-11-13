// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Angelo Salese
/**************************************************************************************************

Ultracart ROM scheme

Observed in SITSA MicroCalc SW (Altirra calls it this way),
PCB label pic has clear "ULTRACART" with an unreadable sub-title from available pic
("PIGNY MEXICO"?)
Sports extra SN74LS169BN (synchronous 4-bit up/down binary counter), any access to CCTL
bankswitch to the next index, disarms RD5 after bank 3, re-enables from bank 0 if accessed again.

"Blizzard 32KB" looks a derived design of Ultracart, it joins a binary counter with RD5 disarm
once it goes past 3rd bank index.

"aDawliah" scheme is again very similar but without any RD5 disarm (just loops back to index 0)
PCB marking "A-NA0002"

TODO:
- exact interface with SN74LS169;

**************************************************************************************************/

#include "emu.h"
#include "ultracart.h"

DEFINE_DEVICE_TYPE(A800_ROM_ULTRACART,     a800_rom_ultracart_device,     "a800_ultracart",    "Atari 8-bit Ultracart \"MicroCalc\" cart")
DEFINE_DEVICE_TYPE(A800_ROM_BLIZZARD_32KB, a800_rom_blizzard_32kb_device, "a800_blizzard_32kb",    "Atari 8-bit Blizzard 32KB cart")
DEFINE_DEVICE_TYPE(A800_ROM_ADAWLIAH,      a800_rom_adawliah_device,      "a800_adawliah",    "Atari 8-bit aDawliah 32KB cart")


a800_rom_ultracart_device::a800_rom_ultracart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, type, tag, owner, clock)
	, m_bank(0)
{
}

a800_rom_ultracart_device::a800_rom_ultracart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_ultracart_device(mconfig, A800_ROM_ULTRACART, tag, owner, clock)
{
}

void a800_rom_ultracart_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_ultracart_device::device_reset()
{
	m_bank = 0;
}

void a800_rom_ultracart_device::cart_map(address_map &map)
{
	map(0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)]; })
	);
}

void a800_rom_ultracart_device::cctl_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(a800_rom_ultracart_device::config_bank_r), FUNC(a800_rom_ultracart_device::config_bank_w));
}

inline void a800_rom_ultracart_device::binary_counter_access()
{
	// TODO: simplification, the real counter has several config options
	m_bank = (m_bank + 1) & 0xf;

	if (m_bank & 0xc)
	{
		rd5_w(0);
		m_bank = 0xf;
	}
	else
		rd5_w(1);
}

u8 a800_rom_ultracart_device::config_bank_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		binary_counter_access();

	return 0xff;
}

void a800_rom_ultracart_device::config_bank_w(offs_t offset, u8 data)
{
	binary_counter_access();
}


/*-------------------------------------------------

 Blizzard 32KB carts

 -------------------------------------------------*/

a800_rom_blizzard_32kb_device::a800_rom_blizzard_32kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_ultracart_device(mconfig, A800_ROM_BLIZZARD_32KB, tag, owner, clock)
{
}

inline void a800_rom_blizzard_32kb_device::binary_counter_access()
{
	// TODO: simplification, the real counter has several config options
	m_bank = (m_bank + 1) & 0xf;

	// as per phoenix carts RD5 can only be re-enabled by restarting the machine
	// it's unknown about how the binary counter behaves once this event occurs
	if (m_bank & 0xc)
		rd5_w(0);
}

/*-------------------------------------------------

 aDawliah 32KB carts

 -------------------------------------------------*/

a800_rom_adawliah_device::a800_rom_adawliah_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_ultracart_device(mconfig, A800_ROM_ADAWLIAH, tag, owner, clock)
{
}

inline void a800_rom_adawliah_device::binary_counter_access()
{
	// TODO: simplification, the real counter has several config options
	m_bank = (m_bank + 1) & 0x3;
}
