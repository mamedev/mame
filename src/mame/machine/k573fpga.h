// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_MACHINE_K573FPGA_H
#define MAME_MACHINE_K573FPGA_H

#pragma once

#include "sound/mas3507d.h"
#include "machine/ds2401.h"

#include "sound/samples.h"

#define MINIMP3_ONLY_MP3
#define MINIMP3_NO_STDIO
#include "minimp3/minimp3.h"
#include "minimp3/minimp3_ex.h"

DECLARE_DEVICE_TYPE(KONAMI_573_DIGITAL_FPGA, k573fpga_device)

class k573fpga_device : public device_t
{
public:
	k573fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	required_device<mas3507d_device> mas3507d;
	required_device<samples_device> m_samples;

	void set_ddrsbm_fpga(bool flag) { use_ddrsbm_fpga = flag; }

	void set_ram(uint16_t *v) { ram = v; }

	void set_crypto_key1(uint16_t v) { orig_crypto_key1 = crypto_key1 = v; }
	void set_crypto_key2(uint16_t v) { orig_crypto_key2 = crypto_key2 = v; }
	void set_crypto_key3(uint8_t v) { orig_crypto_key3 = crypto_key3 = v; }

	uint32_t get_mp3_start_adr() { return mp3_start_adr; }
	void set_mp3_start_adr(uint32_t v) { mp3_start_adr = v; }

	uint32_t get_mp3_end_adr() { return mp3_end_adr; }
	void set_mp3_end_adr(uint32_t v) { mp3_end_adr = v; }

	uint32_t get_mp3_playback();

	uint16_t i2c_read();
	void i2c_write(uint16_t data);

	uint16_t get_mpeg_ctrl();
	void set_mpeg_ctrl(uint16_t data);

	void set_mp3_dynamic_base(uint32_t base) { mp3_dynamic_base = base; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	uint16_t *ram;
	std::unique_ptr<uint16_t[]> ram_swap;

	uint16_t crypto_key1, crypto_key2, orig_crypto_key1, orig_crypto_key2;
	uint8_t crypto_key3, orig_crypto_key3;

	uint32_t mp3_start_adr, mp3_end_adr, mpeg_ctrl_flag;
	bool use_ddrsbm_fpga;

	uint32_t mp3_last_adr, mp3_next_sync, mp3_last_decrypt_adr;
	int16_t *channel_l_pcm, *channel_r_pcm;
	size_t last_buffer_size_channel_l, last_buffer_size_channel_r, last_copied_samples;
	uint32_t last_position_update, position_diff;
	bool decrypt_finished;

	mp3d_sample_t mp3_pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
	mp3dec_file_info_t mp3_info;
	mp3dec_t mp3_dec;
	mp3dec_frame_info_t mp3_frame_info;
	size_t mp3_allocated;

	uint32_t buffer_speed, mp3_dynamic_base;

	int32_t find_enc_key();

	uint16_t fpga_decrypt_byte_real(uint16_t data);
	uint16_t fpga_decrypt_byte_ddrsbm(uint16_t data, uint32_t crypto_idx);

	SAMPLES_UPDATE_CB_MEMBER(k573fpga_stream_update);
};

#endif // MAME_MACHINE_K573FPGA_H
