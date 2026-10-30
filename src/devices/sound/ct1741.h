// license:BSD-3-Clause
// copyright-holders:Carl, Angelo Salese

#ifndef MAME_SOUND_CT1741_H
#define MAME_SOUND_CT1741_H

#pragma once

#include "cpu/mcs51/i80c52.h"

class ct1741_dsp_device : public device_t
{
public:
	ct1741_dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// host ports
	uint8_t host_data_r();
	void host_cmd_w(uint8_t data);
	void dsp_reset_w(uint8_t data);
	uint8_t dsp_wbuf_status_r(offs_t offset);
	uint8_t dsp_rbuf_status_r(offs_t offset);

	auto ldac_write_cb()  { return m_ldac_w.bind(); }
	auto rdac_write_cb()  { return m_rdac_w.bind(); }
	auto irq8_cb()        { return m_irq8_w.bind(); }
	auto irq16_cb()       { return m_irq16_w.bind(); }
	auto drq8_cb()        { return m_drq8_w.bind(); }
	auto drq16_cb()       { return m_drq16_w.bind(); }
	auto speaker_off_cb() { return m_speaker_off_w.bind(); }

	uint8_t dack_r();
	void dack_w(uint8_t data);
	uint16_t dack16_r();
	void dack16_w(uint16_t data);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void data_map(address_map &map);

private:
	required_device<i80c52_device> m_cpu;
	devcb_write16 m_ldac_w;
	devcb_write16 m_rdac_w;
	devcb_write_line m_irq8_w;
	devcb_write_line m_irq16_w;
	devcb_write_line m_drq8_w;
	devcb_write_line m_drq16_w;
	devcb_write_line m_speaker_off_w;

	uint8_t dsp_data_r();
	void dsp_data_w(uint8_t data);
	uint8_t p1_r();
	void p1_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);
	void rate_w(uint8_t data);
	uint8_t dma8_r();
	void dma8_w(uint8_t data);
	uint8_t ctrl8_r();
	void ctrl8_w(uint8_t data);
	uint8_t ctrl16_r();
	void ctrl16_w(uint8_t data);
	uint8_t dac_ctrl_r();
	void dac_ctrl_w(uint8_t data);
	uint8_t dac_fifo_ctrl_r();
	void dac_fifo_ctrl_w(uint8_t data);
	uint8_t adc_fifo_ctrl_r();
	void adc_fifo_ctrl_w(uint8_t data);
	uint8_t dma_stat_r();
	void dac_data_w(uint8_t data);
	uint8_t adc_data_r();
	uint8_t dma8_ready_r();
	uint8_t adc_data_ready_r();
	uint8_t dma8_cnt_lo_r();
	uint8_t dma8_cnt_hi_r();
	void dma8_len_lo_w(uint8_t data);
	void dma8_len_hi_w(uint8_t data);
	void dma16_len_lo_w(uint8_t data);
	void dma16_len_hi_w(uint8_t data);
	uint8_t mode_r();
	void mode_w(uint8_t data);

	void control_timer();
	TIMER_CALLBACK_MEMBER(timer_tick);

	// internal state
	bool m_data_in;
	uint8_t m_in_byte;
	bool m_data_out;
	uint8_t m_out_byte;

	uint8_t m_freq, m_mode, m_dac_fifo_ctrl, m_adc_fifo_ctrl, m_ctrl8, m_ctrl16;
	uint16_t m_dma8_len, m_dma16_len, m_dma8_cnt, m_dma16_cnt;
	typedef union {
		uint32_t w;
		uint16_t h[2];
		uint8_t  b[4];
	} samples;
	static constexpr int FIFO_SIZE = 16;
	samples m_adc_fifo[FIFO_SIZE], m_dac_fifo[FIFO_SIZE];
	int m_adc_fifo_head, m_adc_fifo_tail, m_dac_fifo_head, m_dac_fifo_tail;
	bool m_adc_r, m_dac_r, m_adc_h, m_dac_h;
	bool m_dma8_done, m_dma16_done;

	emu_timer *m_timer;

};

DECLARE_DEVICE_TYPE(CT1741, ct1741_dsp_device)

#endif // MAME_SOUND_CT1741_H
