// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sony SMO-C501 Magneto Optical Disk Controller

    This is the SCSI/M-ESDI bridge controller for the SMO-D501 and
    SMO-E501 MO disk drives.

    The PCB features some ASICs by ADSI.

**********************************************************************/

#include "emu.h"
#include "smoc501.h"

#include "cpu/nec/v5x.h"
#include "machine/wd33c9x.h"

DEFINE_DEVICE_TYPE(SMOC501, smoc501_device, "smoc501", "Sony SMO-C501 MO Disk Controller")

smoc501_device::smoc501_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SMOC501, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "scsic")
{
}

void smoc501_device::device_start()
{
}


void smoc501_device::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0x0a000, 0x0a01f).rw("scsic", FUNC(wd33c93_device::dir_r), FUNC(wd33c93_device::dir_w));
	map(0xf0000, 0xfffff).rom().region("microcode", 0);
}


static INPUT_PORTS_START(smoc501)
	PORT_START("DSW")
	PORT_DIPNAME(0x80, 0x80, "Loading Control") PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, "Eject Mode") PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(0x40, "Eject Button")
	PORT_DIPSETTING(0x00, "Controller")
	PORT_DIPNAME(0x20, 0x20, "Temporary Mode") PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(0x00, "512 Bytes/Sector")
	PORT_DIPSETTING(0x20, "1024 Bytes/Sector")
	PORT_DIPNAME(0x10, 0x00, "Terminator") PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, "On (Last Drive)")
	PORT_DIPNAME(0x08, 0x00, "Auto Spin Up") PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(0x08, "By Drive Unit")
	PORT_DIPSETTING(0x00, "By Controller")
	PORT_DIPNAME(0x07, 0x06, "Drive Number") PORT_DIPLOCATION("DSW:8,7,6")
	PORT_DIPSETTING(0x07, "0")
	PORT_DIPSETTING(0x06, "1")
	PORT_DIPSETTING(0x05, "2")
	PORT_DIPSETTING(0x04, "3")
	PORT_DIPSETTING(0x03, "4")
	PORT_DIPSETTING(0x02, "5")
	PORT_DIPSETTING(0x01, "6")
	PORT_DIPSETTING(0x00, "7")
INPUT_PORTS_END

ioport_constructor smoc501_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(smoc501);
}

void smoc501_device::device_add_mconfig(machine_config &config)
{
	v40_device &mpu(V40(config, "mpu", 16'000'000)); // clock is guessed
	mpu.set_addrmap(AS_PROGRAM, &smoc501_device::mem_map);

	wd33c93_device &scsic(WD33C93(config, "scsic", 10'000'000)); // clock is guessed
	scsic.irq_cb().set_inputline("mpu", INPUT_LINE_IRQ5);
}

ROM_START(smoc501)
	ROM_REGION(0x10000, "microcode", 0)
	ROM_LOAD("c501-00.rom", 0x00000, 0x10000, CRC(0ae04500) SHA1(96c73a3a2ecdc8c903e920f2afba5b4edd5cdba6))
ROM_END

const tiny_rom_entry *smoc501_device::device_rom_region() const
{
	return ROM_NAME(smoc501);
}
