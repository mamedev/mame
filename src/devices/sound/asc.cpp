// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    asc.cpp

    Apple Sound Chip (ASC) 344S0053 (original), 344S0063 (cost-reduced)
    Enhanced Apple Sound Chip (EASC) 343S1036
    Audio portion of "V8" ASIC (343S0116)
    Audio portion of "Sonora" ASIC (343S1065)
    Audio portion of "IOSB" ASIC (343S1078), copy-pasted in
      "PrimeTime" (343S0135) and "PrimeTime II" (343S0138) ASICs.

    Emulation by R. Belmont
    Big thanks to Doug Brown for the ASCTester utility which tests hardware behavior!
    https://github.com/dougg3/ASCTester/
    https://github.com/dougg3/ASCTester/tree/v4_improvements

    The four-voice wavetable mode is unique to the first-generation ASC.
    EASC (which was codenamed "Batman") and other ASC clones remove it entirely.

    Registers:
    0x800: VERSION
    0x801: MODE (0=inactive, 1=FIFO mode, 2=wavetable mode)
    0x802: CONTROL (bit 0=analog or PWM output, 1=stereo/mono, 7=processing time exceeded)
    0x803: FIFO MODE (bit 7=clear FIFO, bit 1="non-ROM companding", bit 0="ROM companding")
    0x804: FIFO IRQ STATUS (bit 0=ch A 1/2 full, 1=ch A full, 2=ch B 1/2 full, 3=ch B full)
    0x805: WAVETABLE CONTROL (bits 0-3 wavetables 0-3 start)
    0x806: VOLUME (bits 2-4 = 3 bit internal ASC volume, bits 5-7 = volume control sent to Sony sound chip)
    0x807: CLOCK RATE (0 = Mac 22257 Hz, 1 = undefined, 2 = 22050 Hz, 3 = 44100 Hz)
    0x80A: PLAY REC A
    0x80F: TEST (ASC: bits 6-7 = digital test, bits 4-5 = analog test  EASC: bit 7 = 'fast count')
    0x810: WAVETABLE 0 PHASE (big-endian 9.15 fixed-point, only 24 bits valid)
    0x814: WAVETABLE 0 INCREMENT (big-endian 9.15 fixed-point, only 24 bits valid)
    0x818: WAVETABLE 1 PHASE
    0x81C: WAVETABLE 1 INCREMENT
    0x820: WAVETABLE 2 PHASE
    0x824: WAVETABLE 2 INCREMENT
    0x828: WAVETABLE 3 PHASE
    0x82C: WAVETABLE 3 INCREMENT

    EASC only: (IRQ enable also exists on Sonora and IOSB)
    0xF00: Channel A write pointer MSB
    0xF01: Channel A write pointer LSB
    0xF02: Channel A read pointer MSB
    0xF03: Channel A read pointer LSB
    0xF04: Channel A sample rate MSB    (16.16 sample step value minus 1 where 0x00010000 is 44.1 kHz,
    0xF05: Channel A sample rate LSB     so 0x7fff = step value of 0x8000 or 22 kHz)
    0xF06: Channel A left scale factor (volume)
    0xF07: Channel A right scale factor (volume)
    0xF08: Channel A FIFO control (bit 7 = enable sample rate conversion, 0-1 = CD-XA mode)
    0xF09: Channel A FIFO IRQ enable (bit 0 = 0 to enable, 1 to disable)
    0xF10-0xF17: CD-XA ADPCM coefficients for channel A (default: 0x00 0x00 0x00 0x3c 0xcc 0x73 0xc9 0x62)
    0xF20: Channel B write pointer MSB
    0xF21: Channel B write pointer LSB
    0xF22: Channel B read pointer MSB
    0xF23: Channel B read pointer LSB
    0xF24: Channel B sample rate MSB
    0xF25: Channel B sample rate LSB
    0xF26: Channel B left scale factor (volume)
    0xF27: Channel B right scale factor (volume)
    0xF28: Channel B FIFO control (bit 7 = enable sample rate conversion, 0-1 = CD-XA mode)
    0xF29: Channel B FIFO IRQ enable (bit 0 = 0 to enable, 1 to disable)
    0xF30-0xF37: CD-XA ADPCM coefficients for channel B (default: 0x00 0x00 0x00 0x3c 0xcc 0x73 0xc9 0x62)

    Status:
    - Hardware-accurate behavior for ASC, EASC, V8, Sonora, and MSC variants
    - True EASC support including CD-XA playback and sample rate conversion

    TODO:
    - There's some weirdness with the FIFO full IRQ on the original ASC.  It doesn't bother Mac OS,
      but still needs to be understood for a complete ASCTester "pass".
    - The IOSB variant has some surprising differences that aren't yet understood, including 16-bit wide FIFO ports
    - EASC doesn't implement the channel volumes yet because Mac OS doesn't use them
    - Recording is likely to be a new can of worms and an enhanced ASCTester will likely be necessary

***************************************************************************/

#include "emu.h"
#include "asc.h"

#include "multibyte.h"

// device type definition
DEFINE_DEVICE_TYPE(ASC, asc_device, "asc", "Apple Sound Chip")
DEFINE_DEVICE_TYPE(ASC_V8, asc_v8_device, "asc_v8", "Apple Sound Chip, V8 variant")
DEFINE_DEVICE_TYPE(ASC_SONORA, asc_sonora_device, "asc_sonora", "Apple Sound Chip, Sonora variant")
DEFINE_DEVICE_TYPE(ASC_IOSB, asc_iosb_device, "asc_iosb", "Apple Sound Chip, IOSB variant")
DEFINE_DEVICE_TYPE(ASC_MSC, asc_msc_device, "asc_msc", "Apple Sound Chip, MSC variant")
DEFINE_DEVICE_TYPE(ASC_EASC, asc_easc_device, "asc_easc", "Enhanced Apple Sound Chip")

static constexpr u32 CONTROL_STEREO = 1;

static constexpr u8 STAT_HALF_FULL_A        = 0x01;
static constexpr u8 STAT_EMPTY_OR_FULL_A    = 0x02;
static constexpr u8 STAT_HALF_FULL_B        = 0x04;
static constexpr u8 STAT_EMPTY_OR_FULL_B    = 0x08;

//-------------------------------------------------
//  asc_base_device - constructor
//-------------------------------------------------

asc_base_device::asc_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, write_irq(*this)
	, m_fifo_clrptr{ 0x400, 0x400 }
	, m_last_left(0)
	, m_last_right(0)
	, m_sample_rate(22257)
	, m_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void asc_base_device::device_start()
{
	// create the stream
	m_stream = stream_alloc(0, 2, m_sample_rate);

	memset(m_regs, 0, sizeof(m_regs));

	m_timer = timer_alloc(FUNC(asc_base_device::delayed_stream_update), this);

	save_item(NAME(m_fifo_rdptr));
	save_item(NAME(m_fifo_wrptr));
	save_item(NAME(m_fifo_cap));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_clrptr));
	save_item(NAME(m_last_left));
	save_item(NAME(m_last_right));
	save_item(NAME(m_regs));
	save_item(NAME(m_phase));
	save_item(NAME(m_incr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void asc_base_device::device_reset()
{
	m_stream->update();

	memset(m_regs, 0, sizeof(m_regs));
	memset(m_fifo, 0, sizeof(m_fifo));
	memset(m_phase, 0, sizeof(m_phase));
	memset(m_incr, 0, sizeof(m_incr));

	m_fifo_rdptr[0] = m_fifo_rdptr[1] = 0;
	m_fifo_wrptr[0] = m_fifo_wrptr[1] = 0;
	m_fifo_cap[0] = m_fifo_cap[1] = 0;
	m_last_left = m_last_right = 0;
}

//-------------------------------------------------
//  delayed_stream_update -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(asc_base_device::delayed_stream_update)
{
	m_stream->update();
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void asc_base_device::sound_stream_update(sound_stream &stream)
{
	constexpr u32 wtoffs[2] = { 0, 0x200 };

	switch (m_regs[R_MODE] & 3)
	{
		case 0: // chip off
			for (int i = 0; i < stream.samples(); i++)
			{
				stream.put_int(0, i, m_last_left, 32768 / 64);
				stream.put_int(1, i, m_last_right, 32768 / 64);
			}
			break;

		case 1: // FIFO mode
		{
			for (int i = 0; i < stream.samples(); i++)
			{
				s8 smpll = (s8)m_fifo[0][m_fifo_rdptr[0]] ^ 0x80;
				s8 smplr = (s8)m_fifo[1][m_fifo_rdptr[1]] ^ 0x80;
				if (!BIT(m_regs[R_CONTROL], CONTROL_STEREO))
				{
					smplr = smpll;
				}
				m_last_left = smpll;
				m_last_right = smplr;

				// we want the cap # from prior to the decrement so we don't trigger on the last sample in the FIFO
				int cap_a = m_fifo_cap[0];
				int cap_b = m_fifo_cap[1];

				// don't advance the sample pointer if there are no more samples
				if (m_fifo_cap[0])
				{
					m_fifo_rdptr[0]++;
					m_fifo_rdptr[0] &= 0x3ff;
					m_fifo_cap[0]--;
				}

				if (BIT(m_regs[R_CONTROL], CONTROL_STEREO))
				{
					if (m_fifo_cap[1])
					{
						m_fifo_rdptr[1]++;
						m_fifo_rdptr[1] &= 0x3ff;
						m_fifo_cap[1]--;
					}
				}

				//printf("chip updating: cap A %x cap B %x\n", m_fifo_cap[0], m_fifo_cap[1]);
				if (cap_a == 0x1ff)
				{
					m_regs[R_FIFOSTAT] |= STAT_HALF_FULL_A;  // fifo A half-empty
					m_regs[R_FIFOSTAT] &= ~STAT_EMPTY_OR_FULL_A; // fifo A not full/empty
					set_irq_line(ASSERT_LINE);
				}

				if (BIT(m_regs[R_CONTROL], CONTROL_STEREO))
				{
					if (cap_b == 0x1ff)
					{
						m_regs[R_FIFOSTAT] |= STAT_HALF_FULL_B; // fifo B half-empty
						m_regs[R_FIFOSTAT] &= ~STAT_EMPTY_OR_FULL_B; // fifo B not full/empty
						set_irq_line(ASSERT_LINE);
					}
				}

				stream.put_int(0, i, smpll, 32768 / 64);
				stream.put_int(1, i, smplr, 32768 / 64);
			}
			break;
		}

		case 2: // wavetable mode
		{
			for (int i = 0; i < stream.samples(); i++)
			{
				s32 mixL, mixR;

				mixL = mixR = 0;

				// update channel pointers
				for (int ch = 0; ch < 4; ch++)
				{
					m_phase[ch] += m_incr[ch];

					s8 smpl;
					if (ch < 2)
					{
						smpl = (s8)m_fifo[0][((m_phase[ch]>>15)&0x1ff) + wtoffs[ch&1]];
					}
					else
					{
						smpl = (s8)m_fifo[1][((m_phase[ch]>>15)&0x1ff) + wtoffs[ch&1]];
					}
					smpl ^= 0x80;
					mixL += smpl*256;
					mixR += smpl*256;
				}

				m_last_left = mixL;
				m_last_right = mixR;

				stream.put_int(0, i, mixL, 32768 * 4);
				stream.put_int(1, i, mixR, 32768 * 4);
			}
			break;
		}
	}

//  printf("rdA %04x rdB %04x wrA %04x wrB %04x (capA %04x B %04x)\n", m_fifo_rdptr[0], m_fifo_rdptr[1], m_fifo_wrptr[0], m_fifo_wrptr[1], m_fifo_cap[0], m_fifo_cap[1]);
}

//-------------------------------------------------
//  read - read from the chip's registers and internal RAM
//-------------------------------------------------

u8 asc_base_device::read(offs_t offset)
{
	u8 rv;

//  printf("ASC: read at %x\n", offset);

	// not sure what actually happens when the CPU reads the FIFO...
	if (offset < 0x400)
	{
		return m_fifo[0][offset];
	}
	else if (offset < 0x800)
	{
		return m_fifo[1][offset - 0x400];
	}
	else
	{
		m_stream->update();
		switch (offset - 0x800)
		{
			case R_VERSION:
				return get_version();

			case R_MODE:
				break;

			case R_CONTROL:
				break;

			case R_FIFOSTAT:
				rv = m_regs[R_FIFOSTAT];

				if (!machine().side_effects_disabled())
				{
					// reading this register clears all bits, but only on original ASC
					m_regs[R_FIFOSTAT] = 0;

					// reading this clears interrupts
					set_irq_line(CLEAR_LINE);
				}
				return rv;

			// these are known to return 1 on original ASC
			case R_FIFOA_IRQCTRL:
			case R_FIFOB_IRQCTRL:
				return 0x01;

			default:
				break;
		}
	}

	// WT inc/phase registers - rebuild from "live" copies"
	if ((offset >= 0x810) && (offset <= 0x82f))
	{
		put_u24be(&m_regs[0x11], m_phase[0]);
		put_u24be(&m_regs[0x15], m_incr[0]);

		put_u24be(&m_regs[0x19], m_phase[1]);
		put_u24be(&m_regs[0x1d], m_incr[1]);

		put_u24be(&m_regs[0x21], m_phase[2]);
		put_u24be(&m_regs[0x25], m_incr[2]);

		put_u24be(&m_regs[0x29], m_phase[3]);
		put_u24be(&m_regs[0x2d], m_incr[3]);
	}

	if (offset >= 0x1000)
	{
		return 0xff;
	}

	return m_regs[offset - 0x800];
}

//-------------------------------------------------
//  write - write to the chip's registers and internal RAM
//-------------------------------------------------

void asc_base_device::write(offs_t offset, u8 data)
{
	if (offset == 0xe00)
	{
		m_regs[R_FIFOSTAT & 0x7ff] |= 0xf;
		set_irq_line(ASSERT_LINE);
	}

	if (offset < 0x400)
	{
//          printf("FIFO A write %02x @ position %04x cap %04x\n", data, m_fifo_wrptr[0], m_fifo_cap[0]);
		m_fifo[0][m_fifo_wrptr[0]++] = data;
		m_fifo_cap[0]++;

		if (!(m_regs[R_PLAYRECA] & 1))
		{
			if (m_fifo_cap[0] >= 0x200)
			{
				m_regs[R_FIFOSTAT] &= ~STAT_HALF_FULL_A; // fifo A not half-empty

				if (m_fifo_cap[0] >= 0x3ff)
				{
					m_regs[R_FIFOSTAT] |= STAT_EMPTY_OR_FULL_A; // fifo A full
				}
			}
			else
			{
				if (m_fifo_cap[0] > 0)
				{
					m_regs[R_FIFOSTAT] &= ~STAT_EMPTY_OR_FULL_A; // fifo A not full/empty
				}
			}
		}

		m_fifo_wrptr[0] &= 0x3ff;
	}
	else if (offset < 0x800)
	{
		m_fifo[1][m_fifo_wrptr[1]++] = data;
		m_fifo_cap[1]++;

		if (m_fifo_cap[1] >= 0x200)
		{
			m_regs[R_FIFOSTAT] &= ~STAT_HALF_FULL_B;  // fifo B not half-empty

			if (m_fifo_cap[1] >= 0x3ff)
			{
				m_regs[R_FIFOSTAT] |= STAT_EMPTY_OR_FULL_B;  // fifo B full
			}
		}
		else
		{
			if (m_fifo_cap[1] > 0)
			{
				m_regs[R_FIFOSTAT] &= ~STAT_EMPTY_OR_FULL_B;
			}
		}

		m_fifo_wrptr[1] &= 0x3ff;
	}
	else
	{
//      printf("ASC: %02x to %x (was %x)\n", data, offset, m_regs[offset-0x800]);

		m_stream->update();
		switch (offset)
		{
			case R_MODE:
				data &= 3;  // only bits 0 and 1 can be written
				//printf("ASC mode set to %d\n", data);
				if (data != m_regs[R_MODE])
				{
					m_fifo_rdptr[0] = m_fifo_rdptr[1] = 0;
					m_fifo_wrptr[0] = m_fifo_wrptr[1] = 0;
					m_fifo_cap[0] = m_fifo_cap[1] = 0;

					if (data != 0)
					{
						m_timer->adjust(attotime::zero, 0, attotime::from_hz(m_sample_rate/4));
					}
					else
					{
						m_timer->adjust(attotime::never);
					}
				}
				break;

			case R_FIFOMODE:
				if (data & 0x80)
				{
					m_fifo_rdptr[0] = m_fifo_rdptr[1] = 0;
					m_fifo_wrptr[0] = m_fifo_wrptr[1] = 0;
					m_fifo_cap[0] = m_fifo_cap[1] = 0;
					m_regs[R_FIFOSTAT] |= 0xa;  // fifos A&B empty
				}
				break;

			case R_WTCONTROL:
//              printf("One-shot wavetable %02x\n", data);
				break;

			case 0x811:
				m_phase[0] &= 0x00ffff;
				m_phase[0] |= data<<16;
				break;

			case 0x812:
				m_phase[0] &= 0xff00ff;
				m_phase[0] |= data<<8;
				break;

			case 0x813:
				m_phase[0] &= 0xffff00;
				m_phase[0] |= data;
				break;

			case 0x815:
				m_incr[0] &= 0x00ffff;
				m_incr[0] |= data<<16;
				break;

			case 0x816:
				m_incr[0] &= 0xff00ff;
				m_incr[0] |= data<<8;
				break;

			case 0x817:
				m_incr[0] &= 0xffff00;
				m_incr[0] |= data;
				break;

			case 0x819:
				m_phase[1] &= 0x00ffff;
				m_phase[1] |= data<<16;
				break;

			case 0x81a:
				m_phase[1] &= 0xff00ff;
				m_phase[1] |= data<<8;
				break;

			case 0x81b:
				m_phase[1] &= 0xffff00;
				m_phase[1] |= data;
				break;

			case 0x81d:
				m_incr[1] &= 0x00ffff;
				m_incr[1] |= data<<16;
				break;

			case 0x81e:
				m_incr[1] &= 0xff00ff;
				m_incr[1] |= data<<8;
				break;

			case 0x81f:
				m_incr[1] &= 0xffff00;
				m_incr[1] |= data;
				break;

			case 0x821:
				m_phase[2] &= 0x00ffff;
				m_phase[2] |= data<<16;
				break;

			case 0x822:
				m_phase[2] &= 0xff00ff;
				m_phase[2] |= data<<8;
				break;

			case 0x823:
				m_phase[2] &= 0xffff00;
				m_phase[2] |= data;
				break;

			case 0x825:
				m_incr[2] &= 0x00ffff;
				m_incr[2] |= data<<16;
				break;

			case 0x826:
				m_incr[2] &= 0xff00ff;
				m_incr[2] |= data<<8;
				break;

			case 0x827:
				m_incr[2] &= 0xffff00;
				m_incr[2] |= data;
				break;

			case 0x829:
				m_phase[3] &= 0x00ffff;
				m_phase[3] |= data<<16;
				break;

			case 0x82a:
				m_phase[3] &= 0xff00ff;
				m_phase[3] |= data<<8;
				break;

			case 0x82b:
				m_phase[3] &= 0xffff00;
				m_phase[3] |= data;
				break;

			case 0x82d:
				m_incr[3] &= 0x00ffff;
				m_incr[3] |= data<<16;
				break;

			case 0x82e:
				m_incr[3] &= 0xff00ff;
				m_incr[3] |= data<<8;
				break;

			case 0x82f:
				m_incr[3] &= 0xffff00;
				m_incr[3] |= data;
				break;
		}

		if (offset >= 0x800 && offset < 0x1000)
		{
			m_regs[offset-0x800] = data;
		}
	}
}

u8 asc_base_device::get_version()
{
	return 0x00;
}

void asc_base_device::set_irq_line(int status)
{
	write_irq(status);
}

asc_device::asc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: asc_base_device(mconfig, ASC, tag, owner, clock)
{
}

/*
IIci real hardware:
ASCTester test version 4
BoxFlag: 5   ASC Version: $00   System 7.1.0
AddrMapFlags: $0000773F
F09: 0 ($00)  F29: 0 ($00)
804Idle: $00  M0: 1 M1: 1 M2: 1 ($00)
Mono: 1 1 Stereo: 1 1
Mono FIFO Tests:
0 0 1 0 1 0 1 0 1 0 0 0 (1105 0)
Stereo FIFO Tests:
0 0 1 1 1 1 1 1 1 1 0 0 (1116 1116)
VIA2 (1 $0013) 1
Idle IRQ 0 0 0 (0), 0 0 0 (0), 0 0 0
FIFO IRQ 1 0 0 0
(1 1), (1 1), (0 0), (0 0), 1
2 IRQs (start time = 2756):
$0A at t =  2756, $05 at t =  2757
Cancel 1 0 1 0 0 $0F 0

MAME:
ASCTester test version 4
BoxFlag: 5   ASC Version: $00   System 7.1.1
AddrMapFlags: $0000773F
F09: 0 ($00)  F29: 0 ($00)
804Idle: $00  M0: 1 M1: 1 M2: 1 ($00)
Mono: 1 1 Stereo: 1 1
Mono FIFO Tests:
0 0 1 0 1 0 1 0 1 0 0 0 (1099 0)
Stereo FIFO Tests:
0 0 1 1 1 1 1 1 1 1 0 0 (1117 1117)
VIA2 (1 $0013) 1
Idle IRQ 0 0 0 (0), 0 0 0 (0), 0 0 0
FIFO IRQ 1 0 0 0
(1 1), (1 1), (0 0), (3 3), 1
5 IRQs (start time = 1690):
$02 at t =  1690, $02 at t =  1690, $02 at t =  1690, $08 at t =  1690,
$05 at t =  1692
Cancel 1 0 0 0 0 $05 0
*/

u8 asc_device::read(offs_t offset)
{
	switch (offset)
	{
		case R_FIFOA_IRQCTRL:
		case R_FIFOB_IRQCTRL:
			return 0;
	}

	return asc_base_device::read(offset);
}

void asc_device::write(offs_t offset, u8 data)
{
	if (offset < 0x400)
	{
		if (m_regs[R_MODE] == 1)
		{
			m_fifo[0][m_fifo_wrptr[0]++] = data;
			m_fifo_cap[0]++;

			if (m_fifo_cap[0] >= 0x200)
			{
				m_regs[R_FIFOSTAT] &= ~STAT_HALF_FULL_A; // fifo A not half-empty

				if (m_fifo_cap[0] >= 0x3ff)
				{
					m_regs[R_FIFOSTAT] |= STAT_EMPTY_OR_FULL_A; // fifo A full
					set_irq_line(ASSERT_LINE);
				}
			}
			else
			{
				if (m_fifo_cap[0] > 0)
				{
					m_regs[R_FIFOSTAT] &= ~STAT_EMPTY_OR_FULL_A; // fifo A not full/empty
				}
			}

			m_fifo_wrptr[0] &= 0x3ff;
		}
		else
		{
			m_fifo[0][offset] = data;
		}
		return;
	}
	else if (offset < 0x800)
	{
		if (m_regs[R_MODE] == 1)
		{
			if (BIT(m_regs[R_CONTROL], CONTROL_STEREO))
			{
				m_fifo[1][m_fifo_wrptr[1]++] = data;
				m_fifo_cap[1]++;

				if (m_fifo_cap[1] >= 0x200)
				{
					m_regs[R_FIFOSTAT] &= ~STAT_HALF_FULL_B; // fifo B not half-empty

					if (m_fifo_cap[1] >= 0x3ff)
					{
						m_regs[R_FIFOSTAT] |= STAT_EMPTY_OR_FULL_B; // fifo B full
						set_irq_line(ASSERT_LINE);
					}
				}
				else
				{
					if (m_fifo_cap[1] > 0)
					{
						m_regs[R_FIFOSTAT] &= ~STAT_EMPTY_OR_FULL_B;
					}
				}

				m_fifo_wrptr[1] &= 0x3ff;
			}
		}
		else
		{
			m_fifo[1][offset - 0x400] = data;
		}
		return;
	}

	asc_base_device::write(offset, data);
}

// asc_v8_device implementation
asc_v8_device::asc_v8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: asc_v8_device(mconfig, ASC_V8, tag, owner, clock)
{
}

asc_v8_device::asc_v8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: asc_base_device(mconfig, type, tag, owner, clock)
{
}

void asc_v8_device::device_reset()
{
	asc_base_device::device_reset();

	m_regs[R_MODE] = 1;
	m_regs[R_FIFOSTAT] = 0x2; // FIFO A is empty at reset
}
/*
Mac LC real hardware:
ASCTester test version 4
BoxFlag: 13   ASC Version: $E8   System 7.1.0
AddrMapFlags: $0000773F
F09: 0 ($00)  F29: 0 ($00)
804Idle: $03  M0: 0 M1: 1 M2: 0 ($01)
Mono: 1 1 Stereo: 0 0
Mono FIFO Tests:
0 0 1 0 1 0 1 0 1 0 1 0 (1190 0)
VIA2 (1 $0013) 1
Idle IRQ 1 1 1 (50000), 0 0 0 (0), 0 0 0
FIFO IRQ 1 1 0 0
(0 0), (528 528), (50000 50000), (0 0), 0
50528 IRQs (start time = 2497):
$01 at t =  2499, $01 at t =  2499, $01 at 4 =  2499, $01 at t =  2499,
$01 at t =  2499, $01 at t =  2499, $01 at 4 =  2499, $01 at t =  2499,
$01 at t =  2499, $01 at t =  2499, $01 at 4 =  2499, $01 at t =  2499, ...
Cancel 0 0 0 0 0 $00 0

MAME:
ASCTester test version 4
BoxFlag: 13   ASC Version: $E8   System 7.1.1
AddrMapFlags: $0000773F
F09: 0 ($01)  F29: 0 ($01)
804Idle: $03  M0: 0 M1: 1 M2: 0 ($01)
Mono: 1 1 Stereo: 0 0
Mono FIFO Tests:
0 0 1 0 1 0 1 0 1 0 1 0 (1150 0)
VIA2 (1 $0013) 1
Idle IRQ 1 1 1 (50000), 0 0 0 (0), 0 0 0
FIFO IRQ 1 1 0 0
(0 0), (996 996), (50000 50000), (0 0), 0
50996 IRQs (start time = 2637):
$01 at t =  2648, $01 at t =  2648, $01 at 4 =  2648, $01 at t =  2648,
$01 at t =  2648, $01 at t =  2648, $01 at 4 =  2648, $01 at t =  2648,
$01 at t =  2648, $01 at t =  2648, $01 at 4 =  2648, $01 at t =  2648, ...
Cancel 0 0 0 0 0 $00 0
*/

void asc_v8_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		const s8 smpl = (s8)m_fifo[0][m_fifo_rdptr[0]] ^ 0x80;

		// don't advance the sample pointer if there are no more samples
		if (m_fifo_cap[0])
		{
			m_fifo_rdptr[0]++;
			m_fifo_rdptr[0] &= 0x3ff;
			m_fifo_cap[0]--;
		}

		// printf("chip updating: cap A %x cap B %x\n", m_fifo_cap[0], m_fifo_cap[1]);
		if (m_fifo_cap[0] < 0x200)
		{
			m_regs[R_FIFOSTAT] |= STAT_HALF_FULL_A;
			set_irq_line(ASSERT_LINE);
		}
		else
		{
			m_regs[R_FIFOSTAT] &= ~STAT_HALF_FULL_A;
		}

		if (m_fifo_cap[0] == 0x0)
		{
			m_regs[R_FIFOSTAT] |= STAT_EMPTY_OR_FULL_A;
		}
		else
		{
			m_regs[R_FIFOSTAT] &= ~STAT_EMPTY_OR_FULL_A;
		}

		stream.put_int(0, i, smpl, 32768 / 64);
		stream.put_int(1, i, smpl, 32768 / 64);
	}

	//  printf("rdA %04x rdB %04x wrA %04x wrB %04x (capA %04x B %04x)\n", m_fifo_rdptr[0], m_fifo_rdptr[1], m_fifo_wrptr[0], m_fifo_wrptr[1], m_fifo_cap[0], m_fifo_cap[1]);
}

u8 asc_v8_device::read(offs_t offset)
{
	switch (offset - 0x800)
	{
	// these registers always read as 1 on V8
	case R_MODE:
	case R_CONTROL:
	case R_FIFOMODE:
		return 1;

	// these registers always read as 0 on V8
	case R_WTCONTROL:
	case R_CLOCK:
	case R_BATMANCONTROL:
		return 0;

	case R_FIFOSTAT:
		// reading this clears interrupts, but not the FIFO status bits
		if (!machine().side_effects_disabled() && !(m_regs[R_FIFOSTAT] & STAT_HALF_FULL_A))
		{
			set_irq_line(CLEAR_LINE);
		}
		return m_regs[R_FIFOSTAT];

	case R_FIFOA_IRQCTRL:
	case R_FIFOB_IRQCTRL:
		return 1;

	default:
		break;
	}

	return asc_base_device::read(offset);
}

void asc_v8_device::write(offs_t offset, u8 data)
{
	if ((offset >= 0x400) && (offset < 0x800))
	{
		// writes to FIFO B are ignored on V8
		return;
	}

	switch (offset - 0x800)
	{
	// these registers are read-only or do nothing on V8
	case R_MODE:
	case R_CONTROL:
	case R_WTCONTROL:
	case R_CLOCK:
	case R_BATMANCONTROL:
		return;
	}

	asc_base_device::write(offset, data);

	if ((offset < 0x400) && !(m_regs[R_FIFOSTAT]))
	{
		set_irq_line(CLEAR_LINE);
	}
}

u8 asc_v8_device::get_version()
{
	return 0xe8;
}

// asc_sonora_device implementation
asc_sonora_device::asc_sonora_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: asc_base_device(mconfig, ASC_SONORA, tag, owner, clock)
{
}

/*
Mac LC III real hardware:
ASCTester test version 4
BoxFlag: 21   ASC Version: $BC   System 7.1.0
AddrMapFlags: $0000773F
F09: 1 ($01)  F29: 1 ($01)
804Idle: $0E  M0: 0 M1: 1 M2: 0 ($01)
Mono: 1 0 Stereo: 0 1
Stereo FIFO Tests:
1 0 1 1 1 1 0 1 0 1 1 1 (0 1136)
VIA2 (1 $001F) 1
Idle IRQ 0 0 0 (0), 1 1 1 (50000), 1 1 1
FIFO IRQ 1 0 0 0
(0 0), (1199 1199), (50000 50000), (0 0), 0
51199 IRQs (start time = 4577):
$06 at t =  4579, $06 at t =  4579, $06 at t =  4579, $06 at t =  4579,
$06 at t =  4579, $06 at t =  4579, $06 at t =  4579, $06 at t =  4579,
$06 at t =  4579, $06 at t =  4579, $06 at t =  4579, $06 at t =  4579, ...
Cancel 0 0 0 0 0 $00 0

MAME:
ASCTester test version 4
BoxFlag: 21   ASC Version: $BC   System 7.1.1
AddrMapFlags: $0000773F
F09: 1 ($01)  F29: 1 ($01)
804Idle: $0E  M0: 0 M1: 1 M2: 0 ($01)
Mono: 1 0 Stereo: 0 1
Stereo FIFO Tests:
1 0 1 1 1 1 0 1 0 1 1 1 (0 1136)
VIA2 (1 $001F) 1
Idle IRQ 0 0 0 (0), 1 1 1 (50000), 1 1 1
FIFO IRQ 1 0 0 0
(0 0), (1504 1504), (50000 50000), (0 0), 0
51504 IRQs (start time = 5142):
$06 at t =  5153, $06 at t =  5153, $06 at t =  5153, $06 at t =  5153,
$06 at t =  5153, $06 at t =  5153, $06 at t =  5153, $06 at t =  5153,
$06 at t =  5153, $06 at t =  5153, $06 at t =  5153, $06 at t =  5153, ...
Cancel 0 0 0 0 0 $00 0
*/

void asc_sonora_device::device_reset()
{
	asc_base_device::device_reset();

	m_fifo_irqen[0] = 0;
	m_fifo_irqen[1] = 0;

	m_regs[R_MODE] = 1;
	m_regs[R_FIFOSTAT] = 0x2;   // FIFO A is empty at reset
}

void asc_sonora_device::sound_stream_update(sound_stream &stream)
{
	// if we're in playback mode, FIFO A always reads empty
	if (!(m_regs[R_PLAYRECA] & 1))
	{
		m_regs[R_FIFOSTAT] |= STAT_EMPTY_OR_FULL_A;

		if (!(m_fifo_irqen[0] & 1))
		{
			set_irq_line(ASSERT_LINE);
		}
	}

	for (int i = 0; i < stream.samples(); i++)
	{
		const s8 smpll = (s8)m_fifo[0][m_fifo_rdptr[0]] ^ 0x80;
		const s8 smplr = (s8)m_fifo[1][m_fifo_rdptr[1]] ^ 0x80;

		m_last_left = smpll;
		m_last_right = smplr;

		// don't advance the sample pointer if there are no more samples
		if (m_fifo_cap[0])
		{
			m_fifo_rdptr[0]++;
			m_fifo_rdptr[0] &= 0x3ff;
			m_fifo_cap[0]--;
		}

		if (m_fifo_cap[1])
		{
			m_fifo_rdptr[1]++;
			m_fifo_rdptr[1] &= 0x3ff;
			m_fifo_cap[1]--;
		}

		// printf("chip updating: cap A %x cap B %x\n", m_fifo_cap[0], m_fifo_cap[1]);
		if ((m_fifo_cap[0] < 0x200) || (m_fifo_cap[1] < 0x200))
		{
			m_regs[R_FIFOSTAT] |= STAT_HALF_FULL_B;

			if (!(m_fifo_irqen[1] & 1))
			{
				set_irq_line(ASSERT_LINE);
			}
		}
		else
		{
			m_regs[R_FIFOSTAT] &= ~STAT_HALF_FULL_B;
		}

		if ((m_fifo_cap[0] == 0x0) || (m_fifo_cap[1] == 0x0)) // either fifo full or empty
		{
			m_regs[R_FIFOSTAT] |= STAT_EMPTY_OR_FULL_B;
		}
		else
		{
			m_regs[R_FIFOSTAT] &= ~STAT_EMPTY_OR_FULL_B;
		}

		stream.put_int(0, i, smpll, 32768 / 64);
		stream.put_int(1, i, smplr, 32768 / 64);
	}

	//  printf("rdA %04x rdB %04x wrA %04x wrB %04x (capA %04x B %04x)\n", m_fifo_rdptr[0], m_fifo_rdptr[1], m_fifo_wrptr[0], m_fifo_wrptr[1], m_fifo_cap[0], m_fifo_cap[1]);
}

u8 asc_sonora_device::read(offs_t offset)
{
	switch (offset - 0x800)
	{
	case R_MODE:
		return 1;

	// these registers always read as 0 on Sonora
	case R_CONTROL:
	case R_FIFOMODE:
	case R_WTCONTROL:
	case R_CLOCK:
	case R_BATMANCONTROL:
		return 0;

	case R_FIFOSTAT:
		// reading this clears interrupts, but not the FIFO status bits
		if (!machine().side_effects_disabled() && !(m_regs[R_FIFOSTAT] & STAT_HALF_FULL_B))
		{
			set_irq_line(CLEAR_LINE);
		}
		return m_regs[R_FIFOSTAT];

	case R_FIFOA_IRQCTRL:
		return m_fifo_irqen[0];

	case R_FIFOB_IRQCTRL:
		return m_fifo_irqen[1];

	default:
		break;
	}

	return asc_base_device::read(offset);
}

void asc_sonora_device::write(offs_t offset, u8 data)
{
	switch (offset - 0x800)
	{
	// these registers are read-only or do nothing on Sonora
	case R_MODE:
	case R_CONTROL:
	case R_WTCONTROL:
	case R_CLOCK:
	case R_BATMANCONTROL:
		return;

	case R_FIFOA_IRQCTRL:
		if (!(data & 1) && (m_fifo_irqen[0] & 1) && (m_regs[R_FIFOSTAT] & STAT_HALF_FULL_A))
		{
			set_irq_line(ASSERT_LINE);
		}
		else if (data & 1)
		{
			set_irq_line(CLEAR_LINE);
		}
		m_fifo_irqen[0] = data & 1;
		break;

	case R_FIFOB_IRQCTRL:
		if (!(data & 1) && (m_fifo_irqen[1] & 1) && (m_regs[R_FIFOSTAT] & STAT_HALF_FULL_B))
		{
			set_irq_line(ASSERT_LINE);
		}
		else if (data & 1)
		{
			set_irq_line(CLEAR_LINE);
		}
		m_fifo_irqen[1] = data & 1;
		break;
	}

	asc_base_device::write(offset, data);

	// in playback mode, FIFO A is always empty
	if (offset < 0x400)
	{
		if (!(m_regs[R_PLAYRECA] & 1))
		{
			m_regs[R_FIFOSTAT] |= 2;
		}
	}
}

u8 asc_sonora_device::get_version()
{
	return 0xbc;
}

// asc_iosb_device implementation
asc_iosb_device::asc_iosb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: asc_base_device(mconfig, ASC_IOSB, tag, owner, clock)
{
}

/*
Mac LC 475 real hardware:
ASCTester test version 4
BoxFlag: 83   ASC Version: $BB   System 7.5.3
AddrMapFlags: $0500183F
F09: 1 ($01)  F29: 1 ($01)
804Idle: $0E  M0: 1 M1: 1 M2: 0 ($01)
Mono: 1 0 Stereo: 0 1
Stereo FIFO Tests:
1 0 1 1 1 1 0 1 0 1 1 1 (0 1104)
VIA2 (1 $0000) 1
Idle IRQ 0 0 0 (0), 1 0 0 (1), 1 0 0
FIFO IRQ 1 0 0 0
(0 0), (1 1), (0 0), (0 0), 0
1 IRQs (start time = 2614):
$06 at t =  2615
Cancel 1 0 1 0 0 $0E 0

MAME:
ASCTester test version 4
BoxFlag: 29   ASC Version: $BB   System 7.1.1
AddrMapFlags: $0500183F
F09: 1 ($01)  F29: 1 ($01)
804Idle: $0E  M0: 1 M1: 1 M2: 0 ($01)
Mono: 1 0 Stereo: 0 1
Stereo FIFO Tests:
1 0 1 1 1 1 0 1 0 1 1 1 (0 1023)
VIA2 (1 $0000) 1
Idle IRQ 0 0 0 (0), 1 0 0 (1), 0 0 0
FIFO IRQ 1 0 0 0
(0 0), (0 0), (0 0), (0 0), 0
0 IRQs (start time = 2083):
Cancel 1 0 1 0 0 $0E 0 (HW)
Cancel 1 1 1 1 1 $0E 0
         |   | ----------------still in IFR after clear
         |   | -still in IFR after interrupts enabled (?)
         |-signaled in IFR befoe any samples were written
*/
void asc_iosb_device::sound_stream_update(sound_stream &stream)
{
	// printf("ASC-IOSB mode %d fl %02x cap A %x cap B %x\n", m_regs[R_MODE] & 3, m_regs[R_FIFOSTAT], m_fifo_cap[0], m_fifo_cap[1]);
	switch (m_regs[R_MODE])
	{
		case 0: // chip off
			for (int i = 0; i < stream.samples(); i++)
			{
				stream.put_int(0, i, m_last_left, 32768 / 64);
				stream.put_int(1, i, m_last_right, 32768 / 64);
			}
			// if ASC is off, signal half empty
			m_regs[R_FIFOSTAT] |= 4;
			break;

		case 1: // FIFO mode
		{
			// if we're in playback mode, FIFO A always reads empty
			if (!(m_regs[R_PLAYRECA] & 1))
			{
				m_regs[R_FIFOSTAT] |= 2;

				if (!(m_fifo_irqen[0] & 1))
				{
					set_irq_line(ASSERT_LINE);
				}
			}

			for (int i = 0; i < stream.samples(); i++)
			{
				const s8 smpll = (s8)m_fifo[0][m_fifo_rdptr[0]] ^ 0x80;
				const s8 smplr = (s8)m_fifo[1][m_fifo_rdptr[1]] ^ 0x80;

				m_last_left = smpll;
				m_last_right = smplr;

				// don't advance the sample pointer if there are no more samples
				if (m_fifo_cap[0])
				{
					m_fifo_rdptr[0]++;
					m_fifo_rdptr[0] &= 0x3ff;
					m_fifo_cap[0]--;
				}

				if (m_fifo_cap[1])
				{
					m_fifo_rdptr[1]++;
					m_fifo_rdptr[1] &= 0x3ff;
					m_fifo_cap[1]--;
				}

				if ((m_fifo_cap[0] < 0x200) || (m_fifo_cap[1] < 0x200))
				{
					m_regs[R_FIFOSTAT] |= STAT_HALF_FULL_B;
					if (!(m_fifo_irqen[1] & 1))
					{
						set_irq_line(ASSERT_LINE);
					}
				}
				else
				{
					m_regs[R_FIFOSTAT] &= ~STAT_HALF_FULL_B;
				}

				if ((m_fifo_cap[0] == 0x0) || (m_fifo_cap[1] == 0x0)) // either fifo empty
				{
					m_regs[R_FIFOSTAT] |= STAT_EMPTY_OR_FULL_B;
				}
				else
				{
					m_regs[R_FIFOSTAT] &= ~STAT_EMPTY_OR_FULL_B;
				}

				stream.put_int(0, i, smpll, 32768 / 64);
				stream.put_int(1, i, smplr, 32768 / 64);
			}
			break;
		}
	}

	//  printf("rdA %04x rdB %04x wrA %04x wrB %04x (capA %04x B %04x)\n", m_fifo_rdptr[0], m_fifo_rdptr[1], m_fifo_wrptr[0], m_fifo_wrptr[1], m_fifo_cap[0], m_fifo_cap[1]);
}

u8 asc_iosb_device::read(offs_t offset)
{
	switch (offset - 0x800)
	{
		// these registers always read as 0 on Sonora
		case R_CONTROL:
		case R_FIFOMODE:
		case R_WTCONTROL:
		case R_CLOCK:
		case R_BATMANCONTROL:
			return 0;

		case R_FIFOSTAT:
			// reading this clears interrupts, but not the FIFO status bits
			if (!machine().side_effects_disabled() && !(m_regs[R_FIFOSTAT] & STAT_HALF_FULL_B))
			{
				set_irq_line(CLEAR_LINE);
			}
			return m_regs[R_FIFOSTAT];

		case R_FIFOA_IRQCTRL:
			return m_fifo_irqen[0];

		case 0xf0e - 0x800:
			return 0x2c;

		case R_FIFOB_IRQCTRL:
			return m_fifo_irqen[1];

		case 0xf2e - 0x800:
			return 0x2c;

		default:
			break;
	}

	return asc_base_device::read(offset);
}

void asc_iosb_device::write(offs_t offset, u8 data)
{
//  if (offset >= 0x800) printf("ASC-IOSB write 8 %02x to offset %04x\n", data, offset);
	switch (offset - 0x800)
	{
		case R_MODE:
			if ((data & 1) != m_regs[R_MODE])
			{
				m_fifo_rdptr[0] = m_fifo_rdptr[1] = 0;
				m_fifo_wrptr[0] = m_fifo_wrptr[1] = 0;
				m_fifo_cap[0] = m_fifo_cap[1] = 0;
			}
			m_regs[R_MODE] = data & 1;  // only bit 0 can be written
			m_regs[R_FIFOSTAT] |= 0x8;  // signal playback FIFO empty
			return;

		// these registers are read-only or do nothing
		case R_CONTROL:
		case R_WTCONTROL:
		case R_CLOCK:
		case R_BATMANCONTROL:
			return;

		case R_FIFOA_IRQCTRL:
//          printf("FIFO A IRQ enable %d\n", data & 1);
			if (!(data & 1) && (m_fifo_irqen[0] & 1) && (m_regs[R_FIFOSTAT] & 0x1))
			{
//              printf("Firing dummy IRQ A\n");
				set_irq_line(ASSERT_LINE);
			}
			m_fifo_irqen[0] = data & 1;
			break;

		case R_FIFOB_IRQCTRL:
//          printf("FIFO B IRQ enable %d\n", data & 1);
			if (!(data & 1) && (m_fifo_irqen[1] & 1) && (m_regs[R_FIFOSTAT] & 0x4))
			{
//              printf("Firing dummy IRQ B\n");
				set_irq_line(ASSERT_LINE);
			}
			m_fifo_irqen[1] = data & 1;
			break;
	}

	asc_base_device::write(offset, data);

	// in playback mode, FIFO A is always empty
	if (offset < 0x400)
	{
		if (!(m_regs[R_PLAYRECA] & 1))
		{
			m_regs[R_FIFOSTAT] |= 2;
		}
	}
}

u16 asc_iosb_device::read_w(offs_t offset)
{
	if (offset < 0x800)
	{
		return ((m_fifo[0][m_fifo_rdptr[0]] ^ 0x80) << 8) | (m_fifo[1][m_fifo_rdptr[0]] ^ 0x80);
	}
	else
	{
		return (read(offset << 1) << 8) | read((offset << 1) + 1);
	}
}

void asc_iosb_device::write_w(offs_t offset, u16 data)
{
	//printf("write_w %04x @ %04x\n", data, offset);
	if (offset < 0x400)
	{
	//  write(0, ((data >> 8) & 0xff) ^ 0x80);
		write(1, (data & 0xff) ^ 0x80);
	}
	else if (offset < 0x800)
	{
	//  write(0x400, ((data >> 8) & 0xff) ^ 0x80);
		write(0x401, (data & 0xff) ^ 0x80);
	}
}

u8 asc_iosb_device::get_version()
{
	return 0xbb;
}

// asc_msc_device implementation - this is an exact V8 clone with a different version number based on what ASCTester reports.
asc_msc_device::asc_msc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: asc_v8_device(mconfig, ASC_MSC, tag, owner, clock)
{
}

/*
Mac PowerBook Duo 210 real hardware:
ASCTester test version 3
BoxFlag: 23   ASC Version: $E9   System 7.1.1
AddrMapFlags: $0000773F
F09: 0 ($00)  F29: 0 ($00)
804Idle: $03  M0: 0 M1: 1 M2: 0 ($01)
Mono: 1 1 Stereo: 0 0
Mono FIFO Tests:
0 0 1 0 1 0 1 0 1 0 1 0 (1022 0)
VIA2 (1 $00FF) 1
Idle IRQ 1 1 1 (50000), 0 0 0 (0), 0 0 0
FIFO IRQ 1 1 0 0
(0 0), (1236 1236), (50000 50000), (0 0), 0

MAME:
ASCTester test version 3
BoxFlag: 23   ASC Version: $E9   System 7.1.1
AddrMapFlags: $0000773F
F09: 0 ($01)  F29: 0 ($01)
804Idle: $03  M0: 0 M1: 1 M2: 0 ($01)
Mono: 1 1 Stereo: 0 0
Mono FIFO Tests:
0 0 1 0 1 0 1 0 1 0 1 0 (1105 0)
VIA2 (1 $0013) 1
Idle IRQ 1 0 0 (35868), 0 0 0 (0), 0 0 0
FIFO IRQ 1 1 0 0
(0 0), (510 7), (50000 715), (0 0), 0
*/

u8 asc_msc_device::get_version()
{
	return 0xe9;
}

// asc_easc_device implementation
asc_easc_device::asc_easc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: asc_base_device(mconfig, ASC_EASC, tag, owner, clock)
{
	m_src_step[0] = m_src_step[1] = 0;
	m_src_accum[0] = m_src_accum[1] = 0;
}

/*
Mac Quadra 700 real hardware:
ASCTester test version 3
BoxFlag: 16   ASC Version: $B0   System 7.6.1
AddrMapFlags: $05A0183F
F09: 1 ($01)  F29: 1 ($01)
804Idle: $0F  M0: 1 M1: 1 M2: 0 ($01)
Mono: 1 0 Stereo: 0 1
Stereo FIFO Tests:
0 0 1 1 1 1 1 1 1 1 1 1 (1103 1103)
VIA2 (1 $0000) 1
Idle IRQ 0 0 0 (0), 1 0 0 (1), 1 0 0
FIFO IRQ 1 0 0 0
(0 0), (1 1), (0 0), (0 0), 0

MAME:
ASCTester test version 3
BoxFlag: 16   ASC Version: $B0   System 7.6.1
AddrMapFlags: $05A0183F
F09: 1 ($01)  F29: 1 ($01)
804Idle: $0F  M0: 1 M1: 1 M2: 0 ($01)
Mono: 1 0 Stereo: 0 1
Stereo FIFO Tests:
0 0 1 1 1 1 1 1 1 1 1 1 (1075 1075)
VIA2 (1 $0000) 1
Idle IRQ 0 0 0 (0), 1 0 0 (1), 1 0 0
FIFO IRQ 1 0 0 0
(0 0), (1 1), (0 0), (0 0), 0
*/

void asc_easc_device::sound_stream_update(sound_stream &stream)
{
	switch (m_regs[R_MODE] & 3)
	{
		case 0: // chip off
			for (int i = 0; i < stream.samples(); i++)
			{
				stream.put_int(0, i, m_last_left, 32768);
				stream.put_int(1, i, m_last_right, 32768);
			}

			// if ASC is off, signal half empty
			m_regs[R_FIFOSTAT] |= 4;
			break;

		case 1: // FIFO mode
		{
			for (int i = 0; i < stream.samples(); i++)
			{
				s16 smpll = m_last_left;
				s16 smplr = m_last_right;

				int sample_skip = 1;
				if (BIT(m_regs[R_FIFOA_CTRL], 7))
				{
					m_src_accum[0] += m_src_step[0];
					sample_skip = m_src_accum[0] >> 16;
					if (sample_skip > 0)
					{
						m_src_accum[0] -= sample_skip << 16;
					}
				}

				for (int src = 0; src < sample_skip; src++)
				{
					if ((m_regs[R_FIFOA_CTRL] & 0x3) == 0)
					{
						smpll = (s16)(s8)(pop_fifo(0) ^ 0x80) << 8;
					}
					else
					{
						smpll = decode_cdxa(0, m_regs[R_FIFOA_CTRL] & 3);
					}
				}

				sample_skip = 1;
				if (BIT(m_regs[R_FIFOB_CTRL], 7))
				{
					m_src_accum[1] += m_src_step[1];
					sample_skip = m_src_accum[1] >> 16;
					if (sample_skip > 0)
					{
						m_src_accum[1] -= sample_skip << 16;
					}
				}

				for (int src = 0; src < sample_skip; src++)
				{
					if ((m_regs[R_FIFOB_CTRL] & 0x3) == 0)
					{
						smplr = (s16)(s8)(pop_fifo(1) ^ 0x80) << 8;
					}
					else
					{
						smplr = decode_cdxa(1, m_regs[R_FIFOB_CTRL] & 3);
					}
				}

				stream.put_int(0, i, smpll, 32768);
				stream.put_int(1, i, smplr, 32768);
				m_last_left = smpll;
				m_last_right = smplr;
			}
			break;
		}
	}

	//  printf("rdA %04x rdB %04x wrA %04x wrB %04x (capA %04x B %04x)\n", m_fifo_a_rdptr, m_fifo_b_rdptr, m_fifo_a_wrptr, m_fifo_b_wrptr, m_fifo_cap_a, m_fifo_cap_b);
}

u8 asc_easc_device::pop_fifo(int ch)
{
	static constexpr u8 stat_half[2] = { STAT_HALF_FULL_A, STAT_HALF_FULL_B };
	static constexpr u8 stat_empty[2] = { STAT_EMPTY_OR_FULL_A, STAT_EMPTY_OR_FULL_B };

	u8 sample = m_fifo[ch][m_fifo_rdptr[ch]];

	// we want the cap # from prior to the decrement so we don't trigger on the last sample in the FIFO
	const int cap = m_fifo_cap[ch];

	// don't advance the sample pointer if there are no more samples
	if (m_fifo_cap[ch])
	{
		m_fifo_rdptr[ch]++;
		m_fifo_rdptr[ch] &= 0x3ff;
		m_fifo_cap[ch]--;
	}

	if (cap <= 0x1ff)
	{
		m_regs[R_FIFOSTAT] |= stat_half[ch];
		if (!(m_fifo_irqen[ch] & 1))
		{
			set_irq_line(ASSERT_LINE);
		}
	}
	else
	{
		m_regs[R_FIFOSTAT] &= ~stat_half[ch];
	}

	if (cap == 0)
	{
		m_regs[R_FIFOSTAT] |= stat_empty[ch];
	}
	else
	{
		m_regs[R_FIFOSTAT] &= ~stat_empty[ch];
	}

	if (!m_fifo_cap[ch])
	{
		m_fifo_clrptr[ch]--;
		if (m_fifo_clrptr[ch] < 0)
		{
			m_fifo_clrptr[ch] = 0x400;
		}
	}

	return sample;
}

s16 asc_easc_device::decode_cdxa(int ch, u8 mode)
{
	const int reg_base = ch ? R_CDXA_B : R_CDXA_A;

	// Get the filter and shift selector at the start of each 28-byte block
	if (m_xa_pos[ch] == 0)
	{
		m_xa_param[ch] = pop_fifo(ch);
		m_xa_subpos[ch] = 0;
	}

	const int filter_idx = (m_xa_param[ch] >> 4) & 3;
	const int shift = std::min<int>(m_xa_param[ch] & 0xf, 12);
	const s8 K0 = s8(m_regs[reg_base + filter_idx * 2 + 1]);
	const s8 K1 = s8(m_regs[reg_base + filter_idx * 2 + 0]);

	// Get the raw sample (2, 4, or 8 bits) and shift it to the upper bits of a 16-bit temp
	s16 raw = 0;
	switch (mode)
	{
		case 1: // 8-bit ADPCM (2:1) — one byte per sample
			raw = s16(u16(pop_fifo(ch)) << 8);
			break;

		case 2: // 4-bit ADPCM (4:1) — two 4-bit samples per byte, low nibble first
			if (m_xa_subpos[ch] == 0)
			{
				m_xa_byte[ch] = pop_fifo(ch);
				raw = s16(u16(m_xa_byte[ch] & 0xf) << 12);
				m_xa_subpos[ch] = 1;
			}
			else
			{
				raw = s16(u16((m_xa_byte[ch] >> 4) & 0xf) << 12);
				m_xa_subpos[ch] = 0;
			}
			break;

		case 3: // 2-bit ADPCM (8:1) — four 2-bit samples per byte, LSB first
			if (m_xa_subpos[ch] == 0)
			{
				m_xa_byte[ch] = pop_fifo(ch);
			}
			raw = s16(u16((m_xa_byte[ch] >> (m_xa_subpos[ch] * 2)) & 0x3) << 14);
			m_xa_subpos[ch] = (m_xa_subpos[ch] + 1) & 3;
			break;
	}

	s32 sample32 = s32(raw) >> shift;
	sample32 += (s32(K0) * m_xa_s0[ch] + s32(K1) * m_xa_s1[ch] + 32) >> 6;
	const s16 sample = s16(std::clamp<s32>(sample32, -32768, 32767));
	m_xa_s1[ch] = m_xa_s0[ch];
	m_xa_s0[ch] = sample;

	if (++m_xa_pos[ch] >= 28)
	{
		m_xa_pos[ch] = 0;
	}
	return sample;
}

u8 asc_easc_device::read(offs_t offset)
{
	switch (offset - 0x800)
	{
	case R_FIFOSTAT:
		// reading this clears interrupts, but not the FIFO status bits
		if (!machine().side_effects_disabled() && !(m_regs[R_FIFOSTAT] & STAT_HALF_FULL_B))
		{
			set_irq_line(CLEAR_LINE);
		}
		return m_regs[R_FIFOSTAT];

	case R_CLOCK:
		// this register is read-only on EASC and shows what would be 44.1 kHz on the original ASC
		return 3;

	case R_FIFOA_IRQCTRL:
		return m_fifo_irqen[0];

	case R_FIFOB_IRQCTRL:
		return m_fifo_irqen[1];

	default:
		break;
	}

	return asc_base_device::read(offset);
}

void asc_easc_device::write(offs_t offset, u8 data)
{
	switch (offset - 0x800)
	{
		case R_MODE:
			if ((data & 1) != m_regs[R_MODE])
			{
				m_fifo_rdptr[0] = m_fifo_rdptr[1] = 0;
				m_fifo_wrptr[0] = m_fifo_wrptr[1] = 0;
				m_fifo_cap[0] = m_fifo_cap[1] = 0;
			}
			m_regs[R_MODE] = data & 1; // only bit 0 can be written
			m_regs[R_FIFOSTAT] |= 0x8; // signal playback FIFO empty
			return;

		case R_CONTROL:
			return;

		case R_FIFOA_IRQCTRL:
			if (!(data & 1) && (m_fifo_irqen[0] & 1) && (m_regs[R_FIFOSTAT] & STAT_HALF_FULL_A))
			{
				set_irq_line(ASSERT_LINE);
			}
			else if (data & 1)
			{
				set_irq_line(CLEAR_LINE);
			}
			m_fifo_irqen[0] = data & 1;
			break;

		case R_FIFOB_IRQCTRL:
			if (!(data & 1) && (m_fifo_irqen[1] & 1) && (m_regs[R_FIFOSTAT] & STAT_HALF_FULL_B))
			{
				set_irq_line(ASSERT_LINE);
			}
			else if (data & 1)
			{
				set_irq_line(CLEAR_LINE);
			}
			m_fifo_irqen[1] = data & 1;
			break;

		case R_SRCA_H:
		case R_SRCA_L:
			m_regs[offset - 0x800] = data;
			m_src_step[0] = ((m_regs[R_SRCA_H] << 8) | m_regs[R_SRCA_L]) + 1;
			return;

		case R_SRCB_H:
		case R_SRCB_L:
			m_regs[offset - 0x800] = data;
			m_src_step[1] = ((m_regs[R_SRCB_H] << 8) | m_regs[R_SRCB_L]) + 1;
			return;
		}

	asc_base_device::write(offset, data);
}

void asc_easc_device::device_start()
{
	m_sample_rate = 44100;
	asc_base_device::device_start();

	save_item(NAME(m_fifo_irqen));
	save_item(NAME(m_src_step));
	save_item(NAME(m_src_accum));
	save_item(NAME(m_xa_s0));
	save_item(NAME(m_xa_s1));
	save_item(NAME(m_xa_param));
	save_item(NAME(m_xa_pos));
	save_item(NAME(m_xa_byte));
	save_item(NAME(m_xa_subpos));
}

void asc_easc_device::device_reset()
{
	asc_base_device::device_reset();

	m_fifo_irqen[0] = m_fifo_irqen[1] = 1;

	// Reset CD-XA decoder state
	m_xa_s0[0] = m_xa_s0[1] = 0;
	m_xa_s1[0] = m_xa_s1[1] = 0;
	m_xa_param[0] = m_xa_param[1] = 0;
	m_xa_pos[0] = m_xa_pos[1] = 0;
	m_xa_byte[0] = m_xa_byte[1] = 0;
	m_xa_subpos[0] = m_xa_subpos[1] = 0;
}

u8 asc_easc_device::get_version()
{
	return 0xb0;
}
