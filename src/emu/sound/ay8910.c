/***************************************************************************

  ay8910.c


  Emulation of the AY-3-8910 / YM2149 sound chip.

  Based on various code snippets by Ville Hallik, Michael Cuddy,
  Tatsuyuki Satoh, Fabrice Frances, Nicola Salmoria.
  
  Mostly rewritten by couriersud in 2008
  
  TODO:
  * The AY8930 has an extended mode which is currently 
    not emulated.
  * The YMZ284 only has one 1 output channel (mixed chan A,B,C).
    This should be forced.
  * YM2610 & YM2608 will need a separate flag in their config structures
    to distinguish between legacy and discrete mode.
    
  The rewrite also introduces a generic model for the DAC. This model is
  not perfect, but allows channel mixing based on a parametrized approach.
  This model also allows to factor in different loads on individual channels. 
  If a better model is developped in the future or better measurements are
  available, the driver should be easy to change. The model is described
  later.
  
  In order to not break hundreds of existing drivers by default the flag
  AY8910_LEGACY_OUTPUT is used by drivers not changed to take into account the
  new model. All outputs are normalized to the old output range (i.e. 0 .. 7ffff).
  In the case of channel mixing, output range is 0...3 * 7fff.
  
  The main difference between the AY-3-8910 and the YM2149 is, that the 
  AY-3-8910 datasheet mentions, that fixed volume level 0, which is set by 
  registers 8 to 10 is "channel off". The YM2149 mentions, that the generated
  signal has a 2V DC component. This is confirmed by measurements. The approach
  taken here is to assume the 2V DC offset for all outputs for the YM2149.
  For the AY-3-8910, an offset is used if envelope is active for a channel. 
  This is backed by oscilloscope pictures from the datasheet. If a fixed volume 
  is set, i.e. enveloppe is disabled, the output voltage is set to 0V. Recordings
  I found on the web for gyruss indicate, that the AY-3-8910 offset should
  be around 1.5V. This will also make sound levels more compatible with
  user descriptions.
  
  The Model:
                     5V     5V
                      |      |
                      /      |
  Volume Level x >---|       Z
                      >      Z Pullup Resistor RU
                       |     Z
                       Z     |
                    Rx Z     |
                       Z     |
                       |     |
                       '-----+-------->  >---+----> Output signal
                             |               |
                             Z               Z
               Pulldown RD   Z               Z Load RL
                             Z               Z
                             |               |
                            GND             GND

Each Volume level x will select a different resistor Rx. Measurements from fpgaarcade.com
where used to calibrate channel mixing for the YM2149. This was done using
a least square approach using a fixed RL of 1K Ohm.

For the AY measurements cited in e.g. openmsx as "Hacker Kay" for a single
channel were taken. These were normalized to 0 ... 65535 and adapted to on
offset of 1.5V and a VPP of 1.3V.

***************************************************************************/

#include "sndintrf.h"
#include "deprecat.h"
#include "streams.h"
#include "cpuintrf.h"
#include "ay8910.h"

#define MAX_OUTPUT 0x7fff
#define NUM_CHANNELS 3

typedef struct _ay_ym_param ay_ym_param;
struct _ay_ym_param
{
	double r_up;
	double r_down;
	int    N;
	double res[32];
};

struct AY8910
{
	int index;
	int streams;
	int ready;
	sound_stream *Channel;
	const struct AY8910interface *intf;
	INT32 register_latch;
	UINT8 regs[16];
	INT32 lastEnable;
	INT32 Count[NUM_CHANNELS];
	UINT8 Output[NUM_CHANNELS];
	UINT8 OutputN;
	INT32 CountN;
	INT32 CountE;
	INT8 CountEnv;
	UINT32 VolE;
	UINT8 Hold,Alternate,Attack,Holding;
	INT32 RNG;
	UINT8 EnvP;
	/* init parameters ... */
	int step;
	int zero_is_off;
	UINT8 vol_enabled[NUM_CHANNELS];
	ay_ym_param *par;
	ay_ym_param *parE;
	INT32 VolTable[NUM_CHANNELS][16];
	INT32 VolTableE[NUM_CHANNELS][32];
	INT32 vol3d_tab[8*32*32*32];
};

/* register id's */
#define AY_AFINE	(0)
#define AY_ACOARSE	(1)
#define AY_BFINE	(2)
#define AY_BCOARSE	(3)
#define AY_CFINE	(4)
#define AY_CCOARSE	(5)
#define AY_NOISEPER	(6)
#define AY_ENABLE	(7)
#define AY_AVOL		(8)
#define AY_BVOL		(9)
#define AY_CVOL		(10)
#define AY_EFINE	(11)
#define AY_ECOARSE	(12)
#define AY_ESHAPE	(13)

#define AY_PORTA	(14)
#define AY_PORTB	(15)

#define NOISE_ENABLEQ(_psg, _chan)	(((_psg)->regs[AY_ENABLE] >> (3 + _chan)) & 1)
#define TONE_ENABLEQ(_psg, _chan)	(((_psg)->regs[AY_ENABLE] >> (_chan)) & 1)
#define TONE_PERIOD(_psg, _chan)	( (_psg)->regs[(_chan) << 1] | (((_psg)->regs[((_chan) << 1) | 1] & 0x0f) << 8) )
#define NOISE_PERIOD(_psg)			((_psg)->regs[AY_NOISEPER] & 0x1f)
#define TONE_VOLUME(_psg, _chan)	((_psg)->regs[AY_AVOL + (_chan)] & 0x0f)
#define TONE_ENVELOPE(_psg, _chan)	(((_psg)->regs[AY_AVOL + (_chan)] >> 4) & 1)
#define ENVELOPE_PERIOD(_psg)		(((_psg)->regs[AY_EFINE] | ((_psg)->regs[AY_ECOARSE]<<8)))


static ay_ym_param ym2149_param = 
{
	630, 801,
	16,
	{ 73770, 37586, 27458, 21451, 15864, 12371, 8922,  6796,
	   4763,  3521,  2403,  1737,  1123,   762,  438,   251 },
};

static ay_ym_param ym2149_paramE = 
{
	630, 801,
	32,
	{ 103350, 73770, 52657, 37586, 32125, 27458, 24269, 21451,
	   18447, 15864, 14009, 12371, 10506,  8922,  7787,  6796,
	    5689,  4763,  4095,  3521,  2909,  2403,  2043,  1737,
	    1397,  1123,   925,   762,   578,   438,   332,   251 },
};

#if 0
/* RL = 1000, Hacker Kay normalized, 2.1V to 3.2V */
static ay_ym_param ay8910_param = 
{
	664, 913,
	16,
	{ 85785, 34227, 26986, 20398, 14886, 10588,  7810,  4856,
	   4120,  2512,  1737,  1335,  1005,   747,   586,    451 },
};

#endif
/* 
 * RL = 3000, Hacker Kay normalized pattern, 1.5V to 2.8V
 * These values correspond with guesses based on Gyruss schematics
 * They work well with scramble as well.
 */
static ay_ym_param ay8910_param = 
{
	930, 454,
	16,
	{ 85066, 34179, 27027, 20603, 15046, 10724, 7922, 4935,
	   4189,  2557,  1772,  1363,  1028,  766,   602,  464 },
};


static void AY8910_write_reg(struct AY8910 *PSG, int r, int v)
{

	PSG->regs[r] = v;

	/* A note about the period of tones, noise and envelope: for speed reasons,*/
	/* we count down from the period to 0, but careful studies of the chip     */
	/* output prove that it instead counts up from 0 until the counter becomes */
	/* greater or equal to the period. This is an important difference when the*/
	/* program is rapidly changing the period to modulate the sound.           */
	/* To compensate for the difference, when the period is changed we adjust  */
	/* our internal counter.                                                   */
	/* Also, note that period = 0 is the same as period = 1. This is mentioned */
	/* in the YM2203 data sheets. However, this does NOT apply to the Envelope */
	/* period. In that case, period = 0 is half as period = 1. */
	switch( r )
	{
		case AY_AFINE:
		case AY_ACOARSE:
		case AY_BFINE:
		case AY_BCOARSE:
		case AY_CFINE:
		case AY_CCOARSE:
			/* No action required */
			break;
		case AY_NOISEPER:
			/* No action required */
			break;
		case AY_ENABLE:
			if ((PSG->lastEnable == -1) ||
			    ((PSG->lastEnable & 0x40) != (PSG->regs[AY_ENABLE] & 0x40)))
			{
				/* write out 0xff if port set to input */
				if (PSG->intf->portAwrite)
					(*PSG->intf->portAwrite)(Machine, 0, (PSG->regs[AY_ENABLE] & 0x40) ? PSG->regs[AY_PORTA] : 0xff);
			}
	
			if ((PSG->lastEnable == -1) ||
			    ((PSG->lastEnable & 0x80) != (PSG->regs[AY_ENABLE] & 0x80)))
			{
				/* write out 0xff if port set to input */
				if (PSG->intf->portBwrite)
					(*PSG->intf->portBwrite)(Machine, 0, (PSG->regs[AY_ENABLE] & 0x80) ? PSG->regs[AY_PORTB] : 0xff);
			}
	
			PSG->lastEnable = PSG->regs[AY_ENABLE];
			break;
		case AY_AVOL:
		case AY_BVOL:
		case AY_CVOL:
			/* No action required */
			break;
		case AY_EFINE:
		case AY_ECOARSE:
			/* No action required */
			break;
		case AY_ESHAPE:
			/* envelope shapes:
	        C AtAlH
	        0 0 x x  \___
	        0 1 x x  /___
	        1 0 0 0  \\\\
	        1 0 0 1  \___
	        1 0 1 0  \/\/
	        1 0 1 1  \¯¯¯
	        1 1 0 0  ////
	        1 1 0 1  /¯¯¯
	        1 1 1 0  /\/\
	        1 1 1 1  /___
	        The envelope counter on the AY-3-8910 has 16 steps. On the YM2149 it
	        has twice the steps, happening twice as fast.
	        */
			PSG->Attack = (PSG->regs[AY_ESHAPE] & 0x04) ? PSG->EnvP : 0x00;
			if ((PSG->regs[AY_ESHAPE] & 0x08) == 0)
			{
				/* if Continue = 0, map the shape to the equivalent one which has Continue = 1 */
				PSG->Hold = 1;
				PSG->Alternate = PSG->Attack;
			}
			else
			{
				PSG->Hold = PSG->regs[AY_ESHAPE] & 0x01;
				PSG->Alternate = PSG->regs[AY_ESHAPE] & 0x02;
			}
			PSG->CountEnv = PSG->EnvP;
			PSG->Holding = 0;
			PSG->VolE = (PSG->CountEnv ^ PSG->Attack);
			break;
		case AY_PORTA:
			if (PSG->regs[AY_ENABLE] & 0x40)
			{
				if (PSG->intf->portAwrite)
					(*PSG->intf->portAwrite)(Machine, 0, PSG->regs[AY_PORTA]);
				else
					logerror("warning - write %02x to 8910 #%d Port A\n",PSG->regs[AY_PORTA],PSG->index);
			}
			else
			{
				logerror("warning: write to 8910 #%d Port A set as input - ignored\n",PSG->index);
			}
			break;
		case AY_PORTB:
			if (PSG->regs[AY_ENABLE] & 0x80)
			{
				if (PSG->intf->portBwrite)
					(*PSG->intf->portBwrite)(Machine, 0, PSG->regs[AY_PORTB]);
				else
					logerror("warning - write %02x to 8910 #%d Port B\n",PSG->regs[AY_PORTB],PSG->index);
			}
			else
			{
				logerror("warning: write to 8910 #%d Port B set as input - ignored\n",PSG->index);
			}
			break;
	}
}


INLINE UINT16 mix_3D(struct AY8910 *PSG)
{
	int indx = 0, chan;

	for (chan = 0; chan < NUM_CHANNELS; chan++)
		if (TONE_ENVELOPE(PSG, chan))
		{
			indx |= (1 << (chan+15)) | ( PSG->vol_enabled[chan] ? PSG->VolE << (chan*5) : 0);
		}
		else
		{
			indx |= (PSG->vol_enabled[chan] ? TONE_VOLUME(PSG, chan) << (chan*5) : 0);
		}
	return PSG->vol3d_tab[indx];
}

static void AY8910_update(void *param,stream_sample_t **inputs, stream_sample_t **buffer,int length)
{
	struct AY8910 *PSG = param;
	stream_sample_t *buf[NUM_CHANNELS];
	int chan;

	buf[0] = buffer[0];
	buf[1] = NULL;
	buf[2] = NULL;
	if (PSG->streams == NUM_CHANNELS)
	{
		buf[1] = buffer[1];
		buf[2] = buffer[2];
	}

	/* hack to prevent us from hanging when starting filtered outputs */
	if (!PSG->ready)
	{
		for (chan = 0; chan < NUM_CHANNELS; chan++)
			if (buf[chan] != NULL)
				memset(buf[chan], 0, length * sizeof(*buf[chan]));
	}

	/* The 8910 has three outputs, each output is the mix of one of the three */
	/* tone generators and of the (single) noise generator. The two are mixed */
	/* BEFORE going into the DAC. The formula to mix each channel is: */
	/* (ToneOn | ToneDisable) & (NoiseOn | NoiseDisable). */
	/* Note that this means that if both tone and noise are disabled, the output */
	/* is 1, not 0, and can be modulated changing the volume. */
	
	/* buffering loop */
	while (length)
	{
		for (chan = 0; chan < NUM_CHANNELS; chan++)
		{
			PSG->Count[chan]++;
			if (PSG->Count[chan] >= TONE_PERIOD(PSG, chan) * PSG->step)
			{
				PSG->Output[chan] ^= 1;
				PSG->Count[chan] = 0;;
			}
		}

		PSG->CountN++;
		if (PSG->CountN >= NOISE_PERIOD(PSG) * PSG->step)
		{
			/* Is noise output going to change? */
			if ((PSG->RNG + 1) & 2)	/* (bit0^bit1)? */
			{
				PSG->OutputN ^= 1;
			}

			/* The Random Number Generator of the 8910 is a 17-bit shift */
			/* register. The input to the shift register is bit0 XOR bit3 */
			/* (bit0 is the output). This was verified on AY-3-8910 and YM2149 chips. */

			/* The following is a fast way to compute bit17 = bit0^bit3. */
			/* Instead of doing all the logic operations, we only check */
			/* bit0, relying on the fact that after three shifts of the */
			/* register, what now is bit3 will become bit0, and will */
			/* invert, if necessary, bit14, which previously was bit17. */
			if (PSG->RNG & 1) 
				PSG->RNG ^= 0x24000; /* This version is called the "Galois configuration". */
			PSG->RNG >>= 1;
			PSG->CountN = 0;
		}

		for (chan = 0; chan < NUM_CHANNELS; chan++)
		{
			PSG->vol_enabled[chan] = (PSG->Output[chan] | TONE_ENABLEQ(PSG, chan)) & (PSG->OutputN | NOISE_ENABLEQ(PSG, chan));
		}

		/* update envelope */
		if (PSG->Holding == 0)
		{
			PSG->CountE++; 
			if (PSG->CountE >= ENVELOPE_PERIOD(PSG))
			{
				PSG->CountE = 0;
				PSG->CountEnv--;

				/* check envelope current position */
				if (PSG->CountEnv < 0)
				{
					if (PSG->Hold)
					{
						if (PSG->Alternate)
							PSG->Attack ^= PSG->EnvP;
						PSG->Holding = 1;
						PSG->CountEnv = 0;
					}
					else
					{
						/* if CountEnv has looped an odd number of times (usually 1), */
						/* invert the output. */
						if (PSG->Alternate && (PSG->CountEnv & (PSG->EnvP + 1)))
 							PSG->Attack ^= PSG->EnvP;

						PSG->CountEnv &= PSG->EnvP;
					}
				}

			}
		}
		PSG->VolE = (PSG->CountEnv ^ PSG->Attack);

		if (PSG->streams == 3)
		{
			for (chan = 0; chan < NUM_CHANNELS; chan++)
				if (TONE_ENVELOPE(PSG,chan))
				{
					/* Envolope has no "off" state */
					*(buf[chan]++) = PSG->VolTableE[chan][PSG->vol_enabled[chan] ? PSG->VolE : 0];
				}
				else
				{
					*(buf[chan]++) = PSG->VolTable[chan][PSG->vol_enabled[chan] ? TONE_VOLUME(PSG, chan) : 0];
				}
		}
		else
		{
			*(buf[0]++) = mix_3D(PSG);
#if 0
			*(buf[0]) = (  vol_enabled[0] * PSG->VolTable[PSG->Vol[0]] 
			             + vol_enabled[1] * PSG->VolTable[PSG->Vol[1]] 
			             + vol_enabled[2] * PSG->VolTable[PSG->Vol[2]]) / PSG->step;
#endif
		}
		length--;
	}
}


void AY8910_set_volume(int chip,int channel,int volume)
{
	struct AY8910 *PSG = sndti_token(SOUND_AY8910, chip);
	int ch;

	for (ch = 0; ch < PSG->streams; ch++)
		if (channel == ch || PSG->streams == 1 || channel == ALL_8910_CHANNELS)
			stream_set_output_gain(PSG->Channel, ch, volume / 100.0);
}

INLINE void build_single_table(double rl, ay_ym_param *par, int normalize, INT32 *tab, int zero_is_off)
{
	int j;
	double rt, rw = 0;
	double temp[32], min=10.0, max=0.0;

	for (j=0; j < par->N; j++)
	{
		rt = 1.0 / par->r_down + 1.0 / rl;

		rw = 1.0 / par->res[j];
		rt += 1.0 / par->res[j];

		if (!(zero_is_off && j == 0))
		{
			rw += 1.0 / par->r_up;
			rt += 1.0 / par->r_up;
		}
		
		temp[j] = rw / rt;
		if (temp[j] < min)
			min = temp[j];
		if (temp[j] > max)
			max = temp[j];
	}
	if (normalize)
	{
		for (j=0; j < par->N; j++)
			tab[j] = MAX_OUTPUT * (((temp[j] - min)/(max-min)) - 0.25) * 0.5;
	}
	else
	{
		for (j=0; j < par->N; j++)
			tab[j] = MAX_OUTPUT * temp[j];
	}
		
}

INLINE void build_3D_table(double rl, ay_ym_param *par, ay_ym_param *parE, int normalize, double factor, int zero_is_off, INT32 *tab)
{
	int j, j1, j2, j3, e, indx;
	double rt, rw, n;
	double min = 10.0,  max = 0.0;
	double *temp;

	temp = malloc(8*32*32*32*sizeof(*temp));
	
	for (e=0; e < 8; e++)
		for (j1=0; j1 < 32; j1++)
			for (j2=0; j2 < 32; j2++)
				for (j3=0; j3 < 32; j3++)
				{
					if (zero_is_off)
					{
						n  = (j1 != 0 || (e & 0x01)) ? 1 : 0; 
						n += (j2 != 0 || (e & 0x02)) ? 1 : 0; 
						n += (j3 != 0 || (e & 0x04)) ? 1 : 0; 
					}
					else
						n = 3.0;
					
					rt = n / par->r_up + 3.0 / par->r_down + 1.0 / rl;
					rw = n / par->r_up;
					
					rw += 1.0 / ( (e & 0x01) ? parE->res[j1] : par->res[j1]);
					rt += 1.0 / ( (e & 0x01) ? parE->res[j1] : par->res[j1]);
					rw += 1.0 / ( (e & 0x02) ? parE->res[j2] : par->res[j2]);
					rt += 1.0 / ( (e & 0x02) ? parE->res[j2] : par->res[j2]);
					rw += 1.0 / ( (e & 0x04) ? parE->res[j3] : par->res[j3]);
					rt += 1.0 / ( (e & 0x04) ? parE->res[j3] : par->res[j3]);
						
					indx = (e << 15) | (j3<<10) | (j2<<5) | j1;
					temp[indx] = rw / rt;
					if (temp[indx] < min)
						min = temp[indx];
					if (temp[indx] > max)
						max = temp[indx];
				}

	if (normalize)
	{
		for (j=0; j < 32*32*32*8; j++)
			tab[j] = MAX_OUTPUT * (((temp[j] - min)/(max-min)) - 0.25) * factor;
	}
	else
	{
		for (j=0; j < 32*32*32*8; j++)
			tab[j] = MAX_OUTPUT * temp[j];
	}
	
	/* for (e=0;e<16;e++) printf("%d %d\n",e<<10, tab[e<<10]); */
	
	free(temp);
}


static void build_mixer_table(struct AY8910 *PSG)
{
	int	normalize = 0;
	int	chan;
	
	if ((PSG->intf->flags & AY8910_LEGACY_OUTPUT) != 0)
	{
		logerror("AY-3-8910/YM2149 using legacy output levels!\n");
		normalize = 1;
	}
	
	for (chan=0; chan < NUM_CHANNELS; chan++)
	{
		build_single_table(PSG->intf->res_load[chan], PSG->par, normalize, PSG->VolTable[chan], PSG->zero_is_off);
		build_single_table(PSG->intf->res_load[chan], PSG->parE, normalize, PSG->VolTableE[chan], 0);
	}
	build_3D_table(PSG->intf->res_load[0], PSG->par, PSG->parE, normalize, 3, PSG->zero_is_off, PSG->vol3d_tab);
}


static void AY8910_statesave(struct AY8910 *PSG, int sndindex)
{
	state_save_register_item("AY8910", sndindex, PSG->register_latch);
	state_save_register_item_array("AY8910", sndindex, PSG->regs);
	state_save_register_item("AY8910", sndindex, PSG->lastEnable);

	state_save_register_item_array("AY8910", sndindex, PSG->Count);
	state_save_register_item("AY8910", sndindex, PSG->CountN);
	state_save_register_item("AY8910", sndindex, PSG->CountE);

	state_save_register_item("AY8910", sndindex, PSG->VolE);

	state_save_register_item_array("AY8910", sndindex, PSG->Output);
	state_save_register_item("AY8910", sndindex, PSG->OutputN);

	state_save_register_item("AY8910", sndindex, PSG->CountEnv);
	state_save_register_item("AY8910", sndindex, PSG->Hold);
	state_save_register_item("AY8910", sndindex, PSG->Alternate);
	state_save_register_item("AY8910", sndindex, PSG->Attack);
	state_save_register_item("AY8910", sndindex, PSG->Holding);
	state_save_register_item("AY8910", sndindex, PSG->RNG);
}


void *ay8910_start_ym(sound_type chip_type, int sndindex, int clock, const struct AY8910interface *intf)
{
	struct AY8910 *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));
	info->index = sndindex;
	info->intf = intf;
	if ((info->intf->flags & AY8910_SINGLE_OUTPUT) != 0)
	{
		logerror("AY-3-8910/YM2149 using single output!\n");
		info->streams = 1;
	}
	else
		info->streams = 3;
		
	switch (chip_type)
	{
		case SOUND_AY8910:
		case SOUND_AY8930:
			info->step = 1;
			info->par = &ay8910_param;
			info->parE = &ay8910_param;
			info->zero_is_off = 1;
			break;
		case SOUND_YM2149:
		case SOUND_YM2203:
		case SOUND_YM2610:
		case SOUND_YM2610B:
		case SOUND_YM2608:
		case SOUND_YMZ284:
		case SOUND_YMZ294:
		case SOUND_YM3439:
		default:
			info->step = 2;
			info->par = &ym2149_param;
			info->parE = &ym2149_paramE;
			info->zero_is_off = 0;
			break;
	}
	info->EnvP = info->step * 16 - 1; 

	build_mixer_table(info);

	/* the step clock for the tone and noise generators is the chip clock    */
	/* divided by 8 for the YM2149 and 16 for the AY-3-8910, To emulate a    */
	/* full cycle, we have to use a divisor of 4 resp. 8                     */
	info->Channel = stream_create(0,info->streams,clock / (8 / info->step),info,AY8910_update);

	ay8910_set_clock_ym(info,clock);
	AY8910_statesave(info, sndindex);

	return info;
}

void ay8910_stop_ym(void *chip)
{
}

void ay8910_reset_ym(void *chip)
{
	struct AY8910 *PSG = chip;
	int i;

	PSG->register_latch = 0;
	PSG->RNG = 1;
	PSG->Output[0] = 0;
	PSG->Output[1] = 0;
	PSG->Output[2] = 0;
	PSG->Count[0] = 0;
	PSG->Count[1] = 0;
	PSG->Count[2] = 0;
	PSG->CountN = 0;
	PSG->CountE = 0;
	PSG->OutputN = 0x01;
	PSG->lastEnable = -1;	/* force a write */
	for (i = 0;i < AY_PORTA;i++)
		AY8910_write_reg(PSG,i,0);	
	PSG->ready = 1;
}

void ay8910_set_clock_ym(void *chip, int clock)
{
	struct AY8910 *PSG = chip;
	stream_set_sample_rate(PSG->Channel, clock / (8 / PSG->step));
}

void ay8910_write_ym(void *chip, int addr, int data)
{
	struct AY8910 *PSG = chip;

	if (addr & 1)
	{	/* Data port */
		int r = PSG->register_latch;

		if (r > 15) return;
		if (r == AY_ESHAPE || PSG->regs[r] != data)
		{
			/* update the output buffer before changing the register */
			stream_update(PSG->Channel);
		}

		AY8910_write_reg(PSG,r,data);
	}
	else
	{	/* Register port */
		PSG->register_latch = data & 0x0f;
	}
}

int ay8910_read_ym(void *chip)
{
	struct AY8910 *PSG = chip;
	int r = PSG->register_latch;

	if (r > 15) return 0;

	switch (r)
	{
	case AY_PORTA:
		if ((PSG->regs[AY_ENABLE] & 0x40) != 0)
			logerror("warning: read from 8910 #%d Port A set as output\n",PSG->index);
		/*
           even if the port is set as output, we still need to return the external
           data. Some games, like kidniki, need this to work.
         */
		if (PSG->intf->portAread)
			PSG->regs[AY_PORTA] = (*PSG->intf->portAread)(Machine, 0);
		else 
			logerror("PC %04x: warning - read 8910 #%d Port A\n",activecpu_get_pc(),PSG->index);
		break;
	case AY_PORTB:
		if ((PSG->regs[AY_ENABLE] & 0x80) != 0)
			logerror("warning: read from 8910 #%d Port B set as output\n",PSG->index);
		if (PSG->intf->portBread) 
			PSG->regs[AY_PORTB] = (*PSG->intf->portBread)(Machine, 0);
		else 
			logerror("PC %04x: warning - read 8910 #%d Port B\n",activecpu_get_pc(),PSG->index);
		break;
	}
	return PSG->regs[r];
}


static void *ay8910_start(int sndindex, int clock, const void *config)
{
	static const struct AY8910interface generic_ay8910 = 
	{ 	
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		NULL, NULL, NULL, NULL
	};
	const struct AY8910interface *intf = (config ? config : &generic_ay8910);
	return ay8910_start_ym(SOUND_AY8910, sndindex+16, clock, intf);
}

static void *ym2149_start(int sndindex, int clock, const void *config)
{
	static const struct AY8910interface generic_ay8910 = 
	{ 	
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		NULL, NULL, NULL, NULL
	};
	const struct AY8910interface *intf = (config ? config : &generic_ay8910);
	return ay8910_start_ym(SOUND_YM2149, sndindex+16, clock, intf);
}

static void ay8910_stop(void *chip)
{
	ay8910_stop_ym(chip);
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void ay8910_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void ay8910_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_ALIAS:							info->i = SOUND_AY8910;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = ay8910_set_info;		break;
		case SNDINFO_PTR_START:							info->start = ay8910_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = ay8910_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = ay8910_reset_ym;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "AY-3-8910A";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "PSG";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

void ay8912_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = ay8910_start;				break;
		case SNDINFO_STR_NAME:							info->s = "AY-3-8912A";					break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}

void ay8913_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = ay8910_start;				break;
		case SNDINFO_STR_NAME:							info->s = "AY-3-8913A";					break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}

void ay8930_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = ay8910_start;				break;
		case SNDINFO_STR_NAME:							info->s = "AY8930";						break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}

void ym2149_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = ym2149_start;				break;
		case SNDINFO_STR_NAME:							info->s = "YM2149";						break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}

void ym3439_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = ym2149_start;				break;
		case SNDINFO_STR_NAME:							info->s = "YM3439";						break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}

void ymz284_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = ym2149_start;				break;
		case SNDINFO_STR_NAME:							info->s = "YMZ284";						break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}

void ymz294_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = ym2149_start;				break;
		case SNDINFO_STR_NAME:							info->s = "YMZ294";						break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}

/* AY8910 interface */
READ8_HANDLER( AY8910_read_port_0_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 0)); }
READ8_HANDLER( AY8910_read_port_1_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 1)); }
READ8_HANDLER( AY8910_read_port_2_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 2)); }
READ8_HANDLER( AY8910_read_port_3_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 3)); }
READ8_HANDLER( AY8910_read_port_4_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 4)); }
READ16_HANDLER( AY8910_read_port_0_lsb_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 0)); }
READ16_HANDLER( AY8910_read_port_1_lsb_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 1)); }
READ16_HANDLER( AY8910_read_port_2_lsb_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 2)); }
READ16_HANDLER( AY8910_read_port_3_lsb_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 3)); }
READ16_HANDLER( AY8910_read_port_4_lsb_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 4)); }
READ16_HANDLER( AY8910_read_port_0_msb_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 0)) << 8; }
READ16_HANDLER( AY8910_read_port_1_msb_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 1)) << 8; }
READ16_HANDLER( AY8910_read_port_2_msb_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 2)) << 8; }
READ16_HANDLER( AY8910_read_port_3_msb_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 3)) << 8; }
READ16_HANDLER( AY8910_read_port_4_msb_r ) { return ay8910_read_ym(sndti_token(SOUND_AY8910, 4)) << 8; }

WRITE8_HANDLER( AY8910_control_port_0_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 0),0,data); }
WRITE8_HANDLER( AY8910_control_port_1_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 1),0,data); }
WRITE8_HANDLER( AY8910_control_port_2_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 2),0,data); }
WRITE8_HANDLER( AY8910_control_port_3_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 3),0,data); }
WRITE8_HANDLER( AY8910_control_port_4_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 4),0,data); }
WRITE16_HANDLER( AY8910_control_port_0_lsb_w ) { if (ACCESSING_BITS_0_7) ay8910_write_ym(sndti_token(SOUND_AY8910, 0),0,data & 0xff); }
WRITE16_HANDLER( AY8910_control_port_1_lsb_w ) { if (ACCESSING_BITS_0_7) ay8910_write_ym(sndti_token(SOUND_AY8910, 1),0,data & 0xff); }
WRITE16_HANDLER( AY8910_control_port_2_lsb_w ) { if (ACCESSING_BITS_0_7) ay8910_write_ym(sndti_token(SOUND_AY8910, 2),0,data & 0xff); }
WRITE16_HANDLER( AY8910_control_port_3_lsb_w ) { if (ACCESSING_BITS_0_7) ay8910_write_ym(sndti_token(SOUND_AY8910, 3),0,data & 0xff); }
WRITE16_HANDLER( AY8910_control_port_4_lsb_w ) { if (ACCESSING_BITS_0_7) ay8910_write_ym(sndti_token(SOUND_AY8910, 4),0,data & 0xff); }
WRITE16_HANDLER( AY8910_control_port_0_msb_w ) { if (ACCESSING_BITS_8_15) ay8910_write_ym(sndti_token(SOUND_AY8910, 0),0,data >> 8); }
WRITE16_HANDLER( AY8910_control_port_1_msb_w ) { if (ACCESSING_BITS_8_15) ay8910_write_ym(sndti_token(SOUND_AY8910, 1),0,data >> 8); }
WRITE16_HANDLER( AY8910_control_port_2_msb_w ) { if (ACCESSING_BITS_8_15) ay8910_write_ym(sndti_token(SOUND_AY8910, 2),0,data >> 8); }
WRITE16_HANDLER( AY8910_control_port_3_msb_w ) { if (ACCESSING_BITS_8_15) ay8910_write_ym(sndti_token(SOUND_AY8910, 3),0,data >> 8); }
WRITE16_HANDLER( AY8910_control_port_4_msb_w ) { if (ACCESSING_BITS_8_15) ay8910_write_ym(sndti_token(SOUND_AY8910, 4),0,data >> 8); }

WRITE8_HANDLER( AY8910_write_port_0_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 0),1,data); }
WRITE8_HANDLER( AY8910_write_port_1_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 1),1,data); }
WRITE8_HANDLER( AY8910_write_port_2_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 2),1,data); }
WRITE8_HANDLER( AY8910_write_port_3_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 3),1,data); }
WRITE8_HANDLER( AY8910_write_port_4_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 4),1,data); }
WRITE16_HANDLER( AY8910_write_port_0_lsb_w ) { if (ACCESSING_BITS_0_7) ay8910_write_ym(sndti_token(SOUND_AY8910, 0),1,data & 0xff); }
WRITE16_HANDLER( AY8910_write_port_1_lsb_w ) { if (ACCESSING_BITS_0_7) ay8910_write_ym(sndti_token(SOUND_AY8910, 1),1,data & 0xff); }
WRITE16_HANDLER( AY8910_write_port_2_lsb_w ) { if (ACCESSING_BITS_0_7) ay8910_write_ym(sndti_token(SOUND_AY8910, 2),1,data & 0xff); }
WRITE16_HANDLER( AY8910_write_port_3_lsb_w ) { if (ACCESSING_BITS_0_7) ay8910_write_ym(sndti_token(SOUND_AY8910, 3),1,data & 0xff); }
WRITE16_HANDLER( AY8910_write_port_4_lsb_w ) { if (ACCESSING_BITS_0_7) ay8910_write_ym(sndti_token(SOUND_AY8910, 4),1,data & 0xff); }
WRITE16_HANDLER( AY8910_write_port_0_msb_w ) { if (ACCESSING_BITS_8_15) ay8910_write_ym(sndti_token(SOUND_AY8910, 0),1,data >> 8); }
WRITE16_HANDLER( AY8910_write_port_1_msb_w ) { if (ACCESSING_BITS_8_15) ay8910_write_ym(sndti_token(SOUND_AY8910, 1),1,data >> 8); }
WRITE16_HANDLER( AY8910_write_port_2_msb_w ) { if (ACCESSING_BITS_8_15) ay8910_write_ym(sndti_token(SOUND_AY8910, 2),1,data >> 8); }
WRITE16_HANDLER( AY8910_write_port_3_msb_w ) { if (ACCESSING_BITS_8_15) ay8910_write_ym(sndti_token(SOUND_AY8910, 3),1,data >> 8); }
WRITE16_HANDLER( AY8910_write_port_4_msb_w ) { if (ACCESSING_BITS_8_15) ay8910_write_ym(sndti_token(SOUND_AY8910, 4),1,data >> 8); }
