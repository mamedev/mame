// license:BSD-3-Clause
// copyright-holders:David Haywood


#include "emu.h"

#include "mpu4_characteriser_bootleg.h"

// many bootlegs have an initial protection check reading 0x814 or 0x812
// if it passes, other checks are skipped.
//
// this could be a trap, maybe this one is meant to fail and the others are meant to pass



DEFINE_DEVICE_TYPE(MPU4_CHARACTERISER_BL, mpu4_characteriser_bl, "xmpu4chrpalboot19", "MPU4 bootleg Characteriser (fixed returns)")

DEFINE_DEVICE_TYPE(MPU4_CHARACTERISER_BL_BLASTBANK, mpu4_characteriser_bl_blastbank, "mpu4chrboot_blast", "MPU4 bootleg Characteriser (Bank A Blast)")
DEFINE_DEVICE_TYPE(MPU4_CHARACTERISER_BL_COPCASH, mpu4_characteriser_bl_copcash, "mpu4chrboot_cop", "MPU4 bootleg Characteriser (Coppa Cash)")

mpu4_characteriser_bl::mpu4_characteriser_bl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpu4_characteriser_bl(mconfig, MPU4_CHARACTERISER_BL, tag, owner, clock)
{
}

mpu4_characteriser_bl::mpu4_characteriser_bl(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_allow_6809_cheat(false)
{
}

void mpu4_characteriser_bl::device_start()
{
}

void mpu4_characteriser_bl::device_reset()
{
}

mpu4_characteriser_bl_blastbank::mpu4_characteriser_bl_blastbank(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MPU4_CHARACTERISER_BL_BLASTBANK, tag, owner, clock)
{
}

void mpu4_characteriser_bl_blastbank::device_start()
{
}

void mpu4_characteriser_bl_blastbank::device_reset()
{
}

uint8_t mpu4_characteriser_bl_blastbank::read(offs_t offset)
{
	logerror("%s: bootleg Characteriser read offset %02x\n", machine().describe_context(), offset);
	switch (m_prot_col)
	{
	case 0x00: return 0xb8;
	case 0x01: return 0xa8;
	case 0x02: return 0x88;
	case 0x03: return 0x8c;
	case 0x04: return 0x9c;
	case 0x05: return 0xbc;
	}
	return 0xff;
}

void mpu4_characteriser_bl_blastbank::write(offs_t offset, uint8_t data)
{
	logerror("%s: bootleg Characteriser write offset %02x data %02x\n", machine().describe_context(), offset, data);
	m_prot_col = data;
}


mpu4_characteriser_bl_copcash::mpu4_characteriser_bl_copcash(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MPU4_CHARACTERISER_BL_COPCASH, tag, owner, clock)
{
}

void mpu4_characteriser_bl_copcash::device_start()
{
}

void mpu4_characteriser_bl_copcash::device_reset()
{
}

uint8_t mpu4_characteriser_bl_copcash::read(offs_t offset)
{
	logerror("%s: bootleg Characteriser read offset %02x\n", machine().describe_context(), offset);
	switch (m_prot_col)
	{
	case 0x00: return 0xbb;
	case 0x01: return 0xab;
	case 0x02: return 0x8b;
	case 0x03: return 0x8f;
	case 0x04: return 0x9f;
	case 0x05: return 0xbf;
	}
	return 0xff;
}

void mpu4_characteriser_bl_copcash::write(offs_t offset, uint8_t data)
{
	logerror("%s: bootleg Characteriser write offset %02x data %02x\n", machine().describe_context(), offset, data);
	m_prot_col = data;
}



