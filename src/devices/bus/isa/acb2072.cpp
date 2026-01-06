// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Adaptec ACB-2072 RLL Drive Controller

    The PCB features the following ASICs:
    — AIC-010FL Programmable Mass Storage Controller
    — AIC-270L 2,7 RLL Encoder/Decoder
    — AIC-280L (?)
    — AIC-530L (bus interface?)
    — AIC-540L (bus interface?)
    — AIC-6225-15 High-Performance Monolithic Data Separator

    In addition, there is a custom-badged “ACB-2072” 44-pin AMD PLCC which,
    given that it also bears a 1980 Intel copyright as well as Adaptec's,
    is almost certainly a Am8051, Am8053 or equivalent MCU with internal
    ROM. A later revision replaces this IC with a Siemens SAB8052A.

***************************************************************************/

#include "emu.h"
#include "acb2072.h"

#include "cpu/mcs51/i8051.h"

// device type definition
DEFINE_DEVICE_TYPE(ACB2072, acb2072_device, "acb2072", "ACB-2072 RLL Drive Controller")


acb2072_device::acb2072_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ACB2072, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_bios(*this, "bios")
{
}

void acb2072_device::device_start()
{
}

void acb2072_device::device_reset()
{
}


static INPUT_PORTS_START(acb2072)
	PORT_START("ADDR")
	PORT_DIPNAME(0x07, 0x07, "BIOS Address") PORT_DIPLOCATION("J8:3,4,1") // BD, W-X, U-V
	PORT_DIPSETTING(0x03, "Disabled")
	PORT_DIPSETTING(0x07, "C800")
	PORT_DIPSETTING(0x05, "CA00")
	PORT_DIPSETTING(0x04, "CC00")
	PORT_DIPSETTING(0x06, "F400")
	PORT_DIPNAME(0x08, 0x08, "Port Address") PORT_DIPLOCATION("J8:2") // 324
	PORT_DIPSETTING(0x08, "Normal")
	PORT_DIPSETTING(0x00, "Alternate (324)")
	PORT_DIPNAME(0x10, 0x10, "Reserved") PORT_DIPLOCATION("J5:3") // E-F
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "Reserved") PORT_DIPLOCATION("J5:4") // G-H
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, "Reserved") PORT_DIPLOCATION("J5:5") // I-J
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("TABLES")
	PORT_DIPNAME(0x03, 0x03, "Drive 0 Defaults") PORT_DIPLOCATION("J9:2,1") // M-N, O-P
	PORT_DIPSETTING(0x00, "4 Heads, 612 Cylinders (30 MB)")
	PORT_DIPSETTING(0x01, "2 Heads, 612 Cylinders (15 MB)")
	PORT_DIPSETTING(0x02, "5 Heads, 981 Cylinders (60 MB)")
	PORT_DIPSETTING(0x03, "4 Heads, 615 Cylinders (20 MB)")
	PORT_DIPNAME(0x0c, 0x0c, "Drive 1 Defaults") PORT_DIPLOCATION("J9:4,3") // Q-R, S-T
	PORT_DIPSETTING(0x00, "4 Heads, 612 Cylinders (30 MB)")
	PORT_DIPSETTING(0x04, "2 Heads, 612 Cylinders (15 MB)")
	PORT_DIPSETTING(0x08, "5 Heads, 981 Cylinders (60 MB)")
	PORT_DIPSETTING(0x0c, "4 Heads, 615 Cylinders (20 MB)")
	PORT_DIPNAME(0x10, 0x10, "Drive 0 Type") PORT_DIPLOCATION("J5:1") // A-B
	PORT_DIPSETTING(0x10, "Normal")
	PORT_DIPSETTING(0x00, "Syquest")
	PORT_DIPNAME(0x20, 0x20, "Drive 1 Type") PORT_DIPLOCATION("J5:2") // C-D
	PORT_DIPSETTING(0x20, "Normal")
	PORT_DIPSETTING(0x00, "Syquest")
	PORT_DIPNAME(0x40, 0x40, "Self Diagnostics") PORT_DIPLOCATION("J5:6") // K-L
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

ioport_constructor acb2072_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(acb2072);
}

void acb2072_device::device_add_mconfig(machine_config &config)
{
	// XTAL is marked 49S150
	AM8753(config, m_mcu, 15_MHz_XTAL / 2).set_disable();
}


ROM_START(acb2072)
	ROM_REGION(0x2000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "b", "Rev. B")
	ROMX_LOAD("adaptec_inc_408100k_bios_1987_411503_00b.u11", 0x0000, 0x2000, CRC(1b1a854e) SHA1(3550ae7a94e03e77f40bff91024dfc9aed8f6c2f), ROM_BIOS(0)) // M2764AFI
	ROM_SYSTEM_BIOS(1, "a", "Rev. A")
	ROMX_LOAD("adaptec_acb2072_bios_408100_h_411503-00a.u11", 0x0000, 0x2000, CRC(5ca303cf) SHA1(82800005d532f0dc35cbf9ea6af75a97cb8b2e71), ROM_BIOS(1))

	ROM_REGION(0x2000, "mcu", 0)
	ROM_LOAD("adaptec_inc_c86_acb-2072_408101-00b.u8", 0x0000, 0x2000, NO_DUMP)
	//ROM_LOAD("adaptec_inc_c86_acb-2072mcode_408101-00c.u8", 0x0000, 0x2000, NO_DUMP) // SAB8052A-N
ROM_END

const tiny_rom_entry *acb2072_device::device_rom_region() const
{
	return ROM_NAME(acb2072);
}
