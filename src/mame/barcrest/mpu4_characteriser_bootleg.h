// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_BARCREST_MPU4_CHARACTERISER_BL_H
#define MAME_BARCREST_MPU4_CHARACTERISER_BL_H

#pragma once

#include "cpu/m6809/m6809.h"

// bootleg protection
DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_BL, mpu4_characteriser_bl)

// is this a bootleg, or a much earlier official protection, it's more than just a fixed return value at least
// but has only been seen on 2 games
DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_BL_BLASTBANK, mpu4_characteriser_bl_blastbank)

class mpu4_characteriser_bl : public device_t
{
public:
	// construction/destruction
	mpu4_characteriser_bl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_bl_fixed_return(uint8_t ret)
	{
		m_blfixedreturn = ret;
	}

	uint8_t read(offs_t offset)
	{
		logerror("%s: bootleg Characteriser read offset %02x\n", machine().describe_context(), offset);
		return m_blfixedreturn;
	}

	void write(offs_t offset, uint8_t data)
	{
		logerror("%s: bootleg Characteriser write offset %02x data %02x\n", machine().describe_context(), offset, data);
	}

protected:
	mpu4_characteriser_bl(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_blfixedreturn = 0;
};

class mpu4_characteriser_bl_blastbank : public device_t
{
public:
	// construction/destruction
	mpu4_characteriser_bl_blastbank(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	void set_retxor(uint8_t retxor) { m_retxor = retxor; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t m_retxor = 0x00;

	int m_prot_col = 0;
};

#endif // MAME_BARCREST_MPU4_CHARACTERISER_BL_H
