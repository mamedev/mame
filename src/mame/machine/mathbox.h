// license:BSD-3-Clause
// copyright-holders:Eric Smith
/*
 * mathbox.h: math box simulation (Battlezone/Red Baron/Tempest)
 *
 * Copyright Eric Smith
 *
 */
#ifndef MAME_MACHINE_MATHBOX_H
#define MAME_MACHINE_MATHBOX_H

#pragma once


/* ----- device interface ----- */
class mathbox_device : public device_t
{
public:
	mathbox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( go_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_READ8_MEMBER( lo_r );
	DECLARE_READ8_MEMBER( hi_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	private:
	// internal state

	/* math box scratch registers */
	int16_t m_reg[16];

	/* math box result */
	int16_t m_result;
};

DECLARE_DEVICE_TYPE(MATHBOX, mathbox_device)

#endif // MAME_MACHINE_MATHBOX_H
