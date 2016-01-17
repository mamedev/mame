// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*

  ES5503 - Ensoniq ES5503 "DOC" emulator v2.1.1
  By R. Belmont.

  Copyright R. Belmont.

  History: the ES5503 was the next design after the famous C64 "SID" by Bob Yannes.
  It powered the legendary Mirage sampler (the first affordable pro sampler) as well
  as the ESQ-1 synth/sequencer.  The ES5505 (used in Taito's F3 System) and 5506
  (used in the "Soundscape" series of ISA PC sound cards) followed on a fundamentally
  similar architecture.

  Bugs: On the real silicon, oscillators 30 and 31 have random volume fluctuations and are
  unusable for playback.  We don't attempt to emulate that. :-)

  Additionally, in "swap" mode, there's one cycle when the switch takes place where the
  oscillator's output is 0x80 (centerline) regardless of the sample data.  This can
  cause audible clicks and a general degradation of audio quality if the correct sample
  data at that point isn't 0x80 or very near it.

  Changes:
  0.2 (RB) - improved behavior for volumes > 127, fixes missing notes in Nucleus & missing voices in Thexder
  0.3 (RB) - fixed extraneous clicking, improved timing behavior for e.g. Music Construction Set & Music Studio
  0.4 (RB) - major fixes to IRQ semantics and end-of-sample handling.
  0.5 (RB) - more flexible wave memory hookup (incl. banking) and save state support.
  1.0 (RB) - properly respects the input clock
  2.0 (RB) - C++ conversion, more accurate oscillator IRQ timing
  2.1 (RB) - Corrected phase when looping; synthLAB, Arkanoid, and Arkanoid II no longer go out of tune
  2.1.1 (RB) - Fixed issue introduced in 2.0 where IRQs were delayed
*/

#include "emu.h"
#include "es5503.h"

// device type definition
const device_type ES5503 = &device_creator<es5503_device>;

// useful constants
static const UINT16 wavesizes[8] = { 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };
static const UINT32 wavemasks[8] = { 0x1ff00, 0x1fe00, 0x1fc00, 0x1f800, 0x1f000, 0x1e000, 0x1c000, 0x18000 };
static const UINT32 accmasks[8]  = { 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff };
static const int    resshifts[8] = { 9, 10, 11, 12, 13, 14, 15, 16 };

// default address map
static ADDRESS_MAP_START( es5503, AS_0, 8, es5503_device )
	AM_RANGE(0x000000, 0x1ffff) AM_ROM
ADDRESS_MAP_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  es5503_device - constructor
//-------------------------------------------------

es5503_device::es5503_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ES5503, "Ensoniq ES5503", tag, owner, clock, "es5503", __FILE__),
		device_sound_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		m_space_config("es5503_samples", ENDIANNESS_LITTLE, 8, 17, 0, nullptr, *ADDRESS_MAP_NAME(es5503)),
		m_irq_func(*this),
		m_adc_func(*this)
{
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *es5503_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : nullptr;
}

//-------------------------------------------------
//  static_set_type - configuration helper to set
//  the IRQ callback
//-------------------------------------------------

void es5503_device::static_set_channels(device_t &device, int channels)
{
	es5503_device &es5503 = downcast<es5503_device &>(device);
	es5503.output_channels = channels;
}

//-------------------------------------------------
//  device_timer - called when our device timer expires
//-------------------------------------------------

void es5503_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	m_stream->update();
}

// halt_osc: handle halting an oscillator
// chip = chip ptr
// onum = oscillator #
// type = 1 for 0 found in sample data, 0 for hit end of table size
void es5503_device::halt_osc(int onum, int type, UINT32 *accumulator, int resshift)
{
	ES5503Osc *pOsc = &oscillators[onum];
	ES5503Osc *pPartner = &oscillators[onum^1];
	int mode = (pOsc->control>>1) & 3;

	// if 0 found in sample data or mode is not free-run, halt this oscillator
	if ((mode != MODE_FREE) || (type != 0))
	{
		pOsc->control |= 1;
	}
	else    // preserve the relative phase of the oscillator when looping
	{
		UINT16 wtsize = pOsc->wtsize - 1;
		UINT32 altram = (*accumulator) >> resshift;

		if (altram > wtsize)
		{
			altram -= wtsize;
		}
		else
		{
			altram = 0;
		}

		*accumulator = altram << resshift;
	}
	int omode = (pPartner->control>>1) & 3;

	// if swap mode, start the partner
	if ((mode == MODE_SWAP) || (omode == MODE_SWAP))
	{
		pPartner->control &= ~1;    // clear the halt bit
		pPartner->accumulator = 0;  // and make sure it starts from the top (does this also need phase preservation?)
	}

	// IRQ enabled for this voice?
	if (pOsc->control & 0x08)
	{
		pOsc->irqpend = 1;

		m_irq_func(1);
	}
}

void es5503_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	static INT32 mix[(44100/60)*2*8];
	INT32 *mixp;
	int osc, snum, i;
	UINT32 ramptr;

	assert(samples < (44100/60)*2);
	memset(mix, 0, sizeof(mix));

	for (int chan = 0; chan < output_channels; chan++)
	{
		for (osc = 0; osc < (oscsenabled+1); osc++)
		{
			ES5503Osc *pOsc = &oscillators[osc];

			if (!(pOsc->control & 1) && ((pOsc->control >> 4) & (output_channels - 1)) == chan)
			{
				UINT32 wtptr = pOsc->wavetblpointer & wavemasks[pOsc->wavetblsize], altram;
				UINT32 acc = pOsc->accumulator;
				UINT16 wtsize = pOsc->wtsize - 1;
				UINT8 ctrl = pOsc->control;
				UINT16 freq = pOsc->freq;
				INT16 vol = pOsc->vol;
				INT8 data = -128;
				int resshift = resshifts[pOsc->resolution] - pOsc->wavetblsize;
				UINT32 sizemask = accmasks[pOsc->wavetblsize];
				mixp = &mix[0] + chan;

				for (snum = 0; snum < samples; snum++)
				{
					altram = acc >> resshift;
					ramptr = altram & sizemask;

					acc += freq;

					// channel strobe is always valid when reading; this allows potentially banking per voice
					m_channel_strobe = (ctrl>>4) & 0xf;
					data = (INT32)m_direct->read_byte(ramptr + wtptr) ^ 0x80;

					if (m_direct->read_byte(ramptr + wtptr) == 0x00)
					{
						halt_osc(osc, 1, &acc, resshift);
					}
					else
					{
						*mixp += data * vol;
						mixp += output_channels;

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

	mixp = &mix[0];
	for (i = 0; i < samples; i++)
		for (int chan = 0; chan < output_channels; chan++)
			outputs[chan][i] = (*mixp++)>>1;
}


void es5503_device::device_start()
{
	int osc;

	// find our direct access
	m_direct = &space().direct();

	m_irq_func.resolve_safe();
	m_adc_func.resolve_safe(0);

	rege0 = 0xff;

	for (osc = 0; osc < 32; osc++)
	{
		save_item(NAME(oscillators[osc].freq), osc);
		save_item(NAME(oscillators[osc].wtsize), osc);
		save_item(NAME(oscillators[osc].control), osc);
		save_item(NAME(oscillators[osc].vol), osc);
		save_item(NAME(oscillators[osc].data), osc);
		save_item(NAME(oscillators[osc].wavetblpointer), osc);
		save_item(NAME(oscillators[osc].wavetblsize), osc);
		save_item(NAME(oscillators[osc].resolution), osc);
		save_item(NAME(oscillators[osc].accumulator), osc);
		save_item(NAME(oscillators[osc].irqpend), osc);
	}

	output_rate = (clock()/8)/34;   // (input clock / 8) / # of oscs. enabled + 2
	m_stream = machine().sound().stream_alloc(*this, 0, output_channels, output_rate);

	m_timer = timer_alloc(0, nullptr);
	m_timer->adjust(attotime::from_hz(output_rate), 0, attotime::from_hz(output_rate));
}

void es5503_device::device_reset()
{
	rege0 = 0xff;

	for (auto & elem : oscillators)
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

	oscsenabled = 1;

	m_channel_strobe = 0;

	output_rate = (clock()/8)/34;   // (input clock / 8) / # of oscs. enabled + 2
}

READ8_MEMBER( es5503_device::read )
{
	UINT8 retval;
	int i;

	m_stream->update();

	if (offset < 0xe0)
	{
		int osc = offset & 0x1f;

		switch(offset & 0xe0)
		{
			case 0:     // freq lo
				return (oscillators[osc].freq & 0xff);

			case 0x20:      // freq hi
				return (oscillators[osc].freq >> 8);

			case 0x40:  // volume
				return oscillators[osc].vol;

			case 0x60:  // data
				return oscillators[osc].data;

			case 0x80:  // wavetable pointer
				return (oscillators[osc].wavetblpointer>>8) & 0xff;

			case 0xa0:  // oscillator control
				return oscillators[osc].control;

			case 0xc0:  // bank select / wavetable size / resolution
				retval = 0;
				if (oscillators[osc].wavetblpointer & 0x10000)
				{
					retval |= 0x40;
				}

				retval |= (oscillators[osc].wavetblsize<<3);
				retval |= oscillators[osc].resolution;
				return retval;
		}
	}
	else     // global registers
	{
		switch (offset)
		{
			case 0xe0:  // interrupt status
				retval = rege0;

				m_irq_func(0);

				// scan all oscillators
				for (i = 0; i < oscsenabled+1; i++)
				{
					if (oscillators[i].irqpend)
					{
						// signal this oscillator has an interrupt
						retval = i<<1;

						rege0 = retval | 0x80;

						// and clear its flag
						oscillators[i].irqpend = 0;
						break;
					}
				}

				// if any oscillators still need to be serviced, assert IRQ again immediately
				for (i = 0; i < oscsenabled+1; i++)
				{
					if (oscillators[i].irqpend)
					{
						m_irq_func(1);
						break;
					}
				}

				return retval;

			case 0xe1:  // oscillator enable
				return oscsenabled<<1;

			case 0xe2:  // A/D converter
				return m_adc_func();
		}
	}

	return 0;
}

WRITE8_MEMBER( es5503_device::write )
{
	m_stream->update();

	if (offset < 0xe0)
	{
		int osc = offset & 0x1f;

		switch(offset & 0xe0)
		{
			case 0:     // freq lo
				oscillators[osc].freq &= 0xff00;
				oscillators[osc].freq |= data;
				break;

			case 0x20:      // freq hi
				oscillators[osc].freq &= 0x00ff;
				oscillators[osc].freq |= (data<<8);
				break;

			case 0x40:  // volume
				oscillators[osc].vol = data;
				break;

			case 0x60:  // data - ignore writes
				break;

			case 0x80:  // wavetable pointer
				oscillators[osc].wavetblpointer = (data<<8);
				break;

			case 0xa0:  // oscillator control
				// if a fresh key-on, reset the ccumulator
				if ((oscillators[osc].control & 1) && (!(data&1)))
				{
					oscillators[osc].accumulator = 0;
				}

				oscillators[osc].control = data;
				break;

			case 0xc0:  // bank select / wavetable size / resolution
				if (data & 0x40)    // bank select - not used on the Apple IIgs
				{
					oscillators[osc].wavetblpointer |= 0x10000;
				}
				else
				{
					oscillators[osc].wavetblpointer &= 0xffff;
				}

				oscillators[osc].wavetblsize = ((data>>3) & 7);
				oscillators[osc].wtsize = wavesizes[oscillators[osc].wavetblsize];
				oscillators[osc].resolution = (data & 7);
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
				oscsenabled = (data>>1) & 0x1f;

				output_rate = (clock()/8)/(2+oscsenabled);
				m_stream->set_sample_rate(output_rate);
				m_timer->adjust(attotime::from_hz(output_rate), 0, attotime::from_hz(output_rate));
				break;

			case 0xe2:  // A/D converter
				break;
		}
	}
}
