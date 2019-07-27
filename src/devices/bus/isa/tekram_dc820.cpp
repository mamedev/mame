// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Tekram DC-320/DC-820 SCSI Controllers

    These EISA host adapters have, in addition to internal and external
    SCSI and floppy disk connectors, LED and "SPKER" jumper headers. The
    DC-820 and DC-820B also have four SIMM slots in addition to 64K of
    static RAM on board. The DC-320 and DC-320B only have 16K of SRAM;
    the DC-820B firmware switches into DC-320E mode if it fails to read
    dummy values back from select RAM locations beyond the 16K limit.

    The DC-820B has an ASIC in place of the 11 PLDs used by the DC-820.
    DC-320E likely uses the same ASIC, though probably on a smaller PCB.

    It seems likely that these controllers, like the AHA-174X, support a
    legacy ISA port interface as well as the standard EISA doorbell and
    mailbox I/O provided by the BMIC.

***************************************************************************/

#include "emu.h"
#include "tekram_dc820.h"

#include "cpu/i86/i186.h"
#include "machine/i82355.h"
#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_hd.h"

DEFINE_DEVICE_TYPE(TEKRAM_DC320B, tekram_dc320b_device, "dc320b", "Tekram DC-320B SCSI Controller")
DEFINE_DEVICE_TYPE(TEKRAM_DC320E, tekram_dc320e_device, "dc320e", "Tekram DC-320E SCSI Controller")
DEFINE_DEVICE_TYPE(TEKRAM_DC820, tekram_dc820_device, "dc820", "Tekram DC-820 SCSI Cache Controller")
DEFINE_DEVICE_TYPE(TEKRAM_DC820B, tekram_dc820b_device, "dc820b", "Tekram DC-820B SCSI Cache Controller")

tekram_eisa_scsi_device::tekram_eisa_scsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_mpu(*this, "mpu")
	, m_cmdlatch(*this, "cmdlatch")
	, m_hostlatch(*this, "hostlatch")
	, m_eeprom(*this, "eeprom")
	, m_fdc(*this, "fdc")
	, m_bios(*this, "bios")
{
}

tekram_dc320b_device::tekram_dc320b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tekram_eisa_scsi_device(mconfig, TEKRAM_DC320B, tag, owner, clock)
{
}

tekram_dc320e_device::tekram_dc320e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tekram_eisa_scsi_device(mconfig, TEKRAM_DC320E, tag, owner, clock)
{
}

tekram_dc820_device::tekram_dc820_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tekram_eisa_scsi_device(mconfig, TEKRAM_DC820, tag, owner, clock)
{
}

tekram_dc820b_device::tekram_dc820b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tekram_eisa_scsi_device(mconfig, TEKRAM_DC820B, tag, owner, clock)
{
}

void tekram_eisa_scsi_device::device_start()
{
}

u8 tekram_eisa_scsi_device::latch_status_r()
{
	return (m_cmdlatch->pending_r() << 7) | (m_hostlatch->pending_r() << 6);
}

void tekram_eisa_scsi_device::int0_ack_w(u8 data)
{
}

u8 tekram_eisa_scsi_device::status_r()
{
	return m_eeprom->do_read() << 3;
}

void tekram_eisa_scsi_device::misc_w(u8 data)
{
	logerror("%s: Misc. control register(?) = %02X\n", machine().describe_context(), data);
}

void tekram_eisa_scsi_device::aux_w(u8 data)
{
	logerror("%s: Aux. control register(?) = %02X\n", machine().describe_context(), data);
}

void tekram_eisa_scsi_device::mask_w(u8 data)
{
	logerror("%s: Mask register(?) = %02X\n", machine().describe_context(), data);
}

void tekram_eisa_scsi_device::eeprom_w(u8 data)
{
	m_eeprom->di_write(BIT(data, 0));
	m_eeprom->cs_write(BIT(data, 2));
	m_eeprom->clk_write(BIT(data, 5));
}

void tekram_eisa_scsi_device::common_map(address_map &map)
{
	map(0x10040, 0x1005f).m("scsi:7:scsic", FUNC(ncr53cf94_device::map)).umask16(0xff00);
	map(0x10068, 0x10068).rw(FUNC(tekram_eisa_scsi_device::latch_status_r), FUNC(tekram_eisa_scsi_device::int0_ack_w));
	map(0x10069, 0x10069).r(FUNC(tekram_eisa_scsi_device::status_r));
	map(0x1006a, 0x1006a).w(FUNC(tekram_eisa_scsi_device::misc_w));
	map(0x1006b, 0x1006b).w(FUNC(tekram_eisa_scsi_device::aux_w));
	map(0x1006c, 0x1006c).w(FUNC(tekram_eisa_scsi_device::mask_w));
	map(0x1006d, 0x1006d).w(FUNC(tekram_eisa_scsi_device::eeprom_w));
	map(0x1006f, 0x1006f).r(m_cmdlatch, FUNC(generic_latch_8_device::read));
	map(0x1006f, 0x1006f).w(m_hostlatch, FUNC(generic_latch_8_device::write));
	map(0x10080, 0x10085).rw("bmic", FUNC(i82355_device::local_r), FUNC(i82355_device::local_w)).umask16(0x00ff);
	map(0xf0000, 0xfffff).rom().region("firmware", 0);
}

void tekram_dc320e_device::mpu_map(address_map &map)
{
	common_map(map);
	map(0x00000, 0x03fff).ram();
	map(0x04000, 0x0ffff).noprw();
}

void tekram_dc820b_device::mpu_map(address_map &map)
{
	common_map(map);
	map(0x00000, 0x0ffff).ram();
}

void tekram_dc320b_device::eeprom_w(u8 data)
{
	m_eeprom->di_write(BIT(data, 1));
	m_eeprom->cs_write(BIT(data, 2));
	m_eeprom->clk_write(BIT(data, 5));
}

u8 tekram_dc320b_device::latch_status_r()
{
	return (m_cmdlatch->pending_r() << 7) | (m_hostlatch->pending_r() << 6) | (m_eeprom->do_read() << 4);
}

u8 tekram_dc320b_device::status_r()
{
	return 0;
}

void tekram_dc320b_device::mpu_map(address_map &map)
{
	map(0x00000, 0x03fff).ram();
	map(0x08000, 0x0801f).m("scsi:7:scsic", FUNC(ncr53cf94_device::map)).umask16(0x00ff);
	map(0x08001, 0x08001).r(m_cmdlatch, FUNC(generic_latch_8_device::read));
	map(0x08080, 0x08085).rw("bmic", FUNC(i82355_device::local_r), FUNC(i82355_device::local_w)).umask16(0x00ff);
	map(0x08100, 0x08100).w(FUNC(tekram_dc320b_device::int0_ack_w));
	map(0x08105, 0x08105).w(m_hostlatch, FUNC(generic_latch_8_device::write));
	map(0x08108, 0x08108).w(FUNC(tekram_dc320b_device::aux_w));
	map(0x0810a, 0x0810a).w(FUNC(tekram_dc320b_device::eeprom_w));
	map(0x0810c, 0x0810c).w(FUNC(tekram_dc320b_device::mask_w));
	map(0x0810e, 0x0810e).w(FUNC(tekram_dc320b_device::misc_w));
	map(0x08180, 0x08180).r(FUNC(tekram_dc320b_device::latch_status_r));
	map(0x08182, 0x08182).r(FUNC(tekram_dc320b_device::status_r));
	map(0xf0000, 0xfffff).rom().region("firmware", 0);
}

void tekram_dc820_device::eeprom_w(u8 data)
{
	m_eeprom->di_write(BIT(data, 1));
	m_eeprom->cs_write(BIT(data, 2));
	m_eeprom->clk_write(BIT(data, 5));
}

void tekram_dc820_device::mpu_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x10000, 0x1001f).m("scsi:7:scsic", FUNC(ncr53cf94_device::map)).umask16(0x00ff);
	map(0x10080, 0x10085).rw("bmic", FUNC(i82355_device::local_r), FUNC(i82355_device::local_w)).umask16(0x00ff);
	map(0x10104, 0x10104).w(m_hostlatch, FUNC(generic_latch_8_device::write));
	map(0x10106, 0x10106).w(FUNC(tekram_dc820_device::int0_ack_w));
	map(0x10109, 0x10109).w(FUNC(tekram_dc820_device::aux_w));
	map(0x1010b, 0x1010b).w(FUNC(tekram_dc820_device::eeprom_w));
	map(0x1010d, 0x1010d).w(FUNC(tekram_dc820_device::mask_w));
	map(0x1010f, 0x1010f).w(FUNC(tekram_dc820_device::misc_w));
	map(0x10200, 0x10200).r(m_cmdlatch, FUNC(generic_latch_8_device::read));
	map(0x10281, 0x10281).r(FUNC(tekram_dc820_device::latch_status_r));
	map(0x10283, 0x10283).r(FUNC(tekram_dc820_device::status_r));
	map(0xf0000, 0xfffff).rom().region("firmware", 0);
}

static void tekram_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("scsic", NCR53CF94); // or Emulex FAS216
}

void tekram_eisa_scsi_device::scsic_config(device_t *device)
{
	device->set_clock(40_MHz_XTAL);
	downcast<ncr53cf94_device &>(*device).irq_handler_cb().set(m_mpu, FUNC(i80186_cpu_device::int3_w));
}

void tekram_eisa_scsi_device::scsi_add(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", tekram_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", tekram_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", tekram_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", tekram_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", tekram_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", tekram_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", tekram_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", tekram_scsi_devices, "scsic", true)
		.set_option_machine_config("scsic", [this] (device_t *device) { scsic_config(device); });
}

void tekram_dc320b_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_mpu, 25'000'000); // verified for DC-320, but not DC-320B
	m_mpu->set_addrmap(AS_PROGRAM, &tekram_dc320b_device::mpu_map);

	GENERIC_LATCH_8(config, m_cmdlatch);
	m_cmdlatch->data_pending_callback().set(m_mpu, FUNC(i80186_cpu_device::int1_w));

	GENERIC_LATCH_8(config, m_hostlatch);

	EEPROM_93C46_16BIT(config, m_eeprom);

	i82355_device &bmic(I82355(config, "bmic", 0));
	bmic.lint_callback().set(m_mpu, FUNC(i80186_cpu_device::int2_w));

	scsi_add(config);

	WD37C65C(config, m_fdc, 32'000'000, 9'600'000); // clocks verified for DC-320, but not DC-320B
}

void tekram_dc320e_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_mpu, 32'000'000); // clock guessed to be same as DC-820B due to identical firmware
	m_mpu->set_addrmap(AS_PROGRAM, &tekram_dc320e_device::mpu_map);

	GENERIC_LATCH_8(config, m_cmdlatch);
	m_cmdlatch->data_pending_callback().set(m_mpu, FUNC(i80186_cpu_device::int1_w));

	GENERIC_LATCH_8(config, m_hostlatch);

	EEPROM_93C46_16BIT(config, m_eeprom);

	i82355_device &bmic(I82355(config, "bmic", 0));
	bmic.lint_callback().set(m_mpu, FUNC(i80186_cpu_device::int2_w));

	scsi_add(config);

	WD37C65C(config, m_fdc, 32'000'000, 9'600'000); // clocks verified for DC-320, but not DC-320E
}

void tekram_dc820_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_mpu, 32_MHz_XTAL); // N80C186-16
	m_mpu->set_addrmap(AS_PROGRAM, &tekram_dc820_device::mpu_map);

	GENERIC_LATCH_8(config, m_cmdlatch);
	m_cmdlatch->data_pending_callback().set(m_mpu, FUNC(i80186_cpu_device::int1_w));

	GENERIC_LATCH_8(config, m_hostlatch);

	EEPROM_93C46_16BIT(config, m_eeprom);

	i82355_device &bmic(I82355(config, "bmic", 0));
	bmic.lint_callback().set(m_mpu, FUNC(i80186_cpu_device::int2_w));

	scsi_add(config);

	PC8477A(config, m_fdc, 24_MHz_XTAL); // PC8477AVF
}

void tekram_dc820b_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_mpu, 32_MHz_XTAL); // N80186-16
	m_mpu->set_addrmap(AS_PROGRAM, &tekram_dc820b_device::mpu_map);

	GENERIC_LATCH_8(config, m_cmdlatch);
	m_cmdlatch->data_pending_callback().set(m_mpu, FUNC(i80186_cpu_device::int1_w));

	GENERIC_LATCH_8(config, m_hostlatch);

	EEPROM_93C46_16BIT(config, m_eeprom);

	i82355_device &bmic(I82355(config, "bmic", 0));
	bmic.lint_callback().set(m_mpu, FUNC(i80186_cpu_device::int2_w));

	scsi_add(config);

	PC8477A(config, m_fdc, 24_MHz_XTAL); // PC8477BVF
}

ROM_START(dc320b)
	ROM_REGION(0x8000, "bios", 0) // "SCSI controller BIOS VER 1.07 1994-1-31"
	ROM_LOAD("rev_1.07.u28", 0x0000, 0x8000, CRC(2f796842) SHA1(2a98ff5205ba9ab166aedf354f0cb2b5d59ff0ee))

	ROM_REGION16_LE(0x10000, "firmware", 0) // "Firmware Version 1.07 1994-1-31"
	ROM_LOAD16_BYTE("rev_1.07.u2", 0x0000, 0x8000, CRC(591bd878) SHA1(0418c34a75ee4c8e6ddff2c184c6a263abb0a39d))
	ROM_LOAD16_BYTE("rev_1.07.u3", 0x0001, 0x8000, CRC(f3a7a363) SHA1(f4e3c8d7d12c394247c7ac82dedb895c88b9a907))
ROM_END

ROM_START(dc320e)
	ROM_REGION(0x8000, "bios", 0) // "SCSI controller BIOS VER 1.11 1994-3-31"
	ROM_LOAD("rev_1.11.u19", 0x0000, 0x8000, CRC(45cdc4f6) SHA1(aa782220066685dc34db39a943f28127c02e34d4))

	ROM_REGION16_LE(0x10000, "firmware", 0) // "Firmware Version 1.11 1994-3-31"
	ROM_LOAD16_BYTE("rev_1.11.u2", 0x0000, 0x8000, CRC(51cf5044) SHA1(5aee942b0169b4913cb000fca1a247a0e1da75d4))
	ROM_LOAD16_BYTE("rev_1.11.u1", 0x0001, 0x8000, CRC(07070fba) SHA1(b0847e442cae61da658e9041ad9fe3737e67a0ce))
ROM_END

ROM_START(dc820)
	ROM_REGION(0x8000, "bios", 0) // "SCSI controller BIOS VER 1.07 1994-1-31"
	ROM_LOAD("rev_1.07.u46", 0x0000, 0x8000, CRC(2f796842) SHA1(2a98ff5205ba9ab166aedf354f0cb2b5d59ff0ee))

	ROM_REGION16_LE(0x10000, "firmware", 0) // "Firmware Version 1.07 1994-1-31"
	ROM_LOAD16_BYTE("rev_1.07.u5", 0x0000, 0x8000, CRC(2b7bad09) SHA1(20871220b82994c6ac41357c33d5e18ab1c2f425))
	ROM_LOAD16_BYTE("rev_1.07.u6", 0x0001, 0x8000, CRC(62e65545) SHA1(523a3e3258f07a7f9445620294934bcbe62cd912))
ROM_END

ROM_START(dc820b)
	ROM_REGION(0x8000, "bios", 0) // "SCSI controller BIOS VER 1.12 1996-1-24"
	ROM_LOAD("rev_1.12.u14", 0x0000, 0x8000, CRC(1f0fc01b) SHA1(bd789dc0fb66f7fcfdc4dfd9f3d4b7c6ed32cc6a))

	ROM_REGION16_LE(0x10000, "firmware", 0) // "Firmware Version 1.11 1994-3-31"
	ROM_LOAD16_BYTE("rev_1.11.u12", 0x0000, 0x8000, CRC(51cf5044) SHA1(5aee942b0169b4913cb000fca1a247a0e1da75d4))
	ROM_LOAD16_BYTE("rev_1.11.u11", 0x0001, 0x8000, CRC(07070fba) SHA1(b0847e442cae61da658e9041ad9fe3737e67a0ce))
ROM_END

const tiny_rom_entry *tekram_dc320b_device::device_rom_region() const
{
	return ROM_NAME(dc320b);
}

const tiny_rom_entry *tekram_dc320e_device::device_rom_region() const
{
	return ROM_NAME(dc320e);
}

const tiny_rom_entry *tekram_dc820_device::device_rom_region() const
{
	return ROM_NAME(dc820);
}

const tiny_rom_entry *tekram_dc820b_device::device_rom_region() const
{
	return ROM_NAME(dc820b);
}
