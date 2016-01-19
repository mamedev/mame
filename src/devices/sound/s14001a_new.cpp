// license:BSD-3-Clause
// copyright-holders:Ed Bernard

// http://www.vintagecalculators.com/html/speech-.html

#include "emu.h"
#include "s14001a_new.h"



const device_type S14001A_NEW = &device_creator<s14001a_new_device>;

s14001a_new_device::s14001a_new_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, S14001A_NEW, "S14001A_NEW", tag, owner, clock, "s14001a_new", __FILE__),
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

void s14001a_new_device::device_start()
{
	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock() ? clock() : machine().sample_rate());

	// resolve callbacks
	m_ext_read_handler.resolve();
	m_bsy_handler.resolve();
	
	m_uOutputP1 = m_uOutputP2 = 7;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void s14001a_new_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	for (int i = 0; i < samples; i++)
	{
		Clock();
		INT16 sample = INT16(m_uOutputP2) - 7;
		outputs[0][i] = sample * 0x4000;
	}
}



void s14001a_new_device::force_update()
{
	m_stream->update();
}

READ_LINE_MEMBER(s14001a_new_device::romclock_r)
{
	m_stream->update();
	return (m_bPhase1) ? 1 : 0;
}

READ_LINE_MEMBER(s14001a_new_device::busy_r)
{
	m_stream->update();
	return (m_bBusyP1) ? 1 : 0;
}

WRITE8_MEMBER(s14001a_new_device::data_w)
{
	m_stream->update();
	m_uWord = data & 0x3f; // C0-C5
}

WRITE_LINE_MEMBER(s14001a_new_device::start_w)
{
	m_stream->update();
	m_bStart = (state != 0);
	if (m_bStart) m_uStateP1 = WORDWAIT;
}

void s14001a_new_device::set_clock(int clock)
{
	m_stream->update();
	m_stream->set_sample_rate(clock);
}




UINT8 s14001a_new_device::readmem(UINT16 offset, bool phase)
{
	offset &= 0xfff; // 11-bit internal
	return ((m_ext_read_handler.isnull()) ? m_SpeechRom[offset & (m_SpeechRom.bytes() - 1)] : m_ext_read_handler(offset));
}

bool s14001a_new_device::Clock()
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
		m_uRomAddrP2   = m_RomAddrP1;

		// setup carries from phase 2 values
		m_bDAR04To00CarryP2  = m_uDAR04To00P2 == 0x1F;
		m_bPPQCarryP2        = m_bDAR04To00CarryP2 && ((m_uLengthP2&0x03) == 0x03); // pitch period quarter
		m_bRepeatCarryP2     = m_bPPQCarryP2       && ((m_uLengthP2&0x0C) == 0x0C);
		m_bLengthCarryP2     = m_bRepeatCarryP2    && ( m_uLengthP2       == 0x7F);

		return true;
	}
	m_bPhase1 = true;

	// logic done during phase 1
	switch (m_uStateP1)
	{
	case IDLE:
		m_uOutputP1 = 7;
		if (m_bStart) m_uStateP1 = WORDWAIT;

		if (m_bBusyP1 && !m_bsy_handler.isnull())
			m_bsy_handler(0);
		m_bBusyP1 = false;
		break;

	case WORDWAIT:
		// the delta address register latches the word number into bits 03 to 08
		// all other bits forced to 0.  04 to 08 makes a multiply by two.
		m_uDAR13To05P1 = (m_uWord&0x3C)>>2;
		m_uDAR04To00P1 = (m_uWord&0x03)<<3;
		m_RomAddrP1 = (m_uDAR13To05P1<<3)|(m_uDAR04To00P1>>2); // remove lower two bits
		m_uOutputP1 = 7;
		if (m_bStart) m_uStateP1 = WORDWAIT;
		else          m_uStateP1 = CWARMSB;

		if (!m_bBusyP1 && !m_bsy_handler.isnull())
			m_bsy_handler(1);
		m_bBusyP1 = true;
		break;

	case CWARMSB:
		if (m_uPrintLevel >= 1)
			printf("\n speaking word %02x",m_uWord);

		// use uDAR to load uCWAR 8 msb
		m_uCWARP1 = readmem(m_uRomAddrP2,m_bPhase1)<<4; // note use of rom address setup in previous state
		// increment DAR by 4, 2 lsb's count deltas within a byte
		m_uDAR04To00P1 += 4;
		if (m_uDAR04To00P1 >= 32) m_uDAR04To00P1 = 0; // emulate 5 bit counter
		m_RomAddrP1 = (m_uDAR13To05P1<<3)|(m_uDAR04To00P1>>2); // remove lower two bits

		m_uOutputP1 = 7;
		if (m_bStart) m_uStateP1 = WORDWAIT;
		else          m_uStateP1 = CWARLSB;
		break;

	case CWARLSB:
		m_uCWARP1   = m_uCWARP2|(readmem(m_uRomAddrP2,m_bPhase1)>>4); // setup in previous state
		m_RomAddrP1 = m_uCWARP1;

		m_uOutputP1 = 7;
		if (m_bStart) m_uStateP1 = WORDWAIT;
		else          m_uStateP1 = DARMSB;
		break;

	case DARMSB:
		m_uDAR13To05P1 = readmem(m_uRomAddrP2,m_bPhase1)<<1; // 9 bit counter, 8 MSBs from ROM, lsb zeroed
		m_uDAR04To00P1 = 0;
		m_uCWARP1++;
		m_RomAddrP1 = m_uCWARP1;
		m_uNControlWords++; // statistics

		m_uOutputP1 = 7;
		if (m_bStart) m_uStateP1 = WORDWAIT;
		else          m_uStateP1 = CTRLBITS;
		break;

	case CTRLBITS:
		m_bStopP1    = readmem(m_uRomAddrP2,m_bPhase1)&0x80? true: false;
		m_bVoicedP1  = readmem(m_uRomAddrP2,m_bPhase1)&0x40? true: false;
		m_bSilenceP1 = readmem(m_uRomAddrP2,m_bPhase1)&0x20? true: false;
		m_uXRepeatP1 = readmem(m_uRomAddrP2,m_bPhase1)&0x03;
		m_uLengthP1  =(readmem(m_uRomAddrP2,m_bPhase1)&0x1F)<<2; // includes external length and repeat
		m_uDAR04To00P1 = 0;
		m_uCWARP1++; // gets ready for next DARMSB
		m_RomAddrP1  = (m_uDAR13To05P1<<3)|(m_uDAR04To00P1>>2); // remove lower two bits

		m_uOutputP1 = 7;
		if (m_bStart) m_uStateP1 = WORDWAIT;
		else          m_uStateP1 = PLAY;

		if (m_uPrintLevel >= 2)
			printf("\n cw %d %d %d %d %d",m_bStopP1,m_bVoicedP1,m_bSilenceP1,m_uLengthP1>>4,m_uXRepeatP1);

		break;

	case PLAY:
	{
		// statistics
		if (m_bPPQCarryP2)
		{
			// pitch period end
			if (m_uPrintLevel >= 3)
				printf("\n ppe: RomAddr %03x",m_uRomAddrP2);

			m_uNPitchPeriods++;
			if (m_bVoicedP2) m_uNVoiced++;
		}
		// end statistics

		// modify output
		UINT8 uDeltaP2;     // signal line
		UINT8 uIncrementP2; // signal lines
		bool bAddP2;        // signal line
		uDeltaP2 = Mux8To2(m_bVoicedP2,
					m_uLengthP2 & 0x03,     // pitch period quater counter
					m_uDAR04To00P2 & 0x03,  // two bit delta address within byte
					readmem(m_uRomAddrP2,m_bPhase1)
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
			m_uLengthP1 |= (m_uXRepeatP1<<2); // load repeat from external repeat
			m_uDAR13To05P1++; // advances ROM address 8 bytes
			if (m_uDAR13To05P1 >= 0x200) m_uDAR13To05P1 = 0; // emulate 9 bit counter
		}
		if (!m_bVoicedP2 && m_bDAR04To00CarryP2)
		{
			// unvoiced advances each quarter pitch period
			// note repeat counter not reloaded for non voiced speech
			m_uDAR13To05P1++; // advances ROM address 8 bytes
			if (m_uDAR13To05P1 >= 0x200) m_uDAR13To05P1 = 0; // emulate 9 bit counter
		}

		// construct m_RomAddrP1
		m_RomAddrP1 = m_uDAR04To00P1;
		if (m_bVoicedP2 && m_uLengthP1&0x1) // mirroring
		{
			m_RomAddrP1 ^= 0x1f; // count backwards
		}
		m_RomAddrP1 = (m_uDAR13To05P1<<3) | m_RomAddrP1>>2;

		// next state
		if (m_bStart) m_uStateP1 = WORDWAIT;
		else if (m_bStopP2 && m_bLengthCarryP2) m_uStateP1 = DELAY;
		else if (m_bLengthCarryP2)
		{
			m_uStateP1  = DARMSB;
			m_RomAddrP1 = m_uCWARP1; // output correct address
		}
		else m_uStateP1 = PLAY;
		break;
	}

	case DELAY:
		m_uOutputP1 = 7;
		if (m_bStart) m_uStateP1 = WORDWAIT;
		else          m_uStateP1 = IDLE;
		break;
	}

	return true;
}

UINT8 s14001a_new_device::Mux8To2(bool bVoicedP2, UINT8 uPPQtrP2, UINT8 uDeltaAdrP2, UINT8 uRomDataP2)
{
	// pick two bits of rom data as delta

	if (bVoicedP2 && uPPQtrP2&0x01) // mirroring
	{
		uDeltaAdrP2 ^= 0x03; // count backwards
	}
	// emulate 8 to 2 mux to obtain delta from byte (bigendian)
	switch (uDeltaAdrP2)
	{
	case 0x00:
		return (uRomDataP2&0xC0)>>6;
	case 0x01:
		return (uRomDataP2&0x30)>>4;
	case 0x02:
		return (uRomDataP2&0x0C)>>2;
	case 0x03:
		return (uRomDataP2&0x03)>>0;
	}
	return 0xFF;
}

void s14001a_new_device::CalculateIncrement(bool bVoicedP2, UINT8 uPPQtrP2, bool bPPQStartP2, UINT8 uDelta, UINT8 uDeltaOldP2, UINT8 &uDeltaOldP1, UINT8 &uIncrementP2, bool &bAddP2)
{
	// uPPQtr, pitch period quarter counter; 2 lsb of uLength
	// bPPStart, start of a pitch period
	// implemented to mimic silicon (a bit)

	// beginning of a pitch period
	if (uPPQtrP2 == 0x00 && bPPQStartP2) // note this is done for voiced and unvoiced
	{
		uDeltaOldP2 = 0x02;
	}
	static const UINT8 uIncrements[4][4] =
	{
	//    00  01  10  11
		{ 3,  3,  1,  1,}, // 00
		{ 1,  1,  0,  0,}, // 01
		{ 0,  0,  1,  1,}, // 10
		{ 1,  1,  3,  3 }, // 11
	};

#define MIRROR  (uPPQtrP2&0x01)

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
	if (bVoicedP2 && bPPQStartP2 && MIRROR) uIncrementP2 = 0; // no change when first starting mirroring
}

UINT8 s14001a_new_device::CalculateOutput(bool bVoiced, bool bXSilence, UINT8 uPPQtr, bool bPPQStart, UINT8 uLOutput, UINT8 uIncrementP2, bool bAddP2)
{
	// implemented to mimic silicon (a bit)
	// limits output to 0x00 and 0x0f
	UINT8 uTmp; // used for subtraction

#define SILENCE (uPPQtr&0x02)

	// determine output
	if (bXSilence || (bVoiced && SILENCE)) return 7;

	// beginning of a pitch period
	if (uPPQtr == 0x00 && bPPQStart) // note this is done for voiced and nonvoiced
	{
		uLOutput = 7;
	}

	// adder
	uTmp = uLOutput;
	if (!bAddP2) uTmp ^= 0x0F; // turns subtraction into addition

	// add 0, 1, 3; limit at 15
	uTmp += uIncrementP2;
	if (uTmp > 15) uTmp = 15;

	if (!bAddP2) uTmp ^= 0x0F; // turns addition back to subtraction
	return uTmp;
}

void s14001a_new_device::ClearStatistics()
{
	m_uNPitchPeriods = 0;
	m_uNVoiced       = 0;
	m_uPrintLevel    = 0;
	m_uNControlWords = 0;
}

void s14001a_new_device::GetStatistics(UINT32 &uNPitchPeriods, UINT32 &uNVoiced, UINT32 uNControlWords)
{
	uNPitchPeriods = m_uNPitchPeriods;
	uNVoiced = m_uNVoiced;
	uNControlWords = m_uNControlWords;
}
