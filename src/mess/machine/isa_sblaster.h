#pragma once

#ifndef __ISA_SOUND_BLASTER_H__
#define __ISA_SOUND_BLASTER_H__

#include "emu.h"
#include "machine/isa.h"
#include "sound/dac.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct sb8_dsp_state
{
    UINT8 reset_latch;
    UINT8 rbuf_status;
    UINT8 wbuf_status;
    UINT8 fifo[16],fifo_ptr;
    UINT8 fifo_r[16],fifo_r_ptr;
    UINT16 version;
    UINT8 test_reg;
    UINT8 speaker_on;
    UINT32 prot_count;
    INT32 prot_value;
    UINT32 frequency;
    UINT32 dma_length, dma_transferred;
    UINT8 dma_autoinit;
    UINT8 data[128], d_wptr, d_rptr;
    bool dma_timer_started;
    bool dma_throttled;
};

// ======================> sb8_device (parent)

class sb8_device :
		public device_t,
		public device_isa8_card_interface
{
public:
        // construction/destruction
        sb8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, const char *name);

        required_device<dac_device> m_dac;

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
        DECLARE_READ8_MEMBER(joy_port_r);
        DECLARE_WRITE8_MEMBER(joy_port_w);
		virtual ioport_constructor device_input_ports() const;

protected:
        // device-level overrides
        virtual void device_start();
        virtual void device_reset();
        virtual UINT8 dack_r(int line);
        virtual void dack_w(int line, UINT8 data);
        virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

        struct sb8_dsp_state m_dsp;
        UINT8 m_dack_out;
        attotime m_joy_time;

private:
        emu_timer *m_timer;
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

// device type definition
extern const device_type ISA8_SOUND_BLASTER_1_0;
extern const device_type ISA8_SOUND_BLASTER_1_5;

#endif  /* __ISA_SOUND_BLASTER_H__ */
