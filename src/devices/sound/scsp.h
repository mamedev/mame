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
	uint8_t *base;        //samples base address
	uint32_t cur_addr;    //current play address (24.8)
	uint32_t nxt_addr;    //next play address
	uint32_t step;        //pitch step (24.8)
	SCSP_EG_t EG;            //Envelope
	SCSP_LFO_t PLFO;     //Phase LFO
	SCSP_LFO_t ALFO;     //Amplitude LFO
	int slot;
	int16_t Prev;  //Previous sample (for interpolation)
};


class scsp_device : public device_t,
					public device_sound_interface
{
public:
	scsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_roffset(device_t &device, int roffset) { downcast<scsp_device &>(device).m_roffset = roffset; }
	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<scsp_device &>(device).m_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_main_irq_callback(device_t &device, _Object object) { return downcast<scsp_device &>(device).m_main_irq_cb.set_callback(object); }

	// SCSP register access
	uint16_t read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// MIDI I/O access (used for comms on Model 2/3)
	void midi_in(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midi_out_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

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
		uint16_t data[0x30/2];
		uint8_t datab[0x30];
	} m_udata;

	SCSP_SLOT m_Slots[32];
	int16_t m_RINGBUF[128];
	uint8_t m_BUFPTR;
#if FM_DELAY
	int16_t m_DELAYBUF[FM_DELAY];
	uint8_t m_DELAYPTR;
#endif
	uint8_t *m_SCSPRAM;
	uint32_t m_SCSPRAM_LENGTH;
	char m_Master;
	sound_stream * m_stream;

	std::unique_ptr<int32_t[]> m_buffertmpl;
	std::unique_ptr<int32_t[]> m_buffertmpr;

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

	int m_length;

	int16_t *m_RBUFDST;   //this points to where the sample will be stored in the RingBuf

	//LFO
	int m_PLFO_TRI[256], m_PLFO_SQR[256], m_PLFO_SAW[256], m_PLFO_NOI[256];
	int m_ALFO_TRI[256], m_ALFO_SQR[256], m_ALFO_SAW[256], m_ALFO_NOI[256];
	int m_PSCALES[8][256];
	int m_ASCALES[8][256];

	void exec_dma(address_space &space);       /*state DMA transfer function*/
	uint8_t DecodeSCI(uint8_t irq);
	void CheckPendingIRQ();
	void MainCheckPendingIRQ(uint16_t irq_type);
	void ResetInterrupts();
	void timerA_cb(void *ptr, int32_t param);
	void timerB_cb(void *ptr, int32_t param);
	void timerC_cb(void *ptr, int32_t param);
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
	void UpdateReg(address_space &space, int reg);
	void UpdateSlotRegR(int slot, int reg);
	void UpdateRegR(address_space &space, int reg);
	void w16(address_space &space, uint32_t addr, uint16_t val);
	uint16_t r16(address_space &space, uint32_t addr);
	inline int32_t UpdateSlot(SCSP_SLOT *slot);
	void DoMasterSamples(int nsamples);

	//LFO
	void LFO_Init();
	int32_t PLFO_Step(SCSP_LFO_t *LFO);
	int32_t ALFO_Step(SCSP_LFO_t *LFO);
	void LFO_ComputeStep(SCSP_LFO_t *LFO, uint32_t LFOF, uint32_t LFOWS, uint32_t LFOS, int ALFO);
};

extern const device_type SCSP;


#endif /* __SCSP_H__ */
