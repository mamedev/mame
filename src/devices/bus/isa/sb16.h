// license:BSD-3-Clause
// copyright-holders:Carl

#ifndef MAME_BUS_ISA_SB16_H
#define MAME_BUS_ISA_SB16_H

#include "isa.h"
#include "bus/pc_joy/pc_joy.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/dac.h"
#include "sound/ymopl.h"

//*********************************************************************
//   TYPE DEFINITIONS
//*********************************************************************

// ====================> sb16_device

class sb16_lle_device : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	sb16_lle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	uint8_t dack_r(int line) override;
	void dack_w(int line, uint8_t data) override;
	uint16_t dack16_r(int line) override;
	void dack16_w(int line, uint16_t data) override;

private:
	uint8_t mpu401_r(offs_t offset);
	void mpu401_w(offs_t offset, uint8_t data);

	// mcu ports
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

	// host ports
	uint8_t host_data_r();
	void host_cmd_w(uint8_t data);
	void dsp_reset_w(uint8_t data);
	uint8_t dsp_wbuf_status_r(offs_t offset);
	uint8_t dsp_rbuf_status_r(offs_t offset);

	void sb16_io(address_map &map) ATTR_COLD;
	void host_io(address_map &map) ATTR_COLD;

	void control_timer(bool start);

	TIMER_CALLBACK_MEMBER(timer_tick);

	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	required_device<pc_joy_device> m_joy;
	required_device<i80c52_device> m_cpu;

	// internal state
	bool m_data_in;
	uint8_t m_in_byte;
	bool m_data_out;
	uint8_t m_out_byte;

	uint8_t m_freq, m_mode, m_dac_fifo_ctrl, m_adc_fifo_ctrl, m_ctrl8, m_ctrl16, m_mpu_byte;
	uint16_t m_dma8_len, m_dma16_len, m_dma8_cnt, m_dma16_cnt;
	typedef union {
		uint32_t w;
		uint16_t h[2];
		uint8_t  b[4];
	} samples;
	samples m_adc_fifo[16], m_dac_fifo[16];
	int m_adc_fifo_head, m_adc_fifo_tail, m_dac_fifo_head, m_dac_fifo_tail;
	bool m_adc_r, m_dac_r, m_adc_h, m_dac_h, m_irq8, m_irq16, m_irq_midi;
	bool m_dma8_done, m_dma16_done;

	emu_timer *m_timer;
};

// device type definition

DECLARE_DEVICE_TYPE(ISA16_SB16, sb16_lle_device)

#endif // MAME_BUS_ISA_SB16_H
