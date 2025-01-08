// license:BSD-3-Clause
// copyright-holders:R. Belmont, Miodrag Milanovic
#ifndef MAME_BUS_ISA_SBLASTER_H
#define MAME_BUS_ISA_SBLASTER_H

#pragma once

#include "isa.h"
#include "bus/midi/midi.h"
#include "bus/pc_joy/pc_joy.h"
#include "sound/dac.h"
#include "sound/saa1099.h"
#include "sound/ymopl.h"
#include "diserial.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sb_device (parent)

class sb_device :
		public device_t, public device_serial_interface
{
public:
	void process_fifo(uint8_t cmd);
	void queue(uint8_t data);
	void queue_r(uint8_t data);
	uint8_t dequeue_r();

	uint8_t dsp_reset_r(offs_t offset);
	void dsp_reset_w(offs_t offset, uint8_t data);
	uint8_t dsp_data_r(offs_t offset);
	void dsp_data_w(offs_t offset, uint8_t data);
	uint8_t dsp_rbuf_status_r(offs_t offset);
	uint8_t dsp_wbuf_status_r(offs_t offset);
	void dsp_rbuf_status_w(offs_t offset, uint8_t data);
	void dsp_cmd_w(offs_t offset, uint8_t data);

	void midi_rx_w(int state) { device_serial_interface::rx_w((uint8_t)state); }

protected:
	void common(machine_config &config);

	struct sb8_dsp_state
	{
		uint8_t reset_latch;
		uint8_t rbuf_status;
		uint8_t wbuf_status;
		uint8_t fifo[16],fifo_ptr;
		uint8_t fifo_r[52],fifo_r_ptr;
		uint16_t version;
		uint8_t test_reg;
		uint8_t speaker_on;
		bool dma_no_irq;
		uint32_t prot_count;
		int32_t prot_value;
		uint32_t frequency;
		uint32_t adc_freq;
		uint32_t dma_length, dma_transferred;
		uint32_t adc_length, adc_transferred;
		uint8_t dma_autoinit;
		uint8_t data[128], d_wptr, d_rptr;
		bool dma_timer_started;
		bool dma_throttled;
		uint8_t flags;
		uint8_t irq_active;
		bool adpcm_new_ref;
		uint8_t adpcm_ref;
		uint8_t adpcm_step;
		uint8_t adpcm_count;
	};

	struct sb8_mixer
	{
		uint8_t status;
		uint8_t main_vol;
		uint8_t dac_vol;
		uint8_t fm_vol;
		uint8_t mic_vol;
		uint8_t in_filter;
		uint8_t stereo_sel;
		uint8_t cd_vol;
		uint8_t line_vol;
	};

	struct sb16_mixer
	{
		uint8_t data;
		uint8_t status;
		uint8_t main_vol[2];
		uint8_t dac_vol[2];
		uint8_t fm_vol[2];
		uint8_t cd_vol[2];
		uint8_t line_vol[2];
		uint8_t mic_vol;
		uint8_t pc_speaker_vol;
		uint8_t output_ctl;
		uint8_t input_ctl[2];
		uint8_t input_gain[2];
		uint8_t output_gain[2];
		uint8_t agc;
		uint8_t treble[2];
		uint8_t bass[2];
	};

	// construction/destruction
	sb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t dack_r(int line);
	void dack_w(int line, uint8_t data);
	virtual void drq16_w(int state) { }
	virtual void drq_w(int state) { }
	virtual void irq_w(int state, int source) { }
	virtual void mixer_reset() { }
	void adpcm_decode(uint8_t sample, int size);

	// serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	static constexpr unsigned MIDI_RING_SIZE = 2048;

	TIMER_CALLBACK_MEMBER(timer_tick);

	required_device<dac_16bit_r2r_device> m_ldac;
	required_device<dac_16bit_r2r_device> m_rdac;
	required_device<pc_joy_device> m_joy;
	required_device<midi_port_device> m_mdout;
	required_ioport m_config;

	struct sb8_dsp_state m_dsp;
	uint8_t m_dack_out;
	void xmit_char(uint8_t data);
	bool m_onebyte_midi, m_uart_midi, m_uart_irq, m_mpu_midi;
	int m_rx_waiting, m_tx_waiting;
	uint8_t m_recvring[MIDI_RING_SIZE];
	uint8_t m_xmitring[MIDI_RING_SIZE];
	int m_xmit_read, m_xmit_write;
	int m_recv_read, m_recv_write;
	bool m_tx_busy;

	emu_timer *m_timer;
};

class sb8_device : public sb_device,
					public device_isa8_card_interface
{
public:
	uint8_t ym3812_16_r(offs_t offset);
	void ym3812_16_w(offs_t offset, uint8_t data);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
protected:
	// construction/destruction
	sb8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void drq_w(int state) override;
	virtual void irq_w(int state, int source) override;
	virtual uint8_t dack_r(int line) override { return sb_device::dack_r(line); }
	virtual void dack_w(int line, uint8_t data) override { sb_device::dack_w(line, data); }

	required_device<ym3812_device> m_ym3812;
};

class isa8_sblaster1_0_device : public sb8_device
{
public:
	// construction/destruction
	isa8_sblaster1_0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t saa1099_16_r(offs_t offset);
	void saa1099_1_16_w(offs_t offset, uint8_t data);
	void saa1099_2_16_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	required_device<saa1099_device> m_saa1099_1;
	required_device<saa1099_device> m_saa1099_2;
};

class isa8_sblaster1_5_device : public sb8_device
{
public:
	// construction/destruction
	isa8_sblaster1_5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class sb16_device : public sb_device,
					public device_isa16_card_interface
{
public:
	uint8_t mpu401_r(offs_t offset);
	void mpu401_w(offs_t offset, uint8_t data);
	uint8_t mixer_r(offs_t offset);
	void mixer_w(offs_t offset, uint8_t data);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
protected:
	// construction/destruction
	sb16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
	virtual uint16_t dack16_r(int line) override;
	virtual uint8_t dack_r(int line) override { return sb_device::dack_r(line); }
	virtual void dack_w(int line, uint8_t data) override { sb_device::dack_w(line, data); }
	virtual void dack16_w(int line, uint16_t data) override;
	virtual void drq16_w(int state) override;
	virtual void drq_w(int state) override;
	virtual void irq_w(int state, int source) override;
	virtual void mixer_reset() override;
	void mixer_set();
	virtual void rcv_complete() override;    // Rx completed receiving byte
private:
	struct sb16_mixer m_mixer;
};

class isa16_sblaster16_device : public sb16_device
{
public:
	// construction/destruction
	isa16_sblaster16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_SOUND_BLASTER_1_0, isa8_sblaster1_0_device)
DECLARE_DEVICE_TYPE(ISA8_SOUND_BLASTER_1_5, isa8_sblaster1_5_device)
DECLARE_DEVICE_TYPE(ISA16_SOUND_BLASTER_16, isa16_sblaster16_device)

#endif // MAME_BUS_ISA_SBLASTER_H
