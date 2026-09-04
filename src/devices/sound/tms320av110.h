// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_SOUND_TMS320AV110_H
#define MAME_SOUND_TMS320AV110_H

#pragma once

#include "mpeg_audio.h"

class tms320av110_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	tms320av110_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	auto req() { return m_req_cb.bind(); } // active-low compressed-data request output
	void set_external_dram(bool external) { m_external_dram = external; }

	void map(address_map &map) ATTR_COLD;
	void reset_w(int state); // active-low RESET input

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// The host interface has seven address pins, SADDR6 through SADDR0.
	static constexpr u32 HOST_REGISTER_COUNT = 0x80;

	// Host-interface register addresses from the data sheet.
	static constexpr offs_t REG_BUFF_L = 0x12;   // input buffer word count, bits 7:0
	static constexpr offs_t REG_BUFF_H = 0x13;   // input buffer word count, bits 14:8
	static constexpr offs_t REG_FREE_FORM_L = 0x14; // free-format frame length, bits 7:0
	static constexpr offs_t REG_FREE_FORM_H = 0x15; // free-format frame length, bits 10:8
	static constexpr offs_t REG_PCM_18 = 0x16;   // PCM output precision
	static constexpr offs_t REG_DATAIN = 0x18;   // memory-mapped compressed-data input
	static constexpr offs_t REG_INTR_L = 0x1a;   // interrupt status, bits 7:0
	static constexpr offs_t REG_INTR_H = 0x1b;   // interrupt status, bits 15:8
	static constexpr offs_t REG_INTR_EN_L = 0x1c; // interrupt enable, bits 7:0
	static constexpr offs_t REG_INTR_EN_H = 0x1d; // interrupt enable, bits 15:8
	static constexpr offs_t REG_ATTEN_L = 0x1e;  // left output attenuation
	static constexpr offs_t REG_ATTEN_R = 0x20;  // right output attenuation
	static constexpr offs_t REG_AUD_ID = 0x22;   // MPEG system/packet audio stream ID
	static constexpr offs_t REG_AUD_ID_EN = 0x24; // audio stream ID filtering enable
	static constexpr offs_t REG_SYNC_ST = 0x26;  // synchronization status
	static constexpr offs_t REG_SYNC_LCK = 0x28; // required additional synchronization words
	static constexpr offs_t REG_CRC_ECM = 0x2a;  // CRC error concealment mode
	static constexpr offs_t REG_SYNC_ECM = 0x2c; // synchronization error concealment mode
	static constexpr offs_t REG_PLAY = 0x2e;     // decoded audio output enable
	static constexpr offs_t REG_MUTE = 0x30;     // decoded audio mute
	static constexpr offs_t REG_SKIP = 0x32;     // skip next audio frame
	static constexpr offs_t REG_REPEAT = 0x34;   // repeat next audio frame
	static constexpr offs_t REG_STR_SEL = 0x36;  // compressed input stream format
	static constexpr offs_t REG_PCM_ORD = 0x38;  // PCM output bit order
	static constexpr offs_t REG_LATENCY = 0x3c;  // synchronization lookahead enable
	static constexpr offs_t REG_DRAM_EXT = 0x3e; // external input-buffer DRAM present
	static constexpr offs_t REG_RESET = 0x40;    // decoder reset command/status
	static constexpr offs_t REG_RESTART = 0x42;  // data-buffer flush command/status
	static constexpr offs_t REG_PCM_FS = 0x44;   // decoded sampling frequency
	static constexpr offs_t REG_PCM_DIV = 0x6e;  // PCM clock divider
	static constexpr offs_t REG_DIF = 0x6f;      // 18-bit PCM justification
	static constexpr offs_t REG_SIN_EN = 0x70;   // serial compressed-data input enable

	// The on-chip input SRAM is 256 bytes; external DRAM is 256K by four bits.
	static constexpr u32 INPUT_BUFFER_BYTES_WITHOUT_DRAM = 256;
	static constexpr u32 INPUT_BUFFER_BYTES_WITH_DRAM = 256 * 1024 * 4 / 8;
	// One further byte may be accepted after REQ reports full.
	static constexpr u32 INPUT_FIFO_BYTES = INPUT_BUFFER_BYTES_WITH_DRAM + 1;

	// Approximately 700 microseconds at the nominal 24 MHz oscillator frequency.
	static constexpr u32 RESET_CYCLES_WITHOUT_DRAM = 16'800;
	// Approximately 3.7 milliseconds at the nominal 24 MHz oscillator frequency.
	static constexpr u32 RESET_CYCLES_WITH_DRAM = 88'800;

	static constexpr u32 LEFT_CHANNEL = 0;
	static constexpr u32 RIGHT_CHANNEL = 1;
	static constexpr u32 OUTPUT_CHANNELS = 2;
	static constexpr u32 INITIAL_SAMPLE_RATE = 44'100;
	static constexpr u32 MPEG_LAYER_II_SAMPLES_PER_FRAME = 1'152;
	static constexpr u32 MAX_MPEG1_LAYER_II_BIT_RATE = 384'000;
	static constexpr u32 MIN_MPEG1_SAMPLE_RATE = 32'000;
	static constexpr u32 MPEG1_LAYER_II_FRAME_SCALE = 144;
	static constexpr u32 MPEG_FRAME_PADDING_BYTES = 1;
	static constexpr u32 MAX_MPEG_AUDIO_FRAME_BYTES =
		(MPEG1_LAYER_II_FRAME_SCALE * MAX_MPEG1_LAYER_II_BIT_RATE / MIN_MPEG1_SAMPLE_RATE) + MPEG_FRAME_PADDING_BYTES;
	// Private staging storage for adapting streaming input to the frame decoder.
	static constexpr u32 DECODER_INPUT_BYTES = 2 * MAX_MPEG_AUDIO_FRAME_BYTES;

	void decoder_reset();
	bool decode_frame();
	u8 buffer_low_r();
	u8 buffer_high_r();
	u8 external_dram_r();
	void data_w(u8 data);
	void reset_command_w(u8 data);
	void restart_command_w(u8 data);
	u8 unimplemented_r(offs_t offset);
	void unimplemented_w(offs_t offset, u8 data);
	void store_register(offs_t reg, u8 data, u8 mask, bool update_stream);
	template <offs_t Register> u8 register_r();
	void free_form_low_w(u8 data);
	void free_form_high_w(u8 data);
	void pcm_precision_w(u8 data);
	void interrupt_enable_w(offs_t offset, u8 data);
	void left_attenuation_w(u8 data);
	void right_attenuation_w(u8 data);
	void audio_id_w(u8 data);
	void audio_id_enable_w(u8 data);
	void sync_words_w(u8 data);
	void crc_error_concealment_w(u8 data);
	void sync_error_concealment_w(u8 data);
	void play_w(u8 data);
	void mute_w(u8 data);
	void skip_w(u8 data);
	void repeat_w(u8 data);
	void stream_select_w(u8 data);
	void pcm_order_w(u8 data);
	void latency_w(u8 data);
	void pcm_divider_w(u8 data);
	void pcm_justification_w(u8 data);
	void serial_input_enable_w(u8 data);
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
	u8 m_registers[HOST_REGISTER_COUNT];
	std::unique_ptr<u8[]> m_input_fifo;
	u8 m_input[DECODER_INPUT_BYTES];
	s16 m_pcm[MPEG_LAYER_II_SAMPLES_PER_FRAME * OUTPUT_CHANNELS];
	std::unique_ptr<mpeg_audio> m_decoder;
	u32 m_input_fifo_read;
	u32 m_input_fifo_write;
	u32 m_input_fifo_count;
	u32 m_input_bytes;
	u32 m_input_bit_position;
	u32 m_pcm_position;
	u32 m_pcm_count;
	u32 m_pcm_channels;
	bool m_external_dram;
	emu_timer *m_input_timer;
	emu_timer *m_reset_timer;
	bool m_reset_asserted;
	bool m_reset_cycle;
	bool m_data_access;
	u8 m_req_state;
};

DECLARE_DEVICE_TYPE(TMS320AV110, tms320av110_device)

#endif // MAME_SOUND_TMS320AV110_H
