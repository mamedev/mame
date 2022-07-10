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
#include "cpu/m68hc16/m68hc16z.h"

DEFINE_DEVICE_TYPE(CFP1080S, cfp1080s_device, "cfp1080s", "Conner CFP1080S")

cfp1080s_device::cfp1080s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nscsi_device(mconfig, CFP1080S, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_hdcpu(*this, "hdcpu")
	, m_addrptr(0)
{
}

void cfp1080s_device::device_start()
{
	save_item(NAME(m_addrptr));
}

u8 cfp1080s_device::ff081_r()
{
	return 0x80; // code often loops until bit 7 of this status register is set
}

u8 cfp1080s_device::ff082_r()
{
	return 0; // bit 6 tested
}

void cfp1080s_device::data_port_w(u8 data)
{
	logerror("Writing $%02X to data port (address pointer = $%06X)\n", data, m_addrptr);
}

void cfp1080s_device::addrptrl_w(u8 data)
{
	m_addrptr = (m_addrptr & 0xffff00) | data;
}

void cfp1080s_device::addrptrm_w(u8 data)
{
	m_addrptr = (m_addrptr & 0xff00ff) | u32(data) << 8;
}

void cfp1080s_device::addrptrh_w(u8 data)
{
	m_addrptr = (m_addrptr & 0x00ffff) | u32(data) << 16;
}

void cfp1080s_device::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("firmware", 0);
	map(0x20000, 0x21fff).ram();
	map(0xff081, 0xff081).r(FUNC(cfp1080s_device::ff081_r));
	map(0xff082, 0xff082).r(FUNC(cfp1080s_device::ff082_r));
	map(0xff090, 0xff090).w(FUNC(cfp1080s_device::data_port_w));
	map(0xff098, 0xff098).w(FUNC(cfp1080s_device::addrptrl_w));
	map(0xff099, 0xff099).w(FUNC(cfp1080s_device::addrptrm_w));
	map(0xff09a, 0xff09a).w(FUNC(cfp1080s_device::addrptrh_w));
	map(0xff8c6, 0xff8c7).noprw(); // ?
}

void cfp1080s_device::device_add_mconfig(machine_config &config)
{
	MC68HC16Z1(config, m_hdcpu, 20'000'000); // exact type and clock unknown
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
