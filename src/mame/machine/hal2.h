// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HAL2 Audio Controller emulation

**********************************************************************/

#ifndef MAME_MACHINE_HAL2_H
#define MAME_MACHINE_HAL2_H

#pragma once

class hal2_device : public device_t
{
public:
	hal2_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: hal2_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	hal2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE32_MEMBER(write);
	DECLARE_READ32_MEMBER(read);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	enum
	{
		IAR_TYPE       = 0xf000,
		IAR_NUM        = 0x0f00,
		IAR_ACCESS_SEL = 0x0080,
		IAR_PARAM      = 0x000c,
		IAR_RB_INDEX   = 0x0003,
	};

	enum
	{
		ISR_TSTATUS      = 0x01,
		ISR_USTATUS      = 0x02,
		ISR_QUAD_MODE    = 0x04,
		ISR_GLOBAL_RESET = 0x08,
		ISR_CODEC_RESET  = 0x10,
	};

	enum
	{
		STATUS_REG           = 0x0010/4,
		REVISION_REG         = 0x0020/4,
		INDIRECT_ADDRESS_REG = 0x0030/4,
		INDIRECT_DATA0_REG   = 0x0040/4,
		INDIRECT_DATA1_REG   = 0x0050/4,
		INDIRECT_DATA2_REG   = 0x0060/4,
		INDIRECT_DATA3_REG   = 0x0070/4,
	};

	uint32_t m_isr;
	uint32_t m_iar;
	uint32_t m_idr[4];
};

DECLARE_DEVICE_TYPE(SGI_HAL2, hal2_device)

#endif // MAME_MACHINE_HAL2_H
