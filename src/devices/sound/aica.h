// license:BSD-3-Clause
// copyright-holders:ElSemi, Deunan Knute, R. Belmont
// thanks-to: kingshriek
/*

    Sega/Yamaha AICA emulation
*/

#ifndef MAME_SOUND_AICA_H
#define MAME_SOUND_AICA_H

#pragma once

#include "aicadsp.h"


class aica_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	aica_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_master(bool master) { m_master = master; }
	void set_roffset(int roffset) { m_roffset = roffset; }
	auto irq() { return m_irq_cb.bind(); }
	auto main_irq() { return m_main_irq_cb.bind(); }

	// AICA register access
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	// MIDI I/O access
	DECLARE_WRITE16_MEMBER( midi_in );
	DECLARE_READ16_MEMBER( midi_out_r );

	void set_ram_base(void *base, int size);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	enum AICA_STATE {AICA_ATTACK,AICA_DECAY1,AICA_DECAY2,AICA_RELEASE};

	struct AICA_LFO_t
	{
		unsigned short phase;
		uint32_t phase_step;
		int *table;
		int *scale;
	};

	struct AICA_EG_t
	{
		int volume; //
		AICA_STATE state;
		int step;
		//step vals
		int AR;     //Attack
		int D1R;    //Decay1
		int D2R;    //Decay2
		int RR;     //Release

		int DL;     //Decay level
		uint8_t LPLINK;
	};

	struct AICA_SLOT
	{
		union
		{
			uint16_t data[0x40];  //only 0x1a bytes used
			uint8_t datab[0x80];
		} udata;
		uint8_t active;   //this slot is currently playing
		uint8_t *base;        //samples base address
		uint32_t prv_addr;    // previous play address (for ADPCM)
		uint32_t cur_addr;    //current play address (24.8)
		uint32_t nxt_addr;    //next play address
		uint32_t step;        //pitch step (24.8)
		uint8_t Backwards;    //the wave is playing backwards
		AICA_EG_t EG;            //Envelope
		AICA_LFO_t PLFO;     //Phase LFO
		AICA_LFO_t ALFO;     //Amplitude LFO
		int slot;
		int cur_sample;       //current ADPCM sample
		int cur_quant;        //current ADPCM step
		int curstep;
		int cur_lpquant, cur_lpsample, cur_lpstep;
		uint8_t *adbase, *adlpbase;
		uint8_t lpend;
	};


	unsigned char DecodeSCI(unsigned char irq);
	void ResetInterrupts();

	void CheckPendingIRQ();
	void CheckPendingIRQ_SH4();
	TIMER_CALLBACK_MEMBER( timerA_cb );
	TIMER_CALLBACK_MEMBER( timerB_cb );
	TIMER_CALLBACK_MEMBER( timerC_cb );
	int Get_AR(int base,int R);
	int Get_DR(int base,int R);
	int Get_RR(int base,int R);
	void Compute_EG(AICA_SLOT *slot);
	int EG_Update(AICA_SLOT *slot);
	uint32_t Step(AICA_SLOT *slot);
	void Compute_LFO(AICA_SLOT *slot);
	void InitADPCM(int *PrevSignal, int *PrevQuant);
	inline signed short DecodeADPCM(int *PrevSignal, unsigned char Delta, int *PrevQuant);
	void StartSlot(AICA_SLOT *slot);
	void StopSlot(AICA_SLOT *slot,int keyoff);
	void Init();
	void ClockChange();
	void UpdateSlotReg(int s,int r);
	void UpdateReg(address_space &space, int reg);
	void UpdateSlotRegR(int slot,int reg);
	void UpdateRegR(address_space &space, int reg);
	void w16(address_space &space,unsigned int addr,unsigned short val);
	unsigned short r16(address_space &space, unsigned int addr);
	inline int32_t UpdateSlot(AICA_SLOT *slot);
	void DoMasterSamples(int nsamples);
	void aica_exec_dma(address_space &space);


	void AICALFO_Init();
	inline signed int AICAPLFO_Step(AICA_LFO_t *LFO);
	inline signed int AICAALFO_Step(AICA_LFO_t *LFO);
	void AICALFO_ComputeStep(AICA_LFO_t *LFO,uint32_t LFOF,uint32_t LFOWS,uint32_t LFOS,int ALFO);

	bool m_master;
	double m_rate;
	int m_roffset;                /* offset in the region */
	devcb_write_line m_irq_cb;
	devcb_write_line m_main_irq_cb;
	optional_memory_region m_ram_region;

	union
	{
		uint16_t data[0xc0/2];
		uint8_t datab[0xc0];
	} m_udata;

	uint16_t m_IRQL, m_IRQR;
	uint16_t m_EFSPAN[0x48];
	AICA_SLOT m_Slots[64];
	signed short m_RINGBUF[64];
	unsigned char m_BUFPTR;
	unsigned char *m_AICARAM;
	uint32_t m_AICARAM_LENGTH, m_RAM_MASK, m_RAM_MASK16;
	sound_stream * m_stream;

	std::vector<int32_t> m_buffertmpl;
	std::vector<int32_t> m_buffertmpr;

	uint32_t m_IrqTimA;
	uint32_t m_IrqTimBC;
	uint32_t m_IrqMidi;

	uint8_t m_MidiOutW,m_MidiOutR;
	uint8_t m_MidiStack[16];
	uint8_t m_MidiW,m_MidiR;

	int m_LPANTABLE[0x20000];
	int m_RPANTABLE[0x20000];

	int m_TimPris[3];
	int m_TimCnt[3];

	uint16_t m_mcieb, m_mcipd;

	// timers
	emu_timer *m_timerA, *m_timerB, *m_timerC;

	// DMA stuff
	struct {
		uint32_t dmea;
		uint16_t drga;
		uint16_t dlg;
		uint8_t dgate;
		uint8_t ddir;
	} m_dma;


	int m_ARTABLE[64], m_DRTABLE[64];

	AICADSP m_DSP;

	stream_sample_t *m_bufferl;
	stream_sample_t *m_bufferr;
	stream_sample_t *m_exts0;
	stream_sample_t *m_exts1;

	int m_length;

	signed short *m_RBUFDST;   //this points to where the sample will be stored in the RingBuf
	int32_t m_EG_TABLE[0x400];
	int m_PLFO_TRI[256],m_PLFO_SQR[256],m_PLFO_SAW[256],m_PLFO_NOI[256];
	int m_ALFO_TRI[256],m_ALFO_SQR[256],m_ALFO_SAW[256],m_ALFO_NOI[256];
	int m_PSCALES[8][256];
	int m_ASCALES[8][256];

};

DECLARE_DEVICE_TYPE(AICA, aica_device)

#endif // MAME_SOUND_AICA_H
