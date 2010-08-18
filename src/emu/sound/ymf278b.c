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
   Sep. 8, 2002 - fixed ymf278b_compute_rate when OCT is negative (RB)
   Dec. 11, 2002 - added ability to set non-standard clock rates (RB)
                   fixed envelope target for release (fixes missing
           instruments in hotdebut).
                   Thanks to Team Japump! for MP3s from a real PCB.
           fixed crash if MAME is run with no sound.
   June 4, 2003 -  Changed to dual-license with LGPL for use in OpenMSX.
                   OpenMSX contributed a bugfix where looped samples were
            not being addressed properly, causing pitch fluctuation.
*/

#include "emu.h"
#include "streams.h"
#include "ymf278b.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

typedef struct
{
	INT16 wave;		/* wavetable number */
	INT16 FN;		/* f-number */
	INT8 OCT;		/* octave */
	INT8 PRVB;		/* pseudo-reverb */
	INT8 LD;		/* level direct */
	INT8 TL;		/* total level */
	INT8 pan;		/* panpot */
	INT8 lfo;		/* LFO */
	INT8 vib;		/* vibrato */
	INT8 AM;		/* AM level */

	INT8 AR;
	INT8 D1R;
	INT8 DL;
	INT8 D2R;
	INT8 RC;		/* rate correction */
	INT8 RR;

	UINT32 step;	/* fixed-point frequency step */
	UINT32 stepptr;	/* fixed-point pointer into the sample */

	INT8 active;		/* slot keyed on */
	INT8 bits;		/* width of the samples */
	UINT32 startaddr;
	UINT32 loopaddr;
	UINT32 endaddr;

	int env_step;
	UINT32 env_vol;
	UINT32 env_vol_step;
	UINT32 env_vol_lim;
} YMF278BSlot;

typedef struct
{
	YMF278BSlot slots[24];
	INT8 lsitest0;
	INT8 lsitest1;
	INT8 wavetblhdr;
	INT8 memmode;
	INT32 memadr;

	INT32 fm_l, fm_r;
	INT32 pcm_l, pcm_r;

	UINT8 timer_a_count, timer_b_count, enable, current_irq;
	emu_timer *timer_a, *timer_b;
	int irq_line;

	UINT8 port_A, port_B, port_C;
	void (*irq_callback)(running_device *, int);
	running_device *device;

	const UINT8 *rom;
	int clock;

	INT32 volume[256*4];			// precalculated attenuation values with some marging for enveloppe and pan levels
	int pan_left[16], pan_right[16];	// pan volume offsets
	INT32 mix_level[8];

	sound_stream * stream;
} YMF278BChip;

INLINE YMF278BChip *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_YMF278B);
	return (YMF278BChip *)downcast<legacy_device_base *>(device)->token();
}

static INT32 *mix;

static int ymf278b_compute_rate(YMF278BSlot *slot, int val)
{
	int res, oct;

	if(val == 0)
		return 0;
	if(val == 15)
		return 63;
	if(slot->RC != 15)
	{
		oct = slot->OCT;
		if (oct & 8) oct |= -8;

		res = (oct+slot->RC)*2 + (slot->FN & 0x200 ? 1 : 0) + val*4;
	}
	else
		res = val * 4;
	if(res < 0)
		res = 0;
	else if(res > 63)
		res = 63;
	return res;
}

static UINT32 ymf278_compute_decay_rate(int num)
{
	int samples;

	if (num <= 3)
		samples = 0;
	else if (num >= 60)
		samples = 15 << 4;
	else
	{
		samples = (15 << (21 - num / 4)) / (4 + num % 4);
		if (num % 4 && num / 4 <= 11)
			samples += 2;
		else if (num == 51)
			samples += 2;
	}

	return samples;
}

static void ymf278b_envelope_next(YMF278BSlot *slot)
{
	if(slot->env_step == 0)
	{
		// Attack
		slot->env_vol = (256U << 23) - 1;
		slot->env_vol_lim = 256U<<23;
		LOG(("YMF278B: Skipping attack (rate = %d)\n", slot->AR));
		slot->env_step++;
	}
	if(slot->env_step == 1)
	{
		// Decay 1
		slot->env_vol = 0;
		slot->env_step++;
		if(slot->DL)
		{
			int rate = ymf278b_compute_rate(slot, slot->D1R);
			LOG(("YMF278B: Decay step 1, dl=%d, val = %d rate = %d, delay = %g\n", slot->DL, slot->D1R, rate, ymf278_compute_decay_rate(rate)*1000.0));

			if(rate<4)
				slot->env_vol_step = 0;
			else
				slot->env_vol_step = ((slot->DL*8)<<23) / ymf278_compute_decay_rate(rate);
			slot->env_vol_lim = (slot->DL*8)<<23;
			return;
		}
	}
	if(slot->env_step == 2)
	{
		// Decay 2
		int rate = ymf278b_compute_rate(slot, slot->D2R);

		LOG(("YMF278B: Decay step 2, val = %d, rate = %d, delay = %g, current vol = %d\n", slot->D2R, rate, ymf278_compute_decay_rate(rate)*1000.0, slot->env_vol >> 23));
		if(rate<4)
			slot->env_vol_step = 0;
		else
			slot->env_vol_step = ((256U-slot->DL*8)<<23) / ymf278_compute_decay_rate(rate);
		slot->env_vol_lim = 256U<<23;
		slot->env_step++;
		return;
	}
	if(slot->env_step == 3)
	{
		// Decay 2 reached -96dB
		LOG(("YMF278B: Voice cleared because of decay 2\n"));
		slot->env_vol = 256U<<23;
		slot->env_vol_step = 0;
		slot->env_vol_lim = 0;
		slot->active = 0;
		return;
	}
	if(slot->env_step == 4)
	{
		// Release
		int rate = ymf278b_compute_rate(slot, slot->RR);

		LOG(("YMF278B: Release, val = %d, rate = %d, delay = %g\n", slot->RR, rate, ymf278_compute_decay_rate(rate)*1000.0));
		if(rate<4)
			slot->env_vol_step = 0;
		else
			slot->env_vol_step = ((256U<<23)-slot->env_vol) / ymf278_compute_decay_rate(rate);
		slot->env_vol_lim = 256U<<23;
		slot->env_step++;
		return;
	}
	if(slot->env_step == 5)
	{
		// Release reached -96dB
		LOG(("YMF278B: Release ends\n"));
		slot->env_vol = 256U<<23;
		slot->env_vol_step = 0;
		slot->env_vol_lim = 0;
		slot->active = 0;
		return;
	}
}

static STREAM_UPDATE( ymf278b_pcm_update )
{
	YMF278BChip *chip = (YMF278BChip *)param;
	int i, j;
	YMF278BSlot *slot = NULL;
	INT16 sample = 0;
	const UINT8 *rombase;
	INT32 *mixp;
	INT32 vl, vr;

	memset(mix, 0, sizeof(mix[0])*samples*2);

	rombase = chip->rom;

	for (i = 0; i < 24; i++)
	{
		slot = &chip->slots[i];

		if (slot->active)
		{
			mixp = mix;

			for (j = 0; j < samples; j++)
			{
				if(slot->stepptr >= slot->endaddr)
				{
					slot->stepptr = slot->stepptr - slot->endaddr + slot->loopaddr;
					// If the step is bigger than the loop, finish the sample forcibly
					if(slot->stepptr >= slot->endaddr)
					{
						slot->env_vol = 256U<<23;
						slot->env_vol_step = 0;
						slot->env_vol_lim = 0;
						slot->active = 0;
						slot->stepptr = 0;
						slot->step = 0;
					}
				}

				switch (slot->bits)
				{
					case 8: 	// 8 bit
						sample = rombase[slot->startaddr + (slot->stepptr>>16)]<<8;
						break;

					case 12:	// 12 bit
						if (slot->stepptr & 1)
							sample = rombase[slot->startaddr + (slot->stepptr>>17)*3 + 2]<<8 | ((rombase[slot->startaddr + (slot->stepptr>>17)*3 + 1] << 4) & 0xf0);
						else
							sample = rombase[slot->startaddr + (slot->stepptr>>17)*3]<<8 | (rombase[slot->startaddr + (slot->stepptr>>17)*3 + 1] & 0xf0);
						break;

					case 16:	// 16 bit
						sample = rombase[slot->startaddr + ((slot->stepptr>>16)*2)]<<8;
						sample |= rombase[slot->startaddr + ((slot->stepptr>>16)*2) + 1];
						break;
				}

				*mixp++ += (sample * chip->volume[slot->TL+chip->pan_left [slot->pan]+(slot->env_vol>>23)])>>17;
				*mixp++ += (sample * chip->volume[slot->TL+chip->pan_right[slot->pan]+(slot->env_vol>>23)])>>17;

				// update frequency
				slot->stepptr += slot->step;

				// update envelope
				slot->env_vol += slot->env_vol_step;
				if(((INT32)(slot->env_vol - slot->env_vol_lim)) >= 0)
					ymf278b_envelope_next(slot);
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

static void ymf278b_irq_check(running_machine *machine, YMF278BChip *chip)
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

static void ymf278b_timer_a_reset(YMF278BChip *chip)
{
	if(chip->enable & 1)
	{
		attotime period = ATTOTIME_IN_NSEC((256-chip->timer_a_count) * 80800);

		if (chip->clock != YMF278B_STD_CLOCK)
			period = attotime_div(attotime_mul(period, chip->clock), YMF278B_STD_CLOCK);

		timer_adjust_periodic(chip->timer_a, period, 0, period);
	}
	else
		timer_adjust_oneshot(chip->timer_a, attotime_never, 0);
}

static void ymf278b_timer_b_reset(YMF278BChip *chip)
{
	if(chip->enable & 2)
	{
		attotime period = ATTOTIME_IN_NSEC((256-chip->timer_b_count) * 323100);

		if (chip->clock != YMF278B_STD_CLOCK)
			period = attotime_div(attotime_mul(period, chip->clock), YMF278B_STD_CLOCK);

		timer_adjust_periodic(chip->timer_b, period, 0, period);
	}
	else
		timer_adjust_oneshot(chip->timer_b, attotime_never, 0);
}

static void ymf278b_A_w(running_machine *machine, YMF278BChip *chip, UINT8 reg, UINT8 data)
{
	switch(reg)
	{
		case 0x02:
			chip->timer_a_count = data;
			ymf278b_timer_a_reset(chip);
			break;
		case 0x03:
			chip->timer_b_count = data;
			ymf278b_timer_b_reset(chip);
			break;
		case 0x04:
			if(data & 0x80)
				chip->current_irq = 0;
			else
			{
				UINT8 old_enable = chip->enable;
				chip->enable = data;
				chip->current_irq &= ~data;
				if((old_enable ^ data) & 1)
					ymf278b_timer_a_reset(chip);
				if((old_enable ^ data) & 2)
					ymf278b_timer_b_reset(chip);
			}
			ymf278b_irq_check(machine, chip);
			break;
		default:
			logerror("YMF278B:  Port A write %02x, %02x\n", reg, data);
	}
}

static void ymf278b_B_w(YMF278BChip *chip, UINT8 reg, UINT8 data)
{
	logerror("YMF278B:  Port B write %02x, %02x\n", reg, data);
}

static void ymf278b_C_w(YMF278BChip *chip, UINT8 reg, UINT8 data)
{
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
				const UINT8 *p;

				slot->wave &= 0x100;
				slot->wave |= data;

				if(slot->wave < 384 || !chip->wavetblhdr)
					p = chip->rom + (slot->wave * 12);
				else
					p = chip->rom + chip->wavetblhdr*0x80000 + ((slot->wave - 384) * 12);

				switch (p[0]&0xc0)
				{
					case 0:
						slot->bits = 8;
						break;
					case 0x40:
						slot->bits = 12;
						break;
					case 0x80:
						slot->bits = 16;
						break;
				}

				slot->lfo = (p[7] >> 2) & 7;
				slot->vib = p[7] & 7;
				slot->AR = p[8] >> 4;
				slot->D1R = p[8] & 0xf;
				slot->DL = p[9] >> 4;
				slot->D2R = p[9] & 0xf;
				slot->RC = p[10] >> 4;
				slot->RR = p[10] & 0xf;
				slot->AM = p[11] & 7;

				slot->startaddr = (p[2] | (p[1]<<8) | ((p[0]&0x3f)<<16));
				slot->loopaddr = (p[4]<<16) | (p[3]<<24);
				slot->endaddr = (p[6]<<16) | (p[5]<<24);
				slot->endaddr -= 0x00010000U;
				slot->endaddr ^= 0xffff0000U;
				break;
			}
			case 1:
				slot->wave &= 0xff;
				slot->wave |= ((data&0x1)<<8);
				slot->FN &= 0x380;
				slot->FN |= (data>>1);
				break;
			case 2:
				slot->FN &= 0x07f;
				slot->FN |= ((data&0x07)<<7);
				slot->PRVB = ((data&0x4)>>3);
				slot->OCT = ((data&0xf0)>>4);
				break;
			case 3:
				slot->TL = (data>>1);
				slot->LD = data&0x1;
				break;
			case 4:
				slot->pan = data&0xf;
				if (data & 0x80)
				{
					unsigned int step;
					int oct;

					slot->active = 1;

					oct = slot->OCT;
					if(oct & 8)
						oct |= -8;

					slot->env_step = 0;
					slot->env_vol = 256U<<23;
					slot->env_vol_step = 0;
					slot->env_vol_lim = 256U<<23;
					slot->stepptr = 0;
					slot->step = 0;

					step = (slot->FN | 1024) << (oct + 7);
					slot->step = step / 4;

					ymf278b_envelope_next(slot);

					LOG(("YMF278B: slot %2d wave %3d lfo=%d vib=%d ar=%d d1r=%d dl=%d d2r=%d rc=%d rr=%d am=%d\n", snum, slot->wave,
							 slot->lfo, slot->vib, slot->AR, slot->D1R, slot->DL, slot->D2R, slot->RC, slot->RR, slot->AM));
					LOG(("                  b=%d, start=%x, loop=%x, end=%x, oct=%d, fn=%d, step=%x\n", slot->bits, slot->startaddr, slot->loopaddr>>16, slot->endaddr>>16, oct, slot->FN, slot->step));
				}
				else
				{
					LOG(("YMF278B: slot %2d off\n", snum));
					if(slot->active)
					{
						slot->env_step = 4;
						ymf278b_envelope_next(slot);
					}
				}
				break;
			case 5:
				slot->vib = data&0x7;
				slot->lfo = (data>>3)&0x7;
		    	break;
			case 6:
				slot->AR = data>>4;
				slot->D1R = data&0xf;
				break;
			case 7:
				slot->DL = data>>4;
				slot->D2R = data&0xf;
				break;
			case 8:
				slot->RC = data>>4;
				slot->RR = data&0xf;
				break;
			case 9:
				slot->AM = data & 0x7;
				break;
		}
	}
	else
	{
		// All non-slot registers
		switch (reg)
		{
			case 0x00:  	// TEST
			case 0x01:
				break;

			case 0x02:
				chip->wavetblhdr = (data>>2)&0x7;
				chip->memmode = data&1;
				break;

			case 0x03:
				chip->memadr &= 0xffff;
				chip->memadr |= (data<<16);
				break;

			case 0x04:
				chip->memadr &= 0xff00ff;
				chip->memadr |= (data<<8);
				break;

			case 0x05:
				chip->memadr &= 0xffff00;
				chip->memadr |= data;
				break;

			case 0x06:  // memory data (ignored, we don't support RAM)
			case 0x07:	// unused
				break;

			case 0xf8:
				chip->fm_l = data & 0x7;
				chip->fm_r = (data>>3)&0x7;
				break;

			case 0xf9:
				chip->pcm_l = data & 0x7;
				chip->pcm_r = (data>>3)&0x7;
				break;
		}
	}
}

READ8_DEVICE_HANDLER( ymf278b_r )
{
	YMF278BChip *chip = get_safe_token(device);

	switch (offset)
	{
		case 0:
			return chip->current_irq | (chip->irq_line == ASSERT_LINE ? 0x80 : 0x00);

		default:
			logerror("%s: unexpected read at offset %X from ymf278b\n", cpuexec_describe_context(device->machine), offset);
			break;
	}
	return 0xff;
}

WRITE8_DEVICE_HANDLER( ymf278b_w )
{
	YMF278BChip *chip = get_safe_token(device);

	switch (offset)
	{
		case 0:
			chip->port_A = data;
			break;

		case 1:
			ymf278b_A_w(device->machine, chip, chip->port_A, data);
			break;

		case 2:
			chip->port_B = data;
			break;

		case 3:
			ymf278b_B_w(chip, chip->port_B, data);
			break;

		case 4:
			chip->port_C = data;
			break;

		case 5:
			ymf278b_C_w(chip, chip->port_C, data);
			break;

		default:
			logerror("%s: unexpected write at offset %X to ymf278b = %02X\n", cpuexec_describe_context(device->machine), offset, data);
			break;
	}
}

static void ymf278b_init(running_device *device, YMF278BChip *chip, void (*cb)(running_device *, int))
{
	chip->rom = *device->region();
	chip->irq_callback = cb;
	chip->timer_a = timer_alloc(device->machine, ymf278b_timer_a_tick, chip);
	chip->timer_b = timer_alloc(device->machine, ymf278b_timer_b_tick, chip);
	chip->irq_line = CLEAR_LINE;
	chip->clock = device->clock();

	mix = auto_alloc_array(device->machine, INT32, 44100*2);
}

static void ymf278b_register_save_state(running_device *device, YMF278BChip *chip)
{
	int i;

	state_save_register_device_item(device, 0, chip->lsitest0);
	state_save_register_device_item(device, 0, chip->lsitest1);
	state_save_register_device_item(device, 0, chip->wavetblhdr);
	state_save_register_device_item(device, 0, chip->memmode);
	state_save_register_device_item(device, 0, chip->memadr);
	state_save_register_device_item(device, 0, chip->fm_l);
	state_save_register_device_item(device, 0, chip->fm_r);
	state_save_register_device_item(device, 0, chip->pcm_l);
	state_save_register_device_item(device, 0, chip->pcm_r);
	state_save_register_device_item(device, 0, chip->timer_a_count);
	state_save_register_device_item(device, 0, chip->timer_b_count);
	state_save_register_device_item(device, 0, chip->enable);
	state_save_register_device_item(device, 0, chip->current_irq);
	state_save_register_device_item(device, 0, chip->irq_line);
	state_save_register_device_item(device, 0, chip->port_A);
	state_save_register_device_item(device, 0, chip->port_B);
	state_save_register_device_item(device, 0, chip->port_C);

	for (i = 0; i < 24; ++i)
	{
		state_save_register_device_item(device, i, chip->slots[i].wave);
		state_save_register_device_item(device, i, chip->slots[i].FN);
		state_save_register_device_item(device, i, chip->slots[i].OCT);
		state_save_register_device_item(device, i, chip->slots[i].PRVB);
		state_save_register_device_item(device, i, chip->slots[i].LD);
		state_save_register_device_item(device, i, chip->slots[i].TL);
		state_save_register_device_item(device, i, chip->slots[i].pan);
		state_save_register_device_item(device, i, chip->slots[i].lfo);
		state_save_register_device_item(device, i, chip->slots[i].vib);
		state_save_register_device_item(device, i, chip->slots[i].AM);

		state_save_register_device_item(device, i, chip->slots[i].AR);
		state_save_register_device_item(device, i, chip->slots[i].D1R);
		state_save_register_device_item(device, i, chip->slots[i].DL);
		state_save_register_device_item(device, i, chip->slots[i].D2R);
		state_save_register_device_item(device, i, chip->slots[i].RC);
		state_save_register_device_item(device, i, chip->slots[i].RR);

		state_save_register_device_item(device, i, chip->slots[i].step);
		state_save_register_device_item(device, i, chip->slots[i].stepptr);

		state_save_register_device_item(device, i, chip->slots[i].active);
		state_save_register_device_item(device, i, chip->slots[i].bits);
		state_save_register_device_item(device, i, chip->slots[i].startaddr);
		state_save_register_device_item(device, i, chip->slots[i].loopaddr);
		state_save_register_device_item(device, i, chip->slots[i].endaddr);

		state_save_register_device_item(device, i, chip->slots[i].env_step);
		state_save_register_device_item(device, i, chip->slots[i].env_vol);
		state_save_register_device_item(device, i, chip->slots[i].env_vol_step);
		state_save_register_device_item(device, i, chip->slots[i].env_vol_lim);
	}
}

static DEVICE_START( ymf278b )
{
	static const ymf278b_interface defintrf = { 0 };
	const ymf278b_interface *intf;
	int i;
	YMF278BChip *chip = get_safe_token(device);

	chip->device = device;
	intf = (device->baseconfig().static_config() != NULL) ? (const ymf278b_interface *)device->baseconfig().static_config() : &defintrf;

	ymf278b_init(device, chip, intf->irq_callback);
	chip->stream = stream_create(device, 0, 2, device->clock()/768, chip, ymf278b_pcm_update);

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

	// Mixing levels, units are -3dB, and add some marging to avoid clipping
	for(i=0; i<7; i++)
		chip->mix_level[i] = chip->volume[8*i+8];
	chip->mix_level[7] = 0;

	// Register state for saving
	ymf278b_register_save_state(device, chip);
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( ymf278b )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(YMF278BChip);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( ymf278b );		break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "YMF278B");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Yamaha FM");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(YMF278B, ymf278b);
