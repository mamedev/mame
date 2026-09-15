// license:BSD-3-Clause
// copyright-holders:gregre365
#ifndef MAME_ARKTECHNICO_WILDCATS_REEL_H
#define MAME_ARKTECHNICO_WILDCATS_REEL_H

#pragma once

#include "machine/steppers.h"

class wildcats_reel_device : public stepper_device
{
public:
	wildcats_reel_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		int16_t start_index,
		int16_t end_index,
		int16_t index_pattern,
		uint8_t init_phase,
		int16_t max_steps = 200 * 2);
	wildcats_reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void advance_phase() override;
};

DECLARE_DEVICE_TYPE(WILDCATS_REEL, wildcats_reel_device)

#endif // MAME_ARKTECHNICO_WILDCATS_REEL_H
