// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_KONAMI_K573FPGA_H
#define MAME_KONAMI_K573FPGA_H

#pragma once

#include "sound/mas3507d.h"
#include "machine/ds2401.h"
#include "machine/timer.h"

DECLARE_DEVICE_TYPE(KONAMI_573_DIGITAL_FPGA, k573fpga_device)

class k573fpga_device : public device_t
{
public:
	k573fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename... T> void add_route(T &&... args) { mas3507d.lookup()->add_route(std::forward<T>(args)...); }
	template <typename T> void set_ram(T &&tag) { ram.set_tag(std::forward<T>(tag)); }

	void set_ddrsbm_fpga(bool flag) { is_ddrsbm_fpga = flag; }

	void mpeg_frame_sync(int state);
	void mas3507d_demand(int state);

	void set_crypto_key1(uint16_t v);
	void set_crypto_key2(uint16_t v);
	void set_crypto_key3(uint8_t v);

	uint32_t get_mp3_start_addr() { return mp3_start_addr; }
	void set_mp3_start_addr(uint32_t v);

	uint32_t get_mp3_end_addr() { return mp3_end_addr; }
	void set_mp3_end_addr(uint32_t v);

	uint16_t mas_i2c_r();
	void mas_i2c_w(uint16_t data);

	uint16_t get_fpga_ctrl();
	void set_fpga_ctrl(uint16_t data);

	uint16_t get_mpeg_ctrl();

	uint32_t get_counter();
	uint32_t get_counter_diff();
	uint16_t get_mp3_frame_count();

	void reset_counter();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(update_stream);
	void update_counter();
	void update_mp3_decode_state();

	uint16_t decrypt_common(uint16_t data, uint16_t key);
	uint16_t decrypt_default(uint16_t data);
	uint16_t decrypt_ddrsbm(uint16_t data);

	emu_timer* m_stream_timer;

	enum {
		PLAYBACK_STATE_DEMAND = 12,
		PLAYBACK_STATE_IDLE = 13,
		PLAYBACK_STATE_PLAYING = 14,
		PLAYBACK_STATE_ENABLED = 15,
	};

	enum {
		// Allows MP3 data to be decrypted?
		// If this is 0 then data won't be sent to the MAS3507D even if FPGA_STREAMING_ENABLE is 1.
		FPGA_MP3_ENABLE = 13,

		// Allows data to be streamed to MAS3507D.
		// This needs to be set before the register at 0x1f6400ae will return the streaming status.
		FPGA_STREAMING_ENABLE = 14,

		// Allows frame counter to be incremented based on the MPEG frame sync pin from the MAS3507D.
		// Setting this to 0 resets the frame counter register.
		FPGA_FRAME_COUNTER_ENABLE = 15,
	};

	required_shared_ptr<uint16_t> ram;
	required_device<mas3507d_device> mas3507d;

	bool is_ddrsbm_fpga;

	uint16_t mpeg_status, fpga_status;

	uint16_t crypto_key1, crypto_key2;
	uint8_t crypto_key3;
	uint16_t crypto_key1_start, crypto_key2_start;
	uint8_t crypto_key3_start;

	uint32_t mp3_start_addr, mp3_end_addr;
	uint32_t mp3_cur_start_addr, mp3_cur_end_addr, mp3_cur_addr;
	uint16_t mp3_data;
	int mp3_remaining_bytes;

	bool is_mpeg_frame_synced;
	uint32_t mp3_frame_counter;

	attotime counter_current, counter_base;
	double counter_value;
};

#endif // MAME_KONAMI_K573FPGA_H
