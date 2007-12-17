#include <math.h>

#include "sndintrf.h"
#include "streams.h"

#include "msm5232.h"

/*
    OKI MSM5232RS
    8 channel tone generator
*/

typedef struct {
	UINT8 mode;

	int		TG_count_period;
	int		TG_count;

	UINT8	TG_cnt;		/* 7 bits binary counter (frequency output) */
	UINT8	TG_out16;	/* bit number (of TG_cnt) for 16' output */
	UINT8	TG_out8;	/* bit number (of TG_cnt) for  8' output */
	UINT8	TG_out4;	/* bit number (of TG_cnt) for  4' output */
	UINT8	TG_out2;	/* bit number (of TG_cnt) for  2' output */

	int		egvol;
	int		eg_sect;
	int		counter;
	int		eg;

	UINT8	eg_arm;		/* attack/release mode */

	double	ar_rate;
	double	dr_rate;
	double	rr_rate;

	int	pitch;			/* current pitch data */
} VOICE;


typedef struct {
	sound_stream *stream;

	VOICE	voi[8];

	UINT32 EN_out16[2];	/* enable 16' output masks for both groups (0-disabled ; ~0 -enabled) */
	UINT32 EN_out8[2];	/* enable 8'  output masks */
	UINT32 EN_out4[2];	/* enable 4'  output masks */
	UINT32 EN_out2[2];	/* enable 2'  output masks */

	int noise_cnt;
	int noise_step;
	int noise_rng;
	int noise_clocks;	/* number of the noise_rng (output) level changes */

	unsigned int UpdateStep;

	/* rate tables */
	double	ar_tbl[8];
	double	dr_tbl[16];

	UINT8   control1;
	UINT8   control2;

	int		clock;		/* chip clock in Hz */
	int		rate;		/* sample rate in Hz */

	double	external_capacity[8]; /* in Farads, eg 0.39e-6 = 0.36 uF (microFarads) */

} MSM5232;


/* Default chip clock is 2119040 Hz */
/* At this clock chip generates exactly 440.0 Hz signal on 8' output when pitch data=0x21 */


/* ROM table to convert from pitch data into data for programmable counter and binary counter */
/* Chip has 88x12bits ROM   (addressing (in hex) from 0x00 to 0x57) */
#define ROM(counter,bindiv)	(counter|(bindiv<<9))

static const UINT16 MSM5232_ROM[88]={
/* higher values are Programmable Counter data (9 bits) */
/* lesser values are Binary Counter shift data (3 bits) */

/* 0 */	ROM (506, 7),

/* 1 */ ROM (478, 7),/* 2 */ ROM (451, 7),/* 3 */ ROM (426, 7),/* 4 */ ROM (402, 7),
/* 5 */ ROM (379, 7),/* 6 */ ROM (358, 7),/* 7 */ ROM (338, 7),/* 8 */ ROM (319, 7),
/* 9 */ ROM (301, 7),/* A */ ROM (284, 7),/* B */ ROM (268, 7),/* C */ ROM (253, 7),

/* D */ ROM (478, 6),/* E */ ROM (451, 6),/* F */ ROM (426, 6),/*10 */ ROM (402, 6),
/*11 */ ROM (379, 6),/*12 */ ROM (358, 6),/*13 */ ROM (338, 6),/*14 */ ROM (319, 6),
/*15 */ ROM (301, 6),/*16 */ ROM (284, 6),/*17 */ ROM (268, 6),/*18 */ ROM (253, 6),

/*19 */ ROM (478, 5),/*1A */ ROM (451, 5),/*1B */ ROM (426, 5),/*1C */ ROM (402, 5),
/*1D */ ROM (379, 5),/*1E */ ROM (358, 5),/*1F */ ROM (338, 5),/*20 */ ROM (319, 5),
/*21 */ ROM (301, 5),/*22 */ ROM (284, 5),/*23 */ ROM (268, 5),/*24 */ ROM (253, 5),

/*25 */ ROM (478, 4),/*26 */ ROM (451, 4),/*27 */ ROM (426, 4),/*28 */ ROM (402, 4),
/*29 */ ROM (379, 4),/*2A */ ROM (358, 4),/*2B */ ROM (338, 4),/*2C */ ROM (319, 4),
/*2D */ ROM (301, 4),/*2E */ ROM (284, 4),/*2F */ ROM (268, 4),/*30 */ ROM (253, 4),

/*31 */ ROM (478, 3),/*32 */ ROM (451, 3),/*33 */ ROM (426, 3),/*34 */ ROM (402, 3),
/*35 */ ROM (379, 3),/*36 */ ROM (358, 3),/*37 */ ROM (338, 3),/*38 */ ROM (319, 3),
/*39 */ ROM (301, 3),/*3A */ ROM (284, 3),/*3B */ ROM (268, 3),/*3C */ ROM (253, 3),

/*3D */ ROM (478, 2),/*3E */ ROM (451, 2),/*3F */ ROM (426, 2),/*40 */ ROM (402, 2),
/*41 */ ROM (379, 2),/*42 */ ROM (358, 2),/*43 */ ROM (338, 2),/*44 */ ROM (319, 2),
/*45 */ ROM (301, 2),/*46 */ ROM (284, 2),/*47 */ ROM (268, 2),/*48 */ ROM (253, 2),

/*49 */ ROM (478, 1),/*4A */ ROM (451, 1),/*4B */ ROM (426, 1),/*4C */ ROM (402, 1),
/*4D */ ROM (379, 1),/*4E */ ROM (358, 1),/*4F */ ROM (338, 1),/*50 */ ROM (319, 1),
/*51 */ ROM (301, 1),/*52 */ ROM (284, 1),/*53 */ ROM (268, 1),/*54 */ ROM (253, 1),

/*55 */ ROM (253, 1),/*56 */ ROM (253, 1),

/*57 */ ROM (13, 7)
};
#undef ROM


#define STEP_SH	(16)	/* step calculations accuracy */


/* save output as raw 16-bit sample */
/* #define SAVE_SAMPLE */
/* #define SAVE_SEPARATE_CHANNELS */
#if defined SAVE_SAMPLE || defined SAVE_SEPARATE_CHANNELS
static FILE *sample[9];
#endif



/*
 * resistance values are guesswork, default capacity is mentioned in the datasheets
 *
 * charges external capacitor (default is 0.39uF) via R51
 * in approx. 5*1400 * 0.39e-6
 *
 * external capacitor is discharged through R52
 * in approx. 5*28750 * 0.39e-6
 */


#define R51 1400 	/* charge resistance */
#define R52 28750	/* discharge resistance */

#if 0
/*
    C24 = external capacity

    mame_printf_debug("Time constant T=R*C =%f sec.\n",R51*C24);
    mame_printf_debug("Cap fully charged after 5T=%f sec (sample=%f). Level=%f\n",(R51*C24)*5,(R51*C24)*5*sample_rate , VMAX*0.99326 );
    mame_printf_debug("Cap charged after 5T=%f sec (sample=%f). Level=%20.16f\n",(R51*C24)*5,(R51*C24)*5*sample_rate ,
           VMAX*(1.0-pow(2.718,-0.0748/(R51*C24))) );
*/
#endif




static void msm5232_init_tables( MSM5232 *chip )
{
	int i;
	double scale;

	/* sample rate = chip clock !!!  But : */
	/* highest possible frequency is chipclock/13/16 (pitch data=0x57) */
	/* at 2MHz : 2000000/13/16 = 9615 Hz */

	i = ((double)(1<<STEP_SH) * (double)chip->rate) / (double)chip->clock;
	chip->UpdateStep = i;
	/* logerror("clock=%i Hz rate=%i Hz, UpdateStep=%i\n",
            chip->clock, chip->rate, chip->UpdateStep); */

	scale = ((double)chip->clock) / (double)chip->rate;
	chip->noise_step = ((1<<STEP_SH)/128.0) * scale; /* step of the rng reg in 16.16 format */
	/* logerror("noise step=%8x\n", chip->noise_step); */

#if 0
{
	/* rate tables (in miliseconds) */
	static int ATBL[8] = { 2,4,8,16, 32,64, 32,64};
	static int DTBL[16]= { 40,80,160,320, 640,1280, 640,1280,
							333,500,1000,2000, 4000,8000, 4000,8000};
	for (i=0; i<8; i++)
	{
		double clockscale = (double)chip->clock / 2119040.0;
		double time = (ATBL[i] / 1000.0) / clockscale;	/* attack time in seconds */
		chip->ar_tbl[i] = (float)0.50 * ( (1.0/time) / (double)chip->rate );
		/* logerror("ATBL[%i] = %20.16f time = %f s\n",i, chip->ar_tbl[i], time); */
	}

	for (i=0; i<16; i++)
	{
		double clockscale = (double)chip->clock / 2119040.0;
		double time = (DTBL[i] / 1000.0) / clockscale;	/* decay time in seconds */
		chip->dr_tbl[i] = (float)0.50 * ( (1.0/time) / (double)chip->rate );
		/* logerror("DTBL[%i] = %20.16f time = %f s\n",i, chip->dr_tbl[i], time); */
	}
}
#endif


	for (i=0; i<8; i++)
	{
		double clockscale = (double)chip->clock / 2119040.0;
		chip->ar_tbl[i]   = ((1<<i) / clockscale) * (double)R51;
	}

	for (i=0; i<8; i++)
	{
		double clockscale = (double)chip->clock / 2119040.0;
		chip->dr_tbl[i]   = (     (1<<i) / clockscale) * (double)R52;
		chip->dr_tbl[i+8] = (6.25*(1<<i) / clockscale) * (double)R52;
	}


#ifdef SAVE_SAMPLE
	sample[8]=fopen("sampsum.pcm","wb");
#endif
#ifdef SAVE_SEPARATE_CHANNELS
	sample[0]=fopen("samp0.pcm","wb");
	sample[1]=fopen("samp1.pcm","wb");
	sample[2]=fopen("samp2.pcm","wb");
	sample[3]=fopen("samp3.pcm","wb");
	sample[4]=fopen("samp4.pcm","wb");
	sample[5]=fopen("samp5.pcm","wb");
	sample[6]=fopen("samp6.pcm","wb");
	sample[7]=fopen("samp7.pcm","wb");
#endif
}


static void msm5232_init_voice(MSM5232 *chip, int i)
{
	chip->voi[i].ar_rate= chip->ar_tbl[0] * chip->external_capacity[i];
	chip->voi[i].dr_rate= chip->dr_tbl[0] * chip->external_capacity[i];
	chip->voi[i].rr_rate= chip->dr_tbl[0] * chip->external_capacity[i];	/* this is constant value */
	chip->voi[i].eg_sect= -1;
	chip->voi[i].eg		= 0.0;
	chip->voi[i].eg_arm	= 0;
	chip->voi[i].pitch	= -1.0;
}


static void msm5232_write(MSM5232 *chip, int ofst, int data);


static void msm5232_reset(void *_chip)
{
	MSM5232 *chip = _chip;
	int i;

	for (i=0; i<8; i++)
	{
		msm5232_write(chip,i,0x80);
		msm5232_write(chip,i,0x00);
	}
	chip->noise_cnt		= 0;
	chip->noise_rng		= 1;
	chip->noise_clocks	= 0;

	chip->control1		= 0;
	chip->EN_out16[0]	= 0;
	chip->EN_out8[0]	= 0;
	chip->EN_out4[0]	= 0;
	chip->EN_out2[0]	= 0;

	chip->control2		= 0;
	chip->EN_out16[1]	= 0;
	chip->EN_out8[1]	= 0;
	chip->EN_out4[1]	= 0;
	chip->EN_out2[1]	= 0;

}

static void msm5232_init(MSM5232 *chip, const struct MSM5232interface *intf, int clock, int rate)
{
	int j;

	chip->clock = clock;
	chip->rate  = rate ? rate : 44100;	/* avoid division by 0 */

	for (j=0; j<8; j++)
	{
		chip->external_capacity[j] = intf->capacity[j];
	}

	msm5232_init_tables( chip );

	for (j=0; j<8; j++)
	{
		memset(&chip->voi[j],0,sizeof(VOICE));
		msm5232_init_voice(chip,j);
	}
	msm5232_reset( chip );
}

static void msm5232_shutdown(void *chip)
{
#ifdef SAVE_SAMPLE
	fclose(sample[8]);
#endif
#ifdef SAVE_SEPARATE_CHANNELS
	fclose(sample[0]);
	fclose(sample[1]);
	fclose(sample[2]);
	fclose(sample[3]);
	fclose(sample[4]);
	fclose(sample[5]);
	fclose(sample[6]);
	fclose(sample[7]);
#endif
}

static void msm5232_write(MSM5232 *chip, int ofst, int data)
{
	if (ofst > 0x0d)
		return;

	if (ofst < 0x08) /* pitch */
	{
		int ch = ofst&7;
		if(data&0x80)
		{
			if(data >= 0xd8)
			{
				/*if ((data&0x7f) != 0x5f) logerror("MSM5232: WRONG PITCH CODE = %2x\n",data&0x7f);*/
				chip->voi[ch].mode = 1;		/* noise mode */
				chip->voi[ch].eg_sect = 0;	/* Key On */
			}
			else
			{
				if ( chip->voi[ch].pitch != (data&0x7f) )
				{
					int n;
					UINT16 pg;

					chip->voi[ch].pitch = data&0x7f;

					pg = MSM5232_ROM[ data&0x7f ];

					chip->voi[ch].TG_count_period = (pg & 0x1ff) * chip->UpdateStep / 2;

					n = (pg>>9) & 7;	/* n = bit number for 16' output */
					chip->voi[ch].TG_out16 = 1<<n;
										/* for 8' it is bit n-1 (bit 0 if n-1<0) */
										/* for 4' it is bit n-2 (bit 0 if n-2<0) */
										/* for 2' it is bit n-3 (bit 0 if n-3<0) */
					n = (n>0)? n-1: 0;
					chip->voi[ch].TG_out8  = 1<<n;

					n = (n>0)? n-1: 0;
					chip->voi[ch].TG_out4  = 1<<n;

					n = (n>0)? n-1: 0;
					chip->voi[ch].TG_out2  = 1<<n;

				}
				chip->voi[ch].mode = 0;		/* tone mode */
				chip->voi[ch].eg_sect = 0;	/* Key On */
			}
		}
		else
		{
				if ( !chip->voi[ch].eg_arm )	/* arm = 0 */
					chip->voi[ch].eg_sect = 2;	/* Key Off -> go to release */
				else							/* arm = 1 */
					chip->voi[ch].eg_sect = 1;	/* Key Off -> go to decay */
		}
	}
	else
	{
		int i;
		switch(ofst)
		{
		case 0x08:	/* group1 attack */
			for (i=0; i<4; i++)
				chip->voi[i].ar_rate   = chip->ar_tbl[data&0x7] * chip->external_capacity[i];
			break;

		case 0x09:	/* group2 attack */
			for (i=0; i<4; i++)
				chip->voi[i+4].ar_rate = chip->ar_tbl[data&0x7] * chip->external_capacity[i+4];
			break;

		case 0x0a:	/* group1 decay */
			for (i=0; i<4; i++)
				chip->voi[i].dr_rate   = chip->dr_tbl[data&0xf] * chip->external_capacity[i];
			break;

		case 0x0b:	/* group2 decay */
			for (i=0; i<4; i++)
				chip->voi[i+4].dr_rate = chip->dr_tbl[data&0xf] * chip->external_capacity[i+4];
			break;

		case 0x0c:	/* group1 control */

			/*if (chip->control1 != data)
                logerror("msm5232: control1 ctrl=%x OE=%x\n", data&0xf0, data&0x0f);*/

			/*if (data & 0x10)
                popmessage("msm5232: control1 ctrl=%2x\n", data);*/

			chip->control1 = data;

			for (i=0; i<4; i++)
				chip->voi[i].eg_arm = data&0x10;

			chip->EN_out16[0] = (data&1) ? ~0:0;
			chip->EN_out8[0]  = (data&2) ? ~0:0;
			chip->EN_out4[0]  = (data&4) ? ~0:0;
			chip->EN_out2[0]  = (data&8) ? ~0:0;

			break;

		case 0x0d:	/* group2 control */

			/*if (chip->control2 != data)
                logerror("msm5232: control2 ctrl=%x OE=%x\n", data&0xf0, data&0x0f);*/

			/*if (data & 0x10)
                popmessage("msm5232: control2 ctrl=%2x\n", data);*/

			chip->control2 = data;

			for (i=0; i<4; i++)
				chip->voi[i+4].eg_arm = data&0x10;

			chip->EN_out16[1] = (data&1) ? ~0:0;
			chip->EN_out8[1]  = (data&2) ? ~0:0;
			chip->EN_out4[1]  = (data&4) ? ~0:0;
			chip->EN_out2[1]  = (data&8) ? ~0:0;

			break;
		}
	}
}



#define VMIN	0
#define VMAX	32768


INLINE void EG_voices_advance(MSM5232 *chip)
{
	VOICE *voi = &chip->voi[0];
	int samplerate = chip->rate;
	int i;

	i = 8;
	do
	{
		switch(voi->eg_sect)
		{
		case 0:	/* attack */

			/* capacitor charge */
			if (voi->eg < VMAX)
			{
				voi->counter -= (int)((VMAX - voi->eg) / voi->ar_rate);
				if ( voi->counter <= 0 )
				{
					int n = -voi->counter / samplerate + 1;
					voi->counter += n * samplerate;
					if ( (voi->eg += n) > VMAX )
						voi->eg = VMAX;
				}
			}

			/* when ARM=0, EG switches to decay as soon as cap is charged to VT (EG inversion voltage; about 80% of MAX) */
			if (!voi->eg_arm)
			{
				if(voi->eg >= VMAX * 80/100 )
				{
					voi->eg_sect = 1;
				}
			}
			else
			/* ARM=1 */
			{
				/* when ARM=1, EG stays at maximum until key off */
			}

			voi->egvol = voi->eg / 16; /*32768/16 = 2048 max*/

			break;

		case 1: /* decay */

			/* capacitor discharge */
			if (voi->eg > VMIN)
			{
				voi->counter -= (int)((voi->eg - VMIN) / voi->dr_rate);
				if ( voi->counter <= 0 )
				{
					int n = -voi->counter / samplerate + 1;
					voi->counter += n * samplerate;
					if ( (voi->eg -= n) < VMIN )
						voi->eg = VMIN;
				}
			}
			else /* voi->eg <= VMIN */
			{
				voi->eg_sect =-1;
			}

			voi->egvol = voi->eg / 16; /*32768/16 = 2048 max*/

			break;

		case 2: /* release */

			/* capacitor discharge */
			if (voi->eg > VMIN)
			{
				voi->counter -= (int)((voi->eg - VMIN) / voi->rr_rate);
				if ( voi->counter <= 0 )
				{
					int n = -voi->counter / samplerate + 1;
					voi->counter += n * samplerate;
					if ( (voi->eg -= n) < VMIN )
						voi->eg = VMIN;
				}
			}
			else /* voi->eg <= VMIN */
			{
				voi->eg_sect =-1;
			}

			voi->egvol = voi->eg / 16; /*32768/16 = 2048 max*/

			break;

		default:
			break;
		}

		voi++;
		i--;
	} while (i>0);

}

static int o2,o4,o8,o16;

INLINE void TG_group_advance(MSM5232 *chip, int groupidx)
{
	VOICE *voi = &chip->voi[groupidx*4];
	int i;

	o2 = o4 = o8 = o16 = 0;

	i=4;
	do
	{
		int out2, out4, out8, out16;

		out2 = out4 = out8 = out16 = 0;

		if (voi->mode==0)	/* generate square tone */
		{
			int left = 1<<STEP_SH;
			do
			{
				int nextevent = left;

				if (voi->TG_cnt&voi->TG_out16)	out16+=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out8)	out8 +=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out4)	out4 +=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out2)	out2 +=voi->TG_count;

				voi->TG_count -= nextevent;

				while (voi->TG_count <= 0)
				{
					voi->TG_count += voi->TG_count_period;
					voi->TG_cnt++;
					if (voi->TG_cnt&voi->TG_out16)	out16+=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out8 )	out8 +=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out4 )	out4 +=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out2 )	out2 +=voi->TG_count_period;

					if (voi->TG_count > 0)
						break;

					voi->TG_count += voi->TG_count_period;
					voi->TG_cnt++;
					if (voi->TG_cnt&voi->TG_out16)	out16+=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out8 )	out8 +=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out4 )	out4 +=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out2 )	out2 +=voi->TG_count_period;
				}
				if (voi->TG_cnt&voi->TG_out16)	out16-=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out8 )	out8 -=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out4 )	out4 -=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out2 )	out2 -=voi->TG_count;

				left -=nextevent;

			}while (left>0);
		}
		else	/* generate noise */
		{
			if (chip->noise_clocks&8)	out16+=(1<<STEP_SH);
			if (chip->noise_clocks&4)	out8 +=(1<<STEP_SH);
			if (chip->noise_clocks&2)	out4 +=(1<<STEP_SH);
			if (chip->noise_clocks&1)	out2 +=(1<<STEP_SH);
		}

		/* calculate signed output */
		o16 += ( (out16-(1<<(STEP_SH-1))) * voi->egvol) >> STEP_SH;
		o8  += ( (out8 -(1<<(STEP_SH-1))) * voi->egvol) >> STEP_SH;
		o4  += ( (out4 -(1<<(STEP_SH-1))) * voi->egvol) >> STEP_SH;
		o2  += ( (out2 -(1<<(STEP_SH-1))) * voi->egvol) >> STEP_SH;
		voi++;
		i--;
	}while (i>0);

	/* cut off disabled output lines */
	o16 &= chip->EN_out16[groupidx];
	o8  &= chip->EN_out8 [groupidx];
	o4  &= chip->EN_out4 [groupidx];
	o2  &= chip->EN_out2 [groupidx];
}


/* macro saves feet data to mono file */
#ifdef SAVE_SEPARATE_CHANNELS
  #define SAVE_SINGLE_CHANNEL(j,val) \
  {	signed int pom= val; \
	if (pom > 32767) pom = 32767; else if (pom < -32768) pom = -32768; \
	fputc((unsigned short)pom&0xff,sample[j]); \
	fputc(((unsigned short)pom>>8)&0xff,sample[j]);  }
#else
  #define SAVE_SINGLE_CHANNEL(j,val)
#endif

/* first macro saves all 8 feet outputs to mixed (mono) file */
/* second macro saves one group into left and the other in right channel */
#if 1	/*MONO*/
	#ifdef SAVE_SAMPLE
	  #define SAVE_ALL_CHANNELS \
	  {	signed int pom = buf1[i] + buf2[i]; \
		fputc((unsigned short)pom&0xff,sample[8]); \
		fputc(((unsigned short)pom>>8)&0xff,sample[8]); \
	  }
	#else
	  #define SAVE_ALL_CHANNELS
	#endif
#else	/*STEREO*/
	#ifdef SAVE_SAMPLE
	  #define SAVE_ALL_CHANNELS \
	  {	signed int pom = buf1[i]; \
		fputc((unsigned short)pom&0xff,sample[8]); \
		fputc(((unsigned short)pom>>8)&0xff,sample[8]); \
		pom = buf2[i]; \
		fputc((unsigned short)pom&0xff,sample[8]); \
		fputc(((unsigned short)pom>>8)&0xff,sample[8]); \
	  }
	#else
	  #define SAVE_ALL_CHANNELS
	#endif
#endif


void MSM5232_update_one(void *param, stream_sample_t **inputs, stream_sample_t** buffer, int samples)
{
	MSM5232 * chip = param;
	stream_sample_t *buf1 = buffer[0];
	stream_sample_t *buf2 = buffer[1];
	int i;

	for (i=0; i<samples; i++)
	{
		int out;

		/* calculate all voices' envelopes */
		EG_voices_advance(chip);

		TG_group_advance(chip,0);	/* calculate tones group 1 */
		out = (o2+o4+o8+o16);
		if (out>32767)
			out = 32767;
		else if (out<-32768)
			out = -32768;
		buf1[i] = out;				/* should be 4 separate outputs */
		SAVE_SINGLE_CHANNEL(0,o2)
		SAVE_SINGLE_CHANNEL(1,o4)
		SAVE_SINGLE_CHANNEL(2,o8)
		SAVE_SINGLE_CHANNEL(3,o16)

		TG_group_advance(chip,1);	/* calculate tones group 2 */
		out = (o2+o4+o8+o16);
		if (out>32767)
			out = 32767;
		else if (out<-32768)
			out = -32768;
		buf2[i] = out;				/* should be 4 separate outputs */
		SAVE_SINGLE_CHANNEL(4,o2)
		SAVE_SINGLE_CHANNEL(5,o4)
		SAVE_SINGLE_CHANNEL(6,o8)
		SAVE_SINGLE_CHANNEL(7,o16)

		SAVE_ALL_CHANNELS

		/* update noise generator */
		{
			int cnt = (chip->noise_cnt+=chip->noise_step) >> STEP_SH;
			chip->noise_cnt &= ((1<<STEP_SH)-1);
			while (cnt > 0)
			{
				int tmp = chip->noise_rng & (1<<16);		/* store current level */

				if (chip->noise_rng&1)
					chip->noise_rng ^= 0x28000;
				chip->noise_rng>>=1;

				if ( (chip->noise_rng & (1<<16)) != tmp )	/* level change detect */
					chip->noise_clocks++;

				cnt--;
			}
		}
	}
}



/* MAME Interface */

static void *msm5232_start(int sndindex, int clock, const void *config)
{
	const struct MSM5232interface *intf = config;
	int rate = clock/16;
	MSM5232 *chip;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	msm5232_init(chip, intf, clock, rate);

	chip->stream = stream_create(0,2,rate,chip,MSM5232_update_one);
	return chip;
}

static void msm5232_stop (void *chip)
{
	msm5232_shutdown(chip);
}

WRITE8_HANDLER ( MSM5232_0_w )
{
	MSM5232 *chip = sndti_token(SOUND_MSM5232, 0);
	stream_update (chip->stream);
	msm5232_write(chip, offset, data);
}

WRITE8_HANDLER ( MSM5232_1_w )
{
	MSM5232 *chip = sndti_token(SOUND_MSM5232, 1);
	stream_update (chip->stream);
	msm5232_write(chip, offset, data);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void msm5232_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void msm5232_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = msm5232_set_info;		break;
		case SNDINFO_PTR_START:							info->start = msm5232_start;			break;
		case SNDINFO_PTR_STOP:							info->stop = msm5232_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = msm5232_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "MSM5232";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "ADPCM";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

