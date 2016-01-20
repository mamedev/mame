// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi
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

    TODO:
    - Sample format needs to be double checked;
    - Octave Control/BPM/Pitch, right now XRally Network BGM wants 66150 Hz which is definitely too fast for Terry Bogard speech;
    - Key Off;
    - ADSR (registers 2 & 4?);

***************************************************************************/

#include "emu.h"
#include "l7a1045_l6028_dsp_a.h"
#include "debugger.h"


// device type definition
const device_type L7A1045 = &device_creator<l7a1045_sound_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  l7a1045_sound_device - constructor
//-------------------------------------------------

l7a1045_sound_device::l7a1045_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, L7A1045, "L7A1045 L6028 DSP-A", tag, owner, clock, "l7a1045_custom", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_key(0),
		m_rom(nullptr),
		m_rom_size(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void l7a1045_sound_device::device_start()
{
	/* Allocate the stream */
	m_stream = stream_alloc(0, 2, 66150); //clock() / 384);

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
			UINT32 end = vptr->end;
			UINT32 step  = 0x400;

			UINT32 pos = vptr->pos;
			UINT32 frac = vptr->frac;

			for (int j = 0; j < samples; j++)
			{
				INT32 sample;
				UINT8 data;

				pos += (frac >> 12);
				frac &= 0xfff;

				if ((start + pos) >= end)
				{
					if(vptr->mode == true) // loop
					{
						pos = vptr->pos = 0;
						frac = vptr->frac = 0;
					}
					else // no loop, keyoff
					{
						m_key &= ~(1 << i);
						break;
					}
				}


				data = m_rom[(start + pos) & (m_rom_size-1)];
				sample = ((INT8)(data & 0xfc)) << (3 - (data & 3));
				frac += step;

				outputs[0][j] += ((sample * vptr->l_volume) >> 9);
				outputs[1][j] += ((sample * vptr->r_volume) >> 9);
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

	if(offset == 0)
		printf("sound_select_r?\n");
	else
		return sound_data_r(space,offset -1,mem_mask);

	return 0xffff;
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

	//if(m_audioregister != 0 && m_audioregister != 1 && m_audioregister != 7)
	//  printf("%04x %04x (%04x %04x)\n",offset,data,m_audioregister,m_audiochannel);

	m_audiodat[m_audioregister][m_audiochannel].dat[offset] = data;

	switch (m_audioregister)
	{
		case 0x00:

			vptr->start = (m_audiodat[m_audioregister][m_audiochannel].dat[2] & 0x000f) << (16 + 4);
			vptr->start |= (m_audiodat[m_audioregister][m_audiochannel].dat[1] & 0xffff) << (4);
			vptr->start |= (m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xf000) >> (12);

			vptr->start &= m_rom_size - 1;

			break;
		case 0x01:
			// relative to start
				//printf("%04x\n",m_audiodat[m_audioregister][m_audiochannel].dat[0]);
				//printf("%04x\n",m_audiodat[m_audioregister][m_audiochannel].dat[1]);
				//printf("%04x\n",m_audiodat[m_audioregister][m_audiochannel].dat[2]);

			if(m_audiodat[m_audioregister][m_audiochannel].dat[2] & 0x100)
			{
				vptr->end = (m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xffff) << 2;
				vptr->end += vptr->start;
				vptr->mode = false;
				// hopefully it'll never happen? Maybe assert here?
				vptr->end &= m_rom_size - 1;

			}
			else // absolute
			{
				vptr->end = (m_audiodat[m_audioregister][m_audiochannel].dat[2] & 0x000f) << (16 + 4);
				vptr->end |= (m_audiodat[m_audioregister][m_audiochannel].dat[1] & 0xffff) << (4);
				vptr->end |= (m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xf000) >> (12);
				vptr->mode = true;

				vptr->end &= m_rom_size - 1;
			}

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


READ16_MEMBER(l7a1045_sound_device::sound_data_r)
{
	//printf("%04x (%04x %04x)\n",offset,m_audioregister,m_audiochannel);
	//debugger_break(machine());
	l7a1045_voice *vptr = &m_voice[m_audiochannel];

	switch(m_audioregister)
	{
		case 0x00:
		{
			UINT32 current_addr;
			UINT16 res;

			current_addr = vptr->start + vptr->pos;
			if(offset == 0)
				res = (current_addr & 0xf) << 12; // TODO: frac
			else if(offset == 1)
				res = (current_addr & 0xffff0) >> 4;
			else
				res = (current_addr & 0xf00000) >> 20;

			return res;
		}
	}

	return 0;
}

WRITE16_MEMBER(l7a1045_sound_device::sound_status_w)
{
	if(data & 0x100) // keyin
	{
		l7a1045_voice *vptr = &m_voice[m_audiochannel];

		#if 0
		if(vptr->start != 0)
		{
		printf("%08x START\n",vptr->start);
		printf("%08x END\n",vptr->end);

		for(int i=0;i<0x10;i++)
			printf("%02x (%02x) = %04x%04x%04x\n",m_audiochannel,i,m_audiodat[i][m_audiochannel].dat[2],m_audiodat[i][m_audiochannel].dat[1],m_audiodat[i][m_audiochannel].dat[0]);
		}
		#endif

		vptr->frac = 0;
		vptr->pos = 0;
		m_key |= 1 << m_audiochannel;
	}
}
