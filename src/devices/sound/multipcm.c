// license:BSD-3-Clause
// copyright-holders:Miguel Angel Horna
/*
 * Sega System 32 Multi/Model 1/Model 2 custom PCM chip (315-5560) emulation.
 *
 * by Miguel Angel Horna (ElSemi) for Model 2 Emulator and MAME.
 * Information by R. Belmont and the YMF278B (OPL4) manual.
 *
 * voice registers:
 * 0: Pan
 * 1: Index of sample
 * 2: LSB of pitch (low 2 bits seem unused so)
 * 3: MSB of pitch (ooooppppppppppxx) (o=octave (4 bit signed), p=pitch (10 bits), x=unused?
 * 4: voice control: top bit = 1 for key on, 0 for key off
 * 5: bit 0: 0: interpolate volume changes, 1: direct set volume,
      bits 1-7 = volume attenuate (0=max, 7f=min)
 * 6: LFO frequency + Phase LFO depth
 * 7: Amplitude LFO size
 *
 * The first sample ROM contains a variable length table with 12
 * bytes per instrument/sample. This is very similar to the YMF278B.
 *
 * The first 3 bytes are the offset into the file (big endian).
 * The next 2 are the loop start offset into the file (big endian)
 * The next 2 are the 2's complement of the total sample size (big endian)
 * The next byte is LFO freq + depth (copied to reg 6 ?)
 * The next 3 are envelope params (Attack, Decay1 and 2, sustain level, release, Key Rate Scaling)
 * The next byte is Amplitude LFO size (copied to reg 7 ?)
 *
 * TODO
 * - The YM278B manual states that the chip supports 512 instruments. The MultiPCM probably supports them
 * too but the high bit position is unknown (probably reg 2 low bit). Any game use more than 256?
 *
 */

#include "emu.h"
#include "multipcm.h"

//????
#define MULTIPCM_CLOCKDIV       (180.0f)

ALLOW_SAVE_TYPE(STATE); // allow save_item on a non-fundamental type

static signed int LPANTABLE[0x800],RPANTABLE[0x800];

#define FIX(v)  ((UINT32) ((float) (1<<SHIFT)*(v)))

static const int val2chan[] =
{
	0, 1, 2, 3, 4, 5, 6 , -1,
	7, 8, 9, 10,11,12,13, -1,
	14,15,16,17,18,19,20, -1,
	21,22,23,24,25,26,27, -1,
};


#define SHIFT       12


#define MULTIPCM_RATE   44100.0




/*******************************
        ENVELOPE SECTION
*******************************/

//Times are based on a 44100Hz timebase. It's adjusted to the actual sampling rate on startup

static const double BaseTimes[64]={0,0,0,0,6222.95,4978.37,4148.66,3556.01,3111.47,2489.21,2074.33,1778.00,1555.74,1244.63,1037.19,889.02,
777.87,622.31,518.59,444.54,388.93,311.16,259.32,222.27,194.47,155.60,129.66,111.16,97.23,77.82,64.85,55.60,
48.62,38.91,32.43,27.80,24.31,19.46,16.24,13.92,12.15,9.75,8.12,6.98,6.08,4.90,4.08,3.49,
3.04,2.49,2.13,1.90,1.72,1.41,1.18,1.04,0.91,0.73,0.59,0.50,0.45,0.45,0.45,0.45};
#define AR2DR   14.32833
static signed int lin2expvol[0x400];
static int TLSteps[2];

#define EG_SHIFT    16

static int EG_Update(SLOT *slot)
{
	switch(slot->EG.state)
	{
		case ATTACK:
			slot->EG.volume+=slot->EG.AR;
			if(slot->EG.volume>=(0x3ff<<EG_SHIFT))
			{
				slot->EG.state=DECAY1;
				if(slot->EG.D1R>=(0x400<<EG_SHIFT)) //Skip DECAY1, go directly to DECAY2
					slot->EG.state=DECAY2;
				slot->EG.volume=0x3ff<<EG_SHIFT;
			}
			break;
		case DECAY1:
			slot->EG.volume-=slot->EG.D1R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;
			if(slot->EG.volume>>EG_SHIFT<=(slot->EG.DL<<(10-4)))
				slot->EG.state=DECAY2;
			break;
		case DECAY2:
			slot->EG.volume-=slot->EG.D2R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;
			break;
		case RELEASE:
			slot->EG.volume-=slot->EG.RR;
			if(slot->EG.volume<=0)
			{
				slot->EG.volume=0;
				slot->Playing=0;
			}
			break;
		default:
			return 1<<SHIFT;
	}
	return lin2expvol[slot->EG.volume>>EG_SHIFT];
}

static unsigned int Get_RATE(unsigned int *Steps,unsigned int rate,unsigned int val)
{
	int r=4*val+rate;
	if(val==0)
		return Steps[0];
	if(val==0xf)
		return Steps[0x3f];
	if(r>0x3f)
		r=0x3f;
	return Steps[r];
}

void multipcm_device::EG_Calc(SLOT *slot)
{
	int octave=((slot->Regs[3]>>4)-1)&0xf;
	int rate;
	if(octave&8) octave=octave-16;
	if(slot->Sample->KRS!=0xf)
		rate=(octave+slot->Sample->KRS)*2+((slot->Regs[3]>>3)&1);
	else
		rate=0;

	slot->EG.AR=Get_RATE(m_ARStep,rate,slot->Sample->AR);
	slot->EG.D1R=Get_RATE(m_DRStep,rate,slot->Sample->DR1);
	slot->EG.D2R=Get_RATE(m_DRStep,rate,slot->Sample->DR2);
	slot->EG.RR=Get_RATE(m_DRStep,rate,slot->Sample->RR);
	slot->EG.DL=0xf-slot->Sample->DL;

}

/*****************************
        LFO  SECTION
*****************************/

#define LFO_SHIFT   8


#define LFIX(v) ((unsigned int) ((float) (1<<LFO_SHIFT)*(v)))

//Convert DB to multiply amplitude
#define DB(v)   LFIX(powf(10.0f,v/20.0f))

//Convert cents to step increment
#define CENTS(v) LFIX(powf(2.0f,v/1200.0f))

static int PLFO_TRI[256];
static int ALFO_TRI[256];

static const float LFOFreq[8]={0.168f,2.019f,3.196f,4.206f,5.215f,5.888f,6.224f,7.066f};    //Hz;
static const float PSCALE[8]={0.0f,3.378f,5.065f,6.750f,10.114f,20.170f,40.180f,79.307f};   //cents
static const float ASCALE[8]={0.0f,0.4f,0.8f,1.5f,3.0f,6.0f,12.0f,24.0f};                   //DB
static int PSCALES[8][256];
static int ASCALES[8][256];

static void LFO_Init(void)
{
	int i,s;
	for(i=0;i<256;++i)
	{
		int a;  //amplitude
		int p;  //phase

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
		ALFO_TRI[i]=a;
		PLFO_TRI[i]=p;
	}

	for(s=0;s<8;++s)
	{
		float limit=PSCALE[s];
		for(i=-128;i<128;++i)
		{
			PSCALES[s][i+128]=CENTS(((limit*(float) i)/128.0f));
		}
		limit=-ASCALE[s];
		for(i=0;i<256;++i)
		{
			ASCALES[s][i]=DB(((limit*(float) i)/256.0f));
		}
	}
}

INLINE signed int PLFO_Step(LFO_t *LFO)
{
	int p;
	LFO->phase+=LFO->phase_step;
	p=LFO->table[(LFO->phase>>LFO_SHIFT)&0xff];
	p=LFO->scale[p+128];
	return p<<(SHIFT-LFO_SHIFT);
}

INLINE signed int ALFO_Step(LFO_t *LFO)
{
	int p;
	LFO->phase+=LFO->phase_step;
	p=LFO->table[(LFO->phase>>LFO_SHIFT)&0xff];
	p=LFO->scale[p];
	return p<<(SHIFT-LFO_SHIFT);
}

void multipcm_device::LFO_ComputeStep(LFO_t *LFO,UINT32 LFOF,UINT32 LFOS,int ALFO)
{
	float step=(float) LFOFreq[LFOF]*256.0f/(float) m_Rate;
	LFO->phase_step=(unsigned int) ((float) (1<<LFO_SHIFT)*step);
	if(ALFO)
	{
		LFO->table=ALFO_TRI;
		LFO->scale=ASCALES[LFOS];
	}
	else
	{
		LFO->table=PLFO_TRI;
		LFO->scale=PSCALES[LFOS];
	}
}



void multipcm_device::WriteSlot(SLOT *slot,int reg,unsigned char data)
{
	slot->Regs[reg]=data;

	switch(reg)
	{
		case 0: //PANPOT
			slot->Pan=(data>>4)&0xf;
			break;
		case 1: //Sample
			//according to YMF278 sample write causes some base params written to the regs (envelope+lfos)
			//the game should never change the sample while playing.
			{
				Sample_t *Sample=m_Samples+slot->Regs[1];
				WriteSlot(slot,6,Sample->LFOVIB);
				WriteSlot(slot,7,Sample->AM);
			}
			break;
		case 2: //Pitch
		case 3:
			{
				unsigned int oct=((slot->Regs[3]>>4)-1)&0xf;
				unsigned int pitch=((slot->Regs[3]&0xf)<<6)|(slot->Regs[2]>>2);
				pitch=m_FNS_Table[pitch];
				if(oct&0x8)
					pitch>>=(16-oct);
				else
					pitch<<=oct;
				slot->step=pitch/m_Rate;
			}
			break;
		case 4:     //KeyOn/Off (and more?)
			{
				if(data&0x80)       //KeyOn
				{
					slot->Sample=m_Samples+slot->Regs[1];
					slot->Playing=1;
					slot->Base=slot->Sample->Start;
					slot->offset=0;
					slot->Prev=0;
					slot->TL=slot->DstTL<<SHIFT;

					EG_Calc(slot);
					slot->EG.state=ATTACK;
					slot->EG.volume=0;

					if(slot->Base>=0x100000)
					{
						if(slot->Pan&8)
							slot->Base=(slot->Base&0xfffff)|(m_BankL);
						else
							slot->Base=(slot->Base&0xfffff)|(m_BankR);
					}

				}
				else
				{
					if(slot->Playing)
					{
						if(slot->Sample->RR!=0xf)
							slot->EG.state=RELEASE;
						else
							slot->Playing=0;
					}
				}
			}
			break;
		case 5: //TL+Interpolation
			{
				slot->DstTL=(data>>1)&0x7f;
				if(!(data&1))   //Interpolate TL
				{
					if((slot->TL>>SHIFT)>slot->DstTL)
						slot->TLStep=TLSteps[0];        //decrease
					else
						slot->TLStep=TLSteps[1];        //increase
				}
				else
					slot->TL=slot->DstTL<<SHIFT;
			}
			break;
		case 6: //LFO freq+PLFO
			{
				if(data)
				{
					LFO_ComputeStep(&(slot->PLFO),(slot->Regs[6]>>3)&7,slot->Regs[6]&7,0);
					LFO_ComputeStep(&(slot->ALFO),(slot->Regs[6]>>3)&7,slot->Regs[7]&7,1);
				}
			}
			break;
		case 7: //ALFO
			{
				if(data)
				{
					LFO_ComputeStep(&(slot->PLFO),(slot->Regs[6]>>3)&7,slot->Regs[6]&7,0);
					LFO_ComputeStep(&(slot->ALFO),(slot->Regs[6]>>3)&7,slot->Regs[7]&7,1);
				}
			}
			break;
	}
}

READ8_MEMBER( multipcm_device::read )
{
	return 0;
}


WRITE8_MEMBER( multipcm_device::write )
{
	switch(offset)
	{
		case 0:     //Data write
			WriteSlot(m_Slots+m_CurSlot,m_Address,data);
			break;
		case 1:
			m_CurSlot=val2chan[data&0x1f];
			break;

		case 2:
			m_Address=(data>7)?7:data;
			break;
	}
}

/* MAME/M1 access functions */

void multipcm_device::set_bank(UINT32 leftoffs, UINT32 rightoffs)
{
	m_BankL = leftoffs;
	m_BankR = rightoffs;
}

const device_type MULTIPCM = &device_creator<multipcm_device>;

// default address map
static ADDRESS_MAP_START( multipcm, AS_0, 8, multipcm_device )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
ADDRESS_MAP_END

multipcm_device::multipcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MULTIPCM, "Sega/Yamaha 315-5560", tag, owner, clock, "multipcm", __FILE__),
		device_sound_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		m_space_config("mpcm_samples", ENDIANNESS_LITTLE, 8, 24, 0, NULL),
		m_stream(NULL),
		//m_Samples(0x200),
		//m_Slots[28],
		m_CurSlot(0),
		m_Address(0),
		m_BankR(0),
		m_BankL(0),
		m_Rate(0)
		//m_ARStep(0),
		//m_DRStep(0),
		//m_FNS_Table(0)
{
	m_address_map[0] = *ADDRESS_MAP_NAME(multipcm);
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *multipcm_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void multipcm_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void multipcm_device::device_start()
{
		int i;

	// find our direct access
	m_direct = &space().direct();

	m_Rate=(float) clock() / MULTIPCM_CLOCKDIV;

	m_stream = machine().sound().stream_alloc(*this, 0, 2, m_Rate);

	//Volume+pan table
	for(i=0;i<0x800;++i)
	{
		float SegaDB=0;
		float TL;
		float LPAN,RPAN;

		unsigned char iTL=i&0x7f;
		unsigned char iPAN=(i>>7)&0xf;

		SegaDB=(float) iTL*(-24.0f)/(float) 0x40;

		TL=powf(10.0f,SegaDB/20.0f);


		if(iPAN==0x8)
		{
			LPAN=RPAN=0.0;
		}
		else if(iPAN==0x0)
		{
			LPAN=RPAN=1.0;
		}
		else if(iPAN&0x8)
		{
			LPAN=1.0;

			iPAN=0x10-iPAN;

			SegaDB=(float) iPAN*(-12.0f)/(float) 0x4;

			RPAN=pow(10.0f,SegaDB/20.0f);

			if((iPAN&0x7)==7)
				RPAN=0.0;
		}
		else
		{
			RPAN=1.0;

			SegaDB=(float) iPAN*(-12.0f)/(float) 0x4;

			LPAN=pow(10.0f,SegaDB/20.0f);
			if((iPAN&0x7)==7)
				LPAN=0.0;
		}

		TL/=4.0f;

		LPANTABLE[i]=FIX((LPAN*TL));
		RPANTABLE[i]=FIX((RPAN*TL));
	}

	//Pitch steps
	for(i=0;i<0x400;++i)
	{
		float fcent=m_Rate*(1024.0f+(float) i)/1024.0f;
		m_FNS_Table[i]=(unsigned int ) ((float) (1<<SHIFT) *fcent);
	}

	//Envelope steps
	for(i=0;i<0x40;++i)
	{
		//Times are based on 44100 clock, adjust to real chip clock
		m_ARStep[i]=(float) (0x400<<EG_SHIFT)/(float)(BaseTimes[i]*44100.0/(1000.0));
		m_DRStep[i]=(float) (0x400<<EG_SHIFT)/(float)(BaseTimes[i]*AR2DR*44100.0/(1000.0));
	}
	m_ARStep[0]=m_ARStep[1]=m_ARStep[2]=m_ARStep[3]=0;
	m_ARStep[0x3f]=0x400<<EG_SHIFT;
	m_DRStep[0]=m_DRStep[1]=m_DRStep[2]=m_DRStep[3]=0;

	//TL Interpolation steps
	//lower
	TLSteps[0]=-(float) (0x80<<SHIFT)/(78.2f*44100.0f/1000.0f);
	//raise
	TLSteps[1]=(float) (0x80<<SHIFT)/(78.2f*2*44100.0f/1000.0f);

	//build the linear->exponential ramps
	for(i=0;i<0x400;++i)
	{
		float db=-(96.0f-(96.0f*(float) i/(float) 0x400));
		lin2expvol[i]=powf(10.0f,db/20.0f)*(float) (1<<SHIFT);
	}


	for(i=0;i<512;++i)
	{
		UINT8 ptSample[12];

		for (int j = 0; j < 12; j++)
		{
			ptSample[j] = (UINT8)m_direct->read_byte((i*12) + j);
		}

		m_Samples[i].Start=(ptSample[0]<<16)|(ptSample[1]<<8)|(ptSample[2]<<0);
		m_Samples[i].Loop=(ptSample[3]<<8)|(ptSample[4]<<0);
		m_Samples[i].End=0xffff-((ptSample[5]<<8)|(ptSample[6]<<0));
		m_Samples[i].LFOVIB=ptSample[7];
		m_Samples[i].DR1=ptSample[8]&0xf;
		m_Samples[i].AR=(ptSample[8]>>4)&0xf;
		m_Samples[i].DR2=ptSample[9]&0xf;
		m_Samples[i].DL=(ptSample[9]>>4)&0xf;
		m_Samples[i].RR=ptSample[10]&0xf;
		m_Samples[i].KRS=(ptSample[10]>>4)&0xf;
		m_Samples[i].AM=ptSample[11];
	}

	save_item(NAME(m_CurSlot));
	save_item(NAME(m_Address));
	save_item(NAME(m_BankL));
	save_item(NAME(m_BankR));

	for(i=0;i<28;++i)
	{
		m_Slots[i].Num=i;
		m_Slots[i].Playing=0;

		save_item(NAME(m_Slots[i].Num), i);
		save_item(NAME(m_Slots[i].Regs), i);
		save_item(NAME(m_Slots[i].Playing), i);
		save_item(NAME(m_Slots[i].Base), i);
		save_item(NAME(m_Slots[i].offset), i);
		save_item(NAME(m_Slots[i].step), i);
		save_item(NAME(m_Slots[i].Pan), i);
		save_item(NAME(m_Slots[i].TL), i);
		save_item(NAME(m_Slots[i].DstTL), i);
		save_item(NAME(m_Slots[i].TLStep), i);
		save_item(NAME(m_Slots[i].Prev), i);
		save_item(NAME(m_Slots[i].EG.volume), i);
		save_item(NAME(m_Slots[i].EG.state), i);
		save_item(NAME(m_Slots[i].EG.step), i);
		save_item(NAME(m_Slots[i].EG.AR), i);
		save_item(NAME(m_Slots[i].EG.D1R), i);
		save_item(NAME(m_Slots[i].EG.D2R), i);
		save_item(NAME(m_Slots[i].EG.RR), i);
		save_item(NAME(m_Slots[i].EG.DL), i);
		save_item(NAME(m_Slots[i].PLFO.phase), i);
		save_item(NAME(m_Slots[i].PLFO.phase_step), i);
		save_item(NAME(m_Slots[i].ALFO.phase), i);
		save_item(NAME(m_Slots[i].ALFO.phase_step), i);
	}

	LFO_Init();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void multipcm_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t  *datap[2];
	int i,sl;

	datap[0] = outputs[0];
	datap[1] = outputs[1];

	memset(datap[0], 0, sizeof(*datap[0])*samples);
	memset(datap[1], 0, sizeof(*datap[1])*samples);

	for(i=0;i<samples;++i)
	{
		signed int smpl=0;
		signed int smpr=0;
		for(sl=0;sl<28;++sl)
		{
			SLOT *slot=m_Slots+sl;
			if(slot->Playing)
			{
				unsigned int vol=(slot->TL>>SHIFT)|(slot->Pan<<7);
				unsigned int adr=slot->offset>>SHIFT;
				signed int sample;
				unsigned int step=slot->step;
				signed int csample=(signed short) (m_direct->read_byte(slot->Base+adr)<<8);
				signed int fpart=slot->offset&((1<<SHIFT)-1);
				sample=(csample*fpart+slot->Prev*((1<<SHIFT)-fpart))>>SHIFT;

				if(slot->Regs[6]&7) //Vibrato enabled
				{
					step=step*PLFO_Step(&(slot->PLFO));
					step>>=SHIFT;
				}

				slot->offset+=step;
				if(slot->offset>=(slot->Sample->End<<SHIFT))
				{
					slot->offset=slot->Sample->Loop<<SHIFT;
				}
				if(adr^(slot->offset>>SHIFT))
				{
					slot->Prev=csample;
				}

				if((slot->TL>>SHIFT)!=slot->DstTL)
					slot->TL+=slot->TLStep;

				if(slot->Regs[7]&7) //Tremolo enabled
				{
					sample=sample*ALFO_Step(&(slot->ALFO));
					sample>>=SHIFT;
				}

				sample=(sample*EG_Update(slot))>>10;

				smpl+=(LPANTABLE[vol]*sample)>>SHIFT;
				smpr+=(RPANTABLE[vol]*sample)>>SHIFT;
			}
		}
#define ICLIP16(x) (x<-32768)?-32768:((x>32767)?32767:x)
		datap[0][i]=ICLIP16(smpl);
		datap[1][i]=ICLIP16(smpr);
	}
}
