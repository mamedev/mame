// license:BSD-3-Clause
// copyright-holders:ElSemi
#include "emu.h"
#include "vrender0.h"

/***********************************
        VRENDER ZERO
        AUDIO EMULATION
************************************/
/************
MISSING:
envelopes
reverb
interrupts
*************/

//Correct table thanks to Evoga
//they left a ulaw<->linear conversion tool inside the roms
static const unsigned short ULawTo16[]=
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


#define STATUS          m_SOUNDREGS[0x404/4]
#define CURSADDR(chan)  (m_SOUNDREGS[(0x20/4)*chan+0x00])
#define DSADDR(chan)    ((m_SOUNDREGS[(0x20/4)*chan+0x08/4]>>0)&0xffff)
#define LOOPBEGIN(chan) (m_SOUNDREGS[(0x20/4)*chan+0x0c/4]&0x3fffff)
#define LOOPEND(chan)   (m_SOUNDREGS[(0x20/4)*chan+0x10/4]&0x3fffff)
#define ENVVOL(chan)    (m_SOUNDREGS[(0x20/4)*chan+0x04/4]&0xffffff)

/*
#define GETSOUNDREG16(Chan,Offs) space.read_word(m_reg_base+0x20*Chan+Offs)
#define GETSOUNDREG32(Chan,Offs) space.read_dword(m_reg_base+0x20*Chan+Offs)

#define CURSADDR(chan)  GETSOUNDREG32(chan,0x00)
#define DSADDR(chan)    GETSOUNDREG16(chan,0x08)
#define LOOPBEGIN(chan) (GETSOUNDREG32(chan,0x0c)&0x3fffff)
#define LOOPEND(chan)   (GETSOUNDREG32(chan,0x10)&0x3fffff)
#define ENVVOL(chan)    (GETSOUNDREG32(chan,0x04)&0xffffff)
*/



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

const device_type VRENDER0 = &device_creator<vrender0_device>;

//-------------------------------------------------
//  vrender0_device - constructor
//-------------------------------------------------

vrender0_device::vrender0_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VRENDER0, "VRender0", tag, owner, clock, "vrender0", __FILE__),
		device_sound_interface(mconfig, *this),
		m_TexBase(nullptr),
		m_FBBase(nullptr),
		m_stream(nullptr),
		m_reg_base(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vrender0_device::device_start()
{
	memset(m_SOUNDREGS,0,sizeof(m_SOUNDREGS));

	m_stream = stream_alloc(0, 2, 44100);

	save_item(NAME(m_SOUNDREGS));
}


//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void vrender0_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	VR0_RenderAudio(samples, outputs[0], outputs[1]);
}



READ32_MEMBER(vrender0_device::vr0_snd_read)
{
	return m_SOUNDREGS[offset];
}


WRITE32_MEMBER(vrender0_device::vr0_snd_write)
{
	if(offset==0x404/4)
	{
		data&=0xffff;
		if(data&0x8000)
		{
			UINT32 c=data&0x1f;
			STATUS|=1<<c;
			CURSADDR(c)=0;
		}
		else
		{
			STATUS&=~(1<<(data&0x1f));
		}
		return;
	}
	else
	{
		COMBINE_DATA(&m_SOUNDREGS[offset]);
	}
}


void vrender0_device::set_areas(UINT32 *texture, UINT32 *frame)
{
	m_TexBase=texture;
	m_FBBase=frame;
}


void vrender0_device::VR0_RenderAudio(int nsamples, stream_sample_t *l, stream_sample_t *r)
{
	INT16 *SAMPLES;
	UINT32 st=STATUS;
	signed int lsample,rsample=0;
	UINT32 CLK=(m_SOUNDREGS[0x600/4]>>0)&0xff;
	UINT32 NCH=(m_SOUNDREGS[0x600/4]>>8)&0xff;
	UINT32 CT1=(m_SOUNDREGS[0x600/4]>>16)&0xff;
	UINT32 CT2=(m_SOUNDREGS[0x600/4]>>24)&0xff;
	int div;
	int s;


	if(CT1&0x20)
		SAMPLES=(INT16 *)m_TexBase;
	else
		SAMPLES=(INT16 *)m_FBBase;

	if(CLK)
		div=((30<<16)|0x8000)/(CLK+1);
	else
		div=1<<16;

	for(s=0;s<nsamples;++s)
	{
		int i;
		lsample=rsample=0;
		for(i=0;i<=NCH;++i)
		{
			signed int sample;
			UINT32 cur=CURSADDR(i);
			UINT32 a=LOOPBEGIN(i)+(cur>>10);
			UINT8 Mode=m_SOUNDREGS[(0x20/4)*i+0x8/4]>>24;
			signed int LVOL=m_SOUNDREGS[(0x20/4)*i+0xc/4]>>24;
			signed int RVOL=m_SOUNDREGS[(0x20/4)*i+0x10/4]>>24;

			INT32 DSADD=(DSADDR(i)*div)>>16;

			if(!(st&(1<<i)) || !(CT2&0x80))
				continue;

			if(Mode&0x10)       //u-law
			{
				UINT16 s=SAMPLES[a];
				if((cur&0x200))
					s>>=8;
				sample=(signed short)ULawTo16[s&0xff];
			}
			else
			{
				if(Mode&0x20)   //8bit
				{
					UINT16 s=SAMPLES[a];
					if((cur&0x200))
						s>>=8;
					sample=(signed short) (((signed char) (s&0xff))<<8);
				}
				else                //16bit
				{
					sample=SAMPLES[a];
				}
			}

			CURSADDR(i)+=DSADD;
			if(a>=LOOPEND(i))
			{
				if(Mode&1)  //Loop
					CURSADDR(i)=0;//LOOPBEGIN(i)<<10;
				else
				{
					STATUS&=~(1<<(i&0x1f));
					break;
				}
			}
//          UINT32 v=(ENVVOL(i))>>8;
//          sample=(sample*v)>>16;
			lsample+=(sample*LVOL)>>8;
			rsample+=(sample*RVOL)>>8;
		}
		if(lsample>32767)
			lsample=32767;
		if(lsample<-32768)
			lsample=-32768;
		l[s]=lsample;
		if(rsample>32767)
			rsample=32767;
		if(rsample<-32768)
			rsample=-32768;
		r[s]=rsample;
	}
}
