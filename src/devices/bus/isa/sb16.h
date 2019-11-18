// license:BSD-3-Clause
// copyright-holders:Carl

#ifndef MAME_BUS_ISA_SB16_H
#define MAME_BUS_ISA_SB16_H

#include "isa.h"
#include "bus/pc_joy/pc_joy.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/262intf.h"
#include "sound/dac.h"

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
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	uint8_t dack_r(int line) override;
	void dack_w(int line, uint8_t data) override;
	uint16_t dack16_r(int line) override;
	void dack16_w(int line, uint16_t data) override;

private:
	READ8_MEMBER( mpu401_r );
	WRITE8_MEMBER( mpu401_w );

	// mcu ports
	DECLARE_READ8_MEMBER( dsp_data_r );
	DECLARE_WRITE8_MEMBER( dsp_data_w );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_WRITE8_MEMBER( rate_w );
	DECLARE_READ8_MEMBER( dma8_r );
	DECLARE_WRITE8_MEMBER( dma8_w );
	DECLARE_READ8_MEMBER( ctrl8_r );
	DECLARE_WRITE8_MEMBER( ctrl8_w );
	DECLARE_READ8_MEMBER( ctrl16_r );
	DECLARE_WRITE8_MEMBER( ctrl16_w );
	DECLARE_READ8_MEMBER( dac_ctrl_r );
	DECLARE_WRITE8_MEMBER( dac_ctrl_w );
	DECLARE_READ8_MEMBER( dac_fifo_ctrl_r );
	DECLARE_WRITE8_MEMBER( dac_fifo_ctrl_w );
	DECLARE_READ8_MEMBER( adc_fifo_ctrl_r );
	DECLARE_WRITE8_MEMBER( adc_fifo_ctrl_w );
	DECLARE_READ8_MEMBER( dma_stat_r );
	DECLARE_WRITE8_MEMBER( dac_data_w );
	DECLARE_READ8_MEMBER( adc_data_r );
	DECLARE_READ8_MEMBER( dma8_ready_r );
	DECLARE_READ8_MEMBER( adc_data_ready_r );
	DECLARE_READ8_MEMBER( dma8_cnt_lo_r );
	DECLARE_READ8_MEMBER( dma8_cnt_hi_r );
	DECLARE_WRITE8_MEMBER( dma8_len_lo_w );
	DECLARE_WRITE8_MEMBER( dma8_len_hi_w );
	DECLARE_WRITE8_MEMBER( dma16_len_lo_w );
	DECLARE_WRITE8_MEMBER( dma16_len_hi_w );
	DECLARE_READ8_MEMBER( mode_r );
	DECLARE_WRITE8_MEMBER( mode_w );

	// host ports
	DECLARE_READ8_MEMBER( host_data_r );
	DECLARE_WRITE8_MEMBER( host_cmd_w );
	DECLARE_WRITE8_MEMBER( dsp_reset_w );
	DECLARE_READ8_MEMBER( dsp_wbuf_status_r );
	DECLARE_READ8_MEMBER( dsp_rbuf_status_r );
	DECLARE_READ8_MEMBER( invalid_r );
	DECLARE_WRITE8_MEMBER( invalid_w );

	void sb16_io(address_map &map);

	void control_timer(bool start);

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
