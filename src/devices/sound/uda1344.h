// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

	Philips UDA1344 Stereo Audio Codec skeleton

****************************************************************************/

#ifndef MAME_SOUND_UDA1344_H
#define MAME_SOUND_UDA1344_H

#pragma once

class uda1344_device : public device_t, public device_sound_interface
{
public:
	uda1344_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void l3_addr_w(offs_t offset, uint8_t data);
	void l3_data_w(offs_t offset, uint8_t data);

	auto l3_ack_out() { return m_l3_ack_out.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	enum : uint8_t
	{
		CHIP_ADDR_MASK		= 0xfc,
		CHIP_ADDR			= 0x14,

		REG_TYPE_MASK		= 0xc0,
		REG_TYPE_BIT		= 6,

		VOLUME_REG			= 0x00,
		VOLUME_REG_MASK		= 0x3f,

		EQUALIZER_REG		= 0x01,
		EQUALIZER_REG_MASK	= 0x3f,
		EQUALIZER_BB_MASK	= 0x3c,
		EQUALIZER_BB_BIT	= 2,
		EQUALIZER_TR_MASK	= 0x03,
		EQUALIZER_TR_BIT	= 0,

		FILTER_REG			= 0x02,
		FILTER_REG_MASK		= 0x1f,
		FILTER_DE_MASK		= 0x18,
		FILTER_DE_BIT		= 3,
		FILTER_MT_BIT		= 2,
		FILTER_MODE_MASK	= 0x03,
		FILTER_MODE_BIT		= 0,

		POWER_REG			= 0x03,
		POWER_REG_MASK		= 0x03,
		POWER_ADC_BIT		= 1,
		POWER_DAC_BIT		= 0,

		STATUS_REG_MASK		= 0x3f,
		STATUS_SC_MASK		= 0x30,
		STATUS_SC_BIT		= 4,
		STATUS_IF_MASK		= 0xe0,
		STATUS_IF_BIT		= 1,
		STATUS_DC_BIT		= 0
	};

	sound_stream *m_stream;

	uint8_t m_data_transfer_mode;
	uint8_t m_status_reg;
	uint8_t m_volume_reg;
	uint8_t m_equalizer_reg;
	uint8_t m_filter_reg;
	uint8_t m_power_reg;

	devcb_write_line m_l3_ack_out;
};

DECLARE_DEVICE_TYPE(UDA1344, uda1344_device)

#endif // MAME_SOUND_UDA1344_H
