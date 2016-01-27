// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont
/*
    SCSP (YMF292-F) header
*/

#pragma once

#ifndef __SCSP_H__
#define __SCSP_H__

#include "scspdsp.h"

#define FM_DELAY    0    // delay in number of slots processed before samples are written to the FM ring buffer
				// driver code indicates should be 4, but sounds distorted then


#define MCFG_SCSP_ROFFSET(_offs) \
	scsp_device::set_roffset(*device, _offs);

#define MCFG_SCSP_IRQ_CB(_devcb) \
	devcb = &scsp_device::set_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_SCSP_MAIN_IRQ_CB(_devcb) \
	devcb = &scsp_device::set_main_irq_callback(*device, DEVCB_##_devcb);


enum SCSP_STATE {SCSP_ATTACK,SCSP_DECAY1,SCSP_DECAY2,SCSP_RELEASE};

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
	UINT8 EGHOLD;
	UINT8 LPLINK;
};

struct SCSP_LFO_t
{
	UINT16 phase;
	UINT32 phase_step;
	int *table;
	int *scale;
};

struct SCSP_SLOT
{
	union
	{
		UINT16 data[0x10];  //only 0x1a bytes used
		UINT8 datab[0x20];
	} udata;

	UINT8 Backwards;    //the wave is playing backwards
	UINT8 active;   //this slot is currently playing
	UINT8 *base;        //samples base address
	UINT32 cur_addr;    //current play address (24.8)
	UINT32 nxt_addr;    //next play address
	UINT32 step;        //pitch step (24.8)
	SCSP_EG_t EG;            //Envelope
	SCSP_LFO_t PLFO;     //Phase LFO
	SCSP_LFO_t ALFO;     //Amplitude LFO
	int slot;
	INT16 Prev;  //Previous sample (for interpolation)
};


class scsp_device : public device_t,
					public device_sound_interface
{
public:
	scsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_roffset(device_t &device, int roffset) { downcast<scsp_device &>(device).m_roffset = roffset; }
	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<scsp_device &>(device).m_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_main_irq_callback(device_t &device, _Object object) { return downcast<scsp_device &>(device).m_main_irq_cb.set_callback(object); }

	// SCSP register access
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	// MIDI I/O access (used for comms on Model 2/3)
	DECLARE_WRITE16_MEMBER( midi_in );
	DECLARE_READ16_MEMBER( midi_out_r );

	void set_ram_base(void *base);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	int m_roffset;                /* offset in the region */
	devcb_write8       m_irq_cb;  /* irq callback */
	devcb_write_line   m_main_irq_cb;

	union
	{
		UINT16 data[0x30/2];
		UINT8 datab[0x30];
	} m_udata;

	SCSP_SLOT m_Slots[32];
	INT16 m_RINGBUF[128];
	UINT8 m_BUFPTR;
#if FM_DELAY
	INT16 m_DELAYBUF[FM_DELAY];
	UINT8 m_DELAYPTR;
#endif
	UINT8 *m_SCSPRAM;
	UINT32 m_SCSPRAM_LENGTH;
	char m_Master;
	sound_stream * m_stream;

	std::unique_ptr<INT32[]> m_buffertmpl;
	std::unique_ptr<INT32[]> m_buffertmpr;

	UINT32 m_IrqTimA;
	UINT32 m_IrqTimBC;
	UINT32 m_IrqMidi;

	UINT8 m_MidiOutW, m_MidiOutR;
	UINT8 m_MidiStack[32];
	UINT8 m_MidiW, m_MidiR;

	INT32 m_EG_TABLE[0x400];

	int m_LPANTABLE[0x10000];
	int m_RPANTABLE[0x10000];

	int m_TimPris[3];
	int m_TimCnt[3];

	// timers
	emu_timer *m_timerA, *m_timerB, *m_timerC;

	// DMA stuff
	struct
	{
		UINT32 dmea;
		UINT16 drga;
		UINT16 dtlg;
		UINT8 dgate;
		UINT8 ddir;
	} m_dma;

	UINT16 m_mcieb;
	UINT16 m_mcipd;

	int m_ARTABLE[64], m_DRTABLE[64];

	SCSPDSP m_DSP;

	stream_sample_t *m_bufferl;
	stream_sample_t *m_bufferr;

	int m_length;

	INT16 *m_RBUFDST;   //this points to where the sample will be stored in the RingBuf

	//LFO
	int m_PLFO_TRI[256], m_PLFO_SQR[256], m_PLFO_SAW[256], m_PLFO_NOI[256];
	int m_ALFO_TRI[256], m_ALFO_SQR[256], m_ALFO_SAW[256], m_ALFO_NOI[256];
	int m_PSCALES[8][256];
	int m_ASCALES[8][256];

	void exec_dma(address_space &space);       /*state DMA transfer function*/
	UINT8 DecodeSCI(UINT8 irq);
	void CheckPendingIRQ();
	void MainCheckPendingIRQ(UINT16 irq_type);
	void ResetInterrupts();
	TIMER_CALLBACK_MEMBER( timerA_cb );
	TIMER_CALLBACK_MEMBER( timerB_cb );
	TIMER_CALLBACK_MEMBER( timerC_cb );
	int Get_AR(int base, int R);
	int Get_DR(int base, int R);
	int Get_RR(int base, int R);
	void Compute_EG(SCSP_SLOT *slot);
	int EG_Update(SCSP_SLOT *slot);
	UINT32 Step(SCSP_SLOT *slot);
	void Compute_LFO(SCSP_SLOT *slot);
	void StartSlot(SCSP_SLOT *slot);
	void StopSlot(SCSP_SLOT *slot, int keyoff);
	void init();
	void UpdateSlotReg(int s, int r);
	void UpdateReg(address_space &space, int reg);
	void UpdateSlotRegR(int slot, int reg);
	void UpdateRegR(address_space &space, int reg);
	void w16(address_space &space, UINT32 addr, UINT16 val);
	UINT16 r16(address_space &space, UINT32 addr);
	inline INT32 UpdateSlot(SCSP_SLOT *slot);
	void DoMasterSamples(int nsamples);

	//LFO
	void LFO_Init();
	INT32 PLFO_Step(SCSP_LFO_t *LFO);
	INT32 ALFO_Step(SCSP_LFO_t *LFO);
	void LFO_ComputeStep(SCSP_LFO_t *LFO, UINT32 LFOF, UINT32 LFOWS, UINT32 LFOS, int ALFO);
};

extern const device_type SCSP;


#endif /* __SCSP_H__ */
