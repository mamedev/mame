// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont
/*
    SCSP (YMF292-F) header
*/

#ifndef MAME_SOUND_SCSP_H
#define MAME_SOUND_SCSP_H

#pragma once

#include "scspdsp.h"

#include "dirom.h"


#define SCSP_FM_DELAY    0    // delay in number of slots processed before samples are written to the FM ring buffer
				// driver code indicates should be 4, but sounds distorted then


class scsp_device : public device_t,
	public device_sound_interface,
	public device_rom_interface<20, 1, 0, ENDIANNESS_BIG>
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // DSP / EG incorrections, etc

	scsp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 22'579'200);

	auto irq_cb() { return m_irq_cb.bind(); }
	auto main_irq_cb() { return m_main_irq_cb.bind(); }

	// SCSP register access
	u16 read(offs_t offset);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

	// MIDI I/O access (used for comms on Model 2/3)
	void midi_in(u8 data);
	u16 midi_out_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	virtual void rom_bank_pre_change() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	enum SCSP_STATE { SCSP_ATTACK, SCSP_DECAY1, SCSP_DECAY2, SCSP_RELEASE };

	struct SCSP_EG_t
	{
		int volume; //
		SCSP_STATE state;
		int step;
		//step vals
		int AR;     //Attack
		int D1R;    //Decay1
		int D2R;    //Decay2
		int RR;     //Release

		int DL;     //Decay level
		u8 EGHOLD;
		u8 LPLINK;
	};

	struct SCSP_LFO_t
	{
		u16 phase;
		u32 phase_step;
		int *table;
		int *scale;
	};

	struct SCSP_SLOT
	{
		union
		{
			u16 data[0x10];  //only 0x1a bytes used
			u8 datab[0x20];
		} udata;

		u8 Backwards;    //the wave is playing backwards
		u8 active;   //this slot is currently playing
		u32 cur_addr;    //current play address (24.8)
		u32 nxt_addr;    //next play address
		u32 step;        //pitch step (24.8)
		SCSP_EG_t EG;            //Envelope
		SCSP_LFO_t PLFO;     //Phase LFO
		SCSP_LFO_t ALFO;     //Amplitude LFO
		int slot;
		s16 Prev;  //Previous sample (for interpolation)
	};

	devcb_write8       m_irq_cb;  /* irq callback */
	devcb_write_line   m_main_irq_cb;

	union
	{
		u16 data[0x30/2];
		u8 datab[0x30];
	} m_udata;

	SCSP_SLOT m_Slots[32];
	s16 m_RINGBUF[128];
	u8 m_BUFPTR;
#if SCSP_FM_DELAY
	s16 m_DELAYBUF[SCSP_FM_DELAY];
	u8 m_DELAYPTR;
#endif
	sound_stream * m_stream;

	u32 m_IrqTimA;
	u32 m_IrqTimBC;
	u32 m_IrqMidi;

	u8 m_MidiOutW, m_MidiOutR;
	u8 m_MidiStack[32];
	u8 m_MidiW, m_MidiR;

	s32 m_EG_TABLE[0x400];

	int m_LPANTABLE[0x10000];
	int m_RPANTABLE[0x10000];

	int m_TimPris[3];
	int m_TimCnt[3];

	// timers
	emu_timer *m_timerA, *m_timerB, *m_timerC;

	// DMA stuff
	struct
	{
		u32 dmea;
		u16 drga;
		u16 dtlg;
		u8 dgate;
		u8 ddir;
	} m_dma;

	u16 m_mcieb;
	u16 m_mcipd;

	int m_ARTABLE[64], m_DRTABLE[64];

	SCSPDSP m_DSP;

	s16 *m_RBUFDST;   //this points to where the sample will be stored in the RingBuf

	//LFO
	int m_PLFO_TRI[256], m_PLFO_SQR[256], m_PLFO_SAW[256], m_PLFO_NOI[256];
	int m_ALFO_TRI[256], m_ALFO_SQR[256], m_ALFO_SAW[256], m_ALFO_NOI[256];
	int m_PSCALES[8][256];
	int m_ASCALES[8][256];

	void exec_dma();       /*state DMA transfer function*/
	u8 DecodeSCI(u8 irq);
	void CheckPendingIRQ();
	void MainCheckPendingIRQ(u16 irq_type);
	void ResetInterrupts();
	TIMER_CALLBACK_MEMBER(timerA_cb);
	TIMER_CALLBACK_MEMBER(timerB_cb);
	TIMER_CALLBACK_MEMBER(timerC_cb);
	int Get_AR(int base, int R);
	int Get_DR(int base, int R);
	void Compute_EG(SCSP_SLOT *slot);
	int EG_Update(SCSP_SLOT *slot);
	u32 Step(SCSP_SLOT *slot);
	void Compute_LFO(SCSP_SLOT *slot);
	void StartSlot(SCSP_SLOT *slot);
	void StopSlot(SCSP_SLOT *slot, int keyoff);
	void init();
	void UpdateSlotReg(int s, int r);
	void UpdateReg(int reg);
	void UpdateSlotRegR(int slot, int reg);
	void UpdateRegR(int reg);
	void w16(u32 addr, u16 val);
	u16 r16(u32 addr);
	inline s32 UpdateSlot(SCSP_SLOT *slot);
	void DoMasterSamples(std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs);

	//LFO
	void LFO_Init();
	s32 PLFO_Step(SCSP_LFO_t *LFO);
	s32 ALFO_Step(SCSP_LFO_t *LFO);
	void LFO_ComputeStep(SCSP_LFO_t *LFO, u32 LFOF, u32 LFOWS, u32 LFOS, int ALFO);
};

DECLARE_DEVICE_TYPE(SCSP, scsp_device)

#endif // MAME_SOUND_SCSP_H
