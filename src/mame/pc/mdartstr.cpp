// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Medalist Dart Star

https://www.youtube.com/watch?v=-kxk8UtTeIM

TODO:
- error 012, in Quadtel-ese means failure from 8237 DMA;
- Implement proper SCATsx chipset (common with pc/at.cpp anch386s romset);
- Emulate 65535 (S)VGA, same as IBM PC-110;
- ROM disk, in ISA space;
- Sound, from parallel ports?
- (very eventually) requires a dart layout;

===================================================================================================

Spectrum Avanti (Medalist Dart Star)
Processor: Intel 386SX (25MHz) (NG80386SX25)

Chipset: CHIPS F82C836 (Single Chip 386sx / SCATsx)

System RAM: 2 x Alliance AS4C256K16F0-50JC (1 MB) [640K Main, 384K Extended]

Video controller: CHIPS F65535/A
VRAM: Alliance AS4C256K16F0-50JC (512 KB)

Additional chips:
2 x NEC D71055C
IDT 7202
Analog Devices AD7224KN
Samsung K6T1008C2E-DL70 near mem roms (mixed ROM/RAM disk?)

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa.h"
#include "cpu/i386/i386.h"
#include "video/pc_vga.h"

namespace {

class mdartstr_state : public driver_device
{
public:
	mdartstr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_isabus(*this, "isa")
	{ }

	void mdartstr(machine_config &config);

private:
	required_device<i386sx_device> m_maincpu;
	required_device<isa16_device> m_isabus;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

void mdartstr_state::main_map(address_map &map)
{
	map(0x000000, 0x09ffff).ram();
	map(0x0a0000, 0x0bffff).rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w));
	map(0x0c8000, 0x0cffff).rom().region("bios", 0x48000); // VGA BIOS + virtual floppy ISA
	map(0x0e0000, 0x0fffff).rom().region("bios", 0x60000);
	map(0x100000, 0x15ffff).ram();
	map(0xf80000, 0xffffff).rom().region("bios", 0x00000);
}

void mdartstr_state::main_io(address_map &map)
{
	map(0x03b0, 0x03df).m("vga", FUNC(vga_device::io_map));
}

static INPUT_PORTS_START( mdartstr )
INPUT_PORTS_END

void mdartstr_state::mdartstr(machine_config &config)
{
	I386SX(config, m_maincpu, 25'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mdartstr_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &mdartstr_state::main_io);
//	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800),900,0,640,526,0,480);
	screen.set_screen_update("vga", FUNC(vga_device::screen_update));

	// TODO: bump to 65535, move to internal ISA space
	vga_device &vga(VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(512*1024);

	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);
//	m_isabus->irq2_callback().set("pic8259_2", FUNC(pic8259_device::ir2_w));
//	m_isabus->irq3_callback().set("pic8259_1", FUNC(pic8259_device::ir3_w));
//	m_isabus->irq4_callback().set("pic8259_1", FUNC(pic8259_device::ir4_w));
//	m_isabus->irq5_callback().set("pic8259_1", FUNC(pic8259_device::ir5_w));
//	m_isabus->irq6_callback().set("pic8259_1", FUNC(pic8259_device::ir6_w));
//	m_isabus->irq7_callback().set("pic8259_1", FUNC(pic8259_device::ir7_w));
//	m_isabus->drq1_callback().set("dma8237_1", FUNC(am9517a_device::dreq1_w));
//	m_isabus->drq2_callback().set("dma8237_1", FUNC(am9517a_device::dreq2_w));
//	m_isabus->drq3_callback().set("dma8237_1", FUNC(am9517a_device::dreq3_w));
}

ROM_START( mdartstr )
	ROM_REGION16_LE( 0x80000, "bios", 0 )
	// 0xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "system rev 1.0.bin", 0x000000, 0x080000, CRC(cdd36a31) SHA1(4ced7065e0923d9cb414b65d2a2d955da080c46b) )

	ROM_REGION16_LE( 0x800000, "romdisk", ROMREGION_ERASEFF )
	// TODO: actual socket positions, verify actual loading
	ROM_LOAD("mem_0 rev 3.25.bin",  0x000000, 0x100000,  CRC(8fa930f1) SHA1(a898b180678086c730dc059f14ddd34a334625c7) )
	ROM_LOAD("mem_1 rev 3.25.bin",  0x100000, 0x100000, CRC(8145bd30) SHA1(70d6a1f7e2ca63431396fd923b6d7d2bdabd56e8) )
	// empty socket mem 2
	// empty socket mem 3
	ROM_LOAD("mem_4 rev 2.0.bin",   0x400000, 0x100000, CRC(61adadd7) SHA1(b1705626e0c47ab213f85d74bc5148c0013e0da9) )
	ROM_LOAD("english rev 3.1.bin", 0x500000, 0x100000, CRC(72ed547b) SHA1(57b80dda3996cd75398c06e4bde92491d4d99c14) ) // mem 5 socket
	// empty socket mem 6
	// static RAM in mem 7
ROM_END

} // anonymous namespace

GAME( 2001, mdartstr, 0, mdartstr, mdartstr, mdartstr_state, empty_init, ROT0, "Medalist Marketing", "Medalist Dart Star (Rev 3.25)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
