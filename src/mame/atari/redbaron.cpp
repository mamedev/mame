// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Nicola Salmoria
/*************************************************************************

    Atari Red Baron sound hardware

    TODO:
    - convert to netlist

*************************************************************************/
/*

    Red Baron sound notes:
    bit function
    7-4 explosion volume
    3   start led
    2   shot (machine gun sound)
    1   squeal (nosedive sound)
    0   POTSEL (rb_input_select)
*/

#include "emu.h"
#include "redbaron.h"

#include <algorithm>


#define OUTPUT_RATE     (48000)


// device type definition
DEFINE_DEVICE_TYPE(REDBARON, redbaron_sound_device, "redbaron_custom", "Red Baron Custom Sound")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  redbaron_sound_device - constructor
//-------------------------------------------------

redbaron_sound_device::redbaron_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, REDBARON, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_vol_lookup(nullptr),
	m_channel(nullptr),
	m_latch(0),
	m_poly_counter(0),
	m_poly_shift(0),
	m_filter_counter(0),
	m_crash_amp(0),
	m_shot_amp(0),
	m_shot_amp_counter(0),
	m_squeal_amp(0),
	m_squeal_amp_counter(0),
	m_squeal_off_counter(0),
	m_squeal_on_counter(0),
	m_squeal_out(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void redbaron_sound_device::device_start()
{
	m_vol_lookup = std::make_unique<int16_t[]>(32768);
	for( int i = 0; i < 0x8000; i++ )
		m_vol_lookup[0x7fff-i] = int16_t(0x7fff/exp(1.0*i/4096));

	for( int i = 0; i < 16; i++ )
	{
		/* r0 = R18 and R24, r1 = open */
		double r0 = 1.0/(5600 + 680), r1 = 1/6e12;

		/* R14 */
		if( i & 1 )
			r1 += 1.0/8200;
		else
			r0 += 1.0/8200;
		/* R15 */
		if( i & 2 )
			r1 += 1.0/3900;
		else
			r0 += 1.0/3900;
		/* R16 */
		if( i & 4 )
			r1 += 1.0/2200;
		else
			r0 += 1.0/2200;
		/* R17 */
		if( i & 8 )
			r1 += 1.0/1000;
		else
			r0 += 1.0/1000;
		r0 = 1.0/r0;
		r1 = 1.0/r1;
		m_vol_crash[i] = 32767 * r0 / (r0 + r1);
	}

	m_channel = stream_alloc(0, 1, OUTPUT_RATE);

	save_item(NAME(m_latch));
	save_item(NAME(m_poly_counter));
	save_item(NAME(m_poly_shift));
	save_item(NAME(m_filter_counter));
	save_item(NAME(m_crash_amp));
	save_item(NAME(m_shot_amp));
	save_item(NAME(m_shot_amp_counter));
	save_item(NAME(m_squeal_amp));
	save_item(NAME(m_squeal_amp_counter));
	save_item(NAME(m_squeal_off_counter));
	save_item(NAME(m_squeal_on_counter));
	save_item(NAME(m_squeal_out));
}



//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void redbaron_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	auto &buffer = outputs[0];
	for (int sampindex = 0; sampindex < buffer.samples(); sampindex++)
	{
		int sum = 0;

		/* polynomial shifter E5 and F4 (LS164) clocked with 12kHz */
		m_poly_counter -= 12000;
		while( m_poly_counter <= 0 )
		{
			m_poly_counter += OUTPUT_RATE;
			if( BIT(m_poly_shift, 0) == BIT(m_poly_shift, 14) )
				m_poly_shift = (m_poly_shift << 1) | 1;
			else
				m_poly_shift <<= 1;
		}

		/* What is the exact low pass filter frequency? */
		m_filter_counter -= 330;
		while( m_filter_counter <= 0 )
		{
			m_filter_counter += OUTPUT_RATE;
			m_crash_amp = (m_poly_shift & 1) ? m_latch >> 4 : 0;
		}
		/* mix crash sound at 35% */
		sum += m_vol_crash[m_crash_amp] * 35 / 100;

		/* shot not active: charge C32 (0.1u) */
		if( (m_latch & 0x04) == 0 )
			m_shot_amp = 32767;
		else if( (m_poly_shift & 0x8000) == 0 )
		{
			if( m_shot_amp > 0 )
			{
				/* discharge C32 (0.1u) through R26 (33k) + R27 (15k)
				 * 0.68 * C32 * (R26 + R27) = 3264us
				 */
				//constexpr int C32_DISCHARGE_TIME = int(32767 / 0.003264);
				/* I think this is to short. Is C32 really 1u? */
				constexpr int C32_DISCHARGE_TIME = int(32767 / 0.03264);
				m_shot_amp_counter -= C32_DISCHARGE_TIME;
				while( m_shot_amp_counter <= 0 )
				{
					m_shot_amp_counter += OUTPUT_RATE;
					if( --m_shot_amp == 0 )
						break;
				}
				/* mix shot sound at 35% */
				sum += m_vol_lookup[m_shot_amp] * 35 / 100;
			}
		}


		if( (m_latch & 0x02) == 0 )
			m_squeal_amp = 0;
		else
		{
			if( m_squeal_amp < 32767 )
			{
				/* charge C5 (22u) over R3 (68k) and CR1 (1N914)
				 * time = 0.68 * C5 * R3 = 1017280us
				 */
				constexpr int C5_CHARGE_TIME = int(32767 / 1.01728);
				m_squeal_amp_counter -= C5_CHARGE_TIME;
				while( m_squeal_amp_counter <= 0 )
				{
					m_squeal_amp_counter += OUTPUT_RATE;
					if( ++m_squeal_amp == 32767 )
						break;
				}
			}

			if( m_squeal_out )
			{
				/* NE555 setup as pulse position modulator
				 * C = 0.01u, Ra = 33k, Rb = 47k
				 * frequency = 1.44 / ((33k + 2*47k) * 0.01u) = 1134Hz
				 * modulated by squeal_amp
				 */
				m_squeal_off_counter -= 2268 - 1134 * m_squeal_amp / 32767;
				while( m_squeal_off_counter <= 0 )
				{
					m_squeal_off_counter += OUTPUT_RATE;
					m_squeal_out = 0;
				}
			}
			else
			{
				m_squeal_on_counter -= 1134;
				while( m_squeal_on_counter <= 0 )
				{
					m_squeal_on_counter += OUTPUT_RATE;
					m_squeal_out = 1;
				}
			}
		}

		/* mix squeal sound at 40% */
		if( m_squeal_out )
			sum += 32767 * 40 / 100;

		buffer.put_int(sampindex, sum, 32768);
	}
}


void redbaron_sound_device::sounds_w(uint8_t data)
{
	/* If sound is off, don't bother playing samples */
	if( data == m_latch )
		return;

	m_channel->update();
	m_latch = data;
}
