// license:BSD-3-Clause
// copyright-holders:Eric Smith
/*
 * mathbox.h: math box simulation (Battlezone/Red Baron/Tempest)
 *
 * Copyright Eric Smith
 *
 */
#ifndef MAME_ATARI_MATHBOX_H
#define MAME_ATARI_MATHBOX_H

#pragma once


/* ----- device interface ----- */
class mathbox_device : public device_t
{
public:
	mathbox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void go_w(offs_t offset, uint8_t data);
	uint8_t status_r();
	uint8_t lo_r();
	uint8_t hi_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	private:
	// internal state

	/* math box scratch registers */
	int16_t m_reg[16]{};

	/* math box result */
	int16_t m_result = 0;
};

DECLARE_DEVICE_TYPE(MATHBOX, mathbox_device)

#endif // MAME_ATARI_MATHBOX_H
