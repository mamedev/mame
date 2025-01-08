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


class aica_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	aica_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq() { return m_irq_cb.bind(); }
	auto main_irq() { return m_main_irq_cb.bind(); }

	// AICA register access
	u16 read(offs_t offset);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

	// MIDI I/O access
	void midi_in(u8 data);
	u8 midi_out_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;

private:
	enum AICA_STATE {AICA_ATTACK,AICA_DECAY1,AICA_DECAY2,AICA_RELEASE};

	struct AICA_LFO_t
	{
		u16 phase;
		u32 phase_step;
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
	};

	struct AICA_SLOT
	{
		union
		{
			u16 data[0x40];  //only 0x1a bytes used
			u8 datab[0x80];
		} udata;
		u8 active;   //this slot is currently playing
		u32 prv_addr;    // previous play address (for ADPCM)
		u32 cur_addr;    //current play address (24.8)
		u32 nxt_addr;    //next play address
		u32 step;        //pitch step (24.8)
		u8 Backwards;    //the wave is playing backwards
		AICA_EG_t EG;            //Envelope
		AICA_LFO_t PLFO;     //Phase LFO
		AICA_LFO_t ALFO;     //Amplitude LFO
		int slot;
		int cur_sample;       //current ADPCM sample
		int cur_quant;        //current ADPCM step
		int curstep;
		int cur_lpquant, cur_lpsample, cur_lpstep;
		u32 adbase;
		u8 lpend;
	};


	u8 DecodeSCI(u8 irq);
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
	u32 Step(AICA_SLOT *slot);
	void Compute_LFO(AICA_SLOT *slot);
	void InitADPCM(int *PrevSignal, int *PrevQuant);
	inline s16 DecodeADPCM(int *PrevSignal, u8 Delta, int *PrevQuant);
	void StartSlot(AICA_SLOT *slot);
	void StopSlot(AICA_SLOT *slot,int keyoff);
	void Init();
	void ClockChange();
	void UpdateSlotReg(int s,int r);
	void UpdateReg(int reg);
	void UpdateSlotRegR(int slot,int reg);
	void UpdateRegR(int reg);
	void w16(u32 addr,u16 val);
	u16 r16(u32 addr);
	[[maybe_unused]] void TimersAddTicks(int ticks);
	s32 UpdateSlot(AICA_SLOT *slot);
	void DoMasterSamples(std::vector<read_stream_view> const &inputs, write_stream_view &bufl, write_stream_view &bufr);
	void exec_dma();


	void LFO_Init();
	inline s32 PLFO_Step(AICA_LFO_t *LFO);
	inline s32 ALFO_Step(AICA_LFO_t *LFO);
	void LFO_ComputeStep(AICA_LFO_t *LFO,u32 LFOF,u32 LFOWS,u32 LFOS,int ALFO);

	double m_rate;
	devcb_write_line m_irq_cb;
	devcb_write_line m_main_irq_cb;

	union
	{
		u16 data[0xc0/2];
		u8 datab[0xc0];
	} m_udata;

	u16 m_IRQL, m_IRQR;
	u16 m_EFSPAN[0x48];
	AICA_SLOT m_Slots[64];

	memory_access<23, 1, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<23, 1, 0, ENDIANNESS_LITTLE>::specific m_data;
	sound_stream * m_stream;

	u32 m_IrqTimA;
	u32 m_IrqTimBC;
	u32 m_IrqMidi;

	u8 m_MidiOutW,m_MidiOutR;
	u8 m_MidiStack[16];
	u8 m_MidiW,m_MidiR;

	int m_LPANTABLE[0x20000];
	int m_RPANTABLE[0x20000];

	int m_TimPris[3];
	int m_TimCnt[3];

	u16 m_mcieb, m_mcipd;

	// timers
	emu_timer *m_timerA, *m_timerB, *m_timerC;

	// DMA stuff
	struct {
		u32 dmea;
		u16 drga;
		u16 dlg;
		u8 dgate;
		u8 ddir;
	} m_dma;


	int m_ARTABLE[64], m_DRTABLE[64];

	AICADSP m_DSP;

	s32 m_EG_TABLE[0x400];
	int m_PLFO_TRI[256],m_PLFO_SQR[256],m_PLFO_SAW[256],m_PLFO_NOI[256];
	int m_ALFO_TRI[256],m_ALFO_SQR[256],m_ALFO_SAW[256],m_ALFO_NOI[256];
	int m_PSCALES[8][256];
	int m_ASCALES[8][256];

};

DECLARE_DEVICE_TYPE(AICA, aica_device)

#endif // MAME_SOUND_AICA_H
