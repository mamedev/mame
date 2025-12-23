// license:BSD-3-Clause
// copyright-holders:ElSemi
#include "emu.h"
#include "vrender0.h"

/*************************************************************************************
                                      VRENDER ZERO
                                     AUDIO EMULATION

    TODO
    - Envelope, Interrupt functions aren't verified from real hardware behavior.
    - Reverb, Pingpong/Reversed loop, Most of Channel/Overall control registers
      are Not implemented
    - Sample Rate is unverified

*************************************************************************************/

//Correct table thanks to Evoga
//they left a ulaw<->linear conversion tool inside the roms
static const u16 ulaw_to_16[]=
{
	0x8000,0x8400,0x8800,0x8c00,0x9000,0x9400,0x9800,0x9c00,
	0xa000,0xa400,0xa800,0xac00,0xb000,0xb400,0xb800,0xbc00,
	0x4000,0x4400,0x4800,0x4c00,0x5000,0x5400,0x5800,0x5c00,
	0x6000,0x6400,0x6800,0x6c00,0x7000,0x7400,0x7800,0x7c00,
	0xc000,0xc200,0xc400,0xc600,0xc800,0xca00,0xcc00,0xce00,
	0xd000,0xd200,0xd400,0xd600,0xd800,0xda00,0xdc00,0xde00,
	0x2000,0x2200,0x2400,0x2600,0x2800,0x2a00,0x2c00,0x2e00,
	0x3000,0x3200,0x3400,0x3600,0x3800,0x3a00,0x3c00,0x3e00,
	0xe000,0xe100,0xe200,0xe300,0xe400,0xe500,0xe600,0xe700,
	0xe800,0xe900,0xea00,0xeb00,0xec00,0xed00,0xee00,0xef00,
	0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,
	0x1800,0x1900,0x1a00,0x1b00,0x1c00,0x1d00,0x1e00,0x1f00,
	0xf000,0xf080,0xf100,0xf180,0xf200,0xf280,0xf300,0xf380,
	0xf400,0xf480,0xf500,0xf580,0xf600,0xf680,0xf700,0xf780,
	0x0800,0x0880,0x0900,0x0980,0x0a00,0x0a80,0x0b00,0x0b80,
	0x0c00,0x0c80,0x0d00,0x0d80,0x0e00,0x0e80,0x0f00,0x0f80,
	0xf800,0xf840,0xf880,0xf8c0,0xf900,0xf940,0xf980,0xf9c0,
	0xfa00,0xfa40,0xfa80,0xfac0,0xfb00,0xfb40,0xfb80,0xfbc0,
	0x0400,0x0440,0x0480,0x04c0,0x0500,0x0540,0x0580,0x05c0,
	0x0600,0x0640,0x0680,0x06c0,0x0700,0x0740,0x0780,0x07c0,
	0xfc00,0xfc20,0xfc40,0xfc60,0xfc80,0xfca0,0xfcc0,0xfce0,
	0xfd00,0xfd20,0xfd40,0xfd60,0xfd80,0xfda0,0xfdc0,0xfde0,
	0x0200,0x0220,0x0240,0x0260,0x0280,0x02a0,0x02c0,0x02e0,
	0x0300,0x0320,0x0340,0x0360,0x0380,0x03a0,0x03c0,0x03e0,
	0xfe00,0xfe10,0xfe20,0xfe30,0xfe40,0xfe50,0xfe60,0xfe70,
	0xfe80,0xfe90,0xfea0,0xfeb0,0xfec0,0xfed0,0xfee0,0xfef0,
	0x0100,0x0110,0x0120,0x0130,0x0140,0x0150,0x0160,0x0170,
	0x0180,0x0190,0x01a0,0x01b0,0x01c0,0x01d0,0x01e0,0x01f0,
	0x0000,0x0008,0x0010,0x0018,0x0020,0x0028,0x0030,0x0038,
	0x0040,0x0048,0x0050,0x0058,0x0060,0x0068,0x0070,0x0078,
	0xff80,0xff88,0xff90,0xff98,0xffa0,0xffa8,0xffb0,0xffb8,
	0xffc0,0xffc8,0xffd0,0xffd8,0xffe0,0xffe8,0xfff0,0xfff8,
};

// 16 bit access only
void vr0sound_device::sound_map(address_map &map)
{
	map(0x000, 0x3ff).rw(FUNC(vr0sound_device::channel_r), FUNC(vr0sound_device::channel_w));

	/*
	Sound Control Registers

	       fedcba98 76543210
	404(R) xxxxxxxx xxxxxxxx Status (Low); Channel 0-15
	406(R) xxxxxxxx xxxxxxxx Status (High); Channel 16-31
	404(W)
	406(W) x------- -------- Status Assign (0 = Off, 1 = On)
	       -------- ---xxxxx Status Channel
	408(R) xxxxxxxx xxxxxxxx NoteOn (Low); Channel 0-15
	40a(R) xxxxxxxx xxxxxxxx NoteOn (High); Channel 16-31
	408(W)
	40a(W) x------- -------- NoteOn Assign (0 = Off, 1 = On)
	       -------- ---xxxxx NoteOn Channel
	410    -------- xxxxxxxx RevFactor
	412    -------- -xxxxxxx BufferSAddr (Top 7 bit of Reverb Buffer Start Address)
	420    ----xxxx xxxxxxxx BufferSize0
	422    ----xxxx xxxxxxxx BufferSize1
	440    ----xxxx xxxxxxxx BufferSize2
	442    ----xxxx xxxxxxxx BufferSize3
	480    ----xxxx xxxxxxxx IntMask (Low); Channel 0-15
	482    ----xxxx xxxxxxxx IntMask (High); Channel 16-31
	500    ----xxxx xxxxxxxx IntPend (Low); Channel 0-15
	502    ----xxxx xxxxxxxx IntPend (High); Channel 16-31
	600    ---xxxxx -------- MaxChn
	       -------- xxxxxxxx ChnClkNum (Clock Number per Channel)
	602    x------- -------- RS (Run Sound)
	       -------- --x----- TM (Texture Memory)
	       -------- ---x---- RE (Reverb Enable)
	       -------- -----x-- CW (32bit Adder Wait)
	       -------- ------x- AW (16bit Adder Wait)
	       -------- -------x MW (Multipler Wait)
	*/

	map(0x404, 0x407).rw(FUNC(vr0sound_device::status_r), FUNC(vr0sound_device::status_w));
	map(0x408, 0x40b).rw(FUNC(vr0sound_device::noteon_r), FUNC(vr0sound_device::noteon_w));
	map(0x410, 0x411).rw(FUNC(vr0sound_device::revfactor_r), FUNC(vr0sound_device::revfactor_w));
	map(0x412, 0x413).rw(FUNC(vr0sound_device::buffersaddr_r), FUNC(vr0sound_device::buffersaddr_w));
	map(0x420, 0x421).rw(FUNC(vr0sound_device::buffersize0_r), FUNC(vr0sound_device::buffersize0_w));
	map(0x422, 0x423).rw(FUNC(vr0sound_device::buffersize1_r), FUNC(vr0sound_device::buffersize1_w));
	map(0x440, 0x441).rw(FUNC(vr0sound_device::buffersize2_r), FUNC(vr0sound_device::buffersize2_w));
	map(0x442, 0x443).rw(FUNC(vr0sound_device::buffersize3_r), FUNC(vr0sound_device::buffersize3_w));
	map(0x480, 0x483).rw(FUNC(vr0sound_device::intmask_r), FUNC(vr0sound_device::intmask_w));
	map(0x500, 0x503).rw(FUNC(vr0sound_device::intpend_r), FUNC(vr0sound_device::intpend_w));
	map(0x600, 0x601).rw(FUNC(vr0sound_device::chnnum_r), FUNC(vr0sound_device::chnnum_w));
	map(0x602, 0x603).rw(FUNC(vr0sound_device::ctrl_r), FUNC(vr0sound_device::ctrl_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(SOUND_VRENDER0, vr0sound_device, "vr0sound", "MagicEyes VRender0 Sound Engine")

//-------------------------------------------------
//  vr0sound_device - constructor
//-------------------------------------------------

vr0sound_device::vr0sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SOUND_VRENDER0, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_texture_config("texture", ENDIANNESS_LITTLE, 16, 23) // 64 MBit (8 MB) Texture Memory Support
	, m_frame_config("frame", ENDIANNESS_LITTLE, 16, 23) // 64 MBit (8 MB) Framebuffer Memory Support
	, m_stream(nullptr)
	, m_irq_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vr0sound_device::device_start()
{
	// Find our direct access
	space(AS_TEXTURE).cache(m_texcache);
	space(AS_FRAME).cache(m_fbcache);
	m_texcache_ctrl = &m_fbcache;
	for (auto &elem : m_channel)
		elem.cache = &m_fbcache;

	m_stream = stream_alloc(0, 2, clock() / 972); // TODO : Correct source / divider?

	save_item(STRUCT_MEMBER(m_channel, cur_saddr));
	save_item(STRUCT_MEMBER(m_channel, env_vol));
	save_item(STRUCT_MEMBER(m_channel, env_stage));
	save_item(STRUCT_MEMBER(m_channel, ds_addr));
	save_item(STRUCT_MEMBER(m_channel, modes));
	save_item(STRUCT_MEMBER(m_channel, ld));
	save_item(STRUCT_MEMBER(m_channel, loop_begin));
	save_item(STRUCT_MEMBER(m_channel, loop_end));
	save_item(STRUCT_MEMBER(m_channel, l_chn_vol));
	save_item(STRUCT_MEMBER(m_channel, r_chn_vol));
	save_item(STRUCT_MEMBER(m_channel, env_rate));
	save_item(STRUCT_MEMBER(m_channel, env_target));
	save_item(NAME(m_status));
	save_item(NAME(m_note_on));
	save_item(NAME(m_rev_factor));
	save_item(NAME(m_buffer_addr));
	save_item(NAME(m_buffer_size));
	save_item(NAME(m_int_mask));
	save_item(NAME(m_int_pend));
	save_item(NAME(m_max_chan));
	save_item(NAME(m_chan_clk_num));
	save_item(NAME(m_ctrl));
}

//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void vr0sound_device::device_post_load()
{
	device_clock_changed();
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void vr0sound_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 972);
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector vr0sound_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_TEXTURE, &m_texture_config),
		std::make_pair(AS_FRAME, &m_frame_config)
	};
}

//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void vr0sound_device::sound_stream_update(sound_stream &stream)
{
	render_audio(stream);
}

u16 vr0sound_device::channel_r(offs_t offset)
{
	return m_channel[(offset >> 4) & 0x1f].read(offset & 0xf);
}

void vr0sound_device::channel_w(offs_t offset, u16 data, u16 mem_mask)
{
	channel_t &channel = m_channel[(offset >> 4) & 0x1f];
	const u16 old_mode = channel.modes;
	m_channel[(offset >> 4) & 0x1f].write(offset & 0xf, data, mem_mask);
	if ((old_mode ^ channel.modes) & MODE_TEXTURE)
	{
		channel.cache = (channel.modes & MODE_TEXTURE) ? m_texcache_ctrl : &m_fbcache;
	}
}

u16 vr0sound_device::status_r(offs_t offset)
{
	return m_status >> ((offset & 1) << 2);
}

void vr0sound_device::status_w(offs_t offset, u16 data)
{
	const u32 c = data & 0x1f;
	if (BIT(data, 15))
	{
		m_status |= 1 << c;
	}
	else
	{
		m_status &= ~(1 << c);
	}
}

u16 vr0sound_device::noteon_r(offs_t offset)
{
	return m_note_on >> ((offset & 1) << 2);
}

void vr0sound_device::noteon_w(offs_t offset, u16 data)
{
	const u32 c = data & 0x1f;
	if (BIT(data, 15))
	{
		m_note_on |= 1 << c;
	}
	else
	{
		m_note_on &= ~(1 << c);
	}
}

u16 vr0sound_device::revfactor_r(offs_t offset)
{
	return m_rev_factor & 0xff;
}

void vr0sound_device::revfactor_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_rev_factor = data & 0xff;
}

/*
    Buffer Address
        1                0
        fedcba9876543210 fedcba9876543210
        ----------0xxxxx x--------------- BufferSAddr
        ---------------- -xxx------------ Buffer Select
        ---------------- ----xxxxxxxxxxxx Buffer Pointer
*/

u16 vr0sound_device::buffersaddr_r(offs_t offset)
{
	return (m_buffer_addr >> 14) & 0x7f;
}

void vr0sound_device::buffersaddr_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_buffer_addr = (m_buffer_addr & ~(0x7f << 14)) | ((data & 0x7f) << 14);
}

u16 vr0sound_device::buffersize0_r(offs_t offset) { return m_buffer_size[0] & 0xfff; }
u16 vr0sound_device::buffersize1_r(offs_t offset) { return m_buffer_size[1] & 0xfff; }
u16 vr0sound_device::buffersize2_r(offs_t offset) { return m_buffer_size[2] & 0xfff; }
u16 vr0sound_device::buffersize3_r(offs_t offset) { return m_buffer_size[3] & 0xfff; }

void vr0sound_device::buffersize0_w(offs_t offset, u16 data, u16 mem_mask) { data &= 0xfff; COMBINE_DATA(&m_buffer_size[0]); }
void vr0sound_device::buffersize1_w(offs_t offset, u16 data, u16 mem_mask) { data &= 0xfff; COMBINE_DATA(&m_buffer_size[1]); }
void vr0sound_device::buffersize2_w(offs_t offset, u16 data, u16 mem_mask) { data &= 0xfff; COMBINE_DATA(&m_buffer_size[2]); }
void vr0sound_device::buffersize3_w(offs_t offset, u16 data, u16 mem_mask) { data &= 0xfff; COMBINE_DATA(&m_buffer_size[3]); }

u16 vr0sound_device::intmask_r(offs_t offset)
{
	return m_int_mask >> ((offset & 1) << 2);
}

void vr0sound_device::intmask_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int shift = ((offset & 1) << 2);
	m_int_mask = (m_int_mask & ~(mem_mask << shift)) | ((data & mem_mask) << shift);
}

u16 vr0sound_device::intpend_r(offs_t offset)
{
	return m_int_pend >> ((offset & 1) << 2);
}

void vr0sound_device::intpend_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_int_pend &= ~((data & mem_mask) << ((offset & 1) << 2));
	if (!m_int_pend)
		m_irq_cb(false);
}

u16 vr0sound_device::chnnum_r(offs_t offset)
{
	return ((m_max_chan & 0x1f) << 8) | (m_chan_clk_num & 0xff);
}

void vr0sound_device::chnnum_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_chan_clk_num = data & 0xff;
	if (ACCESSING_BITS_8_15)
		m_max_chan = (data >> 8) & 0x1f;
}

u16 vr0sound_device::ctrl_r(offs_t offset)
{
	return m_ctrl;
}

void vr0sound_device::ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 old_ctrl = m_ctrl;
	COMBINE_DATA(&m_ctrl);
	if ((old_ctrl ^ m_ctrl) & CTRL_TM)
	{
		m_texcache_ctrl = (m_ctrl & CTRL_TM) ? &m_texcache : &m_fbcache;
		// refresh channel cache
		for (auto &elem : m_channel)
		{
			if (elem.modes & MODE_TEXTURE)
				elem.cache = m_texcache_ctrl;
		}
	}
}

/*
Channel Parameter Register (32 bytes for each channels)

    fedcba98 76543210
00  xxxxxxxx xxxxxxxx CurSAddr (15:0)
02  xxxxxxxx xxxxxxxx CurSAddr (31:16)
04  xxxxxxxx xxxxxxxx EnvVol (15:0)
06  -11x---- -------- Loop Direction(LD)
    -11-xxxx -------- EnvStage
    -------- xxxxxxxx EnvVol (23:16)
08  xxxxxxxx xxxxxxxx DSAddr (15:0)
0a  -xxxxxxx -------- Modes
0c  xxxxxxxx xxxxxxxx LoopBegin (15:0)
0e  -xxxxxxx -------- LChnVol
    -------- --xxxxxx LoopBegin (21:16)
10  xxxxxxxx xxxxxxxx LoopEnd (15:0)
12  -xxxxxxx -------- RChnVol
    -------- --xxxxxx LoopEnd (21:16)
14  xxxxxxxx xxxxxxxx EnvRate0 (15:0)
16  xxxxxxxx xxxxxxxx EnvRate1 (15:0)
18  xxxxxxxx xxxxxxxx EnvRate2 (15:0)
1a  xxxxxxxx xxxxxxxx EnvRate3 (15:0)
1c  x------- -------- EnvRate1 (16)
    -xxxxxxx -------- EnvTarget1
    -------- x------- EnvRate0 (16)
    -------- -xxxxxxx EnvTarget0
1e  x------- -------- EnvRate3 (16)
    -xxxxxxx -------- EnvTarget3
    -------- x------- EnvRate2 (16)
    -------- -xxxxxxx EnvTarget2
*/

u16 vr0sound_device::channel_t::read(offs_t offset)
{
	u16 ret = 0;

	switch (offset)
	{
		case 0x00/2:
			ret = cur_saddr & 0x0000ffff;
			break;
		case 0x02/2:
			ret = (cur_saddr & 0xffff0000) >> 16;
			break;
		case 0x04/2:
			ret = env_vol & 0xffff;
			break;
		case 0x06/2:
			ret = 0x6000 | (ld ? 0x1000 : 0) | ((env_stage << 8) & 0x0f00) | ((env_vol & 0xff0000) >> 16);
			break;
		case 0x08/2:
			ret = ds_addr;
			break;
		case 0x0a/2:
			ret = (modes << 8) & 0x7f00;
			break;
		case 0x0c/2:
			ret = loop_begin & 0x00ffff;
			break;
		case 0x0e/2:
			ret = ((l_chn_vol << 8) & 0x7f00) | ((loop_begin & 0x3f0000) >> 16);
			break;
		case 0x10/2:
			ret = loop_end & 0x00ffff;
			break;
		case 0x12/2:
			ret = ((r_chn_vol << 8) & 0x7f00) | ((loop_end & 0x3f0000) >> 16);
			break;
		case 0x14/2:
		case 0x16/2:
		case 0x18/2:
		case 0x1a/2:
			ret = env_rate[offset - (0x14/2)] & 0x0ffff;
			break;
		case 0x1c/2:
		case 0x1e/2:
			ret = (env_target[((offset - (0x1c/2)) * 2) + 0] & 0x007f) | ((env_target[((offset - (0x1c/2)) * 2) + 1] << 8) & 0x7f00);
			ret |= ((env_rate[((offset - (0x1c/2)) * 2) + 0] & 0x10000) >> 9) | ((env_rate[((offset - (0x1c/2)) * 2) + 1] & 0x10000) >> 1);
			break;
	}
	return ret;
}

void vr0sound_device::channel_t::write(offs_t offset, u16 data, u16 mem_mask)
{
	u16 newdata = read(offset);
	COMBINE_DATA(&newdata);

	data = newdata;
	switch (offset)
	{
		case 0x00/2:
			cur_saddr = (cur_saddr & 0xffff0000) | (data & 0x0000ffff);
			break;
		case 0x02/2:
			cur_saddr = (cur_saddr & 0x0000ffff) | ((u32(data) << 16) & 0xffff0000);
			break;
		case 0x04/2:
			env_vol = (env_vol & ~0xffff) | (data & 0xffff);
			break;
		case 0x06/2:
			ld = BIT(data, 12);
			env_stage = (data & 0x0f00) >> 8;
			env_vol = util::sext((env_vol & 0x00ffff) | ((u32(data) << 16) & 0xff0000), 24);
			break;
		case 0x08/2:
			ds_addr = data & 0xffff;
			break;
		case 0x0a/2:
			modes = (data & 0x7f00) >> 8;
			break;
		case 0x0c/2:
			loop_begin = (loop_begin & 0x3f0000) | (data & 0x00ffff);
			break;
		case 0x0e/2:
			l_chn_vol = (data & 0x7f00) >> 8;
			loop_begin = (loop_begin & 0x00ffff) | ((u32(data) << 16) & 0x3f0000);
			break;
		case 0x10/2:
			loop_end = (loop_end & 0x3f0000) | (data & 0x00ffff);
			break;
		case 0x12/2:
			r_chn_vol = (data & 0x7f00) >> 8;
			loop_end = (loop_end & 0x00ffff) | ((u32(data) << 16) & 0x3f0000);
			break;
		case 0x14/2:
		case 0x16/2:
		case 0x18/2:
		case 0x1a/2:
			env_rate[offset - (0x14/2)] = (env_rate[offset - (0x14/2)] & ~0xffff) | (data & 0xffff);
			break;
		case 0x1c/2:
		case 0x1e/2:
			env_target[((offset - (0x1c/2)) * 2) + 0] = (data & 0x007f);
			env_target[((offset - (0x1c/2)) * 2) + 1] = ((data & 0x7f00) >> 8);
			env_rate[((offset - (0x1c/2)) * 2) + 0] = util::sext((env_rate[((offset - (0x1c/2)) * 2) + 0] & 0xffff) | ((data & 0x0080) << 9), 17);
			env_rate[((offset - (0x1c/2)) * 2) + 1] = util::sext((env_rate[((offset - (0x1c/2)) * 2) + 1] & 0xffff) | ((data & 0x8000) << 1), 17);
			break;
	}
}

void vr0sound_device::render_audio(sound_stream &stream)
{
	int div;
	if (m_chan_clk_num)
		div = ((30 << 16) | 0x8000) / (m_chan_clk_num + 1); // TODO : Verify algorithm
	else
		div = 1 << 16;

	for (int s = 0; s < stream.samples(); s++)
	{
		s32 lsample = 0, rsample = 0;
		for (int i = 0; i <= m_max_chan; i++)
		{
			channel_t &channel = m_channel[i];
			s32 sample = 0;
			const u32 loopbegin_scaled = channel.loop_begin << 10;
			const u32 loopend_scaled = channel.loop_end << 10;

			if (!(m_status & (1 << i)) || !(m_ctrl & CTRL_RS))
				continue;

			if (channel.modes & MODE_ULAW)       //u-law
			{
				sample = channel.cache->read_byte(channel.cur_saddr >> 9);
				sample = s16(ulaw_to_16[sample & 0xff]);
			}
			else
			{
				if (channel.modes & MODE_8BIT)   //8bit
				{
					sample = channel.cache->read_byte(channel.cur_saddr >> 9);
					sample = s16(s8(sample & 0xff) << 8);
				}
				else                //16bit
				{
					sample = s16(channel.cache->read_word((channel.cur_saddr >> 9) & ~1));
				}
			}

			channel.cur_saddr += (channel.ds_addr * div) >> 16;
			if (channel.cur_saddr >= loopend_scaled)
			{
				if (channel.modes & MODE_LOOP)  //Loop
					channel.cur_saddr = (channel.cur_saddr - loopend_scaled) + loopbegin_scaled;
				else
				{
					m_status &= ~(1 << (i & 0x1f));
					if (m_int_mask != 0xffffffff) // Interrupt, TODO : Partially implemented, Verify behavior from real hardware
					{
						const u32 old_pend = m_int_pend;
						m_int_pend |= (~m_int_mask & (1 << (i & 0x1f))); // it's can be with loop?
						if ((m_int_pend != 0) && (old_pend != m_int_pend))
							m_irq_cb(true);
					}
					break;
				}
			}

			const s32 v = channel.env_vol >> 16;
			sample = (sample * v) >> 8;

			if (channel.modes & MODE_ENVELOPE) // Envelope, TODO : Partially implemented, Verify behavior from real hardware
			{
				for (int level = 0; level < 4; level++)
				{
					if (BIT(channel.env_stage, level))
					{
						const s32 rate = (channel.env_rate[level] * div) >> 16;

						channel.env_vol += rate;
						if (rate > 0)
						{
							if (((channel.env_vol >> 16) & 0x7f) >= channel.env_target[level])
							{
								channel.env_stage <<= 1;
							}
						}
						else if (rate < 0)
						{
							if (((channel.env_vol >> 16) & 0x7f) <= channel.env_target[level])
							{
								channel.env_stage <<= 1;
							}
						}
					}
				}
			}
			lsample += (sample * channel.l_chn_vol) >> 8;
			rsample += (sample * channel.r_chn_vol) >> 8;
		}
		stream.put_int_clamp(0, s, lsample, 32768);
		stream.put_int_clamp(1, s, rsample, 32768);
	}
}
