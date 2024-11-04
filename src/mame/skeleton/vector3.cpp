// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Vector Graphic, Inc Vector 3 and Vector MZ. Vector Graphic was a Californian
company that made a few popular computers until they were crushed flat by the
IBM PC.

2009-12-08 Skeleton driver for "Vector 4".
2022-12-03 Renamed driver to "Vector 3"

This driver was based on the Vector 4 User Guide but accidentally with the
Vector 3 or other ROMs, because of confusing naming. To get the ROMs to work
required Vector 3 behavior.

The Vector 3 and Vector MZ had similar internal hardware. Both are based on the
ZCB, Flashwriter II, and Micropolis floppy controller.

The 7100-0245-00-00 Extended Systems Monitor 4.3 Users Manual seems to
correspond with BIOS 1.

BIOS 0 only talks to UARTs at 2-7 and nothing else. It uses a terminal
for all communications.

BIOS 1 and 2 only talk to UARTs at 0-7 and the undocumented port 40.
They also write a video screen at F000-F7FF, but there's no writes
to any video controller. Also, the chargen roms (2x 2716) are missing.

https://deramp.com/vector_graphic.html
7200-0203-03-02: https://bitsavers.org/pdf/vectorGraphic/hardware/7200-0204-03-02_ZCB_Single_Board_Computer_Jun80.pdf
https://bitsavers.org/pdf/vectorGraphic/hardware/Vector_Z80_Board_Users_Manual_May79.pdf
https://bitsavers.org/pdf/vectorGraphic/software/7100-0245-00-00_Extended_Systems_Monitor_4.3_Users_Manual_Jul81.pdf
https://bitsavers.org/pdf/vectorGraphic/hardware/Vector_Flashwriter_II_Mar79.pdf
https://deramp.com/downloads/vector_graphic/hardware/Flashwriter%20II%20Manual.pdf
https://deramp.com/downloads/vector_graphic/hardware/Flashwriter%20II%20Schematic%20Rev%204.tif
https://bitsavers.org/pdf/vectorGraphic/hardware/Micropolis_Disk_Controller_Board.pdf

TODO:
- H command goes crazy
- chargen roms
- video
- keyboard
- probable keyboard mcu
- floppies
- many other things

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"


namespace {

class vector3_state : public driver_device
{
public:
	vector3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
	{ }

	void vector3(machine_config &config);

private:
	void vector3_io(address_map &map) ATTR_COLD;
	void vector3_mem(address_map &map) ATTR_COLD;

	memory_passthrough_handler m_rom_shadow_tap;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	void machine_reset() override ATTR_COLD;
};


void vector3_state::vector3_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram().share("mainram");
	map(0xe000, 0xefff).rom().region("maincpu", 0);
	//map(0xf000, 0xf7ff).ram().share("videoram");  // bios 1,2
	map(0xf800, 0xf8ff).rom().region("maincpu", 0x1000);
}

void vector3_state::vector3_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x01).rw("uart0", FUNC(i8251_device::read), FUNC(i8251_device::write)); // keyboard
	map(0x02, 0x03).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write)); // terminal, bios 0 only
	map(0x04, 0x05).mirror(0x02).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x08, 0x0b).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	// map(0x40, 0x40)  undocumented
}

/* Input ports */
static INPUT_PORTS_START( vector3 )
INPUT_PORTS_END


void vector3_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x0fff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xe000, 0xefff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x0fff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}


void vector3_state::vector3(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vector3_state::vector3_mem);
	m_maincpu->set_addrmap(AS_IO, &vector3_state::vector3_io);

	/* video hardware */
	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set("uart1", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart0", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart0", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart1", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_rxc));

	I8251(config, "uart0", 0);

	i8251_device &uart1(I8251(config, "uart1", 0));
	uart1.txd_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	uart1.dtr_handler().set("rs232a", FUNC(rs232_port_device::write_dtr));
	uart1.rts_handler().set("rs232a", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));
	rs232a.cts_handler().set("uart1", FUNC(i8251_device::write_cts));

	i8251_device &uart2(I8251(config, "uart2", 0));
	uart2.txd_handler().set("rs232b", FUNC(rs232_port_device::write_txd));
	uart2.dtr_handler().set("rs232b", FUNC(rs232_port_device::write_dtr));
	uart2.rts_handler().set("rs232b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("uart2", FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set("uart2", FUNC(i8251_device::write_dsr));
	rs232b.cts_handler().set("uart2", FUNC(i8251_device::write_cts));

	I8255A(config, "ppi");
	PIT8253(config, "pit", 0);
}

/* ROM definition */
ROM_START( vector3 )
	ROM_REGION( 0x1100, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mfdc.bin", 0x1000, 0x0100, CRC(d82a40d6) SHA1(cd1ef5fb0312cd1640e0853d2442d7d858bc3e3b))

	ROM_SYSTEM_BIOS( 0, "v4c", "ver 4.0c" ) // VECTOR GRAPHIC MONITOR VERSION 4.0C
	ROMX_LOAD( "vg40cl_ihl.bin", 0x0000, 0x0400, CRC(dcaf79e6) SHA1(63619ddb12ff51e5862902fb1b33a6630f555ad7), ROM_BIOS(0))
	ROMX_LOAD( "vg40ch_ihl.bin", 0x0400, 0x0400, CRC(3ff97d70) SHA1(b401e49aa97ac106c2fd5ee72d89e683ebe34e34), ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "v43", "ver 4.3" ) // VECTOR GRAPHIC MONITOR VERSION 4.3
	ROMX_LOAD( "vg-em-43.bin",   0x0000, 0x1000, CRC(29a0fcee) SHA1(ca44de527f525b72f78b1c084c39aa6ce21731b5), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 2, "v5", "ver 5.0" ) // VECTOR GRAPHIC EXECUTIVE 5.0
	ROMX_LOAD( "vg-zcb50.bin",   0x0000, 0x1000, CRC(22d692ce) SHA1(cbb21b0acc98983bf5febd59ff67615d71596e36), ROM_BIOS(2))

	// bios 1,2 need these (pair of 2716) - rom names not known
	ROM_REGION( 0x1000, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "chargen1.bin", 0x0000, 0x0800, NO_DUMP )
	ROM_LOAD( "chargen2.bin", 0x0800, 0x0800, NO_DUMP )
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY            FULLNAME    FLAGS
COMP( 1979, vector3, 0,      0,      vector3, vector3, vector3_state, empty_init, "Vector Graphic", "Vector 3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
