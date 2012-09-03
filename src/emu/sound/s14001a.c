/*

 TSI S14001A emulator v1.32
 By Jonathan Gevaryahu ("Lord Nightmare") with help from Kevin Horton ("kevtris")
 MAME conversion and integration by R. Belmont
 Clock Frequency control updated by Zsolt Vasvari
 Other fixes by AtariAce

 Copyright Jonathan Gevaryahu.

 Version history:
 0.8 initial version - LN
 0.9 MAME conversion, glue code added - R. Belmont
 1.0 partly fixed stream update - LN (0.111u4)
 1.01 fixed clipping problem - LN (0.111u5)
 1.1 add VSU-1000 features, fully fixed stream update by fixing word latching - LN (0.111u6)
 1.11 fix signedness of output, pre-multiply, fixes clicking on VSU-1000 volume change - LN (0.111u7)
 1.20 supports setting the clock freq directly - reset is done by external hardware,
      the chip has no reset line ZV (0.122)
 1.30 move main dac to 4 bits only with no extension (4->16 bit range extension is now done by output).
 Added a somewhat better, but still not perfect, filtering system - LN
 1.31 fix a minor bug with the dac range. wolfpack clips again, and I'm almost sure its an encoding error on the original speech - LN (0.125u9)
 1.31a Add chip pinout and other notes - LN (0.128u4)
 1.31b slight update to notes to clarify input bus stuff, mostly finish the state map in the comments - LN
 1.31c remove usage of deprecat lib - AtariAce (0.128u5)
 1.32 fix the squealing noise using a define; it isn't accurate to the chip exactly, but there are other issues which need to be fixed too. see TODO. - LN (0.136u2)

 TODO:
 * increase accuracy of internal S14001A 'filter' for both driven and undriven cycles (its not terribly inaccurate for undriven cycles, but the dc sliding of driven cycles is not emulated)
 * add option for and attach Frank P.'s emulation of the Analog external filter from the vsu-1000 using the discrete core. (with the direction of independent sound core and analog stuff, this should actually be attached in the main berzerk/frenzy driver and not here)
 * fix the local and global silence stuff to not force the dac to a specific level, but cease doing deltas (i.e. force all deltas to 0) after the last sample; this should fix the clipping in wolfpack and in the fidelity games in mess.
*/

/* Chip Pinout:
The original datasheet (which is lost as far as I know) clearly called the
s14001a chip the 'CRC chip', or 'Custom Rom Controller', as it appears with
this name on the Stern and Canon schematics, as well as on some TSI speech
print advertisements.
Labels are not based on the labels used by the Atari wolf pack and Stern
schematics, as these are inconsistent. Atari calls the word select/speech address
input pins SAx while Stern calls them Cx. Also Atari and Canon both have the bit
ordering for the word select/speech address bus backwards, which may indicate it
was so on the original datasheet. Stern has it correct, and I've used their Cx
labeling.

                      ______    ______
                    _|o     \__/      |_
            +5V -- |_|1             40|_| -> /BUSY*
                    _|                |_
          ?TEST ?? |_|2             39|_| <- ROM D7
                    _|                |_
 XTAL CLOCK/CKC -> |_|3             38|_| -> ROM A11
                    _|                |_
  ROM CLOCK/CKR <- |_|4             37|_| <- ROM D6
                    _|                |_
     ?DAC TEST? ?? |_|5             36|_| -> ROM A10
                    _|                |_
     ?DAC TEST? ?? |_|6             35|_| -> ROM A9
                    _|                |_
     ?DAC TEST? ?? |_|7             34|_| <- ROM D5
                    _|                |_
     ?DAC TEST? ?? |_|8             33|_| -> ROM A8
                    _|                |_
        ROM /EN <- |_|9             32|_| <- ROM D4
                    _|       S        |_
          START -> |_|10 7   1   T  31|_| -> ROM A7
                    _|   7   4   S    |_
      AUDIO OUT <- |_|11 3   0   I  30|_| <- ROM D3
                    _|   7   0        |_
         ROM A0 <- |_|12     1      29|_| -> ROM A6
                    _|       A        |_
SPCH ADR BUS C0 -> |_|13            28|_| <- SPCH ADR BUS C5
                    _|                |_
         ROM A1 <- |_|14            27|_| <- ROM D2
                    _|                |_
SPCH ADR BUS C1 -> |_|15            26|_| <- SPCH ADR BUS C4
                    _|                |_
         ROM A2 <- |_|16            25|_| <- ROM D1
                    _|                |_
SPCH ADR BUS C2 -> |_|17            24|_| <- SPCH ADR BUS C3
                    _|                |_
         ROM A3 <- |_|18            23|_| <- ROM D0
                    _|                |_
         ROM A4 <- |_|19            22|_| -> ROM A5
                    _|                |_
            GND -- |_|20            21|_| -- -10V
                     |________________|

*Note from Kevin Horton when testing the hookup of the S14001A: the /BUSY line
is not a standard voltage line: when it is in its HIGH state (i.e. not busy) it
puts out a voltage of -10 volts, so it needs to be dropped back to a sane
voltage level before it can be passed to any sort of modern IC. The address
lines for the speech rom (A0-A11) do not have this problem, they output at a
TTL/CMOS compatible voltage. The AUDIO OUT pin also outputs a voltage below GND,
and the TEST pins may do so too.

START is pulled high when a word is to be said and the word number is on the
word select/speech address input lines. The Canon 'Canola' uses a separate 'rom
strobe' signal independent of the chip to either enable or clock the speech rom.
Its likely that they did this to be able to force the speech chip to stop talking,
which is normally impossible. The later 'version 3' TSI speech board as featured in
an advertisement in the John Cater book probably also has this feature, in addition
to external speech rom banking.

Because it requires -10V to operate, the chip manufacturing process must be PMOS.

/-----------\
> Operation <
\-----------/
Put the 6-bit address of the word to be said onto the C0-C5 word select/speech
address bus lines. Next, clock the START line low-high-low. As long as the START
line is held high, the first address byte of the first word will be read repeatedly
every clock, with the rom enable line enabled constantly (i.e. it doesn't toggle on
and off as it normally does during speech). Once START has gone low-high-low, the
/BUSY line will go low until 3 clocks after the chip is done speaking.
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
 *   if offset < 8, increment offset within 8-byte phone
 *   if offset = 8: (see PostPhoneme code to understand how this part works, its a bit complicated)

 *   next state: depends on playparams:
 *     if we're in mirrored mode, next will be LoadAndPlayBackward1
 *     if we're in nonmirrored mode, next will be PlayForward1

 * state 9(LoadAndPlayBackward1)
 *   grab byte at (((phoneaddress<<8)+(oddphone*8))+(phonepos>>2)) -> PlayRegister high end, bits F to 8 <- check code on this, I think its backwards here but its correct in the code
 *   see code for this, its basically the same as state 8 but with the byte grab mentioned above, and the values fed to the delta demod table are switched
 * state 10(PlayBackward2)
 *   see code for this, its basically the same as state 7 but the values fed to the delta demod table are switched
 * state 11(PlayBackward3)
 *   see code for this, its basically the same as state 6 but the values fed to the delta demod table are switched
 * state 12(PlayBackward4)
 *   see code for this, its basically the same as state 5 but with no byte grab, and the values fed to the delta demod table are switched, and a bit below similar to state 5
 *   if offset > -1, decrement offset within 8-byte phone
 *   if offset = -1: (see PostPhoneme code to understand how this part works, its a bit complicated)
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

#undef ACCURATE_SQUEAL

#include "emu.h"
#include "s14001a.h"

typedef struct
{
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
	INT16 filtervals[8];
	UINT8 VSU1000_amp; // amplitude setting on VSU-1000 board
} S14001AChip;

INLINE S14001AChip *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == S14001A);
	return (S14001AChip *)downcast<s14001a_device *>(device)->token();
}


//#define DEBUGSTATE

#define SILENCE 0x7 // value output when silent
#define ALTFLAG 0xFF // value to tell renderer that this frame's output is the average of the 8 prior frames and not directly used.

#define LASTSYLLABLE ((chip->PlayParams & 0x80)>>7)
#define MIRRORMODE ((chip->PlayParams & 0x40)>>6)
#define SILENCEFLAG ((chip->PlayParams & 0x20)>>5)
#define LENGTHCOUNT ((chip->PlayParams & 0x1C)>>1) // remember: its 4 bits and the bottom bit is always zero!
#define REPEATCOUNT ((chip->PlayParams<<1)&0x6) // remember: its 3 bits and the bottom bit is always zero!
#define LOCALSILENCESTATE ((chip->OutputCounter & 0x2) && (MIRRORMODE)) // 1 when silent output, 0 when DAC output.

static const INT8 DeltaTable[4][4] =
{
	{ -3, -3, -1, -1, },
	{ -1, -1,  0,  0, },
	{  0,  0,  1,  1, },
	{  1,  1,  3,  3  },
};

#ifdef ACCURATE_SQUEAL
static INT16 audiofilter(S14001AChip *chip) /* rewrite me to better match the real filter! */
{
	UINT8 temp1;
        INT16 temp2 = 0;
	/* mean averaging filter! 1/n exponential *would* be somewhat better, but I'm lazy... */
	for (temp1 = 0; temp1 < 8; temp1++) { temp2 += chip->filtervals[temp1]; }
	temp2 >>= 3;
	return temp2;
}

static void shiftIntoFilter(S14001AChip *chip, INT16 inputvalue)
{
	UINT8 temp1;
	for (temp1 = 7; temp1 > 0; temp1--)
	{
		chip->filtervals[temp1] = chip->filtervals[(temp1 - 1)];
	}
	chip->filtervals[0] = inputvalue;
}
#endif

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

static void s14001a_clock(S14001AChip *chip) /* called once per clock */
{
	UINT8 CurDelta; // Current delta

	/* on even clocks, audio output is floating, /romen is low so rom data bus is driven
         * on odd clocks, audio output is driven, /romen is high, state machine 2 is clocked
         */
	chip->oddeven = !(chip->oddeven); // invert the clock
	if (chip->oddeven == 0) // even clock
	{
#ifdef ACCURATE_SQUEAL
		chip->audioout = ALTFLAG; // flag to the renderer that this output should be the average of the last 8
#endif
		// DIGITAL INPUT *MIGHT* occur on the test pins occurs on this cycle?
	}
	else // odd clock
	{
		// fix dac output between samples. theoretically this might be unnecessary but it would require some messy logic in state 5 on the first sample load.
		// Note: this behavior is NOT accurate, and needs to be fixed. see TODO.
		if (chip->GlobalSilenceState || LOCALSILENCESTATE)
		{
			chip->DACOutput = SILENCE;
			chip->OldDelta = 2;
		}
		chip->audioout = (chip->GlobalSilenceState || LOCALSILENCESTATE) ? SILENCE : chip->DACOutput; // when either silence state is 1, output silence.
		// DIGITAL OUTPUT *might* be driven onto the test pins on this cycle?
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
			chip->DACOutput = SILENCE ; // set DAC output to center/silence position
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

	        /* the dac is 4 bits wide. if a delta step forced it outside of 4 bits, mask it back over here */
	        chip->DACOutput &= 0xF;
	}
}

/**************************************************************************
   MAME glue code
 **************************************************************************/

static STREAM_UPDATE( s14001a_pcm_update )
{
	S14001AChip *chip = (S14001AChip *)param;
	int i;

	for (i = 0; i < samples; i++)
	{
		s14001a_clock(chip);
#ifdef ACCURATE_SQUEAL
		if (chip->audioout == ALTFLAG) // input from test pins -> output
		{
			shiftIntoFilter(chip, audiofilter(chip)); // shift over the previous outputs and stick in audioout.
			outputs[0][i] = audiofilter(chip)*chip->VSU1000_amp;
		}
		else // normal, dac-driven output
		{
			shiftIntoFilter(chip, ((((INT16)chip->audioout)-8)<<9)); // shift over the previous outputs and stick in audioout 4 times. note <<9 instead of <<10, to prevent clipping, and to simulate that the filtered output normally has a somewhat lower amplitude than the driven one.
#endif
			outputs[0][i] = ((((INT16)chip->audioout)-8)<<10)*chip->VSU1000_amp;
#ifdef ACCURATE_SQUEAL
		}
#endif
	}
}

static DEVICE_START( s14001a )
{
	S14001AChip *chip = get_safe_token(device);
	int i;

	chip->GlobalSilenceState = 1;
	chip->OldDelta = 0x02;
	chip->DACOutput = SILENCE;

	for (i = 0; i < 8; i++)
	{
		chip->filtervals[i] = SILENCE;
	}

	chip->SpeechRom = *device->region();

	chip->stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock() ? device->clock() : device->machine().sample_rate(), chip, s14001a_pcm_update);
}

int s14001a_bsy_r(device_t *device)
{
	S14001AChip *chip = get_safe_token(device);
	chip->stream->update();
#ifdef DEBUGSTATE
	fprintf(stderr,"busy state checked: %d\n",(chip->machineState != 0) );
#endif
	return (chip->machineState != 0);
}

void s14001a_reg_w(device_t *device, int data)
{
	S14001AChip *chip = get_safe_token(device);
	chip->stream->update();
	chip->WordInput = data;
}

void s14001a_rst_w(device_t *device, int data)
{
	S14001AChip *chip = get_safe_token(device);
	chip->stream->update();
	chip->LatchedWord = chip->WordInput;
	chip->resetState = (data==1);
	chip->machineState = chip->resetState ? 1 : chip->machineState;
}

void s14001a_set_clock(device_t *device, int clock)
{
	S14001AChip *chip = get_safe_token(device);
	chip->stream->set_sample_rate(clock);
}

void s14001a_set_volume(device_t *device, int volume)
{
	S14001AChip *chip = get_safe_token(device);
	chip->stream->update();
	chip->VSU1000_amp = volume;
}

const device_type S14001A = &device_creator<s14001a_device>;

s14001a_device::s14001a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, S14001A, "S14001A", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(S14001AChip));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void s14001a_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s14001a_device::device_start()
{
	DEVICE_START_NAME( s14001a )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void s14001a_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


