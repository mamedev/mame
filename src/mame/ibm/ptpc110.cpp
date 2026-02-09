// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

IBM Palm Top PC-110

TODO:
- Skeleton-ish, needs SCAMP chipset and Super I/O to proceed

===================================================================================================

Components:
- Intel Flash ROM (unknown type);
- RIOS 89G6403 (Intel 486SX-33);
- RIOS 89G6402 (VLSI VL82C420 "SCAMP IV");
- SMC FDC37C665IR Super I/O;
- Chips and Technologies F55535 VGA chip, 512 KiB RAM;
- ESS488 AudioDrive;
- Ricoh RB5C396 (PCMCIA interface, Intel 82365 compatible?). Type-3 Slot, or two Type-1;
- Dallas DS1669S;
- TI TPS2201 (PC Card Power);
- LCD display for time and battery level;
- Modem board, tied to 128KiB SRAM (Samsung KM610000) and 29F040A-12 flash firmware (undumped). 2400bps Data, 9600bps FAX;
- Infra-red port;
- Maxim 786CAI (PSU Controller);
- 4 or 8 MiB RAM, can go up to 28 MiB thru patches. 1 expansion slot;
- 1 port replicator connector;
- Keyboard 89-key Compact JIS with Fn key;
- "Smart-Pico" Flash Card Slot;
- Headphone jack;

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "video/pc_vga_chips.h"

#include "screen.h"

namespace {

class ptpc110_state : public driver_device
{
public:
	ptpc110_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vga(*this, "vga")
	{}

	void ptpc110(machine_config &config);

private:
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	required_device<i486_device> m_maincpu;
	required_device<f65535_vga_device> m_vga;
};


void ptpc110_state::main_map(address_map &map)
{
	map(0x0000'0000, 0x0009'ffff).ram();
	map(0x000a'0000, 0x000b'ffff).rw(m_vga, FUNC(f65535_vga_device::mem_r), FUNC(f65535_vga_device::mem_w));
	map(0x000c'0000, 0x000f'ffff).rom().region("bios", 0);
	map(0xfffc'0000, 0xffff'ffff).rom().region("bios", 0);
}

void ptpc110_state::main_io(address_map &map)
{
	map.unmap_value_high();
	map(0x03b0, 0x03df).m(m_vga, FUNC(f65535_vga_device::io_map));
}


void ptpc110_state::ptpc110(machine_config &config)
{
	const XTAL xtal = XTAL(33'000'000);
	I486(config, m_maincpu, xtal); // i486sx
	m_maincpu->set_addrmap(AS_PROGRAM, &ptpc110_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &ptpc110_state::main_io);
//	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// TODO: on ISA bus
	// 4.7" display size
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(f65535_vga_device::screen_update));

	F65535_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(512*1024);

}


ROM_START( ptpc110 )
	ROM_REGION32_LE( 0x40000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "pc110-flash.bin", 0, 0x40000, CRC(d68bb7a4) SHA1(b5c075842b60accae06bc78ddbf6454d9127de4f) )
ROM_END

}  // anonymous namespace

COMP( 1995, ptpc110, 0, 0, ptpc110, 0, ptpc110_state, empty_init, "International Business Machines", "Palm Top PC-110 (Japan)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
