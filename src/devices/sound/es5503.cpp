// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*

  ES5503 - Ensoniq ES5503 "DOC" emulator v2.4
  By R. Belmont.

  Copyright R. Belmont.

  History: the ES5503 was the next design after the famous C64 "SID" by Bob Yannes.
  It powered the legendary Mirage sampler (the first affordable pro sampler) as well
  as the ESQ-1 synth/sequencer.  The ES5505 (used in Taito's F3 System) and 5506
  (used in the "Soundscape" series of ISA PC sound cards) followed on a fundamentally
  similar architecture.

  Bugs: On the real silicon, the uppermost enabled oscillator contributes to the output 3 times.
        This is likely why the Apple IIgs system software doesn't let you use oscillators 30 and 31.

  Additionally, in "swap" mode, there's one cycle when the switch takes place where the
  oscillator's output is 0x80 (centerline) regardless of the sample data.  This can
  cause audible clicks and a general degradation of audio quality if the correct sample
  data at that point isn't 0x80 or very near it.

  Changes:
  0.2 (RB)   - improved behavior for volumes > 127, fixes missing notes in Nucleus & missing voices in Thexder
  0.3 (RB)   - fixed extraneous clicking, improved timing behavior for e.g. Music Construction Set & Music Studio
  0.4 (RB)   - major fixes to IRQ semantics and end-of-sample handling.
  0.5 (RB)   - more flexible wave memory hookup (incl. banking) and save state support.
  1.0 (RB)   - properly respects the input clock
  2.0 (RB)   - C++ conversion, more accurate oscillator IRQ timing
  2.1 (RB)   - Corrected phase when looping; synthLAB, Arkanoid, and Arkanoid II no longer go out of tune
  2.1.1 (RB) - Fixed issue introduced in 2.0 where IRQs were delayed
  2.1.2 (RB) - Fixed SoundSmith POLY.SYNTH inst where one-shot on the even oscillator and swap on the odd should loop.
               Conversely, the intro voice in FTA Delta Demo has swap on the even and one-shot on the odd and doesn't
               want to loop.
  2.1.3 (RB) - Fixed oscillator enable register off-by-1 which caused everything to be half a step sharp.
  2.2 (RB)   - More precise one-shot even/swap odd behavior from hardware observations with Ian Brumby's SWAPTEST.
  2.3 (RB)   - Sync & AM modes added, emulate the volume glitch for the highest-numbered enabled oscillator.
  2.3.1 (RB) - Fixed thinko in the volume glitch emulation and minor cleanup.
  2.4 (RB)   - Halting an oscillator from the CPU behaves the same as halting it from the DOC itself.
               Skate or Die on the IIgs accidentally relies on this behavior.
  2.4.1 (RB) - Fixed the phase preservation on looping samples, which was introducing a subtle (1 sample position)
               pitch error on each loop.
*/

#include "emu.h"
#include "es5503.h"

// device type definition
DEFINE_DEVICE_TYPE(ES5503, es5503_device, "es5503", "Ensoniq ES5503")

// useful constants
static constexpr uint16_t wavesizes[8] = { 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };
static constexpr uint32_t wavemasks[8] = { 0x1ff00, 0x1fe00, 0x1fc00, 0x1f800, 0x1f000, 0x1e000, 0x1c000, 0x18000 };
static constexpr uint32_t accmasks[8]  = { 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff };
static constexpr int      resshifts[8] = { 9, 10, 11, 12, 13, 14, 15, 16 };

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  es5503_device - constructor
//-------------------------------------------------

es5503_device::es5503_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ES5503, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_rom_interface(mconfig, *this),
	m_irq_func(*this),
	m_adc_func(*this, 0)
{
}


//-------------------------------------------------
//  delayed_stream_update -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(es5503_device::delayed_stream_update)
{
	m_stream->update();
}

//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void es5503_device::rom_bank_pre_change()
{
	m_stream->update();
}

// halt_osc: handle halting an oscillator
// onum = oscillator #
// type = 1 for 0 found in sample data, 0 for hit end of table size
void es5503_device::halt_osc(int onum, int type, uint32_t *accumulator, int resshift)
{
	ES5503Osc *pOsc = &m_oscillators[onum];
	ES5503Osc *pPartner = &m_oscillators[onum^1];
	int mode = (pOsc->control>>1) & 3;
	const int partnerMode = (pPartner->control>>1) & 3;

	// check for sync mode
	if (mode == MODE_SYNCAM)
	{
		if (!(onum & 1))
		{
			// we're even, so if the odd oscillator 1 below us is playing,
			// restart it.
			if (!(m_oscillators[onum - 1].control & 1))
			{
				m_oscillators[onum - 1].accumulator = 0;
			}
		}

		// loop this oscillator for both sync and AM
		mode = MODE_FREE;
	}

	// if 0 found in sample data or mode is not free-run, halt this oscillator
	if ((mode != MODE_FREE) || (type != 0))
	{
		pOsc->control |= 1;
	}
	else    // preserve the relative phase of the oscillator when looping
	{
		const uint16_t wtsize = pOsc->wtsize;
		// For the ideal case, the accumulator is greater than or equal to the wave table size.
		// Unfortunately degenerate cases can occur on this chip (especially with SoundSmith on
		// the IIgs), so in those cases just zero the integer part of the accumulator.)
		if ((*accumulator >> resshift) < wtsize)
		{
			*accumulator -= ((*accumulator >> resshift) << resshift);
		}
		else    // Ideal case.  Just subtract the wave table size from the integer part.
		{
			*accumulator -= (wtsize << resshift);
		}
	}

	// if we're in swap mode, start the partner
	if (mode == MODE_SWAP)
	{
		pPartner->control &= ~1;    // clear the halt bit
		pPartner->accumulator = 0;  // and make sure it starts from the top (does this also need phase preservation?)
	}
	else
	{
		// if we're the even oscillator of the pair and the partner's swap
		// but we aren't, we retrigger (!!!)  Verified on IIgs hardware, and hinted by
		// the Ensoniq data sheet.
		if ((partnerMode == MODE_SWAP) && ((onum & 1)==0))
		{
			pOsc->control &= ~1;

			// preserve the phase in this case too
			uint16_t wtsize = pOsc->wtsize - 1;
			*accumulator -= (wtsize << resshift);
		}
	}
	// IRQ enabled for this voice?
	if (pOsc->control & 0x08)
	{
		pOsc->irqpend = 1;

		m_irq_func(1);
	}
}
void es5503_device::sound_stream_update(sound_stream &stream)
{
	int32_t *mixp;
	int osc, snum, i;
	uint32_t ramptr;
	int samples = stream.samples();

	assert(samples < (44100/50));
	std::fill_n(&m_mix_buffer[0], samples * m_output_channels, 0);

	for (int chan = 0; chan < m_output_channels; chan++)
	{
		for (osc = 0; osc < m_oscsenabled; osc++)
		{
			ES5503Osc *pOsc = &m_oscillators[osc];

			if (!(pOsc->control & 1) && ((pOsc->control >> 4) & (m_output_channels - 1)) == chan)
			{
				uint32_t wtptr = pOsc->wavetblpointer & wavemasks[pOsc->wavetblsize], altram;
				uint32_t acc = pOsc->accumulator;
				const uint16_t wtsize = pOsc->wtsize - 1;
				uint8_t ctrl = pOsc->control;
				const uint16_t freq = pOsc->freq;
				int16_t vol = pOsc->vol;
				int8_t data = -128;
				const int resshift = resshifts[pOsc->resolution] - pOsc->wavetblsize;
				const uint32_t sizemask = accmasks[pOsc->wavetblsize];
				const int mode = (pOsc->control>>1) & 3;
				mixp = &m_mix_buffer[0] + chan;

				for (snum = 0; snum < samples; snum++)
				{
					altram = acc >> resshift;
					ramptr = altram & sizemask;

					acc += freq;

					// channel strobe is always valid when reading; this allows potentially banking per voice
					m_channel_strobe = (ctrl>>4) & 0xf;
					data = (int32_t)read_byte(ramptr + wtptr) ^ 0x80;

					if (read_byte(ramptr + wtptr) == 0x00)
					{
						halt_osc(osc, 1, &acc, resshift);
					}
					else
					{
						if (mode != MODE_SYNCAM)
						{
							*mixp += data * vol;
							if (osc == (m_oscsenabled - 1))
							{
								*mixp += data * vol;
								*mixp += data * vol;
							}
						}
						else
						{
							// if we're odd, we play nothing ourselves
							if (osc & 1)
							{
								if (osc < 31)
								{
									// if the next oscillator up is playing, it's volume becomes our control
									if (!(m_oscillators[osc + 1].control & 1))
									{
										m_oscillators[osc + 1].vol = data ^ 0x80;
									}
								}
							}
							else    // hard sync, both oscillators play?
							{
								*mixp += data * vol;
								if (osc == (m_oscsenabled - 1))
								{
									*mixp += data * vol;
									*mixp += data * vol;
								}
							}
						}
						mixp += m_output_channels;

						if (altram >= wtsize)
						{
							halt_osc(osc, 0, &acc, resshift);
						}
					}

					// if oscillator halted, we've got no more samples to generate
					if (pOsc->control & 1)
					{
						ctrl |= 1;
						break;
					}
				}

				pOsc->control = ctrl;
				pOsc->accumulator = acc;
				pOsc->data = data ^ 0x80;
			}
		}
	}
	mixp = &m_mix_buffer[0];
	for (int chan = 0; chan < m_output_channels; chan++)
	{
		for (i = 0; i < stream.samples(); i++)
		{
			stream.put_int(chan, i, *mixp++, 32768*8);
		}
	}
}


void es5503_device::device_start()
{
	m_rege0 = 0xff;

	save_pointer(STRUCT_MEMBER(m_oscillators, freq), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, wtsize), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, control), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, vol), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, data), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, wavetblpointer), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, wavetblsize), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, resolution), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, accumulator), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, irqpend), 32);

	m_oscsenabled = 1;
	m_output_rate = (clock() / 8) / (m_oscsenabled + 2);
	m_stream = stream_alloc(0, m_output_channels, m_output_rate);

	m_timer = timer_alloc(FUNC(es5503_device::delayed_stream_update), this);
}

void es5503_device::device_clock_changed()
{
	m_output_rate = (clock() / 8) / (m_oscsenabled + 2);
	m_stream->set_sample_rate(m_output_rate);

	m_mix_buffer.resize((m_output_rate/50)*8);

	attotime update_rate = m_output_rate ? attotime::from_hz(m_output_rate) : attotime::never;
	m_timer->adjust(update_rate, 0, update_rate);
}

void es5503_device::device_reset()
{
	m_rege0 = 0xff;

	for (auto & elem : m_oscillators)
	{
		elem.freq = 0;
		elem.wtsize = 0;
		elem.control = 0;
		elem.vol = 0;
		elem.data = 0x80;
		elem.wavetblpointer = 0;
		elem.wavetblsize = 0;
		elem.resolution = 0;
		elem.accumulator = 0;
		elem.irqpend = 0;
	}

	m_oscsenabled = 1;
	notify_clock_changed();

	m_channel_strobe = 0;
}

u8 es5503_device::read(offs_t offset)
{
	uint8_t retval;
	int i;

	m_stream->update();

	if (offset < 0xe0)
	{
		int osc = offset & 0x1f;

		switch(offset & 0xe0)
		{
			case 0:     // freq lo
				return (m_oscillators[osc].freq & 0xff);

			case 0x20:      // freq hi
				return (m_oscillators[osc].freq >> 8);

			case 0x40:  // volume
				return m_oscillators[osc].vol;

			case 0x60:  // data
				return m_oscillators[osc].data;

			case 0x80:  // wavetable pointer
				return (m_oscillators[osc].wavetblpointer>>8) & 0xff;

			case 0xa0:  // oscillator control
				return m_oscillators[osc].control;

			case 0xc0:  // bank select / wavetable size / resolution
				retval = 0;
				if (m_oscillators[osc].wavetblpointer & 0x10000)
				{
					retval |= 0x40;
				}

				retval |= (m_oscillators[osc].wavetblsize<<3);
				retval |= m_oscillators[osc].resolution;
				return retval;
		}
	}
	else     // global registers
	{
		switch (offset)
		{
			case 0xe0:  // interrupt status
				retval = m_rege0;

				m_irq_func(0);

				// scan all oscillators
				for (i = 0; i < m_oscsenabled; i++)
				{
					if (m_oscillators[i].irqpend)
					{
						// signal this oscillator has an interrupt
						retval = i<<1;

						m_rege0 = retval | 0x80;

						// and clear its flag
						m_oscillators[i].irqpend = 0;
						break;
					}
				}

				// if any oscillators still need to be serviced, assert IRQ again immediately
				for (i = 0; i < m_oscsenabled; i++)
				{
					if (m_oscillators[i].irqpend)
					{
						m_irq_func(1);
						break;
					}
				}

				return retval | 0x41;

			case 0xe1:  // oscillator enable
				return (m_oscsenabled - 1) << 1;

			case 0xe2:  // A/D converter
				return m_adc_func();
		}
	}

	return 0;
}

void es5503_device::write(offs_t offset, u8 data)
{
	m_stream->update();

	if (offset < 0xe0)
	{
		int osc = offset & 0x1f;

		switch(offset & 0xe0)
		{
			case 0:     // freq lo
				m_oscillators[osc].freq &= 0xff00;
				m_oscillators[osc].freq |= data;
				break;

			case 0x20:      // freq hi
				m_oscillators[osc].freq &= 0x00ff;
				m_oscillators[osc].freq |= (data<<8);
				break;

			case 0x40:  // volume
				m_oscillators[osc].vol = data;
				break;

			case 0x60:  // data - ignore writes
				break;

			case 0x80:  // wavetable pointer
				m_oscillators[osc].wavetblpointer = (data<<8);
				break;

			case 0xa0:  // oscillator control
				// key on?
				if ((m_oscillators[osc].control & 1) && (!(data&1)))
				{
					m_oscillators[osc].accumulator = 0;
				}

				// The Ensoniq data sheet says that if the low bit of the mode is set,
				// then halting either internally or from the CPU will reset the oscillator.
				// In practice, this means in swap mode that we will also do the swap.
				if (!(m_oscillators[osc].control & 1) && ((data & 1)) && ((data >> 1) & 1))
				{
					halt_osc(osc, 0, &m_oscillators[osc].accumulator, resshifts[m_oscillators[osc].resolution]);
				}
				m_oscillators[osc].control = data;
				break;

			case 0xc0:           // bank select / wavetable size / resolution
				if (data & 0x40) // bank select, effectively A16 for external addressing
				{
					m_oscillators[osc].wavetblpointer |= 0x10000;
				}
				else
				{
					m_oscillators[osc].wavetblpointer &= 0xffff;
				}

				m_oscillators[osc].wavetblsize = ((data >> 3) & 7);
				m_oscillators[osc].wtsize = wavesizes[m_oscillators[osc].wavetblsize];
				m_oscillators[osc].resolution = (data & 7);
				break;
		}
	}
	else     // global registers
	{
		switch (offset)
		{
			case 0xe0:  // interrupt status
				break;

			case 0xe1:  // oscillator enable
				// The number here is the number of oscillators to enable -1 times 2.  You can never
				// have zero oscilllators enabled.  So a value of 62 enables all 32 oscillators.
				m_oscsenabled = ((data>>1) & 0x1f) + 1;
				notify_clock_changed();
				break;

			case 0xe2:  // A/D converter
				break;
		}
	}
}
