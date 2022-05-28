// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/***************************************************************************

    Fruit machine mechanical sound samples

***************************************************************************/

#include "emu.h"

#include "fruitsamples.h"

#include "speaker.h"

namespace {

const char *const fruit_sample_names[fruit_samples_device::SAMPLE_END + 2] =
{
	"*fruitsamples",
	"payout",
	"meter",
	nullptr   /* end of array */
};

const uint8_t fruit_sample_channels[fruit_samples_device::SAMPLE_END] =
{
	0,
	1
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(FRUIT_SAMPLES, fruit_samples_device, "fruit_samples", "Fruit machine mechanical samples")

fruit_samples_device::fruit_samples_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, FRUIT_SAMPLES, tag, owner, clock),
	m_samples(*this, "samples")
{
}

void fruit_samples_device::device_start()
{

}

void fruit_samples_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "fruitmech").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(fruit_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "fruitmech", 1.0);
}

void fruit_samples_device::play(uint8_t index)
{
	if (index < SAMPLE_END)
		m_samples->start(fruit_sample_channels[index], index);
	else
		fatalerror("fruit_samples_device: Sample index %u out of range\n", index);
}
