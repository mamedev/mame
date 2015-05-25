// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    asc.c

    Apple Sound Chip (ASC) 344S0063
    Enhanced Apple Sound Chip (EASC) 343S1063

    Emulation by R. Belmont

    Registers:
    0x800: VERSION
    0x801: MODE (1=FIFO mode, 2=wavetable mode)
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

***************************************************************************/

#include "emu.h"
#include "asc.h"

// device type definition
const device_type ASC = &device_creator<asc_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  asc_device - constructor
//-------------------------------------------------

asc_device::asc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ASC, "ASC", tag, owner, clock, "asc", __FILE__),
		device_sound_interface(mconfig, *this),
		write_irq(*this),
		m_chip_type(0)
{
}


//-------------------------------------------------
//  static_set_type - configuration helper to set
//  the chip type
//-------------------------------------------------

void asc_device::static_set_type(device_t &device, int type)
{
	asc_device &asc = downcast<asc_device &>(device);
	asc.m_chip_type = type;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void asc_device::device_start()
{
	// create the stream
	m_stream = machine().sound().stream_alloc(*this, 0, 2, 22257);

	memset(m_regs, 0, sizeof(m_regs));

	m_timer = timer_alloc(0, NULL);

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

	write_irq.resolve_safe();
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
//  device_timer - called when our device timer expires
//-------------------------------------------------

void asc_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	m_stream->update();
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void asc_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *outL, *outR;
	int i, ch;
	static UINT32 wtoffs[2] = { 0, 0x200 };

	outL = outputs[0];
	outR = outputs[1];

	switch (m_regs[R_MODE-0x800] & 3)
	{
		case 0: // chip off
			for (i = 0; i < samples; i++)
			{
				outL[i] = outR[i] = 0;
			}
			break;

		case 1: // FIFO mode
			for (i = 0; i < samples; i++)
			{
				INT8 smpll, smplr;

				smpll = (INT8)m_fifo_a[m_fifo_a_rdptr]^0x80;
				smplr = (INT8)m_fifo_b[m_fifo_b_rdptr]^0x80;

				// don't advance the sample pointer if there are no more samples
				if (m_fifo_cap_a)
				{
					m_fifo_a_rdptr++;
					m_fifo_a_rdptr &= 0x3ff;
					m_fifo_cap_a--;
				}

				if (m_fifo_cap_b)
				{
					m_fifo_b_rdptr++;
					m_fifo_b_rdptr &= 0x3ff;
					m_fifo_cap_b--;
				}

				switch (m_chip_type)
				{
					case ASC_TYPE_SONORA:
						if (m_fifo_cap_a < 0x200)
						{
							m_regs[R_FIFOSTAT-0x800] |= 0x4;    // fifo less than half full
							m_regs[R_FIFOSTAT-0x800] |= 0x8;    // just pass the damn test
							write_irq(ASSERT_LINE);
						}
						break;

					default:
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
				}

				outL[i] = smpll * 64;
				outR[i] = smplr * 64;
			}
			break;

		case 2: // wavetable mode
			for (i = 0; i < samples; i++)
			{
				INT32 mixL, mixR;
				INT8 smpl;

				mixL = mixR = 0;

				// update channel pointers
				for (ch = 0; ch < 4; ch++)
				{
					m_phase[ch] += m_incr[ch];

					if (ch < 2)
					{
						smpl = (INT8)m_fifo_a[((m_phase[ch]>>15)&0x1ff) + wtoffs[ch&1]];
					}
					else
					{
						smpl = (INT8)m_fifo_b[((m_phase[ch]>>15)&0x1ff) + wtoffs[ch&1]];
					}

					smpl ^= 0x80;
					mixL += smpl*256;
					mixR += smpl*256;
				}

				outL[i] = mixL>>2;
				outR[i] = mixR>>2;
			}
			break;
	}

//  printf("rdA %04x rdB %04x wrA %04x wrB %04x (capA %04x B %04x)\n", m_fifo_a_rdptr, m_fifo_b_rdptr, m_fifo_a_wrptr, m_fifo_b_wrptr, m_fifo_cap_a, m_fifo_cap_b);
}

//-------------------------------------------------
//  read - read from the chip's registers and internal RAM
//-------------------------------------------------

READ8_MEMBER( asc_device::read )
{
	UINT8 rv;

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
					case ASC_TYPE_ASC:
						return 0;

					case ASC_TYPE_V8:
					case ASC_TYPE_EAGLE:
					case ASC_TYPE_SPICE:
					case ASC_TYPE_VASP:
						return 0xe8;

					case ASC_TYPE_SONORA:
						return 0xbc;

					default:    // return the actual register value
						break;
				}
				break;

			case R_MODE:
				switch (m_chip_type)
				{
					case ASC_TYPE_V8:
					case ASC_TYPE_EAGLE:
					case ASC_TYPE_SPICE:
					case ASC_TYPE_VASP:
						return 1;

					default:
						break;
				}
				break;

			case R_CONTROL:
				switch (m_chip_type)
				{
					case ASC_TYPE_V8:
					case ASC_TYPE_EAGLE:
					case ASC_TYPE_SPICE:
					case ASC_TYPE_VASP:
						return 1;

					default:
						break;
				}
				break;

			case R_FIFOSTAT:
				if (m_chip_type == ASC_TYPE_V8)
				{
					rv = 3;
				}
				else
				{
					rv = m_regs[R_FIFOSTAT-0x800];
				}

//              printf("Read FIFO stat = %02x\n", rv);

				// reading this register clears all bits
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
		m_regs[0x11] = m_phase[0]>>16;
		m_regs[0x12] = m_phase[0]>>8;
		m_regs[0x13] = m_phase[0];
		m_regs[0x15] = m_incr[0]>>16;
		m_regs[0x16] = m_incr[0]>>8;
		m_regs[0x17] = m_incr[0];

		m_regs[0x19] = m_phase[1]>>16;
		m_regs[0x1a] = m_phase[1]>>8;
		m_regs[0x1b] = m_phase[1];
		m_regs[0x1d] = m_incr[1]>>16;
		m_regs[0x1e] = m_incr[1]>>8;
		m_regs[0x1f] = m_incr[1];

		m_regs[0x21] = m_phase[2]>>16;
		m_regs[0x22] = m_phase[2]>>8;
		m_regs[0x23] = m_phase[2];
		m_regs[0x25] = m_incr[2]>>16;
		m_regs[0x26] = m_incr[2]>>8;
		m_regs[0x27] = m_incr[2];

		m_regs[0x29] = m_phase[3]>>16;
		m_regs[0x2a] = m_phase[3]>>8;
		m_regs[0x2b] = m_phase[3];
		m_regs[0x2d] = m_incr[3]>>16;
		m_regs[0x2e] = m_incr[3]>>8;
		m_regs[0x2f] = m_incr[3];
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

WRITE8_MEMBER( asc_device::write )
{
//  printf("ASC: write %02x to %x\n", data, offset);

	if (offset < 0x400)
	{
		if (m_regs[R_MODE-0x800] == 1)
		{
			m_fifo_a[m_fifo_a_wrptr++] = data;
			m_fifo_cap_a++;

			if (m_fifo_cap_a == 0x3ff)
			{
				m_regs[R_FIFOSTAT-0x800] |= 2;  // fifo A full
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

			if (m_fifo_cap_b == 0x3ff)
			{
				m_regs[R_FIFOSTAT-0x800] |= 8;  // fifo B full
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
