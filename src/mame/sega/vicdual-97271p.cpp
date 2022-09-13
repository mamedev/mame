// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
N-Sub Oscillator 97271-P
-----------------------------
for the time being this is a simple sample player based on the work by MASH.
*/

#include "emu.h"
#include "speaker.h"
#include "vicdual-97271p.h"

#define S97271P_TAG     "s97271p"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(S97271P, s97271p_device, "s97271p", "N-Sub Oscillator 97271-P")

/* bit definitions - sound effect drive outputs */
#define S97271P_WARNING      0x01
#define S97271P_SONAR        0x02
#define S97271P_LAUNCH       0x04
#define S97271P_EXPL_L       0x08
#define S97271P_EXPL_S       0x10
#define S97271P_BONUS        0x20
#define S97271P_CODE         0x40
#define S97271P_BOAT         0x80

/* sample file names */
static const char *const nsub_sample_names[] =
{
	"*nsub",
	"SND_EXPL_L0",
	"SND_EXPL_L1",
	"SND_SONAR",
	"SND_LAUNCH0",
	"SND_LAUNCH1",
	"SND_WARNING0",
	"SND_WARNING1",
	"SND_EXPL_S0",
	"SND_EXPL_S1",
	"SND_BONUS0",
	"SND_BONUS1",
	"SND_CODE",
	"SND_BOAT",
	nullptr
};

/* sample ids - must match sample file name table above */
enum
{
	SND_EXPL_L0 = 0,
	SND_EXPL_L1,
	SND_SONAR,
	SND_LAUNCH0,
	SND_LAUNCH1,
	SND_WARNING0,
	SND_WARNING1,
	SND_EXPL_S0,
	SND_EXPL_S1,
	SND_BONUS0,
	SND_BONUS1,
	SND_CODE,
	SND_BOAT
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s97271p_device - constructor
//-------------------------------------------------

s97271p_device::s97271p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, S97271P, tag, owner, clock),
	m_samples(*this, "samples")
{
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void s97271p_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	/* samples */
	SAMPLES(config, m_samples);
	m_samples->set_channels(13);
	m_samples->set_samples_names(nsub_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s97271p_device::device_start()
{
	save_item(NAME(m_state));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s97271p_device::device_reset()
{
	m_state = 0xff;
}

void s97271p_device::port_w(uint8_t data)
{
	uint8_t bitsChanged = m_state ^ data;
	uint8_t bitsGoneHigh = bitsChanged & data;
	uint8_t bitsGoneLow = bitsChanged & ~data;
	m_state = data;

	if (bitsGoneLow & S97271P_WARNING)
	{
		m_samples->start(SND_WARNING0, SND_WARNING0, 1);
		m_samples->stop(SND_WARNING1);
	} else if (bitsGoneHigh & S97271P_WARNING)
	{
		m_samples->start(SND_WARNING1, SND_WARNING1, 0);
		m_samples->stop(SND_WARNING0);
	}

	if (bitsGoneLow & S97271P_SONAR)
	{
		m_samples->start(SND_SONAR, SND_SONAR, 1);
	} else if (bitsGoneHigh & S97271P_SONAR)
	{
		m_samples->stop(SND_SONAR);
	}

	if (bitsGoneLow & S97271P_LAUNCH)
	{
		m_samples->start(SND_LAUNCH0, SND_LAUNCH0, 1);
		m_samples->stop(SND_LAUNCH1);
	} else if (bitsGoneHigh & S97271P_LAUNCH)
	{
		m_samples->start(SND_LAUNCH1, SND_LAUNCH1, 0);
		m_samples->stop(SND_LAUNCH0);
	}

	if (bitsGoneLow & S97271P_EXPL_L)
	{
		m_samples->start(SND_EXPL_L0, SND_EXPL_L0, 1);
		m_samples->stop(SND_EXPL_L1);
	} else if (bitsGoneHigh & S97271P_EXPL_L)
	{
		m_samples->start(SND_EXPL_L1, SND_EXPL_L1, 0);
		m_samples->stop(SND_EXPL_L0);
	}

	if (bitsGoneLow & S97271P_EXPL_S)
	{
		m_samples->start(SND_EXPL_S0, SND_EXPL_S0, 1);
		m_samples->stop(SND_EXPL_S1);
	} else if (bitsGoneHigh & S97271P_EXPL_S)
	{
		m_samples->start(SND_EXPL_S1, SND_EXPL_S1, 0);
		m_samples->stop(SND_EXPL_S0);
	}

	if (bitsGoneLow & S97271P_BONUS)
	{
		m_samples->start(SND_BONUS0, SND_BONUS0, 1);
		m_samples->stop(SND_BONUS1);
	} else if (bitsGoneHigh & S97271P_BONUS)
	{
		m_samples->start(SND_BONUS1, SND_BONUS1, 0);
		m_samples->stop(SND_BONUS0);
	}

	if (bitsGoneLow & S97271P_CODE)
	{
		m_samples->start(SND_CODE, SND_CODE, 1);
	} else if (bitsGoneHigh & S97271P_CODE)
	{
		m_samples->stop(SND_CODE);
	}

	if (bitsGoneLow & S97271P_BOAT)
	{
		m_samples->start(SND_BOAT, SND_BOAT, 1);
	} else if (bitsGoneHigh & S97271P_BOAT)
	{
		m_samples->stop(SND_BOAT);
	}
}
