// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
#ifndef MAME_SEGA_DSBZ80_H
#define MAME_SEGA_DSBZ80_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "sound/mpeg_audio.h"

#include <memory>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dsbz80_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	dsbz80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	required_device<cpu_device> m_ourcpu;
	required_device<i8251_device> m_uart;
	required_region_ptr<uint8_t> m_mpeg_rom;

	devcb_write_line m_rxd_handler;

	std::unique_ptr<mpeg_audio> m_decoder;
	int16_t m_audio_buf[1152*2];
	uint32_t m_mp_start, m_mp_end, m_mp_vol, m_mp_pan, m_mp_state, m_lp_start, m_lp_end, m_start, m_end;
	int32_t m_mp_pos, m_audio_pos, m_audio_avail;

	void output_txd(int state);

	void mpeg_trigger_w(uint8_t data);
	void mpeg_start_w(offs_t offset, uint8_t data);
	void mpeg_end_w(offs_t offset, uint8_t data);
	void mpeg_volume_w(uint8_t data);
	void mpeg_stereo_w(uint8_t data);
	uint8_t mpeg_pos_r(offs_t offset);

	void dsbz80_map(address_map &map) ATTR_COLD;
	void dsbz80io_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(DSBZ80, dsbz80_device)

#endif // MAME_SEGA_DSBZ80_H
