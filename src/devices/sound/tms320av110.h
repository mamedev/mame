// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_SOUND_TMS320AV110_H
#define MAME_SOUND_TMS320AV110_H

#pragma once

class tms320av110_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	tms320av110_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~tms320av110_device();

	auto req() { return m_req_cb.bind(); } // active-low compressed-data request output
	void set_external_dram(bool external) { m_external_dram = external; }

	void reset_w(int state); // active-low RESET input
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	struct decoder_state;

	void decoder_create();
	void decoder_reset();
	bool decode_frame();
	void fifo_w(u8 data);
	void input_fifo_reset();
	void start_input_timer();
	void start_reset(bool pin_reset);
	void start_restart();
	void set_req(int state);
	void update_req();
	TIMER_CALLBACK_MEMBER(input_tick);
	TIMER_CALLBACK_MEMBER(reset_complete);

	sound_stream *m_stream;
	devcb_write_line m_req_cb;
	std::unique_ptr<decoder_state> m_decoder;
	bool m_external_dram;
	emu_timer *m_input_timer;
	emu_timer *m_reset_timer;
	bool m_reset_asserted;
	bool m_reset_cycle;
	bool m_data_access;
	int m_req_state;
};

DECLARE_DEVICE_TYPE(TMS320AV110, tms320av110_device)

#endif // MAME_SOUND_TMS320AV110_H
