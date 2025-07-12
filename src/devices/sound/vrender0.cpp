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
static const u16 ULawTo16[]=
{
	0x8000,0x8400,0x8800,0x8C00,0x9000,0x9400,0x9800,0x9C00,
	0xA000,0xA400,0xA800,0xAC00,0xB000,0xB400,0xB800,0xBC00,
	0x4000,0x4400,0x4800,0x4C00,0x5000,0x5400,0x5800,0x5C00,
	0x6000,0x6400,0x6800,0x6C00,0x7000,0x7400,0x7800,0x7C00,
	0xC000,0xC200,0xC400,0xC600,0xC800,0xCA00,0xCC00,0xCE00,
	0xD000,0xD200,0xD400,0xD600,0xD800,0xDA00,0xDC00,0xDE00,
	0x2000,0x2200,0x2400,0x2600,0x2800,0x2A00,0x2C00,0x2E00,
	0x3000,0x3200,0x3400,0x3600,0x3800,0x3A00,0x3C00,0x3E00,
	0xE000,0xE100,0xE200,0xE300,0xE400,0xE500,0xE600,0xE700,
	0xE800,0xE900,0xEA00,0xEB00,0xEC00,0xED00,0xEE00,0xEF00,
	0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,
	0x1800,0x1900,0x1A00,0x1B00,0x1C00,0x1D00,0x1E00,0x1F00,
	0xF000,0xF080,0xF100,0xF180,0xF200,0xF280,0xF300,0xF380,
	0xF400,0xF480,0xF500,0xF580,0xF600,0xF680,0xF700,0xF780,
	0x0800,0x0880,0x0900,0x0980,0x0A00,0x0A80,0x0B00,0x0B80,
	0x0C00,0x0C80,0x0D00,0x0D80,0x0E00,0x0E80,0x0F00,0x0F80,
	0xF800,0xF840,0xF880,0xF8C0,0xF900,0xF940,0xF980,0xF9C0,
	0xFA00,0xFA40,0xFA80,0xFAC0,0xFB00,0xFB40,0xFB80,0xFBC0,
	0x0400,0x0440,0x0480,0x04C0,0x0500,0x0540,0x0580,0x05C0,
	0x0600,0x0640,0x0680,0x06C0,0x0700,0x0740,0x0780,0x07C0,
	0xFC00,0xFC20,0xFC40,0xFC60,0xFC80,0xFCA0,0xFCC0,0xFCE0,
	0xFD00,0xFD20,0xFD40,0xFD60,0xFD80,0xFDA0,0xFDC0,0xFDE0,
	0x0200,0x0220,0x0240,0x0260,0x0280,0x02A0,0x02C0,0x02E0,
	0x0300,0x0320,0x0340,0x0360,0x0380,0x03A0,0x03C0,0x03E0,
	0xFE00,0xFE10,0xFE20,0xFE30,0xFE40,0xFE50,0xFE60,0xFE70,
	0xFE80,0xFE90,0xFEA0,0xFEB0,0xFEC0,0xFED0,0xFEE0,0xFEF0,
	0x0100,0x0110,0x0120,0x0130,0x0140,0x0150,0x0160,0x0170,
	0x0180,0x0190,0x01A0,0x01B0,0x01C0,0x01D0,0x01E0,0x01F0,
	0x0000,0x0008,0x0010,0x0018,0x0020,0x0028,0x0030,0x0038,
	0x0040,0x0048,0x0050,0x0058,0x0060,0x0068,0x0070,0x0078,
	0xFF80,0xFF88,0xFF90,0xFF98,0xFFA0,0xFFA8,0xFFB0,0xFFB8,
	0xFFC0,0xFFC8,0xFFD0,0xFFD8,0xFFE0,0xFFE8,0xFFF0,0xFFF8,
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
		elem.Cache = &m_fbcache;

	m_stream = stream_alloc(0, 2, clock() / 972); // TODO : Correct source / divider?

	save_item(STRUCT_MEMBER(m_channel, CurSAddr));
	save_item(STRUCT_MEMBER(m_channel, EnvVol));
	save_item(STRUCT_MEMBER(m_channel, EnvStage));
	save_item(STRUCT_MEMBER(m_channel, dSAddr));
	save_item(STRUCT_MEMBER(m_channel, Modes));
	save_item(STRUCT_MEMBER(m_channel, LD));
	save_item(STRUCT_MEMBER(m_channel, LoopBegin));
	save_item(STRUCT_MEMBER(m_channel, LoopEnd));
	save_item(STRUCT_MEMBER(m_channel, LChnVol));
	save_item(STRUCT_MEMBER(m_channel, RChnVol));
	save_item(STRUCT_MEMBER(m_channel, EnvRate));
	save_item(STRUCT_MEMBER(m_channel, EnvTarget));
	save_item(NAME(m_Status));
	save_item(NAME(m_NoteOn));
	save_item(NAME(m_RevFactor));
	save_item(NAME(m_BufferAddr));
	save_item(NAME(m_BufferSize));
	save_item(NAME(m_IntMask));
	save_item(NAME(m_IntPend));
	save_item(NAME(m_MaxChn));
	save_item(NAME(m_ChnClkNum));
	save_item(NAME(m_Ctrl));
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
	VR0_RenderAudio(stream);
}

u16 vr0sound_device::channel_r(offs_t offset)
{
	return m_channel[(offset >> 4) & 0x1f].read(offset & 0xf);
}

void vr0sound_device::channel_w(offs_t offset, u16 data, u16 mem_mask)
{
	channel_t *channel = &m_channel[(offset >> 4) & 0x1f];
	u16 old_mode = channel->Modes;
	m_channel[(offset >> 4) & 0x1f].write(offset & 0xf, data, mem_mask);
	if ((old_mode ^ channel->Modes) & MODE_TEXTURE)
	{
		channel->Cache = (channel->Modes & MODE_TEXTURE) ? m_texcache_ctrl : &m_fbcache;
	}
}

u16 vr0sound_device::status_r(offs_t offset)
{
	return m_Status >> ((offset & 1) << 2);
}

void vr0sound_device::status_w(offs_t offset, u16 data)
{
	const u32 c = data & 0x1f;
	if (data & 0x8000)
	{
		m_Status |= 1 << c;
	}
	else
	{
		m_Status &= ~(1 << c);
	}
}

u16 vr0sound_device::noteon_r(offs_t offset)
{
	return m_NoteOn >> ((offset & 1) << 2);
}

void vr0sound_device::noteon_w(offs_t offset, u16 data)
{
	const u32 c = data & 0x1f;
	if (data & 0x8000)
	{
		m_NoteOn |= 1 << c;
	}
	else
	{
		m_NoteOn &= ~(1 << c);
	}
}

u16 vr0sound_device::revfactor_r(offs_t offset)
{
	return m_RevFactor & 0xff;
}

void vr0sound_device::revfactor_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_RevFactor = data & 0xff;
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
	return (m_BufferAddr >> 14) & 0x7f;
}

void vr0sound_device::buffersaddr_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_BufferAddr = (m_BufferAddr & ~(0x7f << 14)) | ((data & 0x7f) << 14);
}

u16 vr0sound_device::buffersize0_r(offs_t offset) { return m_BufferSize[0] & 0xfff; }
u16 vr0sound_device::buffersize1_r(offs_t offset) { return m_BufferSize[1] & 0xfff; }
u16 vr0sound_device::buffersize2_r(offs_t offset) { return m_BufferSize[2] & 0xfff; }
u16 vr0sound_device::buffersize3_r(offs_t offset) { return m_BufferSize[3] & 0xfff; }

void vr0sound_device::buffersize0_w(offs_t offset, u16 data, u16 mem_mask) { data &= 0xfff; COMBINE_DATA(&m_BufferSize[0]); }
void vr0sound_device::buffersize1_w(offs_t offset, u16 data, u16 mem_mask) { data &= 0xfff; COMBINE_DATA(&m_BufferSize[1]); }
void vr0sound_device::buffersize2_w(offs_t offset, u16 data, u16 mem_mask) { data &= 0xfff; COMBINE_DATA(&m_BufferSize[2]); }
void vr0sound_device::buffersize3_w(offs_t offset, u16 data, u16 mem_mask) { data &= 0xfff; COMBINE_DATA(&m_BufferSize[3]); }

u16 vr0sound_device::intmask_r(offs_t offset)
{
	return m_IntMask >> ((offset & 1) << 2);
}

void vr0sound_device::intmask_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int shift = ((offset & 1) << 2);
	m_IntMask = (m_IntMask & ~(mem_mask << shift)) | ((data & mem_mask) << shift);
}

u16 vr0sound_device::intpend_r(offs_t offset)
{
	return m_IntPend >> ((offset & 1) << 2);
}

void vr0sound_device::intpend_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_IntPend &= ~((data & mem_mask) << ((offset & 1) << 2));
	if (!m_IntPend)
		m_irq_cb(false);
}

u16 vr0sound_device::chnnum_r(offs_t offset)
{
	return ((m_MaxChn & 0x1f) << 8) | (m_ChnClkNum & 0xff);
}

void vr0sound_device::chnnum_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_ChnClkNum = data & 0xff;
	if (ACCESSING_BITS_8_15)
		m_MaxChn = (data >> 8) & 0x1f;
}

u16 vr0sound_device::ctrl_r(offs_t offset)
{
	return m_Ctrl;
}

void vr0sound_device::ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 old_ctrl = m_Ctrl;
	COMBINE_DATA(&m_Ctrl);
	if ((old_ctrl ^ m_Ctrl) & CTRL_TM)
		m_texcache_ctrl = (m_Ctrl & CTRL_TM) ? &m_texcache : &m_fbcache;
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
			ret = CurSAddr & 0x0000ffff;
			break;
		case 0x02/2:
			ret = (CurSAddr & 0xffff0000) >> 16;
			break;
		case 0x04/2:
			ret = EnvVol & 0xffff;
			break;
		case 0x06/2:
			ret = 0x6000 | (LD ? 0x1000 : 0) | ((EnvStage << 8) & 0x0f00) | ((EnvVol & 0xff0000) >> 16);
			break;
		case 0x08/2:
			ret = dSAddr;
			break;
		case 0x0a/2:
			ret = (Modes << 8) & 0x7f00;
			break;
		case 0x0c/2:
			ret = LoopBegin & 0x00ffff;
			break;
		case 0x0e/2:
			ret = ((LChnVol << 8) & 0x7f00) | ((LoopBegin & 0x3f0000) >> 16);
			break;
		case 0x10/2:
			ret = LoopEnd & 0x00ffff;
			break;
		case 0x12/2:
			ret = ((RChnVol << 8) & 0x7f00) | ((LoopEnd & 0x3f0000) >> 16);
			break;
		case 0x14/2:
		case 0x16/2:
		case 0x18/2:
		case 0x1a/2:
			ret = EnvRate[offset - (0x14/2)] & 0x0ffff;
			break;
		case 0x1c/2:
		case 0x1e/2:
			ret = (EnvTarget[((offset - (0x1c/2)) * 2) + 0] & 0x007f) | ((EnvTarget[((offset - (0x1c/2)) * 2) + 1] << 8) & 0x7f00);
			ret |= ((EnvRate[((offset - (0x1c/2)) * 2) + 0] & 0x10000) >> 9) | ((EnvRate[((offset - (0x1c/2)) * 2) + 1] & 0x10000) >> 1);
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
			CurSAddr = (CurSAddr & 0xffff0000) | (data & 0x0000ffff);
			break;
		case 0x02/2:
			CurSAddr = (CurSAddr & 0x0000ffff) | ((data << 16) & 0xffff0000);
			break;
		case 0x04/2:
			EnvVol = (EnvVol & ~0xffff) | (data & 0xffff);
			break;
		case 0x06/2:
			LD = data & 0x1000;
			EnvStage = (data & 0x0f00) >> 8;
			EnvVol = util::sext((EnvVol & 0x00ffff) | ((data << 16) & 0xff0000), 24);
			break;
		case 0x08/2:
			dSAddr = data & 0xffff;
			break;
		case 0x0a/2:
			Modes = (data & 0x7f00) >> 8;
			break;
		case 0x0c/2:
			LoopBegin = (LoopBegin & 0x3f0000) | (data & 0x00ffff);
			break;
		case 0x0e/2:
			LChnVol = (data & 0x7f00) >> 8;
			LoopBegin = (LoopBegin & 0x00ffff) | ((data << 16) & 0x3f0000);
			break;
		case 0x10/2:
			LoopEnd = (LoopEnd & 0x3f0000) | (data & 0x00ffff);
			break;
		case 0x12/2:
			RChnVol = (data & 0x7f00) >> 8;
			LoopEnd = (LoopEnd & 0x00ffff) | ((data << 16) & 0x3f0000);
			break;
		case 0x14/2:
		case 0x16/2:
		case 0x18/2:
		case 0x1a/2:
			EnvRate[offset - (0x14/2)] = (EnvRate[offset - (0x14/2)] & ~0xffff) | (data & 0xffff);
			break;
		case 0x1c/2:
		case 0x1e/2:
			EnvTarget[((offset - (0x1c/2)) * 2) + 0] = (data & 0x007f);
			EnvTarget[((offset - (0x1c/2)) * 2) + 1] = ((data & 0x7f00) >> 8);
			EnvRate[((offset - (0x1c/2)) * 2) + 0] = util::sext((EnvRate[((offset - (0x1c/2)) * 2) + 0] & 0xffff) | ((data & 0x0080) << 9), 17);
			EnvRate[((offset - (0x1c/2)) * 2) + 1] = util::sext((EnvRate[((offset - (0x1c/2)) * 2) + 1] & 0xffff) | ((data & 0x8000) << 1), 17);
			break;
	}
}

void vr0sound_device::VR0_RenderAudio(sound_stream &stream)
{
	int div;
	if (m_ChnClkNum)
		div = ((30 << 16) | 0x8000) / (m_ChnClkNum + 1); // TODO : Verify algorithm
	else
		div = 1 << 16;

	for (int s = 0; s < stream.samples(); s++)
	{
		s32 lsample = 0, rsample = 0;
		for (int i = 0; i <= m_MaxChn; i++)
		{
			channel_t *channel = &m_channel[i];
			s32 sample = 0;
			const u32 loopbegin_scaled = channel->LoopBegin << 10;
			const u32 loopend_scaled = channel->LoopEnd << 10;

			if (!(m_Status & (1 << i)) || !(m_Ctrl & CTRL_RS))
				continue;

			if (channel->Modes & MODE_ULAW)       //u-law
			{
				sample = channel->Cache->read_byte(channel->CurSAddr >> 9);
				sample = (s16)ULawTo16[sample & 0xff];
			}
			else
			{
				if (channel->Modes & MODE_8BIT)   //8bit
				{
					sample = channel->Cache->read_byte(channel->CurSAddr >> 9);
					sample = (s16)(((s8)(sample & 0xff)) << 8);
				}
				else                //16bit
				{
					sample = (s16)(channel->Cache->read_word((channel->CurSAddr >> 9) & ~1));
				}
			}

			channel->CurSAddr += (channel->dSAddr * div) >> 16;
			if (channel->CurSAddr >= loopend_scaled)
			{
				if (channel->Modes & MODE_LOOP)  //Loop
					channel->CurSAddr = (channel->CurSAddr - loopend_scaled) + loopbegin_scaled;
				else
				{
					m_Status &= ~(1 << (i & 0x1f));
					if (m_IntMask != 0xffffffff) // Interrupt, TODO : Partially implemented, Verify behavior from real hardware
					{
						const u32 old_pend = m_IntPend;
						m_IntPend |= (~m_IntMask & (1 << (i & 0x1f))); // it's can be with loop?
						if ((m_IntPend != 0) && (old_pend != m_IntPend))
							m_irq_cb(true);
					}
					break;
				}
			}

			const s32 v = channel->EnvVol >> 16;
			sample = (sample * v) >> 8;

			if (channel->Modes & MODE_ENVELOPE) // Envelope, TODO : Partially implemented, Verify behavior from real hardware
			{
				for (int level = 0; level < 4; level++)
				{
					if (BIT(channel->EnvStage, level))
					{
						s32 RATE = (channel->EnvRate[level] * div) >> 16;

						channel->EnvVol += RATE;
						if (RATE > 0)
						{
							if (((channel->EnvVol >> 16) & 0x7f) >= channel->EnvTarget[level])
							{
								channel->EnvStage <<= 1;
							}
						}
						else if (RATE < 0)
						{
							if (((channel->EnvVol >> 16) & 0x7f) <= channel->EnvTarget[level])
							{
								channel->EnvStage <<= 1;
							}
						}
					}
				}
			}
			lsample += (sample * channel->LChnVol) >> 8;
			rsample += (sample * channel->RChnVol) >> 8;
		}
		stream.put_int_clamp(0, s, lsample, 32768);
		stream.put_int_clamp(1, s, rsample, 32768);
	}
}
