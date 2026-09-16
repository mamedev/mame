// license:BSD-3-Clause
// copyright-holders:David Haywood, James Wallace

/*

Universal sampled sound program card PCB 683077
Sampled sound card, using a PIA and PTM for timing and data handling
The sound hardware is technically twin channel stereo, but there's no evidence to suggest it was ever connected as such
I've separated the channels here, to tie back at the game level

*/

#include "emu.h"

#include "mpu4_oki_sampled_sound.h"

//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MPU4_OKI_SAMPLED_SOUND, mpu4_oki_sampled_sound, "mpu4okisnd", "Barcrest Sampled Sound Program Card")

mpu4_oki_sampled_sound::mpu4_oki_sampled_sound(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MPU4_OKI_SAMPLED_SOUND, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_cb2_handler(*this),
	m_ptm_ic3ss(*this, "ptm_ic3ss"),
	m_pia_ic4ss(*this, "pia_ic4ss"),
	m_msm6376(*this, "msm6376"),
	m_expansion_latch(0),
	m_global_volume(0),
	m_t1(0),
	m_t3l(0),
	m_t3h(0),
	m_last_reset(0)
{
}

/*
The MSM6376 sound chip is configured in a slightly strange way, to enable dynamic
sample rate changes (8Khz, 10.6 Khz, 16 KHz) by varying the clock.
According to the BwB programmer's guide, the formula is:
MSM6376 clock frequency:-
freq = (CPU_CLOCK/((t3L+1)(t3H+1)))*[(t3H(T3L+1)+1)/(2(t1+1))]
where [] means rounded up integer,
t3L is the LSB of Clock 3,
t3H is the MSB of Clock 3,
and t1 is the initial value in clock 1.

As originally made, this is the peripheral clock (1.72 MHz), but for video it's the E Clock of the 68k (1MHz)

//O3 -> G1  O1 -> c2 o2 -> c1

This is a bit of a cheat - since we don't clock into the OKI chip directly, we need to
calculate the oscillation frequency in advance. We're running the timer for interrupt
purposes, but the frequency calculation is done by plucking the values out as they are written.
*/

void mpu4_oki_sampled_sound::ic3_write(offs_t offset, uint8_t data)
{
	m_ptm_ic3ss->write(offset, data);

	if (offset == 3)
	{
		m_t1 = data;
	}
	if (offset == 6)
	{
		m_t3h = data;
	}
	if (offset == 7)
	{
		m_t3l = data;
	}

	float const num = float(clock()) / ((m_t3l + 1) * (m_t3h + 1));
	float const denom = std::ceil(float(m_t3h * (m_t3l + 1) + 1) / (2 * (m_t1 + 1))); //need to round up, this gives same precision as chip

	int const freq = int(num * denom);

	if (freq)
	{
		m_msm6376->set_unscaled_clock(freq);
	}
}

uint8_t mpu4_oki_sampled_sound::ic3_read(offs_t offset)
{
	return m_ptm_ic3ss->read(offset);
}

void mpu4_oki_sampled_sound::ic4_write(offs_t offset, uint8_t data)
{
	m_pia_ic4ss->write(offset, data);
}

uint8_t mpu4_oki_sampled_sound::ic4_read(offs_t offset)
{
	return m_pia_ic4ss->read(offset);
}

void mpu4_oki_sampled_sound::pia_gb_porta_w(uint8_t data)
{
	LOG("PIA Port A Set to %2x\n", data);
	m_msm6376->write(data);
}

void mpu4_oki_sampled_sound::pia_gb_portb_w(uint8_t data)
{
	uint8_t changed = m_expansion_latch^data;

	LOG("PIA Port B Set to %2x\n", data);

	if (changed & 0x20)
	{ // digital volume clock line changed
		if (!(data & 0x20))
		{ // changed from high to low,
			if (!(data & 0x10)) // down
			{
				if (m_global_volume < 32) m_global_volume++; //steps unknown
			}
			else // up
			{
				if (m_global_volume > 0) m_global_volume--;
			}

			LOG("Volume Set to %2x\n", data);
			float percent = (32-m_global_volume)/32.0;
			m_msm6376->set_output_gain(0, percent);
		}
	}
	m_msm6376->ch2_w(data & 0x02);
	m_msm6376->st_w(data & 0x01);
}

uint8_t mpu4_oki_sampled_sound::pia_gb_portb_r()
{
	LOG("PIA Read of Port B\n", machine().describe_context());
	uint8_t data = 0;
	// b7 NAR - we can load another address into Channel 1
	// b6, 1 = OKI ready, 0 = OKI busy
	// b5, vol clock
	// b4, 1 = Vol down, 0 = Vol up
	//

	if (m_msm6376->nar_r()) data |= 0x80;
	else                    data &= ~0x80;

	if (m_msm6376->busy_r()) data |= 0x40;
	else                     data &= ~0x40;

	return data | m_expansion_latch;
}

void mpu4_oki_sampled_sound::pia_gb_ca2_w(int state)
{
	LOG("OKI RESET data = %02X\n", state);
	if ((m_last_reset != state) && !state)
	{
		m_msm6376->reset();
	}
	m_last_reset = state;
//  reset line
}



void mpu4_oki_sampled_sound::pia_gb_cb2_w(int state)
{
	m_cb2_handler(state);
}


void mpu4_oki_sampled_sound::device_add_mconfig(machine_config &config)
{
	PTM6840(config, m_ptm_ic3ss, clock());
	m_ptm_ic3ss->set_external_clocks(0, 0, 0);
	m_ptm_ic3ss->o1_callback().set("ptm_ic3ss", FUNC(ptm6840_device::set_c2));
	m_ptm_ic3ss->o2_callback().set("ptm_ic3ss", FUNC(ptm6840_device::set_c1));
	m_ptm_ic3ss->o3_callback().set("ptm_ic3ss", FUNC(ptm6840_device::set_g1));

	PIA6821(config, m_pia_ic4ss);
	m_pia_ic4ss->readpb_handler().set(FUNC(mpu4_oki_sampled_sound::pia_gb_portb_r));
	m_pia_ic4ss->writepa_handler().set(FUNC(mpu4_oki_sampled_sound::pia_gb_porta_w));
	m_pia_ic4ss->writepb_handler().set(FUNC(mpu4_oki_sampled_sound::pia_gb_portb_w));
	m_pia_ic4ss->ca2_handler().set(FUNC(mpu4_oki_sampled_sound::pia_gb_ca2_w));
	m_pia_ic4ss->cb2_handler().set(FUNC(mpu4_oki_sampled_sound::pia_gb_cb2_w));

	OKIM6376(config, m_msm6376, 128000);     //Adjusted by IC3, default to 16KHz sample. Can also be 85430 at 10.5KHz and 64000 at 8KHz
	m_msm6376->add_route(ALL_OUTPUTS, *this, 1.0);
}


void mpu4_oki_sampled_sound::device_start()
{
	save_item(NAME(m_expansion_latch));
	save_item(NAME(m_global_volume));
	save_item(NAME(m_t1));
	save_item(NAME(m_t3l));
	save_item(NAME(m_t3h));
	save_item(NAME(m_last_reset));
}
