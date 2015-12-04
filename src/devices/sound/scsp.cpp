// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont
/*
    Sega/Yamaha YMF292-F (SCSP = Saturn Custom Sound Processor) emulation
    By ElSemi
    MAME/M1 conversion and cleanup by R. Belmont
    Additional code and bugfixes by kingshriek

    This chip has 32 voices.  Each voice can play a sample or be part of
    an FM construct.  Unlike traditional Yamaha FM chips, the base waveform
    for the FM still comes from the wavetable RAM.

    ChangeLog:
    * November 25, 2003  (ES) Fixed buggy timers and envelope overflows.
                         (RB) Improved sample rates other than 44100, multiple
                             chips now works properly.
    * December 02, 2003  (ES) Added DISDL register support, improves mix.
    * April 28, 2004     (ES) Corrected envelope rates, added key-rate scaling,
                             added ringbuffer support.
    * January 8, 2005    (RB) Added ability to specify region offset for RAM.
    * January 26, 2007   (ES) Added on-board DSP capability
    * September 24, 2007 (RB+ES) Removed fake reverb.  Rewrote timers and IRQ handling.
                             Fixed case where voice frequency is updated while looping.
                             Enabled DSP again.
    * December 16, 2007  (kingshriek) Many EG bug fixes, implemented effects mixer,
                             implemented FM.
    * January 5, 2008    (kingshriek+RB) Working, good-sounding FM, removed obsolete non-USEDSP code.
    * April 22, 2009     ("PluginNinja") Improved slot monitor, misc cleanups
    * June 6, 2011       (AS) Rewrote DMA from scratch, Darius 2 relies on it.
*/

#include "emu.h"
#include "sound/cdda.h"
#include "scsp.h"


#define ICLIP16(x) (x<-32768)?-32768:((x>32767)?32767:x)

#define SHIFT   12
#define LFO_SHIFT   8
#define FIX(v)  ((UINT32) ((float) (1<<SHIFT)*(v)))


#define EG_SHIFT    16


/*
    SCSP features 32 programmable slots
    that can generate FM and PCM (from ROM/RAM) sound
*/

//SLOT PARAMETERS
#define KEYONEX(slot)       ((slot->udata.data[0x0]>>0x0)&0x1000)
#define KEYONB(slot)        ((slot->udata.data[0x0]>>0x0)&0x0800)
#define SBCTL(slot)     ((slot->udata.data[0x0]>>0x9)&0x0003)
#define SSCTL(slot)     ((slot->udata.data[0x0]>>0x7)&0x0003)
#define LPCTL(slot)     ((slot->udata.data[0x0]>>0x5)&0x0003)
#define PCM8B(slot)     ((slot->udata.data[0x0]>>0x0)&0x0010)

#define SA(slot)        (((slot->udata.data[0x0]&0xF)<<16)|(slot->udata.data[0x1]))

#define LSA(slot)       (slot->udata.data[0x2])

#define LEA(slot)       (slot->udata.data[0x3])

#define D2R(slot)       ((slot->udata.data[0x4]>>0xB)&0x001F)
#define D1R(slot)       ((slot->udata.data[0x4]>>0x6)&0x001F)
#define EGHOLD(slot)        ((slot->udata.data[0x4]>>0x0)&0x0020)
#define AR(slot)        ((slot->udata.data[0x4]>>0x0)&0x001F)

#define LPSLNK(slot)        ((slot->udata.data[0x5]>>0x0)&0x4000)
#define KRS(slot)       ((slot->udata.data[0x5]>>0xA)&0x000F)
#define DL(slot)        ((slot->udata.data[0x5]>>0x5)&0x001F)
#define RR(slot)        ((slot->udata.data[0x5]>>0x0)&0x001F)

#define STWINH(slot)        ((slot->udata.data[0x6]>>0x0)&0x0200)
#define SDIR(slot)      ((slot->udata.data[0x6]>>0x0)&0x0100)
#define TL(slot)        ((slot->udata.data[0x6]>>0x0)&0x00FF)

#define MDL(slot)       ((slot->udata.data[0x7]>>0xC)&0x000F)
#define MDXSL(slot)     ((slot->udata.data[0x7]>>0x6)&0x003F)
#define MDYSL(slot)     ((slot->udata.data[0x7]>>0x0)&0x003F)

#define OCT(slot)       ((slot->udata.data[0x8]>>0xB)&0x000F)
#define FNS(slot)       ((slot->udata.data[0x8]>>0x0)&0x03FF)

#define LFORE(slot)     ((slot->udata.data[0x9]>>0x0)&0x8000)
#define LFOF(slot)      ((slot->udata.data[0x9]>>0xA)&0x001F)
#define PLFOWS(slot)        ((slot->udata.data[0x9]>>0x8)&0x0003)
#define PLFOS(slot)     ((slot->udata.data[0x9]>>0x5)&0x0007)
#define ALFOWS(slot)        ((slot->udata.data[0x9]>>0x3)&0x0003)
#define ALFOS(slot)     ((slot->udata.data[0x9]>>0x0)&0x0007)

#define ISEL(slot)      ((slot->udata.data[0xA]>>0x3)&0x000F)
#define IMXL(slot)      ((slot->udata.data[0xA]>>0x0)&0x0007)

#define DISDL(slot)     ((slot->udata.data[0xB]>>0xD)&0x0007)
#define DIPAN(slot)     ((slot->udata.data[0xB]>>0x8)&0x001F)
#define EFSDL(slot)     ((slot->udata.data[0xB]>>0x5)&0x0007)
#define EFPAN(slot)     ((slot->udata.data[0xB]>>0x0)&0x001F)

//Envelope times in ms
static const double ARTimes[64]={100000/*infinity*/,100000/*infinity*/,8100.0,6900.0,6000.0,4800.0,4000.0,3400.0,3000.0,2400.0,2000.0,1700.0,1500.0,
					1200.0,1000.0,860.0,760.0,600.0,500.0,430.0,380.0,300.0,250.0,220.0,190.0,150.0,130.0,110.0,95.0,
					76.0,63.0,55.0,47.0,38.0,31.0,27.0,24.0,19.0,15.0,13.0,12.0,9.4,7.9,6.8,6.0,4.7,3.8,3.4,3.0,2.4,
					2.0,1.8,1.6,1.3,1.1,0.93,0.85,0.65,0.53,0.44,0.40,0.35,0.0,0.0};
static const double DRTimes[64]={100000/*infinity*/,100000/*infinity*/,118200.0,101300.0,88600.0,70900.0,59100.0,50700.0,44300.0,35500.0,29600.0,25300.0,22200.0,17700.0,
					14800.0,12700.0,11100.0,8900.0,7400.0,6300.0,5500.0,4400.0,3700.0,3200.0,2800.0,2200.0,1800.0,1600.0,1400.0,1100.0,
					920.0,790.0,690.0,550.0,460.0,390.0,340.0,270.0,230.0,200.0,170.0,140.0,110.0,98.0,85.0,68.0,57.0,49.0,43.0,34.0,
					28.0,25.0,22.0,18.0,14.0,12.0,11.0,8.5,7.1,6.1,5.4,4.3,3.6,3.1};

#define MEM4B()     ((m_udata.data[0]>>0x0)&0x0200)
#define DAC18B()        ((m_udata.data[0]>>0x0)&0x0100)
#define MVOL()      ((m_udata.data[0]>>0x0)&0x000F)
#define RBL()       ((m_udata.data[1]>>0x7)&0x0003)
#define RBP()       ((m_udata.data[1]>>0x0)&0x003F)
#define MOFULL()        ((m_udata.data[2]>>0x0)&0x1000)
#define MOEMPTY()       ((m_udata.data[2]>>0x0)&0x0800)
#define MIOVF()     ((m_udata.data[2]>>0x0)&0x0400)
#define MIFULL()        ((m_udata.data[2]>>0x0)&0x0200)
#define MIEMPTY()       ((m_udata.data[2]>>0x0)&0x0100)

#define SCILV0()        ((m_udata.data[0x24/2]>>0x0)&0xff)
#define SCILV1()        ((m_udata.data[0x26/2]>>0x0)&0xff)
#define SCILV2()        ((m_udata.data[0x28/2]>>0x0)&0xff)

#define SCIEX0  0
#define SCIEX1  1
#define SCIEX2  2
#define SCIMID  3
#define SCIDMA  4
#define SCIIRQ  5
#define SCITMA  6
#define SCITMB  7

#define USEDSP

/* TODO */
//#define dma_transfer_end  ((scsp_regs[0x24/2] & 0x10)>>4)|(((scsp_regs[0x26/2] & 0x10)>>4)<<1)|(((scsp_regs[0x28/2] & 0x10)>>4)<<2)

static const float SDLT[8]={-1000000.0f,-36.0f,-30.0f,-24.0f,-18.0f,-12.0f,-6.0f,0.0f};

const device_type SCSP = &device_creator<scsp_device>;

scsp_device::scsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SCSP, "SCSP", tag, owner, clock, "scsp", __FILE__),
		device_sound_interface(mconfig, *this),
		m_roffset(0),
		m_irq_cb(*this),
		m_main_irq_cb(*this),
		m_BUFPTR(0),
		m_SCSPRAM(nullptr),
		m_SCSPRAM_LENGTH(0),
		m_Master(0),
		m_stream(nullptr),
		m_buffertmpl(nullptr),
		m_buffertmpr(nullptr),
		m_IrqTimA(0),
		m_IrqTimBC(0),
		m_IrqMidi(0),
		m_MidiOutW(0),
		m_MidiOutR(0),
		m_MidiW(0),
		m_MidiR(0),
		m_timerA(nullptr),
		m_timerB(nullptr),
		m_timerC(nullptr),
		m_mcieb(0),
		m_mcipd(0),
		m_bufferl(nullptr),
		m_bufferr(nullptr),
		m_length(0),
		m_RBUFDST(nullptr)
{
	memset(m_RINGBUF, 0, sizeof(m_RINGBUF));
	memset(m_MidiStack, 0, sizeof(m_MidiStack));
	memset(m_LPANTABLE, 0, sizeof(m_LPANTABLE));
	memset(m_RPANTABLE, 0, sizeof(m_RPANTABLE));
	memset(m_TimPris, 0, sizeof(m_TimPris));
	memset(m_ARTABLE, 0, sizeof(m_ARTABLE));
	memset(m_DRTABLE, 0, sizeof(m_DRTABLE));
	memset(m_EG_TABLE, 0, sizeof(m_EG_TABLE));
	memset(m_PLFO_TRI, 0, sizeof(m_PLFO_TRI));
	memset(m_PLFO_SQR, 0, sizeof(m_PLFO_SQR));
	memset(m_PLFO_SAW, 0, sizeof(m_PLFO_SAW));
	memset(m_PLFO_NOI, 0, sizeof(m_PLFO_NOI));
	memset(m_ALFO_TRI, 0, sizeof(m_ALFO_TRI));
	memset(m_ALFO_SQR, 0, sizeof(m_ALFO_SQR));
	memset(m_ALFO_SAW, 0, sizeof(m_ALFO_SAW));
	memset(m_ALFO_NOI, 0, sizeof(m_ALFO_NOI));
	memset(m_PSCALES, 0, sizeof(m_PSCALES));
	memset(m_ASCALES, 0, sizeof(m_ASCALES));
	memset(&m_Slots, 0, sizeof(m_Slots));
	memset(&m_udata.data, 0, sizeof(m_udata.data));
	m_TimCnt[0] = 0;
	m_TimCnt[1] = 0;
	m_TimCnt[2] = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void scsp_device::device_start()
{
	// init the emulation
	init();

	// set up the IRQ callbacks
	m_irq_cb.resolve_safe();
	m_main_irq_cb.resolve_safe();

	m_stream = machine().sound().stream_alloc(*this, 0, 2, 44100);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void scsp_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	m_bufferl = outputs[0];
	m_bufferr = outputs[1];
	m_length = samples;
	DoMasterSamples(samples);
}

unsigned char scsp_device::DecodeSCI(unsigned char irq)
{
	unsigned char SCI=0;
	unsigned char v;
	v=(SCILV0()&(1<<irq))?1:0;
	SCI|=v;
	v=(SCILV1()&(1<<irq))?1:0;
	SCI|=v<<1;
	v=(SCILV2()&(1<<irq))?1:0;
	SCI|=v<<2;
	return SCI;
}

void scsp_device::CheckPendingIRQ()
{
	UINT32 pend=m_udata.data[0x20/2];
	UINT32 en=m_udata.data[0x1e/2];
	if(m_MidiW!=m_MidiR)
	{
		m_udata.data[0x20/2] |= 8;
		pend |= 8;
	}
	if(!pend)
		return;
	if(pend&0x40)
		if(en&0x40)
		{
			m_irq_cb(m_IrqTimA, ASSERT_LINE);
			return;
		}
	if(pend&0x80)
		if(en&0x80)
		{
			m_irq_cb(m_IrqTimBC, ASSERT_LINE);
			return;
		}
	if(pend&0x100)
		if(en&0x100)
		{
			m_irq_cb(m_IrqTimBC, ASSERT_LINE);
			return;
		}
	if(pend&8)
		if (en&8)
		{
			m_irq_cb(m_IrqMidi, ASSERT_LINE);
			m_udata.data[0x20/2] &= ~8;
			return;
		}

	m_irq_cb((offs_t)0, CLEAR_LINE);
}

void scsp_device::MainCheckPendingIRQ(UINT16 irq_type)
{
	m_mcipd |= irq_type;

	//machine().scheduler().synchronize(); // force resync

	if(m_mcipd & m_mcieb)
		m_main_irq_cb(1);
	else
		m_main_irq_cb(0);
}

void scsp_device::ResetInterrupts()
{
	UINT32 reset = m_udata.data[0x22/2];

	if (reset & 0x40)
	{
		m_irq_cb(m_IrqTimA, CLEAR_LINE);
	}
	if (reset & 0x180)
	{
		m_irq_cb(m_IrqTimBC, CLEAR_LINE);
	}
	if (reset & 0x8)
	{
		m_irq_cb(m_IrqMidi, CLEAR_LINE);
	}

	CheckPendingIRQ();
}

TIMER_CALLBACK_MEMBER( scsp_device::timerA_cb )
{
	m_TimCnt[0] = 0xFFFF;
	m_udata.data[0x20/2]|=0x40;
	m_udata.data[0x18/2]&=0xff00;
	m_udata.data[0x18/2]|=m_TimCnt[0]>>8;

	CheckPendingIRQ();
	MainCheckPendingIRQ(0x40);
}

TIMER_CALLBACK_MEMBER( scsp_device::timerB_cb )
{
	m_TimCnt[1] = 0xFFFF;
	m_udata.data[0x20/2]|=0x80;
	m_udata.data[0x1a/2]&=0xff00;
	m_udata.data[0x1a/2]|=m_TimCnt[1]>>8;

	CheckPendingIRQ();
}

TIMER_CALLBACK_MEMBER( scsp_device::timerC_cb )
{
	m_TimCnt[2] = 0xFFFF;
	m_udata.data[0x20/2]|=0x100;
	m_udata.data[0x1c/2]&=0xff00;
	m_udata.data[0x1c/2]|=m_TimCnt[2]>>8;

	CheckPendingIRQ();
}

int scsp_device::Get_AR(int base, int R)
{
	int Rate=base+(R<<1);
	if(Rate>63) Rate=63;
	if(Rate<0) Rate=0;
	return m_ARTABLE[Rate];
}

int scsp_device::Get_DR(int base, int R)
{
	int Rate=base+(R<<1);
	if(Rate>63) Rate=63;
	if(Rate<0) Rate=0;
	return m_DRTABLE[Rate];
}

int scsp_device::Get_RR(int base, int R)
{
	int Rate=base+(R<<1);
	if(Rate>63) Rate=63;
	if(Rate<0) Rate=0;
	return m_DRTABLE[Rate];
}

void scsp_device::Compute_EG(SCSP_SLOT *slot)
{
	int octave=(OCT(slot)^8)-8;
	int rate;
	if(KRS(slot)!=0xf)
		rate=octave+2*KRS(slot)+((FNS(slot)>>9)&1);
	else
		rate=0; //rate=((FNS(slot)>>9)&1);

	slot->EG.volume=0x17F<<EG_SHIFT;
	slot->EG.AR=Get_AR(rate,AR(slot));
	slot->EG.D1R=Get_DR(rate,D1R(slot));
	slot->EG.D2R=Get_DR(rate,D2R(slot));
	slot->EG.RR=Get_RR(rate,RR(slot));
	slot->EG.DL=0x1f-DL(slot);
	slot->EG.EGHOLD=EGHOLD(slot);
}

int scsp_device::EG_Update(SCSP_SLOT *slot)
{
	switch(slot->EG.state)
	{
		case SCSP_ATTACK:
			slot->EG.volume+=slot->EG.AR;
			if(slot->EG.volume>=(0x3ff<<EG_SHIFT))
			{
				if (!LPSLNK(slot))
				{
					slot->EG.state=SCSP_DECAY1;
					if(slot->EG.D1R>=(1024<<EG_SHIFT)) //Skip SCSP_DECAY1, go directly to SCSP_DECAY2
						slot->EG.state=SCSP_DECAY2;
				}
				slot->EG.volume=0x3ff<<EG_SHIFT;
			}
			if(slot->EG.EGHOLD)
				return 0x3ff<<(SHIFT-10);
			break;
		case SCSP_DECAY1:
			slot->EG.volume-=slot->EG.D1R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;
			if(slot->EG.volume>>(EG_SHIFT+5)<=slot->EG.DL)
				slot->EG.state=SCSP_DECAY2;
			break;
		case SCSP_DECAY2:
			if(D2R(slot)==0)
				return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
			slot->EG.volume-=slot->EG.D2R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;

			break;
		case SCSP_RELEASE:
			slot->EG.volume-=slot->EG.RR;
			if(slot->EG.volume<=0)
			{
				slot->EG.volume=0;
				StopSlot(slot,0);
				//slot->EG.volume=0x17F<<EG_SHIFT;
				//slot->EG.state=SCSP_ATTACK;
			}
			break;
		default:
			return 1<<SHIFT;
	}
	return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
}

UINT32 scsp_device::Step(SCSP_SLOT *slot)
{
	int octave=(OCT(slot)^8)-8+SHIFT-10;
	UINT32 Fn=FNS(slot)+(1 << 10);
	if (octave >= 0)
	{
		Fn<<=octave;
	}
	else
	{
		Fn>>=-octave;
	}

	return Fn;
}


void scsp_device::Compute_LFO(SCSP_SLOT *slot)
{
	if(PLFOS(slot)!=0)
		LFO_ComputeStep(&(slot->PLFO),LFOF(slot),PLFOWS(slot),PLFOS(slot),0);
	if(ALFOS(slot)!=0)
		LFO_ComputeStep(&(slot->ALFO),LFOF(slot),ALFOWS(slot),ALFOS(slot),1);
}

void scsp_device::StartSlot(SCSP_SLOT *slot)
{
	UINT32 start_offset;

	slot->active=1;
	start_offset = PCM8B(slot) ? SA(slot) : SA(slot) & 0x7FFFE;
	slot->base=m_SCSPRAM + start_offset;
	slot->cur_addr=0;
	slot->nxt_addr=1<<SHIFT;
	slot->step=Step(slot);
	Compute_EG(slot);
	slot->EG.state=SCSP_ATTACK;
	slot->EG.volume=0x17F<<EG_SHIFT;
	slot->Prev=0;
	slot->Backwards=0;

	Compute_LFO(slot);

//  printf("StartSlot[%p]: SA %x PCM8B %x LPCTL %x ALFOS %x STWINH %x TL %x EFSDL %x\n", slot, SA(slot), PCM8B(slot), LPCTL(slot), ALFOS(slot), STWINH(slot), TL(slot), EFSDL(slot));
}

void scsp_device::StopSlot(SCSP_SLOT *slot,int keyoff)
{
	if(keyoff /*&& slot->EG.state!=SCSP_RELEASE*/)
	{
		slot->EG.state=SCSP_RELEASE;
	}
	else
	{
		slot->active=0;
	}
	slot->udata.data[0]&=~0x800;
}

#define log_base_2(n) (log((double)(n))/log(2.0))

void scsp_device::init()
{
	int i;

	SCSPDSP_Init(&m_DSP);

	m_IrqTimA = m_IrqTimBC = m_IrqMidi = 0;
	m_MidiR=m_MidiW=0;
	m_MidiOutR=m_MidiOutW=0;

	// get SCSP RAM
	if (strcmp(tag(), ":scsp") == 0 || strcmp(tag(), ":scsp1") == 0)
	{
		m_Master=1;
	}
	else
	{
		m_Master=0;
	}

	m_SCSPRAM = region()->base();
	if (m_SCSPRAM)
	{
		m_SCSPRAM_LENGTH = region()->bytes();
		m_DSP.SCSPRAM = (UINT16 *)m_SCSPRAM;
		m_DSP.SCSPRAM_LENGTH = m_SCSPRAM_LENGTH/2;
		m_SCSPRAM += m_roffset;
	}

	m_timerA = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(scsp_device::timerA_cb), this));
	m_timerB = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(scsp_device::timerB_cb), this));
	m_timerC = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(scsp_device::timerC_cb), this));

	for(i=0;i<0x400;++i)
	{
		float envDB=((float)(3*(i-0x3ff)))/32.0f;
		float scale=(float)(1<<SHIFT);
		m_EG_TABLE[i]=(INT32)(powf(10.0f,envDB/20.0f)*scale);
	}

	for(i=0;i<0x10000;++i)
	{
		int iTL =(i>>0x0)&0xff;
		int iPAN=(i>>0x8)&0x1f;
		int iSDL=(i>>0xD)&0x07;
		float TL=1.0f;
		float SegaDB=0.0f;
		float fSDL=1.0f;
		float PAN=1.0f;
		float LPAN,RPAN;

		if(iTL&0x01) SegaDB-=0.4f;
		if(iTL&0x02) SegaDB-=0.8f;
		if(iTL&0x04) SegaDB-=1.5f;
		if(iTL&0x08) SegaDB-=3.0f;
		if(iTL&0x10) SegaDB-=6.0f;
		if(iTL&0x20) SegaDB-=12.0f;
		if(iTL&0x40) SegaDB-=24.0f;
		if(iTL&0x80) SegaDB-=48.0f;

		TL=powf(10.0f,SegaDB/20.0f);

		SegaDB=0;
		if(iPAN&0x1) SegaDB-=3.0f;
		if(iPAN&0x2) SegaDB-=6.0f;
		if(iPAN&0x4) SegaDB-=12.0f;
		if(iPAN&0x8) SegaDB-=24.0f;

		if((iPAN&0xf)==0xf) PAN=0.0;
		else PAN=powf(10.0f,SegaDB/20.0f);

		if(iPAN<0x10)
		{
			LPAN=PAN;
			RPAN=1.0;
		}
		else
		{
			RPAN=PAN;
			LPAN=1.0;
		}

		if(iSDL)
			fSDL=powf(10.0f,(SDLT[iSDL])/20.0f);
		else
			fSDL=0.0;

		m_LPANTABLE[i]=FIX((4.0f*LPAN*TL*fSDL));
		m_RPANTABLE[i]=FIX((4.0f*RPAN*TL*fSDL));
	}

	m_ARTABLE[0]=m_DRTABLE[0]=0;    //Infinite time
	m_ARTABLE[1]=m_DRTABLE[1]=0;    //Infinite time
	for(i=2;i<64;++i)
	{
		double t,step,scale;
		t=ARTimes[i];   //In ms
		if(t!=0.0)
		{
			step=(1023*1000.0)/( 44100.0*t);
			scale=(double) (1<<EG_SHIFT);
			m_ARTABLE[i]=(int) (step*scale);
		}
		else
			m_ARTABLE[i]=1024<<EG_SHIFT;

		t=DRTimes[i];   //In ms
		step=(1023*1000.0)/( 44100.0*t);
		scale=(double) (1<<EG_SHIFT);
		m_DRTABLE[i]=(int) (step*scale);
	}

	// make sure all the slots are off
	for(i=0;i<32;++i)
	{
		m_Slots[i].slot=i;
		m_Slots[i].active=0;
		m_Slots[i].base=nullptr;
		m_Slots[i].EG.state=SCSP_RELEASE;
	}

	LFO_Init();
	m_buffertmpl=auto_alloc_array_clear(machine(), signed int, 44100);
	m_buffertmpr=auto_alloc_array_clear(machine(), signed int, 44100);

	// no "pend"
	m_udata.data[0x20/2] = 0;
	m_TimCnt[0] = 0xffff;
	m_TimCnt[1] = 0xffff;
	m_TimCnt[2] = 0xffff;
}

void scsp_device::UpdateSlotReg(int s,int r)
{
	SCSP_SLOT *slot=m_Slots+s;
	int sl;
	switch(r&0x3f)
	{
		case 0:
		case 1:
			if(KEYONEX(slot))
			{
				for(sl=0;sl<32;++sl)
				{
					SCSP_SLOT *s2=m_Slots+sl;
					{
						if(KEYONB(s2) && s2->EG.state==SCSP_RELEASE/*&& !s2->active*/)
						{
							StartSlot(s2);
						}
						if(!KEYONB(s2) /*&& s2->active*/)
						{
							StopSlot(s2,1);
						}
					}
				}
				slot->udata.data[0]&=~0x1000;
			}
			break;
		case 0x10:
		case 0x11:
			slot->step=Step(slot);
			break;
		case 0xA:
		case 0xB:
			slot->EG.RR=Get_RR(0,RR(slot));
			slot->EG.DL=0x1f-DL(slot);
			break;
		case 0x12:
		case 0x13:
			Compute_LFO(slot);
			break;
	}
}

void scsp_device::UpdateReg(address_space &space, int reg)
{
	switch(reg&0x3f)
	{
		case 0x0:
			m_stream->set_output_gain(0,MVOL() / 15.0);
			m_stream->set_output_gain(1,MVOL() / 15.0);
			break;
		case 0x2:
		case 0x3:
			{
				unsigned int v=RBL();
				m_DSP.RBP=RBP();
				if(v==0)
					m_DSP.RBL=8*1024;
				else if(v==1)
					m_DSP.RBL=16*1024;
				if(v==2)
					m_DSP.RBL=32*1024;
				if(v==3)
					m_DSP.RBL=64*1024;
			}
			break;
		case 0x6:
		case 0x7:
			midi_in(space, 0, m_udata.data[0x6/2]&0xff, 0);
			break;
		case 8:
		case 9:
			/* Only MSLC could be written.  */
			m_udata.data[0x8/2] &= 0xf800; /**< @todo Docs claims MSLC to be 0x7800, but Jikkyou Parodius doesn't agree. */
			break;
		case 0x12:
		case 0x13:
			m_dma.dmea = (m_udata.data[0x12/2] & 0xfffe) | (m_dma.dmea & 0xf0000);
			break;
		case 0x14:
		case 0x15:
			m_dma.dmea = ((m_udata.data[0x14/2] & 0xf000) << 4) | (m_dma.dmea & 0xfffe);
			m_dma.drga = (m_udata.data[0x14/2] & 0x0ffe);
			break;
		case 0x16:
		case 0x17:
			m_dma.dtlg = (m_udata.data[0x16/2] & 0x0ffe);
			m_dma.ddir = (m_udata.data[0x16/2] & 0x2000) >> 13;
			m_dma.dgate = (m_udata.data[0x16/2] & 0x4000) >> 14;
			if(m_udata.data[0x16/2] & 0x1000) // dexe
				exec_dma(space);
			break;
		case 0x18:
		case 0x19:
			if(m_Master)
			{
				UINT32 time;

				m_TimPris[0]=1<<((m_udata.data[0x18/2]>>8)&0x7);
				m_TimCnt[0]=(m_udata.data[0x18/2]&0xff)<<8;

				if ((m_udata.data[0x18/2]&0xff) != 255)
				{
					time = (44100 / m_TimPris[0]) / (255-(m_udata.data[0x18/2]&0xff));
					if (time)
					{
						m_timerA->adjust(attotime::from_hz(time));
					}
				}
			}
			break;
		case 0x1a:
		case 0x1b:
			if(m_Master)
			{
				UINT32 time;

				m_TimPris[1]=1<<((m_udata.data[0x1A/2]>>8)&0x7);
				m_TimCnt[1]=(m_udata.data[0x1A/2]&0xff)<<8;

				if ((m_udata.data[0x1A/2]&0xff) != 255)
				{
					time = (44100 / m_TimPris[1]) / (255-(m_udata.data[0x1A/2]&0xff));
					if (time)
					{
						m_timerB->adjust(attotime::from_hz(time));
					}
				}
			}
			break;
		case 0x1C:
		case 0x1D:
			if(m_Master)
			{
				UINT32 time;

				m_TimPris[2]=1<<((m_udata.data[0x1C/2]>>8)&0x7);
				m_TimCnt[2]=(m_udata.data[0x1C/2]&0xff)<<8;

				if ((m_udata.data[0x1C/2]&0xff) != 255)
				{
					time = (44100 / m_TimPris[2]) / (255-(m_udata.data[0x1C/2]&0xff));
					if (time)
					{
						m_timerC->adjust(attotime::from_hz(time));
					}
				}
			}
			break;
		case 0x1e: // SCIEB
		case 0x1f:
			if(m_Master)
			{
				CheckPendingIRQ();

				if(m_udata.data[0x1e/2] & 0x610)
					popmessage("SCSP SCIEB enabled %04x, contact MAMEdev",m_udata.data[0x1e/2]);
			}
			break;
		case 0x20: // SCIPD
		case 0x21:
			if(m_Master)
			{
				if(m_udata.data[0x1e/2] & m_udata.data[0x20/2] & 0x20)
					popmessage("SCSP SCIPD write %04x, contact MAMEdev",m_udata.data[0x20/2]);
			}
			break;
		case 0x22:  //SCIRE
		case 0x23:

			if(m_Master)
			{
				m_udata.data[0x20/2]&=~m_udata.data[0x22/2];
				ResetInterrupts();

				// behavior from real hardware: if you SCIRE a timer that's expired,
				// it'll immediately pop up again in SCIPD.  ask Sakura Taisen on the Saturn...
				if (m_TimCnt[0] == 0xffff)
				{
					m_udata.data[0x20/2] |= 0x40;
				}
				if (m_TimCnt[1] == 0xffff)
				{
					m_udata.data[0x20/2] |= 0x80;
				}
				if (m_TimCnt[2] == 0xffff)
				{
					m_udata.data[0x20/2] |= 0x100;
				}
			}
			break;
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
			if(m_Master)
			{
				m_IrqTimA=DecodeSCI(SCITMA);
				m_IrqTimBC=DecodeSCI(SCITMB);
				m_IrqMidi=DecodeSCI(SCIMID);
			}
			break;
		case 0x2a:
		case 0x2b:
			m_mcieb = m_udata.data[0x2a/2];

			MainCheckPendingIRQ(0);
			if(m_mcieb & ~0x60)
				popmessage("SCSP MCIEB enabled %04x, contact MAMEdev",m_mcieb);
			break;
		case 0x2c:
		case 0x2d:
			if(m_udata.data[0x2c/2] & 0x20)
				MainCheckPendingIRQ(0x20);
			break;
		case 0x2e:
		case 0x2f:
			m_mcipd &= ~m_udata.data[0x2e/2];
			MainCheckPendingIRQ(0);
			break;

	}
}

void scsp_device::UpdateSlotRegR(int slot,int reg)
{
}

void scsp_device::UpdateRegR(address_space &space, int reg)
{
	switch(reg&0x3f)
	{
		case 4:
		case 5:
			{
				unsigned short v=m_udata.data[0x5/2];
				v&=0xff00;
				v|=m_MidiStack[m_MidiR];
				m_irq_cb(m_IrqMidi, CLEAR_LINE);   // cancel the IRQ
				logerror("Read %x from SCSP MIDI\n", v);
				if(m_MidiR!=m_MidiW)
				{
					++m_MidiR;
					m_MidiR&=31;
				}
				m_udata.data[0x5/2]=v;
			}
			break;
		case 8:
		case 9:
			{
				// MSLC     |  CA   |SGC|EG
				// f e d c b a 9 8 7 6 5 4 3 2 1 0
				unsigned char MSLC=(m_udata.data[0x8/2]>>11)&0x1f;
				SCSP_SLOT *slot=m_Slots + MSLC;
				unsigned int SGC = (slot->EG.state) & 3;
				unsigned int CA = (slot->cur_addr>>(SHIFT+12)) & 0xf;
				unsigned int EG = (0x1f - (slot->EG.volume>>(EG_SHIFT+5))) & 0x1f;
				/* note: according to the manual MSLC is write only, CA, SGC and EG read only.  */
				m_udata.data[0x8/2] =  /*(MSLC << 11) |*/ (CA << 7) | (SGC << 5) | EG;
			}
			break;

		case 0x18:
		case 0x19:
			break;

		case 0x1a:
		case 0x1b:
			break;

		case 0x1c:
		case 0x1d:
			break;

		case 0x2a:
		case 0x2b:
			m_udata.data[0x2a/2] = m_mcieb;
			break;

		case 0x2c:
		case 0x2d:
			m_udata.data[0x2c/2] = m_mcipd;
			break;
	}
}

void scsp_device::w16(address_space &space,unsigned int addr,unsigned short val)
{
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		*((unsigned short *) (m_Slots[slot].udata.datab+(addr))) = val;
		UpdateSlotReg(slot,addr&0x1f);
	}
	else if(addr<0x600)
	{
		if (addr < 0x430)
		{
			*((unsigned short *) (m_udata.datab+((addr&0x3f)))) = val;
			UpdateReg(space, addr&0x3f);
		}
	}
	else if(addr<0x700)
		m_RINGBUF[(addr-0x600)/2]=val;
	else
	{
		//DSP
		if(addr<0x780)  //COEF
			*((unsigned short *) (m_DSP.COEF+(addr-0x700)/2))=val;
		else if(addr<0x7c0)
			*((unsigned short *) (m_DSP.MADRS+(addr-0x780)/2))=val;
		else if(addr<0x800) // MADRS is mirrored twice
			*((unsigned short *) (m_DSP.MADRS+(addr-0x7c0)/2))=val;
		else if(addr<0xC00)
		{
			*((unsigned short *) (m_DSP.MPRO+(addr-0x800)/2))=val;

			if(addr==0xBF0)
			{
				SCSPDSP_Start(&m_DSP);
			}
		}
	}
}

unsigned short scsp_device::r16(address_space &space, unsigned int addr)
{
	unsigned short v=0;
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		UpdateSlotRegR(slot,addr&0x1f);
		v=*((unsigned short *) (m_Slots[slot].udata.datab+(addr)));
	}
	else if(addr<0x600)
	{
		if (addr < 0x430)
		{
			UpdateRegR(space, addr&0x3f);
			v= *((unsigned short *) (m_udata.datab+((addr&0x3f))));
		}
	}
	else if(addr<0x700)
		v=m_RINGBUF[(addr-0x600)/2];
	else
	{
		//DSP
		if(addr<0x780)  //COEF
			v= *((unsigned short *) (m_DSP.COEF+(addr-0x700)/2));
		else if(addr<0x7c0)
			v= *((unsigned short *) (m_DSP.MADRS+(addr-0x780)/2));
		else if(addr<0x800)
			v= *((unsigned short *) (m_DSP.MADRS+(addr-0x7c0)/2));
		else if(addr<0xC00)
			v= *((unsigned short *) (m_DSP.MPRO+(addr-0x800)/2));
		else if(addr<0xE00)
		{
			if(addr & 2)
				v= m_DSP.TEMP[(addr >> 2) & 0x7f] & 0xffff;
			else
				v= m_DSP.TEMP[(addr >> 2) & 0x7f] >> 16;
		}
		else if(addr<0xE80)
		{
			if(addr & 2)
				v= m_DSP.MEMS[(addr >> 2) & 0x1f] & 0xffff;
			else
				v= m_DSP.MEMS[(addr >> 2) & 0x1f] >> 16;
		}
		else if(addr<0xEC0)
		{
			if(addr & 2)
				v= m_DSP.MIXS[(addr >> 2) & 0xf] & 0xffff;
			else
				v= m_DSP.MIXS[(addr >> 2) & 0xf] >> 16;
		}
		else if(addr<0xEE0)
			v= *((unsigned short *) (m_DSP.EFREG+(addr-0xec0)/2));
		else
		{
			/**!
			@todo Kyuutenkai reads from 0xee0/0xee2, it's tied with EXTS register(s) also used for CD-Rom Player equalizer.
			This port is actually an external parallel port, directly connected from the CD Block device, hence code is a bit of an hack.
			Kyuutenkai code snippet for reference:
			004A3A: 207C 0010 0EE0             movea.l #$100ee0, A0
			004A40: 43EA 0090                  lea     ($90,A2), A1 ;A2=0x700
			004A44: 6100 0254                  bsr     $4c9a
			004A48: 207C 0010 0EE2             movea.l #$100ee2, A0
			004A4E: 43EA 0092                  lea     ($92,A2), A1
			004A52: 6100 0246                  bsr     $4c9a
			004A56: 207C 0010 0ED2             movea.l #$100ed2, A0
			004A5C: 43EA 0094                  lea     ($94,A2), A1
			004A60: 6100 0238                  bsr     $4c9a
			004A64: 3540 0096                  move.w  D0, ($96,A2)
			004A68: 207C 0010 0ED4             movea.l #$100ed4, A0
			004A6E: 43EA 0098                  lea     ($98,A2), A1
			004A72: 6100 0226                  bsr     $4c9a
			004A76: 3540 009A                  move.w  D0, ($9a,A2)
			004A7A: 207C 0010 0ED6             movea.l #$100ed6, A0
			004A80: 43EA 009C                  lea     ($9c,A2), A1
			004A84: 6100 0214                  bsr     $4c9a
			004A88: 3540 009E                  move.w  D0, ($9e,A2)
			004A8C: 4E75                       rts

			    004C9A: 48E7 4000                  movem.l D1, -(A7)
			    004C9E: 3010                       move.w  (A0), D0 ;reads from 0x100ee0/ee2
			    004CA0: 4A40                       tst.w   D0
			    004CA2: 6A00 0004                  bpl     $4ca8
			    004CA6: 4440                       neg.w   D0
			    004CA8: 3211                       move.w  (A1), D1
			    004CAA: D041                       add.w   D1, D0
			    004CAC: E248                       lsr.w   #1, D0
			    004CAE: 3280                       move.w  D0, (A1) ;writes to RAM buffer 0x790/0x792
			    004CB0: 4CDF 0002                  movem.l (A7)+, D1
			    004CB4: 4E75                       rts
			*/
			logerror("SCSP: Reading from EXTS register %08x\n",addr);
			if(addr == 0xee0)
				v = space.machine().device<cdda_device>("cdda")->get_channel_volume(0);
			if(addr == 0xee2)
				v = space.machine().device<cdda_device>("cdda")->get_channel_volume(1);
		}
	}
	return v;
}


#define REVSIGN(v) ((~v)+1)

inline INT32 scsp_device::UpdateSlot(SCSP_SLOT *slot)
{
	INT32 sample;
	int step=slot->step;
	UINT32 addr1,addr2,addr_select;                                   // current and next sample addresses
	UINT32 *addr[2]      = {&addr1, &addr2};                          // used for linear interpolation
	UINT32 *slot_addr[2] = {&(slot->cur_addr), &(slot->nxt_addr)};    //

	if(SSCTL(slot)!=0)  //no FM or noise yet
		return 0;

	if(PLFOS(slot)!=0)
	{
		step=step*PLFO_Step(&(slot->PLFO));
		step>>=SHIFT;
	}

	if(PCM8B(slot))
	{
		addr1=slot->cur_addr>>SHIFT;
		addr2=slot->nxt_addr>>SHIFT;
	}
	else
	{
		addr1=(slot->cur_addr>>(SHIFT-1))&0x7fffe;
		addr2=(slot->nxt_addr>>(SHIFT-1))&0x7fffe;
	}

	if(MDL(slot)!=0 || MDXSL(slot)!=0 || MDYSL(slot)!=0)
	{
		INT32 smp=(m_RINGBUF[(m_BUFPTR+MDXSL(slot))&63]+m_RINGBUF[(m_BUFPTR+MDYSL(slot))&63])/2;

		smp<<=0xA; // associate cycle with 1024
		smp>>=0x1A-MDL(slot); // ex. for MDL=0xF, sample range corresponds to +/- 64 pi (32=2^5 cycles) so shift by 11 (16-5 == 0x1A-0xF)
		if(!PCM8B(slot)) smp<<=1;

		addr1+=smp; addr2+=smp;
	}

	if(PCM8B(slot)) //8 bit signed
	{
		INT8 *p1=(signed char *) (m_SCSPRAM+BYTE_XOR_BE(((SA(slot)+addr1))&0x7FFFF));
		INT8 *p2=(signed char *) (m_SCSPRAM+BYTE_XOR_BE(((SA(slot)+addr2))&0x7FFFF));
		//sample=(p[0])<<8;
		INT32 s;
		INT32 fpart=slot->cur_addr&((1<<SHIFT)-1);
		s=(int) (p1[0]<<8)*((1<<SHIFT)-fpart)+(int) (p2[0]<<8)*fpart;
		sample=(s>>SHIFT);
	}
	else    //16 bit signed (endianness?)
	{
		INT16 *p1=(signed short *) (m_SCSPRAM+((SA(slot)+addr1)&0x7FFFE));
		INT16 *p2=(signed short *) (m_SCSPRAM+((SA(slot)+addr2)&0x7FFFE));
		INT32 s;
		INT32 fpart=slot->cur_addr&((1<<SHIFT)-1);
		s=(int)(p1[0])*((1<<SHIFT)-fpart)+(int)(p2[0])*fpart;
		sample=(s>>SHIFT);
	}

	if(SBCTL(slot)&0x1)
		sample ^= 0x7FFF;
	if(SBCTL(slot)&0x2)
		sample = (INT16)(sample^0x8000);

	if(slot->Backwards)
		slot->cur_addr-=step;
	else
		slot->cur_addr+=step;
	slot->nxt_addr=slot->cur_addr+(1<<SHIFT);

	addr1=slot->cur_addr>>SHIFT;
	addr2=slot->nxt_addr>>SHIFT;

	if(addr1>=LSA(slot) && !(slot->Backwards))
	{
		if(LPSLNK(slot) && slot->EG.state==SCSP_ATTACK)
			slot->EG.state = SCSP_DECAY1;
	}

	for (addr_select=0;addr_select<2;addr_select++)
	{
		INT32 rem_addr;
		switch(LPCTL(slot))
		{
		case 0: //no loop
			if(*addr[addr_select]>=LSA(slot) && *addr[addr_select]>=LEA(slot))
			{
			//slot->active=0;
			StopSlot(slot,0);
			}
			break;
		case 1: //normal loop
			if(*addr[addr_select]>=LEA(slot))
			{
				rem_addr = *slot_addr[addr_select] - (LEA(slot)<<SHIFT);
				*slot_addr[addr_select]=(LSA(slot)<<SHIFT) + rem_addr;
			}
			break;
		case 2: //reverse loop
			if((*addr[addr_select]>=LSA(slot)) && !(slot->Backwards))
			{
				rem_addr = *slot_addr[addr_select] - (LSA(slot)<<SHIFT);
				*slot_addr[addr_select]=(LEA(slot)<<SHIFT) - rem_addr;
				slot->Backwards=1;
			}
			else if((*addr[addr_select]<LSA(slot) || (*slot_addr[addr_select]&0x80000000)) && slot->Backwards)
			{
				rem_addr = (LSA(slot)<<SHIFT) - *slot_addr[addr_select];
				*slot_addr[addr_select]=(LEA(slot)<<SHIFT) - rem_addr;
			}
			break;
		case 3: //ping-pong
			if(*addr[addr_select]>=LEA(slot)) //reached end, reverse till start
			{
				rem_addr = *slot_addr[addr_select] - (LEA(slot)<<SHIFT);
				*slot_addr[addr_select]=(LEA(slot)<<SHIFT) - rem_addr;
				slot->Backwards=1;
			}
			else if((*addr[addr_select]<LSA(slot) || (*slot_addr[addr_select]&0x80000000)) && slot->Backwards)//reached start or negative
			{
				rem_addr = (LSA(slot)<<SHIFT) - *slot_addr[addr_select];
				*slot_addr[addr_select]=(LSA(slot)<<SHIFT) + rem_addr;
				slot->Backwards=0;
			}
			break;
		}
	}

	if(!SDIR(slot))
	{
		if(ALFOS(slot)!=0)
		{
			sample=sample*ALFO_Step(&(slot->ALFO));
			sample>>=SHIFT;
		}

		if(slot->EG.state==SCSP_ATTACK)
			sample=(sample*EG_Update(slot))>>SHIFT;
		else
			sample=(sample*m_EG_TABLE[EG_Update(slot)>>(SHIFT-10)])>>SHIFT;
	}

	if(!STWINH(slot))
	{
		if(!SDIR(slot))
		{
			unsigned short Enc=((TL(slot))<<0x0)|(0x7<<0xd);
			*m_RBUFDST=(sample*m_LPANTABLE[Enc])>>(SHIFT+1);
		}
		else
		{
			unsigned short Enc=(0<<0x0)|(0x7<<0xd);
			*m_RBUFDST=(sample*m_LPANTABLE[Enc])>>(SHIFT+1);
		}
	}

	return sample;
}

void scsp_device::DoMasterSamples(int nsamples)
{
	stream_sample_t *bufr,*bufl;
	int sl, s, i;

	bufr=m_bufferr;
	bufl=m_bufferl;

	for(s=0;s<nsamples;++s)
	{
		INT32 smpl, smpr;

		smpl = smpr = 0;

		for(sl=0;sl<32;++sl)
		{
#if FM_DELAY
			m_RBUFDST=m_DELAYBUF+m_DELAYPTR;
#else
			m_RBUFDST=m_RINGBUF+m_BUFPTR;
#endif
			if(m_Slots[sl].active)
			{
				SCSP_SLOT *slot=m_Slots+sl;
				unsigned short Enc;
				signed int sample;

				sample=UpdateSlot(slot);

				Enc=((TL(slot))<<0x0)|((IMXL(slot))<<0xd);
				SCSPDSP_SetSample(&m_DSP,(sample*m_LPANTABLE[Enc])>>(SHIFT-2),ISEL(slot),IMXL(slot));
				Enc=((TL(slot))<<0x0)|((DIPAN(slot))<<0x8)|((DISDL(slot))<<0xd);
				{
					smpl+=(sample*m_LPANTABLE[Enc])>>SHIFT;
					smpr+=(sample*m_RPANTABLE[Enc])>>SHIFT;
				}
			}

#if FM_DELAY
			m_RINGBUF[(m_BUFPTR+64-(FM_DELAY-1))&63] = m_DELAYBUF[(m_DELAYPTR+FM_DELAY-(FM_DELAY-1))%FM_DELAY];
#endif
			++m_BUFPTR;
			m_BUFPTR&=63;
#if FM_DELAY
			++m_DELAYPTR;
			if(m_DELAYPTR>FM_DELAY-1) m_DELAYPTR=0;
#endif
		}

		SCSPDSP_Step(&m_DSP);

		for(i=0;i<16;++i)
		{
			SCSP_SLOT *slot=m_Slots+i;
			if(EFSDL(slot))
			{
				unsigned short Enc=((EFPAN(slot))<<0x8)|((EFSDL(slot))<<0xd);
				smpl+=(m_DSP.EFREG[i]*m_LPANTABLE[Enc])>>SHIFT;
				smpr+=(m_DSP.EFREG[i]*m_RPANTABLE[Enc])>>SHIFT;
			}
		}

		*bufl++ = ICLIP16(smpl>>2);
		*bufr++ = ICLIP16(smpr>>2);
	}
}

/* TODO: this needs to be timer-ized */
void scsp_device::exec_dma(address_space &space)
{
	static UINT16 tmp_dma[3];
	int i;

	logerror("SCSP: DMA transfer START\n"
				"DMEA: %04x DRGA: %04x DTLG: %04x\n"
				"DGATE: %d  DDIR: %d\n",m_dma.dmea,m_dma.drga,m_dma.dtlg,m_dma.dgate ? 1 : 0,m_dma.ddir ? 1 : 0);

	/* Copy the dma values in a temp storage for resuming later */
		/* (DMA *can't* overwrite its parameters).                  */
	if(!(m_dma.ddir))
	{
		for(i=0;i<3;i++)
			tmp_dma[i] = m_udata.data[(0x12+(i*2))/2];
	}

	/* note: we don't use space.read_word / write_word because it can happen that SH-2 enables the DMA instead of m68k. */
	/* TODO: don't know if params auto-updates, I guess not ... */
	if(m_dma.ddir)
	{
		if(m_dma.dgate)
		{
			popmessage("Check: SCSP DMA DGATE enabled, contact MAME/MESSdev");
			for(i=0;i < m_dma.dtlg;i+=2)
			{
				m_SCSPRAM[m_dma.dmea] = 0;
				m_SCSPRAM[m_dma.dmea+1] = 0;
				m_dma.dmea+=2;
			}
		}
		else
		{
			for(i=0;i < m_dma.dtlg;i+=2)
			{
				UINT16 tmp;
				tmp = r16(space, m_dma.drga);
				m_SCSPRAM[m_dma.dmea] = tmp & 0xff;
				m_SCSPRAM[m_dma.dmea+1] = tmp>>8;
				m_dma.dmea+=2;
				m_dma.drga+=2;
			}
		}
	}
	else
	{
		if(m_dma.dgate)
		{
			popmessage("Check: SCSP DMA DGATE enabled, contact MAME/MESSdev");
			for(i=0;i < m_dma.dtlg;i+=2)
			{
				w16(space, m_dma.drga, 0);
				m_dma.drga+=2;
			}
		}
		else
		{
			for(i=0;i < m_dma.dtlg;i+=2)
			{
				UINT16 tmp;
				tmp = m_SCSPRAM[m_dma.dmea];
				tmp|= m_SCSPRAM[m_dma.dmea+1]<<8;
				w16(space, m_dma.drga, tmp);
				m_dma.dmea+=2;
				m_dma.drga+=2;
			}
		}
	}

	/*Resume the values*/
	if(!(m_dma.ddir))
	{
		for(i=0;i<3;i++)
			m_udata.data[(0x12+(i*2))/2] = tmp_dma[i];
	}

	/* Job done */
	m_udata.data[0x16/2] &= ~0x1000;
	/* request a dma end irq (TODO: make it inside the interface) */
	if(m_udata.data[0x1e/2] & 0x10)
	{
		popmessage("SCSP DMA IRQ triggered, contact MAMEdev");
		space.machine().device("audiocpu")->execute().set_input_line(DecodeSCI(SCIDMA),HOLD_LINE);
	}
}

#ifdef UNUSED_FUNCTION
int IRQCB(void *param)
{
	CheckPendingIRQ(param);
	return -1;
}
#endif


void scsp_device::set_ram_base(void *base)
{
	if (this)
	{
		m_SCSPRAM = (unsigned char *)base;
		m_DSP.SCSPRAM = (UINT16 *)base;
		m_SCSPRAM_LENGTH = 0x80000;
		m_DSP.SCSPRAM_LENGTH = 0x80000/2;
	}
}


READ16_MEMBER( scsp_device::read )
{
	m_stream->update();
	return r16(space, offset*2);
}

WRITE16_MEMBER( scsp_device::write )
{
	UINT16 tmp;

	m_stream->update();

	tmp = r16(space, offset*2);
	COMBINE_DATA(&tmp);
	w16(space,offset*2, tmp);
}

WRITE16_MEMBER( scsp_device::midi_in )
{
	//    printf("scsp_midi_in: %02x\n", data);

	m_MidiStack[m_MidiW++]=data;
	m_MidiW &= 31;

	CheckPendingIRQ();
}

READ16_MEMBER( scsp_device::midi_out_r )
{
	unsigned char val;

	val=m_MidiStack[m_MidiR++];
	m_MidiR&=31;
	return val;
}

//LFO handling

#define LFIX(v) ((unsigned int) ((float) (1<<LFO_SHIFT)*(v)))

//Convert DB to multiply amplitude
#define DB(v)   LFIX(powf(10.0f,v/20.0f))

//Convert cents to step increment
#define CENTS(v) LFIX(powf(2.0f,v/1200.0f))


static const float LFOFreq[32]=
{
	0.17f,0.19f,0.23f,0.27f,0.34f,0.39f,0.45f,0.55f,0.68f,0.78f,0.92f,1.10f,1.39f,1.60f,1.87f,2.27f,
	2.87f,3.31f,3.92f,4.79f,6.15f,7.18f,8.60f,10.8f,14.4f,17.2f,21.5f,28.7f,43.1f,57.4f,86.1f,172.3f
};
static const float ASCALE[8]={0.0f,0.4f,0.8f,1.5f,3.0f,6.0f,12.0f,24.0f};
static const float PSCALE[8]={0.0f,7.0f,13.5f,27.0f,55.0f,112.0f,230.0f,494.0f};


void scsp_device::LFO_Init()
{
	int i,s;
	for(i=0;i<256;++i)
	{
		int a,p;
//      float TL;
		//Saw
		a=255-i;
		if(i<128)
			p=i;
		else
			p=i-256;
		m_ALFO_SAW[i]=a;
		m_PLFO_SAW[i]=p;

		//Square
		if(i<128)
		{
			a=255;
			p=127;
		}
		else
		{
			a=0;
			p=-128;
		}
		m_ALFO_SQR[i]=a;
		m_PLFO_SQR[i]=p;

		//Tri
		if(i<128)
			a=255-(i*2);
		else
			a=(i*2)-256;
		if(i<64)
			p=i*2;
		else if(i<128)
			p=255-i*2;
		else if(i<192)
			p=256-i*2;
		else
			p=i*2-511;
		m_ALFO_TRI[i]=a;
		m_PLFO_TRI[i]=p;

		//noise
		//a=lfo_noise[i];
		a=machine().rand()&0xff;
		p=128-a;
		m_ALFO_NOI[i]=a;
		m_PLFO_NOI[i]=p;
	}

	for(s=0;s<8;++s)
	{
		float limit=PSCALE[s];
		for(i=-128;i<128;++i)
		{
			m_PSCALES[s][i+128]=CENTS(((limit*(float) i)/128.0f));
		}
		limit=-ASCALE[s];
		for(i=0;i<256;++i)
		{
			m_ASCALES[s][i]=DB(((limit*(float) i)/256.0f));
		}
	}
}

signed int scsp_device::PLFO_Step(SCSP_LFO_t *LFO)
{
	int p;
	LFO->phase+=LFO->phase_step;
#if LFO_SHIFT!=8
	LFO->phase&=(1<<(LFO_SHIFT+8))-1;
#endif
	p=LFO->table[LFO->phase>>LFO_SHIFT];
	p=LFO->scale[p+128];
	return p<<(SHIFT-LFO_SHIFT);
}

signed int scsp_device::ALFO_Step(SCSP_LFO_t *LFO)
{
	int p;
	LFO->phase+=LFO->phase_step;
#if LFO_SHIFT!=8
	LFO->phase&=(1<<(LFO_SHIFT+8))-1;
#endif
	p=LFO->table[LFO->phase>>LFO_SHIFT];
	p=LFO->scale[p];
	return p<<(SHIFT-LFO_SHIFT);
}

void scsp_device::LFO_ComputeStep(SCSP_LFO_t *LFO,UINT32 LFOF,UINT32 LFOWS,UINT32 LFOS,int ALFO)
{
	float step=(float) LFOFreq[LFOF]*256.0f/(float)44100;
	LFO->phase_step=(unsigned int) ((float) (1<<LFO_SHIFT)*step);
	if(ALFO)
	{
		switch(LFOWS)
		{
			case 0: LFO->table=m_ALFO_SAW; break;
			case 1: LFO->table=m_ALFO_SQR; break;
			case 2: LFO->table=m_ALFO_TRI; break;
			case 3: LFO->table=m_ALFO_NOI; break;
		}
		LFO->scale=m_ASCALES[LFOS];
	}
	else
	{
		switch(LFOWS)
		{
			case 0: LFO->table=m_PLFO_SAW; break;
			case 1: LFO->table=m_PLFO_SQR; break;
			case 2: LFO->table=m_PLFO_TRI; break;
			case 3: LFO->table=m_PLFO_NOI; break;
		}
		LFO->scale=m_PSCALES[LFOS];
	}
}
