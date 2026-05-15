// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_GENERALPLUS_GPL_DMA_H
#define MAME_MACHINE_GENERALPLUS_GPL_DMA_H

#pragma once

class gpl_dma_device : public device_t
{
public:
	gpl_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto space_read_callback() { return m_space_read_cb.bind(); }
	auto space_write_callback() { return m_space_write_cb.bind(); }
	auto dma_complete_callback() { return m_dma_complete_cb.bind(); }

	u16 system_dma_status_r();
	void system_dma_status_w(u16 data);
	u16 system_dma_memtype_r();
	void system_dma_memtype_w(u16 data);

	void write_dma_params(int channel, int offset, u16 data);
	u16 read_dma_params(int channel, int offset);

	u16 system_dma_params_channel0_r(offs_t offset);
	void system_dma_params_channel0_w(offs_t offset, u16 data);
	u16 system_dma_params_channel1_r(offs_t offset);
	void system_dma_params_channel1_w(offs_t offset, u16 data);
	u16 system_dma_params_channel2_r(offs_t offset);
	void system_dma_params_channel2_w(offs_t offset, u16 data);
	u16 system_dma_params_channel3_r(offs_t offset);
	void system_dma_params_channel3_w(offs_t offset, u16 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void trigger_systemm_dma(int channel);

private:

	u16 m_system_dma_memtype;
	u16 m_dma_params[8][4];
	u8 m_dma_latched[4];
	u16 m_dma_status;

	devcb_read16 m_space_read_cb;
	devcb_write16 m_space_write_cb;
	devcb_write_line m_dma_complete_cb;
};

DECLARE_DEVICE_TYPE(GPL_DMA, gpl_dma_device)

#endif // MAME_MACHINE_GENERALPLUS_GPL_DMA_H
