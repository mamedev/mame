// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Nippon Steel Corp NN71003F mpeg audio decoder

#ifndef DEVICES_SOUND_NN71003F_H
#define DEVICES_SOUND_NN71003F_H

#pragma once

#include "mpeg_audio.h"

class nn71003f_device : public device_t, public device_sound_interface
{
public:
	nn71003f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void frm_w(int state);
	void dat_w(int state);
	void clk_w(int state);

	void cmd_atn_w(int state);
	void cmd_clk_w(int state);
	void cmd_dat_w(int state);

	auto irq_cb() { return m_irq_cb.bind(); }
	
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	devcb_write_line m_irq_cb;
	u8 m_cmd_byte, m_cmd_cnt;
	int m_cmd_atn, m_cmd_clk, m_cmd_dat;
};

DECLARE_DEVICE_TYPE(NN71003F, nn71003f_device)

#endif
