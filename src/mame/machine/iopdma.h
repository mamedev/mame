// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 IOP DMA device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_IOPDMA_H
#define MAME_MACHINE_IOPDMA_H

#pragma once


class iop_dma_device : public device_t
{
public:
    iop_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER(ctrl0_r);
	DECLARE_WRITE32_MEMBER(ctrl0_w);
	DECLARE_READ32_MEMBER(ctrl1_r);
	DECLARE_WRITE32_MEMBER(ctrl1_w);

protected:
	struct intctrl_t
	{
		uint8_t m_mask;
		uint8_t m_status;
		bool m_enabled;
	};

	struct channel_t
	{
		uint8_t m_priority;
		bool m_enabled;
	};

    virtual void device_start() override;
    virtual void device_reset() override;

	void set_dpcr(uint32_t data, uint32_t index);
	void set_dicr(uint32_t data, uint32_t index);
	void update_interrupts();

	uint32_t m_dpcr[2];
	uint32_t m_dicr[2];
	channel_t m_channels[16];
	intctrl_t m_int_ctrl[2];
};

DECLARE_DEVICE_TYPE(SONYIOP_DMA, iop_dma_device)

#endif // MAME_MACHINE_IOPDMA_H
