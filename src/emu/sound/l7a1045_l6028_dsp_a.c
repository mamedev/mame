/***************************************************************************

    L7A1045 L6028 DSP-A
	(QFP120 package)

	this is the audio chip used on the following
	SNK Hyper NeoGeo 64 (arcade platform)
	AKAI MPC3000 (synth)

	both are driven by a V53, the MPC3000 isn't dumped.

	appears to write a register number and channel/voice using
	l7a1045_sound_select_w (offset 0)
	format:

	---- rrrr ---c cccc
	r = register, c = channel

	the channel select appears to address 32 different voices (5-bits)
	the register select appears to use 4-bits with 0x0 to 0xa being valid

	the registers data is written / read using offsets 1,2,3 after
	setting the register + channel, this gives 3 16-bit values for
	each register.

	register format:

	   offset 3           offset 2           offset 1
	   fedcba9876543210 | fedcba9876543210 | fedcba9876543210

	0  ----------------   ----------------   ----------------

	1  ----------------   ----------------   ----------------

	2  ----------------   ----------------   ----------------

	3  ----------------   ----------------   ----------------

	4  ----------------   ----------------   ----------------

	5  ----------------   ----------------   ----------------

	6  ----------------   ----------------   ----------------

	7  ----------------   ----------------   llllllllrrrrrrrr left/right volume

	8  ----------------   ----------------   ---------------- (read only?)

	9  ----------------   ----------------   ---------------- (read only?)

	a  ----------------   ----------------   ----------------

	Registers are not yet understood.

	probably sample start, end, loop positions, panning etc.
	like CPS3, Qsound etc.

	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03: // 00003fffffff (startup only?)
	case 0x04: // doesn't use 6
	case 0x05: // 00003fffffff (mostly, often)
	case 0x06: // 00007ff0ffff mostly
	case 0x07: // 0000000f0708 etc. (low values)
	case 0x08: // doesn't write to 2/4/6 with this set??
	case 0x09: // doesn't write to 2/4/6 with this set??
	case 0x0a: // random looking values

	Some of the other ports on the HNG64 sound CPU may also be tied
	to this chip, this isn't yet clear.
	Port $8 bit 8 is keyon, low byte is sound status related (masked with 0x7f)

	Sample data format TBA

***************************************************************************/

#include "emu.h"
#include "l7a1045_l6028_dsp_a.h"


// device type definition
const device_type L7A1045 = &device_creator<l7a1045_sound_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  l7a1045_sound_device - constructor
//-------------------------------------------------

l7a1045_sound_device::l7a1045_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, L7A1045, "L7A1045 L6028 DSP-A", tag, owner, clock, "l7a1045_custom", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(NULL),
		m_key(0),
		m_rom(NULL),
		m_rom_size(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void l7a1045_sound_device::device_start()
{
	/* Allocate the stream */
	m_stream = stream_alloc(0, 2, 44100/4); //clock() / 384);

	m_rom = m_region->base();
	m_rom_size = m_region->bytes();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void l7a1045_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	/* Clear the buffers */
	memset(outputs[0], 0, samples*sizeof(*outputs[0]));
	memset(outputs[1], 0, samples*sizeof(*outputs[1]));

	for (int i = 0; i < 32; i++)
	{
		if (m_key & (1 << i))
		{
			l7a1045_voice *vptr = &m_voice[i];

			UINT32 start = vptr->start;
			UINT32 end = vptr->start+0x002000;
			UINT32 step  = 0x0400;

			UINT32 pos = vptr->pos;
			UINT32 frac = vptr->frac;

			for (int j = 0; j < samples; j++)
			{
				INT32 sample;

				pos += 1;//(frac >> 12);
				frac &= 0xfff;

				if ((start + pos) >= end)
				{
					m_key &= ~(1 << i);

				}

				sample = (INT8)m_rom[(start + pos) & (m_rom_size-1)];
				frac += step;

				outputs[0][j] += ((sample * vptr->l_volume) >> 8);
				outputs[1][j] += ((sample * vptr->r_volume) >> 8);
			}

			vptr->pos = pos;
			vptr->frac = frac;
		}
	}
}

// TODO: needs proper memory map
WRITE16_MEMBER( l7a1045_sound_device::l7a1045_sound_w )
{
	m_stream->update(); // TODO

	if(offset == 0)
		sound_select_w(space, offset, data, mem_mask);
	else if(offset == 8/2)
		sound_status_w(space, offset, data, mem_mask);
	else
		sound_data_w(space,offset - 1,data,mem_mask);
}


READ16_MEMBER( l7a1045_sound_device::l7a1045_sound_r )
{
	m_stream->update();

	switch (offset)
	{
	case 0x00:
	case 0x01:
		printf("%08x: l7a1045_sound_r unknown offset %02x\n", space.device().safe_pc(), offset * 2);
		return 0x0000;

	case 0x02: return l7a1045_sound_port_0004_r(space, offset, mem_mask);
	case 0x03: return l7a1045_sound_port_0006_r(space, offset, mem_mask);
	}
	return 0x000;
}


WRITE16_MEMBER(l7a1045_sound_device::sound_select_w)
{
	// I'm guessing these addresses are the sound chip / DSP?

	// ---- ---- 000c cccc
	// c = channel

	if (ACCESSING_BITS_0_7)
	{
		m_audiochannel = data;
		if (m_audiochannel & 0xe0) printf("%08x: l7a1045_sound_select_w unknown channel %01x\n", space.device().safe_pc(), m_audiochannel & 0xff);
		m_audiochannel &= 0x1f;
	}

	if (ACCESSING_BITS_8_15)
	{
		m_audioregister = (data >> 8);
		if (m_audioregister >0x0a) printf("%08x: l7a1045_sound_select_w unknown register %01x\n", space.device().safe_pc(), m_audioregister & 0xff);
		m_audioregister &= 0x0f;
	}

}

WRITE16_MEMBER(l7a1045_sound_device::sound_data_w)
{
	l7a1045_voice *vptr = &m_voice[m_audiochannel];

	if(m_audioregister != 0)
		printf("%04x %04x (%04x %04x)\n",offset,data,m_audioregister,m_audiochannel);

	m_audiodat[m_audioregister][m_audiochannel].dat[offset] = data;

	switch (m_audioregister)
	{
		case 0x00:

			vptr->start = (m_audiodat[0][m_audiochannel].dat[2] & 0x000f) << (16 + 4);
			vptr->start |= (m_audiodat[0][m_audiochannel].dat[1] & 0xffff) << (4);
			vptr->start |= (m_audiodat[0][m_audiochannel].dat[0] & 0xf000) >> (12);

			vptr->start &= m_rom_size - 1;

			//printf("%08x: REGISTER 00 write port 0x0002 chansel %02x data %04x (%04x%04x%04x)\n", space.device().safe_pc(), m_audiochannel, data, m_audiodat[m_audioregister][m_audiochannel].dat[0], m_audiodat[m_audioregister][m_audiochannel].dat[1], m_audiodat[m_audioregister][m_audiochannel].dat[2]);
			break;

		case 0x07:

			vptr->r_volume = (m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xff);
			/* TODO: volume tables, linear? */
			vptr->r_volume = (vptr->r_volume) | (vptr->r_volume << 8);
			vptr->l_volume = (m_audiodat[m_audioregister][m_audiochannel].dat[0] >> 8) & 0xff;
			vptr->l_volume = (vptr->l_volume) | (vptr->l_volume << 8);
			//printf("%04x %02x %02x\n",m_audiodat[m_audioregister][m_audiochannel].dat[0],vptr->l_volume,vptr->r_volume);

			break;
	}
}

WRITE16_MEMBER(l7a1045_sound_device::sound_status_w)
{
	if(data & 0x100) // keyin
	{
		l7a1045_voice *vptr = &m_voice[m_audiochannel];

		vptr->frac = 0;
		vptr->pos = 0;
		m_key |= 1 << m_audiochannel;
	}
}

READ16_MEMBER(l7a1045_sound_device::l7a1045_sound_port_0004_r)
{
	// it writes the channel select before reading this.. so either it works on channels, or the command..

	// buriki reads registers 03/05/00 these at the moment, others don't
	// also reads 06

	switch (m_audioregister)
	{
	default:

	case 0x01:
	case 0x02:
	case 0x04:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
		printf("%08x: unexpected read port 0x0004 register %02x chansel %02x (%04x%04x%04x)\n", space.device().safe_pc(), m_audioregister, m_audiochannel, m_audiodat[m_audioregister][m_audiochannel].dat[0], m_audiodat[m_audioregister][m_audiochannel].dat[1], m_audiodat[m_audioregister][m_audiochannel].dat[2]);
		break;

	case 0x03:
	case 0x05:
	case 0x00:
	case 0x06:
		//printf("%08x: read port 0x0004 register %02x chansel %02x (%04x%04x%04x)\n", space.device().safe_pc(), m_audioregister, m_audiochannel, m_audiodat[m_audioregister][m_audiochannel].dat[0], m_audiodat[m_audioregister][m_audiochannel].dat[1], m_audiodat[m_audioregister][m_audiochannel].dat[2]);
		break;


	}
	return 0;
}

READ16_MEMBER(l7a1045_sound_device::l7a1045_sound_port_0006_r)
{
	// it writes the channel select before reading this.. so either it works on channels, or the command..

	// buriki reads register 00

	switch (m_audioregister)
	{
	default:

	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
		printf("%08x: unexpected read port 0x0006 register %02x chansel %02x (%04x%04x%04x)\n", space.device().safe_pc(), m_audioregister, m_audiochannel, m_audiodat[m_audioregister][m_audiochannel].dat[0], m_audiodat[m_audioregister][m_audiochannel].dat[1], m_audiodat[m_audioregister][m_audiochannel].dat[2]);
		break;

	case 0x00:
		//printf("%08x: read port 0x0006 register %02x chansel %02x (%04x%04x%04x)\n", space.device().safe_pc(), m_audioregister, m_audiochannel, m_audiodat[m_audioregister][m_audiochannel].dat[0], m_audiodat[m_audioregister][m_audiochannel].dat[1], m_audiodat[m_audioregister][m_audiochannel].dat[2]);
		break;
	}
	return rand();
}
