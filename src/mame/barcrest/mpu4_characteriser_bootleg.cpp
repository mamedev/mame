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

mpu4_characteriser_bl::mpu4_characteriser_bl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpu4_characteriser_bl(mconfig, MPU4_CHARACTERISER_BL, tag, owner, clock)
{
}

mpu4_characteriser_bl::mpu4_characteriser_bl(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock)
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
	uint8_t ret = 0x00;

	switch (m_prot_col)
	{
	case 0x00: ret = 0xb8; break;
	case 0x01: ret = 0xa8; break;
	case 0x02: ret = 0x88; break;
	case 0x03: ret = 0x8c; break;
	case 0x04: ret = 0x9c; break;
	case 0x05: ret = 0xbc; break;
	}
	ret ^= m_retxor;

	logerror("%s: bootleg Characteriser read offset %02x returning %02x\n", machine().describe_context(), offset, ret);

	return ret;
}

void mpu4_characteriser_bl_blastbank::write(offs_t offset, uint8_t data)
{
	logerror("%s: bootleg Characteriser write offset %02x data %02x\n", machine().describe_context(), offset, data);
	m_prot_col = data;
}




