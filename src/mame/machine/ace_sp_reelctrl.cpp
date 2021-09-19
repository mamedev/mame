// license:BSD-3-Clause
// copyright-holders:David Haywood

// This device handles the reel controller boards for the sp.ACE system which were based around a 68705P3 MCU
// The PCP version is a reverse engineered clone of the original ACE one, created by PCP to get their own
// games on the platform.  They are not fully compatible with each other.

#include "emu.h"
#include "ace_sp_reelctrl.h"

DEFINE_DEVICE_TYPE(ACE_SP_REELCTRL, ace_sp_reelctrl_device, "ace_sp_reelctrl", "ACE sp.ACE Reel Controller PCB")
DEFINE_DEVICE_TYPE(ACE_SP_REELCTRL_PCP, ace_sp_reelctrl_pcp_device, "ace_sp_reelctrl_pcp", "ACE sp.ACE Reel Controller PCB (PCP clone)")

ace_sp_reelctrl_base_device::ace_sp_reelctrl_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_mcu(*this, "mcu")
{
}

ace_sp_reelctrl_device::ace_sp_reelctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ace_sp_reelctrl_base_device(mconfig, ACE_SP_REELCTRL, tag, owner, clock)
{
}

ace_sp_reelctrl_pcp_device::ace_sp_reelctrl_pcp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ace_sp_reelctrl_base_device(mconfig, ACE_SP_REELCTRL_PCP, tag, owner, clock)
{
}


void ace_sp_reelctrl_base_device::device_start()
{
}

void ace_sp_reelctrl_base_device::device_reset()
{
}

void ace_sp_reelctrl_base_device::device_add_mconfig(machine_config &config)
{
	M68705P3(config, m_mcu, DERIVED_CLOCK(1, 1));
}

ROM_START( ace_sp_reelctrl )
	ROM_REGION16_BE( 0x0800, "mcu", 0 ) 
	ROM_LOAD( "ace reel mcu v9 _space_.bin", 0, 0x0800, CRC(d7b91fe0) SHA1(44f78da8138caf4f79f1e8e9a2abe53d79c0fe79) ) // MC68705P3 internal ROM
ROM_END

const tiny_rom_entry *ace_sp_reelctrl_device::device_rom_region() const
{
	return ROM_NAME(ace_sp_reelctrl);
}

ROM_START( ace_sp_reelctrl_pcp )
	ROM_REGION16_BE( 0x0800, "mcu", 0 ) 
	ROM_LOAD( "pcp reel mcu fcr 1 _space_.bin", 0, 0x0800, CRC(1c8019bf) SHA1(d1d6ea68d9aace4a4ea0de212f17426bccfe5856) ) // MC68705P3 internal ROM
ROM_END

const tiny_rom_entry *ace_sp_reelctrl_pcp_device::device_rom_region() const
{
	return ROM_NAME(ace_sp_reelctrl_pcp);
}

