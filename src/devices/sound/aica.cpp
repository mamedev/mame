// license:BSD-3-Clause
// copyright-holders:ElSemi, Deunan Knute, R. Belmont
// thanks-to: kingshriek
/*
    Sega/Yamaha AICA emulation

    This is effectively a 64-voice SCSP, with the following differences:
    - No FM mode
    - A third sample format (ADPCM) has been added
    - Some minor other tweeks (no EGHOLD, slighly more capable DSP)
*/

#include "emu.h"
#include "aica.h"

#define ICLIP16(x) (x<-32768)?-32768:((x>32767)?32767:x)

#define SHIFT   12
#define FIX(v)  ((UINT32) ((float) (1<<SHIFT)*(v)))

#define EG_SHIFT    16
#define LFO_SHIFT   8

#define LFIX(v) ((unsigned int) ((float) (1<<LFO_SHIFT)*(v)))

//Convert DB to multiply amplitude
#define DB(v)   LFIX(powf(10.0f,v/20.0f))

//Convert cents to step increment
#define CENTS(v) LFIX(powf(2.0f,v/1200.0f))

/*
    AICA features 64 programmable slots
    that can generate PCM and ADPCM (from ROM/RAM) sound
*/

//SLOT PARAMETERS
#define KEYONEX(slot)       ((slot->udata.data[0x0]>>0x0)&0x8000)
#define KEYONB(slot)        ((slot->udata.data[0x0]>>0x0)&0x4000)
#define SSCTL(slot)     ((slot->udata.data[0x0]>>0xA)&0x0001)
#define LPCTL(slot)     ((slot->udata.data[0x0]>>0x9)&0x0001)
#define PCMS(slot)      ((slot->udata.data[0x0]>>0x7)&0x0003)

#define SA(slot)        (((slot->udata.data[0x0]&0x7F)<<16)|(slot->udata.data[0x4/2]))

#define LSA(slot)       (slot->udata.data[0x8/2])

#define LEA(slot)       (slot->udata.data[0xc/2])

#define D2R(slot)       ((slot->udata.data[0x10/2]>>0xB)&0x001F)
#define D1R(slot)       ((slot->udata.data[0x10/2]>>0x6)&0x001F)
#define AR(slot)        ((slot->udata.data[0x10/2]>>0x0)&0x001F)

#define LPSLNK(slot)        ((slot->udata.data[0x14/2]>>0x0)&0x4000)
#define KRS(slot)       ((slot->udata.data[0x14/2]>>0xA)&0x000F)
#define DL(slot)        ((slot->udata.data[0x14/2]>>0x5)&0x001F)
#define RR(slot)        ((slot->udata.data[0x14/2]>>0x0)&0x001F)

#define TL(slot)        ((slot->udata.data[0x28/2]>>0x8)&0x00FF)

#define OCT(slot)       ((slot->udata.data[0x18/2]>>0xB)&0x000F)
#define FNS(slot)       ((slot->udata.data[0x18/2]>>0x0)&0x03FF)

#define LFORE(slot)     ((slot->udata.data[0x1c/2]>>0x0)&0x8000)
#define LFOF(slot)      ((slot->udata.data[0x1c/2]>>0xA)&0x001F)
#define PLFOWS(slot)        ((slot->udata.data[0x1c/2]>>0x8)&0x0003)
#define PLFOS(slot)     ((slot->udata.data[0x1c/2]>>0x5)&0x0007)
#define ALFOWS(slot)        ((slot->udata.data[0x1c/2]>>0x3)&0x0003)
#define ALFOS(slot)     ((slot->udata.data[0x1c/2]>>0x0)&0x0007)

#define ISEL(slot)      ((slot->udata.data[0x20/2]>>0x0)&0x000F)
#define IMXL(slot)      ((slot->udata.data[0x20/2]>>0x4)&0x000F)

#define DISDL(slot)     ((slot->udata.data[0x24/2]>>0x8)&0x000F)
#define DIPAN(slot)     ((slot->udata.data[0x24/2]>>0x0)&0x001F)

#define EFSDL(slot)     ((m_EFSPAN[slot*4]>>8)&0x000f)
#define EFPAN(slot)     ((m_EFSPAN[slot*4]>>0)&0x001f)

//Envelope times in ms
static const double ARTimes[64]={100000/*infinity*/,100000/*infinity*/,8100.0,6900.0,6000.0,4800.0,4000.0,3400.0,3000.0,2400.0,2000.0,1700.0,1500.0,
					1200.0,1000.0,860.0,760.0,600.0,500.0,430.0,380.0,300.0,250.0,220.0,190.0,150.0,130.0,110.0,95.0,
					76.0,63.0,55.0,47.0,38.0,31.0,27.0,24.0,19.0,15.0,13.0,12.0,9.4,7.9,6.8,6.0,4.7,3.8,3.4,3.0,2.4,
					2.0,1.8,1.6,1.3,1.1,0.93,0.85,0.65,0.53,0.44,0.40,0.35,0.0,0.0};
static const double DRTimes[64]={100000/*infinity*/,100000/*infinity*/,118200.0,101300.0,88600.0,70900.0,59100.0,50700.0,44300.0,35500.0,29600.0,25300.0,22200.0,17700.0,
					14800.0,12700.0,11100.0,8900.0,7400.0,6300.0,5500.0,4400.0,3700.0,3200.0,2800.0,2200.0,1800.0,1600.0,1400.0,1100.0,
					920.0,790.0,690.0,550.0,460.0,390.0,340.0,270.0,230.0,200.0,170.0,140.0,110.0,98.0,85.0,68.0,57.0,49.0,43.0,34.0,
					28.0,25.0,22.0,18.0,14.0,12.0,11.0,8.5,7.1,6.1,5.4,4.3,3.6,3.1};

#define MEM4B(aica)     ((m_udata.data[0]>>0x0)&0x0200)
#define DAC18B(aica)        ((m_udata.data[0]>>0x0)&0x0100)
#define MVOL(aica)      ((m_udata.data[0]>>0x0)&0x000F)
#define RBL(aica)       ((m_udata.data[2]>>0xD)&0x0003)
#define RBP(aica)       ((m_udata.data[2]>>0x0)&0x0fff)
#define MOFULL(aica)        ((m_udata.data[4]>>0x0)&0x1000)
#define MOEMPTY(aica)       ((m_udata.data[4]>>0x0)&0x0800)
#define MIOVF(aica)     ((m_udata.data[4]>>0x0)&0x0400)
#define MIFULL(aica)        ((m_udata.data[4]>>0x0)&0x0200)
#define MIEMPTY(aica)       ((m_udata.data[4]>>0x0)&0x0100)

#define AFSEL(aica)     ((m_udata.data[0xc/2]>>0x0)&0x4000)
#define MSLC(aica)      ((m_udata.data[0xc/2]>>0x8)&0x3F)

#define SCILV0(aica)        ((m_udata.data[0xa8/2]>>0x0)&0xff)
#define SCILV1(aica)        ((m_udata.data[0xac/2]>>0x0)&0xff)
#define SCILV2(aica)        ((m_udata.data[0xb0/2]>>0x0)&0xff)

#define MCIEB(aica)     ((m_udata.data[0xb4/2]>>0x0)&0xff)
#define MCIPD(aica)     ((m_udata.data[0xb8/2]>>0x0)&0xff)
#define MCIRE(aica)     ((m_udata.data[0xbc/2]>>0x0)&0xff)

#define SCIEX0  0
#define SCIEX1  1
#define SCIEX2  2
#define SCIMID  3
#define SCIDMA  4
#define SCIIRQ  5
#define SCITMA  6
#define SCITMB  7

static const float SDLT[16]={-1000000.0,-42.0,-39.0,-36.0,-33.0,-30.0,-27.0,-24.0,-21.0,-18.0,-15.0,-12.0,-9.0,-6.0,-3.0,0.0};

unsigned char aica_device::DecodeSCI(unsigned char irq)
{
	unsigned char SCI=0;
	unsigned char v;
	v=(SCILV0((AICA))&(1<<irq))?1:0;
	SCI|=v;
	v=(SCILV1((AICA))&(1<<irq))?1:0;
	SCI|=v<<1;
	v=(SCILV2((AICA))&(1<<irq))?1:0;
	SCI|=v<<2;
	return SCI;
}

void aica_device::ResetInterrupts()
{
#if 0
	UINT32 reset = m_udata.data[0xa4/2];

	if (reset & 0x40)
		m_irq_cb(-m_IrqTimA);
	if (reset & 0x180)
		m_irq_cb(-m_IrqTimBC);
#endif
}

void aica_device::CheckPendingIRQ()
{
	UINT32 pend=m_udata.data[0xa0/2];
	UINT32 en=m_udata.data[0x9c/2];

	if(m_MidiW!=m_MidiR)
	{
		m_IRQL = m_IrqMidi;
		m_irq_cb(1);
		return;
	}
	if(!pend)
		return;
	if(pend&0x40)
		if(en&0x40)
		{
			m_IRQL = m_IrqTimA;
			m_irq_cb(1);
			return;
		}
	if(pend&0x80)
		if(en&0x80)
		{
			m_IRQL = m_IrqTimBC;
			m_irq_cb(1);
			return;
		}
	if(pend&0x100)
		if(en&0x100)
		{
			m_IRQL = m_IrqTimBC;
			m_irq_cb(1);
			return;
		}
}

void aica_device::CheckPendingIRQ_SH4()
{
	if(m_mcipd & m_mcieb)
		m_main_irq_cb(1);

	if((m_mcipd & m_mcieb) == 0)
		m_main_irq_cb(0);
}

TIMER_CALLBACK_MEMBER( aica_device::timerA_cb )
{
	m_TimCnt[0] = 0xFFFF;
	m_udata.data[0xa0/2]|=0x40;
	m_mcipd |= 0x40;
	m_udata.data[0x90/2]&=0xff00;
	m_udata.data[0x90/2]|=m_TimCnt[0]>>8;

	CheckPendingIRQ();
	CheckPendingIRQ_SH4();

}

TIMER_CALLBACK_MEMBER( aica_device::timerB_cb )
{
	m_TimCnt[1] = 0xFFFF;
	m_udata.data[0xa0/2]|=0x80;
	m_mcipd |= 0x80;
	m_udata.data[0x94/2]&=0xff00;
	m_udata.data[0x94/2]|=m_TimCnt[1]>>8;

	CheckPendingIRQ();
	CheckPendingIRQ_SH4();
}

TIMER_CALLBACK_MEMBER( aica_device::timerC_cb )
{
	m_TimCnt[2] = 0xFFFF;
	m_udata.data[0xa0/2]|=0x100;
	m_mcipd |= 0x100;
	m_udata.data[0x98/2]&=0xff00;
	m_udata.data[0x98/2]|=m_TimCnt[2]>>8;

	CheckPendingIRQ();
	CheckPendingIRQ_SH4();
}

int aica_device::Get_AR(int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63) Rate=63;
	if(Rate<0) Rate=0;
	return m_ARTABLE[Rate];
}

int aica_device::Get_DR(int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63) Rate=63;
	if(Rate<0) Rate=0;
	return m_DRTABLE[Rate];
}

int aica_device::Get_RR(int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63) Rate=63;
	if(Rate<0) Rate=0;
	return m_DRTABLE[Rate];
}

void aica_device::Compute_EG(AICA_SLOT *slot)
{
	int octave=(OCT(slot)^8)-8;
	int rate;
	if(KRS(slot)!=0xf)
		rate=octave+2*KRS(slot)+((FNS(slot)>>9)&1);
	else
		rate=0; //rate=((FNS(slot)>>9)&1);

	slot->EG.volume=0x17f<<EG_SHIFT;
	slot->EG.AR=Get_AR(rate,AR(slot));
	slot->EG.D1R=Get_DR(rate,D1R(slot));
	slot->EG.D2R=Get_DR(rate,D2R(slot));
	slot->EG.RR=Get_RR(rate,RR(slot));
	slot->EG.RR=Get_RR(rate,RR(slot));
	slot->EG.DL=0x1f-DL(slot);
}

int aica_device::EG_Update(AICA_SLOT *slot)
{
	switch(slot->EG.state)
	{
		case AICA_ATTACK:
			slot->EG.volume+=slot->EG.AR;
			if(slot->EG.volume>=(0x3ff<<EG_SHIFT))
			{
				if (!LPSLNK(slot) && slot->EG.D1R)
				{
					slot->EG.state=AICA_DECAY1;
					if(slot->EG.D1R>=(1024<<EG_SHIFT) && slot->EG.D2R) //Skip DECAY1, go directly to DECAY2
						slot->EG.state=AICA_DECAY2;
				}
				slot->EG.volume=0x3ff<<EG_SHIFT;
			}
			break;
		case AICA_DECAY1:
			slot->EG.volume-=slot->EG.D1R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;
			if(slot->EG.volume>>(EG_SHIFT+5)<=slot->EG.DL)
				slot->EG.state=AICA_DECAY2;
			break;
		case AICA_DECAY2:
			if(D2R(slot)==0)
				return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
			slot->EG.volume-=slot->EG.D2R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;

			break;
		case AICA_RELEASE:
			slot->EG.volume-=slot->EG.RR;
			if(slot->EG.volume<=0)
			{
				slot->EG.volume=0;
				StopSlot(slot,0);
//              slot->EG.volume=0x17f<<EG_SHIFT;
//              slot->EG.state=AICA_ATTACK;
			}
			break;
		default:
			return 1<<SHIFT;
	}
	return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
}

UINT32 aica_device::Step(AICA_SLOT *slot)
{
	int octave=(OCT(slot)^8)-8+SHIFT-10;
	UINT32 Fn=FNS(slot) + (0x400);
	if (octave >= 0)
		Fn<<=octave;
	else
		Fn>>=-octave;
	return Fn;
}


void aica_device::Compute_LFO(AICA_SLOT *slot)
{
	if(PLFOS(slot)!=0)
		AICALFO_ComputeStep(&(slot->PLFO),LFOF(slot),PLFOWS(slot),PLFOS(slot),0);
	if(ALFOS(slot)!=0)
		AICALFO_ComputeStep(&(slot->ALFO),LFOF(slot),ALFOWS(slot),ALFOS(slot),1);
}

#define ADPCMSHIFT  8
#define ADFIX(f)    (int) ((float) f*(float) (1<<ADPCMSHIFT))

static const int TableQuant[8]={ADFIX(0.8984375),ADFIX(0.8984375),ADFIX(0.8984375),ADFIX(0.8984375),ADFIX(1.19921875),ADFIX(1.59765625),ADFIX(2.0),ADFIX(2.3984375)};
static const int quant_mul[16]= { 1, 3, 5, 7, 9, 11, 13, 15, -1, -3, -5, -7, -9, -11, -13, -15};

void aica_device::InitADPCM(int *PrevSignal, int *PrevQuant)
{
	*PrevSignal=0;
	*PrevQuant=0x7f;
}

signed short aica_device::DecodeADPCM(int *PrevSignal, unsigned char Delta, int *PrevQuant)
{
	int x = *PrevQuant * quant_mul [Delta & 15];
		x = *PrevSignal + ((int)(x + ((UINT32)x >> 29)) >> 3);
	*PrevSignal=ICLIP16(x);
	*PrevQuant=(*PrevQuant*TableQuant[Delta&7])>>ADPCMSHIFT;
	*PrevQuant=(*PrevQuant<0x7f)?0x7f:((*PrevQuant>0x6000)?0x6000:*PrevQuant);
	return *PrevSignal;
}

void aica_device::StartSlot(AICA_SLOT *slot)
{
	UINT64 start_offset;

	slot->active=1;
	slot->Backwards=0;
	slot->cur_addr=0; slot->nxt_addr=1<<SHIFT; slot->prv_addr=-1;
	start_offset = SA(slot);    // AICA can play 16-bit samples from any boundry
	slot->base=&m_AICARAM[start_offset];
	slot->step=Step(slot);
	Compute_EG(slot);
	slot->EG.state=AICA_ATTACK;
	slot->EG.volume=0x17f<<EG_SHIFT;
	Compute_LFO(slot);

	if (PCMS(slot) >= 2)
	{
		slot->curstep = 0;
		slot->adbase = (unsigned char *) (m_AICARAM+((SA(slot))&0x7fffff));
		InitADPCM(&(slot->cur_sample), &(slot->cur_quant));
		InitADPCM(&(slot->cur_lpsample), &(slot->cur_lpquant));

		// on real hardware this creates undefined behavior.
		if (LSA(slot) > LEA(slot))
		{
			slot->udata.data[0xc/2] = 0xffff;
		}
	}
}

void aica_device::StopSlot(AICA_SLOT *slot,int keyoff)
{
	if(keyoff /*&& slot->EG.state!=AICA_RELEASE*/)
	{
		slot->EG.state=AICA_RELEASE;
	}
	else
	{
		slot->active=0;
		slot->lpend = 1;
	}
	slot->udata.data[0]&=~0x4000;
}

#define log_base_2(n) (log((float) n)/log((float) 2))

void aica_device::Init()
{
	int i;

	m_IrqTimA = m_IrqTimBC = m_IrqMidi = 0;
	m_MidiR=m_MidiW=0;
	m_MidiOutR=m_MidiOutW=0;

	// get AICA RAM
	{
		m_AICARAM = region()->base();
		if (m_AICARAM)
		{
			m_AICARAM += m_roffset;
			m_AICARAM_LENGTH = region()->bytes();
			m_RAM_MASK = m_AICARAM_LENGTH-1;
			m_RAM_MASK16 = m_RAM_MASK & 0x7ffffe;
			m_DSP.AICARAM = (UINT16 *)m_AICARAM;
			m_DSP.AICARAM_LENGTH = m_AICARAM_LENGTH/2;
		}
	}

	m_timerA = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aica_device::timerA_cb), this));
	m_timerB = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aica_device::timerB_cb), this));
	m_timerC = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aica_device::timerC_cb), this));

	for(i=0;i<0x400;++i)
	{
		float envDB=((float)(3*(i-0x3ff)))/32.0f;
		float scale=(float)(1<<SHIFT);
		m_EG_TABLE[i]=(INT32)(powf(10.0f,envDB/20.0f)*scale);
	}

	for(i=0;i<0x20000;++i)
	{
		int iTL =(i>>0x0)&0xff;
		int iPAN=(i>>0x8)&0x1f;
		int iSDL=(i>>0xD)&0x0F;
		float TL;
		float SegaDB=0;
		float fSDL;
		float PAN;
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
			step=(1023*1000.0)/(44100.0*t);
			scale=(double) (1<<EG_SHIFT);
			m_ARTABLE[i]=(int) (step*scale);
		}
		else
			m_ARTABLE[i]=1024<<EG_SHIFT;

		t=DRTimes[i];   //In ms
		step=(1023*1000.0)/(44100.0*t);
		scale=(double) (1<<EG_SHIFT);
		m_DRTABLE[i]=(int) (step*scale);
	}

	// make sure all the slots are off
	for(i=0;i<64;++i)
	{
		m_Slots[i].slot=i;
		m_Slots[i].active=0;
		m_Slots[i].base=nullptr;
		m_Slots[i].EG.state=AICA_RELEASE;
		m_Slots[i].lpend=1;
	}

	AICALFO_Init();
	m_buffertmpl=make_unique_clear<INT32[]>(44100);
	m_buffertmpr=make_unique_clear<INT32[]>(44100);

	// no "pend"
	m_udata.data[0xa0/2] = 0;
	//AICA[1].udata.data[0x20/2] = 0;
	m_TimCnt[0] = 0xffff;
	m_TimCnt[1] = 0xffff;
	m_TimCnt[2] = 0xffff;
}

void aica_device::UpdateSlotReg(int s,int r)
{
	AICA_SLOT *slot=m_Slots+s;
	int sl;
	switch(r&0x7f)
	{
		case 0:
		case 1:
			if(KEYONEX(slot))
			{
				for(sl=0;sl<64;++sl)
				{
					AICA_SLOT *s2=m_Slots+sl;
					{
						if(KEYONB(s2) && s2->EG.state==AICA_RELEASE/*&& !s2->active*/)
						{
							s2->lpend = 0;
							StartSlot(s2);
							#if 0
							printf("StartSlot[%02X]:   SSCTL %01X SA %06X LSA %04X LEA %04X PCMS %01X LPCTL %01X\n",sl,SSCTL(s2),SA(s2),LSA(s2),LEA(s2),PCMS(s2),LPCTL(s2));
							printf("                 AR %02X D1R %02X D2R %02X RR %02X DL %02X KRS %01X LPSLNK %01X\n",AR(s2),D1R(s2),D2R(s2),RR(s2),DL(s2),KRS(s2),LPSLNK(s2)>>14);
							printf("                 TL %02X OCT %01X FNS %03X\n",TL(s2),OCT(s2),FNS(s2));
							printf("                 LFORE %01X LFOF %02X ALFOWS %01X ALFOS %01X PLFOWS %01X PLFOS %01X\n",LFORE(s2),LFOF(s2),ALFOWS(s2),ALFOS(s2),PLFOWS(s2),PLFOS(s2));
							printf("                 IMXL %01X ISEL %01X DISDL %01X DIPAN %02X\n",IMXL(s2),ISEL(s2),DISDL(s2),DIPAN(s2));
							printf("\n");
							fflush(stdout);
							#endif
						}
						if(!KEYONB(s2) /*&& s2->active*/)
						{
							StopSlot(s2,1);
						}
					}
				}
				slot->udata.data[0]&=~0x8000;
			}
			break;
		case 0x18:
		case 0x19:
			slot->step=Step(slot);
			break;
		case 0x14:
		case 0x15:
			slot->EG.RR=Get_RR(0,RR(slot));
			slot->EG.DL=0x1f-DL(slot);
			break;
		case 0x1c:
		case 0x1d:
			Compute_LFO(slot);
			break;
		case 0x24:
//          printf("[%02d]: %x to DISDL/DIPAN (PC=%x)\n", s, slot->udata.data[0x24/2], arm7_get_register(15));
			break;
	}
}

void aica_device::UpdateReg(address_space &space, int reg)
{
	switch(reg&0xff)
	{
		case 0x4:
		case 0x5:
			{
				unsigned int v=RBL();
				m_DSP.RBP=RBP();
				if(v==0)
					m_DSP.RBL=8*1024;
				else if(v==1)
					m_DSP.RBL=16*1024;
				else if(v==2)
					m_DSP.RBL=32*1024;
				else if(v==3)
					m_DSP.RBL=64*1024;
			}
			break;
		case 0x8:
		case 0x9:
			midi_in(space, 0, m_udata.data[0x8/2]&0xff, 0xffff);
			break;

		//case 0x0c:
		//case 0x0d:
		//  printf("%04x\n",m_udata.data[0xc/2]);
		//  break;

		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			break;

		case 0x80:
		case 0x81:
			m_dma.dmea = ((m_udata.data[0x80/2] & 0xfe00) << 7) | (m_dma.dmea & 0xfffc);
			/* TODO: $TSCD - MRWINH regs */
			break;

		case 0x84:
		case 0x85:
			m_dma.dmea = (m_udata.data[0x84/2] & 0xfffc) | (m_dma.dmea & 0x7f0000);
			break;

		case 0x88:
		case 0x89:
			m_dma.drga = (m_udata.data[0x88/2] & 0x7ffc);
			m_dma.dgate = (m_udata.data[0x88/2] & 0x8000) >> 15;
			break;

		case 0x8c:
		case 0x8d:
			m_dma.dlg = (m_udata.data[0x8c/2] & 0x7ffc);
			m_dma.ddir = (m_udata.data[0x8c/2] & 0x8000) >> 15;
			if(m_udata.data[0x8c/2] & 1) // dexe
				aica_exec_dma(space);
			break;

		case 0x90:
		case 0x91:
			if(m_master)
			{
				UINT32 time;

				m_TimPris[0]=1<<((m_udata.data[0x90/2]>>8)&0x7);
				m_TimCnt[0]=(m_udata.data[0x90/2]&0xff)<<8;

				if ((m_udata.data[0x90/2]&0xff) != 255)
				{
					time = (44100 / m_TimPris[0]) / (255-(m_udata.data[0x90/2]&0xff));
					if (time)
					{
						m_timerA->adjust(attotime::from_hz(time));
					}
				}
			}
			break;
		case 0x94:
		case 0x95:
			if(m_master)
			{
				UINT32 time;

				m_TimPris[1]=1<<((m_udata.data[0x94/2]>>8)&0x7);
				m_TimCnt[1]=(m_udata.data[0x94/2]&0xff)<<8;

				if ((m_udata.data[0x94/2]&0xff) != 255)
				{
					time = (44100 / m_TimPris[1]) / (255-(m_udata.data[0x94/2]&0xff));
					if (time)
					{
						m_timerB->adjust(attotime::from_hz(time));
					}
				}
			}
			break;
		case 0x98:
		case 0x99:
			if(m_master)
			{
				UINT32 time;

				m_TimPris[2]=1<<((m_udata.data[0x98/2]>>8)&0x7);
				m_TimCnt[2]=(m_udata.data[0x98/2]&0xff)<<8;

				if ((m_udata.data[0x98/2]&0xff) != 255)
				{
					time = (44100 / m_TimPris[2]) / (255-(m_udata.data[0x98/2]&0xff));
					if (time)
					{
						m_timerC->adjust(attotime::from_hz(time));
					}
				}
			}
			break;

		case 0x9c: //SCIEB
		case 0x9d:
			if(m_udata.data[0x9c/2] & 0x631)
				popmessage("AICA: SCIEB enabled %04x, contact MAME/MESSdev",m_udata.data[0x9c/2]);
			break;

		case 0xa4:  //SCIRE
		case 0xa5:

			if(m_master)
			{
				m_udata.data[0xa0/2] &= ~m_udata.data[0xa4/2];
				ResetInterrupts();

				// behavior from real hardware (SCSP, assumed to carry over): if you SCIRE a timer that's expired,
				// it'll immediately pop up again
				if (m_TimCnt[0] >= 0xff00)
				{
					m_udata.data[0xa0/2] |= 0x40;
				}
				if (m_TimCnt[1] >= 0xff00)
				{
					m_udata.data[0xa0/2] |= 0x80;
				}
				if (m_TimCnt[2] >= 0xff00)
				{
					m_udata.data[0xa0/2] |= 0x100;
				}
			}
			break;
		case 0xa8:
		case 0xa9:
		case 0xac:
		case 0xad:
		case 0xb0:
		case 0xb1:
			if(m_master)
			{
				m_IrqTimA=DecodeSCI(SCITMA);
				m_IrqTimBC=DecodeSCI(SCITMB);
				m_IrqMidi=DecodeSCI(SCIMID);
			}
			break;

		case 0xb4: //MCIEB
		case 0xb5:
			if(m_udata.data[0xb4/2] & 0x7df)
				popmessage("AICA: MCIEB enabled %04x, contact MAME/MESSdev",m_udata.data[0xb4/2]);
			m_mcieb = m_udata.data[0xb4/2];
			CheckPendingIRQ_SH4();
			break;

		case 0xb8:
		case 0xb9:
			if(m_udata.data[0xb8/2] & 0x20)
				m_mcipd |= 0x20;
			CheckPendingIRQ_SH4();
			break;

		case 0xbc:
		case 0xbd:
			m_mcipd &= ~m_udata.data[0xbc/2];
			CheckPendingIRQ_SH4();
			break;
	}
}

void aica_device::UpdateSlotRegR(int slot,int reg)
{
}

void aica_device::UpdateRegR(address_space &space, int reg)
{
	switch(reg&0xff)
	{
		case 8:
		case 9:
			{
				unsigned short v=m_udata.data[0x8/2];
				v&=0xff00;
				v|=m_MidiStack[m_MidiR];
				m_irq_cb(0);    // cancel the IRQ
				if(m_MidiR!=m_MidiW)
				{
					++m_MidiR;
					m_MidiR&=15;
				}
				m_udata.data[0x8/2]=v;
			}
			break;

		case 0x10:  // LP check
		case 0x11:
			{
				int slotnum = MSLC();
				AICA_SLOT *slot=m_Slots + slotnum;
				UINT16 LP;
				if (!(AFSEL()))
				{
					UINT16 SGC;
					int EG;

					LP = slot->lpend ? 0x8000 : 0x0000;
					slot->lpend = 0;
					SGC = (slot->EG.state << 13) & 0x6000;
					EG = slot->active ? slot->EG.volume : 0;
					EG >>= (EG_SHIFT - 13);
					EG = 0x1FFF - EG;
					if (EG < 0) EG = 0;

					m_udata.data[0x10/2] = (EG & 0x1FF8) | SGC | LP;
				}
				else
				{
					LP = slot->lpend ? 0x8000 : 0x0000;
					m_udata.data[0x10/2] = LP;
				}
			}
			break;

		case 0x14:  // CA (slot address)
		case 0x15:
			{
				//m_stream->update();
				int slotnum = MSLC();
				AICA_SLOT *slot=m_Slots+slotnum;
				unsigned int CA;

				if (PCMS(slot) == 0)    // 16-bit samples
				{
					CA = (slot->cur_addr>>(SHIFT-1))&m_RAM_MASK16;
				}
				else    // 8-bit PCM and 4-bit ADPCM
				{
					CA = (slot->cur_addr>>SHIFT)&m_RAM_MASK;
				}

				//printf("%08x %08x\n",CA,slot->cur_addr&m_RAM_MASK16);

				m_udata.data[0x14/2] = CA;
			}
			break;
		case 0xb8:
		case 0xb9:
			m_udata.data[0xb8/2] = m_mcipd;
			break;
	}
}

void aica_device::w16(address_space &space,unsigned int addr,unsigned short val)
{
	addr&=0xffff;
	if(addr<0x2000)
	{
		int slot=addr/0x80;
		addr&=0x7f;
//      printf("%x to slot %d offset %x\n", val, slot, addr);
		*((unsigned short *) (m_Slots[slot].udata.datab+(addr))) = val;
		UpdateSlotReg(slot,addr&0x7f);
	}
	else if (addr < 0x2800)
	{
		if (addr <= 0x2044)
		{
//          printf("%x to EFSxx slot %d (addr %x)\n", val, (addr-0x2000)/4, addr&0x7f);
			m_EFSPAN[addr&0x7f] = val;
		}
	}
	else if(addr<0x3000)
	{
		if (addr < 0x28be)
		{
//          printf("%x to AICA global @ %x\n", val, addr & 0xff);
			*((unsigned short *) (m_udata.datab+((addr&0xff)))) = val;
			UpdateReg(space, addr&0xff);

		}
		else if (addr == 0x2d00)
		{
			m_IRQL = val;
			popmessage("AICA: write to IRQL?");
		}
		else if (addr == 0x2d04)
		{
			m_IRQR = val;

			if (val & 1)
			{
				m_irq_cb(0);
			}
			if (val & 0x100)
				popmessage("AICA: SH-4 write protection enabled!");

			if (val & 0xfefe)
				popmessage("AICA: IRQR %04x!",val);
		}
	}
	else
	{
		//DSP
		if(addr<0x3200) //COEF
			*((unsigned short *) (m_DSP.COEF+(addr-0x3000)/2))=val;
		else if(addr<0x3400)
			*((unsigned short *) (m_DSP.MADRS+(addr-0x3200)/2))=val;
		else if(addr<0x3c00)
		{
			*((unsigned short *) (m_DSP.MPRO+(addr-0x3400)/2))=val;

			if (addr == 0x3bfe)
			{
				aica_dsp_start(&m_DSP);
			}
		}
		else if(addr<0x4000)
		{
			popmessage("AICADSP write to undocumented reg %04x -> %04x",addr,val);
		}
		else if(addr<0x4400)
		{
			if(addr & 4)
				m_DSP.TEMP[(addr >> 3) & 0x7f] = (m_DSP.TEMP[(addr >> 3) & 0x7f] & 0xffff0000) | (val & 0xffff);
			else
				m_DSP.TEMP[(addr >> 3) & 0x7f] = (m_DSP.TEMP[(addr >> 3) & 0x7f] & 0xffff) | (val << 16);
		}
		else if(addr<0x4500)
		{
			if(addr & 4)
				m_DSP.MEMS[(addr >> 3) & 0x1f] = (m_DSP.MEMS[(addr >> 3) & 0x1f] & 0xffff0000) | (val & 0xffff);
			else
				m_DSP.MEMS[(addr >> 3) & 0x1f] = (m_DSP.MEMS[(addr >> 3) & 0x1f] & 0xffff) | (val << 16);
		}
		else if(addr<0x4580)
		{
			if(addr & 4)
				m_DSP.MIXS[(addr >> 3) & 0xf] = (m_DSP.MIXS[(addr >> 3) & 0xf] & 0xffff0000) | (val & 0xffff);
			else
				m_DSP.MIXS[(addr >> 3) & 0xf] = (m_DSP.MIXS[(addr >> 3) & 0xf] & 0xffff) | (val << 16);
		}
		else if(addr<0x45c0)
			*((unsigned short *) (m_DSP.EFREG+(addr-0x4580)/4))=val;
		else if(addr<0x45c8)
			*((unsigned short *) (m_DSP.EXTS+(addr-0x45c0)/2))=val;
	}
}

unsigned short aica_device::r16(address_space &space, unsigned int addr)
{
	unsigned short v=0;
	addr&=0xffff;
	if(addr<0x2000)
	{
		int slot=addr/0x80;
		addr&=0x7f;
		UpdateSlotRegR(slot,addr&0x7f);
		v=*((unsigned short *) (m_Slots[slot].udata.datab+(addr)));
	}
	else if(addr<0x3000)
	{
		if (addr <= 0x2044)
		{
			v = m_EFSPAN[addr&0x7f];
		}
		else if (addr < 0x28be)
		{
			UpdateRegR(space, addr&0xff);
			v= *((unsigned short *) (m_udata.datab+((addr&0xff))));
			if((addr&0xfffe)==0x2810) m_udata.data[0x10/2] &= 0x7FFF;   // reset LP on read
		}
		else if (addr == 0x2d00)
		{
			return m_IRQL;
		}
		else if (addr == 0x2d04)
		{
			//popmessage("AICA: read to IRQR?");
			return m_IRQR;
		}
	}
	else
	{
		if(addr<0x3200) //COEF
			v= *((unsigned short *) (m_DSP.COEF+(addr-0x3000)/2));
		else if(addr<0x3400)
			v= *((unsigned short *) (m_DSP.MADRS+(addr-0x3200)/2));
		else if(addr<0x3c00)
			v= *((unsigned short *) (m_DSP.MPRO+(addr-0x3400)/2));
		else if(addr<0x4000)
		{
			v= 0xffff;
			popmessage("AICADSP read to undocumented reg %04x",addr);
		}
		else if(addr<0x4400)
		{
			if(addr & 4)
				v= m_DSP.TEMP[(addr >> 3) & 0x7f] & 0xffff;
			else
				v= m_DSP.TEMP[(addr >> 3) & 0x7f] >> 16;
		}
		else if(addr<0x4500)
		{
			if(addr & 4)
				v= m_DSP.MEMS[(addr >> 3) & 0x1f] & 0xffff;
			else
				v= m_DSP.MEMS[(addr >> 3) & 0x1f] >> 16;
		}
		else if(addr<0x4580)
		{
			if(addr & 4)
				v= m_DSP.MIXS[(addr >> 3) & 0xf] & 0xffff;
			else
				v= m_DSP.MIXS[(addr >> 3) & 0xf] >> 16;
		}
		else if(addr<0x45c0)
			v = *((unsigned short *) (m_DSP.EFREG+(addr-0x4580)/4));
		else if(addr<0x45c8)
			v = *((unsigned short *) (m_DSP.EXTS+(addr-0x45c0)/2));
	}
//  else if (addr<0x700)
//      v=m_RINGBUF[(addr-0x600)/2];
	return v;
}


#define REVSIGN(v) ((~v)+1)

#ifdef UNUSED_FUNCTION
void aica_device::TimersAddTicks(int ticks)
{
	if(m_TimCnt[0]<=0xff00)
	{
		m_TimCnt[0] += ticks << (8-((m_udata.data[0x18/2]>>8)&0x7));
		if (m_TimCnt[0] > 0xFF00)
		{
			m_TimCnt[0] = 0xFFFF;
			m_udata.data[0xa0/2]|=0x40;
		}
		m_udata.data[0x90/2]&=0xff00;
		m_udata.data[0x90/2]|=m_TimCnt[0]>>8;
	}

	if(m_TimCnt[1]<=0xff00)
	{
		m_TimCnt[1] += ticks << (8-((m_udata.data[0x1a/2]>>8)&0x7));
		if (m_TimCnt[1] > 0xFF00)
		{
			m_TimCnt[1] = 0xFFFF;
			m_udata.data[0xa0/2]|=0x80;
		}
		m_udata.data[0x94/2]&=0xff00;
		m_udata.data[0x94/2]|=m_TimCnt[1]>>8;
	}

	if(m_TimCnt[2]<=0xff00)
	{
		m_TimCnt[2] += ticks << (8-((m_udata.data[0x1c/2]>>8)&0x7));
		if (m_TimCnt[2] > 0xFF00)
		{
			m_TimCnt[2] = 0xFFFF;
			m_udata.data[0xa0/2]|=0x100;
		}
		m_udata.data[0x98/2]&=0xff00;
		m_udata.data[0x98/2]|=m_TimCnt[2]>>8;
	}
}
#endif

INT32 aica_device::UpdateSlot(AICA_SLOT *slot)
{
	INT32 sample;
	int step=slot->step;
	UINT32 addr1,addr2,addr_select;                                   // current and next sample addresses
	UINT32 *addr[2]      = {&addr1, &addr2};                          // used for linear interpolation
	UINT32 *slot_addr[2] = {&(slot->cur_addr), &(slot->nxt_addr)};    //
	UINT32 chanlea = LEA(slot);

	if(SSCTL(slot)!=0)  //no FM or noise yet
		return 0;

	if(PCMS(slot) == 3) // Red Dog music relies on this
		chanlea = (chanlea + 3) & ~3;

	if(PLFOS(slot)!=0)
	{
		step=step*AICAPLFO_Step(&(slot->PLFO));
		step>>=SHIFT;
	}

	if(PCMS(slot) == 1)
	{
		addr1=slot->cur_addr>>SHIFT;
		addr2=slot->nxt_addr>>SHIFT;
	}
	else if(PCMS(slot) == 0)
	{
		addr1=(slot->cur_addr>>(SHIFT-1))&m_RAM_MASK16;
		addr2=(slot->nxt_addr>>(SHIFT-1))&m_RAM_MASK16;
	}
	else
	{
		addr1=slot->cur_addr>>SHIFT;
		addr2=slot->nxt_addr>>SHIFT;
	}

	if(PCMS(slot) == 1) // 8-bit signed
	{
		INT8 *p1=(signed char *) (m_AICARAM+(((SA(slot)+addr1))&m_RAM_MASK));
		INT8 *p2=(signed char *) (m_AICARAM+(((SA(slot)+addr2))&m_RAM_MASK));
		INT32 s;
		INT32 fpart=slot->cur_addr&((1<<SHIFT)-1);
		s=(int) (p1[0]<<8)*((1<<SHIFT)-fpart)+(int) (p2[0]<<8)*fpart;
		sample=(s>>SHIFT);
	}
	else if (PCMS(slot) == 0)   //16 bit signed
	{
		UINT8 *p1=(UINT8 *) (m_AICARAM+((SA(slot)+addr1)&m_RAM_MASK));
		UINT8 *p2=(UINT8 *) (m_AICARAM+((SA(slot)+addr2)&m_RAM_MASK));
		INT32 s;
		INT32 fpart=slot->cur_addr&((1<<SHIFT)-1);
		s=(int) ((INT16)(p1[0] | (p1[1]<<8)))*((1<<SHIFT)-fpart)+(int) ((INT16)(p2[0] | (p2[1]<<8)))*fpart;
		sample=(s>>SHIFT);
	}
	else    // 4-bit ADPCM
	{
		UINT8 *base= slot->adbase;
		INT32 s;
		int cur_sample;       //current ADPCM sample
		int nxt_sample;       //next ADPCM sample
		INT32 fpart=slot->cur_addr&((1<<SHIFT)-1);
		UINT32 steps_to_go = addr1 > addr2 ? chanlea : addr2, curstep = slot->curstep;

		if (slot->adbase)
		{
			cur_sample = slot->cur_sample; // may already contains current decoded sample

			// seek to the interpolation sample
			while (curstep < steps_to_go)
			{
				int shift1 = 4 & (curstep << 2);
				unsigned char delta1 = (*base>>shift1)&0xf;
				DecodeADPCM(&(slot->cur_sample),delta1,&(slot->cur_quant));
				if (!(++curstep & 1))
					base++;
				if (curstep == addr1)
					cur_sample = slot->cur_sample;
				if (curstep == LSA(slot))
				{
					slot->cur_lpsample = slot->cur_sample;
					slot->cur_lpquant = slot->cur_quant;
				}
			}
			nxt_sample = slot->cur_sample;

			slot->adbase = base;
			slot->curstep = curstep;

			s=(int)cur_sample*((1<<SHIFT)-fpart)+(int)nxt_sample*fpart;
		}
		else
		{
			s = 0;
		}

		sample=(s>>SHIFT);
	}

	slot->prv_addr=slot->cur_addr;
	slot->cur_addr+=step;
	slot->nxt_addr=slot->cur_addr+(1<<SHIFT);

	addr1=slot->cur_addr>>SHIFT;
	addr2=slot->nxt_addr>>SHIFT;

	if(addr1>=LSA(slot))
	{
		if(LPSLNK(slot) && slot->EG.state==AICA_ATTACK && slot->EG.D1R)
			slot->EG.state = AICA_DECAY1;
	}

	for (addr_select=0; addr_select<2; addr_select++)
	{
		INT32 rem_addr;
		switch(LPCTL(slot))
		{
		case 0: //no loop
			if(*addr[addr_select]>=LSA(slot) && *addr[addr_select]>=chanlea)
			{
				StopSlot(slot,0);
			}
			break;
		case 1: //normal loop
			if(*addr[addr_select]>=chanlea)
			{
				slot->lpend = 1;
				rem_addr = *slot_addr[addr_select] - (chanlea<<SHIFT);
				*slot_addr[addr_select]=(LSA(slot)<<SHIFT) + rem_addr;

				if(PCMS(slot)>=2 && addr_select == 0)
				{
					// restore the state @ LSA - the sampler will naturally walk to (LSA + remainder)
					slot->adbase = &m_AICARAM[SA(slot)+(LSA(slot)/2)];
					slot->curstep = LSA(slot);
					if (PCMS(slot) == 2)
					{
						slot->cur_sample = slot->cur_lpsample;
						slot->cur_quant = slot->cur_lpquant;
					}

//                  printf("Looping: slot_addr %x LSA %x LEA %x step %x base %x\n", *slot_addr[addr_select]>>SHIFT, LSA(slot), LEA(slot), slot->curstep, slot->adbase);
				}
			}
			break;
		}
	}

	if(ALFOS(slot)!=0)
	{
		sample=sample*AICAALFO_Step(&(slot->ALFO));
		sample>>=SHIFT;
	}

	if(slot->EG.state==AICA_ATTACK)
		sample=(sample*EG_Update(slot))>>SHIFT;
	else
		sample=(sample*m_EG_TABLE[EG_Update(slot)>>(SHIFT-10)])>>SHIFT;

	return sample;
}

void aica_device::DoMasterSamples(int nsamples)
{
	stream_sample_t *bufr,*bufl;
	int sl, s, i;

	bufr=m_bufferr;
	bufl=m_bufferl;

	for(s=0;s<nsamples;++s)
	{
		INT32 smpl, smpr;

		smpl = smpr = 0;

		// mix slots' direct output
		for(sl=0;sl<64;++sl)
		{
			AICA_SLOT *slot=m_Slots+sl;
			m_RBUFDST=m_RINGBUF+m_BUFPTR;
			if(m_Slots[sl].active)
			{
				unsigned int Enc;
				signed int sample;

				sample=UpdateSlot(slot);

				Enc=((TL(slot))<<0x0)|((IMXL(slot))<<0xd);
				aica_dsp_setsample(&m_DSP,(sample*m_LPANTABLE[Enc])>>(SHIFT-2),ISEL(slot),IMXL(slot));
				Enc=((TL(slot))<<0x0)|((DIPAN(slot))<<0x8)|((DISDL(slot))<<0xd);
				{
					smpl+=(sample*m_LPANTABLE[Enc])>>SHIFT;
					smpr+=(sample*m_RPANTABLE[Enc])>>SHIFT;
				}
			}

			m_BUFPTR&=63;
		}

		// process the DSP
		aica_dsp_step(&m_DSP);

		// mix DSP output
		for(i=0;i<16;++i)
		{
			if(EFSDL(i))
			{
				unsigned int Enc=((EFPAN(i))<<0x8)|((EFSDL(i))<<0xd);
				smpl+=(m_DSP.EFREG[i]*m_LPANTABLE[Enc])>>SHIFT;
				smpr+=(m_DSP.EFREG[i]*m_RPANTABLE[Enc])>>SHIFT;
			}
		}

		*bufl++ = ICLIP16(smpl>>3);
		*bufr++ = ICLIP16(smpr>>3);
	}
}

/* TODO: this needs to be timer-ized */
void aica_device::aica_exec_dma(address_space &space)
{
	static UINT16 tmp_dma[4];
	int i;

	printf("AICA: DMA transfer START\n"
				"DMEA: %08x DRGA: %08x DLG: %04x\n"
				"DGATE: %d  DDIR: %d\n",m_dma.dmea,m_dma.drga,m_dma.dlg,m_dma.dgate,m_dma.ddir);

	/* Copy the dma values in a temp storage for resuming later */
		/* (DMA *can't* overwrite its parameters).                  */
	if(!(m_dma.ddir))
	{
		for(i=0;i<4;i++)
			tmp_dma[i] = m_udata.data[(0x80+(i*4))/2];
	}

	/* note: we don't use space.read_word / write_word because it can happen that SH-4 enables the DMA instead of ARM like in DCLP tester. */
	/* TODO: don't know if params auto-updates, I guess not ... */
	if(m_dma.ddir)
	{
		if(m_dma.dgate)
		{
			for(i=0;i < m_dma.dlg;i+=2)
			{
				m_AICARAM[m_dma.dmea] = 0;
				m_AICARAM[m_dma.dmea+1] = 0;
				m_dma.dmea+=2;
			}
		}
		else
		{
			for(i=0;i < m_dma.dlg;i+=2)
			{
				UINT16 tmp;
				tmp = r16(space, m_dma.drga);;
				m_AICARAM[m_dma.dmea] = tmp & 0xff;
				m_AICARAM[m_dma.dmea+1] = tmp>>8;
				m_dma.dmea+=4;
				m_dma.drga+=4;
			}
		}
	}
	else
	{
		if(m_dma.dgate)
		{
			for(i=0;i < m_dma.dlg;i+=2)
			{
				w16(space, m_dma.drga, 0);
				m_dma.drga+=4;
			}
		}
		else
		{
			for(i=0;i < m_dma.dlg;i+=2)
			{
				UINT16 tmp;
				tmp = m_AICARAM[m_dma.dmea];
				tmp|= m_AICARAM[m_dma.dmea+1]<<8;
				w16(space, m_dma.drga, tmp);
				m_dma.dmea+=4;
				m_dma.drga+=4;
			}
		}
	}

	/*Resume the values*/
	if(!(m_dma.ddir))
	{
		for(i=0;i<4;i++)
			m_udata.data[(0x80+(i*4))/2] = tmp_dma[i];
	}

	/* Job done, clear DEXE */
	m_udata.data[0x8c/2] &= ~1;
	/* request a dma end irq */
	m_mcipd |= 0x10;
	CheckPendingIRQ_SH4();
}

#ifdef UNUSED_FUNCTION
int aica_device::IRQCB(void *param)
{
	CheckPendingIRQ(param);
	return -1;
}
#endif

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void aica_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	m_bufferl = outputs[0];
	m_bufferr = outputs[1];
	m_length = samples;
	DoMasterSamples(samples);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aica_device::device_start()
{
	// init the emulation
	Init();

	// set up the IRQ callbacks
	m_irq_cb.resolve_safe();
	m_main_irq_cb.resolve_safe();

	m_stream = machine().sound().stream_alloc(*this, 0, 2, 44100);

	// save state
	save_item(NAME(m_IrqTimA));
	save_item(NAME(m_IrqTimBC));
	save_item(NAME(m_IrqMidi));
	save_item(NAME(m_MidiOutW));
	save_item(NAME(m_MidiOutR));
	save_item(NAME(m_MidiStack),16);
	save_item(NAME(m_MidiW));
	save_item(NAME(m_MidiR));
	save_item(NAME(m_LPANTABLE),0x20000);
	save_item(NAME(m_RPANTABLE),0x20000);
	save_item(NAME(m_TimPris),3);
	save_item(NAME(m_TimCnt),3);
}

void aica_device::set_ram_base(void *base, int size)
{
	m_AICARAM = (unsigned char *)base;
	m_AICARAM_LENGTH = size;
	m_RAM_MASK = m_AICARAM_LENGTH-1;
	m_RAM_MASK16 = m_RAM_MASK & 0x7ffffe;
	m_DSP.AICARAM = (UINT16 *)base;
	m_DSP.AICARAM_LENGTH = size;
}

READ16_MEMBER( aica_device::read )
{
	return r16(space,offset*2);
}

WRITE16_MEMBER( aica_device::write )
{
	UINT16 tmp;

	tmp = r16(space, offset*2);
	COMBINE_DATA(&tmp);
	w16(space, offset*2, tmp);
}

WRITE16_MEMBER( aica_device::midi_in )
{
	m_MidiStack[m_MidiW++]=data;
	m_MidiW &= 15;
}

READ16_MEMBER( aica_device::midi_out_r )
{
	unsigned char val;

	val=m_MidiStack[m_MidiR++];
	m_MidiR&=7;
	return val;
}

const device_type AICA = &device_creator<aica_device>;

aica_device::aica_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AICA, "AICA", tag, owner, clock, "aica", __FILE__),
		device_sound_interface(mconfig, *this),
		m_master(false),
		m_roffset(0),
		m_irq_cb(*this),
		m_main_irq_cb(*this),
		m_IRQL(0),
		m_IRQR(0),
		m_BUFPTR(0),
		m_AICARAM(nullptr),
		m_AICARAM_LENGTH(0),
		m_RAM_MASK(0),
		m_RAM_MASK16(0),
		m_buffertmpl(nullptr),
		m_buffertmpr(nullptr),
		m_IrqTimA(0),
		m_IrqTimBC(0),
		m_IrqMidi(0),
		m_MidiOutW(0),
		m_MidiOutR(0),
		m_MidiW(0),
		m_MidiR(0),
		m_mcieb(0),
		m_mcipd(0),
		m_bufferl(nullptr),
		m_bufferr(nullptr),
		m_length(0),
		m_RBUFDST(nullptr)

{
	memset(&m_udata.data, 0, sizeof(m_udata.data));
	memset(m_EFSPAN, 0, sizeof(m_EFSPAN));
	memset(m_Slots, 0, sizeof(m_Slots));
	memset(m_RINGBUF, 0, sizeof(m_RINGBUF));
	memset(m_MidiStack, 0, sizeof(m_MidiStack));

	memset(m_LPANTABLE, 0, sizeof(m_LPANTABLE));
	memset(m_RPANTABLE, 0, sizeof(m_RPANTABLE));

	memset(m_TimPris, 0, sizeof(m_TimPris));
	memset(m_TimCnt, 0, sizeof(m_TimCnt));

	memset(&m_dma, 0, sizeof(m_dma));

	memset(m_ARTABLE, 0, sizeof(m_ARTABLE));
	memset(m_DRTABLE, 0, sizeof(m_DRTABLE));

	memset(&m_DSP, 0, sizeof(m_DSP));

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
}


static const float LFOFreq[32]={0.17f,0.19f,0.23f,0.27f,0.34f,0.39f,0.45f,0.55f,0.68f,0.78f,0.92f,1.10f,1.39f,1.60f,1.87f,2.27f,
				2.87f,3.31f,3.92f,4.79f,6.15f,7.18f,8.60f,10.8f,14.4f,17.2f,21.5f,28.7f,43.1f,57.4f,86.1f,172.3f};
static const float ASCALE[8]={0.0f,0.4f,0.8f,1.5f,3.0f,6.0f,12.0f,24.0f};
static const float PSCALE[8]={0.0f,7.0f,13.5f,27.0f,55.0f,112.0f,230.0f,494.0f};

void aica_device::AICALFO_Init()
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

signed int aica_device::AICAPLFO_Step(AICA_LFO_t *LFO)
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

signed int aica_device::AICAALFO_Step(AICA_LFO_t *LFO)
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

void aica_device::AICALFO_ComputeStep(AICA_LFO_t *LFO,UINT32 LFOF,UINT32 LFOWS,UINT32 LFOS,int ALFO)
{
	float step=(float) LFOFreq[LFOF]*256.0f/(float)44100.0f;
	LFO->phase_step=(unsigned int) ((float) (1<<LFO_SHIFT)*step);
	if(ALFO)
	{
		switch(LFOWS)
		{
			case 0: LFO->table=m_ALFO_SAW; break;
			case 1: LFO->table=m_ALFO_SQR; break;
			case 2: LFO->table=m_ALFO_TRI; break;
			case 3: LFO->table=m_ALFO_NOI; break;
			default: printf("Unknown ALFO %d\n", LFOWS);
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
			default: printf("Unknown PLFO %d\n", LFOWS);
		}
		LFO->scale=m_PSCALES[LFOS];
	}
}
