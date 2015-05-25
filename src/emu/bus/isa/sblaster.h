// license:BSD-3-Clause
// copyright-holders:R. Belmont, Miodrag Milanovic
#pragma once

#ifndef __ISA_SOUND_BLASTER_H__
#define __ISA_SOUND_BLASTER_H__

#include "bus/midi/midi.h"
#include "isa.h"
#include "sound/dac.h"
#include "bus/pc_joy/pc_joy.h"
#include "sound/3812intf.h"
#include "sound/saa1099.h"

#define SIXTEENBIT  0x01
#define STEREO      0x02
#define SIGNED      0x04
#define ADPCM2      0x08
#define ADPCM3      0x10
#define ADPCM4      0x20

#define IRQ_DMA8    0x01
#define IRQ_DMA16   0x02
#define IRQ_MPU     0x04
#define IRQ_ALL     0xff

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
	bool dma_no_irq;
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

struct sb8_mixer
{
	UINT8 status;
	UINT8 main_vol;
	UINT8 dac_vol;
	UINT8 fm_vol;
	UINT8 mic_vol;
	UINT8 in_filter;
	UINT8 stereo_sel;
	UINT8 cd_vol;
	UINT8 line_vol;
};

struct sb16_mixer
{
	UINT8 data;
	UINT8 status;
	UINT8 main_vol[2];
	UINT8 dac_vol[2];
	UINT8 fm_vol[2];
	UINT8 cd_vol[2];
	UINT8 line_vol[2];
	UINT8 mic_vol;
	UINT8 pc_speaker_vol;
	UINT8 output_ctl;
	UINT8 input_ctl[2];
	UINT8 input_gain[2];
	UINT8 output_gain[2];
	UINT8 agc;
	UINT8 treble[2];
	UINT8 bass[2];
};

// ======================> sb_device (parent)

class sb_device :
		public device_t, public device_serial_interface
{
public:
		// construction/destruction
		sb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, const char *name, const char *shortname, const char *source);

		required_device<dac_device> m_dacl;
		required_device<dac_device> m_dacr;
		required_device<pc_joy_device> m_joy;
		required_device<midi_port_device> m_mdout;

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

		DECLARE_WRITE_LINE_MEMBER( midi_rx_w ) { device_serial_interface::rx_w((UINT8)state); }

protected:
		// device-level overrides
		virtual void device_reset();
		UINT8 dack_r(int line);
		void dack_w(int line, UINT8 data);
		virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
		virtual void drq16_w(int state) { }
		virtual void drq_w(int state) { }
		virtual void irq_w(int state, int source) { }
		virtual void mixer_reset() {}
		void adpcm_decode(UINT8 sample, int size);

		// serial overrides
		virtual void rcv_complete();    // Rx completed receiving byte
		virtual void tra_complete();    // Tx completed sending byte
		virtual void tra_callback();    // Tx send bit

		static const int MIDI_RING_SIZE = 2048;

		struct sb8_dsp_state m_dsp;
		UINT8 m_dack_out;
		void xmit_char(UINT8 data);
		bool m_onebyte_midi, m_uart_midi, m_uart_irq, m_mpu_midi;
		int m_rx_waiting, m_tx_waiting;
		UINT8 m_recvring[MIDI_RING_SIZE];
		UINT8 m_xmitring[MIDI_RING_SIZE];
		int m_xmit_read, m_xmit_write;
		int m_recv_read, m_recv_write;
		bool m_tx_busy;

		emu_timer *m_timer;
};

class sb8_device : public sb_device,
					public device_isa8_card_interface
{
public:
		// construction/destruction
		sb8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, const char *name, const char *shortname, const char *source);

		DECLARE_READ8_MEMBER(ym3812_16_r);
		DECLARE_WRITE8_MEMBER(ym3812_16_w);
protected:
		virtual void device_start();
		virtual void drq_w(int state) { m_isa->drq1_w(state); }
		virtual void irq_w(int state, int source) { m_isa->irq5_w(state); }
		virtual UINT8 dack_r(int line) { return sb_device::dack_r(line); }
		virtual void dack_w(int line, UINT8 data) { sb_device::dack_w(line, data); }
private:
		required_device<ym3812_device> m_ym3812;
};

class isa8_sblaster1_0_device : public sb8_device
{
public:
		// construction/destruction
		isa8_sblaster1_0_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;

		DECLARE_READ8_MEMBER(saa1099_16_r);
		DECLARE_WRITE8_MEMBER(saa1099_1_16_w);
		DECLARE_WRITE8_MEMBER(saa1099_2_16_w);
protected:
		// device-level overrides
		virtual void device_start();
private:
		// internal state
		required_device<saa1099_device> m_saa1099_1;
		required_device<saa1099_device> m_saa1099_2;

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
private:
		// internal state
};

class sb16_device : public sb_device,
					public device_isa16_card_interface
{
public:
		// construction/destruction
		sb16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, const char *name, const char *shortname, const char *source);
		DECLARE_READ8_MEMBER(mpu401_r);
		DECLARE_WRITE8_MEMBER(mpu401_w);
		DECLARE_READ8_MEMBER(mixer_r);
		DECLARE_WRITE8_MEMBER(mixer_w);
protected:
		virtual void device_start();
		virtual UINT16 dack16_r(int line);
		virtual UINT8 dack_r(int line) { return sb_device::dack_r(line); }
		virtual void dack_w(int line, UINT8 data) { sb_device::dack_w(line, data); }
		virtual void dack16_w(int line, UINT16 data);
		virtual void drq16_w(int state) { m_isa->drq5_w(state); }
		virtual void drq_w(int state) { m_isa->drq1_w(state); }
		virtual void irq_w(int state, int source) { (state?m_dsp.irq_active|=source:m_dsp.irq_active&=~source); m_isa->irq5_w(m_dsp.irq_active != 0);  }
		virtual void mixer_reset();
		void mixer_set();
		virtual void rcv_complete();    // Rx completed receiving byte
private:
		struct sb16_mixer m_mixer;
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
private:
		// internal state
};

// device type definition
extern const device_type ISA8_SOUND_BLASTER_1_0;
extern const device_type ISA8_SOUND_BLASTER_1_5;
extern const device_type ISA16_SOUND_BLASTER_16;

#endif  /* __ISA_SOUND_BLASTER_H__ */
