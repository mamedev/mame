/***************************************************************************

  ay8910.c


  Emulation of the AY-3-8910 / YM2149 sound chip.

  Based on various code snippets by Ville Hallik, Michael Cuddy,
  Tatsuyuki Satoh, Fabrice Frances, Nicola Salmoria.

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "cpuintrf.h"
#include "ay8910.h"

#define MAX_OUTPUT 0x7fff

#define STEP 2


struct AY8910
{
	int index;
	int streams;
	int ready;
	sound_stream *Channel;
	read8_handler PortAread;
	read8_handler PortBread;
	write8_handler PortAwrite;
	write8_handler PortBwrite;
	INT32 register_latch;
	UINT8 Regs[16];
	INT32 lastEnable;
	INT32 PeriodA,PeriodB,PeriodC,PeriodN,PeriodE;
	INT32 CountA,CountB,CountC,CountN,CountE;
	UINT32 VolA,VolB,VolC,VolE;
	UINT8 EnvelopeA,EnvelopeB,EnvelopeC;
	UINT8 OutputA,OutputB,OutputC,OutputN;
	INT8 CountEnv;
	UINT8 Hold,Alternate,Attack,Holding;
	INT32 RNG;
	unsigned int VolTable[32];
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


static void _AYWriteReg(struct AY8910 *PSG, int r, int v)
{
	int old;


	PSG->Regs[r] = v;

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
		PSG->Regs[AY_ACOARSE] &= 0x0f;
		old = PSG->PeriodA;
		PSG->PeriodA = (PSG->Regs[AY_AFINE] + 256 * PSG->Regs[AY_ACOARSE]) * STEP;
		if (PSG->PeriodA == 0) PSG->PeriodA = STEP;
		PSG->CountA += PSG->PeriodA - old;
		if (PSG->CountA <= 0) PSG->CountA = 1;
		break;
	case AY_BFINE:
	case AY_BCOARSE:
		PSG->Regs[AY_BCOARSE] &= 0x0f;
		old = PSG->PeriodB;
		PSG->PeriodB = (PSG->Regs[AY_BFINE] + 256 * PSG->Regs[AY_BCOARSE]) * STEP;
		if (PSG->PeriodB == 0) PSG->PeriodB = STEP;
		PSG->CountB += PSG->PeriodB - old;
		if (PSG->CountB <= 0) PSG->CountB = 1;
		break;
	case AY_CFINE:
	case AY_CCOARSE:
		PSG->Regs[AY_CCOARSE] &= 0x0f;
		old = PSG->PeriodC;
		PSG->PeriodC = (PSG->Regs[AY_CFINE] + 256 * PSG->Regs[AY_CCOARSE]) * STEP;
		if (PSG->PeriodC == 0) PSG->PeriodC = STEP;
		PSG->CountC += PSG->PeriodC - old;
		if (PSG->CountC <= 0) PSG->CountC = 1;
		break;
	case AY_NOISEPER:
		PSG->Regs[AY_NOISEPER] &= 0x1f;
		old = PSG->PeriodN;
		PSG->PeriodN = PSG->Regs[AY_NOISEPER] * STEP;
		if (PSG->PeriodN == 0) PSG->PeriodN = STEP;
		PSG->CountN += PSG->PeriodN - old;
		if (PSG->CountN <= 0) PSG->CountN = 1;
		break;
	case AY_ENABLE:
		if ((PSG->lastEnable == -1) ||
		    ((PSG->lastEnable & 0x40) != (PSG->Regs[AY_ENABLE] & 0x40)))
		{
			/* write out 0xff if port set to input */
			if (PSG->PortAwrite)
				(*PSG->PortAwrite)(0, (PSG->Regs[AY_ENABLE] & 0x40) ? PSG->Regs[AY_PORTA] : 0xff);
		}

		if ((PSG->lastEnable == -1) ||
		    ((PSG->lastEnable & 0x80) != (PSG->Regs[AY_ENABLE] & 0x80)))
		{
			/* write out 0xff if port set to input */
			if (PSG->PortBwrite)
				(*PSG->PortBwrite)(0, (PSG->Regs[AY_ENABLE] & 0x80) ? PSG->Regs[AY_PORTB] : 0xff);
		}

		PSG->lastEnable = PSG->Regs[AY_ENABLE];
		break;
	case AY_AVOL:
		PSG->Regs[AY_AVOL] &= 0x1f;
		PSG->EnvelopeA = PSG->Regs[AY_AVOL] & 0x10;
		PSG->VolA = PSG->EnvelopeA ? PSG->VolE : PSG->VolTable[PSG->Regs[AY_AVOL] ? PSG->Regs[AY_AVOL]*2+1 : 0];
		break;
	case AY_BVOL:
		PSG->Regs[AY_BVOL] &= 0x1f;
		PSG->EnvelopeB = PSG->Regs[AY_BVOL] & 0x10;
		PSG->VolB = PSG->EnvelopeB ? PSG->VolE : PSG->VolTable[PSG->Regs[AY_BVOL] ? PSG->Regs[AY_BVOL]*2+1 : 0];
		break;
	case AY_CVOL:
		PSG->Regs[AY_CVOL] &= 0x1f;
		PSG->EnvelopeC = PSG->Regs[AY_CVOL] & 0x10;
		PSG->VolC = PSG->EnvelopeC ? PSG->VolE : PSG->VolTable[PSG->Regs[AY_CVOL] ? PSG->Regs[AY_CVOL]*2+1 : 0];
		break;
	case AY_EFINE:
	case AY_ECOARSE:
		old = PSG->PeriodE;
		PSG->PeriodE = ((PSG->Regs[AY_EFINE] + 256 * PSG->Regs[AY_ECOARSE])) * STEP;
		if (PSG->PeriodE == 0) PSG->PeriodE = STEP / 2;
		PSG->CountE += PSG->PeriodE - old;
		if (PSG->CountE <= 0) PSG->CountE = 1;
		break;
	case AY_ESHAPE:
		/* envelope shapes:
        C AtAlH
        0 0 x x  \___

        0 1 x x  /___

        1 0 0 0  \\\\

        1 0 0 1  \___

        1 0 1 0  \/\/
                  ___
        1 0 1 1  \

        1 1 0 0  ////
                  ___
        1 1 0 1  /

        1 1 1 0  /\/\

        1 1 1 1  /___

        The envelope counter on the AY-3-8910 has 16 steps. On the YM2149 it
        has twice the steps, happening twice as fast. Since the end result is
        just a smoother curve, we always use the YM2149 behaviour.
        */
		PSG->Regs[AY_ESHAPE] &= 0x0f;
		PSG->Attack = (PSG->Regs[AY_ESHAPE] & 0x04) ? 0x1f : 0x00;
		if ((PSG->Regs[AY_ESHAPE] & 0x08) == 0)
		{
			/* if Continue = 0, map the shape to the equivalent one which has Continue = 1 */
			PSG->Hold = 1;
			PSG->Alternate = PSG->Attack;
		}
		else
		{
			PSG->Hold = PSG->Regs[AY_ESHAPE] & 0x01;
			PSG->Alternate = PSG->Regs[AY_ESHAPE] & 0x02;
		}
		PSG->CountE = PSG->PeriodE;
		PSG->CountEnv = 0x1f;
		PSG->Holding = 0;
		PSG->VolE = PSG->VolTable[PSG->CountEnv ^ PSG->Attack];
		if (PSG->EnvelopeA) PSG->VolA = PSG->VolE;
		if (PSG->EnvelopeB) PSG->VolB = PSG->VolE;
		if (PSG->EnvelopeC) PSG->VolC = PSG->VolE;
		break;
	case AY_PORTA:
		if (PSG->Regs[AY_ENABLE] & 0x40)
		{
			if (PSG->PortAwrite)
				(*PSG->PortAwrite)(0, PSG->Regs[AY_PORTA]);
			else
				logerror("warning - write %02x to 8910 #%d Port A\n",PSG->Regs[AY_PORTA],PSG->index);
		}
		else
		{
			logerror("warning: write to 8910 #%d Port A set as input - ignored\n",PSG->index);
		}
		break;
	case AY_PORTB:
		if (PSG->Regs[AY_ENABLE] & 0x80)
		{
			if (PSG->PortBwrite)
				(*PSG->PortBwrite)(0, PSG->Regs[AY_PORTB]);
			else
				logerror("warning - write %02x to 8910 #%d Port B\n",PSG->Regs[AY_PORTB],PSG->index);
		}
		else
		{
			logerror("warning: write to 8910 #%d Port B set as input - ignored\n",PSG->index);
		}
		break;
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
WRITE16_HANDLER( AY8910_control_port_0_lsb_w ) { if (ACCESSING_LSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 0),0,data & 0xff); }
WRITE16_HANDLER( AY8910_control_port_1_lsb_w ) { if (ACCESSING_LSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 1),0,data & 0xff); }
WRITE16_HANDLER( AY8910_control_port_2_lsb_w ) { if (ACCESSING_LSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 2),0,data & 0xff); }
WRITE16_HANDLER( AY8910_control_port_3_lsb_w ) { if (ACCESSING_LSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 3),0,data & 0xff); }
WRITE16_HANDLER( AY8910_control_port_4_lsb_w ) { if (ACCESSING_LSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 4),0,data & 0xff); }
WRITE16_HANDLER( AY8910_control_port_0_msb_w ) { if (ACCESSING_MSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 0),0,data >> 8); }
WRITE16_HANDLER( AY8910_control_port_1_msb_w ) { if (ACCESSING_MSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 1),0,data >> 8); }
WRITE16_HANDLER( AY8910_control_port_2_msb_w ) { if (ACCESSING_MSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 2),0,data >> 8); }
WRITE16_HANDLER( AY8910_control_port_3_msb_w ) { if (ACCESSING_MSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 3),0,data >> 8); }
WRITE16_HANDLER( AY8910_control_port_4_msb_w ) { if (ACCESSING_MSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 4),0,data >> 8); }

WRITE8_HANDLER( AY8910_write_port_0_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 0),1,data); }
WRITE8_HANDLER( AY8910_write_port_1_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 1),1,data); }
WRITE8_HANDLER( AY8910_write_port_2_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 2),1,data); }
WRITE8_HANDLER( AY8910_write_port_3_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 3),1,data); }
WRITE8_HANDLER( AY8910_write_port_4_w ) { ay8910_write_ym(sndti_token(SOUND_AY8910, 4),1,data); }
WRITE16_HANDLER( AY8910_write_port_0_lsb_w ) { if (ACCESSING_LSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 0),1,data & 0xff); }
WRITE16_HANDLER( AY8910_write_port_1_lsb_w ) { if (ACCESSING_LSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 1),1,data & 0xff); }
WRITE16_HANDLER( AY8910_write_port_2_lsb_w ) { if (ACCESSING_LSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 2),1,data & 0xff); }
WRITE16_HANDLER( AY8910_write_port_3_lsb_w ) { if (ACCESSING_LSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 3),1,data & 0xff); }
WRITE16_HANDLER( AY8910_write_port_4_lsb_w ) { if (ACCESSING_LSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 4),1,data & 0xff); }
WRITE16_HANDLER( AY8910_write_port_0_msb_w ) { if (ACCESSING_MSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 0),1,data >> 8); }
WRITE16_HANDLER( AY8910_write_port_1_msb_w ) { if (ACCESSING_MSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 1),1,data >> 8); }
WRITE16_HANDLER( AY8910_write_port_2_msb_w ) { if (ACCESSING_MSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 2),1,data >> 8); }
WRITE16_HANDLER( AY8910_write_port_3_msb_w ) { if (ACCESSING_MSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 3),1,data >> 8); }
WRITE16_HANDLER( AY8910_write_port_4_msb_w ) { if (ACCESSING_MSB) ay8910_write_ym(sndti_token(SOUND_AY8910, 4),1,data >> 8); }



static void AY8910Update(void *param,stream_sample_t **inputs, stream_sample_t **buffer,int length)
{
	struct AY8910 *PSG = param;
	stream_sample_t *buf1,*buf2,*buf3;
	int outn;

	buf1 = buffer[0];
	buf2 = NULL;
	buf3 = NULL;
	if (PSG->streams == 3)
	{
		buf2 = buffer[1];
		buf3 = buffer[2];
	}

	/* hack to prevent us from hanging when starting filtered outputs */
	if (!PSG->ready)
	{
		memset(buf1, 0, length * sizeof(*buf1));
		if (buf2)
			memset(buf2, 0, length * sizeof(*buf2));
		if (buf3)
			memset(buf3, 0, length * sizeof(*buf3));
		return;
	}


	/* The 8910 has three outputs, each output is the mix of one of the three */
	/* tone generators and of the (single) noise generator. The two are mixed */
	/* BEFORE going into the DAC. The formula to mix each channel is: */
	/* (ToneOn | ToneDisable) & (NoiseOn | NoiseDisable). */
	/* Note that this means that if both tone and noise are disabled, the output */
	/* is 1, not 0, and can be modulated changing the volume. */


	/* If the channels are disabled, set their output to 1, and increase the */
	/* counter, if necessary, so they will not be inverted during this update. */
	/* Setting the output to 1 is necessary because a disabled channel is locked */
	/* into the ON state (see above); and it has no effect if the volume is 0. */
	/* If the volume is 0, increase the counter, but don't touch the output. */
	if (PSG->Regs[AY_ENABLE] & 0x01)
	{
		if (PSG->CountA <= length*STEP) PSG->CountA += length*STEP;
		PSG->OutputA = 1;
	}
	else if (PSG->Regs[AY_AVOL] == 0)
	{
		/* note that I do count += length, NOT count = length + 1. You might think */
		/* it's the same since the volume is 0, but doing the latter could cause */
		/* interferencies when the program is rapidly modulating the volume. */
		if (PSG->CountA <= length*STEP) PSG->CountA += length*STEP;
	}
	if (PSG->Regs[AY_ENABLE] & 0x02)
	{
		if (PSG->CountB <= length*STEP) PSG->CountB += length*STEP;
		PSG->OutputB = 1;
	}
	else if (PSG->Regs[AY_BVOL] == 0)
	{
		if (PSG->CountB <= length*STEP) PSG->CountB += length*STEP;
	}
	if (PSG->Regs[AY_ENABLE] & 0x04)
	{
		if (PSG->CountC <= length*STEP) PSG->CountC += length*STEP;
		PSG->OutputC = 1;
	}
	else if (PSG->Regs[AY_CVOL] == 0)
	{
		if (PSG->CountC <= length*STEP) PSG->CountC += length*STEP;
	}

	/* for the noise channel we must not touch OutputN - it's also not necessary */
	/* since we use outn. */
	if ((PSG->Regs[AY_ENABLE] & 0x38) == 0x38)	/* all off */
		if (PSG->CountN <= length*STEP) PSG->CountN += length*STEP;

	outn = (PSG->OutputN | PSG->Regs[AY_ENABLE]);


	/* buffering loop */
	while (length)
	{
		int vola,volb,volc;
		int left;


		/* vola, volb and volc keep track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		vola = volb = volc = 0;

		left = STEP;
		do
		{
			int nextevent;


			if (PSG->CountN < left) nextevent = PSG->CountN;
			else nextevent = left;

			if (outn & 0x08)
			{
				if (PSG->OutputA) vola += PSG->CountA;
				PSG->CountA -= nextevent;
				/* PeriodA is the half period of the square wave. Here, in each */
				/* loop I add PeriodA twice, so that at the end of the loop the */
				/* square wave is in the same status (0 or 1) it was at the start. */
				/* vola is also incremented by PeriodA, since the wave has been 1 */
				/* exactly half of the time, regardless of the initial position. */
				/* If we exit the loop in the middle, OutputA has to be inverted */
				/* and vola incremented only if the exit status of the square */
				/* wave is 1. */
				while (PSG->CountA <= 0)
				{
					PSG->CountA += PSG->PeriodA;
					if (PSG->CountA > 0)
					{
						PSG->OutputA ^= 1;
						if (PSG->OutputA) vola += PSG->PeriodA;
						break;
					}
					PSG->CountA += PSG->PeriodA;
					vola += PSG->PeriodA;
				}
				if (PSG->OutputA) vola -= PSG->CountA;
			}
			else
			{
				PSG->CountA -= nextevent;
				while (PSG->CountA <= 0)
				{
					PSG->CountA += PSG->PeriodA;
					if (PSG->CountA > 0)
					{
						PSG->OutputA ^= 1;
						break;
					}
					PSG->CountA += PSG->PeriodA;
				}
			}

			if (outn & 0x10)
			{
				if (PSG->OutputB) volb += PSG->CountB;
				PSG->CountB -= nextevent;
				while (PSG->CountB <= 0)
				{
					PSG->CountB += PSG->PeriodB;
					if (PSG->CountB > 0)
					{
						PSG->OutputB ^= 1;
						if (PSG->OutputB) volb += PSG->PeriodB;
						break;
					}
					PSG->CountB += PSG->PeriodB;
					volb += PSG->PeriodB;
				}
				if (PSG->OutputB) volb -= PSG->CountB;
			}
			else
			{
				PSG->CountB -= nextevent;
				while (PSG->CountB <= 0)
				{
					PSG->CountB += PSG->PeriodB;
					if (PSG->CountB > 0)
					{
						PSG->OutputB ^= 1;
						break;
					}
					PSG->CountB += PSG->PeriodB;
				}
			}

			if (outn & 0x20)
			{
				if (PSG->OutputC) volc += PSG->CountC;
				PSG->CountC -= nextevent;
				while (PSG->CountC <= 0)
				{
					PSG->CountC += PSG->PeriodC;
					if (PSG->CountC > 0)
					{
						PSG->OutputC ^= 1;
						if (PSG->OutputC) volc += PSG->PeriodC;
						break;
					}
					PSG->CountC += PSG->PeriodC;
					volc += PSG->PeriodC;
				}
				if (PSG->OutputC) volc -= PSG->CountC;
			}
			else
			{
				PSG->CountC -= nextevent;
				while (PSG->CountC <= 0)
				{
					PSG->CountC += PSG->PeriodC;
					if (PSG->CountC > 0)
					{
						PSG->OutputC ^= 1;
						break;
					}
					PSG->CountC += PSG->PeriodC;
				}
			}

			PSG->CountN -= nextevent;
			if (PSG->CountN <= 0)
			{
				/* Is noise output going to change? */
				if ((PSG->RNG + 1) & 2)	/* (bit0^bit1)? */
				{
					PSG->OutputN = ~PSG->OutputN;
					outn = (PSG->OutputN | PSG->Regs[AY_ENABLE]);
				}

				/* The Random Number Generator of the 8910 is a 17-bit shift */
				/* register. The input to the shift register is bit0 XOR bit3 */
				/* (bit0 is the output). This was verified on AY-3-8910 and YM2149 chips. */

				/* The following is a fast way to compute bit17 = bit0^bit3. */
				/* Instead of doing all the logic operations, we only check */
				/* bit0, relying on the fact that after three shifts of the */
				/* register, what now is bit3 will become bit0, and will */
				/* invert, if necessary, bit14, which previously was bit17. */
				if (PSG->RNG & 1) PSG->RNG ^= 0x24000; /* This version is called the "Galois configuration". */
				PSG->RNG >>= 1;
				PSG->CountN += PSG->PeriodN;
			}

			left -= nextevent;
		} while (left > 0);

		/* update envelope */
		if (PSG->Holding == 0)
		{
			PSG->CountE -= STEP;
			if (PSG->CountE <= 0)
			{
				do
				{
					PSG->CountEnv--;
					PSG->CountE += PSG->PeriodE;
				} while (PSG->CountE <= 0);

				/* check envelope current position */
				if (PSG->CountEnv < 0)
				{
					if (PSG->Hold)
					{
						if (PSG->Alternate)
							PSG->Attack ^= 0x1f;
						PSG->Holding = 1;
						PSG->CountEnv = 0;
					}
					else
					{
						/* if CountEnv has looped an odd number of times (usually 1), */
						/* invert the output. */
						if (PSG->Alternate && (PSG->CountEnv & 0x20))
 							PSG->Attack ^= 0x1f;

						PSG->CountEnv &= 0x1f;
					}
				}

				PSG->VolE = PSG->VolTable[PSG->CountEnv ^ PSG->Attack];
				/* reload volume */
				if (PSG->EnvelopeA) PSG->VolA = PSG->VolE;
				if (PSG->EnvelopeB) PSG->VolB = PSG->VolE;
				if (PSG->EnvelopeC) PSG->VolC = PSG->VolE;
			}
		}

		if (PSG->streams == 3)
		{
			*(buf1++) = (vola * PSG->VolA) / STEP;
			*(buf2++) = (volb * PSG->VolB) / STEP;
			*(buf3++) = (volc * PSG->VolC) / STEP;
		}
		else
			*(buf1++) = (vola * PSG->VolA + volb * PSG->VolB + volc * PSG->VolC) / STEP;

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


static void build_mixer_table(struct AY8910 *PSG)
{
	int i;
	double out;


	/* calculate the volume->voltage conversion table */
	/* The AY-3-8910 has 16 levels, in a logarithmic scale (3dB per step) */
	/* The YM2149 still has 16 levels for the tone generators, but 32 for */
	/* the envelope generator (1.5dB per step). */
	out = MAX_OUTPUT;
	for (i = 31;i > 0;i--)
	{
		PSG->VolTable[i] = out + 0.5;	/* round to nearest */

		out /= 1.188502227;	/* = 10 ^ (1.5/20) = 1.5dB */
	}
	PSG->VolTable[0] = 0;
}



static void AY8910_init(struct AY8910 *PSG, int streams,
		int clock,int sample_rate,
		read8_handler portAread,read8_handler portBread,
		write8_handler portAwrite,write8_handler portBwrite)
{
	PSG->PortAread = portAread;
	PSG->PortBread = portBread;
	PSG->PortAwrite = portAwrite;
	PSG->PortBwrite = portBwrite;

	/* the step clock for the tone and noise generators is the chip clock    */
	/* divided by 8; for the envelope generator of the AY-3-8910, it is half */
	/* that much (clock/16), but the envelope of the YM2149 goes twice as    */
	/* fast, therefore again clock/8.                                        */
	PSG->Channel = stream_create(0,streams,clock/8,PSG,AY8910Update);

	ay8910_set_clock_ym(PSG,clock);
}


static void AY8910_statesave(struct AY8910 *PSG, int sndindex)
{
	state_save_register_item("AY8910", sndindex, PSG->register_latch);
	state_save_register_item_array("AY8910", sndindex, PSG->Regs);
	state_save_register_item("AY8910", sndindex, PSG->lastEnable);

	state_save_register_item("AY8910", sndindex, PSG->PeriodA);
	state_save_register_item("AY8910", sndindex, PSG->PeriodB);
	state_save_register_item("AY8910", sndindex, PSG->PeriodC);
	state_save_register_item("AY8910", sndindex, PSG->PeriodN);
	state_save_register_item("AY8910", sndindex, PSG->PeriodE);

	state_save_register_item("AY8910", sndindex, PSG->CountA);
	state_save_register_item("AY8910", sndindex, PSG->CountB);
	state_save_register_item("AY8910", sndindex, PSG->CountC);
	state_save_register_item("AY8910", sndindex, PSG->CountN);
	state_save_register_item("AY8910", sndindex, PSG->CountE);

	state_save_register_item("AY8910", sndindex, PSG->VolA);
	state_save_register_item("AY8910", sndindex, PSG->VolB);
	state_save_register_item("AY8910", sndindex, PSG->VolC);
	state_save_register_item("AY8910", sndindex, PSG->VolE);

	state_save_register_item("AY8910", sndindex, PSG->EnvelopeA);
	state_save_register_item("AY8910", sndindex, PSG->EnvelopeB);
	state_save_register_item("AY8910", sndindex, PSG->EnvelopeC);

	state_save_register_item("AY8910", sndindex, PSG->OutputA);
	state_save_register_item("AY8910", sndindex, PSG->OutputB);
	state_save_register_item("AY8910", sndindex, PSG->OutputC);
	state_save_register_item("AY8910", sndindex, PSG->OutputN);

	state_save_register_item("AY8910", sndindex, PSG->CountEnv);
	state_save_register_item("AY8910", sndindex, PSG->Hold);
	state_save_register_item("AY8910", sndindex, PSG->Alternate);
	state_save_register_item("AY8910", sndindex, PSG->Attack);
	state_save_register_item("AY8910", sndindex, PSG->Holding);
	state_save_register_item("AY8910", sndindex, PSG->RNG);
}


void *ay8910_start_ym(sound_type chip_type, int sndindex, int clock, int streams,
		read8_handler portAread, read8_handler portBread,
		write8_handler portAwrite, write8_handler portBwrite)
{
	struct AY8910 *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));
	info->index = sndindex;
	info->streams = streams;

	AY8910_init(info, streams, clock,
			clock/8,
			portAread,portBread,
			portAwrite,portBwrite);

	build_mixer_table(info);
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
	PSG->OutputA = 0;
	PSG->OutputB = 0;
	PSG->OutputC = 0;
	PSG->OutputN = 0xff;
	PSG->lastEnable = -1;	/* force a write */
	for (i = 0;i < AY_PORTA;i++)
		_AYWriteReg(PSG,i,0);	/* AYWriteReg() uses the timer system; we cannot */
								/* call it at this time because the timer system */
								/* has not been initialized. */
	PSG->ready = 1;
}

void ay8910_set_clock_ym(void *chip, int clock)
{
	struct AY8910 *PSG = chip;
	stream_set_sample_rate(PSG->Channel, clock/8);
}

void ay8910_write_ym(void *chip, int addr, int data)
{
	struct AY8910 *PSG = chip;

	if (addr & 1)
	{	/* Data port */
		int r = PSG->register_latch;

		if (r > 15) return;
		if (r < 14)
		{
			if (r == AY_ESHAPE || PSG->Regs[r] != data)
			{
				/* update the output buffer before changing the register */
				stream_update(PSG->Channel);
			}
		}

		_AYWriteReg(PSG,r,data);
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
		if ((PSG->Regs[AY_ENABLE] & 0x40) != 0)
			logerror("warning: read from 8910 #%d Port A set as output\n",PSG->index);
		/*
           even if the port is set as output, we still need to return the external
           data. Some games, like kidniki, need this to work.
         */
		if (PSG->PortAread) PSG->Regs[AY_PORTA] = (*PSG->PortAread)(0);
		else logerror("PC %04x: warning - read 8910 #%d Port A\n",activecpu_get_pc(),PSG->index);
		break;
	case AY_PORTB:
		if ((PSG->Regs[AY_ENABLE] & 0x80) != 0)
			logerror("warning: read from 8910 #%d Port B set as output\n",PSG->index);
		if (PSG->PortBread) PSG->Regs[AY_PORTB] = (*PSG->PortBread)(0);
		else logerror("PC %04x: warning - read 8910 #%d Port B\n",activecpu_get_pc(),PSG->index);
		break;
	}
	return PSG->Regs[r];
}



static void *ay8910_start(int sndindex, int clock, const void *config)
{
	static const struct AY8910interface generic_ay8910 = { 0 };
	const struct AY8910interface *intf = config ? config : &generic_ay8910;
	return ay8910_start_ym(SOUND_AY8910, sndindex+16, clock, 3, intf->portAread, intf->portBread, intf->portAwrite, intf->portBwrite);
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
		case SNDINFO_PTR_START:							info->start = ay8910_start;				break;
		case SNDINFO_STR_NAME:							info->s = "YM2149";						break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}

void ym3439_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = ay8910_start;				break;
		case SNDINFO_STR_NAME:							info->s = "YM3439";						break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}

void ymz284_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = ay8910_start;				break;
		case SNDINFO_STR_NAME:							info->s = "YMZ284";						break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}

void ymz294_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = ay8910_start;				break;
		case SNDINFO_STR_NAME:							info->s = "YMZ294";						break;
		default: 										ay8910_get_info(token, state, info);	break;
	}
}
