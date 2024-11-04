// license:BSD-3-Clause
// copyright-holders:Tatsuyuki Satoh
#ifndef MAME_SOUND_VLM5030_H
#define MAME_SOUND_VLM5030_H

#pragma once

#include "dirom.h"

class vlm5030_device : public device_t, public device_sound_interface, public device_rom_interface<16>
{
public:
	vlm5030_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	/* get BSY pin level */
	int bsy();

	/* latch contoll data */
	void data_w(uint8_t data);

	/* set RST pin level : reset / set table address A8-A15 */
	void rst(int state);

	/* set VCU pin level : ?? unknown */
	void vcu(int state);

	/* set ST pin level  : set table address A0-A7 / start speech */
	void st(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

private:
	// configuration state
	const address_space_config m_space_config;

	// internal state
	sound_stream *m_channel;

	/* coefficient tables */
	const struct tms5100_coeffs *m_coeff;

	/* need to save state */
	uint16_t m_address;
	uint8_t m_pin_BSY;
	uint8_t m_pin_ST;
	uint8_t m_pin_VCU;
	uint8_t m_pin_RST;
	uint8_t m_latch_data;
	uint16_t m_vcu_addr_h;
	uint8_t m_parameter;
	uint8_t m_phase;

	/* state of option parameter */
	int m_frame_size;
	int m_pitch_offset;
	uint8_t m_interp_step;

	uint8_t m_interp_count;       /* number of interp periods    */
	uint8_t m_sample_count;       /* sample number within interp */
	uint8_t m_pitch_count;

	/* these contain data describing the current and previous voice frames */
	uint16_t m_old_energy;
	uint8_t m_old_pitch;
	int16_t  m_old_k[10];
	uint16_t m_target_energy;
	uint8_t m_target_pitch;
	int16_t m_target_k[10];

	uint16_t m_new_energy;
	uint8_t m_new_pitch;
	int16_t m_new_k[10];

	/* these are all used to contain the current state of the sound generation */
	unsigned int m_current_energy;
	unsigned int m_current_pitch;
	int m_current_k[10];

	int32_t m_x[10];

	int get_bits(int sbit,int bits);
	int parse_frame();
	void update();
	void setup_parameter(uint8_t param);
	void restore_state();
};

DECLARE_DEVICE_TYPE(VLM5030, vlm5030_device)

#endif // MAME_SOUND_VLM5030_H
