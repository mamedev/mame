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

   Copyright R. Belmont and O. Galibert.

   This software is dual-licensed: it may be used in MAME and properly licensed
   MAME derivatives under the terms of the MAME license.  For use outside of
   MAME and properly licensed derivatives, it is available under the
   terms of the GNU Lesser General Public License (LGPL), version 2.1.
   You may read the LGPL at http://www.gnu.org/licenses/lgpl.html

   Changelog:
   Sep. 8, 2002 - fixed ymf278b_compute_rate when octave is negative (RB)
   Dec. 11, 2002 - added ability to set non-standard clock rates (RB)
                   fixed envelope target for release (fixes missing
           instruments in hotdebut).
                   Thanks to Team Japump! for MP3s from a real PCB.
           fixed crash if MAME is run with no sound.
   June 4, 2003 -  Changed to dual-license with LGPL for use in openMSX.
                   openMSX contributed a bugfix where looped samples were
            not being addressed properly, causing pitch fluctuation.

   With further improvements over the years by MAME team.

   TODO:
   - accurate timing of envelopes
   - LFO (vibrato, tremolo)
   - integrate YMF262 (used by Fuuki games, not used by Psikyo and Metro games)
   - able to hook up "Moonsound", supporting mixed ROM+RAM (for MSX driver in MESS)
*/

#include "emu.h"
#include "ymf278b.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

struct YMF278BSlot 
{
	INT16 wave;		/* wavetable number */
	INT16 F_NUMBER;	/* frequency */
	INT8 octave;	/* octave */
	INT8 preverb;	/* pseudo-reverb */
	INT8 DAMP;		/* damping */
	INT8 CH;		/* output channel */
	INT8 LD;		/* level direct */
	INT8 TL;		/* total level */
	INT8 pan;		/* panpot */
	INT8 LFO;		/* LFO */
	INT8 VIB;		/* vibrato */
	INT8 AM;		/* tremolo */

	INT8 AR;		/* attack rate */
	INT8 D1R;		/* decay 1 rate */
	INT8 DL;		/* decay level */
	INT8 D2R;		/* decay 2 rate */
	INT8 RC;		/* rate correction */
	INT8 RR;		/* release rate */

	UINT32 step;	/* fixed-point frequency step */
	UINT64 stepptr;	/* fixed-point pointer into the sample */

	INT8 active;	/* channel is playing */
	INT8 KEY_ON;	/* slot keyed on */
	INT8 bits;		/* width of the samples */
	UINT32 startaddr;
	UINT32 loopaddr;
	UINT32 endaddr;

	int env_step;
	UINT32 env_vol;
	UINT32 env_vol_step;
	UINT32 env_vol_lim;
	INT8 env_preverb;

	int num;		/* slot number (for debug only) */
	struct _YMF278BChip *chip;	/* pointer back to parent chip */
};

typedef struct _YMF278BChip
{
	UINT8 pcmregs[256];
	YMF278BSlot slots[24];
	INT8 wavetblhdr;
	INT8 memmode;
	INT32 memadr;

	UINT8 status_busy, status_ld;
	emu_timer *timer_busy;
	emu_timer *timer_ld;
	UINT8 exp;

	INT32 fm_l, fm_r;
	INT32 pcm_l, pcm_r;

	attotime timer_base;
	UINT8 timer_a_count, timer_b_count;
	UINT8 enable, current_irq;
	emu_timer *timer_a, *timer_b;
	int irq_line;

	UINT8 port_C, port_AB, lastport;
	void (*irq_callback)(device_t *, int);
	device_t *device;

	const UINT8 *rom;
	UINT32 romsize;
	int clock;

	// precomputed tables
	UINT32 lut_ar[64];				// attack rate
	UINT32 lut_dr[64];				// decay rate
	INT32 volume[256*4];			// precalculated attenuation values with some margin for envelope and pan levels
	int pan_left[16],pan_right[16];	// pan volume offsets
	INT32 mix_level[8];

	sound_stream * stream;
} YMF278BChip;

INLINE YMF278BChip *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == YMF278B);
	return (YMF278BChip *)downcast<ymf278b_device *>(device)->token();
}

static void ymf278b_write_memory(YMF278BChip *chip, UINT32 offset, UINT8 data)
{
	logerror("YMF278B:  Memory write %02x to %x\n", data, offset);
}

INLINE UINT8 ymf278b_read_memory(YMF278BChip *chip, UINT32 offset)
{
	if (offset >= chip->romsize)
	{
		// logerror("YMF278B:  Memory read overflow %x\n", offset);
		return 0xff;
	}
	return chip->rom[offset];
}


/**************************************************************************/

static int ymf278b_compute_rate(YMF278BSlot *slot, int val)
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

static UINT32 ymf278_compute_decay_env_vol_step(YMF278BSlot *slot, int val)
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
		rate = ymf278b_compute_rate(slot, val);

	if (rate < 4)
		res = 0;
	else
		res = (256U<<23) / slot->chip->lut_dr[rate];

	return res;
}

static void ymf278b_compute_freq_step(YMF278BSlot *slot)
{
	UINT32 step;
	int oct;

	oct = slot->octave;
	if(oct & 8)
		oct |= -8;

	step = (slot->F_NUMBER | 1024) << (oct + 8);
	slot->step = step >> 3;
}

static void ymf278b_compute_envelope(YMF278BSlot *slot)
{
	switch (slot->env_step)
	{
		// Attack
		case 0:
		{
			// Attack
			int rate = ymf278b_compute_rate(slot, slot->AR);
			slot->env_vol = 256U<<23;
			slot->env_vol_lim = (256U<<23) - 1;

			if (rate==63)
			{
				// immediate
				LOG(("YMF278B: Attack skipped - "));
				slot->env_vol = 0;
				slot->env_step++;
				ymf278b_compute_envelope(slot);
			}
			else if (rate<4)
			{
				slot->env_vol_step = 0;
			}
			else
			{
				// NOTE: attack rate is linear here, but datasheet shows a smooth curve
				LOG(("YMF278B: Attack, val = %d, rate = %d, delay = %g\n", slot->AR, rate, slot->chip->lut_ar[rate]*1000.0));
				slot->env_vol_step = ~((256U<<23) / slot->chip->lut_ar[rate]);
			}

			break;
		}

		// Decay 1
		case 1:
			if(slot->DL)
			{
				LOG(("YMF278B: Decay step 1, dl=%d, val = %d rate = %d, delay = %g, PRVB = %d, DAMP = %d\n", slot->DL, slot->D1R, ymf278b_compute_rate(slot, slot->D1R), slot->chip->lut_dr[ymf278b_compute_rate(slot, slot->D1R)]*1000.0, slot->preverb, slot->DAMP));
				slot->env_vol_step = ymf278_compute_decay_env_vol_step(slot, slot->D1R);
				slot->env_vol_lim = (slot->DL*8)<<23;
			}
			else
			{
				LOG(("YMF278B: Decay 1 skipped - "));
				slot->env_step++;
				ymf278b_compute_envelope(slot);
			}

			break;

		// Decay 2
		case 2:
			LOG(("YMF278B: Decay step 2, val = %d, rate = %d, delay = %g, , PRVB = %d, DAMP = %d, current vol = %d\n", slot->D2R, ymf278b_compute_rate(slot, slot->D2R), slot->chip->lut_dr[ymf278b_compute_rate(slot, slot->D2R)]*1000.0, slot->preverb, slot->DAMP, slot->env_vol >> 23));
			slot->env_vol_step = ymf278_compute_decay_env_vol_step(slot, slot->D2R);
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
			LOG(("YMF278B: Release, val = %d, rate = %d, delay = %g, PRVB = %d, DAMP = %d\n", slot->RR, ymf278b_compute_rate(slot, slot->RR), slot->chip->lut_dr[ymf278b_compute_rate(slot, slot->RR)]*1000.0, slot->preverb, slot->DAMP));
			slot->env_vol_step = ymf278_compute_decay_env_vol_step(slot, slot->RR);
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

static STREAM_UPDATE( ymf278b_pcm_update )
{
	YMF278BChip *chip = (YMF278BChip *)param;
	int i, j;
	YMF278BSlot *slot = NULL;
	INT16 sample = 0;
	INT32 *mixp;
	INT32 vl, vr;
	INT32 mix[44100*2];

	memset(mix, 0, sizeof(mix[0])*samples*2);

	for (i = 0; i < 24; i++)
	{
		slot = &chip->slots[i];

		if (slot->active)
		{
			mixp = mix;

			for (j = 0; j < samples; j++)
			{
				if (slot->stepptr >= slot->endaddr)
				{
					slot->stepptr = slot->stepptr - slot->endaddr + slot->loopaddr;
					if (slot->stepptr >= slot->endaddr)
						slot->stepptr = slot->loopaddr; // loop overflow
				}

				switch (slot->bits)
				{
					// 8 bit
					case 0:
						sample = ymf278b_read_memory(chip, slot->startaddr + (slot->stepptr>>16))<<8;
						break;

					// 12 bit
					case 1:
						if (slot->stepptr & 0x10000)
							sample = ymf278b_read_memory(chip, slot->startaddr + (slot->stepptr>>17)*3+2)<<8 |
								(ymf278b_read_memory(chip, slot->startaddr + (slot->stepptr>>17)*3+1) << 4 & 0xf0);
						else
							sample = ymf278b_read_memory(chip, slot->startaddr + (slot->stepptr>>17)*3)<<8 |
								(ymf278b_read_memory(chip, slot->startaddr + (slot->stepptr>>17)*3+1) & 0xf0);
						break;

					// 16 bit
					case 2:
						sample = ymf278b_read_memory(chip, slot->startaddr + ((slot->stepptr>>16)*2))<<8 |
							ymf278b_read_memory(chip, slot->startaddr + ((slot->stepptr>>16)*2)+1);
						break;

					// ?? bit, effect is unknown, datasheet says it's prohibited
					case 3:
						sample = 0;
						break;
				}

				*mixp++ += (sample * chip->volume[slot->TL+chip->pan_left [slot->pan]+(slot->env_vol>>23)])>>17;
				*mixp++ += (sample * chip->volume[slot->TL+chip->pan_right[slot->pan]+(slot->env_vol>>23)])>>17;

				// update frequency
				slot->stepptr += slot->step;

				// update envelope
				slot->env_vol += slot->env_vol_step;
				if (((INT32)(slot->env_vol - slot->env_vol_lim)) >= 0)
				{
					slot->env_step++;
					ymf278b_compute_envelope(slot);
				}
				else if (slot->preverb && !slot->env_preverb && slot->env_step && slot->env_vol > ((6*8)<<23))
					ymf278b_compute_envelope(slot);
			}
		}
	}

	mixp = mix;
	vl = chip->mix_level[chip->pcm_l];
	vr = chip->mix_level[chip->pcm_r];
	for (i = 0; i < samples; i++)
	{
		outputs[0][i] = (*mixp++ * vl) >> 16;
		outputs[1][i] = (*mixp++ * vr) >> 16;
	}
}

static void ymf278b_irq_check(running_machine &machine, YMF278BChip *chip)
{
	int prev_line = chip->irq_line;
	chip->irq_line = chip->current_irq ? ASSERT_LINE : CLEAR_LINE;
	if(chip->irq_line != prev_line && chip->irq_callback)
		chip->irq_callback(chip->device, chip->irq_line);
}

static TIMER_CALLBACK( ymf278b_timer_a_tick )
{
	YMF278BChip *chip = (YMF278BChip *)ptr;
	if(!(chip->enable & 0x40))
	{
		chip->current_irq |= 0x40;
		ymf278b_irq_check(machine, chip);
	}
}

static TIMER_CALLBACK( ymf278b_timer_b_tick )
{
	YMF278BChip *chip = (YMF278BChip *)ptr;
	if(!(chip->enable & 0x20))
	{
		chip->current_irq |= 0x20;
		ymf278b_irq_check(machine, chip);
	}
}


/**************************************************************************/

static void ymf278b_A_w(running_machine &machine, YMF278BChip *chip, UINT8 reg, UINT8 data)
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
			if (data != chip->timer_a_count)
			{
				chip->timer_a_count = data;

				// change period, ~80.8us * t
				if (chip->enable & 1)
					chip->timer_a->adjust(chip->timer_a->remaining(), 0, chip->timer_base * (256-data) * 4);
			}
			break;

		// timer b count
		case 0x03:
			if (data != chip->timer_b_count)
			{
				chip->timer_b_count = data;

				// change period, ~323.1us * t
				if (chip->enable & 2)
					chip->timer_b->adjust(chip->timer_b->remaining(), 0, chip->timer_base * (256-data) * 16);
			}
			break;

		// timer control
		case 0x04:
			if(data & 0x80)
				chip->current_irq = 0;
			else
			{
				// reset timers
				if((chip->enable ^ data) & 1)
				{
					attotime period = (data & 1) ? chip->timer_base * (256-chip->timer_a_count) * 4 : attotime::never;
					chip->timer_a->adjust(period, 0, period);
				}
				if((chip->enable ^ data) & 2)
				{
					attotime period = (data & 2) ? chip->timer_base * (256-chip->timer_b_count) * 16 : attotime::never;
					chip->timer_b->adjust(period, 0, period);
				}

				chip->enable = data;
				chip->current_irq &= ~data;
			}
			ymf278b_irq_check(machine, chip);
			break;

		default:
			logerror("YMF278B:  Port A write %02x, %02x\n", reg, data);
			break;
	}
}

static void ymf278b_B_w(YMF278BChip *chip, UINT8 reg, UINT8 data)
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
			chip->exp = data;
			break;

		default:
			logerror("YMF278B:  Port B write %02x, %02x\n", reg, data);
			break;
	}
}

static TIMER_CALLBACK( ymf278b_timer_ld_clear )
{
	YMF278BChip *chip = (YMF278BChip *)ptr;
	chip->status_ld = 0;
}

static void ymf278b_retrigger_note(YMF278BSlot *slot)
{
	// activate channel
	if (slot->octave != 8)
		slot->active = 1;

	// reset sample pos and go to attack stage
	slot->stepptr = 0;
	slot->env_step = 0;
	slot->env_preverb = 0;

	ymf278b_compute_freq_step(slot);
	ymf278b_compute_envelope(slot);
}

static void ymf278b_C_w(YMF278BChip *chip, UINT8 reg, UINT8 data, int init)
{
	if (!init)
	{
		// PCM regs are only accessible if NEW2 is set
		if (~chip->exp & 2)
			return;

		chip->stream->update();
	}

	// Handle slot registers specifically
	if (reg >= 0x08 && reg <= 0xf7)
	{
		YMF278BSlot *slot = NULL;
		int snum;
		snum = (reg-8) % 24;
		slot = &chip->slots[snum];
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
				if(slot->wave < 384 || !chip->wavetblhdr)
					offset = slot->wave * 12;
				else
					offset = chip->wavetblhdr*0x80000 + (slot->wave - 384) * 12;
				for (i = 0; i < 12; i++)
					p[i] = ymf278b_read_memory(chip, offset+i);

				slot->bits = (p[0]&0xc0)>>6;
				slot->startaddr = (p[2] | (p[1]<<8) | ((p[0]&0x3f)<<16));
				slot->loopaddr = (p[4]<<16) | (p[3]<<24);
				slot->endaddr = (p[6]<<16) | (p[5]<<24);
				slot->endaddr -= 0x00010000U;
				slot->endaddr ^= 0xffff0000U;

				// copy internal registers data
				for (i = 7; i < 12; i++)
					ymf278b_C_w(chip, 8 + snum + (i-2) * 24, p[i], 1);

				// status register LD bit is on for approx 300us
				chip->status_ld = 1;
				period = attotime::from_usec(300);
				if (chip->clock != YMF278B_STD_CLOCK)
					period = (period * chip->clock) / YMF278B_STD_CLOCK;
				chip->timer_ld->adjust(period);

				// retrigger if key is on
				if (slot->KEY_ON)
					ymf278b_retrigger_note(slot);
				else if (slot->active)
				{
					// deactivate channel
					slot->env_step = 5;
					ymf278b_compute_envelope(slot);
				}

				break;
			}

			case 1:
				slot->wave &= 0xff;
				slot->wave |= ((data&0x1)<<8);
				slot->F_NUMBER &= 0x380;
				slot->F_NUMBER |= (data>>1);
				if (slot->active && (data ^ chip->pcmregs[reg]) & 0xfe)
				{
					ymf278b_compute_freq_step(slot);
					ymf278b_compute_envelope(slot);
				}
				break;

			case 2:
				slot->F_NUMBER &= 0x07f;
				slot->F_NUMBER |= ((data&0x07)<<7);
				slot->preverb = (data&0x8)>>3;
				slot->octave = (data&0xf0)>>4;
				if (data != chip->pcmregs[reg])
				{
					// channel goes off if octave is set to -8 (datasheet says it's prohibited)
					// (it is ok if this activates the channel while it was off: ymf278b_compute_envelope will reset it again if needed)
					slot->active = (slot->octave != 8);

					if (slot->active)
					{
						slot->env_preverb = 0;
						ymf278b_compute_freq_step(slot);
						ymf278b_compute_envelope(slot);
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
						if ((data ^ chip->pcmregs[reg]) & 0x40)
							ymf278b_compute_envelope(slot);

						break;
					}

					ymf278b_retrigger_note(slot);
				}
				else if (slot->active)
				{
					// release
					slot->env_step = 4;
					ymf278b_compute_envelope(slot);
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
				if (slot->active && data != chip->pcmregs[reg])
					ymf278b_compute_envelope(slot);
				break;

			case 7:
				slot->DL = data>>4;
				slot->D2R = data&0xf;
				if (slot->active && data != chip->pcmregs[reg])
					ymf278b_compute_envelope(slot);
				break;

			case 8:
				slot->RC = data>>4;
				slot->RR = data&0xf;
				if (slot->active && data != chip->pcmregs[reg])
					ymf278b_compute_envelope(slot);
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
				chip->wavetblhdr = (data>>2)&0x7;
				chip->memmode = data&3;
				break;

			case 0x03:
			case 0x04:
				break;
			case 0x05:
				// set memory address
				chip->memadr = (chip->pcmregs[3] & 0x3f) << 16 | chip->pcmregs[4] << 8 | data;
				break;

			case 0x06:
				// memory data (ignored, we don't support RAM)
				ymf278b_write_memory(chip, chip->memadr, data);
				chip->memadr = (chip->memadr + 1) & 0x3fffff;
				break;

			case 0x07:
				break; // unused

			case 0xf8:
				chip->fm_l = data & 0x7;
				chip->fm_r = (data>>3)&0x7;
				break;

			case 0xf9:
				chip->pcm_l = data & 0x7;
				chip->pcm_r = (data>>3)&0x7;
				break;

			default:
				logerror("YMF278B:  Port C write %02x, %02x\n", reg, data);
				break;
		}
	}

	chip->pcmregs[reg] = data;
}

static TIMER_CALLBACK( ymf278b_timer_busy_clear )
{
	YMF278BChip *chip = (YMF278BChip *)ptr;
	chip->status_busy = 0;
}

static void ymf278b_timer_busy_start(YMF278BChip *chip, int is_pcm)
{
	// status register BUSY bit is on for 56(FM) or 88(PCM) cycles
	chip->status_busy = 1;
	chip->timer_busy->adjust(attotime::from_hz(chip->clock / (is_pcm ? 88 : 56)));
}

WRITE8_DEVICE_HANDLER( ymf278b_w )
{
	YMF278BChip *chip = get_safe_token(device);

	switch (offset)
	{
		case 0:
		case 2:
			ymf278b_timer_busy_start(chip, 0);
			chip->port_AB = data;
			chip->lastport = offset>>1 & 1;
			break;

		case 1:
		case 3:
			ymf278b_timer_busy_start(chip, 0);
			if (chip->lastport) ymf278b_B_w(chip, chip->port_AB, data);
			else ymf278b_A_w(device->machine(), chip, chip->port_AB, data);
			break;

		case 4:
			ymf278b_timer_busy_start(chip, 1);
			chip->port_C = data;
			break;

		case 5:
			ymf278b_timer_busy_start(chip, 1);
			ymf278b_C_w(chip, chip->port_C, data, 0);
			break;

		default:
			logerror("%s: unexpected write at offset %X to ymf278b = %02X\n", device->machine().describe_context(), offset, data);
			break;
	}
}


READ8_DEVICE_HANDLER( ymf278b_r )
{
	YMF278BChip *chip = get_safe_token(device);
	UINT8 ret = 0;

	switch (offset)
	{
		// status register
		case 0:
		{
			// bits 0 and 1 are only valid if NEW2 is set
			UINT8 newbits = 0;
			if (chip->exp & 2)
				newbits = (chip->status_ld << 1) | chip->status_busy;

			ret = newbits | chip->current_irq | (chip->irq_line == ASSERT_LINE ? 0x80 : 0x00);
			break;
		}

		// FM regs can be read too (on contrary to what the datasheet says)
		case 1:
		case 3:
			// but they're not implemented here yet
			break;

		// PCM regs
		case 5:
			// only accessible if NEW2 is set
			if (~chip->exp & 2)
				break;

			switch (chip->port_C)
			{
				// special cases
				case 2:
					ret = (chip->pcmregs[chip->port_C] & 0x1f) | 0x20; // device ID in upper bits
					break;
				case 6:
					ret = ymf278b_read_memory(chip, chip->memadr);
					chip->memadr = (chip->memadr + 1) & 0x3fffff;
					break;

				default:
					ret = chip->pcmregs[chip->port_C];
					break;
			}
			break;

		default:
			logerror("%s: unexpected read at offset %X from ymf278b\n", device->machine().describe_context(), offset);
			break;
	}

	return ret;
}


/**************************************************************************/

static DEVICE_RESET( ymf278b )
{
	YMF278BChip *chip = get_safe_token(device);
	int i;

	// clear registers
	for (i = 0; i <= 4; i++)
		ymf278b_A_w(device->machine(), chip, i, 0);
	ymf278b_B_w(chip, 5, 0);
	for (i = 0; i < 8; i++)
		ymf278b_C_w(chip, i, 0, 1);
	for (i = 0xff; i >= 8; i--)
		ymf278b_C_w(chip, i, 0, 1);
	ymf278b_C_w(chip, 0xf8, 0x1b, 1);

	chip->port_AB = chip->port_C = 0;
	chip->lastport = 0;
	chip->memadr = 0;

	// init/silence channels
	for (i = 0; i < 24 ; i++)
	{
		YMF278BSlot *slot = &chip->slots[i];

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
		ymf278b_compute_envelope(slot);
	}

	chip->timer_a->reset();
	chip->timer_b->reset();
	chip->timer_busy->reset();	chip->status_busy = 0;
	chip->timer_ld->reset();	chip->status_ld = 0;

	chip->irq_line = CLEAR_LINE;
}

static void ymf278b_init(device_t *device, YMF278BChip *chip, void (*cb)(device_t *, int))
{
	int i;

	chip->rom = *device->region();
	chip->romsize = device->region()->bytes();
	chip->clock = device->clock();
	chip->irq_callback = cb;

	chip->timer_base = attotime::from_hz(chip->clock) * (19*36);
	chip->timer_a = device->machine().scheduler().timer_alloc(FUNC(ymf278b_timer_a_tick), chip);
	chip->timer_b = device->machine().scheduler().timer_alloc(FUNC(ymf278b_timer_b_tick), chip);
	chip->timer_busy = device->machine().scheduler().timer_alloc(FUNC(ymf278b_timer_busy_clear), chip);
	chip->timer_ld = device->machine().scheduler().timer_alloc(FUNC(ymf278b_timer_ld_clear), chip);

	for (i = 0; i < 24; i++)
	{
		chip->slots[i].num = i;
		chip->slots[i].chip = chip;
	}
}

static void precompute_rate_tables(YMF278BChip *chip)
{
	int i;

	// decay rate
	for (i = 0; i < 64; i++)
	{
		if (i <= 3)
			chip->lut_dr[i] = 0;
		else if (i >= 60)
			chip->lut_dr[i] = 15 << 4;
		else
			chip->lut_dr[i] = (15 << (21 - i / 4)) / (4 + i % 4);
	}

	// attack rate (manual shows curve instead of linear though, so this is not entirely accurate)
	for (i = 0; i < 64; i++)
	{
		if (i <= 3 || i == 63)
			chip->lut_ar[i] = 0;
		else if (i >= 60)
			chip->lut_ar[i] = 17;
		else
			chip->lut_ar[i] = (67 << (15 - i / 4)) / (4 + i % 4);
	}
}

static void ymf278b_register_save_state(device_t *device, YMF278BChip *chip)
{
	int i;

	device->save_item(NAME(chip->pcmregs));
	device->save_item(NAME(chip->wavetblhdr));
	device->save_item(NAME(chip->memmode));
	device->save_item(NAME(chip->memadr));
	device->save_item(NAME(chip->status_busy));
	device->save_item(NAME(chip->status_ld));
	device->save_item(NAME(chip->exp));
	device->save_item(NAME(chip->fm_l));
	device->save_item(NAME(chip->fm_r));
	device->save_item(NAME(chip->pcm_l));
	device->save_item(NAME(chip->pcm_r));
	device->save_item(NAME(chip->timer_a_count));
	device->save_item(NAME(chip->timer_b_count));
	device->save_item(NAME(chip->enable));
	device->save_item(NAME(chip->current_irq));
	device->save_item(NAME(chip->irq_line));
	device->save_item(NAME(chip->port_AB));
	device->save_item(NAME(chip->port_C));
	device->save_item(NAME(chip->lastport));

	for (i = 0; i < 24; ++i)
	{
		device->save_item(NAME(chip->slots[i].wave), i);
		device->save_item(NAME(chip->slots[i].F_NUMBER), i);
		device->save_item(NAME(chip->slots[i].octave), i);
		device->save_item(NAME(chip->slots[i].preverb), i);
		device->save_item(NAME(chip->slots[i].DAMP), i);
		device->save_item(NAME(chip->slots[i].CH), i);
		device->save_item(NAME(chip->slots[i].LD), i);
		device->save_item(NAME(chip->slots[i].TL), i);
		device->save_item(NAME(chip->slots[i].pan), i);
		device->save_item(NAME(chip->slots[i].LFO), i);
		device->save_item(NAME(chip->slots[i].VIB), i);
		device->save_item(NAME(chip->slots[i].AM), i);

		device->save_item(NAME(chip->slots[i].AR), i);
		device->save_item(NAME(chip->slots[i].D1R), i);
		device->save_item(NAME(chip->slots[i].DL), i);
		device->save_item(NAME(chip->slots[i].D2R), i);
		device->save_item(NAME(chip->slots[i].RC), i);
		device->save_item(NAME(chip->slots[i].RR), i);

		device->save_item(NAME(chip->slots[i].step), i);
		device->save_item(NAME(chip->slots[i].stepptr), i);

		device->save_item(NAME(chip->slots[i].active), i);
		device->save_item(NAME(chip->slots[i].KEY_ON), i);
		device->save_item(NAME(chip->slots[i].bits), i);
		device->save_item(NAME(chip->slots[i].startaddr), i);
		device->save_item(NAME(chip->slots[i].loopaddr), i);
		device->save_item(NAME(chip->slots[i].endaddr), i);

		device->save_item(NAME(chip->slots[i].env_step), i);
		device->save_item(NAME(chip->slots[i].env_vol), i);
		device->save_item(NAME(chip->slots[i].env_vol_step), i);
		device->save_item(NAME(chip->slots[i].env_vol_lim), i);
		device->save_item(NAME(chip->slots[i].env_preverb), i);
	}
}

static DEVICE_START( ymf278b )
{
	static const ymf278b_interface defintrf = { 0 };
	const ymf278b_interface *intf;
	int i;
	YMF278BChip *chip = get_safe_token(device);

	chip->device = device;
	intf = (device->static_config() != NULL) ? (const ymf278b_interface *)device->static_config() : &defintrf;

	ymf278b_init(device, chip, intf->irq_callback);
	chip->stream = device->machine().sound().stream_alloc(*device, 0, 2, device->clock()/768, chip, ymf278b_pcm_update);

	// rate tables
	precompute_rate_tables(chip);

	// Volume table, 1 = -0.375dB, 8 = -3dB, 256 = -96dB
	for(i = 0; i < 256; i++)
		chip->volume[i] = 65536*pow(2.0, (-0.375/6)*i);
	for(i = 256; i < 256*4; i++)
		chip->volume[i] = 0;

	// Pan values, units are -3dB, i.e. 8.
	for(i = 0; i < 16; i++)
	{
		chip->pan_left[i] = i < 7 ? i*8 : i < 9 ? 256 : 0;
		chip->pan_right[i] = i < 8 ? 0 : i < 10 ? 256 : (16-i)*8;
	}

	// Mixing levels, units are -3dB, and add some margin to avoid clipping
	for(i=0; i<7; i++)
		chip->mix_level[i] = chip->volume[8*i+13];
	chip->mix_level[7] = 0;

	// Register state for saving
	ymf278b_register_save_state(device, chip);
}


const device_type YMF278B = &device_creator<ymf278b_device>;

ymf278b_device::ymf278b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YMF278B, "YMF278B", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(YMF278BChip));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ymf278b_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ymf278b_device::device_start()
{
	DEVICE_START_NAME( ymf278b )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ymf278b_device::device_reset()
{
	DEVICE_RESET_NAME( ymf278b )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ymf278b_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


