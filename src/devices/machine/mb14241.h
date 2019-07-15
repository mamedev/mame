// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*****************************************************************************

    MB14241 shifter IC emulation

 *****************************************************************************/

#ifndef MAME_MACHINE_MB14241_H
#define MAME_MACHINE_MB14241_H

#pragma once

class mb14241_device : public device_t
{
public:
	mb14241_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_WRITE8_MEMBER( shift_count_w );
	DECLARE_WRITE8_MEMBER( shift_data_w );
	DECLARE_READ8_MEMBER( shift_result_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state

	uint16_t m_shift_data;  /* 15 bits only */
	uint8_t m_shift_count;  /* 3 bits */
};

DECLARE_DEVICE_TYPE(MB14241, mb14241_device)

#endif // MAME_MACHINE_MB14241_H
