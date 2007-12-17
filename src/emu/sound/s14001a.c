/*

 TSI S14001A emulator v1.11
 By Jonathan Gevaryahu ("Lord Nightmare") with help from Kevin Horton ("kevtris")
 MAME conversion and integration by R. Belmont

 Copyright (c) 2007 Jonathan Gevaryahu.

 Version history:
 0.8 initial version - LN
 0.9 MAME conversion, glue code added - R. Belmont
 1.0 partly fixed stream update - LN (0.111u4)
 1.01 fixed clipping problem - LN (0.111u5)
 1.1 add VSU-1000 features, fully fixed stream update by fixing word latching - LN (0.111u6)
 1.11 fix signedness of output, pre-multiply, fixes clicking on VSU-1000 volume change - LN (0.111u7)

 TODO:
 * increase accuracy of internal S14001A 'filter' for both driven and undriven cycles (its not terribly inaccurate for undriven cycles, but the dc sliding of driven cycles is not emulated)
 * add option for and attach Frank P.'s emulation of the Analog external filter from the vsu-1000 using the discrete core.
*/

/* state map:

 * state machine 1: odd/even clock state
 * on even clocks, audio output is floating, /romen is low so rom data bus is driven, input is latched?
 * on odd clocks, audio output is driven, /romen is high, state machine 2 is clocked
 * *****
 * state machine 2: decoder state
 * NOTE: holding the start line high forces the state machine 2 state to go to or remain in state 1!
 * state 0(Idle): Idle (no sample rom bus activity, output at 0), next state is 0(Idle)

 * state 1(GetHiWord):
 *   grab byte at (wordinput<<1) -> register_WH
 *   reset output DAC accumulator to 0x8 <- ???
 *   reset OldValHi to 1
 *   reset OldValLo to 0
 *   next state is 2(GetLoWord) UNLESS the PLAY line is still high, in which case the state remains at 1

 * state 2(GetLoWord):
 *   grab byte at (wordinput<<1)+1 -> register_WL
 *   next state is 3(GetHiPhon)

 * state 3(GetHiPhon):
 *   grab byte at ((register_WH<<8) + (register_WL))>>4 -> phoneaddress
 *   next state is 4(GetLoPhon)

 * state 4(GetLoPhon):
 *   grab byte at (((register_WH<<8) + (register_WL))>>4)+1 -> playparams
 *   set phonepos register to 0
 *   set oddphone register to 0
 *   next state is 5(PlayForward1)
 *   playparams:
 *   7 6 5 4 3 2 1 0
 *   G                G = LastPhone
 *     B              B = PlayMode
 *       Y            Y = Silenceflag
 *         S S S      S = Length count load value
 *               R R  R = Repeat count reload value (upon carry/overflow of 3 bits)
 *   load the repeat counter with the bits 'R R 0'
 *   load the length counter with the bits 'S S S 0'
 *   NOTE: though only three bits of the length counter load value are controllable, there is a fourth lower bit which is assumed 0 on start and controls the direction of playback, i.e. forwards or backwards within a phone.
 *   NOTE: though only two bits of the repeat counter reload value are controllable, there is a third bit which is loaded to 0 on phoneme start, and this hidden low-order bit of the counter itself is what controls whether the output is forced to silence in mirrored mode. the 'carry' from the highest bit of the 3 bit counter is what increments the address pointer for pointing to the next phoneme in mirrored mode


 *   shift register diagram:
 *   F E D C B A 9 8 7 6 5 4 3 2 1 0
 *   <new byte here>
 *               C C                 C = Current delta sample read point
 *                   O O             O = Old delta sample read point
 * I *OPTIMIZED OUT* the shift register by making use of the fact that the device reads each rom byte 4 times

 * state 5(PlayForward1):
 *   grab byte at (((phoneaddress<<8)+(oddphone*8))+(phonepos>>2)) -> PlayRegister high end, bits F to 8
 *   if Playmode is mirrored, set OldValHi and OldValLo to 1 and 0 respectively, otherwise leave them with whatever was in them before.
 *   Put OldValHi in bit 7 of PlayRegister
 *   Put OldValLo in bit 6 of PlayRegister
 *   Get new OldValHi from bit 9
 *   Get new OldValLo from bit 8
 *   feed current delta (bits 9 and 8) and olddelta (bits 7 and 6) to delta demodulator table, delta demodulator table applies a delta to the accumulator, accumulator goes to enable/disable latch which Silenceflag enables or disables (forces output to 0x8 on disable), then to DAC to output.
 *   next state: state 6(PlayForward2)

 * state 6(PlayForward2):
 *   grab byte at (((phoneaddress<<8)+oddphone)+(phonepos>>2)) -> PlayRegister bits D to 6.
 *   Put OldValHi in bit 7 of PlayRegister\____already done by above operation
 *   Put OldValLo in bit 6 of PlayRegister/
 *   Get new OldValHi from bit 9
 *   Get new OldValLo from bit 8
 *   feed current delta (bits 9 and 8) and olddelta (bits 7 and 6) to delta demodulator table, delta demodulator table applies a delta to the accumulator, accumulator goes to enable/disable latch which Silenceflag enables or disables (forces output to 0x8 on disable), then to DAC to output.
 *   next state: state 7(PlayForward3)

 * state 7(PlayForward3):
 *   grab byte at (((phoneaddress<<8)+oddphone)+(phonepos>>2)) -> PlayRegister bits B to 4.
 *   Put OldValHi in bit 7 of PlayRegister\____already done by above operation
 *   Put OldValLo in bit 6 of PlayRegister/
 *   Get new OldValHi from bit 9
 *   Get new OldValLo from bit 8
 *   feed current delta (bits 9 and 8) and olddelta (bits 7 and 6) to delta demodulator table, delta demodulator table applies a delta to the accumulator, accumulator goes to enable/disable latch which Silenceflag enables or disables (forces output to 0x8 on disable), then to DAC to output.
 *   next state: state 8(PlayForward4)

 * state 8(PlayForward4):
 *   grab byte at (((phoneaddress<<8)+oddphone)+(phonepos>>2)) -> PlayRegister bits 9 to 2.
 *   Put OldValHi in bit 7 of PlayRegister\____already done by above operation
 *   Put OldValLo in bit 6 of PlayRegister/
 *   Get new OldValHi from bit 9
 *   Get new OldValLo from bit 8
 *   feed current delta (bits 9 and 8) and olddelta (bits 7 and 6) to delta demodulator table, delta demodulator table applies a delta to the accumulator, accumulator goes to enable/disable latch which Silenceflag enables or disables (forces output to 0x8 on disable), then to DAC to output.
 *   Call function: increment address

 *   next state: depends on playparams:
 *     if we're in mirrored mode, next will be LoadAndPlayBackward1

 * state 9(LoadAndPlayBackward1)
 * state 10(PlayBackward2)
 * state 11(PlayBackward3)
 * state 12(PlayBackward4)
*/

/* increment address function:
 *   increment repeat counter
        if repeat counter produces a carry, do two things:
           1. if mirrored mode is ON, increment oddphone. if oddphone carries out (i.e. if it was 1), increment phoneaddress and zero oddphone
       2. increment lengthcounter. if lengthcounter carries out, we're done this phone.
 *   increment output counter
 *      if mirrored mode is on, output direction is
 *   if mirrored mode is OFF, increment oddphone. if not, don't touch it here. if oddphone was 1 before the increment, increment phoneaddress and set oddphone to 0
 *
 */

#include <math.h>
#include "sndintrf.h"
#include "s14001a.h"
#include "streams.h"

typedef struct
{
	int index;
	sound_stream * stream;

	UINT8 WordInput; // value on word input bus
	UINT8 LatchedWord; // value latched from input bus
	UINT16 SyllableAddress; // address read from word table
	UINT16 PhoneAddress; // starting/current phone address from syllable table
	UINT8 PlayParams; // playback parameters from syllable table
	UINT8 PhoneOffset; // offset within phone
	UINT8 LengthCounter; // 4-bit counter which holds the inverted length of the word in phones, leftshifted by 1
	UINT8 RepeatCounter; // 3-bit counter which holds the inverted number of repeats per phone, leftshifted by 1
	UINT8 OutputCounter; // 2-bit counter to determine forward/backward and output/silence state.
	UINT8 machineState; // chip state machine state
	UINT8 nextstate; // chip state machine's new state
	UINT8 laststate; // chip state machine's previous state, needed for mirror increment masking
	UINT8 resetState; // reset line state
	UINT8 oddeven; // odd versus even cycle toggle
	UINT8 GlobalSilenceState; // same as above but for silent syllables instead of silent portions of mirrored syllables
	UINT8 OldDelta; // 2-bit old delta value
	UINT8 DACOutput; // 4-bit DAC Accumulator/output
	UINT8 audioout; // filtered audio output
	UINT8 *SpeechRom; // array to hold rom contents, mame will not need this, will use a pointer
	UINT8 filtervals[8];
        UINT8 VSU1000_amp; // amplitude setting on VSU-1000 board
        UINT16 VSU1000_freq; // frequency setting on VSU-1000 board
        UINT16 VSU1000_counter; // counter for freq divider
} S14001AChip;

//#define DEBUGSTATE

#define SILENCE 0x77 // value output when silent

#define LASTSYLLABLE ((chip->PlayParams & 0x80)>>7)
#define MIRRORMODE ((chip->PlayParams & 0x40)>>6)
#define SILENCEFLAG ((chip->PlayParams & 0x20)>>5)
#define LENGTHCOUNT ((chip->PlayParams & 0x1C)>>1) // remember: its 4 bits and the bottom bit is always zero!
#define REPEATCOUNT ((chip->PlayParams<<1)&0x6) // remember: its 3 bits and the bottom bit is always zero!
#define LOCALSILENCESTATE ((chip->OutputCounter & 0x2) && (MIRRORMODE)) // 1 when silent output, 0 when DAC output.

static INT8 DeltaTable[4][4] =
{
	{ 0xCD, 0xCD, 0xEF, 0xEF, },
	{ 0xEF, 0xEF, 0x00, 0x00, },
	{ 0x00, 0x00, 0x11, 0x11, },
	{ 0x11, 0x11, 0x33, 0x33  },
};

static UINT8 audiofilter(S14001AChip *chip) /* rewrite me to better match the real filter! */
{
	UINT16 temp1, temp2 = 0;
	/* crappy averaging filter! */
	for (temp1 = 0; temp1 < 8; temp1++) { temp2 += chip->filtervals[temp1]; }
	temp2 >>= 3;
	return temp2;
}

static void shiftIntoFilter(S14001AChip *chip, UINT8 inputvalue)
{
	UINT8 temp1;
	for (temp1 = 7; temp1 > 0; temp1--)
	{
		chip->filtervals[temp1] = chip->filtervals[(temp1 - 1)];
	}
	chip->filtervals[0] = inputvalue;

}

static void PostPhoneme(S14001AChip *chip) /* figure out what the heck to do after playing a phoneme */
{
#ifdef DEBUGSTATE
	fprintf(stderr,"0: entered PostPhoneme\n");
#endif
	chip->RepeatCounter++; // increment the repeat counter
	chip->OutputCounter++; // increment the output counter
	if (MIRRORMODE) // if mirroring is enabled
	{
#ifdef DEBUGSTATE
		fprintf(stderr,"1: MIRRORMODE was on\n");
#endif
		if (chip->RepeatCounter == 0x8) // exceeded 3 bits?
		{
#ifdef DEBUGSTATE
			fprintf(stderr,"2: RepeatCounter was == 8\n");
#endif
			// reset repeat counter, increment length counter
			// but first check if lowest bit is set
			chip->RepeatCounter = REPEATCOUNT; // reload repeat counter with reload value
			if (chip->LengthCounter & 0x1) // if low bit is 1 (will carry after increment)
			{
#ifdef DEBUGSTATE
				fprintf(stderr,"3: LengthCounter's low bit was 1\n");
#endif
				chip->PhoneAddress+=8; // go to next phone in this syllable
			}
			chip->LengthCounter++;
			if (chip->LengthCounter == 0x10) // if Length counter carried out of 4 bits
			{
#ifdef DEBUGSTATE
				fprintf(stderr,"3: LengthCounter overflowed\n");
#endif
				chip->SyllableAddress += 2; // go to next syllable
				chip->nextstate = LASTSYLLABLE ? 13 : 3; // if we're on the last syllable, go to end state, otherwise go and load the next syllable.
			}
			else
			{
#ifdef DEBUGSTATE
				fprintf(stderr,"3: LengthCounter's low bit wasn't 1 and it didn't overflow\n");
#endif
				chip->PhoneOffset = (chip->OutputCounter&1) ? 7 : 0;
				chip->nextstate = (chip->OutputCounter&1) ? 9 : 5;
			}
		}
		else // repeatcounter did NOT carry out of 3 bits so leave length counter alone
		{
#ifdef DEBUGSTATE
			fprintf(stderr,"2: RepeatCounter is less than 8 (its actually %d)\n", chip->RepeatCounter);
#endif
			chip->PhoneOffset = (chip->OutputCounter&1) ? 7 : 0;
			chip->nextstate = (chip->OutputCounter&1) ? 9 : 5;
		}
	}
	else // if mirroring is NOT enabled
	{
#ifdef DEBUGSTATE
		fprintf(stderr,"1: MIRRORMODE was off\n");
#endif
		if (chip->RepeatCounter == 0x8) // exceeded 3 bits?
		{
#ifdef DEBUGSTATE
			fprintf(stderr,"2: RepeatCounter was == 8\n");
#endif
			// reset repeat counter, increment length counter
			chip->RepeatCounter = REPEATCOUNT; // reload repeat counter with reload value
			chip->LengthCounter++;
			if (chip->LengthCounter == 0x10) // if Length counter carried out of 4 bits
			{
#ifdef DEBUGSTATE
				fprintf(stderr,"3: LengthCounter overflowed\n");
#endif
				chip->SyllableAddress += 2; // go to next syllable
				chip->nextstate = LASTSYLLABLE ? 13 : 3; // if we're on the last syllable, go to end state, otherwise go and load the next syllable.
#ifdef DEBUGSTATE
				fprintf(stderr,"nextstate is now %d\n", chip->nextstate); // see line below, same reason.
#endif
				return; // need a return here so we don't hit the 'nextstate = 5' line below
			}
		}
		chip->PhoneAddress += 8; // regardless of counters, the phone address always increments in non-mirrored mode
		chip->PhoneOffset = 0;
		chip->nextstate = 5;
	}
#ifdef DEBUGSTATE
	fprintf(stderr,"nextstate is now %d\n", chip->nextstate);
#endif
}

void s14001a_clock(S14001AChip *chip) /* called once per clock */
{
	UINT8 CurDelta; // Current delta

	/* on even clocks, audio output is floating, /romen is low so rom data bus is driven, input is latched?
     * on odd clocks, audio output is driven, /romen is high, state machine 2 is clocked*/
	chip->oddeven = !(chip->oddeven); // invert the clock
	if (chip->oddeven == 0) // even clock
        {
		chip->audioout = audiofilter(chip); // function to handle output filtering by internal capacitance based on clock speed and such
		shiftIntoFilter(chip, chip->audioout); // shift over all the filter outputs and stick in audioout
	}
	else // odd clock
	{
		// fix dac output between samples. theoretically this might be unnecessary but it would require some messy logic in state 5 on the first sample load.
		if (chip->GlobalSilenceState || LOCALSILENCESTATE)
		{
			chip->DACOutput = SILENCE;
			chip->OldDelta = 2;
		}
		chip->audioout = (chip->GlobalSilenceState || LOCALSILENCESTATE) ? SILENCE : chip->DACOutput; // when either silence state is 1, output silence.
		shiftIntoFilter(chip, chip->audioout); // shift over all the filter outputs and stick in audioout
		switch(chip->machineState) // HUUUUUGE switch statement
		{
		case 0: // idle state
			chip->nextstate = 0;
			break;
		case 1: // read starting syllable high byte from word table
			chip->SyllableAddress = 0; // clear syllable address
			chip->SyllableAddress |= chip->SpeechRom[(chip->LatchedWord<<1)]<<4;
			chip->nextstate = chip->resetState ? 1 : 2;
			break;
		case 2: // read starting syllable low byte from word table
			chip->SyllableAddress |= chip->SpeechRom[(chip->LatchedWord<<1)+1]>>4;
			chip->nextstate = 3;
			break;
		case 3: // read starting phone address
			chip->PhoneAddress = chip->SpeechRom[chip->SyllableAddress]<<4;
			chip->nextstate = 4;
			break;
		case 4: // read playback parameters and prepare for play
			chip->PlayParams = chip->SpeechRom[chip->SyllableAddress+1];
			chip->GlobalSilenceState = SILENCEFLAG; // load phone silence flag
			chip->LengthCounter = LENGTHCOUNT; // load length counter
			chip->RepeatCounter = REPEATCOUNT; // load repeat counter
			chip->OutputCounter = 0; // clear output counter and disable mirrored phoneme silence indirectly via LOCALSILENCESTATE
			chip->PhoneOffset = 0; // set offset within phone to zero
			chip->OldDelta = 0x2; // set old delta to 2 <- is this right?
			chip->DACOutput = 0x88; // set DAC output to center/silence position (0x88)
			chip->nextstate = 5;
			break;
		case 5: // Play phone forward, shift = 0 (also load)
			CurDelta = (chip->SpeechRom[(chip->PhoneAddress)+chip->PhoneOffset]&0xc0)>>6; // grab current delta from high 2 bits of high nybble
			chip->DACOutput += DeltaTable[CurDelta][chip->OldDelta]; // send data to forward delta table and add result to accumulator
			chip->OldDelta = CurDelta; // Move current delta to old
			chip->nextstate = 6;
			break;
		case 6: // Play phone forward, shift = 2
	   		CurDelta = (chip->SpeechRom[(chip->PhoneAddress)+chip->PhoneOffset]&0x30)>>4; // grab current delta from low 2 bits of high nybble
			chip->DACOutput += DeltaTable[CurDelta][chip->OldDelta]; // send data to forward delta table and add result to accumulator
			chip->OldDelta = CurDelta; // Move current delta to old
			chip->nextstate = 7;
			break;
		case 7: // Play phone forward, shift = 4
			CurDelta = (chip->SpeechRom[(chip->PhoneAddress)+chip->PhoneOffset]&0xc)>>2; // grab current delta from high 2 bits of low nybble
			chip->DACOutput += DeltaTable[CurDelta][chip->OldDelta]; // send data to forward delta table and add result to accumulator
			chip->OldDelta = CurDelta; // Move current delta to old
			chip->nextstate = 8;
			break;
		case 8: // Play phone forward, shift = 6 (increment address if needed)
			CurDelta = chip->SpeechRom[(chip->PhoneAddress)+chip->PhoneOffset]&0x3; // grab current delta from low 2 bits of low nybble
			chip->DACOutput += DeltaTable[CurDelta][chip->OldDelta]; // send data to forward delta table and add result to accumulator
			chip->OldDelta = CurDelta; // Move current delta to old
			chip->PhoneOffset++; // increment phone offset
			if (chip->PhoneOffset == 0x8) // if we're now done this phone
			{
				/* call the PostPhoneme Function */
				PostPhoneme(chip);
			}
			else
			{
				chip->nextstate = 5;
			}
			break;
		case 9: // Play phone backward, shift = 6 (also load)
			CurDelta = (chip->SpeechRom[(chip->PhoneAddress)+chip->PhoneOffset]&0x3); // grab current delta from low 2 bits of low nybble
			if (chip->laststate != 8) // ignore first (bogus) dac change in mirrored backwards mode. observations and the patent show this.
			{
				chip->DACOutput -= DeltaTable[chip->OldDelta][CurDelta]; // send data to forward delta table and subtract result from accumulator
			}
			chip->OldDelta = CurDelta; // Move current delta to old
			chip->nextstate = 10;
			break;
		case 10: // Play phone backward, shift = 4
			CurDelta = (chip->SpeechRom[(chip->PhoneAddress)+chip->PhoneOffset]&0xc)>>2; // grab current delta from high 2 bits of low nybble
			chip->DACOutput -= DeltaTable[chip->OldDelta][CurDelta]; // send data to forward delta table and subtract result from accumulator
			chip->OldDelta = CurDelta; // Move current delta to old
			chip->nextstate = 11;
			break;
		case 11: // Play phone backward, shift = 2
			CurDelta = (chip->SpeechRom[(chip->PhoneAddress)+chip->PhoneOffset]&0x30)>>4; // grab current delta from low 2 bits of high nybble
			chip->DACOutput -= DeltaTable[chip->OldDelta][CurDelta]; // send data to forward delta table and subtract result from accumulator
			chip->OldDelta = CurDelta; // Move current delta to old
			chip->nextstate = 12;
			break;
		case 12: // Play phone backward, shift = 0 (increment address if needed)
			CurDelta = (chip->SpeechRom[(chip->PhoneAddress)+chip->PhoneOffset]&0xc0)>>6; // grab current delta from high 2 bits of high nybble
			chip->DACOutput -= DeltaTable[chip->OldDelta][CurDelta]; // send data to forward delta table and subtract result from accumulator
			chip->OldDelta = CurDelta; // Move current delta to old
			chip->PhoneOffset--; // decrement phone offset
			if (chip->PhoneOffset == 0xFF) // if we're now done this phone
			{
				/* call the PostPhoneme() function */
				PostPhoneme(chip);
			}
			else
			{
				chip->nextstate = 9;
			}
			break;
		case 13: // For those pedantic among us, consume an extra two clocks like the real chip does.
			chip->nextstate = 0;
			break;
		}
#ifdef DEBUGSTATE
		fprintf(stderr, "Machine state is now %d, was %d, PhoneOffset is %d\n", chip->nextstate, chip->machineState, chip->PhoneOffset);
#endif
		chip->laststate = chip->machineState;
		chip->machineState = chip->nextstate;
	}
}

/**************************************************************************
   MAME glue code
 **************************************************************************/

static void s14001a_pcm_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int length)
{
	INT32 mix[48000];
	INT32 *mixp;
	S14001AChip *chip = param;
	int i;

	memset(mix, 0, sizeof(mix));

	mixp = &mix[0];
	for (i = 0; i < length; i++)
	{
		if (--chip->VSU1000_counter==0)
		  {
		  s14001a_clock(chip);
		  chip->VSU1000_counter = chip->VSU1000_freq;
		  }
		outputs[0][i] = ((((INT16)chip->audioout)-128)<<6)*chip->VSU1000_amp;
	}
}

static void *s14001a_start(int sndindex, int clock, const void *config)
{
	const struct S14001A_interface *intf;
	S14001AChip *chip;
	int i;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));
	chip->index = sndindex;

	chip->GlobalSilenceState = 1;
	chip->OldDelta = 0x02;
	chip->DACOutput = SILENCE;
	chip->VSU1000_amp = 0; /* reset by /reset line */
	chip->VSU1000_freq = 1; /* base-1; reset by /reset line */
	chip->VSU1000_counter = 1; /* base-1; not reset by /reset line but this is the best place to reset it */

	for (i = 0; i < 8; i++)
	{
		chip->filtervals[i] = SILENCE;
	}

	intf = config;

	chip->SpeechRom = memory_region(intf->region);

	chip->stream = stream_create(0, 1, clock, chip, s14001a_pcm_update);

	return chip;
}

static void s14001a_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}

int S14001A_bsy_0_r(void)
{
	S14001AChip *chip = sndti_token(SOUND_S14001A, 0);
        stream_update(chip->stream);
#ifdef DEBUGSTATE
	fprintf(stderr,"busy state checked: %d\n",(chip->machineState != 0) );
#endif
	return (chip->machineState != 0);
}

void S14001A_reg_0_w(int data)
{
	S14001AChip *chip = sndti_token(SOUND_S14001A, 0);
	stream_update(chip->stream);
	chip->WordInput = data;
}

void S14001A_rst_0_w(int data)
{
	S14001AChip *chip = sndti_token(SOUND_S14001A, 0);
	stream_update(chip->stream);
        chip->LatchedWord = chip->WordInput;
	chip->resetState = (data==1);
	chip->machineState = chip->resetState ? 1 : chip->machineState;
}

void S14001A_set_rate(int newrate)
{
  	S14001AChip *chip = sndti_token(SOUND_S14001A, 0);
        stream_update(chip->stream);
        chip->VSU1000_freq = newrate;
}

void S14001A_set_volume(int volume)
{
	S14001AChip *chip = sndti_token(SOUND_S14001A, 0);
        stream_update(chip->stream);
        chip->VSU1000_amp = volume;
}

void s14001a_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:					info->set_info = s14001a_set_info;	break;
		case SNDINFO_PTR_START:						info->start = s14001a_start;		break;
		case SNDINFO_PTR_STOP:						/* Nothing */				break;
		case SNDINFO_PTR_RESET:						/* Nothing */				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:						info->s = "S14001A";			break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "TSI S14001A";		break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.11";			break;
		case SNDINFO_STR_CORE_FILE:					info->s = __FILE__;			break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2007 Jonathan Gevaryahu"; break;
	}
}

