// license:BSD-3-Clause
// copyright-holders:windyfairy
#include "emu.h"
#include "k573kara.h"

#define LOG_GENERAL    (1 << 0)
// #define VERBOSE        (LOG_GENERAL)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

/*
  Karaoke PCB
  ---------------

  GX921-PWB(B)(?) (C)1999 KONAMI CO. LTD.
*/

DEFINE_DEVICE_TYPE(KONAMI_573_KARAOKE_IO_BOARD, k573kara_device, "k573kara", "Konami 573 karaoke I/O board")

void k573kara_device::amap(address_map &map)
{
	// Known addresses I could find that were used in the game's code
	// Not a full list
	//map(0x10, 0x11)
	//map(0x90, 0x91)
	//map(0x92, 0x93)
	//map(0x94, 0x95)
	//map(0x96, 0x97)
	//map(0x98, 0x99)
	//map(0x9a, 0x9b)
	//map(0xb0, 0xb1)
	//map(0xc0, 0xc0)
	//map(0xd0, 0xd1)
	map(0xe0, 0xe1).rw(FUNC(k573kara_device::digital_id_r), FUNC(k573kara_device::digital_id_w));
	//map(0xf0, 0xf1)
	map(0xf8, 0xf9).rw(FUNC(k573kara_device::coin_box_r), FUNC(k573kara_device::coin_box_w));
}

k573kara_device::k573kara_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KONAMI_573_KARAOKE_IO_BOARD, tag, owner, clock),
	digital_id(*this, "digital_id")
{
}

void k573kara_device::device_start()
{
}

void k573kara_device::device_reset()
{
	coin_box_val = 0;
}

ROM_START( k573kara )
	ROM_REGION( 0x000008, "digital_id", 0 )
	ROM_LOAD( "digital-id.bin",   0x000000, 0x000008, CRC(2b977f4d) SHA1(2b108a56653f91cb3351718c45dfcf979bc35ef1) )
ROM_END

const tiny_rom_entry *k573kara_device::device_rom_region() const
{
	return ROM_NAME(k573kara);
}

void k573kara_device::device_add_mconfig(machine_config &config)
{
	DS2401(config, digital_id);
}

void k573kara_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}

uint16_t k573kara_device::coin_box_r()
{
	return coin_box_val;
}

void k573kara_device::coin_box_w(uint16_t data)
{
	coin_box_val = data;
}

uint16_t k573kara_device::digital_id_r()
{
	return digital_id->read();
}

void k573kara_device::digital_id_w(uint16_t data)
{
	digital_id->write( data & 1 );
}
