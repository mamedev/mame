// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************

    Konami 054539 (TOP) PCM Sound Chip

    A lot of information comes from Amuse.
    Big thanks to them.

*********************************************************/

#include "emu.h"
#include "k054539.h"

const device_type K054539 = &device_creator<k054539_device>;

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

k054539_device::k054539_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K054539, "K054539 ADPCM", tag, owner, clock, "k054539", __FILE__),
		device_sound_interface(mconfig, *this), flags(0), ram(nullptr), reverb_pos(0), cur_ptr(0), cur_limit(0), 
	cur_zone(nullptr), rom(nullptr), rom_size(0), rom_mask(0), stream(nullptr), m_timer(nullptr), m_timer_state(0),
		m_timer_handler(*this),
		m_rgnoverride(NULL)
{
}


/* Registers:
   00..ff: 20 bytes/channel, 8 channels
     00..02: pitch (lsb, mid, msb)
         03: volume (0=max, 0x40=-36dB)
         04: reverb volume (idem)
     05: pan (1-f right, 10 middle, 11-1f left)
     06..07: reverb delay (0=max, current computation non-trusted)
     08..0a: loop (lsb, mid, msb)
     0c..0e: start (lsb, mid, msb) (and current position ?)

   100.1ff: effects?
     13f: pan of the analog input (1-1f)

   200..20f: 2 bytes/channel, 8 channels
     00: type (b2-3), reverse (b5)
     01: loop (b0)

   214: Key on (b0-7 = channel 0-7)
   215: Key off          ""
   225: ?
   227: Timer frequency
   228: ?
   229: ?
   22a: ?
   22b: ?
   22c: Channel active? (b0-7 = channel 0-7)
   22d: Data read/write port
   22e: ROM/RAM select (00..7f == ROM banks, 80 = Reverb RAM)
   22f: Global control:
        .......x - Enable PCM
        ......x. - Timer related?
        ...x.... - Enable ROM/RAM readback from 0x22d
        ..x..... - Timer output enable?
        x....... - Disable register RAM updates

    The chip has an optional 0x8000 byte reverb buffer.
    The reverb delay is actually an offset in this buffer.
*/

void k054539_device::init_flags(int _flags)
{
	flags = _flags;
}

void k054539_device::set_gain(int channel, double _gain)
{
	if(_gain >= 0)
		gain[channel] = _gain;
}
//*

bool k054539_device::regupdate()
{
	return !(regs[0x22f] & 0x80);
}

void k054539_device::keyon(int channel)
{
	if(regupdate())
		regs[0x22c] |= 1 << channel;
}

void k054539_device::keyoff(int channel)
{
	if(regupdate())
		regs[0x22c] &= ~(1 << channel);
}

void k054539_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
#define VOL_CAP 1.80

	static const INT16 dpcm[16] = {
		0<<8, 1<<8, 4<<8, 9<<8, 16<<8, 25<<8, 36<<8, 49<<8,
		-64<<8, -49<<8, -36<<8, -25<<8, -16<<8, -9<<8, -4<<8, -1<<8
	};


	INT16 *rbase = (INT16 *)ram;

	if(!(regs[0x22f] & 1))
		return;

	for(int sample = 0; sample != samples; sample++) {
		double lval, rval;
		if(!(flags & DISABLE_REVERB))
			lval = rval = rbase[reverb_pos];
		else
			lval = rval = 0;
		rbase[reverb_pos] = 0;

		for(int ch=0; ch<8; ch++)
			if(regs[0x22c] & (1<<ch)) {
				unsigned char *base1 = regs + 0x20*ch;
				unsigned char *base2 = regs + 0x200 + 0x2*ch;
				channel *chan = channels + ch;

				int delta = base1[0x00] | (base1[0x01] << 8) | (base1[0x02] << 16);

				int vol = base1[0x03];

				int bval = vol + base1[0x04];
				if (bval > 255)
					bval = 255;

				int pan = base1[0x05];
				// DJ Main: 81-87 right, 88 middle, 89-8f left
				if (pan >= 0x81 && pan <= 0x8f)
					pan -= 0x81;
				else if (pan >= 0x11 && pan <= 0x1f)
					pan -= 0x11;
				else
					pan = 0x18 - 0x11;

				double cur_gain = gain[ch];

				double lvol = voltab[vol] * pantab[pan] * cur_gain;
				if (lvol > VOL_CAP)
					lvol = VOL_CAP;

				double rvol = voltab[vol] * pantab[0xe - pan] * cur_gain;
				if (rvol > VOL_CAP)
					rvol = VOL_CAP;

				double rbvol= voltab[bval] * cur_gain / 2;
				if (rbvol > VOL_CAP)
					rbvol = VOL_CAP;

				int rdelta = (base1[6] | (base1[7] << 8)) >> 3;
				rdelta = (rdelta + reverb_pos) & 0x3fff;

				int cur_pos = (base1[0x0c] | (base1[0x0d] << 8) | (base1[0x0e] << 16)) & rom_mask;

				int fdelta, pdelta;
				if(base2[0] & 0x20) {
					delta = -delta;
					fdelta = +0x10000;
					pdelta = -1;
				} else {
					fdelta = -0x10000;
					pdelta = +1;
				}

				int cur_pfrac, cur_val, cur_pval;
				if(cur_pos != chan->pos) {
					chan->pos = cur_pos;
					cur_pfrac = 0;
					cur_val = 0;
					cur_pval = 0;
				} else {
					cur_pfrac = chan->pfrac;
					cur_val = chan->val;
					cur_pval = chan->pval;
				}

				switch(base2[0] & 0xc) {
				case 0x0: { // 8bit pcm
					cur_pfrac += delta;
					while(cur_pfrac & ~0xffff) {
						cur_pfrac += fdelta;
						cur_pos += pdelta;

						cur_pval = cur_val;
						cur_val = (INT16)(rom[cur_pos] << 8);
						if(cur_val == (INT16)0x8000 && (base2[1] & 1)) {
							cur_pos = (base1[0x08] | (base1[0x09] << 8) | (base1[0x0a] << 16)) & rom_mask;
							cur_val = (INT16)(rom[cur_pos] << 8);
						}
						if(cur_val == (INT16)0x8000) {
							keyoff(ch);
							cur_val = 0;
							break;
						}
					}
					break;
				}

				case 0x4: { // 16bit pcm lsb first
					pdelta <<= 1;

					cur_pfrac += delta;
					while(cur_pfrac & ~0xffff) {
						cur_pfrac += fdelta;
						cur_pos += pdelta;

						cur_pval = cur_val;
						cur_val = (INT16)(rom[cur_pos] | rom[cur_pos+1]<<8);
						if(cur_val == (INT16)0x8000 && (base2[1] & 1)) {
							cur_pos = (base1[0x08] | (base1[0x09] << 8) | (base1[0x0a] << 16)) & rom_mask;
							cur_val = (INT16)(rom[cur_pos] | rom[cur_pos+1]<<8);
						}
						if(cur_val == (INT16)0x8000) {
							keyoff(ch);
							cur_val = 0;
							break;
						}
					}
					break;
				}

				case 0x8: { // 4bit dpcm
					cur_pos <<= 1;
					cur_pfrac <<= 1;
					if(cur_pfrac & 0x10000) {
						cur_pfrac &= 0xffff;
						cur_pos |= 1;
					}

					cur_pfrac += delta;
					while(cur_pfrac & ~0xffff) {
						cur_pfrac += fdelta;
						cur_pos += pdelta;

						cur_pval = cur_val;
						cur_val = rom[cur_pos>>1];
						if(cur_val == 0x88 && (base2[1] & 1)) {
							cur_pos = ((base1[0x08] | (base1[0x09] << 8) | (base1[0x0a] << 16)) & rom_mask) << 1;
							cur_val = rom[cur_pos>>1];
						}
						if(cur_val == 0x88) {
							keyoff(ch);
							cur_val = 0;
							break;
						}
						if(cur_pos & 1)
							cur_val >>= 4;
						else
							cur_val &= 15;
						cur_val = cur_pval + dpcm[cur_val];
						if(cur_val < -32768)
							cur_val = -32768;
						else if(cur_val > 32767)
							cur_val = 32767;
					}

					cur_pfrac >>= 1;
					if(cur_pos & 1)
						cur_pfrac |= 0x8000;
					cur_pos >>= 1;
					break;
				}
				default:
					LOG(("Unknown sample type %x for channel %d\n", base2[0] & 0xc, ch));
					break;
				}
				lval += cur_val * lvol;
				rval += cur_val * rvol;
				rbase[(rdelta + reverb_pos) & 0x1fff] += INT16(cur_val*rbvol);

				chan->pos = cur_pos;
				chan->pfrac = cur_pfrac;
				chan->pval = cur_pval;
				chan->val = cur_val;

				if(regupdate()) {
					base1[0x0c] = cur_pos     & 0xff;
					base1[0x0d] = cur_pos>> 8 & 0xff;
					base1[0x0e] = cur_pos>>16 & 0xff;
				}
			}
		reverb_pos = (reverb_pos + 1) & 0x1fff;
		outputs[0][sample] = INT16(lval);
		outputs[1][sample] = INT16(rval);
	}
}


void k054539_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (regs[0x22f] & 0x20)
		m_timer_handler(m_timer_state ^= 1);
}

void k054539_device::init_chip()
{
	memset(regs, 0, sizeof(regs));
	memset(posreg_latch, 0, sizeof(posreg_latch)); //*
	flags |= UPDATE_AT_KEYON; //* make it default until proven otherwise

	ram = auto_alloc_array(machine(), unsigned char, 0x4000);
	reverb_pos = 0;
	cur_ptr = 0;
	memset(ram, 0, 0x4000);

	memory_region *reg = (m_rgnoverride != NULL) ? owner()->memregion(m_rgnoverride) : region();
	rom = reg->base();
	rom_size = reg->bytes();
	rom_mask = 0xffffffffU;
	for(int i=0; i<32; i++)
		if((1U<<i) >= rom_size) {
			rom_mask = (1U<<i) - 1;
			break;
		}

	stream = stream_alloc(0, 2, clock() / 384);

	save_item(NAME(regs));
	save_pointer(NAME(ram), 0x4000);
	save_item(NAME(cur_ptr));
}

WRITE8_MEMBER(k054539_device::write)
{
	if(0) {
		int voice, reg;

		/* The K054539 has behavior like many other wavetable chips including
		   the Ensoniq 550x and Gravis GF-1: if a voice is active, writing
		   to it's current position is silently ignored.

		   Dadandaan depends on this or the vocals go wrong.
		*/
		if (offset < 8*0x20)
		{
			voice = offset / 0x20;
			reg = offset & ~0x20;

			if(regs[0x22c] & (1<<voice))
				if (reg >= 0xc && reg <= 0xe)
					return;
		}
	}

	bool latch = (flags & UPDATE_AT_KEYON) && (regs[0x22f] & 1);

	if (latch && offset < 0x100)
	{
		int offs = (offset & 0x1f) - 0xc;
		int ch = offset >> 5;

		if (offs >= 0 && offs <= 2)
		{
			// latch writes to the position index registers
			posreg_latch[ch][offs] = data;
			return;
		}
	}

	else
		switch(offset) {
		case 0x13f: {
			int pan = data >= 0x11 && data <= 0x1f ? data - 0x11 : 0x18 - 0x11;
			if (!m_apan_cb.isnull())
				m_apan_cb(pantab[pan], pantab[0xe - pan]);
			break;
		}

		case 0x214:
			if (latch)
			{
				for(int ch=0; ch<8; ch++)
				{
					if(data & (1<<ch))
					{
						UINT8 *posptr = &posreg_latch[ch][0];
						UINT8 *regptr = regs + (ch<<5) + 0xc;

						// update the chip at key-on
						regptr[0] = posptr[0];
						regptr[1] = posptr[1];
						regptr[2] = posptr[2];

						keyon(ch);
					}
				}
			}
			else
			{
				for(int ch=0; ch<8; ch++)
					if(data & (1<<ch))
						keyon(ch);
			}
		break;

		case 0x215:
			for(int ch=0; ch<8; ch++)
				if(data & (1<<ch))
					keyoff(ch);
		break;

		case 0x227:
		{
			attotime period = attotime::from_hz((float)(38 + data) * (clock()/384.0f/14400.0f)) / 2.0f;

			m_timer->adjust(period, 0, period);

			m_timer_state = 0;
			m_timer_handler(m_timer_state);
		}
		break;

		case 0x22d:
			if(regs[0x22e] == 0x80)
				cur_zone[cur_ptr] = data;
			cur_ptr++;
			if(cur_ptr == cur_limit)
				cur_ptr = 0;
		break;

		case 0x22e:
			cur_zone =
				data == 0x80 ? ram :
				rom + 0x20000*data;
			cur_limit = data == 0x80 ? 0x4000 : 0x20000;
			cur_ptr = 0;
		break;

		case 0x22f:
			if (!(data & 0x20)) // Disable timer output?
			{
				m_timer_state = 0;
				m_timer_handler(m_timer_state);
			}
		break;

		default:
#if 0
			if(regs[offset] != data) {
				if((offset & 0xff00) == 0) {
					chanoff = offset & 0x1f;
					if(chanoff < 4 || chanoff == 5 ||
						(chanoff >=8 && chanoff <= 0xa) ||
						(chanoff >= 0xc && chanoff <= 0xe))
						break;
				}
				if(1 || ((offset >= 0x200) && (offset <= 0x210)))
					break;
				logerror("K054539 %03x = %02x\n", offset, data);
			}
#endif
		break;
	}

	regs[offset] = data;
}

void k054539_device::device_post_load()
{
	int data = regs[0x22e];
	cur_zone = data == 0x80 ? ram : rom + 0x20000*data;
	cur_limit = data == 0x80 ? 0x4000 : 0x20000;
}

READ8_MEMBER(k054539_device::read)
{
	switch(offset) {
	case 0x22d:
		if(regs[0x22f] & 0x10) {
			UINT8 res = cur_zone[cur_ptr];
			cur_ptr++;
			if(cur_ptr == cur_limit)
				cur_ptr = 0;
			return res;
		} else
			return 0;
	case 0x22c:
		break;
	default:
		LOG(("K054539 read %03x\n", offset));
		break;
	}
	return regs[offset];
}

void k054539_device::device_start()
{
	m_timer = timer_alloc(0);

	// resolve / bind callbacks
	m_timer_handler.resolve_safe();
	m_apan_cb.bind_relative_to(*owner());

	for (int i = 0; i < 8; i++)
		gain[i] = 1.0;

	flags = RESET_FLAGS;

	/*
	    I've tried various equations on volume control but none worked consistently.
	    The upper four channels in most MW/GX games simply need a significant boost
	    to sound right. For example, the bass and smash sound volumes in Violent Storm
	    have roughly the same values and the voices in Tokimeki Puzzledama are given
	    values smaller than those of the hihats. Needless to say the two K054539 chips
	    in Mystic Warriors are completely out of balance. Rather than forcing a
	    "one size fits all" function to the voltab the current invert exponential
	    appraoch seems most appropriate.
	*/
	// Factor the 1/4 for the number of channels in the volume (1/8 is too harsh, 1/2 gives clipping)
	// vol=0 -> no attenuation, vol=0x40 -> -36dB
	for(int i=0; i<256; i++)
		voltab[i] = pow(10.0, (-36.0 * (double)i / (double)0x40) / 20.0) / 4.0;

	// Pan table for the left channel
	// Right channel is identical with inverted index
	// Formula is such that pan[i]**2+pan[0xe-i]**2 = 1 (constant output power)
	// and pan[0xe] = 1 (full panning)
	for(int i=0; i<0xf; i++)
		pantab[i] = sqrt((double)i) / sqrt((double)0xe);

	init_chip();
}

void k054539_device::device_reset()
{
	m_timer->enable(false);
}
