// license:BSD-3-Clause
// copyright-holders:windyfairy
#include "emu.h"
#include "k573kara.h"

#include "bus/rs232/rs232.h"

/*
  Karaoke PCB
  ---------------

  GX921-PWB(B)(?) (C)1999 KONAMI CO. LTD.

  External plate:
  An external plate connects the outside of the machine to the System 573.
  From left to right, top to bottom:
  - 4 pin mini-DIN port labeled "Expansion"
  - 5 pin mini-DIN port labeled "Helper PC"
  - 8 pin mini-DIN port labeled "Coin Box"
  - DB9 male connector labeled "RS232C"
  - DB15 female connector labeled "Control Panel"

  - RCA connector labeled "Video in", internal wiring labeled "1"
  - RCA connector labeled "Video out", internal wiring labeled "2"
  - RCA connector labeled "Montior 1 out", internal wiring labeled "3"
  - RCA connector labeled "Monitor 2 out", internal wiring labeled "4"
  - RJ45 jack labeled "Network 1"
  - RJ45 jack labeled "Network 2"
  - 4 pin dipswitch labeled "DIP"

  About dipswitch: there is a graphic below the dipswitch with the following explanation:
  "Dipswitch 3 controls network 1. Lower it when port is not in use."
  "Dipswitch 4 controls network 2. Lower it when port is not in use."


  Internal I/O board notes:
  - 36.864 MHz clock
  - 2x RJ45(?) ports labeled "MODULAR-BP"
  - 4x composite video plugs labeled "1" (Video in), "2" (Video out), "3" (Monitor 1 out), and "4" (Monitor 2 out)
  - CXA1645M RGB Encoder
  - PC1652D
  - LT1381CS RS232 chip
  - LS245
  - 2x LVT245S
  - Xilinx XC9572
  - Xilinx XC9536
  - CN? Combined RGB + subcarrier signal from main Sys573 board
  - CN3 Helper Computer
  - CN11 Coin Box
  - CN6 Expansion + Lamps(?)
  - CN7 Lamps(?)
*/

DEFINE_DEVICE_TYPE(KONAMI_573_KARAOKE_IO_BOARD, k573kara_device, "k573kara", "Konami 573 karaoke I/O board")

void k573kara_device::amap(address_map &map)
{
	// Known addresses I could find that were used in the game's code
	// Not a full list
	//map(0x10, 0x11)
	map(0x90, 0xaf).rw(m_duart_com, FUNC(pc16552_device::read), FUNC(pc16552_device::write)).umask16(0xff);
	map(0xb0, 0xb1).rw(FUNC(k573kara_device::io_r), FUNC(k573kara_device::lamp1_w));
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

	// The PC Helper RS232 and the PC16552D are right next to each other but they may possibly be separate.
	PC16552D(config, m_duart_com, 0);
	auto& duart_chan0(NS16550(config, "duart_com:chan0", clock() / 2));
	auto& rs232_chan0(RS232_PORT(config, "rs232_chan0", default_rs232_devices, nullptr));
	rs232_chan0.rxd_handler().set("duart_com:chan0", FUNC(ins8250_uart_device::rx_w));
	rs232_chan0.dcd_handler().set("duart_com:chan0", FUNC(ins8250_uart_device::dcd_w));
	rs232_chan0.dsr_handler().set("duart_com:chan0", FUNC(ins8250_uart_device::dsr_w));
	rs232_chan0.ri_handler().set("duart_com:chan0", FUNC(ins8250_uart_device::ri_w));
	rs232_chan0.cts_handler().set("duart_com:chan0", FUNC(ins8250_uart_device::cts_w));
	duart_chan0.out_tx_callback().set("rs232_chan0", FUNC(rs232_port_device::write_txd));
	duart_chan0.out_dtr_callback().set("rs232_chan0", FUNC(rs232_port_device::write_dtr));
	duart_chan0.out_rts_callback().set("rs232_chan0", FUNC(rs232_port_device::write_rts));
}

void k573kara_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
}

uint16_t k573kara_device::io_r()
{
	// Stage platforms can be connected through the RJ45 ports and the data returned through here.
	// 0x200 must be set for this I/O port to be used.
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
