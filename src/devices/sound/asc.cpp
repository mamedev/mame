// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    asc.c

    Apple Sound Chip (ASC) 344S0053 (original), 344S0063 (cost-reduced)
    Enhanced Apple Sound Chip (EASC) 343S1036

    Emulation by R. Belmont

    The four-voice wavetable mode is unique to the first-generation ASC.
    EASC (which was codenamed "Batman") and other ASC clones remove it
    entirely.

    Registers:
    0x800: VERSION
    0x801: MODE (0=inactive, 1=FIFO mode, 2=wavetable mode)
    0x802: CONTROL (bit 0=analog or PWM output, 1=stereo/mono, 7=processing time exceeded)
    0x803: FIFO MODE (bit 7=clear FIFO, bit 1="non-ROM companding", bit 0="ROM companding")
    0x804: FIFO IRQ STATUS (bit 0=ch A 1/2 full, 1=ch A full, 2=ch B 1/2 full, 3=ch B full)
    0x805: WAVETABLE CONTROL (bits 0-3 wavetables 0-3 start)
    0x806: VOLUME (bits 2-4 = 3 bit internal ASC volume, bits 5-7 = volume control sent to Sony sound chip)
    0x807: CLOCK RATE (0 = Mac 22257 Hz, 1 = undefined, 2 = 22050 Hz, 3 = 44100 Hz)
    0x80a: PLAY REC A
    0x80f: TEST (bits 6-7 = digital test, bits 4-5 = analog test)
    0x810: WAVETABLE 0 PHASE (big-endian 9.15 fixed-point, only 24 bits valid)
    0x814: WAVETABLE 0 INCREMENT (big-endian 9.15 fixed-point, only 24 bits valid)
    0x818: WAVETABLE 1 PHASE
    0x81C: WAVETABLE 1 INCREMENT
    0x820: WAVETABLE 2 PHASE
    0x824: WAVETABLE 2 INCREMENT
    0x828: WAVETABLE 3 PHASE
    0x82C: WAVETABLE 3 INCREMENT

    TODO: rewrite this, we know so much more now and the "chip variant type" pattern must die.

***************************************************************************/

#include "emu.h"
#include "asc.h"

#include "multibyte.h"

// device type definition
DEFINE_DEVICE_TYPE(ASC, asc_device, "asc", "ASC")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  asc_device - constructor
//-------------------------------------------------

asc_device::asc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ASC, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, write_irq(*this)
	, m_chip_type(asc_type::ASC)
	, m_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void asc_device::device_start()
{
	// create the stream
	m_stream = stream_alloc(0, 2, 22257);

	memset(m_regs, 0, sizeof(m_regs));

	m_timer = timer_alloc(FUNC(asc_device::delayed_stream_update), this);

	save_item(NAME(m_fifo_a_rdptr));
	save_item(NAME(m_fifo_b_rdptr));
	save_item(NAME(m_fifo_a_wrptr));
	save_item(NAME(m_fifo_b_wrptr));
	save_item(NAME(m_fifo_cap_a));
	save_item(NAME(m_fifo_cap_b));
	save_item(NAME(m_fifo_a));
	save_item(NAME(m_fifo_b));
	save_item(NAME(m_regs));
	save_item(NAME(m_phase));
	save_item(NAME(m_incr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void asc_device::device_reset()
{
	m_stream->update();

	memset(m_regs, 0, sizeof(m_regs));
	memset(m_fifo_a, 0, sizeof(m_fifo_a));
	memset(m_fifo_b, 0, sizeof(m_fifo_b));
	memset(m_phase, 0, sizeof(m_phase));
	memset(m_incr, 0, sizeof(m_incr));

	m_fifo_a_rdptr = m_fifo_b_rdptr = 0;
	m_fifo_a_wrptr = m_fifo_b_wrptr = 0;
	m_fifo_cap_a = m_fifo_cap_b = 0;
}

//-------------------------------------------------
//  delayed_stream_update -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(asc_device::delayed_stream_update)
{
	m_stream->update();
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void asc_device::sound_stream_update(sound_stream &stream)
{
	int i, ch;
	static uint32_t wtoffs[2] = { 0, 0x200 };

	switch (m_regs[R_MODE-0x800] & 3)
	{
		case 0: // chip off
			// IIvx/IIvi bootrom indicates VASP updates this flag even when the chip is off
			if (m_chip_type == asc_type::VASP)
			{
				if (m_fifo_cap_a < 0x1ff)
				{
					m_regs[R_FIFOSTAT-0x800] |= 1;  // fifo A less than half full

					if (m_fifo_cap_a == 0)   // fifo A fully empty
					{
						m_regs[R_FIFOSTAT-0x800] |= 2;  // fifo A empty
					}
				}
			}
			break;

		case 1: // FIFO mode
		{
			for (i = 0; i < stream.samples(); i++)
			{
				int8_t smpll, smplr;

				smpll = (int8_t)m_fifo_a[m_fifo_a_rdptr]^0x80;
				if ((m_chip_type <= asc_type::EASC) || (m_chip_type == asc_type::SONORA))
				{
					smplr = (int8_t)m_fifo_b[m_fifo_b_rdptr]^0x80;
				}
				else
				{
					smplr = smpll;
				}

				// don't advance the sample pointer if there are no more samples
				if (m_fifo_cap_a)
				{
					m_fifo_a_rdptr++;
					m_fifo_a_rdptr &= 0x3ff;
					m_fifo_cap_a--;
				}

				if ((m_fifo_cap_b) && ((m_chip_type <= asc_type::EASC) || (m_chip_type == asc_type::SONORA)))
				{
					m_fifo_b_rdptr++;
					m_fifo_b_rdptr &= 0x3ff;
					m_fifo_cap_b--;
				}

				//printf("chip updating: cap A %x cap B %x\n", m_fifo_cap_a, m_fifo_cap_b);
				switch (m_chip_type)
				{
					case asc_type::ASC:
					case asc_type::EASC:
						if (m_fifo_cap_a == 0x1ff)
						{
							m_regs[R_FIFOSTAT-0x800] |= 1;  // fifo A half-empty
							write_irq(ASSERT_LINE);
						}
						else if (m_fifo_cap_a == 0x1)   // fifo A fully empty
						{
							m_regs[R_FIFOSTAT-0x800] |= 2;  // fifo A empty
							write_irq(ASSERT_LINE);
						}

						if (m_fifo_cap_b == 0x1ff)
						{
							m_regs[R_FIFOSTAT-0x800] |= 4;  // fifo B half-empty
							write_irq(ASSERT_LINE);
						}
						else if (m_fifo_cap_b == 0x1)   // fifo B fully empty
						{
							m_regs[R_FIFOSTAT-0x800] |= 8;  // fifo B empty
							write_irq(ASSERT_LINE);
						}
						break;

					// Sonora sets the 1/2 full flag continuously, ASC/EASC only does it when it happens
					case asc_type::SONORA:
					case asc_type::ARDBEG:
						if (m_fifo_cap_a <= 0x200)
						{
							m_regs[R_FIFOSTAT-0x800] |= 1;  // fifo A less than half full

							if (m_fifo_cap_a == 0)   // fifo A fully empty
							{
								m_regs[R_FIFOSTAT-0x800] |= 2;  // fifo A empty
							}
							if (!(m_regs[0xf09 - 0x800] & 1))
							{
								write_irq(ASSERT_LINE);
							}
						}

						if (m_fifo_cap_b <= 0x200)
						{
							m_regs[R_FIFOSTAT-0x800] |= 4;  // fifo B less than half full

							if (m_fifo_cap_b == 0)   // fifo B fully empty
							{
								m_regs[R_FIFOSTAT-0x800] |= 8;  // fifo B empty
							}
							if (!(m_regs[0xf29 - 0x800] & 1))
							{
								write_irq(ASSERT_LINE);
							}
						}
						break;

					default:    // V8/Eagle/etc
						if (m_fifo_cap_a < 0x1ff)
						{
							m_regs[R_FIFOSTAT-0x800] |= 1;  // fifo A less than half full

							if (m_fifo_cap_a == 0)   // fifo A fully empty
							{
								m_regs[R_FIFOSTAT-0x800] |= 2;  // fifo A empty
							}
							write_irq(ASSERT_LINE);
						}
						break;
				}

				stream.put_int(0, i, smpll, 32768 / 64);
				stream.put_int(1, i, smplr, 32768 / 64);
			}
			break;
		}

		case 2: // wavetable mode
		{
			for (i = 0; i < stream.samples(); i++)
			{
				int32_t mixL, mixR;
				int8_t smpl;

				mixL = mixR = 0;

				// update channel pointers
				for (ch = 0; ch < 4; ch++)
				{
					m_phase[ch] += m_incr[ch];

					if (ch < 2)
					{
						smpl = (int8_t)m_fifo_a[((m_phase[ch]>>15)&0x1ff) + wtoffs[ch&1]];
					}
					else
					{
						smpl = (int8_t)m_fifo_b[((m_phase[ch]>>15)&0x1ff) + wtoffs[ch&1]];
					}

					smpl ^= 0x80;
					mixL += smpl*256;
					mixR += smpl*256;
				}

				stream.put_int(0, i, mixL, 32768 * 4);
				stream.put_int(1, i, mixR, 32768 * 4);
			}
			break;
		}
	}

//  printf("rdA %04x rdB %04x wrA %04x wrB %04x (capA %04x B %04x)\n", m_fifo_a_rdptr, m_fifo_b_rdptr, m_fifo_a_wrptr, m_fifo_b_wrptr, m_fifo_cap_a, m_fifo_cap_b);
}

//-------------------------------------------------
//  read - read from the chip's registers and internal RAM
//-------------------------------------------------

uint8_t asc_device::read(offs_t offset)
{
	uint8_t rv;

//  printf("ASC: read at %x\n", offset);

	// not sure what actually happens when the CPU reads the FIFO...
	if (offset < 0x400)
	{
		return m_fifo_a[offset];
	}
	else if (offset < 0x800)
	{
		return m_fifo_b[offset-0x400];
	}
	else
	{
		m_stream->update();
		switch (offset)
		{
			case R_VERSION:
				switch (m_chip_type)
				{
					case asc_type::ASC:
						return 0;

					case asc_type::EAGLE:
						return 0xe0;

					case asc_type::V8:
					case asc_type::SPICE:
					case asc_type::VASP:
						return 0xe8;

					case asc_type::SONORA:
						return 0xbc;

					default:    // return the actual register value
						break;
				}
				break;

			case R_MODE:
				switch (m_chip_type)
				{
					case asc_type::V8:
					case asc_type::EAGLE:
					case asc_type::SPICE:
					case asc_type::VASP:
					case asc_type::SONORA:
						return 1;

					default:
						break;
				}
				break;

			case R_CONTROL:
				switch (m_chip_type)
				{
					case asc_type::V8:
					case asc_type::EAGLE:
					case asc_type::SPICE:
					case asc_type::VASP:
					case asc_type::SONORA:
						return 1;

					default:
						break;
				}
				break;

			case R_FIFOSTAT:
				switch (m_chip_type)
				{
					case asc_type::V8:
					case asc_type::EAGLE:
					case asc_type::SPICE:
					case asc_type::VASP:
						rv = m_regs[R_FIFOSTAT-0x800] & 3;
						break;

					default:
						rv = m_regs[R_FIFOSTAT-0x800];
						break;
				}

				//if (rv != 0) printf("Read FIFO stat = %02x\n", rv);

				// reading this register clears all bits (true also on V8/EAGLE?)
				m_regs[R_FIFOSTAT-0x800] = 0;

				// reading this clears interrupts
				write_irq(CLEAR_LINE);

				return rv;

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

	return m_regs[offset-0x800];
}

//-------------------------------------------------
//  write - write to the chip's registers and internal RAM
//-------------------------------------------------

void asc_device::write(offs_t offset, uint8_t data)
{
  //printf("ASC: write %02x to %x\n", data, offset);

	if (offset < 0x400)
	{
		if (m_regs[R_MODE-0x800] == 1)
		{
			m_fifo_a[m_fifo_a_wrptr++] = data;
			m_fifo_cap_a++;

			if (m_fifo_cap_a == 0x400)
			{
				m_regs[R_FIFOSTAT-0x800] |= 2;  // fifo A full
			}

			if (m_fifo_cap_a > 0x200)
			{
				m_regs[R_FIFOSTAT-0x800] &= ~1;
			}

			m_fifo_a_wrptr &= 0x3ff;
		}
		else
		{
			m_fifo_a[offset] = data;
		}
	}
	else if (offset < 0x800)
	{
		if (m_regs[R_MODE-0x800] == 1)
		{
			m_fifo_b[m_fifo_b_wrptr++] = data;
			m_fifo_cap_b++;

			if (m_fifo_cap_b == 0x400)
			{
				m_regs[R_FIFOSTAT-0x800] |= 8;  // fifo B full
			}

			if (m_fifo_cap_b > 0x200)
			{
				m_regs[R_FIFOSTAT-0x800] &= ~4;
			}

			m_fifo_b_wrptr &= 0x3ff;
		}
		else
		{
			m_fifo_b[offset-0x400] = data;
		}
	}
	else
	{
//      printf("ASC: %02x to %x (was %x)\n", data, offset, m_regs[offset-0x800]);

		m_stream->update();
		switch (offset)
		{
			case R_MODE:
				data &= 3;  // only bits 0 and 1 can be written

				if (data != m_regs[R_MODE-0x800])
				{
					m_fifo_a_rdptr = m_fifo_b_rdptr = 0;
					m_fifo_a_wrptr = m_fifo_b_wrptr = 0;
					m_fifo_cap_a = m_fifo_cap_b = 0;

					if (data != 0)
					{
						m_timer->adjust(attotime::zero, 0, attotime::from_hz(22257/4));
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
					m_fifo_a_rdptr = m_fifo_b_rdptr = 0;
					m_fifo_a_wrptr = m_fifo_b_wrptr = 0;
					m_fifo_cap_a = m_fifo_cap_b = 0;
					m_regs[R_FIFOSTAT-0x800] |= 0xa;  // fifos A&B empty
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
