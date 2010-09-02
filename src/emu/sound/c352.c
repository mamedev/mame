/*
    c352.c - Namco C352 custom PCM chip emulation
    v1.1
    By R. Belmont
    Additional code by cync and the hoot development team

    Thanks to Cap of VivaNonno for info and The_Author for preliminary reverse-engineering

    Chip specs:
    32 voices
    Supports 8-bit linear and 8-bit muLaw samples
    Output: digital, 16 bit, 4 channels
    Output sample rate is the input clock / 384.
 */

#include "emu.h"
#include "streams.h"
#include "c352.h"

#define VERBOSE (0)
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

// flags

enum {
	C352_FLG_BUSY		= 0x8000,	// channel is busy
	C352_FLG_KEYON		= 0x4000,	// Keyon
	C352_FLG_KEYOFF		= 0x2000,	// Keyoff
	C352_FLG_LOOPTRG	= 0x1000,	// Loop Trigger
	C352_FLG_LOOPHIST	= 0x0800,	// Loop History
	C352_FLG_FM			= 0x0400,	// Frequency Modulation
	C352_FLG_PHASERL	= 0x0200,	// Rear Left invert phase 180 degrees
	C352_FLG_PHASEFL	= 0x0100,	// Front Left invert phase 180 degrees
	C352_FLG_PHASEFR	= 0x0080,	// invert phase 180 degrees (e.g. flip sign of sample)
	C352_FLG_LDIR		= 0x0040,	// loop direction
	C352_FLG_LINK		= 0x0020,	// "long-format" sample (can't loop, not sure what else it means)
	C352_FLG_NOISE		= 0x0010,	// play noise instead of sample
	C352_FLG_MULAW		= 0x0008,	// sample is mulaw instead of linear 8-bit PCM
	C352_FLG_FILTER		= 0x0004,	// don't apply filter
	C352_FLG_REVLOOP	= 0x0003,	// loop backwards
	C352_FLG_LOOP		= 0x0002,	// loop forward
	C352_FLG_REVERSE	= 0x0001,	// play sample backwards
};

typedef struct
{
	UINT8	vol_l;
	UINT8	vol_r;
	UINT8	vol_l2;
	UINT8	vol_r2;
	UINT8	bank;
	INT16	noise;
	INT16   noisebuf;
	UINT16  noisecnt;
	UINT16	pitch;
	UINT16	start_addr;
	UINT16	end_addr;
	UINT16	repeat_addr;
	UINT32	flag;

	UINT16	start;
	UINT16	repeat;
	UINT32	current_addr;
	UINT32	pos;
} c352_ch_t;

typedef struct _c352_state c352_state;
struct _c352_state
{
	sound_stream *stream;
	c352_ch_t c352_ch[32];
	unsigned char *c352_rom_samples;
	UINT32 c352_rom_length;
	int sample_rate_base;

	long	channel_l[2048*2];
	long	channel_r[2048*2];
	long	channel_l2[2048*2];
	long	channel_r2[2048*2];

	short	mulaw_table[256];
	unsigned int mseq_reg;
};

INLINE c352_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == C352);
	return (c352_state *)downcast<legacy_device_base *>(device)->token();
}

// noise generator
static int get_mseq_bit(c352_state *info)
{
	unsigned int mask = (1 << (7 - 1));
	unsigned int reg = info->mseq_reg;
	unsigned int bit = reg & (1 << (17 - 1));

	if (bit)
	{
    		reg = ((reg ^ mask) << 1) | 1;
	}
	else
	{
		reg = reg << 1;
	}

	info->mseq_reg = reg;

	return (reg & 1);
}

static void c352_mix_one_channel(c352_state *info, unsigned long ch, long sample_count)
{
	int i;

	signed short sample, nextsample;
	signed short noisebuf;
	UINT16 noisecnt;
	INT32 frequency, delta, offset, cnt, flag;
	UINT32 bank;
	UINT32 pos, len;

	frequency = info->c352_ch[ch].pitch;
	delta=frequency;

	pos = info->c352_ch[ch].current_addr;	// sample pointer
	offset = info->c352_ch[ch].pos;	// 16.16 fixed-point offset into the sample
	flag = info->c352_ch[ch].flag;
	bank = info->c352_ch[ch].bank << 16;

	noisecnt = info->c352_ch[ch].noisecnt;
	noisebuf = info->c352_ch[ch].noisebuf;

	len = info->c352_rom_length;
	for(i = 0 ; (i < sample_count) && (flag & C352_FLG_BUSY) ; i++)
	{
		offset += delta;
		cnt = (offset>>16)&0x7fff;
		if (cnt)			// if there is a whole sample part, chop it off now that it's been applied
		{
			offset &= 0xffff;
		}

		if (pos > len)
		{
			info->c352_ch[ch].flag &= ~C352_FLG_BUSY;
			return;
		}

		sample = (char)info->c352_rom_samples[pos];
		nextsample = (char)info->c352_rom_samples[pos+cnt];

		// sample is muLaw, not 8-bit linear (Fighting Layer uses this extensively)
		if (flag & C352_FLG_MULAW)
		{
			sample = info->mulaw_table[(unsigned char)sample];
			nextsample = info->mulaw_table[(unsigned char)nextsample];
		}
		else
		{
			sample <<= 8;
			nextsample <<= 8;
		}

		// play noise instead of sample data
		if (flag & C352_FLG_NOISE)
		{
			int noise_level = 0x8000;
			sample = info->c352_ch[ch].noise = (info->c352_ch[ch].noise << 1) | get_mseq_bit(info);
			sample = (sample & (noise_level - 1)) - (noise_level >> 1);
			if (sample > 0x7f)
			{
				sample = 0x7f;
			}
			else if (sample < 0)
			{
				sample = 0xff;
			}
			sample = info->mulaw_table[(unsigned char)sample];

			if ( (pos+cnt) == pos )
			{
				noisebuf += sample;
				noisecnt++;
				sample = noisebuf / noisecnt;
			}
			else
			{
				if ( noisecnt )
				{
					sample = noisebuf / noisecnt;
				}
				else
				{
					sample = info->mulaw_table[0x7f];		// Nearest sound(s) is here.
				}
				noisebuf = 0;
				noisecnt = ( flag & C352_FLG_FILTER ) ? 0 : 1;
			}
		}

		// apply linear interpolation
		if ( (flag & (C352_FLG_FILTER | C352_FLG_NOISE)) == 0 )
		{
			sample = (short)(sample + ((nextsample-sample) * (((double)(0x0000ffff&offset) )/0x10000)));
		}

		if ( flag & C352_FLG_PHASEFL )
		{
			info->channel_l[i]  += ((-sample * info->c352_ch[ch].vol_l)>>8);
		}
		else
		{
			info->channel_l[i] += ((sample * info->c352_ch[ch].vol_l)>>8);
		}

		if ( flag & C352_FLG_PHASEFR )
		{
			info->channel_r[i]  += ((-sample * info->c352_ch[ch].vol_r)>>8);
		}
		else
		{
			info->channel_r[i] += ((sample * info->c352_ch[ch].vol_r)>>8);
		}

		if ( flag & C352_FLG_PHASERL )
		{
			info->channel_l2[i] += ((-sample * info->c352_ch[ch].vol_l2)>>8);
		}
		else
		{
			info->channel_l2[i] += ((sample * info->c352_ch[ch].vol_l2)>>8);
		}
		info->channel_r2[i] += ((sample * info->c352_ch[ch].vol_r2)>>8);

		if ( (flag & C352_FLG_REVERSE) && (flag & C352_FLG_LOOP) )
		{
			if ( !(flag & C352_FLG_LDIR) )
			{
				pos += cnt;
				if (
					(((pos&0xFFFF) > info->c352_ch[ch].end_addr) && ((pos&0xFFFF) < info->c352_ch[ch].start) && (info->c352_ch[ch].start > info->c352_ch[ch].end_addr) ) ||
					(((pos&0xFFFF) > info->c352_ch[ch].end_addr) && ((pos&0xFFFF) > info->c352_ch[ch].start) && (info->c352_ch[ch].start < info->c352_ch[ch].end_addr) ) ||
					((pos > (bank|0xFFFF)) && (info->c352_ch[ch].end_addr == 0xFFFF))
					)
				{
					info->c352_ch[ch].flag |= C352_FLG_LDIR;
					info->c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
			}
			else
			{
				pos -= cnt;
				if (
					(((pos&0xFFFF) < info->c352_ch[ch].repeat) && ((pos&0xFFFF) < info->c352_ch[ch].end_addr) && (info->c352_ch[ch].end_addr > info->c352_ch[ch].start) ) ||
					(((pos&0xFFFF) < info->c352_ch[ch].repeat) && ((pos&0xFFFF) > info->c352_ch[ch].end_addr) && (info->c352_ch[ch].end_addr < info->c352_ch[ch].start) ) ||
					((pos < bank) && (info->c352_ch[ch].repeat == 0x0000))
					)
				{
					info->c352_ch[ch].flag &= ~C352_FLG_LDIR;
					info->c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
			}
		}
		else if ( flag & C352_FLG_REVERSE )
		{
			pos -= cnt;
			if (
				(((pos&0xFFFF) < info->c352_ch[ch].end_addr) && ((pos&0xFFFF) < info->c352_ch[ch].start) && (info->c352_ch[ch].start > info->c352_ch[ch].end_addr) ) ||
				(((pos&0xFFFF) < info->c352_ch[ch].end_addr) && ((pos&0xFFFF) > info->c352_ch[ch].start) && (info->c352_ch[ch].start < info->c352_ch[ch].end_addr) ) ||
				((pos < bank) && (info->c352_ch[ch].end_addr == 0x0000))
				)
			{
				if ( (flag & C352_FLG_LINK) && (flag & C352_FLG_LOOP) )
				{
					info->c352_ch[ch].bank = info->c352_ch[ch].start_addr & 0xFF;
					info->c352_ch[ch].start_addr = info->c352_ch[ch].repeat_addr;
					info->c352_ch[ch].start = info->c352_ch[ch].start_addr;
					info->c352_ch[ch].repeat = info->c352_ch[ch].repeat_addr;
					pos = (info->c352_ch[ch].bank<<16) + info->c352_ch[ch].start_addr;
					info->c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
				else if (flag & C352_FLG_LOOP)
				{
					pos = (pos & 0xFF0000) + info->c352_ch[ch].repeat;
					info->c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
				else
				{
					info->c352_ch[ch].flag |= C352_FLG_KEYOFF;
					info->c352_ch[ch].flag &= ~C352_FLG_BUSY;
					return;
				}
			}
		} else {
			pos += cnt;
			if (
				(((pos&0xFFFF) > info->c352_ch[ch].end_addr) && ((pos&0xFFFF) < info->c352_ch[ch].start) && (info->c352_ch[ch].start > info->c352_ch[ch].end_addr) ) ||
				(((pos&0xFFFF) > info->c352_ch[ch].end_addr) && ((pos&0xFFFF) > info->c352_ch[ch].start) && (info->c352_ch[ch].start < info->c352_ch[ch].end_addr) ) ||
				((pos > (bank|0xFFFF)) && (info->c352_ch[ch].end_addr == 0xFFFF))
				)
			{
				if ( (flag & C352_FLG_LINK) && (flag & C352_FLG_LOOP) )
				{
					info->c352_ch[ch].bank = info->c352_ch[ch].start_addr & 0xFF;
					info->c352_ch[ch].start_addr = info->c352_ch[ch].repeat_addr;
					info->c352_ch[ch].start = info->c352_ch[ch].start_addr;
					info->c352_ch[ch].repeat = info->c352_ch[ch].repeat_addr;
					pos = (info->c352_ch[ch].bank<<16) + info->c352_ch[ch].start_addr;
					info->c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
				else if (flag & C352_FLG_LOOP)
				{
					pos = (pos & 0xFF0000) + info->c352_ch[ch].repeat;
					info->c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
				else
				{
					info->c352_ch[ch].flag |= C352_FLG_KEYOFF;
					info->c352_ch[ch].flag &= ~C352_FLG_BUSY;
					return;
				}
			}
		}
	}

	info->c352_ch[ch].noisecnt = noisecnt;
	info->c352_ch[ch].noisebuf = noisebuf;
	info->c352_ch[ch].pos = offset;
	info->c352_ch[ch].current_addr = pos;
}


static STREAM_UPDATE( c352_update )
{
	c352_state *info = (c352_state *)param;
	int i, j;
	stream_sample_t *bufferl = outputs[0];
	stream_sample_t *bufferr = outputs[1];
	stream_sample_t *bufferl2 = outputs[2];
	stream_sample_t *bufferr2 = outputs[3];

	for(i = 0 ; i < samples ; i++)
	{
	       info->channel_l[i] = info->channel_r[i] = info->channel_l2[i] = info->channel_r2[i] = 0;
	}

	for (j = 0 ; j < 32 ; j++)
	{
		c352_mix_one_channel(info, j, samples);
	}

	for(i = 0 ; i < samples ; i++)
	{
		*bufferl++ = (short) (info->channel_l[i] >>3);
		*bufferr++ = (short) (info->channel_r[i] >>3);
		*bufferl2++ = (short) (info->channel_l2[i] >>3);
		*bufferr2++ = (short) (info->channel_r2[i] >>3);
	}
}

static unsigned short c352_read_reg16(c352_state *info, unsigned long address)
{
	unsigned long	chan;
	unsigned short	val;

	stream_update(info->stream);

	chan = (address >> 4) & 0xfff;
	if (chan > 31)
	{
		val = 0;
	}
	else
	{
		if ((address & 0xf) == 6)
		{
			val = info->c352_ch[chan].flag;
		}
		else
		{
			val = 0;
		}
	}
	return val;
}

static void c352_write_reg16(c352_state *info, unsigned long address, unsigned short val)
{
	unsigned long	chan;
	int i;

	stream_update(info->stream);

	chan = (address >> 4) & 0xfff;

	if ( address >= 0x400 )
	{
		switch(address)
		{
			case 0x404:	// execute key-ons/offs
				for ( i = 0 ; i <= 31 ; i++ )
				{
					if ( info->c352_ch[i].flag & C352_FLG_KEYON )
					{
						info->c352_ch[i].current_addr = (info->c352_ch[i].bank << 16) + info->c352_ch[i].start_addr;
						info->c352_ch[i].start = info->c352_ch[i].start_addr;
						info->c352_ch[i].repeat = info->c352_ch[i].repeat_addr;
						info->c352_ch[i].noisebuf = 0;
						info->c352_ch[i].noisecnt = 0;
						info->c352_ch[i].flag &= ~(C352_FLG_KEYON | C352_FLG_LOOPHIST);
						info->c352_ch[i].flag |= C352_FLG_BUSY;
					}
					else if ( info->c352_ch[i].flag & C352_FLG_KEYOFF )
					{
						info->c352_ch[i].flag &= ~C352_FLG_BUSY;
						info->c352_ch[i].flag &= ~(C352_FLG_KEYOFF);
					}
				}
				break;
			default:
				break;
		}
		return;
	}

	if (chan > 31)
	{
		LOG(("C352 CTRL %08lx %04x\n", address, val));
		return;
	}
	switch(address & 0xf)
	{
	case 0x0:
		// volumes (output 1)
		LOG(("CH %02ld LVOL %02x RVOL %02x\n", chan, val & 0xff, val >> 8));
		info->c352_ch[chan].vol_l = val & 0xff;
		info->c352_ch[chan].vol_r = val >> 8;
		break;

	case 0x2:
		// volumes (output 2)
		LOG(("CH %02ld RLVOL %02x RRVOL %02x\n", chan, val & 0xff, val >> 8));
		info->c352_ch[chan].vol_l2 = val & 0xff;
		info->c352_ch[chan].vol_r2 = val >> 8;
		break;

	case 0x4:
		// pitch
		LOG(("CH %02ld PITCH %04x\n", chan, val));
		info->c352_ch[chan].pitch = val;
		break;

	case 0x6:
		// flags
		LOG(("CH %02ld FLAG %02x\n", chan, val));
		info->c352_ch[chan].flag = val;
		break;

	case 0x8:
		// bank (bits 16-31 of address);
		info->c352_ch[chan].bank = val & 0xff;
		LOG(("CH %02ld BANK %02x", chan, info->c352_ch[chan].bank));
		break;

	case 0xa:
		// start address
		LOG(("CH %02ld SADDR %04x\n", chan, val));
		info->c352_ch[chan].start_addr = val;
		break;

	case 0xc:
		// end address
		LOG(("CH %02ld EADDR %04x\n", chan, val));
		info->c352_ch[chan].end_addr = val;
		break;

	case 0xe:
		// loop address
		LOG(("CH %02ld LADDR %04x\n", chan, val));
		info->c352_ch[chan].repeat_addr = val;
		break;

	default:
		LOG(("CH %02ld UNKN %01lx %04x", chan, address & 0xf, val));
		break;
	}
}

static void c352_init(c352_state *info, running_device *device)
{
	int i;
	double x_max = 32752.0;
	double y_max = 127.0;
	double u = 10.0;

	// clear all channels states
	memset(info->c352_ch, 0, sizeof(c352_ch_t)*32);

	// generate mulaw table for mulaw format samples
	for (i = 0; i < 256; i++)
	{
	      double y = (double) (i & 0x7f);
	      double x = (exp (y / y_max * log (1.0 + u)) - 1.0) * x_max / u;

	      if (i & 0x80)
	      {
	        x = -x;
	      }
	      info->mulaw_table[i] = (short)x;
	}

	// init noise generator
	info->mseq_reg = 0x12345678;

	// register save state info
	for (i = 0; i < 32; i++)
	{
		state_save_register_device_item(device, i, info->c352_ch[i].vol_l);
		state_save_register_device_item(device, i, info->c352_ch[i].vol_r);
		state_save_register_device_item(device, i, info->c352_ch[i].vol_l2);
		state_save_register_device_item(device, i, info->c352_ch[i].vol_r2);
		state_save_register_device_item(device, i, info->c352_ch[i].bank);
		state_save_register_device_item(device, i, info->c352_ch[i].noise);
		state_save_register_device_item(device, i, info->c352_ch[i].noisebuf);
		state_save_register_device_item(device, i, info->c352_ch[i].noisecnt);
		state_save_register_device_item(device, i, info->c352_ch[i].pitch);
		state_save_register_device_item(device, i, info->c352_ch[i].start_addr);
		state_save_register_device_item(device, i, info->c352_ch[i].end_addr);
		state_save_register_device_item(device, i, info->c352_ch[i].repeat_addr);
		state_save_register_device_item(device, i, info->c352_ch[i].flag);
		state_save_register_device_item(device, i, info->c352_ch[i].start);
		state_save_register_device_item(device, i, info->c352_ch[i].repeat);
		state_save_register_device_item(device, i, info->c352_ch[i].current_addr);
		state_save_register_device_item(device, i, info->c352_ch[i].pos);
	}
}

static DEVICE_START( c352 )
{
	c352_state *info = get_safe_token(device);

	info->c352_rom_samples = *device->region();
	info->c352_rom_length = device->region()->bytes();

	info->sample_rate_base = device->clock() / 192;

	info->stream = stream_create(device, 0, 4, info->sample_rate_base, info, c352_update);

	c352_init(info, device);
}


READ16_DEVICE_HANDLER( c352_r )
{
	return(c352_read_reg16(get_safe_token(device), offset*2));
}

WRITE16_DEVICE_HANDLER( c352_w )
{
	if (mem_mask == 0xffff)
	{
		c352_write_reg16(get_safe_token(device), offset*2, data);
	}
	else
	{
		logerror("C352: byte-wide write unsupported at this time!\n");
	}
}







/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( c352 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(c352_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( c352 );		break;
		case DEVINFO_FCT_STOP:							/* nothing */								break;
		case DEVINFO_FCT_RESET:							/* nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "C352");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Namco PCM");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.1");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(C352, c352);
