// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/***************************************************************************

    Fruit machine mechanical sound samples

***************************************************************************/

#ifndef MAME_SHARED_FRUITSAMPLES_H
#define MAME_SHARED_FRUITSAMPLES_H

#pragma once

#include "sound/samples.h"

class fruit_samples_device : public device_t
{
public:
	fruit_samples_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void play(uint8_t index);
	void stop(uint8_t index);

	enum
	{
		SAMPLE_PAYOUT = 0,
		SAMPLE_METER,
		SAMPLE_BUZZER,
		SAMPLE_EM_REEL_1_START,
		SAMPLE_EM_REEL_2_START,
		SAMPLE_EM_REEL_3_START,
		SAMPLE_EM_REEL_4_START,
		SAMPLE_EM_REEL_1_STOP,
		SAMPLE_EM_REEL_2_STOP,
		SAMPLE_EM_REEL_3_STOP,
		SAMPLE_EM_REEL_4_STOP,
		SAMPLE_END
	};

	struct sample_params
	{
		uint32_t id;
		uint8_t channel;
		bool loop;
	};

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<samples_device> m_samples;
};

DECLARE_DEVICE_TYPE(FRUIT_SAMPLES, fruit_samples_device)

#endif // MAME_SHARED_FRUITSAMPLES_H
