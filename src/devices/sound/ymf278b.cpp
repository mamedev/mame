// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, hap
/*

   YMF278B  FM + Wave table Synthesizer (OPL4)

   Timer and PCM YMF278B.  The FM will be shared with the ymf262, eventually.

   This chip roughly splits the difference between the Sega 315-5560 MultiPCM
   (Multi32, Model 1/2) and YMF 292-F SCSP (later Model 2, STV, Saturn, Model 3).

   Features as listed in LSI-4MF2782 data sheet:
    FM Synthesis (same as YMF262)
     1. Sound generation mode
         Two-operater mode
          Generates eighteen voices or fifteen voices plus five rhythm sounds simultaneously
         Four-operator mode
          Generates six voices in four-operator mode plus six voices in two-operator mode simultaneously,
          or generates six voices in four-operator mode plus three voices in two-operator mode plus five
          rhythm sounds simultaneously
     2. Eight selectable waveforms
     3. Stereo output
    Wave Table Synthesis
     1. Generates twenty-four voices simultaneously
     2. 44.1kHz sampling rate for output sound data
     3. Selectable from 8-bit, 12-bit and 16-bit word lengths for wave data
     4. Stereo output (16-stage panpot for each voice)
    Wave Data
     1. Accepts 32M bit external memory at maximum
     2. Up to 512 wave tables
     3. External ROM or SRAM can be connected. With SRAM connected, the CPU can download wave data
     4. Outputs chip select signals for 1Mbit, 4Mbit, 8Mbit or 16Mbit memory
     5. Can be directly connected to the Yamaha YRW801 (Wave data ROM)
        Features of YRW801 as listed in LSI 4RW801A2
          Built-in wave data of tones which comply with GM system Level 1
           Melody tone ....... 128 tones
           Percussion tone ...  47 tones
          16Mbit capacity (2,097,152word x 8)

   By R. Belmont and O. Galibert.


   TODO:
   - accurate timing of envelopes
   - LFO (vibrato, tremolo)
   - integrate YMF262 mixing (used by Fuuki games, not used by Psikyo and Metro games)
*/

#include "emu.h"
#include "ymf278b.h"
#include "ymf262.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


// default address map
static ADDRESS_MAP_START( ymf278b, AS_0, 8, ymf278b_device )
		AM_RANGE(0x000000, 0x3fffff) AM_ROM
ADDRESS_MAP_END


/**************************************************************************/

int ymf278b_device::compute_rate(YMF278BSlot *slot, int val)
{
	int res, oct;

	if(val == 0)
		return 0;
	if(val == 15)
		return 63;
	if(slot->RC != 15)
	{
		oct = slot->octave;
		if (oct & 8)
			oct |= -8;

		res = (oct+slot->RC)*2 + (slot->F_NUMBER & 0x200 ? 1 : 0) + val*4;
	}
	else
		res = val * 4;
	if(res < 0)
		res = 0;
	else if(res > 63)
		res = 63;

	return res;
}

UINT32 ymf278b_device::compute_decay_env_vol_step(YMF278BSlot *slot, int val)
{
	int rate;
	UINT32 res;

	// rate override with damping/pseudo reverb
	if (slot->DAMP)
		rate = 56; // approximate, datasheet says it's slightly curved though
	else if (slot->preverb && slot->env_vol > ((6*8)<<23))
	{
		// pseudo reverb starts at -18dB (6 in voltab)
		slot->env_preverb = 1;
		rate = 5;
	}
	else
		rate = compute_rate(slot, val);

	if (rate < 4)
		res = 0;
	else
		res = (256U<<23) / m_lut_dr[rate];

	return res;
}

void ymf278b_device::compute_freq_step(YMF278BSlot *slot)
{
	UINT32 step;
	int oct;

	oct = slot->octave;
	if(oct & 8)
		oct |= -8;

	step = (slot->F_NUMBER | 1024) << (oct + 8);
	slot->step = step >> 3;
}

void ymf278b_device::compute_envelope(YMF278BSlot *slot)
{
	switch (slot->env_step)
	{
		// Attack
		case 0:
		{
			// Attack
			int rate = compute_rate(slot, slot->AR);
			slot->env_vol = 256U<<23;
			slot->env_vol_lim = (256U<<23) - 1;

			if (rate==63)
			{
				// immediate
				LOG(("YMF278B: Attack skipped - "));
				slot->env_vol = 0;
				slot->env_step++;
				compute_envelope(slot);
			}
			else if (rate<4)
			{
				slot->env_vol_step = 0;
			}
			else
			{
				// NOTE: attack rate is linear here, but datasheet shows a smooth curve
				LOG(("YMF278B: Attack, val = %d, rate = %d, delay = %g\n", slot->AR, rate, m_lut_ar[rate]*1000.0));
				slot->env_vol_step = ~((256U<<23) / m_lut_ar[rate]);
			}

			break;
		}

		// Decay 1
		case 1:
			if(slot->DL)
			{
				LOG(("YMF278B: Decay step 1, dl=%d, val = %d rate = %d, delay = %g, PRVB = %d, DAMP = %d\n", slot->DL, slot->D1R, compute_rate(slot, slot->D1R), m_lut_dr[compute_rate(slot, slot->D1R)]*1000.0, slot->preverb, slot->DAMP));
				slot->env_vol_step = compute_decay_env_vol_step(slot, slot->D1R);
				slot->env_vol_lim = (slot->DL*8)<<23;
			}
			else
			{
				LOG(("YMF278B: Decay 1 skipped - "));
				slot->env_step++;
				compute_envelope(slot);
			}

			break;

		// Decay 2
		case 2:
			LOG(("YMF278B: Decay step 2, val = %d, rate = %d, delay = %g, , PRVB = %d, DAMP = %d, current vol = %d\n", slot->D2R, compute_rate(slot, slot->D2R), m_lut_dr[compute_rate(slot, slot->D2R)]*1000.0, slot->preverb, slot->DAMP, slot->env_vol >> 23));
			slot->env_vol_step = compute_decay_env_vol_step(slot, slot->D2R);
			slot->env_vol_lim = 256U<<23;
			break;

		// Decay 2 reached -96dB
		case 3:
			LOG(("YMF278B: Voice cleared because of decay 2\n"));
			slot->env_vol = 256U<<23;
			slot->env_vol_step = 0;
			slot->env_vol_lim = 0;
			slot->active = 0;
			break;

		// Release
		case 4:
			LOG(("YMF278B: Release, val = %d, rate = %d, delay = %g, PRVB = %d, DAMP = %d\n", slot->RR, compute_rate(slot, slot->RR), m_lut_dr[compute_rate(slot, slot->RR)]*1000.0, slot->preverb, slot->DAMP));
			slot->env_vol_step = compute_decay_env_vol_step(slot, slot->RR);
			slot->env_vol_lim = 256U<<23;
			break;

		// Release reached -96dB
		case 5:
			LOG(("YMF278B: Release ends\n"));
			slot->env_vol = 256U<<23;
			slot->env_vol_step = 0;
			slot->env_vol_lim = 0;
			slot->active = 0;
			break;

		default: break;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ymf278b_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i, j;
	YMF278BSlot *slot = nullptr;
	INT16 sample = 0;
	INT32 *mixp;
	INT32 vl, vr;

	if (&stream == m_stream_ymf262)
	{
		ymf262_update_one(m_ymf262, outputs, samples);
		return;
	}

	memset(m_mix_buffer.get(), 0, sizeof(m_mix_buffer[0])*samples*2);

	for (i = 0; i < 24; i++)
	{
		slot = &m_slots[i];

		if (slot->active)
		{
			mixp = m_mix_buffer.get();

			for (j = 0; j < samples; j++)
			{
				if (slot->stepptr >= slot->endaddr)
				{
					slot->stepptr = slot->stepptr - slot->endaddr + slot->loopaddr;

					// NOTE: loop overflow is still possible here if (slot->stepptr >= slot->endaddr)
					// This glitch may be (ab)used to your advantage to create pseudorandom noise.
				}

				switch (slot->bits)
				{
					// 8 bit
					case 0:
						sample = m_direct->read_byte(slot->startaddr + (slot->stepptr>>16))<<8;
						break;

					// 12 bit
					case 1:
						if (slot->stepptr & 0x10000)
							sample = m_direct->read_byte(slot->startaddr + (slot->stepptr>>17)*3+2)<<8 |
								(m_direct->read_byte(slot->startaddr + (slot->stepptr>>17)*3+1) << 4 & 0xf0);
						else
							sample = m_direct->read_byte(slot->startaddr + (slot->stepptr>>17)*3)<<8 |
								(m_direct->read_byte(slot->startaddr + (slot->stepptr>>17)*3+1) & 0xf0);
						break;

					// 16 bit
					case 2:
						sample = m_direct->read_byte(slot->startaddr + ((slot->stepptr>>16)*2))<<8 |
							m_direct->read_byte(slot->startaddr + ((slot->stepptr>>16)*2)+1);
						break;

					// ?? bit, effect is unknown, datasheet says it's prohibited
					case 3:
						sample = 0;
						break;
				}

				*mixp++ += (sample * m_volume[slot->TL+m_pan_left [slot->pan]+(slot->env_vol>>23)])>>17;
				*mixp++ += (sample * m_volume[slot->TL+m_pan_right[slot->pan]+(slot->env_vol>>23)])>>17;

				// update frequency
				slot->stepptr += slot->step;

				// update envelope
				slot->env_vol += slot->env_vol_step;
				if (((INT32)(slot->env_vol - slot->env_vol_lim)) >= 0)
				{
					slot->env_step++;
					compute_envelope(slot);
				}
				else if (slot->preverb && !slot->env_preverb && slot->env_step && slot->env_vol > ((6*8)<<23))
					compute_envelope(slot);
			}
		}
	}

	mixp = m_mix_buffer.get();
	vl = m_mix_level[m_pcm_l];
	vr = m_mix_level[m_pcm_r];
	for (i = 0; i < samples; i++)
	{
		outputs[0][i] = (*mixp++ * vl) >> 16;
		outputs[1][i] = (*mixp++ * vr) >> 16;
	}
}

void ymf278b_device::irq_check()
{
	int prev_line = m_irq_line;
	m_irq_line = m_current_irq ? 1 : 0;
	if (m_irq_line != prev_line && !m_irq_handler.isnull())
		m_irq_handler(m_irq_line);
}

enum
{
	TIMER_A = 0,
	TIMER_B,
	TIMER_BUSY_CLEAR,
	TIMER_LD_CLEAR
};

void ymf278b_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_A:
		if(!(m_enable & 0x40))
		{
			m_current_irq |= 0x40;
			irq_check();
		}
		break;

	case TIMER_B:
		if(!(m_enable & 0x20))
		{
			m_current_irq |= 0x20;
			irq_check();
		}
		break;

	case TIMER_BUSY_CLEAR:
		m_status_busy = 0;
		break;

	case TIMER_LD_CLEAR:
		m_status_ld = 0;
		break;
	}
}


/**************************************************************************/

void ymf278b_device::A_w(UINT8 reg, UINT8 data)
{
	// FM register array 0 (compatible with YMF262)
	switch(reg)
	{
		// LSI TEST
		case 0x00:
		case 0x01:
			break;

		// timer a count
		case 0x02:
			if (data != m_timer_a_count)
			{
				m_timer_a_count = data;

				// change period, ~80.8us * t
				if (m_enable & 1)
					m_timer_a->adjust(m_timer_a->remaining(), 0, m_timer_base * (256-data) * 4);
			}
			break;

		// timer b count
		case 0x03:
			if (data != m_timer_b_count)
			{
				m_timer_b_count = data;

				// change period, ~323.1us * t
				if (m_enable & 2)
					m_timer_b->adjust(m_timer_b->remaining(), 0, m_timer_base * (256-data) * 16);
			}
			break;

		// timer control
		case 0x04:
			if(data & 0x80)
				m_current_irq = 0;
			else
			{
				// reset timers
				if((m_enable ^ data) & 1)
				{
					attotime period = (data & 1) ? m_timer_base * (256-m_timer_a_count) * 4 : attotime::never;
					m_timer_a->adjust(period, 0, period);
				}
				if((m_enable ^ data) & 2)
				{
					attotime period = (data & 2) ? m_timer_base * (256-m_timer_b_count) * 16 : attotime::never;
					m_timer_b->adjust(period, 0, period);
				}

				m_enable = data;
				m_current_irq &= ~data;
			}
			irq_check();
			break;

		default:
			logerror("YMF278B:  Port A write %02x, %02x\n", reg, data);
			break;
	}
}

void ymf278b_device::B_w(UINT8 reg, UINT8 data)
{
	// FM register array 1 (compatible with YMF262)
	switch(reg)
	{
		// LSI TEST
		case 0x00:
		case 0x01:
			break;

		// expansion register (NEW2/NEW)
		case 0x05:
			m_exp = data;
			break;

		default:
			logerror("YMF278B:  Port B write %02x, %02x\n", reg, data);
			break;
	}
}

void ymf278b_device::retrigger_note(YMF278BSlot *slot)
{
	// activate channel
	if (slot->octave != 8)
		slot->active = 1;

	// reset sample pos and go to attack stage
	slot->stepptr = 0;
	slot->env_step = 0;
	slot->env_preverb = 0;

	compute_freq_step(slot);
	compute_envelope(slot);
}

void ymf278b_device::C_w(UINT8 reg, UINT8 data)
{
	// Handle slot registers specifically
	if (reg >= 0x08 && reg <= 0xf7)
	{
		YMF278BSlot *slot = nullptr;
		int snum;
		snum = (reg-8) % 24;
		slot = &m_slots[snum];
		switch((reg-8) / 24)
		{
			case 0:
			{
				attotime period;
				UINT32 offset;
				UINT8 p[12];
				int i;

				slot->wave &= 0x100;
				slot->wave |= data;

				// load wavetable header
				if(slot->wave < 384 || !m_wavetblhdr)
					offset = slot->wave * 12;
				else
					offset = m_wavetblhdr*0x80000 + (slot->wave - 384) * 12;
				for (i = 0; i < 12; i++)
					p[i] = m_direct->read_byte(offset+i);

				slot->bits = (p[0]&0xc0)>>6;
				slot->startaddr = (p[2] | (p[1]<<8) | ((p[0]&0x3f)<<16));
				slot->loopaddr = (p[4]<<16) | (p[3]<<24);
				slot->endaddr = (p[6]<<16) | (p[5]<<24);
				slot->endaddr -= 0x00010000U;
				slot->endaddr ^= 0xffff0000U;

				// copy internal registers data
				for (i = 7; i < 12; i++)
					C_w(8 + snum + (i-2) * 24, p[i]);

				// status register LD bit is on for approx 300us
				m_status_ld = 1;
				period = attotime::from_usec(300);
				if (m_clock != YMF278B_STD_CLOCK)
					period = (period * m_clock) / YMF278B_STD_CLOCK;
				m_timer_ld->adjust(period);

				// retrigger if key is on
				if (slot->KEY_ON)
					retrigger_note(slot);
				else if (slot->active)
				{
					// deactivate channel
					slot->env_step = 5;
					compute_envelope(slot);
				}

				break;
			}

			case 1:
				slot->wave &= 0xff;
				slot->wave |= ((data&0x1)<<8);
				slot->F_NUMBER &= 0x380;
				slot->F_NUMBER |= (data>>1);
				if (slot->active && (data ^ m_pcmregs[reg]) & 0xfe)
				{
					compute_freq_step(slot);
					compute_envelope(slot);
				}
				break;

			case 2:
				slot->F_NUMBER &= 0x07f;
				slot->F_NUMBER |= ((data&0x07)<<7);
				slot->preverb = (data&0x8)>>3;
				slot->octave = (data&0xf0)>>4;
				if (data != m_pcmregs[reg])
				{
					// channel goes off if octave is set to -8 (datasheet says it's prohibited)
					// (it is ok if this activates the channel while it was off: compute_envelope will reset it again if needed)
					slot->active = (slot->octave != 8);

					if (slot->active)
					{
						slot->env_preverb = 0;
						compute_freq_step(slot);
						compute_envelope(slot);
					}
				}
				break;

			case 3:
				slot->TL = data>>1;
				slot->LD = data&0x1;
				break;

			case 4:
				slot->CH = (data&0x10)>>4;
				// CH bit note: output to DO1 pin (1) or DO2 pin (0), this may
				// silence the channel depending on how it's wired up on the PCB.
				// For now, it's always enabled.
				// (bit 5 (LFO reset) is also not hooked up yet)

				slot->pan = data&0xf;
				slot->DAMP = (data&0x40)>>6;
				if (data & 0x80)
				{
					// don't retrigger if key was already on
					if (slot->KEY_ON)
					{
						if ((data ^ m_pcmregs[reg]) & 0x40)
							compute_envelope(slot);

						break;
					}

					retrigger_note(slot);
				}
				else if (slot->active)
				{
					// release
					slot->env_step = 4;
					compute_envelope(slot);
				}
				slot->KEY_ON = (data&0x80)>>7;
				break;

			case 5:
				// LFO and vibrato level, not hooked up yet
				slot->LFO = (data>>3)&0x7;
				slot->VIB = data&0x7;
				break;

			case 6:
				slot->AR = data>>4;
				slot->D1R = data&0xf;
				if (slot->active && data != m_pcmregs[reg])
					compute_envelope(slot);
				break;

			case 7:
				slot->DL = data>>4;
				slot->D2R = data&0xf;
				if (slot->active && data != m_pcmregs[reg])
					compute_envelope(slot);
				break;

			case 8:
				slot->RC = data>>4;
				slot->RR = data&0xf;
				if (slot->active && data != m_pcmregs[reg])
					compute_envelope(slot);
				break;

			case 9:
				// tremolo level, not hooked up yet
				slot->AM = data & 0x7;
				break;
		}
	}
	else
	{
		// All non-slot registers
		switch (reg)
		{
			// LSI TEST
			case 0x00:
			case 0x01:
				break;

			case 0x02:
				m_wavetblhdr = (data>>2)&0x7;
				m_memmode = data&3;
				break;

			case 0x03:
				data &= 0x3f; // !
				break;
			case 0x04:
				break;
			case 0x05:
				// set memory address
				m_memadr = m_pcmregs[3] << 16 | m_pcmregs[4] << 8 | data;
				break;

			case 0x06:
				// memory data
				m_addrspace[0]->write_byte(m_memadr, data);
				m_memadr = (m_memadr + 1) & 0x3fffff;
				break;

			case 0x07:
				break; // unused

			case 0xf8:
				m_fm_l = data & 0x7;
				m_fm_r = (data>>3)&0x7;
				break;

			case 0xf9:
				m_pcm_l = data & 0x7;
				m_pcm_r = (data>>3)&0x7;
				break;

			default:
				logerror("YMF278B:  Port C write %02x, %02x\n", reg, data);
				break;
		}
	}

	m_pcmregs[reg] = data;
}

void ymf278b_device::timer_busy_start(int is_pcm)
{
	// status register BUSY bit is on for 56(FM) or 88(PCM) cycles
	m_status_busy = 1;
	m_timer_busy->adjust(attotime::from_hz(m_clock / (is_pcm ? 88 : 56)));
}

WRITE8_MEMBER( ymf278b_device::write )
{
	switch (offset)
	{
		case 0:
		case 2:
			timer_busy_start(0);
			m_port_AB = data;
			m_lastport = offset>>1 & 1;
			ymf262_write(m_ymf262, offset, data);
			break;

		case 1:
		case 3:
			timer_busy_start(0);
			if (m_lastport) B_w(m_port_AB, data);
			else A_w(m_port_AB, data);
			m_last_fm_data = data;
			ymf262_write(m_ymf262, offset, data);
			break;

		case 4:
			timer_busy_start(1);
			m_port_C = data;
			break;

		case 5:
			// PCM regs are only accessible if NEW2 is set
			if (~m_exp & 2)
				break;

			m_stream->update();

			timer_busy_start(1);
			C_w(m_port_C, data);
			break;

		default:
			logerror("%s: unexpected write at offset %X to ymf278b = %02X\n", machine().describe_context(), offset, data);
			break;
	}
}


READ8_MEMBER( ymf278b_device::read )
{
	UINT8 ret = 0;

	switch (offset)
	{
		// status register
		case 0:
		{
			// bits 0 and 1 are only valid if NEW2 is set
			UINT8 newbits = 0;
			if (m_exp & 2)
				newbits = (m_status_ld << 1) | m_status_busy;

			ret = newbits | m_current_irq | (m_irq_line ? 0x80 : 0x00);
			break;
		}

		// FM regs can be read too (on contrary to what the datasheet says)
		case 1:
		case 3:
			// but they're not implemented here yet
			// This may be incorrect, but it makes the mbwave moonsound detection in msx drivers pass.
			ret = m_last_fm_data;
			break;

		// PCM regs
		case 5:
			// only accessible if NEW2 is set
			if (~m_exp & 2)
				break;

			switch (m_port_C)
			{
				// special cases
				case 2:
					ret = (m_pcmregs[m_port_C] & 0x1f) | 0x20; // device ID in upper bits
					break;
				case 6:
					ret = m_direct->read_byte(m_memadr);
					m_memadr = (m_memadr + 1) & 0x3fffff;
					break;

				default:
					ret = m_pcmregs[m_port_C];
					break;
			}
			break;

		default:
			logerror("%s: unexpected read at offset %X from ymf278b\n", machine().describe_context(), offset);
			break;
	}

	return ret;
}


/**************************************************************************/

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ymf278b_device::device_reset()
{
	int i;

	// clear registers
	for (i = 0; i <= 4; i++)
		A_w(i, 0);
	B_w(5, 0);
	for (i = 0; i < 8; i++)
		C_w(i, 0);
	for (i = 0xff; i >= 8; i--)
		C_w(i, 0);
	C_w(0xf8, 0x1b);

	m_port_AB = m_port_C = 0;
	m_lastport = 0;
	m_memadr = 0;

	// init/silence channels
	for (i = 0; i < 24 ; i++)
	{
		YMF278BSlot *slot = &m_slots[i];

		slot->LFO = 0;
		slot->VIB = 0;
		slot->AR = 0;
		slot->D1R = 0;
		slot->DL = 0;
		slot->D2R = 0;
		slot->RC = 0;
		slot->RR = 0;
		slot->AM = 0;

		slot->startaddr = 0;
		slot->loopaddr = 0;
		slot->endaddr = 0;

		slot->env_step = 5;
		compute_envelope(slot);
	}

	m_timer_a->reset();
	m_timer_b->reset();
	m_timer_busy->reset();  m_status_busy = 0;
	m_timer_ld->reset();    m_status_ld = 0;

	m_irq_line = 0;
	m_current_irq = 0;
	if (!m_irq_handler.isnull())
		m_irq_handler(0);

	ymf262_reset_chip(m_ymf262);
}

void ymf278b_device::device_stop()
{
	ymf262_shutdown(m_ymf262);
	m_ymf262 = nullptr;
}

void ymf278b_device::precompute_rate_tables()
{
	int i;

	// decay rate
	for (i = 0; i < 64; i++)
	{
		if (i <= 3)
			m_lut_dr[i] = 0;
		else if (i >= 60)
			m_lut_dr[i] = 15 << 4;
		else
			m_lut_dr[i] = (15 << (21 - i / 4)) / (4 + i % 4);
	}

	// attack rate (manual shows curve instead of linear though, so this is not entirely accurate)
	for (i = 0; i < 64; i++)
	{
		if (i <= 3 || i == 63)
			m_lut_ar[i] = 0;
		else if (i >= 60)
			m_lut_ar[i] = 17;
		else
			m_lut_ar[i] = (67 << (15 - i / 4)) / (4 + i % 4);
	}
}

void ymf278b_device::register_save_state()
{
	int i;

	save_item(NAME(m_pcmregs));
	save_item(NAME(m_wavetblhdr));
	save_item(NAME(m_memmode));
	save_item(NAME(m_memadr));
	save_item(NAME(m_status_busy));
	save_item(NAME(m_status_ld));
	save_item(NAME(m_exp));
	save_item(NAME(m_fm_l));
	save_item(NAME(m_fm_r));
	save_item(NAME(m_pcm_l));
	save_item(NAME(m_pcm_r));
	save_item(NAME(m_timer_a_count));
	save_item(NAME(m_timer_b_count));
	save_item(NAME(m_enable));
	save_item(NAME(m_current_irq));
	save_item(NAME(m_irq_line));
	save_item(NAME(m_port_AB));
	save_item(NAME(m_port_C));
	save_item(NAME(m_lastport));
	save_item(NAME(m_last_fm_data));

	for (i = 0; i < 24; ++i)
	{
		save_item(NAME(m_slots[i].wave), i);
		save_item(NAME(m_slots[i].F_NUMBER), i);
		save_item(NAME(m_slots[i].octave), i);
		save_item(NAME(m_slots[i].preverb), i);
		save_item(NAME(m_slots[i].DAMP), i);
		save_item(NAME(m_slots[i].CH), i);
		save_item(NAME(m_slots[i].LD), i);
		save_item(NAME(m_slots[i].TL), i);
		save_item(NAME(m_slots[i].pan), i);
		save_item(NAME(m_slots[i].LFO), i);
		save_item(NAME(m_slots[i].VIB), i);
		save_item(NAME(m_slots[i].AM), i);

		save_item(NAME(m_slots[i].AR), i);
		save_item(NAME(m_slots[i].D1R), i);
		save_item(NAME(m_slots[i].DL), i);
		save_item(NAME(m_slots[i].D2R), i);
		save_item(NAME(m_slots[i].RC), i);
		save_item(NAME(m_slots[i].RR), i);

		save_item(NAME(m_slots[i].step), i);
		save_item(NAME(m_slots[i].stepptr), i);

		save_item(NAME(m_slots[i].active), i);
		save_item(NAME(m_slots[i].KEY_ON), i);
		save_item(NAME(m_slots[i].bits), i);
		save_item(NAME(m_slots[i].startaddr), i);
		save_item(NAME(m_slots[i].loopaddr), i);
		save_item(NAME(m_slots[i].endaddr), i);

		save_item(NAME(m_slots[i].env_step), i);
		save_item(NAME(m_slots[i].env_vol), i);
		save_item(NAME(m_slots[i].env_vol_step), i);
		save_item(NAME(m_slots[i].env_vol_lim), i);
		save_item(NAME(m_slots[i].env_preverb), i);
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

static void ymf278b_ymf262_irq_handler(void *param,int irq)
{
}


static void ymf278b_ymf262_timer_handler(void *param, int c, const attotime &period)
{
}

static void ymf278b_ymf262_update_request(void *param, int interval)
{
	ymf278b_device *ymf278b = (ymf278b_device *) param;
	ymf278b->ymf262_update_request();
}


void ymf278b_device::ymf262_update_request()
{
	m_stream_ymf262->update();
}


void ymf278b_device::device_start()
{
	int i;

	m_direct = &space().direct();
	m_clock = clock();
	m_irq_handler.resolve();

	m_timer_base = attotime::from_hz(m_clock) * (19*36);
	m_timer_a = timer_alloc(TIMER_A);
	m_timer_b = timer_alloc(TIMER_B);
	m_timer_busy = timer_alloc(TIMER_BUSY_CLEAR);
	m_timer_ld = timer_alloc(TIMER_LD_CLEAR);

	for (i = 0; i < 24; i++)
	{
		m_slots[i].num = i;
	}

	m_stream = machine().sound().stream_alloc(*this, 0, 2, clock()/768);
	m_mix_buffer = std::make_unique<INT32[]>(44100*2);

	// rate tables
	precompute_rate_tables();

	// Volume table, 1 = -0.375dB, 8 = -3dB, 256 = -96dB
	for(i = 0; i < 256; i++)
		m_volume[i] = 65536*pow(2.0, (-0.375/6)*i);
	for(i = 256; i < 256*4; i++)
		m_volume[i] = 0;

	// Pan values, units are -3dB, i.e. 8.
	for(i = 0; i < 16; i++)
	{
		m_pan_left[i] = i < 7 ? i*8 : i < 9 ? 256 : 0;
		m_pan_right[i] = i < 8 ? 0 : i < 10 ? 256 : (16-i)*8;
	}

	// Mixing levels, units are -3dB, and add some margin to avoid clipping
	for(i=0; i<7; i++)
		m_mix_level[i] = m_volume[8*i+13];
	m_mix_level[7] = 0;

	// Register state for saving
	register_save_state();

	// YMF262 related

	/* stream system initialize */
	int ymf262_clock = clock() / (19/8.0);
	m_ymf262 = ymf262_init(this, ymf262_clock, ymf262_clock / 288);
	assert_always(m_ymf262 != nullptr, "Error creating YMF262 chip");

	m_stream_ymf262 = machine().sound().stream_alloc(*this, 0, 4, ymf262_clock / 288);

	/* YMF262 setup */
	ymf262_set_timer_handler (m_ymf262, ymf278b_ymf262_timer_handler, this);
	ymf262_set_irq_handler   (m_ymf262, ymf278b_ymf262_irq_handler, this);
	ymf262_set_update_handler(m_ymf262, ymf278b_ymf262_update_request, this);
}


const device_type YMF278B = &device_creator<ymf278b_device>;

ymf278b_device::ymf278b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YMF278B, "YMF278B", tag, owner, clock, "ymf278b", __FILE__),
		device_sound_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		m_space_config("samples", ENDIANNESS_BIG, 8, 22, 0, nullptr),
		m_irq_handler(*this),
		m_last_fm_data(0)
{
	m_address_map[0] = *ADDRESS_MAP_NAME(ymf278b);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ymf278b_device::device_config_complete()
{
}
