#pragma once

#ifndef __ISA_SOUND_BLASTER_H__
#define __ISA_SOUND_BLASTER_H__

#include "emu.h"
#include "machine/isa.h"
#include "sound/dac.h"
#include "machine/pc_joy.h"

#define SIXTEENBIT	0x01
#define STEREO		0x02
#define SIGNED		0x04
#define ADPCM2		0x08
#define ADPCM3		0x10
#define ADPCM4		0x20

#define IRQ_DMA8	0x01
#define IRQ_DMA16	0x02
#define IRQ_MPU		0x04
#define IRQ_ALL		0xff

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct sb8_dsp_state
{
    UINT8 reset_latch;
    UINT8 rbuf_status;
    UINT8 wbuf_status;
    UINT8 fifo[16],fifo_ptr;
    UINT8 fifo_r[52],fifo_r_ptr;
    UINT16 version;
    UINT8 test_reg;
    UINT8 speaker_on;
    UINT32 prot_count;
    INT32 prot_value;
    UINT32 frequency;
	UINT32 adc_freq;
    UINT32 dma_length, dma_transferred;
	UINT32 adc_length, adc_transferred;
    UINT8 dma_autoinit;
    UINT8 data[128], d_wptr, d_rptr;
    bool dma_timer_started;
    bool dma_throttled;
	UINT8 flags;
	UINT8 irq_active;
	bool adpcm_new_ref;
	UINT8 adpcm_ref;
	UINT8 adpcm_step;
	UINT8 adpcm_count;
};

// ======================> sb8_device (parent)

class sb_device :
		public device_t
{
public:
        // construction/destruction
        sb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, const char *name);

        required_device<dac_device> m_dacl;
        required_device<dac_device> m_dacr;
	required_device<pc_joy_device> m_joy;

        void process_fifo(UINT8 cmd);
        void queue(UINT8 data);
        void queue_r(UINT8 data);
        UINT8 dequeue_r();

        DECLARE_READ8_MEMBER(dsp_reset_r);
        DECLARE_WRITE8_MEMBER(dsp_reset_w);
        DECLARE_READ8_MEMBER(dsp_data_r);
        DECLARE_WRITE8_MEMBER(dsp_data_w);
        DECLARE_READ8_MEMBER(dsp_rbuf_status_r);
        DECLARE_READ8_MEMBER(dsp_wbuf_status_r);
        DECLARE_WRITE8_MEMBER(dsp_rbuf_status_w);
        DECLARE_WRITE8_MEMBER(dsp_cmd_w);
		DECLARE_READ8_MEMBER(mixer_r);
		DECLARE_WRITE8_MEMBER(mixer_w);

protected:
        // device-level overrides
        virtual void device_reset();
        UINT8 dack_r(int line);
        void dack_w(int line, UINT8 data);
        virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
		virtual void drq16_w(int state) { }
		virtual void drq_w(int state) { }
		virtual void irq_w(int state, int source) { }
		void adpcm_decode(UINT8 sample, int size);

        struct sb8_dsp_state m_dsp;
        UINT8 m_dack_out;
		UINT8 m_mixer_index;

        emu_timer *m_timer;
};

class sb8_device : public sb_device,
		    	   public device_isa8_card_interface
{
public:
        // construction/destruction
        sb8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, const char *name);
protected:
        virtual void device_start();
		virtual void drq_w(int state) { m_isa->drq1_w(state); }
		virtual void irq_w(int state, int source) { m_isa->irq5_w(state); }
        virtual UINT8 dack_r(int line) { return sb_device::dack_r(line); }
        virtual void dack_w(int line, UINT8 data) { sb_device::dack_w(line, data); }
};

class isa8_sblaster1_0_device : public sb8_device
{
public:
        // construction/destruction
        isa8_sblaster1_0_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
protected:
        // device-level overrides
        virtual void device_start();
		virtual void device_config_complete() { m_shortname = "isa_sblaster1_0"; }
private:
        // internal state
};

class isa8_sblaster1_5_device : public sb8_device
{
public:
        // construction/destruction
        isa8_sblaster1_5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
protected:
        // device-level overrides
        virtual void device_start();
		virtual void device_config_complete() { m_shortname = "isa_sblaster1_5"; }
private:
        // internal state
};

class sb16_device : public sb_device,
		    		public device_isa16_card_interface
{
public:
        // construction/destruction
        sb16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, const char *name);
        DECLARE_READ8_MEMBER(mpu401_r);
        DECLARE_WRITE8_MEMBER(mpu401_w);
protected:
        virtual void device_start();
		virtual UINT16 dack16_r(int line);
        virtual UINT8 dack_r(int line) { return sb_device::dack_r(line); }
        virtual void dack_w(int line, UINT8 data) { sb_device::dack_w(line, data); }
		virtual void dack16_w(int line, UINT16 data);
		virtual void drq16_w(int state) { m_isa->drq5_w(state); }
		virtual void drq_w(int state) { m_isa->drq1_w(state); }
		virtual void irq_w(int state, int source) { (state?m_dsp.irq_active|=source:m_dsp.irq_active&=~source); m_isa->irq5_w(m_dsp.irq_active&&1);  }
private:
		UINT8 m_mpu_queue[16];
		UINT8 m_tail;
		UINT8 m_head;
};

class isa16_sblaster16_device : public sb16_device
{
public:
		// construction/destruction
        isa16_sblaster16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
protected:
        // device-level overrides
        virtual void device_start();
		virtual void device_config_complete() { m_shortname = "isa_sblaster_16"; }
private:
        // internal state
};

// device type definition
extern const device_type ISA8_SOUND_BLASTER_1_0;
extern const device_type ISA8_SOUND_BLASTER_1_5;
extern const device_type ISA16_SOUND_BLASTER_16;

#endif  /* __ISA_SOUND_BLASTER_H__ */
