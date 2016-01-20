// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu,R. Belmont,Zsolt Vasvari
/*

 TSI S14001A emulator v1.32
 By Jonathan Gevaryahu ("Lord Nightmare") with help from Kevin Horton ("kevtris")
 MAME conversion and integration by R. Belmont
 Clock Frequency control updated by Zsolt Vasvari
 Other fixes by AtariAce

 Copyright (C) 2006-2013 Jonathan Gevaryahu aka Lord Nightmare

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
  DIGITAL OUT 0 <- |_|5             36|_| -> ROM A10
                    _|                |_
  DIGITAL OUT 1 <- |_|6             35|_| -> ROM A9
                    _|                |_
  DIGITAL OUT 2 <- |_|7             34|_| <- ROM D5
                    _|                |_
  DIGITAL OUT 3 <- |_|8             33|_| -> ROM A8
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

The Digital out pins supply a copy of the 4-bit waveform which also goes to the
internal DAC. They are only valid every other clock cycle. It is possible that
on 'invalid' cycles they act as a 4 bit input to drive the dac.

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


//#define DEBUGSTATE

#define SILENCE 0x7 // value output when silent
#define ALTFLAG 0xFF // value to tell renderer that this frame's output is the average of the 8 prior frames and not directly used.

#define LASTSYLLABLE ((m_PlayParams & 0x80)>>7)
#define MIRRORMODE ((m_PlayParams & 0x40)>>6)
#define SILENCEFLAG ((m_PlayParams & 0x20)>>5)
#define LENGTHCOUNT ((m_PlayParams & 0x1C)>>1) // remember: its 4 bits and the bottom bit is always zero!
#define REPEATCOUNT ((m_PlayParams<<1)&0x6) // remember: its 3 bits and the bottom bit is always zero!
#define LOCALSILENCESTATE ((m_OutputCounter & 0x2) && (MIRRORMODE)) // 1 when silent output, 0 when DAC output.

static const INT8 DeltaTable[4][4] =
{
	{ -3, -3, -1, -1, },
	{ -1, -1,  0,  0, },
	{  0,  0,  1,  1, },
	{  1,  1,  3,  3  },
};

#ifdef ACCURATE_SQUEAL
INT16 s14001a_device::audiofilter() /* rewrite me to better match the real filter! */
{
	UINT8 temp1;
		INT16 temp2 = 0;
	/* mean averaging filter! 1/n exponential *would* be somewhat better, but I'm lazy... */
	for (temp1 = 0; temp1 < 8; temp1++) { temp2 += m_filtervals[temp1]; }
	temp2 >>= 3;
	return temp2;
}

void s14001a_device::shiftIntoFilter(INT16 inputvalue)
{
	UINT8 temp1;
	for (temp1 = 7; temp1 > 0; temp1--)
	{
		m_filtervals[temp1] = m_filtervals[(temp1 - 1)];
	}
	m_filtervals[0] = inputvalue;
}
#endif

void s14001a_device::PostPhoneme() /* figure out what the heck to do after playing a phoneme */
{
#ifdef DEBUGSTATE
	fprintf(stderr,"0: entered PostPhoneme\n");
#endif
	m_RepeatCounter++; // increment the repeat counter
	m_OutputCounter++; // increment the output counter
	if (MIRRORMODE) // if mirroring is enabled
	{
#ifdef DEBUGSTATE
		fprintf(stderr,"1: MIRRORMODE was on\n");
#endif
		if (m_RepeatCounter == 0x8) // exceeded 3 bits?
		{
#ifdef DEBUGSTATE
			fprintf(stderr,"2: RepeatCounter was == 8\n");
#endif
			// reset repeat counter, increment length counter
			// but first check if lowest bit is set
			m_RepeatCounter = REPEATCOUNT; // reload repeat counter with reload value
			if (m_LengthCounter & 0x1) // if low bit is 1 (will carry after increment)
			{
#ifdef DEBUGSTATE
				fprintf(stderr,"3: LengthCounter's low bit was 1\n");
#endif
				m_PhoneAddress+=8; // go to next phone in this syllable
			}
			m_LengthCounter++;
			if (m_LengthCounter == 0x10) // if Length counter carried out of 4 bits
			{
#ifdef DEBUGSTATE
				fprintf(stderr,"3: LengthCounter overflowed\n");
#endif
				m_SyllableAddress += 2; // go to next syllable
				m_nextstate = LASTSYLLABLE ? 13 : 3; // if we're on the last syllable, go to end state, otherwise go and load the next syllable.
			}
			else
			{
#ifdef DEBUGSTATE
				fprintf(stderr,"3: LengthCounter's low bit wasn't 1 and it didn't overflow\n");
#endif
				m_PhoneOffset = (m_OutputCounter&1) ? 7 : 0;
				m_nextstate = (m_OutputCounter&1) ? 9 : 5;
			}
		}
		else // repeatcounter did NOT carry out of 3 bits so leave length counter alone
		{
#ifdef DEBUGSTATE
			fprintf(stderr,"2: RepeatCounter is less than 8 (its actually %d)\n", m_RepeatCounter);
#endif
			m_PhoneOffset = (m_OutputCounter&1) ? 7 : 0;
			m_nextstate = (m_OutputCounter&1) ? 9 : 5;
		}
	}
	else // if mirroring is NOT enabled
	{
#ifdef DEBUGSTATE
		fprintf(stderr,"1: MIRRORMODE was off\n");
#endif
		if (m_RepeatCounter == 0x8) // exceeded 3 bits?
		{
#ifdef DEBUGSTATE
			fprintf(stderr,"2: RepeatCounter was == 8\n");
#endif
			// reset repeat counter, increment length counter
			m_RepeatCounter = REPEATCOUNT; // reload repeat counter with reload value
			m_LengthCounter++;
			if (m_LengthCounter == 0x10) // if Length counter carried out of 4 bits
			{
#ifdef DEBUGSTATE
				fprintf(stderr,"3: LengthCounter overflowed\n");
#endif
				m_SyllableAddress += 2; // go to next syllable
				m_nextstate = LASTSYLLABLE ? 13 : 3; // if we're on the last syllable, go to end state, otherwise go and load the next syllable.
#ifdef DEBUGSTATE
				fprintf(stderr,"nextstate is now %d\n", m_nextstate); // see line below, same reason.
#endif
				return; // need a return here so we don't hit the 'nextstate = 5' line below
			}
		}
		m_PhoneAddress += 8; // regardless of counters, the phone address always increments in non-mirrored mode
		m_PhoneOffset = 0;
		m_nextstate = 5;
	}
#ifdef DEBUGSTATE
	fprintf(stderr,"nextstate is now %d\n", m_nextstate);
#endif
}

void s14001a_device::s14001a_clock() /* called once per clock */
{
	UINT8 CurDelta; // Current delta

	/* on even clocks, audio output is floating, /romen is low so rom data bus is driven
	     * on odd clocks, audio output is driven, /romen is high, state machine 2 is clocked
	     */
	m_oddeven = !(m_oddeven); // invert the clock
	if (m_oddeven == 0) // even clock
	{
#ifdef ACCURATE_SQUEAL
		m_audioout = ALTFLAG; // flag to the renderer that this output should be the average of the last 8
#endif
		// DIGITAL INPUT *MIGHT* occur on the test pins occurs on this cycle?
	}
	else // odd clock
	{
		// fix dac output between samples. theoretically this might be unnecessary but it would require some messy logic in state 5 on the first sample load.
		// Note: this behavior is NOT accurate, and needs to be fixed. see TODO.
		if (m_GlobalSilenceState || LOCALSILENCESTATE)
		{
			m_DACOutput = SILENCE;
			m_OldDelta = 2;
		}
		m_audioout = (m_GlobalSilenceState || LOCALSILENCESTATE) ? SILENCE : m_DACOutput; // when either silence state is 1, output silence.
		// DIGITAL OUTPUT *might* be driven onto the test pins on this cycle?
		switch(m_machineState)
		{
		case 0: // idle state
			m_nextstate = 0;
			break;
		case 1: // read starting syllable high byte from word table
			m_SyllableAddress = 0; // clear syllable address
			m_SyllableAddress |= readmem((m_LatchedWord<<1))<<4;
			m_nextstate = m_resetState ? 1 : 2;
			break;
		case 2: // read starting syllable low byte from word table
			m_SyllableAddress |= readmem((m_LatchedWord<<1)+1)>>4;
			m_nextstate = 3;
			break;
		case 3: // read starting phone address
			m_PhoneAddress = readmem(m_SyllableAddress)<<4;
			m_nextstate = 4;
			break;
		case 4: // read playback parameters and prepare for play
			m_PlayParams = readmem(m_SyllableAddress+1);
			m_GlobalSilenceState = SILENCEFLAG; // load phone silence flag
			m_LengthCounter = LENGTHCOUNT; // load length counter
			m_RepeatCounter = REPEATCOUNT; // load repeat counter
			m_OutputCounter = 0; // clear output counter and disable mirrored phoneme silence indirectly via LOCALSILENCESTATE
			m_PhoneOffset = 0; // set offset within phone to zero
			m_OldDelta = 0x2; // set old delta to 2 <- is this right?
			m_DACOutput = SILENCE ; // set DAC output to center/silence position
			m_nextstate = 5;
			break;
		case 5: // Play phone forward, shift = 0 (also load)
			CurDelta = (readmem((m_PhoneAddress)+m_PhoneOffset)&0xc0)>>6; // grab current delta from high 2 bits of high nybble
			m_DACOutput += DeltaTable[CurDelta][m_OldDelta]; // send data to forward delta table and add result to accumulator
			m_OldDelta = CurDelta; // Move current delta to old
			m_nextstate = 6;
			break;
		case 6: // Play phone forward, shift = 2
			CurDelta = (readmem((m_PhoneAddress)+m_PhoneOffset)&0x30)>>4; // grab current delta from low 2 bits of high nybble
			m_DACOutput += DeltaTable[CurDelta][m_OldDelta]; // send data to forward delta table and add result to accumulator
			m_OldDelta = CurDelta; // Move current delta to old
			m_nextstate = 7;
			break;
		case 7: // Play phone forward, shift = 4
			CurDelta = (readmem((m_PhoneAddress)+m_PhoneOffset)&0xc)>>2; // grab current delta from high 2 bits of low nybble
			m_DACOutput += DeltaTable[CurDelta][m_OldDelta]; // send data to forward delta table and add result to accumulator
			m_OldDelta = CurDelta; // Move current delta to old
			m_nextstate = 8;
			break;
		case 8: // Play phone forward, shift = 6 (increment address if needed)
			CurDelta = readmem((m_PhoneAddress)+m_PhoneOffset)&0x3; // grab current delta from low 2 bits of low nybble
			m_DACOutput += DeltaTable[CurDelta][m_OldDelta]; // send data to forward delta table and add result to accumulator
			m_OldDelta = CurDelta; // Move current delta to old
			m_PhoneOffset++; // increment phone offset
			if (m_PhoneOffset == 0x8) // if we're now done this phone
			{
				/* call the PostPhoneme Function */
				PostPhoneme();
			}
			else
			{
				m_nextstate = 5;
			}
			break;
		case 9: // Play phone backward, shift = 6 (also load)
			CurDelta = (readmem((m_PhoneAddress)+m_PhoneOffset)&0x3); // grab current delta from low 2 bits of low nybble
			if (m_laststate != 8) // ignore first (bogus) dac change in mirrored backwards mode. observations and the patent show this.
			{
				m_DACOutput -= DeltaTable[m_OldDelta][CurDelta]; // send data to forward delta table and subtract result from accumulator
			}
			m_OldDelta = CurDelta; // Move current delta to old
			m_nextstate = 10;
			break;
		case 10: // Play phone backward, shift = 4
			CurDelta = (readmem((m_PhoneAddress)+m_PhoneOffset)&0xc)>>2; // grab current delta from high 2 bits of low nybble
			m_DACOutput -= DeltaTable[m_OldDelta][CurDelta]; // send data to forward delta table and subtract result from accumulator
			m_OldDelta = CurDelta; // Move current delta to old
			m_nextstate = 11;
			break;
		case 11: // Play phone backward, shift = 2
			CurDelta = (readmem((m_PhoneAddress)+m_PhoneOffset)&0x30)>>4; // grab current delta from low 2 bits of high nybble
			m_DACOutput -= DeltaTable[m_OldDelta][CurDelta]; // send data to forward delta table and subtract result from accumulator
			m_OldDelta = CurDelta; // Move current delta to old
			m_nextstate = 12;
			break;
		case 12: // Play phone backward, shift = 0 (increment address if needed)
			CurDelta = (readmem((m_PhoneAddress)+m_PhoneOffset)&0xc0)>>6; // grab current delta from high 2 bits of high nybble
			m_DACOutput -= DeltaTable[m_OldDelta][CurDelta]; // send data to forward delta table and subtract result from accumulator
			m_OldDelta = CurDelta; // Move current delta to old
			m_PhoneOffset--; // decrement phone offset
			if (m_PhoneOffset == 0xFF) // if we're now done this phone
			{
				/* call the PostPhoneme() function */
				PostPhoneme();
			}
			else
			{
				m_nextstate = 9;
			}
			break;
		case 13: // For those pedantic among us, consume an extra two clocks like the real chip does.
			m_nextstate = 0;
			break;
		}

		/* the dac is 4 bits wide. if a delta step forced it outside of 4 bits, mask it back over here */
		m_DACOutput &= 0xF;

#ifdef DEBUGSTATE
		fprintf(stderr, "Machine state is now %d, was %d, PhoneOffset is %d\n", m_nextstate, m_machineState, m_PhoneOffset);
#endif
		m_laststate = m_machineState;
		m_machineState = m_nextstate;

		if (bool(m_laststate) != bool(m_machineState) && !m_bsy_handler.isnull())
			m_bsy_handler((m_machineState) ? 1 : 0);
	}
}

UINT8 s14001a_device::readmem(UINT16 offset)
{
	offset &= 0xfff; // 11-bit internal
	return ((m_ext_read_handler.isnull()) ? m_SpeechRom[offset & (m_SpeechRom.bytes() - 1)] : m_ext_read_handler(offset));
}


/**************************************************************************
   MAME glue code
 **************************************************************************/

void s14001a_device::force_update()
{
	m_stream->update();
}

int s14001a_device::bsy_r()
{
	m_stream->update();
#ifdef DEBUGSTATE
	fprintf(stderr,"busy state checked: %d\n",(m_machineState != 0) );
#endif
	return (m_machineState != 0);
}

void s14001a_device::reg_w(int data)
{
	m_stream->update();
	m_WordInput = data;
}

void s14001a_device::rst_w(int data)
{
	m_stream->update();
	m_LatchedWord = m_WordInput;
	m_resetState = (data==1);
	m_machineState = m_resetState ? 1 : m_machineState;
}

void s14001a_device::set_clock(int clock)
{
	m_stream->set_sample_rate(clock);
}

void s14001a_device::set_volume(int volume)
{
	m_stream->update();
	m_VSU1000_amp = volume;
}

const device_type S14001A = &device_creator<s14001a_device>;

s14001a_device::s14001a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, S14001A, "S14001A", tag, owner, clock, "s14001a", __FILE__),
		device_sound_interface(mconfig, *this),
		m_SpeechRom(*this, DEVICE_SELF),
		m_stream(nullptr),
		m_bsy_handler(*this),
		m_ext_read_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s14001a_device::device_start()
{
	m_GlobalSilenceState = 1;
	m_OldDelta = 0x02;
	m_DACOutput = SILENCE;
	m_VSU1000_amp = 15;

	for (int i = 0; i < 8; i++)
	{
		m_filtervals[i] = SILENCE;
	}

	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock() ? clock() : machine().sample_rate());

	// resolve callbacks
	m_ext_read_handler.resolve();
	m_bsy_handler.resolve();

	save_item(NAME(m_WordInput));
	save_item(NAME(m_LatchedWord));
	save_item(NAME(m_SyllableAddress));
	save_item(NAME(m_PhoneAddress));
	save_item(NAME(m_PlayParams));
	save_item(NAME(m_PhoneOffset));
	save_item(NAME(m_LengthCounter));
	save_item(NAME(m_RepeatCounter));
	save_item(NAME(m_OutputCounter));
	save_item(NAME(m_machineState));
	save_item(NAME(m_nextstate));
	save_item(NAME(m_laststate));
	save_item(NAME(m_resetState));
	save_item(NAME(m_oddeven));
	save_item(NAME(m_GlobalSilenceState));
	save_item(NAME(m_OldDelta));
	save_item(NAME(m_DACOutput));
	save_item(NAME(m_audioout));
	save_item(NAME(m_filtervals));
	save_item(NAME(m_VSU1000_amp));
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void s14001a_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i;

	for (i = 0; i < samples; i++)
		{
			s14001a_clock();
	#ifdef ACCURATE_SQUEAL
		if (m_audioout == ALTFLAG) // input from test pins -> output
			{
				shiftIntoFilter(chip, audiofilter(chip)); // shift over the previous outputs and stick in audioout.
				outputs[0][i] = audiofilter(chip)*m_VSU1000_amp;
			}
		else // normal, dac-driven output
			{
				shiftIntoFilter(chip, ((((INT16)m_audioout)-8)<<9)); // shift over the previous outputs and stick in audioout 4 times. note <<9 instead of <<10, to prevent clipping, and to simulate that the filtered output normally has a somewhat lower amplitude than the driven one.
	#endif
				outputs[0][i] = ((((INT16)m_audioout)-8)<<10)*m_VSU1000_amp;
	#ifdef ACCURATE_SQUEAL
			}
	#endif
		}
}
