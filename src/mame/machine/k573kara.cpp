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

  Board notes:
  - 36.864 MHz clock
  - 2x RJ45(?) ports labeled "MODULAR-BP"
  - 4x composite video plugs?
  - CXA1645M RGB Encoder
  - PC1652D
  - LT1381CS RS232 chip
  - LS245
  - 2x LVT245S
  - Xilinx XC9572
  - Xilinx XC9536
  - 5 connector headers
	- Combined RGB + subcarrier signal from main Sys573 board
	- 3 (4?) I/O from front panel
*/

DEFINE_DEVICE_TYPE(KONAMI_573_KARAOKE_IO_BOARD, k573kara_device, "k573kara", "Konami 573 karaoke I/O board")

void k573kara_device::amap(address_map &map)
{
	// Known addresses I could find that were used in the game's code
	// Not a full list
	//map(0x10, 0x11)
	map(0x90, 0xaf).rw(m_duart_com, FUNC(pc16552_device::read), FUNC(pc16552_device::write)).umask16(0xff);
	map(0xb0, 0xb1).rw(FUNC(k573kara_device::unk_r), FUNC(k573kara_device::lamp1_w));
	map(0xc0, 0xc1).w(FUNC(k573kara_device::lamp2_w));
	map(0xd0, 0xd1).w(FUNC(k573kara_device::lamp3_w));
	map(0xe0, 0xe1).rw(FUNC(k573kara_device::digital_id_r), FUNC(k573kara_device::digital_id_w));
	map(0xf0, 0xf1).w(FUNC(k573kara_device::video_selector_w));
	map(0xf8, 0xf9).rw(FUNC(k573kara_device::coin_box_r), FUNC(k573kara_device::coin_box_w));
}

k573kara_device::k573kara_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KONAMI_573_KARAOKE_IO_BOARD, tag, owner, clock),
	digital_id(*this, "digital_id"),
	m_duart_com(*this, "duart_com"),
	m_coin_box_val(0)
{
}

void k573kara_device::device_start()
{
}

void k573kara_device::device_reset()
{
	m_coin_box_val = 0;
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

	// Commands are sent to chan0 from somewhere (the karaoke machine?) to the game via a RS232.
	PC16552D(config, m_duart_com, 0);
	NS16550(config, "duart_com:chan0", clock() / 2);
	NS16550(config, "duart_com:chan1", clock() / 2);
}

void k573kara_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}

uint16_t k573kara_device::unk_r()
{
	// 0x200 seems to be used to trigger something.
	// If 0x200 is set when booting, it says to release the start button.
	return 0;
}

void k573kara_device::lamp1_w(uint16_t data)
{
}

void k573kara_device::lamp2_w(uint16_t data)
{
}

void k573kara_device::lamp3_w(uint16_t data)
{
}

uint16_t k573kara_device::coin_box_r()
{
	// Unsure how the coin box works exactly. It seems to be a toggle instead of a normal coin insert.
	// If you release the coin box button it thinks there is no money inserted and will kick you back to the title screen.
	// The coin box can be disabled in the user settings menu.
	return m_coin_box_val;
}

void k573kara_device::coin_box_w(uint16_t data)
{
	// Possibly some kind of other I/O unrelated to the coin box
	m_coin_box_val = data;
}

void k573kara_device::video_selector_w(uint16_t data)
{
	// This value gets changed when testing during the video selector menu.
	// The IO board seems to have 4 composite video ports so my guess is that it switches connectors.
}

uint16_t k573kara_device::digital_id_r()
{
	return digital_id->read();
}

void k573kara_device::digital_id_w(uint16_t data)
{
	digital_id->write( data & 1 );
}
