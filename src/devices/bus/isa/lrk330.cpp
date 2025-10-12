// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for ESDI controllers by Lark Associates Inc.

    Major ICs excluding EPROMs, in order of decreasing pin count:
    * Silicon Logic LD1111 HDC (QFP128, U1)
    * Xilinx XC2018-70PC68C Logic Cell Array (PLCC84, U11)
    * National DP8473V Floppy Disk Controller PLUS-2 (PLCC52, U4)
    * Signetics SCN8032HCCA44 8-Bit Microcontroller (PLCC44, U20)
    * Sharp LH52258D-45 32Kx8 CMOS Static RAM (SDIP28, U2 & U3)
    * Vitelic V61C16F70L 2Kx8 CMOS Static RAM (SSOP24, U19)

    The purpose of the small SRAM is not clear. Perhaps it can be banked
    into the lower part of the 8032 program space after loading patches
    for the EPROM microcode, since that uses the upper mirror addresses
    almost exclusively.

    A transfer rate of up to 24 megabits per second was advertised.

    LRK-330 is presumably identical to LRK-331, only without the FDC or
    34-pin floppy drive cable connector installed.

***************************************************************************/

#include "emu.h"
#include "lrk330.h"

#include "imagedev/floppy.h"
#include "machine/upd765.h"

// device type definition
DEFINE_DEVICE_TYPE(LRK331, lrk331_device, "lrk331", "Lark Associates LRK-331 ESDI Controller")


lrk331_device::lrk331_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LRK331, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_bios(*this, "bios")
	, m_config(*this, "CONFIG")
{
}

void lrk331_device::device_start()
{
}

void lrk331_device::device_reset()
{
}

void lrk331_device::ucode_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom().region("ucode", 0);
}

void lrk331_device::ext_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x0b, 0x0b).nopw();
}

static INPUT_PORTS_START(lrk331)
	PORT_START("CONFIG")
	PORT_DIPNAME(0x001, 0x001, "Drive Addresses") PORT_DIPLOCATION("W1:1")
	PORT_DIPSETTING(0x001, "Primary (1F0h-1F7h, 3F0h-3F7h)")
	PORT_DIPSETTING(0x000, "Secondary (170h-177h, 370h-377h)")
	PORT_DIPNAME(0x002, 0x002, "Floppy Drive Speed") PORT_DIPLOCATION("W2:2")
	PORT_DIPSETTING(0x002, "Single")
	PORT_DIPSETTING(0x000, "Dual")
	PORT_DIPNAME(0x004, 0x004, "Interrupt Delay") PORT_DIPLOCATION("W3:1")
	PORT_DIPSETTING(0x004, "Disabled")
	PORT_DIPSETTING(0x000, "Enabled")
	PORT_DIPNAME(0x008, 0x008, "Factory Configured - Do Not Alter") PORT_DIPLOCATION("W4:1")
	PORT_DIPSETTING(0x008, DEF_STR(Off))
	PORT_DIPSETTING(0x000, DEF_STR(On))
	PORT_DIPNAME(0x0f0, 0x0e0, "BIOS Address") PORT_DIPLOCATION("W5-W8:1,2,3,4")
	PORT_DIPSETTING(0x0f0, "Disabled")
	PORT_DIPSETTING(0x0e0, "C800h")
	PORT_DIPSETTING(0x0d0, "CC00h")
	PORT_DIPSETTING(0x0b0, "D000h")
	PORT_DIPSETTING(0x070, "D400h")
	PORT_DIPNAME(0x300, 0x200, "IRQ Select") PORT_DIPLOCATION("W9-W10:1,2")
	PORT_DIPSETTING(0x200, "IRQ 14")
	PORT_DIPSETTING(0x100, "IRQ 15")
	PORT_DIPNAME(0x400, 0x000, "Floppy Drive Interface") PORT_DIPLOCATION("W11:1")
	PORT_DIPSETTING(0x400, "Disabled")
	PORT_DIPSETTING(0x000, "Enabled")
INPUT_PORTS_END

ioport_constructor lrk331_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(lrk331);
}

static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void lrk331_device::device_add_mconfig(machine_config &config)
{
	I8032(config, m_mcu, 24_MHz_XTAL / 2);
	m_mcu->set_addrmap(AS_PROGRAM, &lrk331_device::ucode_map);
	m_mcu->set_addrmap(AS_DATA, &lrk331_device::ext_map);

	//LD1111(config, m_hdc, 24_MHz_XTAL);

	DP8473(config, "fdc", 24_MHz_XTAL);
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "525hd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, nullptr, floppy_image_device::default_pc_floppy_formats);
}

ROM_START(lrk331)
	ROM_REGION(0x2000, "bios", 0)
	ROM_LOAD("63308-10.bin", 0x0000, 0x2000, CRC(f16d4f25) SHA1(437664ff9723c8036a40f1e504899b7cacd21ad0))

	ROM_REGION(0x4000, "ucode", 0)
	ROM_LOAD("63307-08.bin", 0x0000, 0x4000, CRC(6d2e8b26) SHA1(1a33462a29b306b01960cf2ac0b209b2adf58dbe))
ROM_END

const tiny_rom_entry *lrk331_device::device_rom_region() const
{
	return ROM_NAME(lrk331);
}
