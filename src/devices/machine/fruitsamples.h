// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/***************************************************************************

	Fruit machine mechanical sound samples

***************************************************************************/

#ifndef MAME_MACHINE_FRUITSAMPLES_H
#define MAME_MACHINE_FRUITSAMPLES_H

#pragma once

#include "sound/samples.h"

#include "speaker.h"

class fruit_samples_device : public device_t
{
public:
	fruit_samples_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void play(uint8_t index);

	enum
	{
		SAMPLE_PAYOUT = 0,
		SAMPLE_METER,
		SAMPLE_END
	};

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<samples_device> m_samples;

	const char *fruit_sample_names[SAMPLE_END + 2] =
	{
		"*fruitsamples",
		"payout",
		"meter",
		nullptr   /* end of array */
	};
	const uint8_t fruit_sample_channels[SAMPLE_END] =
	{
		0,
		1
	};
};

DECLARE_DEVICE_TYPE(FRUIT_SAMPLES, fruit_samples_device)

#endif // MAME_MACHINE_FRUITSAMPLES_H