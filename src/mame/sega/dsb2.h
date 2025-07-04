// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_SEGA_DSB2_H
#define MAME_SEGA_DSB2_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/i8251.h"
#include "sound/mpeg_audio.h"

#include <memory>


class dsb2_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	dsb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto rxd_handler() { return m_rxd_handler.bind(); }

	void write_txd(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	required_device<cpu_device> m_ourcpu;
	required_device<i8251_device> m_uart;
	required_region_ptr<uint8_t> m_mpeg_rom;

	devcb_write_line m_rxd_handler;

	std::unique_ptr<mpeg_audio> m_decoder;
	int16_t m_audio_buf[1152*2];
	uint32_t m_mp_start, m_mp_end, m_mp_vol, m_mp_pan, m_lp_start, m_lp_end, m_start, m_end;
	int32_t m_mp_pos, m_audio_pos, m_audio_avail;

	emu_timer *m_timer_1kHz;
	TIMER_CALLBACK_MEMBER(timer_irq_cb);

	void output_txd(int state);

	void dsb2_map(address_map &map) ATTR_COLD;

	enum mpeg_command_t : u8 {
		IDLE,
		START_ADDRESS_HI,
		START_ADDRESS_MD,
		START_ADDRESS_LO,
		END_ADDRESS_HI,
		END_ADDRESS_MD,
		END_ADDRESS_LO
	};

	enum mpeg_player_t : u8 {
		NOT_PLAYING,
		PLAYING
	};

	mpeg_command_t m_command;
	mpeg_player_t m_player;

	void system_control_w(offs_t offset, u8 data);
	void fifo_w(offs_t offset, u8 data);
};


// device type definition
DECLARE_DEVICE_TYPE(DSB2, dsb2_device)

#endif // MAME_SEGA_DSB2_H
