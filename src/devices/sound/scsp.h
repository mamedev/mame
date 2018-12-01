// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont
/*
    SCSP (YMF292-F) header
*/

#ifndef MAME_SOUND_SCSP_H
#define MAME_SOUND_SCSP_H

#pragma once

#include "scspdsp.h"

#define SCSP_FM_DELAY    0    // delay in number of slots processed before samples are written to the FM ring buffer
				// driver code indicates should be 4, but sounds distorted then


class scsp_device : public device_t,
	public device_sound_interface,
	public device_rom_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // DSP incorrections, etc

	scsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 22'579'200);

	auto irq_cb() { return m_irq_cb.bind(); }
	auto main_irq_cb() { return m_main_irq_cb.bind(); }

	// SCSP register access
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	// MIDI I/O access (used for comms on Model 2/3)
	void midi_in(u8 data);
	DECLARE_READ16_MEMBER( midi_out_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	virtual void rom_bank_updated() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

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
		uint8_t EGHOLD;
		uint8_t LPLINK;
	};

	struct SCSP_LFO_t
	{
		uint16_t phase;
		uint32_t phase_step;
		int *table;
		int *scale;
	};

	struct SCSP_SLOT
	{
		union
		{
			uint16_t data[0x10];  //only 0x1a bytes used
			uint8_t datab[0x20];
		} udata;

		uint8_t Backwards;    //the wave is playing backwards
		uint8_t active;   //this slot is currently playing
		uint32_t cur_addr;    //current play address (24.8)
		uint32_t nxt_addr;    //next play address
		uint32_t step;        //pitch step (24.8)
		SCSP_EG_t EG;            //Envelope
		SCSP_LFO_t PLFO;     //Phase LFO
		SCSP_LFO_t ALFO;     //Amplitude LFO
		int slot;
		int16_t Prev;  //Previous sample (for interpolation)
	};

	devcb_write8       m_irq_cb;  /* irq callback */
	devcb_write_line   m_main_irq_cb;

	union
	{
		uint16_t data[0x30/2];
		uint8_t datab[0x30];
	} m_udata;

	SCSP_SLOT m_Slots[32];
	int16_t m_RINGBUF[128];
	uint8_t m_BUFPTR;
#if SCSP_FM_DELAY
	int16_t m_DELAYBUF[SCSP_FM_DELAY];
	uint8_t m_DELAYPTR;
#endif
	char m_Master;
	sound_stream * m_stream;

	uint32_t m_IrqTimA;
	uint32_t m_IrqTimBC;
	uint32_t m_IrqMidi;

	uint8_t m_MidiOutW, m_MidiOutR;
	uint8_t m_MidiStack[32];
	uint8_t m_MidiW, m_MidiR;

	int32_t m_EG_TABLE[0x400];

	int m_LPANTABLE[0x10000];
	int m_RPANTABLE[0x10000];

	int m_TimPris[3];
	int m_TimCnt[3];

	// timers
	emu_timer *m_timerA, *m_timerB, *m_timerC;

	// DMA stuff
	struct
	{
		uint32_t dmea;
		uint16_t drga;
		uint16_t dtlg;
		uint8_t dgate;
		uint8_t ddir;
	} m_dma;

	uint16_t m_mcieb;
	uint16_t m_mcipd;

	int m_ARTABLE[64], m_DRTABLE[64];

	SCSPDSP m_DSP;

	stream_sample_t *m_bufferl;
	stream_sample_t *m_bufferr;
	stream_sample_t *m_exts0;
	stream_sample_t *m_exts1;

	int m_length;

	int16_t *m_RBUFDST;   //this points to where the sample will be stored in the RingBuf

	//LFO
	int m_PLFO_TRI[256], m_PLFO_SQR[256], m_PLFO_SAW[256], m_PLFO_NOI[256];
	int m_ALFO_TRI[256], m_ALFO_SQR[256], m_ALFO_SAW[256], m_ALFO_NOI[256];
	int m_PSCALES[8][256];
	int m_ASCALES[8][256];

	void exec_dma();       /*state DMA transfer function*/
	uint8_t DecodeSCI(uint8_t irq);
	void CheckPendingIRQ();
	void MainCheckPendingIRQ(uint16_t irq_type);
	void ResetInterrupts();
	TIMER_CALLBACK_MEMBER( timerA_cb );
	TIMER_CALLBACK_MEMBER( timerB_cb );
	TIMER_CALLBACK_MEMBER( timerC_cb );
	int Get_AR(int base, int R);
	int Get_DR(int base, int R);
	int Get_RR(int base, int R);
	void Compute_EG(SCSP_SLOT *slot);
	int EG_Update(SCSP_SLOT *slot);
	uint32_t Step(SCSP_SLOT *slot);
	void Compute_LFO(SCSP_SLOT *slot);
	void StartSlot(SCSP_SLOT *slot);
	void StopSlot(SCSP_SLOT *slot, int keyoff);
	void init();
	void UpdateSlotReg(int s, int r);
	void UpdateReg(int reg);
	void UpdateSlotRegR(int slot, int reg);
	void UpdateRegR(int reg);
	void w16(uint32_t addr, uint16_t val);
	uint16_t r16(uint32_t addr);
	inline int32_t UpdateSlot(SCSP_SLOT *slot);
	void DoMasterSamples(int nsamples);

	//LFO
	void LFO_Init();
	int32_t PLFO_Step(SCSP_LFO_t *LFO);
	int32_t ALFO_Step(SCSP_LFO_t *LFO);
	void LFO_ComputeStep(SCSP_LFO_t *LFO, uint32_t LFOF, uint32_t LFOWS, uint32_t LFOS, int ALFO);
};

DECLARE_DEVICE_TYPE(SCSP, scsp_device)

#endif // MAME_SOUND_SCSP_H
