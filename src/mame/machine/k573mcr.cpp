// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Memory Card Reader
 *
 */

#include "k573mcr.h"

/*
  GE885-PWB(A)A ( contains Toshiba tmpr3904af, ram, rom, tranceiver and glue ).
*/

k573mcr_device::k573mcr_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, KONAMI_573_MEMORY_CARD_READER, "Konami 573 Memory Card Reader", tag, owner, clock, "k573mcr", __FILE__)
{
}

void k573mcr_device::device_start()
{
}

ROM_START( k573mcr )
	ROM_REGION( 0x080000, "tmpr3904", 0 )
	ROM_LOAD( "885a01.bin",   0x000000, 0x080000, CRC(e22d093f) SHA1(927f62f63b5caa7899392decacd12fea0e6fdbea) )
ROM_END

const rom_entry *k573mcr_device::device_rom_region() const
{
	return ROM_NAME( k573mcr );
}

const device_type KONAMI_573_MEMORY_CARD_READER = &device_creator<k573mcr_device>;
