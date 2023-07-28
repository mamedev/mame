// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/*************************************************************

  IBM 72X7377
  PS/2 Type 1 DMA Controller

**************************************************************/

#ifndef MAME_MACHINE_IBM72X7377_H
#define MAME_MACHINE_IBM72X7377_H

#pragma once

#include "machine/am9517a.h"
#include "bus/mca/mca.h"

class ibm72x7377_device : public device_t
{
public:
	ibm72x7377_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t dmac1_r(offs_t offset);
	void dmac1_w(offs_t offset, uint8_t data);

	uint8_t dmac2_r(offs_t offset);
	void dmac2_w(offs_t offset, uint8_t data);

    uint8_t page8_r(offs_t offset);
	void page8_w(offs_t offset, uint8_t data);

	uint8_t extended_function_register_r(offs_t offset);
	void extended_function_register_w(offs_t offset, uint8_t data);

	uint8_t extended_function_execute_r(offs_t offset);
	void extended_function_execute_w(offs_t offset, uint8_t data);

	uint8_t dma_arbiter_r(offs_t offset);
	void dma_arbiter_w(offs_t offset, uint8_t data);
	void dma_feedback_w(offs_t offset, uint8_t data);

	template <unsigned C> void dreq_w(int state) { dma_request(C, state); }

protected:
	void device_start() override;
	void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
    virtual void device_config_complete() override;

private:
    required_device<cpu_device> m_maincpu;
    required_device<mca16_device> m_mcabus;
	required_device<ps2_dma_device> m_dma8237_1;
	required_device<ps2_dma_device> m_dma8237_2;

	void dma_request(int channel, bool state);

	void dma8237_1_out_eop(int state);
	void dma8237_2_out_eop(int state);
	uint8_t dma8237_0_dack_r();
	uint8_t dma8237_1_dack_r();
	uint8_t dma8237_2_dack_r();
	uint8_t dma8237_3_dack_r();
	uint8_t dma8237_5_dack_r();
	uint8_t dma8237_6_dack_r();
	uint8_t dma8237_7_dack_r();
	void dma8237_0_dack_w(uint8_t data);
	void dma8237_1_dack_w(uint8_t data);
	void dma8237_2_dack_w(uint8_t data);
	void dma8237_3_dack_w(uint8_t data);
	void dma8237_5_dack_w(uint8_t data);
	void dma8237_6_dack_w(uint8_t data);
	void dma8237_7_dack_w(uint8_t data);
	void dack0_w(int state);
	void dack1_w(int state);
	void dack2_w(int state);
	void dack3_w(int state);
	void dack4_w(int state);
	void dack5_w(int state);
	void dack6_w(int state);
	void dack7_w(int state);

	void dma_hrq_changed(int state);
	void set_dma_channel(int channel, int state);

    uint8_t dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, uint8_t data);
	uint8_t dma_read_word(offs_t offset);
	void dma_write_word(offs_t offset, uint8_t data);

	u32 get_address(uint8_t channel);
	u32 get_count(uint8_t channel);

	void set_address(uint8_t channel, u8 address, uint8_t byte_pointer);
	void set_count(uint8_t channel, u8 count, uint8_t byte_pointer);

	// DMA ARBUS registers. TODO: Combine into an array.
	uint8_t m_arbus_ch0;
	uint8_t m_arbus_ch4;

	uint8_t m_at_pages[0x10]{};
    uint16_t m_dma_high_byte = 0;
    int m_dma_channel = 0;
    uint8_t m_dma_offset[2][4]{};
    bool m_cur_eop = false, m_cur_eop2 = false;

    uint8_t m_byte_pointer;
	uint8_t m_extended_function;

};

DECLARE_DEVICE_TYPE(IBM72X7377, ibm72x7377_device)

#endif