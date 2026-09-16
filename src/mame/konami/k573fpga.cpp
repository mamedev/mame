// license:BSD-3-Clause
// copyright-holders:windyfairy
#include "emu.h"
#include "speaker.h"

#include "k573fpga.h"

#define VERBOSE      (LOG_GENERAL)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

k573fpga_device::k573fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, KONAMI_573_DIGITAL_FPGA, tag, owner, clock),
	ram(*this, finder_base::DUMMY_TAG),
	mas3507d(*this, "mpeg"),
	is_ddrsbm_fpga(false)
{
}

void k573fpga_device::device_add_mconfig(machine_config &config)
{
	MAS3507D(config, mas3507d);
	mas3507d->mpeg_frame_sync_cb().set(*this, FUNC(k573fpga_device::mpeg_frame_sync));
	mas3507d->demand_cb().set(*this, FUNC(k573fpga_device::mas3507d_demand));
}

void k573fpga_device::device_start()
{
	save_item(NAME(crypto_key1));
	save_item(NAME(crypto_key2));
	save_item(NAME(crypto_key3));
	save_item(NAME(crypto_key1_start));
	save_item(NAME(crypto_key2_start));
	save_item(NAME(crypto_key3_start));
	save_item(NAME(mp3_start_addr));
	save_item(NAME(mp3_end_addr));
	save_item(NAME(mp3_cur_start_addr));
	save_item(NAME(mp3_cur_end_addr));
	save_item(NAME(mp3_cur_addr));
	save_item(NAME(mp3_data));
	save_item(NAME(mp3_remaining_bytes));
	save_item(NAME(is_ddrsbm_fpga));
	save_item(NAME(mpeg_status));
	save_item(NAME(fpga_status));
	save_item(NAME(mp3_frame_counter));
	save_item(NAME(counter_value));
	save_item(NAME(counter_current));
	save_item(NAME(counter_base));
	save_item(NAME(is_mpeg_frame_synced));

	m_stream_timer = timer_alloc(FUNC(k573fpga_device::update_stream), this);
	m_stream_timer->adjust(attotime::never);
}

void k573fpga_device::device_reset()
{
	fpga_status = 0;
	mp3_start_addr = 0;
	mp3_end_addr = 0;
	mp3_cur_start_addr = 0;
	mp3_cur_end_addr = 0;
	mp3_cur_addr = 0;
	mp3_data = 0;
	mp3_remaining_bytes = 0;

	crypto_key1_start = crypto_key1 = 0;
	crypto_key2_start = crypto_key2 = 0;
	crypto_key3_start = crypto_key3 = 0;

	counter_current = counter_base = machine().time();

	mpeg_status = 0;
	mp3_frame_counter = 0;
	counter_value = 0;
	is_mpeg_frame_synced = false;

	mas3507d->reset_playback();
}

void k573fpga_device::reset_counter()
{
	is_mpeg_frame_synced = false;
	counter_current = counter_base = machine().time();
	counter_value = 0;
}

void k573fpga_device::update_counter()
{
	if (is_ddrsbm_fpga) {
		// The counter for Solo Bass Mix is used differently than other games.
		// DDR Solo Bass Mix will sync the internal playback timer to the first second of the MP3 using the MP3 frame counter.
		// After that the playback timer is incremented using the difference between the last counter value and the current counter value.
		// This counter register itself is always running even when no audio is playing.
		// TODO: What happens when mp3_counter_low_w is written to on Solo Bass Mix?
		counter_value = (machine().time() - counter_base).as_double();
	}
	else if (is_mpeg_frame_synced) {
		// Timer only seems to start when the first MPEG frame sync is encountered, so wait for that trigger
		counter_base = counter_current;
		counter_current = machine().time();
		counter_value += (counter_current - counter_base).as_double();
	}
}

uint32_t k573fpga_device::get_counter()
{
	update_counter();
	return counter_value * 44100;
}

uint32_t k573fpga_device::get_counter_diff()
{
	// Delta playback time since last counter update.
	// I couldn't find any active usages of this register but it exists in some code paths.
	// The functionality was tested using custom code running on real hardware.
	// When this is called, it will return the difference between the current counter value
	// and the last read counter value, and then reset the counter back to the previously read counter's value.
	auto prev = counter_value;
	update_counter();
	auto diff = counter_value - prev;
	counter_value = prev;
	return diff * 44100;
}

uint16_t k573fpga_device::get_mp3_frame_count()
{
	// All games can read this but only DDR Solo Bass Mix actively uses it.
	// Returns the same value as using a default read to get the frame counter from the MAS3507D over i2c.
	return mp3_frame_counter & 0xffff;
}

uint16_t k573fpga_device::mas_i2c_r()
{
	uint16_t scl = mas3507d->i2c_scl_r() << 13;
	uint16_t sda = mas3507d->i2c_sda_r() << 12;
	return scl | sda;
}

void k573fpga_device::mas_i2c_w(uint16_t data)
{
	mas3507d->i2c_scl_w(data & 0x2000);
	mas3507d->i2c_sda_w(data & 0x1000);
}

void k573fpga_device::set_crypto_key1(uint16_t v)
{
	crypto_key1_start = crypto_key1 = v;
	update_mp3_decode_state();
}
void k573fpga_device::set_crypto_key2(uint16_t v)
{
	crypto_key2_start = crypto_key2 = v;
	update_mp3_decode_state();
}
void k573fpga_device::set_crypto_key3(uint8_t v)
{
	crypto_key3_start = crypto_key3 = v;
	update_mp3_decode_state();
}

void k573fpga_device::set_mp3_start_addr(uint32_t v)
{
	mp3_start_addr = v;
	update_mp3_decode_state();
}

void k573fpga_device::set_mp3_end_addr(uint32_t v)
{
	mp3_end_addr = v;
	update_mp3_decode_state();
}

uint16_t k573fpga_device::get_mpeg_ctrl()
{
	return mpeg_status;
}

uint16_t k573fpga_device::get_fpga_ctrl()
{
	// 0x0000 Not Streaming
	// 0x1000 Streaming
	int is_streaming = BIT(fpga_status, FPGA_STREAMING_ENABLE)
		&& mp3_cur_addr >= mp3_cur_start_addr
		&& mp3_cur_addr < mp3_cur_end_addr;
	return is_streaming << 12;
}

void k573fpga_device::set_fpga_ctrl(uint16_t data)
{
	LOG("FPGA MPEG control %c%c%c | %04x\n",
				BIT(data, FPGA_FRAME_COUNTER_ENABLE) ? '#' : '.',
				BIT(data, FPGA_STREAMING_ENABLE) ? '#' : '.',
				BIT(data, FPGA_MP3_ENABLE) ? '#' : '.',
				data);

	if (!BIT(data, FPGA_FRAME_COUNTER_ENABLE) && BIT(fpga_status, FPGA_FRAME_COUNTER_ENABLE)) {
		mp3_frame_counter = 0;
	}

	if ((BIT(data, FPGA_MP3_ENABLE) != BIT(fpga_status, FPGA_MP3_ENABLE))
		|| (BIT(data, FPGA_STREAMING_ENABLE) != BIT(fpga_status, FPGA_STREAMING_ENABLE))) {
		mas3507d->reset_playback();
	}

	fpga_status = data;
}


void k573fpga_device::update_mp3_decode_state()
{
	// HACK: The exact timing of when the internal state in the FPGA updates is still unknown
	// so update the state any time one of the core settings (decryption keys or data start/stop addr)
	// for a stream changes.
	mp3_cur_addr = mp3_start_addr;
	mp3_cur_start_addr = mp3_start_addr;
	mp3_cur_end_addr = mp3_end_addr;
	is_mpeg_frame_synced = false;
	mp3_remaining_bytes = 0;
	crypto_key1 = crypto_key1_start;
	crypto_key2 = crypto_key2_start;
	crypto_key3 = crypto_key3_start;
	mp3_frame_counter = 0;
	reset_counter();

	if (is_ddrsbm_fpga) {
		crypto_key3 = crypto_key3_start = 0;
	}

	mas3507d->reset_playback();
}

uint16_t k573fpga_device::decrypt_common(uint16_t data, uint16_t key)
{
	// In both variants of the decryption algorithm, a 16-bit key is derived
	// from the current key state and treated as a set of 8 pairs of bits. Bit 1
	// of each pair (i.e. bits 1, 3, 5, etc. of the key) controls whether the
	// respective bit pair of the data is swapped or not.
	data = bitswap<16>(
		data,
		15 - BIT(key, 15), 14 + BIT(key, 15),
		13 - BIT(key, 13), 12 + BIT(key, 13),
		11 - BIT(key, 11), 10 + BIT(key, 11),
		9 - BIT(key,  9),  8 + BIT(key,  9),
		7 - BIT(key,  7),  6 + BIT(key,  7),
		5 - BIT(key,  5),  4 + BIT(key,  5),
		3 - BIT(key,  3),  2 + BIT(key,  3),
		1 - BIT(key,  1),  0 + BIT(key,  1)
	);

	// Bit 0 of each pair (bits 0, 2, 4, etc. of the key) controls whether the
	// respective data bit is inverted after performing the swap.
	data ^= key & 0x5555;

	return data;
}

uint16_t k573fpga_device::decrypt_default(uint16_t data)
{
	const uint16_t derived_key = bitswap<16>(
		crypto_key1 ^ crypto_key2,
		15, 13, 14, 12,
		11, 10,  9,  7,
		8,  6,  5,  4,
		3,  1,  2,  0
	);

	data = decrypt_common(data, derived_key);

	data ^= bitswap<16>(
		(uint16_t)crypto_key3,
		7, 0, 6, 1,
		5, 2, 4, 3,
		3, 4, 2, 5,
		1, 6, 0, 7
	);

	if (BIT(crypto_key1, 14) ^ BIT(crypto_key1, 15))
		crypto_key2 = (crypto_key2 << 1) | (crypto_key2 >> 15);

	crypto_key1 = (crypto_key1 & 0x8000) | ((crypto_key1 << 1) & 0x7ffe) | ((crypto_key1 >> 14) & 1);
	crypto_key3++;

	return data;
}

uint16_t k573fpga_device::decrypt_ddrsbm(uint16_t data)
{
	data = decrypt_common(data, crypto_key1);
	crypto_key1 = (crypto_key1 << 1) | (crypto_key1 >> 15);
	return data;
}

TIMER_CALLBACK_MEMBER(k573fpga_device::update_stream)
{
	if (!BIT(mpeg_status, PLAYBACK_STATE_DEMAND)) {
		return;
	}

	if (!BIT(fpga_status, FPGA_MP3_ENABLE)
		|| !BIT(fpga_status, FPGA_STREAMING_ENABLE)
		|| mp3_cur_addr < mp3_cur_start_addr
		|| mp3_cur_addr >= mp3_cur_end_addr) {
		return;
	}

	if (mp3_remaining_bytes <= 0) {
		uint16_t src = ram[mp3_cur_addr >> 1];
		mp3_data = is_ddrsbm_fpga ? decrypt_ddrsbm(src) : decrypt_default(src);
		mp3_data = ((mp3_data >> 8) & 0xff) | ((mp3_data & 0xff) << 8);
		mp3_cur_addr += 2;
		mp3_remaining_bytes = 2;
	}

	mas3507d->sid_w(mp3_data & 0xff);
	mp3_data >>= 8;
	mp3_remaining_bytes--;
}

void k573fpga_device::mpeg_frame_sync(int state)
{
	if (state) {
		mpeg_status &= ~(1 << PLAYBACK_STATE_IDLE);
		mpeg_status |= 1 << PLAYBACK_STATE_PLAYING;

		if (!is_mpeg_frame_synced) {
			reset_counter();
			is_mpeg_frame_synced = true;
		}

		if (BIT(fpga_status, FPGA_FRAME_COUNTER_ENABLE)) {
			mp3_frame_counter++;
		}
	}
	else {
		mpeg_status &= ~(1 << PLAYBACK_STATE_PLAYING);
		mpeg_status |= 1 << PLAYBACK_STATE_IDLE;
	}
}

void k573fpga_device::mas3507d_demand(int state)
{
	if (state && !BIT(mpeg_status, PLAYBACK_STATE_DEMAND)) {
		mpeg_status |= 1 << PLAYBACK_STATE_DEMAND;
	}
	else if (!state && BIT(mpeg_status, PLAYBACK_STATE_DEMAND)) {
		mpeg_status &= ~(1 << PLAYBACK_STATE_DEMAND);
		m_stream_timer->adjust(attotime::never);
	}

	if (state && BIT(mpeg_status, PLAYBACK_STATE_DEMAND)) {
		m_stream_timer->adjust(attotime::zero);
	}
}

DEFINE_DEVICE_TYPE(KONAMI_573_DIGITAL_FPGA, k573fpga_device, "k573fpga", "Konami 573 Digital I/O FPGA")
