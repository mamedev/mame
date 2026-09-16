// license:BSD-3-Clause
// copyright-holders:Ed Bernard, Jonathan Gevaryahu, hap
// thanks-to:Kevin Horton
/*

SSi TSI S14001A speech IC emulator
aka CRC: Custom ROM Controller, designed in 1975, first usage in 1976 on TSI Speech+ calculator
Originally written for MAME by Jonathan Gevaryahu(Lord Nightmare) 2006-2013,
replaced with near-complete rewrite by Ed Bernard in 2016

Further reading:
- http://www.vintagecalculators.com/html/speech-.html
- http://www.vintagecalculators.com/html/development_of_the_tsi_speech-.html
- http://www.vintagecalculators.com/html/speech-_state_machine.html
- https://archive.org/stream/pdfy-QPCSwTWiFz1u9WU_/david_djvu.txt

Chip Pinout:

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
It's likely that they did this to be able to force the speech chip to stop talking,
which is normally impossible. The later 'version 3' TSI speech board as featured in
an advertisement in the John Cater book probably also has this feature, in addition
to external speech rom banking.

The Digital out pins supply a copy of the 4-bit waveform which also goes to the
internal DAC. They are only valid every other clock cycle. It is possible that
on 'invalid' cycles they act as a 4 bit input to drive the dac.

Because it requires -10V to operate, the chip manufacturing process must be PMOS.

* Operation:
Put the 6-bit address of the word to be said onto the C0-C5 word select/speech
address bus lines. Next, clock the START line low-high-low. As long as the START
line is held high, the first address byte of the first word will be read repeatedly
every clock, with the rom enable line enabled constantly (i.e. it doesn't toggle on
and off as it normally does during speech). Once START has gone low-high-low, the
/BUSY line will go low until 3 clocks after the chip is done speaking.

*/

#include "emu.h"
#include "s14001a.h"

#define LOG_SPEAK (1 << 1U) // speech start
#define LOG_PPE   (1 << 2U) // pitch period end
#define LOG_CTRL  (1 << 3U) // control word

#define VERBOSE (0)

#include "logmacro.h"

namespace {

u8 Mux8To2(bool bVoicedP2, u8 uPPQtrP2, u8 uDeltaAdrP2, u8 uRomDataP2)
{
	// pick two bits of rom data as delta

	if (bVoicedP2 && (uPPQtrP2 & 0x01)) // mirroring
		uDeltaAdrP2 ^= 0x03; // count backwards

	// emulate 8 to 2 mux to obtain delta from byte (bigendian)
	return uRomDataP2 >> (~uDeltaAdrP2 << 1 & 0x06) & 0x03;
}


void CalculateIncrement(bool bVoicedP2, u8 uPPQtrP2, bool bPPQStartP2, u8 uDelta, u8 uDeltaOldP2, u8 &uDeltaOldP1, u8 &uIncrementP2, bool &bAddP2)
{
	// uPPQtr, pitch period quarter counter; 2 lsb of uLength
	// bPPStart, start of a pitch period
	// implemented to mimic silicon (a bit)

	// beginning of a pitch period
	if ((uPPQtrP2 == 0x00) && bPPQStartP2) // note this is done for voiced and unvoiced
		uDeltaOldP2 = 0x02;

	static constexpr u8 uIncrements[4][4] =
	{
	//    00  01  10  11
		{ 3,  3,  1,  1,}, // 00
		{ 1,  1,  0,  0,}, // 01
		{ 0,  0,  1,  1,}, // 10
		{ 1,  1,  3,  3 }, // 11
	};

	bool const MIRROR = BIT(uPPQtrP2, 0);

	// calculate increment from delta, always done even if silent to update uDeltaOld
	// in silicon a PLA determined 0,1,3 and add/subtract and passed uDelta to uDeltaOld
	if (!bVoicedP2 || !MIRROR)
	{
		uIncrementP2 = uIncrements[uDelta][uDeltaOldP2];
		bAddP2       = uDelta >= 0x02;
	}
	else
	{
		uIncrementP2 = uIncrements[uDeltaOldP2][uDelta];
		bAddP2       = uDeltaOldP2 < 0x02;
	}
	uDeltaOldP1 = uDelta;
	if (bVoicedP2 && bPPQStartP2 && MIRROR)
		uIncrementP2 = 0; // no change when first starting mirroring
}


u8 CalculateOutput(bool bVoiced, bool bXSilence, u8 uPPQtr, bool bPPQStart, u8 uLOutput, u8 uIncrementP2, bool bAddP2)
{
	// implemented to mimic silicon (a bit)
	// limits output to 0x00 and 0x0f

	bool const SILENCE = BIT(uPPQtr, 1);

	// determine output
	if (bXSilence || (bVoiced && SILENCE))
		return 7;

	// beginning of a pitch period
	if ((uPPQtr == 0x00) && bPPQStart) // note this is done for voiced and nonvoiced
		uLOutput = 7;

	// adder
	u8 uTmp = uLOutput;
	if (!bAddP2)
		uTmp ^= 0x0f; // turns subtraction into addition

	// add 0, 1, 3; limit at 15
	uTmp += uIncrementP2;
	if (uTmp > 15)
		uTmp = 15;

	if (!bAddP2)
		uTmp ^= 0x0f; // turns addition back to subtraction

	return uTmp;
}

} // anonymous namespace


// device definition
DEFINE_DEVICE_TYPE(S14001A, s14001a_device, "s14001a", "SSi TSI S14001A")

s14001a_device::s14001a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, S14001A, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_rom_interface(mconfig, *this),
	m_stream(nullptr),
	m_bsy_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

ALLOW_SAVE_TYPE(s14001a_device::states); // allow save_item on a non-fundamental type

void s14001a_device::device_start()
{
	m_stream = stream_alloc(0, 1, clock() ? clock() : machine().sample_rate());

	// zero-fill
	m_bPhase1 = false;
	m_uStateP1 = m_uStateP2 = states::IDLE;
	m_uDAR13To05P1 = 0;
	m_uDAR13To05P2 = 0;
	m_uDAR04To00P1 = 0;
	m_uDAR04To00P2 = 0;
	m_uCWARP1 = 0;
	m_uCWARP2 = 0;
	m_bStopP1 = false;
	m_bStopP2 = false;
	m_bVoicedP1 = false;
	m_bVoicedP2 = false;
	m_bSilenceP1 = false;
	m_bSilenceP2 = false;
	m_uLengthP1 = 0;
	m_uLengthP2 = 0;
	m_uXRepeatP1 = 0;
	m_uXRepeatP2 = 0;
	m_uDeltaOldP1 = 0;
	m_uDeltaOldP2 = 0;
	m_bDAR04To00CarryP2 = false;
	m_bPPQCarryP2 = false;
	m_bRepeatCarryP2 = false;
	m_bLengthCarryP2 = false;
	m_uRomAddrP1 = 0;
	m_uRomAddrP2 = 0;
	m_bBusyP1 = false;
	m_bStart = false;
	m_uWord = 0;
	m_uOutputP1 = m_uOutputP2 = 7;

	// register for savestates
	save_item(NAME(m_bPhase1));
	save_item(NAME(m_uStateP1));
	save_item(NAME(m_uStateP2));
	save_item(NAME(m_uDAR13To05P1));
	save_item(NAME(m_uDAR13To05P2));
	save_item(NAME(m_uDAR04To00P1));
	save_item(NAME(m_uDAR04To00P2));
	save_item(NAME(m_uCWARP1));
	save_item(NAME(m_uCWARP2));

	save_item(NAME(m_bStopP1));
	save_item(NAME(m_bStopP2));
	save_item(NAME(m_bVoicedP1));
	save_item(NAME(m_bVoicedP2));
	save_item(NAME(m_bSilenceP1));
	save_item(NAME(m_bSilenceP2));
	save_item(NAME(m_uLengthP1));
	save_item(NAME(m_uLengthP2));
	save_item(NAME(m_uXRepeatP1));
	save_item(NAME(m_uXRepeatP2));
	save_item(NAME(m_uDeltaOldP1));
	save_item(NAME(m_uDeltaOldP2));
	save_item(NAME(m_uOutputP1));

	save_item(NAME(m_bDAR04To00CarryP2));
	save_item(NAME(m_bPPQCarryP2));
	save_item(NAME(m_bRepeatCarryP2));
	save_item(NAME(m_bLengthCarryP2));
	save_item(NAME(m_uRomAddrP1));

	save_item(NAME(m_uOutputP2));
	save_item(NAME(m_uRomAddrP2));
	save_item(NAME(m_bBusyP1));
	save_item(NAME(m_bStart));
	save_item(NAME(m_uWord));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void s14001a_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		Clock();
		s16 sample = m_uOutputP2 - 7; // range -7..8
		stream.put_int(0, i, sample, 8);
	}
}


/**************************************************************************
    External interface
**************************************************************************/

int s14001a_device::romen_r()
{
	m_stream->update();
	return (m_bPhase1) ? 1 : 0;
}

int s14001a_device::busy_r()
{
	m_stream->update();
	return (m_bBusyP1) ? 1 : 0;
}

void s14001a_device::data_w(u8 data)
{
	m_stream->update();
	m_uWord = data & 0x3f; // C0-C5
}

void s14001a_device::start_w(int state)
{
	m_stream->update();
	if (state && !m_bStart)
		m_uStateP1 = states::WORDWAIT;
	m_bStart = (state != 0);
}


/**************************************************************************
    Device emulation
**************************************************************************/

u8 s14001a_device::ReadMem(u16 offset, bool phase)
{
	offset &= 0xfff; // 11-bit internal
	return read_byte(offset);
}

bool s14001a_device::Clock()
{
	// effectively toggles external clock twice, one cycle
	// internal clock toggles on external clock transition from 0 to 1 so internal clock will always transition here
	// return false if some emulator problem detected

	// On the actual chip, all register phase 1 values needed to be refreshed from phase 2 values
	// or else risk losing their state due to charge loss.
	// But on a computer the values are static.
	// So to reduce code clutter, phase 1 values are only modified if they are different
	// from the preceeding phase 2 values.

	if (m_bPhase1)
	{
		// transition to phase2
		m_bPhase1 = false;

		// transfer phase1 variables to phase2
		m_uStateP2     = m_uStateP1;
		m_uDAR13To05P2 = m_uDAR13To05P1;
		m_uDAR04To00P2 = m_uDAR04To00P1;
		m_uCWARP2      = m_uCWARP1;
		m_bStopP2      = m_bStopP1;
		m_bVoicedP2    = m_bVoicedP1;
		m_bSilenceP2   = m_bSilenceP1;
		m_uLengthP2    = m_uLengthP1;
		m_uXRepeatP2   = m_uXRepeatP1;
		m_uDeltaOldP2  = m_uDeltaOldP1;

		m_uOutputP2    = m_uOutputP1;
		m_uRomAddrP2   = m_uRomAddrP1;

		// setup carries from phase 2 values
		m_bDAR04To00CarryP2 = m_uDAR04To00P2 == 0x1f;
		m_bPPQCarryP2       = m_bDAR04To00CarryP2 && ((m_uLengthP2 & 0x03) == 0x03); // pitch period quarter
		m_bRepeatCarryP2    = m_bPPQCarryP2       && ((m_uLengthP2 & 0x0c) == 0x0c);
		m_bLengthCarryP2    = m_bRepeatCarryP2    && ( m_uLengthP2         == 0x7f);

		return true;
	}
	m_bPhase1 = true;

	// logic done during phase 1
	switch (m_uStateP1)
	{
	case states::IDLE:
		m_uOutputP1 = 7;
		if (m_bStart)
			m_uStateP1 = states::WORDWAIT;

		if (m_bBusyP1)
			m_bsy_handler(0);
		m_bBusyP1 = false;
		break;

	case states::WORDWAIT:
		// the delta address register latches the word number into bits 03 to 08
		// all other bits forced to 0.  04 to 08 makes a multiply by two.
		m_uDAR13To05P1 = (m_uWord & 0x3c) >> 2;
		m_uDAR04To00P1 = (m_uWord & 0x03) << 3;
		m_uRomAddrP1 = (m_uDAR13To05P1 << 3) | (m_uDAR04To00P1 >> 2); // remove lower two bits

		m_uOutputP1 = 7;
		m_uStateP1  = m_bStart ? states::WORDWAIT : states::CWARMSB;

		if (!m_bBusyP1)
			m_bsy_handler(1);
		m_bBusyP1 = true;
		break;

	case states::CWARMSB:
		LOGMASKED(LOG_SPEAK, "speaking word %02x\n", m_uWord);

		// use uDAR to load uCWAR 8 msb
		m_uCWARP1 = ReadMem(m_uRomAddrP2, m_bPhase1) << 4; // note use of rom address setup in previous state
		// increment DAR by 4, 2 lsb's count deltas within a byte
		m_uDAR04To00P1 += 4;
		if (m_uDAR04To00P1 >= 32)
			m_uDAR04To00P1 = 0; // emulate 5 bit counter
		m_uRomAddrP1 = (m_uDAR13To05P1 << 3) | (m_uDAR04To00P1 >> 2); // remove lower two bits

		m_uOutputP1 = 7;
		m_uStateP1  = m_bStart ? states::WORDWAIT : states::CWARLSB;
		break;

	case states::CWARLSB:
		m_uCWARP1   = m_uCWARP2 | (ReadMem(m_uRomAddrP2, m_bPhase1) >> 4); // setup in previous state
		m_uRomAddrP1 = m_uCWARP1;

		m_uOutputP1 = 7;
		m_uStateP1  = m_bStart ? states::WORDWAIT : states::DARMSB;
		break;

	case states::DARMSB:
		m_uDAR13To05P1 = ReadMem(m_uRomAddrP2, m_bPhase1) << 1; // 9 bit counter, 8 MSBs from ROM, lsb zeroed
		m_uDAR04To00P1 = 0;
		m_uCWARP1++;
		m_uRomAddrP1 = m_uCWARP1;

		m_uOutputP1 = 7;
		m_uStateP1  = m_bStart ? states::WORDWAIT : states::CTRLBITS;
		break;

	case states::CTRLBITS:
	{
		u8 data = ReadMem(m_uRomAddrP2, m_bPhase1);

		m_bStopP1    = bool(data & 0x80);
		m_bVoicedP1  = bool(data & 0x40);
		m_bSilenceP1 = bool(data & 0x20);
		m_uXRepeatP1 = data & 0x03;
		m_uLengthP1  = (data & 0x1f) << 2; // includes external length and repeat
		m_uDAR04To00P1 = 0;
		m_uCWARP1++; // gets ready for next DARMSB
		m_uRomAddrP1  = (m_uDAR13To05P1 << 3) | (m_uDAR04To00P1 >> 2); // remove lower two bits

		m_uOutputP1 = 7;
		m_uStateP1  = m_bStart ? states::WORDWAIT : states::PLAY;

		LOGMASKED(LOG_CTRL, "cw %d %d %d %d %d\n", m_bStopP1, m_bVoicedP1, m_bSilenceP1, m_uLengthP1 >> 4, m_uXRepeatP1);
		break;
	}

	case states::PLAY:
	{
		if (m_bPPQCarryP2)
			LOGMASKED(LOG_PPE, "ppe: RomAddr %03x\n", m_uRomAddrP2); // pitch period end

		// modify output
		u8 uDeltaP2;     // signal line
		u8 uIncrementP2; // signal lines
		bool bAddP2;     // signal line
		uDeltaP2 = Mux8To2(m_bVoicedP2,
				m_uLengthP2 & 0x03,     // pitch period quater counter
				m_uDAR04To00P2 & 0x03,  // two bit delta address within byte
				ReadMem(m_uRomAddrP2, m_bPhase1)
		);
		CalculateIncrement(m_bVoicedP2,
				m_uLengthP2 & 0x03,     // pitch period quater counter
				m_uDAR04To00P2 == 0,    // pitch period quarter start
				uDeltaP2,
				m_uDeltaOldP2,          // input
				m_uDeltaOldP1,          // output
				uIncrementP2,           // output 0, 1, or 3
				bAddP2                  // output
		);
		m_uOutputP1 = CalculateOutput(m_bVoicedP2,
				m_bSilenceP2,
				m_uLengthP2 & 0x03,     // pitch period quater counter
				m_uDAR04To00P2 == 0,    // pitch period quarter start
				m_uOutputP2,            // last output
				uIncrementP2,
				bAddP2
		);

		// advance counters
		m_uDAR04To00P1++;
		if (m_bDAR04To00CarryP2) // pitch period quarter end
		{
			m_uDAR04To00P1 = 0; // emulate 5 bit counter

			m_uLengthP1++; // lower two bits of length count quarter pitch periods
			if (m_uLengthP1 >= 0x80)
			{
				m_uLengthP1 = 0; // emulate 7 bit counter
			}
		}

		if (m_bVoicedP2 && m_bRepeatCarryP2) // repeat complete
		{
			m_uLengthP1 &= 0x70; // keep current "length"
			m_uLengthP1 |= (m_uXRepeatP1 << 2); // load repeat from external repeat
			m_uDAR13To05P1++; // advances ROM address 8 bytes
			if (m_uDAR13To05P1 >= 0x200)
				m_uDAR13To05P1 = 0; // emulate 9 bit counter
		}
		if (!m_bVoicedP2 && m_bDAR04To00CarryP2)
		{
			// unvoiced advances each quarter pitch period
			// note repeat counter not reloaded for non voiced speech
			m_uDAR13To05P1++; // advances ROM address 8 bytes
			if (m_uDAR13To05P1 >= 0x200)
				m_uDAR13To05P1 = 0; // emulate 9 bit counter
		}

		// construct m_uRomAddrP1
		m_uRomAddrP1 = m_uDAR04To00P1;
		if (m_bVoicedP2 && m_uLengthP1 & 0x1) // mirroring
			m_uRomAddrP1 ^= 0x1f; // count backwards
		m_uRomAddrP1 = (m_uDAR13To05P1 << 3) | m_uRomAddrP1 >> 2;

		// next state
		if (m_bStart)
			m_uStateP1 = states::WORDWAIT;
		else if (m_bStopP2 && m_bLengthCarryP2)
			m_uStateP1 = states::DELAY;
		else if (m_bLengthCarryP2)
		{
			m_uStateP1  = states::DARMSB;
			m_uRomAddrP1 = m_uCWARP1; // output correct address
		}
		else
			m_uStateP1 = states::PLAY;
		break;
	}

	case states::DELAY:
		m_uOutputP1 = 7;
		m_uStateP1  = m_bStart ? states::WORDWAIT : states::IDLE;
		break;
	}

	return true;
}
