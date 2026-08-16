// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_SOUND_TMS320AV110_H
#define MAME_SOUND_TMS320AV110_H

#pragma once

class tms320av110_device : public device_t, public device_sound_interface
{
public:
	tms320av110_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~tms320av110_device();

	auto req() { return m_req_cb.bind(); } // active-low compressed-data request output

	void reset_w(int state); // active-low RESET input
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	struct decoder_state;

	void decoder_create();
	void decoder_reset();
	bool decode_frame();
	void fifo_w(u8 data);
	void update_req();

	sound_stream *m_stream;
	devcb_write_line m_req_cb;
	std::unique_ptr<decoder_state> m_decoder;
	bool m_reset_asserted;
};

DECLARE_DEVICE_TYPE(TMS320AV110, tms320av110_device)

#endif // MAME_SOUND_TMS320AV110_H
