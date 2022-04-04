// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_MACHINE_K573FPGA_H
#define MAME_MACHINE_K573FPGA_H

#pragma once

#include "sound/mas3507d.h"
#include "machine/ds2401.h"

DECLARE_DEVICE_TYPE(KONAMI_573_DIGITAL_FPGA, k573fpga_device)

class k573fpga_device : public device_t
{
public:
	k573fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename... T> void add_route(T &&... args) { subdevice<mas3507d_device>("mpeg")->add_route(std::forward<T>(args)...); }
	template <typename T> void set_ram(T &&tag) { ram.set_tag(std::forward<T>(tag)); }

	void set_ddrsbm_fpga(bool flag) { use_ddrsbm_fpga = flag; }

	uint16_t get_decrypted();

	void set_crypto_key1(uint16_t v) { crypto_key1 = v; }
	void set_crypto_key2(uint16_t v) { crypto_key2 = v; }
	void set_crypto_key3(uint8_t v) { crypto_key3 = v; }

	uint32_t get_mp3_start_addr() { return mp3_start_addr; }
	void set_mp3_start_addr(uint32_t v) { mp3_start_addr = v; }

	uint32_t get_mp3_end_addr() { return mp3_end_addr; }
	void set_mp3_end_addr(uint32_t v) { mp3_end_addr = v; }

	uint16_t mas_i2c_r();
	void mas_i2c_w(uint16_t data);

	uint16_t get_fpga_ctrl();
	void set_mpeg_ctrl(uint16_t data);

	uint16_t get_mpeg_ctrl();

	uint32_t get_counter();
	uint32_t get_counter_diff();

	void status_update();
	void reset_counter();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	uint16_t decrypt_default(uint16_t data);
	uint16_t decrypt_ddrsbm(uint16_t data);

	bool is_mp3_playing();
	bool is_streaming();

	enum {
		PLAYBACK_STATE_UNKNOWN = 0x8000,
		PLAYBACK_STATE_ERROR = 0xa000, // Error?
		PLAYBACK_STATE_IDLE = 0xb000, // Not playing
		PLAYBACK_STATE_BUFFER_FULL = 0xc000, // Playing, demand pin = 0?
		PLAYBACK_STATE_DEMAND_BUFFER = 0xd000 // Playing, demand pin = 1?
	};

	required_shared_ptr<uint16_t> ram;
	required_device<mas3507d_device> mas3507d;

	uint16_t crypto_key1 = 0, crypto_key2 = 0;
	uint8_t crypto_key3 = 0;

	uint32_t mp3_start_addr = 0, mp3_cur_addr = 0, mp3_end_addr = 0;
	bool use_ddrsbm_fpga = false;

	bool is_stream_active = false, is_timer_active = false;
	uint32_t counter_previous = 0, counter_offset = 0;
	int32_t counter_current = 0;
	uint32_t last_playback_status = 0;
};

#endif // MAME_MACHINE_K573FPGA_H
