// license:BSD-3-Clause
// copyright-holders:Miguel Angel Horna
#pragma once

#ifndef __MULTIPCM_H__
#define __MULTIPCM_H__

struct Sample_t
{
	unsigned int Start;
	unsigned int Loop;
	unsigned int End;
	unsigned char AR,DR1,DR2,DL,RR;
	unsigned char KRS;
	unsigned char LFOVIB;
	unsigned char AM;
};

enum STATE {ATTACK,DECAY1,DECAY2,RELEASE};

struct EG_t
{
	int volume; //
	STATE state;
	int step;
	//step vals
	int AR;     //Attack
	int D1R;    //Decay1
	int D2R;    //Decay2
	int RR;     //Release
	int DL;     //Decay level
};

struct LFO_t
{
	unsigned short phase;
	UINT32 phase_step;
	int *table;
	int *scale;
};


struct SLOT
{
	unsigned char Num;
	unsigned char Regs[8];
	int Playing;
	Sample_t *Sample;
	unsigned int Base;
	unsigned int offset;
	unsigned int step;
	unsigned int Pan,TL;
	unsigned int DstTL;
	int TLStep;
	signed int Prev;
	EG_t EG;
	LFO_t PLFO; //Phase lfo
	LFO_t ALFO; //AM lfo
};

class multipcm_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
public:
	multipcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~multipcm_device() {}

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

	void set_bank(UINT32 leftoffs, UINT32 rightoffs);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	const address_space_config  m_space_config;

private:
	// internal state
	sound_stream * m_stream;
	Sample_t m_Samples[0x200];        //Max 512 samples
	SLOT m_Slots[28];
	unsigned int m_CurSlot;
	unsigned int m_Address;
	unsigned int m_BankR, m_BankL;
	float m_Rate;
	//I include these in the chip because they depend on the chip clock
	unsigned int m_ARStep[0x40], m_DRStep[0x40]; //Envelope step table
	unsigned int m_FNS_Table[0x400];      //Frequency step table

	void EG_Calc(SLOT *slot);
	void LFO_ComputeStep(LFO_t *LFO,UINT32 LFOF,UINT32 LFOS,int ALFO);
	void WriteSlot(SLOT *slot,int reg,unsigned char data);

	direct_read_data *m_direct;
};

extern const device_type MULTIPCM;


#endif /* __MULTIPCM_H__ */
