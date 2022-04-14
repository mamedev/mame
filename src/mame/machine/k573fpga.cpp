// license:BSD-3-Clause
// copyright-holders:windyfairy
#include "emu.h"
#include "speaker.h"

#include "k573fpga.h"

#define LOG_GENERAL  (1 << 0)
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

	m_stream_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(k573fpga_device::update_stream), this));
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

uint16_t k573fpga_device::decrypt_default(uint16_t v)
{
	uint16_t m = crypto_key1 ^ crypto_key2;

	v = bitswap<16>(
		v,
		15 - BIT(m, 0xF),
		14 + BIT(m, 0xF),
		13 - BIT(m, 0xE),
		12 + BIT(m, 0xE),
		11 - BIT(m, 0xB),
		10 + BIT(m, 0xB),
		9 - BIT(m, 0x9),
		8 + BIT(m, 0x9),
		7 - BIT(m, 0x8),
		6 + BIT(m, 0x8),
		5 - BIT(m, 0x5),
		4 + BIT(m, 0x5),
		3 - BIT(m, 0x3),
		2 + BIT(m, 0x3),
		1 - BIT(m, 0x2),
		0 + BIT(m, 0x2)
	);

	v ^= (BIT(m, 0xD) << 14) ^
		(BIT(m, 0xC) << 12) ^
		(BIT(m, 0xA) << 10) ^
		(BIT(m, 0x7) << 8) ^
		(BIT(m, 0x6) << 6) ^
		(BIT(m, 0x4) << 4) ^
		(BIT(m, 0x1) << 2) ^
		(BIT(m, 0x0) << 0);

	v ^= bitswap<16>(
		(uint16_t)crypto_key3,
		7, 0, 6, 1,
		5, 2, 4, 3,
		3, 4, 2, 5,
		1, 6, 0, 7
	);

	crypto_key1 = (crypto_key1 & 0x8000) | ((crypto_key1 << 1) & 0x7FFE) | ((crypto_key1 >> 14) & 1);

	if(((crypto_key1 >> 15) ^ crypto_key1) & 1)
		crypto_key2 = (crypto_key2 << 1) | (crypto_key2 >> 15);

	crypto_key3++;

	return v;
}

uint16_t k573fpga_device::decrypt_ddrsbm(uint16_t data)
{
	// TODO: Work out the proper decryption algorithm.
	// Similar to the other games, ddrsbm is capable of sending a pre-mutated key that is used to simulate seeking by starting MP3 playback from a non-zero offset.
	// The MP3 seeking functionality doesn't appear to be used so the game doesn't break from lack of support from what I can tell.
	// The proper key mutation found in game code is: crypto_key1 = rol(crypto_key1, offset & 0x0f)

	uint8_t key[16] = {0};
	uint16_t key_state = bitswap<16>(
		crypto_key1,
		13, 11, 9, 7,
		5, 3, 1, 15,
		14, 12, 10, 8,
		6, 4, 2, 0
	);

	for(int i = 0; i < 8; i++) {
		key[i * 2] = key_state & 0xff;
		key[i * 2 + 1] = (key_state >> 8) & 0xff;
		key_state = ((key_state & 0x8080) >> 7) | ((key_state & 0x7f7f) << 1);
	}

	uint16_t output_word = 0;
	for(int cur_bit = 0; cur_bit < 8; cur_bit++) {
		int even_bit_shift = cur_bit * 2;
		int odd_bit_shift = cur_bit * 2 + 1;
		bool is_even_bit_set = data & (1 << even_bit_shift);
		bool is_odd_bit_set = data & (1 << odd_bit_shift);
		bool is_key_bit_set = key[crypto_key3 & 15] & (1 << cur_bit);
		bool is_scramble_bit_set = key[(crypto_key3 - 1) & 15] & (1 << cur_bit);

		if(is_scramble_bit_set)
			std::swap(is_even_bit_set, is_odd_bit_set);

		if(is_even_bit_set ^ is_key_bit_set)
			output_word |= 1 << even_bit_shift;

		if(is_odd_bit_set)
			output_word |= 1 << odd_bit_shift;
	}

	crypto_key3++;

	return output_word;
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

WRITE_LINE_MEMBER(k573fpga_device::mpeg_frame_sync)
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

WRITE_LINE_MEMBER(k573fpga_device::mas3507d_demand)
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
