// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
#ifndef GENFRUIT_H_
#define GENFRUIT_H_

#include "sound/samples.h"

#define SAMPLE_METER 0
#define SAMPLE_PAYOUT 1

const char *const genfruit_sample_names[] =
{
	"*genfruit",
	"meter",
	"payout",
	nullptr   /* end of array */
};

class genfruit_class : public driver_device
{
public:
	genfruit_class(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_samples(*this, "samples")
	{ }

	required_device<samples_device> m_samples;

	void genfruit_audio(machine_config &config);
};


#endif /* GENFRUIT_H_ */
