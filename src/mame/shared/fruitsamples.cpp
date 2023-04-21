// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/***************************************************************************

    Fruit machine mechanical sound samples

***************************************************************************/

#include "emu.h"

#include "fruitsamples.h"

#include "speaker.h"

namespace {

const char *const fruit_sample_names[] =
{
	"*fruitsamples",
	"payout",        // 0
	"meter",         // 1
	"buzzer",        // 2
	"em_reel_start", // 3
	"em_reel_stop",  // 4
	nullptr
};

const fruit_samples_device::sample_params params[] =
{
	//id ch loop
	{ 0, 0, false }, // SAMPLE_PAYOUT
	{ 1, 1, false }, // SAMPLE_METER
	{ 2, 2, false }, // SAMPLE_BUZZER
	{ 3, 3, false }, // SAMPLE_EM_REEL_1_START
	{ 3, 4, false }, // SAMPLE_EM_REEL_2_START
	{ 3, 5, false }, // SAMPLE_EM_REEL_3_START
	{ 3, 6, false }, // SAMPLE_EM_REEL_4_START
	{ 4, 3, false }, // SAMPLE_EM_REEL_1_STOP
	{ 4, 4, false }, // SAMPLE_EM_REEL_2_STOP
	{ 4, 5, false }, // SAMPLE_EM_REEL_3_STOP
	{ 4, 6, false }  // SAMPLE_EM_REEL_4_STOP
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(FRUIT_SAMPLES, fruit_samples_device, "fruit_samples", "Fruit Machine Mechanical Samples")

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
	m_samples->set_channels(7);
	m_samples->set_samples_names(fruit_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "fruitmech", 1.0);
}

void fruit_samples_device::play(uint8_t index)
{
	if(index < SAMPLE_END)
	{
		sample_params sample = params[index];
		m_samples->start(sample.channel, sample.id, sample.loop);
	}
	else
	{
		fatalerror("fruit_samples_device::play: Sample index %u out of range\n", index);
	}
}

void fruit_samples_device::stop(uint8_t index)
{
	if(index < SAMPLE_END)
	{
		sample_params sample = params[index];
		m_samples->stop(sample.channel);
	}
	else
	{
		fatalerror("fruit_samples_device::stop: Sample index %u out of range\n", index);
	}
}
