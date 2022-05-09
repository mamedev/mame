// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
#ifndef MAME_MACHINE_GENFRUIT_H
#define MAME_MACHINE_GENFRUIT_H

#pragma once

#include "sound/samples.h"

class genfruit_class : public driver_device
{
public:
	genfruit_class(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_samples(*this, "samples")
	{ }

	required_device<samples_device> m_samples;

	void genfruit_audio(machine_config &config);
	
	enum
	{
		SAMPLE_METER = 0,
		SAMPLE_PAYOUT,
		SAMPLE_END
	};
private:
	const char *const genfruit_sample_names[SAMPLE_END + 2] =
	{
		"*genfruit",
		"meter",
		"payout",
		nullptr   /* end of array */
	};
};


#endif /* MAME_MACHINE_GENFRUIT_H */
