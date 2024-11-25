// license:BSD-3-Clause
// copyright-holders:Paul Arnold
/***************************************************************************

    scc66470.h

***************************************************************************/

#ifndef MAME_VIDEO_SCC66470_H
#define MAME_VIDEO_SCC66470_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> scc66470_device

class scc66470_device : public device_t, public device_memory_interface, public device_video_interface
{
public:
	auto irq()
	{
		return m_irqcallback.bind();
	}

	// construction/destruction
	scc66470_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	uint16_t ipa_r(offs_t offset, uint16_t mem_mask = ~0);
	void ipa_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	bool display_enabled();
	void line(int line, uint8_t *line_buffer, unsigned line_buffer_size);
	unsigned width();
	unsigned height();
	unsigned total_height();
	int dram_dtack_cycles();
	void dram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dram_r(offs_t offset, uint16_t mem_mask = ~0);
	void map(address_map &map) ATTR_COLD;
	void set_vectors(uint16_t *src);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	devcb_write_line m_irqcallback;

	uint32_t m_working_dcp;

	uint16_t m_csr;
	uint16_t m_dcr;
	uint16_t m_vsr;
	uint8_t m_bcr;
	uint16_t m_dcr2;
	uint16_t m_dcp;
	uint16_t m_swm;
	uint8_t m_stm;
	uint16_t m_reg_a;
	uint16_t m_reg_b;
	uint16_t m_pcr;
	uint8_t m_mask;
	uint8_t m_shift;
	uint8_t m_index;
	uint8_t m_fc;
	uint8_t m_bc;
	uint8_t m_tc;
	uint8_t m_csr_r;

private:
	void scc66470_vram(address_map &map) ATTR_COLD;
	void set_vsr(uint32_t vsr);
	void set_dcp(uint32_t dcp);
	uint32_t get_vsr();
	uint32_t get_dcp();
	unsigned border_width();
	int border_height();
	void csr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dcr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vsr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bcr_w(offs_t offset, uint8_t data);
	void dcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dcp_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void swm_w(offs_t offset, uint8_t data);
	void stm_w(offs_t offset, uint8_t data);
	void reg_a_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void reg_b_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pcr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void mask_w(offs_t offset, uint8_t data);
	void shift_w(offs_t offset, uint8_t data);
	void index_w(offs_t offset, uint8_t data);
	void fc_w(offs_t offset, uint8_t data);
	void bc_w(offs_t offset, uint8_t data);
	void tc_w(offs_t offset, uint8_t data);
	uint8_t csr_r(offs_t offset);
	uint16_t reg_b_r(offs_t offset, uint16_t mem_mask = ~0);
	int pixac_trigger();
	void perform_pixac_op();
	inline uint8_t dram_byte_r(offs_t offset);

	TIMER_CALLBACK_MEMBER(process_ica);
	TIMER_CALLBACK_MEMBER(process_dca);
	emu_timer *m_ica_timer;
	emu_timer *m_dca_timer;

	const address_space_config m_space_config;
};

// device type definition
DECLARE_DEVICE_TYPE(SCC66470, scc66470_device)

#endif // MAME_VIDEO_SCC66470_H
