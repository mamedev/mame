// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Emerald Technology Inc. 3XTwin IBM 5251/11 twinax emulation board

    This smallish card is rather tightly packed with DIP ICs (mostly simple
    LSTTL and a few line drivers/receivers) to the point where their
    location numbers are difficult to read. There is no host BIOS, and the
    65C02's "TWINMON" firmware does very little, clearly relying on
    external software.

    Communication between the host system and local CPU seems to take place
    via shared access to a KM62256LP-10 SRAM.

***************************************************************************/

#include "emu.h"
#include "3xtwin.h"

#include "cpu/m6502/r65c02.h"
#include "machine/com52c50.h"


// device type definition
DEFINE_DEVICE_TYPE(ISA8_3XTWIN, isa8_3xtwin_device, "3xtwin", "Emerald Technology 3XTwin Twinax Emulation Card")

isa8_3xtwin_device::isa8_3xtwin_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ISA8_3XTWIN, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
{
}

void isa8_3xtwin_device::device_start()
{
}

void isa8_3xtwin_device::mpu_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2007).m("tic", FUNC(com52c50_device::map));
	map(0x4000, 0xbfff).ram();
	map(0xe000, 0xffff).rom().region("firmware", 0);
}


static INPUT_PORTS_START(3xtwin)
	PORT_START("IOBASE")
	PORT_DIPNAME(0x1f, 0x19, "Base I/O Address") PORT_DIPLOCATION("S1:5,4,3,2,1")
	PORT_DIPSETTING(0x10, "200")
	PORT_DIPSETTING(0x11, "220")
	PORT_DIPSETTING(0x12, "240")
	PORT_DIPSETTING(0x13, "260")
	PORT_DIPSETTING(0x14, "280")
	PORT_DIPSETTING(0x15, "2A0")
	PORT_DIPSETTING(0x16, "2C0")
	PORT_DIPSETTING(0x17, "2E0")
	PORT_DIPSETTING(0x18, "300")
	PORT_DIPSETTING(0x19, "320")
	PORT_DIPSETTING(0x1a, "340")
	PORT_DIPSETTING(0x1b, "360")
	PORT_DIPSETTING(0x1c, "380")
	PORT_DIPSETTING(0x1d, "3A0")
	PORT_DIPSETTING(0x1e, "3C0")
	PORT_DIPSETTING(0x1f, "3E0")

	// TODO: IRQ jumpers
INPUT_PORTS_END

ioport_constructor isa8_3xtwin_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(3xtwin);
}

void isa8_3xtwin_device::device_add_mconfig(machine_config &config)
{
	r65c02_device &mpu(R65C02(config, "mpu", 16_MHz_XTAL / 4)); // R65C02P4
	mpu.set_addrmap(AS_PROGRAM, &isa8_3xtwin_device::mpu_map);

	com52c50_device &tic(COM52C50(config, "tic", 16_MHz_XTAL));
	tic.int1_callback().set_inputline("mpu", r65c02_device::IRQ_LINE);
	tic.int2_callback().set_inputline("mpu", r65c02_device::NMI_LINE);
}

ROM_START(3xtwin)
	ROM_REGION(0x2000, "firmware", 0) // "MON VER E.01"
	ROM_LOAD("370906502.u2", 0x0000, 0x2000, CRC(d4157bc4) SHA1(359428ce0047f9192a44790c8670af956cf6ed70))

	// PLDs (undumped) marked "TWINCLK" (U9: ULC24/22V10-15) and "TWINLGC" (U12: PAL20L10ACNS)
ROM_END

const tiny_rom_entry *isa8_3xtwin_device::device_rom_region() const
{
	return ROM_NAME(3xtwin);
}
