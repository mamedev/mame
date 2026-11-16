// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
  es5503.cpp - Ensoniq ES5503 "DOC" emulator v3.0
  Emulation by R. Belmont.

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
  0.2   (RB) - improved behavior for volumes > 127, fixes missing notes in Nucleus & missing voices in Thexder
  0.3   (RB) - fixed extraneous clicking, improved timing behavior for e.g. Music Construction Set & Music Studio
  0.4   (RB) - major fixes to IRQ semantics and end-of-sample handling.
  0.5   (RB) - more flexible wave memory hookup (incl. banking) and save state support.
  1.0   (RB) - properly respects the input clock
  2.0   (RB) - C++ conversion, more accurate oscillator IRQ timing
  2.1   (RB) - Corrected phase when looping; synthLAB, Arkanoid, and Arkanoid II no longer go out of tune
  2.1.1 (RB) - Fixed issue introduced in 2.0 where IRQs were delayed
  2.1.2 (RB) - Fixed SoundSmith POLY.SYNTH inst where one-shot on the even oscillator and swap on the odd should loop.
               Conversely, the intro voice in FTA Delta Demo has swap on the even and one-shot on the odd and doesn't
               want to loop.
  2.1.3 (RB) - Fixed oscillator enable register off-by-1 which caused everything to be half a step sharp.
  2.2   (RB) - More precise one-shot even/swap odd behavior from hardware observations with Ian Brumby's SWAPTEST.
  2.3   (RB) - Sync & AM modes added, emulate the volume glitch for the highest-numbered enabled oscillator.
  2.3.1 (RB) - Fixed thinko in the volume glitch emulation and minor cleanup.
  2.4   (RB) - Halting an oscillator from the CPU behaves the same as halting it from the DOC itself.
               Skate or Die on the IIgs accidentally relies on this behavior.
  2.4.1 (RB) - Fixed a bug where halting an oscillator from the CPU with IRQs disabled in the new mode would
               fire the IRQ anyway.  This caused Bard's Tale IIgs to crash after the intro screen.
  3.0   (RB) - Oscillators are now serviced one per 8-clock slot in scan order, with halted oscillators
               still fetching.  Swap, sync, one-shot, interrupt request and DATA register behavior
               follow what the DOCGRADE test suite measures on an Apple IIgs.
*/

#include "emu.h"
#include "es5503.h"

// device type definition
DEFINE_DEVICE_TYPE(ES5503, es5503_device, "es5503", "Ensoniq ES5503")

// useful constants
static constexpr uint32_t wavemasks[8] = { 0x1ff00, 0x1fe00, 0x1fc00, 0x1f800, 0x1f000, 0x1e000, 0x1c000, 0x18000 };
static constexpr uint32_t accmasks[8]  = { 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff };

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
	m_stream(nullptr),
	m_irq_func(*this),
	m_adc_func(*this, 0),
	m_oscillators{},
	m_oscsenabled(1),
	m_rege0(0),
	m_irq_active(false),
	m_present_slot(0),
	m_slot(0),
	m_scan(IDLE_SCAN),
	m_idle(1),
	m_scan_off(true),
	m_drift_slots(0),
	m_bus(0),
	m_channel_strobe(0),
	m_output_channels(1),
	m_output_rate(0),
	m_timer(nullptr),
	m_mix{},
	m_last{},
	m_fifo_head(0),
	m_fifo_count(0)
{
}


//-------------------------------------------------
//  scan_tick - keep the scan going when nobody
//  is touching the chip
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(es5503_device::scan_tick)
{
	// stay a couple of slots behind so a host that times its own accesses can still place one
	const u64 now = now_clock();
	if (now > 16)
	{
		run_to(now - 16);
	}
}

//-------------------------------------------------
//  rom_bank_pre_change - bring the scan up to date
//  before the wave memory banking changes
//-------------------------------------------------

void es5503_device::rom_bank_pre_change()
{
	run_to(now_clock());
}

// registers are dynamic and are only refreshed when the scan visits their oscillator
void es5503_device::drift(ES5503Osc &osc)
{
	osc.freq = 0xffff;
	osc.control = 0xff;
	osc.vol = 0xff;
	osc.data = 0xff;
	osc.wavetblpointer = 0x1ff00;
	osc.wavetblsize = 7;
	osc.resolution = 7;
	osc.accumulator = 0xffffff;
	osc.irqpend = 1;
	osc.prevhalt = 1;
}

void es5503_device::check_drift(int onum)
{
	ES5503Osc &osc = m_oscillators[onum];

	if ((m_slot - osc.refresh) > m_drift_slots)
	{
		drift(osc);
		osc.refresh = m_slot;
	}
}

// service one oscillator
void es5503_device::visit(int onum)
{
	check_drift(onum);

	ES5503Osc &osc = m_oscillators[onum];
	ES5503Osc &partner = m_oscillators[onum ^ 1];
	const bool even = !(onum & 1);
	const bool was_halted = osc.control & 1;
	const int mode = (osc.control >> 1) & 3;
	const int resshift = 9 + osc.resolution - osc.wavetblsize;
	const uint32_t top = 1U << (17 + osc.resolution);
	const uint32_t wtptr = osc.wavetblpointer & wavemasks[osc.wavetblsize];
	const bool partner_swapped = (partner.control & 1) && (((partner.control >> 1) & 3) == MODE_SWAP);

	osc.refresh = m_slot;

	// An oscillator found halted in one-shot or swap mode is rewound, a visit late.  One started
	// from the CPU in those modes therefore begins at the top of its table, while free-run and
	// sync oscillators pick up where they stopped.
	if (osc.prevhalt && (mode & 1))
	{
		osc.accumulator = 0;
	}
	osc.prevhalt = was_halted;

	// channel strobe is always valid when reading; this allows potentially banking per voice
	m_channel_strobe = (osc.control >> 4) & 0xf;

	// halted oscillators fetch too
	m_bus = read_byte(wtptr + ((osc.accumulator >> resshift) & accmasks[osc.wavetblsize]));

	// the data register never takes a zero
	if (m_bus != 0)
	{
		osc.data = m_bus;
	}

	bool ended = false;
	if (!was_halted)
	{
		if (m_bus != 0)
		{
			const int32_t data = int8_t(m_bus ^ 0x80);
			const int chan = (osc.control >> 4) & (m_output_channels - 1);

			// in AM mode an odd oscillator plays nothing itself, it sets the volume of the next one up
			if ((mode == MODE_SYNCAM) && !even)
			{
				if ((onum < 31) && !(m_oscillators[onum + 1].control & 1))
				{
					m_oscillators[onum + 1].vol = m_bus;
				}
			}
			else
			{
				// the uppermost enabled oscillator is heard for three slots
				m_mix[chan] += data * osc.vol * ((onum == (m_oscsenabled - 1)) ? 3 : 1);
			}
		}

		// only the bits that address the table take part
		osc.accumulator = (osc.accumulator & (top - 1)) + osc.freq;

		if (m_bus == 0)
		{
			osc.control |= 1;
			ended = true;
		}
		else if (osc.accumulator >= top)
		{
			osc.accumulator &= top - 1;
			ended = true;

			switch (mode)
			{
				case MODE_ONESHOT:
					// an even oscillator keeps going while its partner sits halted in swap mode
					if (!even || !partner_swapped)
					{
						osc.control |= 1;
					}
					break;

				case MODE_SYNCAM:
					// an even oscillator starts itself and its partner over
					if (even)
					{
						osc.accumulator = 0;
						partner.accumulator = 0;
					}
					break;

				case MODE_SWAP:
					osc.control |= 1;
					break;
			}
		}
	}
	else
	{
		// an oscillator halted on a zero finds it again every time
		ended = (m_bus == 0);

		// an odd oscillator halted in swap mode keeps its partner running
		if (even && partner_swapped && (m_bus != 0))
		{
			osc.control &= ~1;
		}
	}

	if (ended)
	{
		if (osc.control & 0x08)
		{
			osc.irqpend = 1;
		}

		// an even oscillator ending in swap mode starts its partner where it left off
		if (even && (mode == MODE_SWAP) && (osc.control & 1))
		{
			partner.control &= ~1;
		}
	}

	// one request is presented at a time, and it holds until 0xe0 is read
	if (osc.irqpend && !m_irq_active)
	{
		m_rege0 = onum;
		m_irq_active = true;
		m_present_slot = m_slot;
		m_irq_func(1);
	}
}

void es5503_device::end_round()
{
	if (m_fifo_count == FIFO_FRAMES)
	{
		m_fifo_head = (m_fifo_head + 1) % FIFO_FRAMES;
		m_fifo_count--;
	}

	const int tail = (m_fifo_head + m_fifo_count) % FIFO_FRAMES;
	std::copy_n(&m_mix[0], m_output_channels, &m_fifo[tail * m_output_channels]);
	std::fill_n(&m_mix[0], m_output_channels, 0);
	m_fifo_count++;
}

// each oscillator gets a slot of 8 clocks, and 2 slots for RAM refresh follow the last one
void es5503_device::run_slot()
{
	if (m_scan_off)
	{
		// nothing is serviced between a reset and the first write of the oscillator count
	}
	else if (m_idle == 2)
	{
		m_idle = 0;
		m_scan = 0;
		end_round();
		visit(m_scan);
	}
	else if (m_idle == 1)
	{
		m_idle = 2;
		m_scan = IDLE_SCAN + 1;
	}
	else if (m_scan == (m_oscsenabled - 1))
	{
		// the end is found by an equality compare, so a scan that is already past it runs on to 31
		m_idle = 1;
		m_scan = IDLE_SCAN;
	}
	else
	{
		m_scan = (m_scan + 1) & 0x1f;
		if (m_scan == 0)
		{
			end_round();
		}
		visit(m_scan);
	}

	m_slot++;
}

void es5503_device::run_to(u64 clock)
{
	const u64 target = clock >> 3;

	while (m_slot <= target)
	{
		run_slot();
	}
}

void es5503_device::sound_stream_update(sound_stream &stream)
{
	run_to(now_clock());

	for (int i = 0; i < stream.samples(); i++)
	{
		if (m_fifo_count != 0)
		{
			std::copy_n(&m_fifo[m_fifo_head * m_output_channels], m_output_channels, &m_last[0]);
			m_fifo_head = (m_fifo_head + 1) % FIFO_FRAMES;
			m_fifo_count--;
		}

		for (int chan = 0; chan < m_output_channels; chan++)
		{
			stream.put_int(chan, i, m_last[chan], 32768*8);
		}
	}
}


void es5503_device::device_start()
{
	save_pointer(STRUCT_MEMBER(m_oscillators, freq), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, control), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, vol), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, data), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, wavetblpointer), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, wavetblsize), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, resolution), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, accumulator), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, irqpend), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, prevhalt), 32);
	save_pointer(STRUCT_MEMBER(m_oscillators, refresh), 32);
	save_item(NAME(m_oscsenabled));
	save_item(NAME(m_rege0));
	save_item(NAME(m_irq_active));
	save_item(NAME(m_present_slot));
	save_item(NAME(m_slot));
	save_item(NAME(m_scan));
	save_item(NAME(m_idle));
	save_item(NAME(m_scan_off));
	save_item(NAME(m_bus));
	save_item(NAME(m_channel_strobe));
	save_item(NAME(m_mix));
	save_item(NAME(m_last));

	m_fifo.resize(FIFO_FRAMES * m_output_channels);

	// the registers are dynamic and nothing has refreshed them yet, so they come up as all ones
	for (auto &osc : m_oscillators)
	{
		drift(osc);
		osc.refresh = 0;
	}

	m_output_rate = (clock() / 8) / (m_oscsenabled + 2);
	m_stream = stream_alloc(0, m_output_channels, m_output_rate);

	m_timer = timer_alloc(FUNC(es5503_device::scan_tick), this);
}

void es5503_device::update_rate()
{
	m_output_rate = (clock() / 8) / (m_oscsenabled + 2);
	m_stream->set_sample_rate(m_output_rate);

	const attotime update_rate = m_output_rate ? attotime::from_hz(m_output_rate) : attotime::never;
	m_timer->adjust(update_rate, 0, update_rate);
}

void es5503_device::device_clock_changed()
{
	// slots are counted from time zero, so move everything that remembers one
	const u64 slot = (now_clock() >> 3) + 1;

	for (auto &elem : m_oscillators)
	{
		elem.refresh = slot - (m_slot - elem.refresh);
	}
	m_present_slot = slot - (m_slot - m_present_slot);
	m_slot = slot;

	m_drift_slots = u64(clock() / 8) * 3 / 2;

	update_rate();
}

void es5503_device::device_reset()
{
	// Reset turns every oscillator off.  Their registers are left alone, but with nothing
	// refreshing them they are soon lost.
	m_slot = (now_clock() >> 3) + 1;
	m_oscsenabled = 1;
	m_irq_active = false;
	m_present_slot = 0;
	m_scan = IDLE_SCAN;
	m_idle = 1;
	m_scan_off = true;
	m_drift_slots = u64(clock() / 8) * 3 / 2;
	m_bus = 0;
	m_channel_strobe = 0;
	m_fifo_head = m_fifo_count = 0;
	std::fill(std::begin(m_mix), std::end(m_mix), 0);
	std::fill(std::begin(m_last), std::end(m_last), 0);

	m_irq_func(0);
	update_rate();
}

u8 es5503_device::read(offs_t offset)
{
	uint8_t retval;

	run_to(now_clock());

	if (offset < 0xe0)
	{
		const int osc = offset & 0x1f;

		check_drift(osc);

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
				retval = 0x80;
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
				retval = (m_rege0 << 1) | 0x41;

				if (m_irq_active)
				{
					// a read in the slot that presents the request leaves it standing
					if (!machine().side_effects_disabled() && (m_present_slot != (m_slot - 1)))
					{
						m_oscillators[m_rege0].irqpend = 0;
						m_irq_active = false;
						m_irq_func(0);
					}
				}
				else
				{
					retval |= 0x80;
				}
				return retval;

			case 0xe1:  // oscillator enable
				return ((m_oscsenabled - 1) << 1) | 0xc1;

			case 0xe2:  // A/D converter
				return m_adc_func();
		}
	}

	return 0;
}

void es5503_device::write(offs_t offset, u8 data)
{
	run_to(now_clock());

	if (offset < 0xe0)
	{
		const int osc = offset & 0x1f;

		check_drift(osc);
		m_oscillators[osc].refresh = m_slot;

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
				m_oscillators[osc].wavetblpointer &= 0x10000;
				m_oscillators[osc].wavetblpointer |= (data<<8);
				break;

			case 0xa0:  // oscillator control
				// Halting from the CPU raises no interrupt and starts no swap partner by itself, and
				// the scan takes care of rewinding.  Any write drops a waiting request.
				m_oscillators[osc].irqpend = 0;
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
				m_stream->update();
				m_oscsenabled = ((data>>1) & 0x1f) + 1;
				update_rate();

				// a write during the refresh slots calls off the end of the round, and the scan
				// carries on from where its counter sits
				m_idle = 0;
				m_scan_off = false;
				break;

			case 0xe2:  // A/D converter
				break;
		}
	}
}
