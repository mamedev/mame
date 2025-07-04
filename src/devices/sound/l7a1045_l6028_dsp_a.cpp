// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi
/**************************************************************************************************

    L7A1045 L6028 DSP-A
    (QFP120 package)

    this is the audio chip used on the following
    SNK Hyper NeoGeo 64 (arcade platform)
    AKAI MPC3000 (synth)

    both are driven by a V53.

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
    - Octave Control/BPM/Pitch, xrally Network BGM wants 66150 Hz which is definitely too fast for
      most fatfurwa samples;
    - Key Off for looping samples (fatfurwa should stop all samples when user insert a credit,
      cfr. reg[0] readback);
    - Most non-looping samples are setup to repeat twice on different channels (cfr. fatfurwa);
    - Fix relative sample end positions (non-loop);
    - ADSR (registers 2 & 4?);
    - How DMA really works?

**************************************************************************************************/

#include "emu.h"
#include "l7a1045_l6028_dsp_a.h"
#include "debugger.h"


// device type definition
DEFINE_DEVICE_TYPE(L7A1045, l7a1045_sound_device, "l7a1045", "L7A1045 L6028 DSP-A")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  l7a1045_sound_device - constructor
//-------------------------------------------------

l7a1045_sound_device::l7a1045_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, L7A1045, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_key(0),
		m_rom(*this, DEVICE_SELF)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void l7a1045_sound_device::device_start()
{
	// assumes it can make an address mask with m_rom.length() - 1
	assert(!(m_rom.length() & (m_rom.length() - 1)));

	// Allocate the stream
//  m_stream = stream_alloc(0, 2, 66150); //clock() / 384);
	// TODO: confirm frequency
	m_stream = stream_alloc(0, 2, 44100);

	save_item(STRUCT_MEMBER(m_voice, start));
	save_item(STRUCT_MEMBER(m_voice, end));
	save_item(STRUCT_MEMBER(m_voice, mode));
	save_item(STRUCT_MEMBER(m_voice, pos));
	save_item(STRUCT_MEMBER(m_voice, frac));
	save_item(STRUCT_MEMBER(m_voice, l_volume));
	save_item(STRUCT_MEMBER(m_voice, r_volume));
	save_item(NAME(m_key));
	save_item(NAME(m_audiochannel));
	save_item(NAME(m_audioregister));
	save_item(STRUCT_MEMBER(m_audiodat, dat));
}

void l7a1045_sound_device::device_reset()
{
	m_key = 0;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void l7a1045_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < 32; i++)
	{
		if (m_key & (1 << i))
		{
			l7a1045_voice *vptr = &m_voice[i];

			uint32_t start = vptr->start;
			uint32_t end = vptr->end;
			uint32_t step  = 0x400;

			uint32_t pos = vptr->pos;
			uint32_t frac = vptr->frac;

			for (int j = 0; j < stream.samples(); j++)
			{
				int32_t sample;
				uint8_t data;

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


				data = m_rom[(start + pos) & (m_rom.length() - 1)];
				sample = int8_t(data & 0xfc) << (3 - (data & 3));
				frac += step;

				stream.add_int(0, j, sample * vptr->l_volume, 32768 * 512);
				stream.add_int(1, j, sample * vptr->r_volume, 32768 * 512);
			}

			vptr->pos = pos;
			vptr->frac = frac;
		}
	}
}

// TODO: needs proper memory map
void l7a1045_sound_device::l7a1045_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_stream->update(); // TODO

	//logerror("%s: %x to %x (mask %04x)\n", tag(), data, offset, mem_mask);

	if(offset == 0)
		sound_select_w(offset, data, mem_mask);
	else if(offset == 8/2)
		sound_status_w(data);
	else
		sound_data_w(offset - 1,data);
}


uint16_t l7a1045_sound_device::l7a1045_sound_r(offs_t offset, uint16_t mem_mask)
{
	m_stream->update();

	//logerror("%s: read at %x (mask %04x)\n", tag(), offset, mem_mask);

	if (offset == 0)
	{
		//logerror("sound_select_r?\n");
	}
	else
		return sound_data_r(offset -1);

	return 0xffff;
}


void l7a1045_sound_device::sound_select_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// I'm guessing these addresses are the sound chip / DSP?

	// ---- ---- 000c cccc
	// c = channel

	if (ACCESSING_BITS_0_7)
	{
		m_audiochannel = data;
		if (m_audiochannel & 0xe0) logerror("%s l7a1045_sound_select_w unknown channel %01x\n", machine().describe_context(), m_audiochannel & 0xff);
		m_audiochannel &= 0x1f;
	}

	if (ACCESSING_BITS_8_15)
	{
		m_audioregister = (data >> 8);
		if (m_audioregister >0x0a) logerror("%s l7a1045_sound_select_w unknown register %01x\n", machine().describe_context(), m_audioregister & 0xff);
		m_audioregister &= 0x0f;
	}

}

void l7a1045_sound_device::sound_data_w(offs_t offset, uint16_t data)
{
	l7a1045_voice *vptr = &m_voice[m_audiochannel];

	//if(m_audioregister != 0 && m_audioregister != 1 && m_audioregister != 7)
	//  logerror("%04x %04x (%04x %04x)\n",offset,data,m_audioregister,m_audiochannel);

	m_audiodat[m_audioregister][m_audiochannel].dat[offset] = data;

	//logerror("%s: %x to ch %d reg %d\n", tag(), data, m_audiochannel, m_audioregister);

	switch (m_audioregister)
	{
		case 0x00:

			vptr->start = (m_audiodat[m_audioregister][m_audiochannel].dat[2] & 0x000f) << (16 + 4);
			vptr->start |= (m_audiodat[m_audioregister][m_audiochannel].dat[1] & 0xffff) << (4);
			vptr->start |= (m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xf000) >> (12);

			//logerror("%s: channel %d start = %08x\n", tag(), m_audiochannel, vptr->start);

			vptr->start &= m_rom.length() - 1;

			// if voice isn't active, clear the pos on start writes (required for DMA tests on MPC3000)
			if (!(m_key & (1 << m_audiochannel)))
			{
				vptr->pos = 0;
			}
			break;
		case 0x01:
			// relative to start
				//logerror("%04x\n",m_audiodat[m_audioregister][m_audiochannel].dat[0]);
				//logerror("%04x\n",m_audiodat[m_audioregister][m_audiochannel].dat[1]);
				//logerror("%04x\n",m_audiodat[m_audioregister][m_audiochannel].dat[2]);

			if(m_audiodat[m_audioregister][m_audiochannel].dat[2] & 0x100)
			{
				// TODO: definitely wrong
				// fatfurwa title screen sample 0x45a (0x8000?)
				// fatfurwa coin 0x3a0 (0x2000?)
				vptr->end = (m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xffff) << 2;
				vptr->end += vptr->start;
				vptr->mode = false;
				// hopefully it'll never happen? Maybe assert here?
				vptr->end &= m_rom.length() - 1;
			}
			else // absolute
			{
				vptr->end = (m_audiodat[m_audioregister][m_audiochannel].dat[2] & 0x000f) << (16 + 4);
				vptr->end |= (m_audiodat[m_audioregister][m_audiochannel].dat[1] & 0xffff) << (4);
				vptr->end |= (m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xf000) >> (12);
				vptr->mode = true;

				vptr->end &= m_rom.length() - 1;
			}
			//logerror("%s: channel %d end = %08x\n", tag(), m_audiochannel, vptr->start);
			break;

		case 0x07:

			vptr->r_volume = (m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xff);
			/* TODO: volume tables, linear? */
			vptr->r_volume = (vptr->r_volume) | (vptr->r_volume << 8);
			vptr->l_volume = (m_audiodat[m_audioregister][m_audiochannel].dat[0] >> 8) & 0xff;
			vptr->l_volume = (vptr->l_volume) | (vptr->l_volume << 8);
			//logerror("%04x %02x %02x\n",m_audiodat[m_audioregister][m_audiochannel].dat[0],vptr->l_volume,vptr->r_volume);

			break;
	}
}


uint16_t l7a1045_sound_device::sound_data_r(offs_t offset)
{
	//logerror("%04x (%04x %04x)\n",offset,m_audioregister,m_audiochannel);
	//machine().debug_break();
	l7a1045_voice *vptr = &m_voice[m_audiochannel];

	switch(m_audioregister)
	{
		case 0x00:
		{
			uint32_t current_addr;
			uint16_t res;

			// TODO: fatfurwa reads offset == 2, ANDs with 0xf and compares against a sample buffer value if it's bigger, smaller or equal
			// Returning 0xffff here for looping samples and they will silence out when user insert a coin ...

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

	// TODO: at least regs [3] and [5], relative position read-back?
	// TODO: reg [6]

	return 0;
}

void l7a1045_sound_device::sound_status_w(uint16_t data)
{
	if(data & 0x100) // keyin
	{
		l7a1045_voice *vptr = &m_voice[m_audiochannel];

#if 0
		if(vptr->start != 0)
		{
			logerror("%08x START\n",vptr->start);
			logerror("%08x END\n",vptr->end);

			for(int i=0;i<0x10;i++)
				logerror("%02x (%02x) = %04x%04x%04x\n",m_audiochannel,i,m_audiodat[i][m_audiochannel].dat[2],m_audiodat[i][m_audiochannel].dat[1],m_audiodat[i][m_audiochannel].dat[0]);
		}
#endif

		vptr->frac = 0;
		vptr->pos = 0;
		m_key |= 1 << m_audiochannel;
	}
}

// TODO: stub functions not really used
void l7a1045_sound_device::dma_hreq_cb(int state)
{
//  m_maincpu->hack_w(1);
}

uint8_t l7a1045_sound_device::dma_r_cb(offs_t offset)
{
//    logerror("dma_ior3_cb: offset %x\n", offset);

	m_voice[0].pos++;
	return 0;
}

void l7a1045_sound_device::dma_w_cb(offs_t offset, uint8_t data)
{
	m_voice[0].pos++;
//    logerror("dma_iow3_cb: offset %x\n", offset);
}
