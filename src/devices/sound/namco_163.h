// license:BSD-3-Clause
// copyright-holders:cam900
#ifndef MAME_SOUND_NAMCO_163_H
#define MAME_SOUND_NAMCO_163_H

#pragma once


class namco_163_sound_device : public device_t,
							public device_sound_interface
{
public:
	namco_163_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void disable_w(int state);

	void addr_w(u8 data);
	void data_w(u8 data);
	u8 data_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// global sound parameters
	u8                    m_ram[0x80];
	u8                    m_reg_addr;
	u8                    m_addr;
	bool                  m_inc;
	bool                  m_disable;
	sound_stream          *m_stream;

	// internals
	inline s8 get_sample(u16 addr);

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
};

DECLARE_DEVICE_TYPE(NAMCO_163, namco_163_sound_device)

#endif // MAME_SOUND_NAMCO_163_H
