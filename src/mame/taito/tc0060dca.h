// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_TAITO_TC0060DCA_H
#define MAME_TAITO_TC0060DCA_H

#pragma once

class tc0060dca_device : public device_t, public device_sound_interface
{
public:
	tc0060dca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void volume1_w(u8 data);
	void volume2_w(u8 data);

protected:
	// device_t override
	virtual void device_start() override ATTR_COLD;

	// device_sound_interface override
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	sound_stream *m_stream;
	float m_gain[2];
	float m_atten_table[256];
};

DECLARE_DEVICE_TYPE(TC0060DCA, tc0060dca_device)

#endif // MAME_TAITO_TC0060DCA_H
