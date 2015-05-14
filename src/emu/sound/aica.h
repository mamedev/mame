// license:???
// copyright-holders:ElSemi, kingshriek, Deunan Knute, R. Belmont
/*

    Sega/Yamaha AICA emulation
*/

#ifndef __AICA_H__
#define __AICA_H__

#include "aicadsp.h"

#define MCFG_AICA_MASTER \
	aica_device::set_master(*device);

#define MCFG_AICA_ROFFSET(_offs) \
	aica_device::set_roffset(*device, _offs);

#define MCFG_AICA_IRQ_CB(_devcb) \
	devcb = &aica_device::set_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_AICA_MAIN_IRQ_CB(_devcb) \
	devcb = &aica_device::set_main_irq_callback(*device, DEVCB_##_devcb);

enum AICA_STATE {AICA_ATTACK,AICA_DECAY1,AICA_DECAY2,AICA_RELEASE};

struct AICA_LFO_t
{
	unsigned short phase;
	UINT32 phase_step;
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
	UINT8 LPLINK;
};

struct AICA_SLOT
{
	union
	{
		UINT16 data[0x40];  //only 0x1a bytes used
		UINT8 datab[0x80];
	} udata;
	UINT8 active;   //this slot is currently playing
	UINT8 *base;        //samples base address
	UINT32 prv_addr;    // previous play address (for ADPCM)
	UINT32 cur_addr;    //current play address (24.8)
	UINT32 nxt_addr;    //next play address
	UINT32 step;        //pitch step (24.8)
	UINT8 Backwards;    //the wave is playing backwards
	AICA_EG_t EG;            //Envelope
	AICA_LFO_t PLFO;     //Phase LFO
	AICA_LFO_t ALFO;     //Amplitude LFO
	int slot;
	int cur_sample;       //current ADPCM sample
	int cur_quant;        //current ADPCM step
	int curstep;
	int cur_lpquant, cur_lpsample, cur_lpstep;
	UINT8 *adbase, *adlpbase;
	UINT8 lpend;
};

class aica_device : public device_t,
									public device_sound_interface
{
public:
	aica_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_master(device_t &device) { downcast<aica_device &>(device).m_master = true; }
	static void set_roffset(device_t &device, int roffset) { downcast<aica_device &>(device).m_roffset = roffset; }
	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<aica_device &>(device).m_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_main_irq_callback(device_t &device, _Object object) { return downcast<aica_device &>(device).m_main_irq_cb.set_callback(object); }

	// AICA register access
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	// MIDI I/O access
	DECLARE_WRITE16_MEMBER( midi_in );
	DECLARE_READ16_MEMBER( midi_out_r );

	void set_ram_base(void *base, int size);

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
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
	UINT32 Step(AICA_SLOT *slot);
	void Compute_LFO(AICA_SLOT *slot);
	void InitADPCM(int *PrevSignal, int *PrevQuant);
	inline signed short DecodeADPCM(int *PrevSignal, unsigned char Delta, int *PrevQuant);
	void StartSlot(AICA_SLOT *slot);
	void StopSlot(AICA_SLOT *slot,int keyoff);
	void Init();
	void UpdateSlotReg(int s,int r);
	void UpdateReg(address_space &space, int reg);
	void UpdateSlotRegR(int slot,int reg);
	void UpdateRegR(address_space &space, int reg);
	void w16(address_space &space,unsigned int addr,unsigned short val);
	unsigned short r16(address_space &space, unsigned int addr);
	inline INT32 UpdateSlot(AICA_SLOT *slot);
	void DoMasterSamples(int nsamples);
	void aica_exec_dma(address_space &space);


	void AICALFO_Init();
	inline signed int AICAPLFO_Step(AICA_LFO_t *LFO);
	inline signed int AICAALFO_Step(AICA_LFO_t *LFO);
	void AICALFO_ComputeStep(AICA_LFO_t *LFO,UINT32 LFOF,UINT32 LFOWS,UINT32 LFOS,int ALFO);

	bool m_master;
	int m_roffset;                /* offset in the region */
	devcb_write_line m_irq_cb;
	devcb_write_line m_main_irq_cb;

	union
	{
		UINT16 data[0xc0/2];
		UINT8 datab[0xc0];
	} m_udata;

	UINT16 m_IRQL, m_IRQR;
	UINT16 m_EFSPAN[0x48];
	AICA_SLOT m_Slots[64];
	signed short m_RINGBUF[64];
	unsigned char m_BUFPTR;
	unsigned char *m_AICARAM;
	UINT32 m_AICARAM_LENGTH, m_RAM_MASK, m_RAM_MASK16;
	sound_stream * m_stream;

	INT32 *m_buffertmpl, *m_buffertmpr;

	UINT32 m_IrqTimA;
	UINT32 m_IrqTimBC;
	UINT32 m_IrqMidi;

	UINT8 m_MidiOutW,m_MidiOutR;
	UINT8 m_MidiStack[16];
	UINT8 m_MidiW,m_MidiR;

	int m_LPANTABLE[0x20000];
	int m_RPANTABLE[0x20000];

	int m_TimPris[3];
	int m_TimCnt[3];

	UINT16 m_mcieb, m_mcipd;

	// timers
	emu_timer *m_timerA, *m_timerB, *m_timerC;

	// DMA stuff
	struct{
		UINT32 dmea;
		UINT16 drga;
		UINT16 dlg;
		UINT8 dgate;
		UINT8 ddir;
	}m_dma;


	int m_ARTABLE[64], m_DRTABLE[64];

	AICADSP m_DSP;

	stream_sample_t *m_bufferl;
	stream_sample_t *m_bufferr;

	int m_length;

	signed short *m_RBUFDST;   //this points to where the sample will be stored in the RingBuf
	INT32 m_EG_TABLE[0x400];
	int m_PLFO_TRI[256],m_PLFO_SQR[256],m_PLFO_SAW[256],m_PLFO_NOI[256];
	int m_ALFO_TRI[256],m_ALFO_SQR[256],m_ALFO_SAW[256],m_ALFO_NOI[256];
	int m_PSCALES[8][256];
	int m_ASCALES[8][256];

};

extern const device_type AICA;


#endif /* __AICA_H__ */
