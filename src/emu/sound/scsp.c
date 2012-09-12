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
#include "scsp.h"
#include "scspdsp.h"


#define ICLIP16(x) (x<-32768)?-32768:((x>32767)?32767:x)

#define SHIFT	12
#define FIX(v)	((UINT32) ((float) (1<<SHIFT)*(v)))


#define EG_SHIFT	16
#define FM_DELAY    0    // delay in number of slots processed before samples are written to the FM ring buffer
			 // driver code indicates should be 4, but sounds distorted then

// include the LFO handling code
#include "scsplfo.c"

/*
    SCSP features 32 programmable slots
    that can generate FM and PCM (from ROM/RAM) sound
*/

//SLOT PARAMETERS
#define KEYONEX(slot)		((slot->udata.data[0x0]>>0x0)&0x1000)
#define KEYONB(slot)		((slot->udata.data[0x0]>>0x0)&0x0800)
#define SBCTL(slot)		((slot->udata.data[0x0]>>0x9)&0x0003)
#define SSCTL(slot)		((slot->udata.data[0x0]>>0x7)&0x0003)
#define LPCTL(slot)		((slot->udata.data[0x0]>>0x5)&0x0003)
#define PCM8B(slot)		((slot->udata.data[0x0]>>0x0)&0x0010)

#define SA(slot)		(((slot->udata.data[0x0]&0xF)<<16)|(slot->udata.data[0x1]))

#define LSA(slot)		(slot->udata.data[0x2])

#define LEA(slot)		(slot->udata.data[0x3])

#define D2R(slot)		((slot->udata.data[0x4]>>0xB)&0x001F)
#define D1R(slot)		((slot->udata.data[0x4]>>0x6)&0x001F)
#define EGHOLD(slot)		((slot->udata.data[0x4]>>0x0)&0x0020)
#define AR(slot)		((slot->udata.data[0x4]>>0x0)&0x001F)

#define LPSLNK(slot)		((slot->udata.data[0x5]>>0x0)&0x4000)
#define KRS(slot)		((slot->udata.data[0x5]>>0xA)&0x000F)
#define DL(slot)		((slot->udata.data[0x5]>>0x5)&0x001F)
#define RR(slot)		((slot->udata.data[0x5]>>0x0)&0x001F)

#define STWINH(slot)		((slot->udata.data[0x6]>>0x0)&0x0200)
#define SDIR(slot)		((slot->udata.data[0x6]>>0x0)&0x0100)
#define TL(slot)		((slot->udata.data[0x6]>>0x0)&0x00FF)

#define MDL(slot)		((slot->udata.data[0x7]>>0xC)&0x000F)
#define MDXSL(slot)		((slot->udata.data[0x7]>>0x6)&0x003F)
#define MDYSL(slot)		((slot->udata.data[0x7]>>0x0)&0x003F)

#define OCT(slot)		((slot->udata.data[0x8]>>0xB)&0x000F)
#define FNS(slot)		((slot->udata.data[0x8]>>0x0)&0x03FF)

#define LFORE(slot)		((slot->udata.data[0x9]>>0x0)&0x8000)
#define LFOF(slot)		((slot->udata.data[0x9]>>0xA)&0x001F)
#define PLFOWS(slot)		((slot->udata.data[0x9]>>0x8)&0x0003)
#define PLFOS(slot)		((slot->udata.data[0x9]>>0x5)&0x0007)
#define ALFOWS(slot)		((slot->udata.data[0x9]>>0x3)&0x0003)
#define ALFOS(slot)		((slot->udata.data[0x9]>>0x0)&0x0007)

#define ISEL(slot)		((slot->udata.data[0xA]>>0x3)&0x000F)
#define IMXL(slot)		((slot->udata.data[0xA]>>0x0)&0x0007)

#define DISDL(slot)		((slot->udata.data[0xB]>>0xD)&0x0007)
#define DIPAN(slot)		((slot->udata.data[0xB]>>0x8)&0x001F)
#define EFSDL(slot)		((slot->udata.data[0xB]>>0x5)&0x0007)
#define EFPAN(slot)		((slot->udata.data[0xB]>>0x0)&0x001F)

//Envelope times in ms
static const double ARTimes[64]={100000/*infinity*/,100000/*infinity*/,8100.0,6900.0,6000.0,4800.0,4000.0,3400.0,3000.0,2400.0,2000.0,1700.0,1500.0,
					1200.0,1000.0,860.0,760.0,600.0,500.0,430.0,380.0,300.0,250.0,220.0,190.0,150.0,130.0,110.0,95.0,
					76.0,63.0,55.0,47.0,38.0,31.0,27.0,24.0,19.0,15.0,13.0,12.0,9.4,7.9,6.8,6.0,4.7,3.8,3.4,3.0,2.4,
					2.0,1.8,1.6,1.3,1.1,0.93,0.85,0.65,0.53,0.44,0.40,0.35,0.0,0.0};
static const double DRTimes[64]={100000/*infinity*/,100000/*infinity*/,118200.0,101300.0,88600.0,70900.0,59100.0,50700.0,44300.0,35500.0,29600.0,25300.0,22200.0,17700.0,
					14800.0,12700.0,11100.0,8900.0,7400.0,6300.0,5500.0,4400.0,3700.0,3200.0,2800.0,2200.0,1800.0,1600.0,1400.0,1100.0,
					920.0,790.0,690.0,550.0,460.0,390.0,340.0,270.0,230.0,200.0,170.0,140.0,110.0,98.0,85.0,68.0,57.0,49.0,43.0,34.0,
					28.0,25.0,22.0,18.0,14.0,12.0,11.0,8.5,7.1,6.1,5.4,4.3,3.6,3.1};
static INT32 EG_TABLE[0x400];

typedef enum {ATTACK,DECAY1,DECAY2,RELEASE} _STATE;
struct _EG
{
	int volume;	//
	_STATE state;
	int step;
	//step vals
	int AR;		//Attack
	int D1R;	//Decay1
	int D2R;	//Decay2
	int RR;		//Release

	int DL;		//Decay level
	UINT8 EGHOLD;
	UINT8 LPLINK;
};

struct _SLOT
{
	union
	{
		UINT16 data[0x10];	//only 0x1a bytes used
		UINT8 datab[0x20];
	} udata;
	UINT8 Backwards;	//the wave is playing backwards
	UINT8 active;	//this slot is currently playing
	UINT8 *base;		//samples base address
	UINT32 cur_addr;	//current play address (24.8)
	UINT32 nxt_addr;	//next play address
	UINT32 step;		//pitch step (24.8)
	struct _EG EG;			//Envelope
	struct _LFO PLFO;		//Phase LFO
	struct _LFO ALFO;		//Amplitude LFO
	int slot;
	signed short Prev;	//Previous sample (for interpolation)
};


#define MEM4B(scsp)		((scsp->udata.data[0]>>0x0)&0x0200)
#define DAC18B(scsp)		((scsp->udata.data[0]>>0x0)&0x0100)
#define MVOL(scsp)		((scsp->udata.data[0]>>0x0)&0x000F)
#define RBL(scsp)		((scsp->udata.data[1]>>0x7)&0x0003)
#define RBP(scsp)		((scsp->udata.data[1]>>0x0)&0x003F)
#define MOFULL(scsp)		((scsp->udata.data[2]>>0x0)&0x1000)
#define MOEMPTY(scsp)		((scsp->udata.data[2]>>0x0)&0x0800)
#define MIOVF(scsp)		((scsp->udata.data[2]>>0x0)&0x0400)
#define MIFULL(scsp)		((scsp->udata.data[2]>>0x0)&0x0200)
#define MIEMPTY(scsp)		((scsp->udata.data[2]>>0x0)&0x0100)

#define SCILV0(scsp)    	((scsp->udata.data[0x24/2]>>0x0)&0xff)
#define SCILV1(scsp)    	((scsp->udata.data[0x26/2]>>0x0)&0xff)
#define SCILV2(scsp)    	((scsp->udata.data[0x28/2]>>0x0)&0xff)

#define SCIEX0	0
#define SCIEX1	1
#define SCIEX2	2
#define SCIMID	3
#define SCIDMA	4
#define SCIIRQ	5
#define SCITMA	6
#define SCITMB	7

#define USEDSP

typedef struct _scsp_state scsp_state;
struct _scsp_state
{
	union
	{
		UINT16 data[0x30/2];
		UINT8 datab[0x30];
	} udata;
	struct _SLOT Slots[32];
	signed short RINGBUF[128];
	unsigned char BUFPTR;
#if FM_DELAY
	signed short DELAYBUF[FM_DELAY];
	unsigned char DELAYPTR;
#endif
	unsigned char *SCSPRAM;
	UINT32 SCSPRAM_LENGTH;
	char Master;
	void (*Int68kCB)(device_t *device, int irq);
	sound_stream * stream;

	INT32 *buffertmpl,*buffertmpr;

	UINT32 IrqTimA;
	UINT32 IrqTimBC;
	UINT32 IrqMidi;

	UINT8 MidiOutW,MidiOutR;
	UINT8 MidiStack[32];
	UINT8 MidiW,MidiR;

	int LPANTABLE[0x10000];
	int RPANTABLE[0x10000];

	int TimPris[3];
	int TimCnt[3];

	// timers
	emu_timer *timerA, *timerB, *timerC;

	// DMA stuff
	UINT32 scsp_dmea;
	UINT16 scsp_drga;
	UINT16 scsp_dtlg;
	UINT16 scsp_dmactrl;

	UINT16 dma_regs[3];

	int ARTABLE[64], DRTABLE[64];

	struct _SCSPDSP DSP;
	devcb_resolved_write_line main_irq;

	device_t *device;
};

static void dma_scsp(address_space *space, scsp_state *scsp);		/*state DMA transfer function*/
#define	scsp_dgate		scsp->scsp_dmactrl & 0x4000
#define	scsp_ddir		scsp->scsp_dmactrl & 0x2000
#define scsp_dexe		scsp->scsp_dmactrl & 0x1000
/* TODO */
//#define dma_transfer_end  ((scsp_regs[0x24/2] & 0x10)>>4)|(((scsp_regs[0x26/2] & 0x10)>>4)<<1)|(((scsp_regs[0x28/2] & 0x10)>>4)<<2)

static const float SDLT[8]={-1000000.0f,-36.0f,-30.0f,-24.0f,-18.0f,-12.0f,-6.0f,0.0f};

static stream_sample_t *bufferl;
static stream_sample_t *bufferr;

static int length;


static signed short *RBUFDST;	//this points to where the sample will be stored in the RingBuf


INLINE scsp_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SCSP);
	return (scsp_state *)downcast<scsp_device *>(device)->token();
}

static unsigned char DecodeSCI(scsp_state *scsp,unsigned char irq)
{
	unsigned char SCI=0;
	unsigned char v;
	v=(SCILV0((scsp))&(1<<irq))?1:0;
	SCI|=v;
	v=(SCILV1((scsp))&(1<<irq))?1:0;
	SCI|=v<<1;
	v=(SCILV2((scsp))&(1<<irq))?1:0;
	SCI|=v<<2;
	return SCI;
}

static void CheckPendingIRQ(scsp_state *scsp)
{
	UINT32 pend=scsp->udata.data[0x20/2];
	UINT32 en=scsp->udata.data[0x1e/2];

	if(scsp->MidiW!=scsp->MidiR)
	{
		scsp->udata.data[0x20/2] |= 8;
		pend |= 8;
	}
	if(!pend)
		return;
	if(pend&0x40)
		if(en&0x40)
		{
			scsp->Int68kCB(scsp->device, scsp->IrqTimA);
			return;
		}
	if(pend&0x80)
		if(en&0x80)
		{
			scsp->Int68kCB(scsp->device, scsp->IrqTimBC);
			return;
		}
	if(pend&0x100)
		if(en&0x100)
		{
			scsp->Int68kCB(scsp->device, scsp->IrqTimBC);
			return;
		}
	if(pend&8)
		if (en&8)
		{
			scsp->Int68kCB(scsp->device, scsp->IrqMidi);
			scsp->udata.data[0x20/2] &= ~8;
			return;
		}

	scsp->Int68kCB(scsp->device, 0);
}

static void ResetInterrupts(scsp_state *scsp)
{
	UINT32 reset = scsp->udata.data[0x22/2];

	if (reset & 0x40)
	{
		scsp->Int68kCB(scsp->device, -scsp->IrqTimA);
	}
	if (reset & 0x180)
	{
		scsp->Int68kCB(scsp->device, -scsp->IrqTimBC);
	}
	if (reset & 0x8)
	{
		scsp->Int68kCB(scsp->device, -scsp->IrqMidi);
	}

	CheckPendingIRQ(scsp);
}

static TIMER_CALLBACK( timerA_cb )
{
	scsp_state *scsp = (scsp_state *)ptr;

	scsp->TimCnt[0] = 0xFFFF;
	scsp->udata.data[0x20/2]|=0x40;
	scsp->udata.data[0x18/2]&=0xff00;
	scsp->udata.data[0x18/2]|=scsp->TimCnt[0]>>8;

	CheckPendingIRQ(scsp);
}

static TIMER_CALLBACK( timerB_cb )
{
	scsp_state *scsp = (scsp_state *)ptr;

	scsp->TimCnt[1] = 0xFFFF;
	scsp->udata.data[0x20/2]|=0x80;
	scsp->udata.data[0x1a/2]&=0xff00;
	scsp->udata.data[0x1a/2]|=scsp->TimCnt[1]>>8;

	CheckPendingIRQ(scsp);
}

static TIMER_CALLBACK( timerC_cb )
{
	scsp_state *scsp = (scsp_state *)ptr;

	scsp->TimCnt[2] = 0xFFFF;
	scsp->udata.data[0x20/2]|=0x100;
	scsp->udata.data[0x1c/2]&=0xff00;
	scsp->udata.data[0x1c/2]|=scsp->TimCnt[2]>>8;

	CheckPendingIRQ(scsp);
}

static int Get_AR(scsp_state *scsp,int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return scsp->ARTABLE[Rate];
}

static int Get_DR(scsp_state *scsp,int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return scsp->DRTABLE[Rate];
}

static int Get_RR(scsp_state *scsp,int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return scsp->DRTABLE[Rate];
}

static void Compute_EG(scsp_state *scsp,struct _SLOT *slot)
{
	int octave=(OCT(slot)^8)-8;
	int rate;
	if(KRS(slot)!=0xf)
		rate=octave+2*KRS(slot)+((FNS(slot)>>9)&1);
	else
		rate=0; //rate=((FNS(slot)>>9)&1);

	slot->EG.volume=0x17F<<EG_SHIFT;
	slot->EG.AR=Get_AR(scsp,rate,AR(slot));
	slot->EG.D1R=Get_DR(scsp,rate,D1R(slot));
	slot->EG.D2R=Get_DR(scsp,rate,D2R(slot));
	slot->EG.RR=Get_RR(scsp,rate,RR(slot));
	slot->EG.DL=0x1f-DL(slot);
	slot->EG.EGHOLD=EGHOLD(slot);
}

static void SCSP_StopSlot(struct _SLOT *slot,int keyoff);

static int EG_Update(struct _SLOT *slot)
{
	switch(slot->EG.state)
	{
		case ATTACK:
			slot->EG.volume+=slot->EG.AR;
			if(slot->EG.volume>=(0x3ff<<EG_SHIFT))
			{
				if (!LPSLNK(slot))
				{
					slot->EG.state=DECAY1;
					if(slot->EG.D1R>=(1024<<EG_SHIFT)) //Skip DECAY1, go directly to DECAY2
						slot->EG.state=DECAY2;
				}
				slot->EG.volume=0x3ff<<EG_SHIFT;
			}
			if(slot->EG.EGHOLD)
				return 0x3ff<<(SHIFT-10);
			break;
		case DECAY1:
			slot->EG.volume-=slot->EG.D1R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;
			if(slot->EG.volume>>(EG_SHIFT+5)<=slot->EG.DL)
				slot->EG.state=DECAY2;
			break;
		case DECAY2:
			if(D2R(slot)==0)
				return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
			slot->EG.volume-=slot->EG.D2R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;

			break;
		case RELEASE:
			slot->EG.volume-=slot->EG.RR;
			if(slot->EG.volume<=0)
			{
				slot->EG.volume=0;
				SCSP_StopSlot(slot,0);
				//slot->EG.volume=0x17F<<EG_SHIFT;
				//slot->EG.state=ATTACK;
			}
			break;
		default:
			return 1<<SHIFT;
	}
	return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
}

static UINT32 SCSP_Step(struct _SLOT *slot)
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


static void Compute_LFO(struct _SLOT *slot)
{
	if(PLFOS(slot)!=0)
		LFO_ComputeStep(&(slot->PLFO),LFOF(slot),PLFOWS(slot),PLFOS(slot),0);
	if(ALFOS(slot)!=0)
		LFO_ComputeStep(&(slot->ALFO),LFOF(slot),ALFOWS(slot),ALFOS(slot),1);
}

static void SCSP_StartSlot(scsp_state *scsp, struct _SLOT *slot)
{
	UINT32 start_offset;

	slot->active=1;
	start_offset = PCM8B(slot) ? SA(slot) : SA(slot) & 0x7FFFE;
	slot->base=scsp->SCSPRAM + start_offset;
	slot->cur_addr=0;
	slot->nxt_addr=1<<SHIFT;
	slot->step=SCSP_Step(slot);
	Compute_EG(scsp,slot);
	slot->EG.state=ATTACK;
	slot->EG.volume=0x17F<<EG_SHIFT;
	slot->Prev=0;
	slot->Backwards=0;

	Compute_LFO(slot);

//  printf("StartSlot[%p]: SA %x PCM8B %x LPCTL %x ALFOS %x STWINH %x TL %x EFSDL %x\n", slot, SA(slot), PCM8B(slot), LPCTL(slot), ALFOS(slot), STWINH(slot), TL(slot), EFSDL(slot));
}

static void SCSP_StopSlot(struct _SLOT *slot,int keyoff)
{
	if(keyoff /*&& slot->EG.state!=RELEASE*/)
	{
		slot->EG.state=RELEASE;
	}
	else
	{
		slot->active=0;
	}
	slot->udata.data[0]&=~0x800;
}

#define log_base_2(n) (log((double)(n))/log(2.0))

static void SCSP_Init(device_t *device, scsp_state *scsp, const scsp_interface *intf)
{
	int i;

	memset(scsp,0,sizeof(*scsp));

	SCSPDSP_Init(&scsp->DSP);

	scsp->device = device;
	scsp->IrqTimA = scsp->IrqTimBC = scsp->IrqMidi = 0;
	scsp->MidiR=scsp->MidiW=0;
	scsp->MidiOutR=scsp->MidiOutW=0;

	// get SCSP RAM
	if (strcmp(device->tag(), ":scsp") == 0 || strcmp(device->tag(), ":scsp1") == 0)
	{
		scsp->Master=1;
	}
	else
	{
		scsp->Master=0;
	}

	scsp->SCSPRAM = *device->region();
	if (scsp->SCSPRAM)
	{
		scsp->SCSPRAM_LENGTH = device->region()->bytes();
		scsp->DSP.SCSPRAM = (UINT16 *)scsp->SCSPRAM;
		scsp->DSP.SCSPRAM_LENGTH = scsp->SCSPRAM_LENGTH/2;
		scsp->SCSPRAM += intf->roffset;
	}

	scsp->timerA = device->machine().scheduler().timer_alloc(FUNC(timerA_cb), scsp);
	scsp->timerB = device->machine().scheduler().timer_alloc(FUNC(timerB_cb), scsp);
	scsp->timerC = device->machine().scheduler().timer_alloc(FUNC(timerC_cb), scsp);

	for(i=0;i<0x400;++i)
	{
		float envDB=((float)(3*(i-0x3ff)))/32.0f;
		float scale=(float)(1<<SHIFT);
		EG_TABLE[i]=(INT32)(pow(10.0,envDB/20.0)*scale);
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

		TL=pow(10.0,SegaDB/20.0);

		SegaDB=0;
		if(iPAN&0x1) SegaDB-=3.0f;
		if(iPAN&0x2) SegaDB-=6.0f;
		if(iPAN&0x4) SegaDB-=12.0f;
		if(iPAN&0x8) SegaDB-=24.0f;

		if((iPAN&0xf)==0xf) PAN=0.0;
		else PAN=pow(10.0,SegaDB/20.0);

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
			fSDL=pow(10.0,(SDLT[iSDL])/20.0);
		else
			fSDL=0.0;

		scsp->LPANTABLE[i]=FIX((4.0*LPAN*TL*fSDL));
		scsp->RPANTABLE[i]=FIX((4.0*RPAN*TL*fSDL));
	}

	scsp->ARTABLE[0]=scsp->DRTABLE[0]=0;	//Infinite time
	scsp->ARTABLE[1]=scsp->DRTABLE[1]=0;	//Infinite time
	for(i=2;i<64;++i)
	{
		double t,step,scale;
		t=ARTimes[i];	//In ms
		if(t!=0.0)
		{
			step=(1023*1000.0)/((float) 44100.0f*t);
			scale=(double) (1<<EG_SHIFT);
			scsp->ARTABLE[i]=(int) (step*scale);
		}
		else
			scsp->ARTABLE[i]=1024<<EG_SHIFT;

		t=DRTimes[i];	//In ms
		step=(1023*1000.0)/((float) 44100.0f*t);
		scale=(double) (1<<EG_SHIFT);
		scsp->DRTABLE[i]=(int) (step*scale);
	}

	// make sure all the slots are off
	for(i=0;i<32;++i)
	{
		scsp->Slots[i].slot=i;
		scsp->Slots[i].active=0;
		scsp->Slots[i].base=NULL;
		scsp->Slots[i].EG.state=RELEASE;
	}

	LFO_Init(device->machine());
	scsp->buffertmpl=auto_alloc_array_clear(device->machine(), signed int, 44100);
	scsp->buffertmpr=auto_alloc_array_clear(device->machine(), signed int, 44100);

	// no "pend"
	scsp->udata.data[0x20/2] = 0;
	scsp->TimCnt[0] = 0xffff;
	scsp->TimCnt[1] = 0xffff;
	scsp->TimCnt[2] = 0xffff;
}

static void SCSP_UpdateSlotReg(scsp_state *scsp,int s,int r)
{
	struct _SLOT *slot=scsp->Slots+s;
	int sl;
	switch(r&0x3f)
	{
		case 0:
		case 1:
			if(KEYONEX(slot))
			{
				for(sl=0;sl<32;++sl)
				{
					struct _SLOT *s2=scsp->Slots+sl;
					{
						if(KEYONB(s2) && s2->EG.state==RELEASE/*&& !s2->active*/)
						{
							SCSP_StartSlot(scsp, s2);
						}
						if(!KEYONB(s2) /*&& s2->active*/)
						{
							SCSP_StopSlot(s2,1);
						}
					}
				}
				slot->udata.data[0]&=~0x1000;
			}
			break;
		case 0x10:
		case 0x11:
			slot->step=SCSP_Step(slot);
			break;
		case 0xA:
		case 0xB:
			slot->EG.RR=Get_RR(scsp,0,RR(slot));
			slot->EG.DL=0x1f-DL(slot);
			break;
		case 0x12:
		case 0x13:
			Compute_LFO(slot);
			break;
	}
}

static void SCSP_UpdateReg(scsp_state *scsp, int reg)
{
	/* temporary hack until this is converted to a device */
	address_space *space = scsp->device->machine().firstcpu->space(AS_PROGRAM);

	switch(reg&0x3f)
	{
		case 0x0:
			scsp->stream->set_output_gain(0,MVOL(scsp) / 15.0);
			scsp->stream->set_output_gain(1,MVOL(scsp) / 15.0);
			break;
		case 0x2:
		case 0x3:
			{
				unsigned int v=RBL(scsp);
				scsp->DSP.RBP=RBP(scsp);
				if(v==0)
					scsp->DSP.RBL=8*1024;
				else if(v==1)
					scsp->DSP.RBL=16*1024;
				if(v==2)
					scsp->DSP.RBL=32*1024;
				if(v==3)
					scsp->DSP.RBL=64*1024;
			}
			break;
		case 0x6:
		case 0x7:
			scsp_midi_in(space->machine().device("scsp"), 0, scsp->udata.data[0x6/2]&0xff, 0);
			break;
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			break;
		case 0x18:
		case 0x19:
			if(scsp->Master)
			{
				UINT32 time;

				scsp->TimPris[0]=1<<((scsp->udata.data[0x18/2]>>8)&0x7);
				scsp->TimCnt[0]=(scsp->udata.data[0x18/2]&0xff)<<8;

				if ((scsp->udata.data[0x18/2]&0xff) != 255)
				{
					time = (44100 / scsp->TimPris[0]) / (255-(scsp->udata.data[0x18/2]&0xff));
					if (time)
					{
						scsp->timerA->adjust(attotime::from_hz(time));
					}
				}
			}
			break;
		case 0x1a:
		case 0x1b:
			if(scsp->Master)
			{
				UINT32 time;

				scsp->TimPris[1]=1<<((scsp->udata.data[0x1A/2]>>8)&0x7);
				scsp->TimCnt[1]=(scsp->udata.data[0x1A/2]&0xff)<<8;

				if ((scsp->udata.data[0x1A/2]&0xff) != 255)
				{
					time = (44100 / scsp->TimPris[1]) / (255-(scsp->udata.data[0x1A/2]&0xff));
					if (time)
					{
						scsp->timerB->adjust(attotime::from_hz(time));
					}
				}
			}
			break;
		case 0x1C:
		case 0x1D:
			if(scsp->Master)
			{
				UINT32 time;

				scsp->TimPris[2]=1<<((scsp->udata.data[0x1C/2]>>8)&0x7);
				scsp->TimCnt[2]=(scsp->udata.data[0x1C/2]&0xff)<<8;

				if ((scsp->udata.data[0x1C/2]&0xff) != 255)
				{
					time = (44100 / scsp->TimPris[2]) / (255-(scsp->udata.data[0x1C/2]&0xff));
					if (time)
					{
						scsp->timerC->adjust(attotime::from_hz(time));
					}
				}
			}
			break;
		case 0x22:	//SCIRE
		case 0x23:

			if(scsp->Master)
			{
				scsp->udata.data[0x20/2]&=~scsp->udata.data[0x22/2];
				ResetInterrupts(scsp);

				// behavior from real hardware: if you SCIRE a timer that's expired,
				// it'll immediately pop up again in SCIPD.  ask Sakura Taisen on the Saturn...
				if (scsp->TimCnt[0] == 0xffff)
				{
					scsp->udata.data[0x20/2] |= 0x40;
				}
				if (scsp->TimCnt[1] == 0xffff)
				{
					scsp->udata.data[0x20/2] |= 0x80;
				}
				if (scsp->TimCnt[2] == 0xffff)
				{
					scsp->udata.data[0x20/2] |= 0x100;
				}
			}
			break;
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
			if(scsp->Master)
			{
				scsp->IrqTimA=DecodeSCI(scsp,SCITMA);
				scsp->IrqTimBC=DecodeSCI(scsp,SCITMB);
				scsp->IrqMidi=DecodeSCI(scsp,SCIMID);
			}
			break;
	}
}

static void SCSP_UpdateSlotRegR(scsp_state *scsp, int slot,int reg)
{

}

static void SCSP_UpdateRegR(scsp_state *scsp, int reg)
{
	switch(reg&0x3f)
	{
		case 4:
		case 5:
			{
				unsigned short v=scsp->udata.data[0x5/2];
				v&=0xff00;
				v|=scsp->MidiStack[scsp->MidiR];
				scsp->Int68kCB(scsp->device, -scsp->IrqMidi);	// cancel the IRQ
                logerror("Read %x from SCSP MIDI\n", v);
				if(scsp->MidiR!=scsp->MidiW)
				{
					++scsp->MidiR;
					scsp->MidiR&=31;
				}
				scsp->udata.data[0x5/2]=v;
			}
			break;
		case 8:
		case 9:
			{
				// MSLC     |  CA   |SGC|EG
				// f e d c b a 9 8 7 6 5 4 3 2 1 0
				unsigned char MSLC=(scsp->udata.data[0x8/2]>>11)&0x1f;
				struct _SLOT *slot=scsp->Slots + MSLC;
				unsigned int SGC = (slot->EG.state) & 3;
				unsigned int CA = (slot->cur_addr>>(SHIFT+12)) & 0xf;
				unsigned int EG = (0x1f - (slot->EG.volume>>(EG_SHIFT+5))) & 0x1f;
				scsp->udata.data[0x8/2] =  (MSLC << 11) | (CA << 7) | (SGC << 5) | EG;
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
	}
}

static void SCSP_w16(scsp_state *scsp,unsigned int addr,unsigned short val)
{
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		*((unsigned short *) (scsp->Slots[slot].udata.datab+(addr))) = val;
		SCSP_UpdateSlotReg(scsp,slot,addr&0x1f);
	}
	else if(addr<0x600)
	{
		if (addr < 0x430)
		{
			*((unsigned short *) (scsp->udata.datab+((addr&0x3f)))) = val;
			SCSP_UpdateReg(scsp, addr&0x3f);
		}
	}
	else if(addr<0x700)
		scsp->RINGBUF[(addr-0x600)/2]=val;
	else
	{
		//DSP
		if(addr<0x780)	//COEF
			*((unsigned short *) (scsp->DSP.COEF+(addr-0x700)/2))=val;
		else if(addr<0x800)
			*((unsigned short *) (scsp->DSP.MADRS+(addr-0x780)/2))=val;
		else if(addr<0xC00)
		{
			*((unsigned short *) (scsp->DSP.MPRO+(addr-0x800)/2))=val;

			if(addr==0xBF0)
			{
				SCSPDSP_Start(&scsp->DSP);
	    	}
		}
	}
}

static unsigned short SCSP_r16(scsp_state *scsp, unsigned int addr)
{
	unsigned short v=0;
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		SCSP_UpdateSlotRegR(scsp, slot,addr&0x1f);
		v=*((unsigned short *) (scsp->Slots[slot].udata.datab+(addr)));
	}
	else if(addr>=0x412 && addr <= 0x416)
		v = scsp->dma_regs[((addr-0x412)/2) & 3];
	else if(addr<0x600)
	{
		if (addr < 0x430)
		{
			SCSP_UpdateRegR(scsp, addr&0x3f);
			v= *((unsigned short *) (scsp->udata.datab+((addr&0x3f))));
		}
	}
	else if(addr<0x700)
		v=scsp->RINGBUF[(addr-0x600)/2];
	return v;
}


#define REVSIGN(v) ((~v)+1)

INLINE INT32 SCSP_UpdateSlot(scsp_state *scsp, struct _SLOT *slot)
{
	INT32 sample;
	int step=slot->step;
	UINT32 addr1,addr2,addr_select;                                   // current and next sample addresses
	UINT32 *addr[2]      = {&addr1, &addr2};                          // used for linear interpolation
	UINT32 *slot_addr[2] = {&(slot->cur_addr), &(slot->nxt_addr)};    //

	if(SSCTL(slot)!=0)	//no FM or noise yet
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
		INT32 smp=(scsp->RINGBUF[(scsp->BUFPTR+MDXSL(slot))&63]+scsp->RINGBUF[(scsp->BUFPTR+MDYSL(slot))&63])/2;

		smp<<=0xA; // associate cycle with 1024
		smp>>=0x1A-MDL(slot); // ex. for MDL=0xF, sample range corresponds to +/- 64 pi (32=2^5 cycles) so shift by 11 (16-5 == 0x1A-0xF)
		if(!PCM8B(slot)) smp<<=1;

		addr1+=smp; addr2+=smp;
	}

	if(PCM8B(slot))	//8 bit signed
	{
		INT8 *p1=(signed char *) (scsp->SCSPRAM+BYTE_XOR_BE(((SA(slot)+addr1))&0x7FFFF));
		INT8 *p2=(signed char *) (scsp->SCSPRAM+BYTE_XOR_BE(((SA(slot)+addr2))&0x7FFFF));
		//sample=(p[0])<<8;
		INT32 s;
		INT32 fpart=slot->cur_addr&((1<<SHIFT)-1);
		s=(int) (p1[0]<<8)*((1<<SHIFT)-fpart)+(int) (p2[0]<<8)*fpart;
		sample=(s>>SHIFT);
	}
	else	//16 bit signed (endianness?)
	{
		INT16 *p1=(signed short *) (scsp->SCSPRAM+((SA(slot)+addr1)&0x7FFFE));
		INT16 *p2=(signed short *) (scsp->SCSPRAM+((SA(slot)+addr2)&0x7FFFE));
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
		if(LPSLNK(slot) && slot->EG.state==ATTACK)
			slot->EG.state = DECAY1;
	}

	for (addr_select=0;addr_select<2;addr_select++)
	{
		INT32 rem_addr;
		switch(LPCTL(slot))
		{
		case 0:	//no loop
			if(*addr[addr_select]>=LSA(slot) && *addr[addr_select]>=LEA(slot))
			{
			//slot->active=0;
			SCSP_StopSlot(slot,0);
			}
			break;
		case 1: //normal loop
			if(*addr[addr_select]>=LEA(slot))
			{
				rem_addr = *slot_addr[addr_select] - (LEA(slot)<<SHIFT);
				*slot_addr[addr_select]=(LSA(slot)<<SHIFT) + rem_addr;
			}
			break;
		case 2:	//reverse loop
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

		if(slot->EG.state==ATTACK)
			sample=(sample*EG_Update(slot))>>SHIFT;
		else
			sample=(sample*EG_TABLE[EG_Update(slot)>>(SHIFT-10)])>>SHIFT;
	}

	if(!STWINH(slot))
	{
		if(!SDIR(slot))
		{
			unsigned short Enc=((TL(slot))<<0x0)|(0x7<<0xd);
			*RBUFDST=(sample*scsp->LPANTABLE[Enc])>>(SHIFT+1);
		}
		else
		{
			unsigned short Enc=(0<<0x0)|(0x7<<0xd);
			*RBUFDST=(sample*scsp->LPANTABLE[Enc])>>(SHIFT+1);
		}
	}

	return sample;
}

static void SCSP_DoMasterSamples(scsp_state *scsp, int nsamples)
{
	stream_sample_t *bufr,*bufl;
	int sl, s, i;

	bufr=bufferr;
	bufl=bufferl;

	for(s=0;s<nsamples;++s)
	{
		INT32 smpl, smpr;

		smpl = smpr = 0;

		for(sl=0;sl<32;++sl)
		{
#if FM_DELAY
			RBUFDST=scsp->DELAYBUF+scsp->DELAYPTR;
#else
			RBUFDST=scsp->RINGBUF+scsp->BUFPTR;
#endif
			if(scsp->Slots[sl].active)
			{
				struct _SLOT *slot=scsp->Slots+sl;
				unsigned short Enc;
				signed int sample;

				sample=SCSP_UpdateSlot(scsp, slot);

				Enc=((TL(slot))<<0x0)|((IMXL(slot))<<0xd);
				SCSPDSP_SetSample(&scsp->DSP,(sample*scsp->LPANTABLE[Enc])>>(SHIFT-2),ISEL(slot),IMXL(slot));
				Enc=((TL(slot))<<0x0)|((DIPAN(slot))<<0x8)|((DISDL(slot))<<0xd);
				{
					smpl+=(sample*scsp->LPANTABLE[Enc])>>SHIFT;
					smpr+=(sample*scsp->RPANTABLE[Enc])>>SHIFT;
				}
			}

#if FM_DELAY
			scsp->RINGBUF[(scsp->BUFPTR+64-(FM_DELAY-1))&63] = scsp->DELAYBUF[(scsp->DELAYPTR+FM_DELAY-(FM_DELAY-1))%FM_DELAY];
#endif
			++scsp->BUFPTR;
			scsp->BUFPTR&=63;
#if FM_DELAY
			++scsp->DELAYPTR;
			if(scsp->DELAYPTR>FM_DELAY-1) scsp->DELAYPTR=0;
#endif
		}

		SCSPDSP_Step(&scsp->DSP);

		for(i=0;i<16;++i)
		{
			struct _SLOT *slot=scsp->Slots+i;
			if(EFSDL(slot))
			{
				unsigned short Enc=((EFPAN(slot))<<0x8)|((EFSDL(slot))<<0xd);
				smpl+=(scsp->DSP.EFREG[i]*scsp->LPANTABLE[Enc])>>SHIFT;
				smpr+=(scsp->DSP.EFREG[i]*scsp->RPANTABLE[Enc])>>SHIFT;
			}
		}

		*bufl++ = ICLIP16(smpl>>2);
		*bufr++ = ICLIP16(smpr>>2);
	}
}

/* TODO: this needs to be timer-ized */
static void dma_scsp(address_space *space, scsp_state *scsp)
{
	static UINT16 tmp_dma[3];
	int i;

	scsp->scsp_dmactrl = scsp->dma_regs[2] & 0x7000;

	if(!(scsp_dexe)) //don't bother if DMA is off
		return;

	/* calc the registers */
	scsp->scsp_dmea = ((scsp->dma_regs[1] & 0xf000) << 4) | (scsp->dma_regs[0] & 0xfffe); /* RAM address */
	scsp->scsp_drga = (scsp->dma_regs[1] & 0x0ffe);
	scsp->scsp_dtlg = (scsp->dma_regs[2] & 0x0ffe);

	logerror("SCSP: DMA transfer START\n"
			 "DMEA: %04x DRGA: %04x DTLG: %04x\n"
			 "DGATE: %d  DDIR: %d\n",scsp->scsp_dmea,scsp->scsp_drga,scsp->scsp_dtlg,scsp_dgate ? 1 : 0,scsp_ddir ? 1 : 0);

	/* Copy the dma values in a temp storage for resuming later */
    	/* (DMA *can't* overwrite his parameters).                  */
	if(!(scsp_ddir))
	{
		for(i=0;i<3;i++)
			tmp_dma[i] = scsp->dma_regs[i];
	}

	/* TODO: don't know if params auto-updates, I guess not ... */
	if(scsp_ddir)
	{
		for(i=0;i < scsp->scsp_dtlg;i+=2)
		{
			space->write_word(scsp->scsp_dmea, space->read_word(0x100000|scsp->scsp_drga));
			scsp->scsp_dmea+=2;
			scsp->scsp_drga+=2;
		}
	}
	else
	{
		for(i=0;i < scsp->scsp_dtlg;i+=2)
		{
			space->write_word(0x100000|scsp->scsp_drga,space->read_word(scsp->scsp_dmea));
			scsp->scsp_dmea+=2;
			scsp->scsp_drga+=2;
		}
	}

	/*Resume the values*/
	if(!(scsp_ddir))
	{
		for(i=0;i<3;i++)
			scsp->dma_regs[i] = tmp_dma[i];
	}

	/* Job done */
	scsp->dma_regs[2] &= ~0x1000;
	/* request a dma end irq (TODO: make it inside the interface) */
	if(scsp->udata.data[0x1e/2] & 0x10)
	{
		popmessage("SCSP DMA IRQ triggered, contact MAMEdev");
		space->machine().device("audiocpu")->execute().set_input_line(DecodeSCI(scsp,SCIDMA),HOLD_LINE);
	}
}

#ifdef UNUSED_FUNCTION
int SCSP_IRQCB(void *param)
{
	CheckPendingIRQ(param);
	return -1;
}
#endif

static STREAM_UPDATE( SCSP_Update )
{
	scsp_state *scsp = (scsp_state *)param;
	bufferl = outputs[0];
	bufferr = outputs[1];
	length = samples;
	SCSP_DoMasterSamples(scsp, samples);
}

static DEVICE_START( scsp )
{
	const scsp_interface *intf;

	scsp_state *scsp = get_safe_token(device);

	intf = (const scsp_interface *)device->static_config();

	// init the emulation
	SCSP_Init(device, scsp, intf);

	// set up the IRQ callbacks
	{
		scsp->Int68kCB = intf->irq_callback;

		scsp->stream = device->machine().sound().stream_alloc(*device, 0, 2, 44100, scsp, SCSP_Update);
	}

	scsp->main_irq.resolve(intf->main_irq, *device);
}


void scsp_set_ram_base(device_t *device, void *base)
{
	scsp_state *scsp = get_safe_token(device);
	if (scsp)
	{
		scsp->SCSPRAM = (unsigned char *)base;
		scsp->DSP.SCSPRAM = (UINT16 *)base;
		scsp->SCSPRAM_LENGTH = 0x80000;
		scsp->DSP.SCSPRAM_LENGTH = 0x80000/2;
	}
}


READ16_DEVICE_HANDLER( scsp_r )
{
	scsp_state *scsp = get_safe_token(device);

	scsp->stream->update();

	return SCSP_r16(scsp, offset*2);
}

WRITE16_DEVICE_HANDLER( scsp_w )
{
	scsp_state *scsp = get_safe_token(device);
	UINT16 tmp;

	scsp->stream->update();

	tmp = SCSP_r16(scsp, offset*2);
	COMBINE_DATA(&tmp);
	SCSP_w16(scsp,offset*2, tmp);

	switch(offset*2)
	{
		// check DMA
		case 0x412:
		case 0x414:
		case 0x416:
			COMBINE_DATA(&scsp->dma_regs[((offset-0x412)/2) & 3]);
			if(ACCESSING_BITS_8_15 && offset*2 == 0x416)
				dma_scsp(device->machine().firstcpu->space(AS_PROGRAM), scsp);
			break;
		case 0x42a:		//check main cpu IRQ
			scsp->main_irq(1);
			break;
		case 0x42c:
			break;
		case 0x42e:
			break;
	}
}

WRITE16_DEVICE_HANDLER( scsp_midi_in )
{
	scsp_state *scsp = get_safe_token(device);

//    printf("scsp_midi_in: %02x\n", data);

	scsp->MidiStack[scsp->MidiW++]=data;
	scsp->MidiW &= 31;

	CheckPendingIRQ(scsp);
}

READ16_DEVICE_HANDLER( scsp_midi_out_r )
{
	scsp_state *scsp = get_safe_token(device);
	unsigned char val;

	val=scsp->MidiStack[scsp->MidiR++];
	scsp->MidiR&=31;
	return val;
}

const device_type SCSP = &device_creator<scsp_device>;

scsp_device::scsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SCSP, "SCSP", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(scsp_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void scsp_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void scsp_device::device_start()
{
	DEVICE_START_NAME( scsp )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void scsp_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


