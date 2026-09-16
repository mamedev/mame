// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HAL2 Audio Controller emulation

**********************************************************************/

#ifndef MAME_SGI_HAL2_H
#define MAME_SGI_HAL2_H

#pragma once

#include "sound/dac.h"
#include "speaker.h"

class hal2_device : public device_t
{
public:
	hal2_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: hal2_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	hal2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint16_t data);
	uint16_t read(offs_t offset);

	attotime get_rate(const uint32_t channel);

	void set_right_volume(uint8_t vol) { m_rdac->set_output_gain(ALL_OUTPUTS, vol / 255.0f); }
	void set_left_volume(uint8_t vol) { m_ldac->set_output_gain(ALL_OUTPUTS, vol / 255.0f); }

	void dma_write(uint32_t channel, int16_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void update_clock_freq(int clock_gen);

	enum
	{
		IAR_TYPE            = 0xf000,
		IAR_TYPE_SHIFT      = 12,
		IAR_NUM             = 0x0f00,
		IAR_NUM_SHIFT       = 8,
		IAR_ACCESS_SEL      = 0x0080,
		IAR_PARAM           = 0x000c,
		IAR_PARAM_SHIFT     = 2,
		IAR_RB_INDEX        = 0x0003
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

	enum
	{
		DAC_L,
		DAC_R
	};

	required_device<dac_16bit_r2r_twos_complement_device> m_ldac;
	required_device<dac_16bit_r2r_twos_complement_device> m_rdac;

	uint16_t m_isr = 0;
	uint16_t m_iar = 0;
	uint16_t m_idr[4]{};

	uint16_t m_codeca_ctrl[2]{};
	uint16_t m_codeca_channel = 0;
	uint16_t m_codeca_clock = 0;
	uint16_t m_codeca_channel_count = 0;

	uint16_t m_codecb_ctrl[2]{};
	uint16_t m_codecb_channel = 0;
	uint16_t m_codecb_clock = 0;
	uint16_t m_codecb_channel_count = 0;

	uint16_t m_bres_clock_sel[3]{};
	uint16_t m_bres_clock_inc[3]{};
	uint16_t m_bres_clock_modctrl[3]{};
	uint16_t m_bres_clock_freq[3]{};
	attotime m_bres_clock_rate[3]{};

	uint16_t m_curr_dac = 0;

	static const uint32_t s_channel_pair[4];
};

DECLARE_DEVICE_TYPE(SGI_HAL2, hal2_device)

#endif // MAME_SGI_HAL2_H
