// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton device for Conner CFP1080S SCSI hard drive.

    Hardware includes "INDY" and "OCEANVIEW" QFP128 ICs by NCR. The MCU is a
    QFP144 type labeled SC415902BFV.


    Model - Conner CFP1080S
    Interface - Fast SCSI-2
    Capacity 1080MB
    C/H/S - 2794/8/72-114
    Total Blocks - 2,177,184

*******************************************************************************/

#include "emu.h"
#include "bus/nscsi/cfp1080s.h"
#include "cpu/m68hc16/cpu16.h"

DEFINE_DEVICE_TYPE(CFP1080S, cfp1080s_device, "cfp1080s", "Conner CFP1080S")

cfp1080s_device::cfp1080s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nscsi_device(mconfig, CFP1080S, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_hdcpu(*this, "hdcpu")
{
}

void cfp1080s_device::device_start()
{
}

void cfp1080s_device::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("firmware", 0);
	map(0xff081, 0xff081).lr8(NAME([]() { return 0x80; })); // status register of some peripheral
}

void cfp1080s_device::device_add_mconfig(machine_config &config)
{
	MC68HC16Z1(config, m_hdcpu, 8'000'000); // exact type and clock unknown
	m_hdcpu->set_addrmap(AS_PROGRAM, &cfp1080s_device::mem_map);
}

ROM_START(cfp1080s)
	ROM_REGION16_BE(0x20000, "firmware", 0)
	ROM_LOAD16_WORD_SWAP("conner_cfp1080s.bin", 0x00000, 0x20000, CRC(3254150a) SHA1(605be871e75ff775554fc8ce4e09bc2e35a6db09))
ROM_END

const tiny_rom_entry *cfp1080s_device::device_rom_region() const
{
	return ROM_NAME(cfp1080s);
}
