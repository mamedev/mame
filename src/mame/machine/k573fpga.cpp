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
	use_ddrsbm_fpga(false)
{
}

void k573fpga_device::device_add_mconfig(machine_config &config)
{
	MAS3507D(config, mas3507d);
	mas3507d->sample_cb().set(*this, FUNC(k573fpga_device::get_decrypted));
}

void k573fpga_device::device_start()
{
	save_item(NAME(crypto_key1));
	save_item(NAME(crypto_key2));
	save_item(NAME(crypto_key3));
	save_item(NAME(mp3_start_addr));
	save_item(NAME(mp3_cur_addr));
	save_item(NAME(mp3_end_addr));
	save_item(NAME(use_ddrsbm_fpga));
	save_item(NAME(is_stream_active));
	save_item(NAME(is_timer_active));
	save_item(NAME(counter_previous));
	save_item(NAME(counter_current));
	save_item(NAME(last_playback_status));
}

void k573fpga_device::device_reset()
{
	mp3_start_addr = 0;
	mp3_cur_addr = 0;
	mp3_end_addr = 0;

	crypto_key1 = 0;
	crypto_key2 = 0;
	crypto_key3 = 0;

	is_stream_active = false;
	is_timer_active = false;

	counter_current = counter_previous = counter_offset = 0;

	mas3507d->reset_playback();
	last_playback_status = get_mpeg_ctrl();
}

void k573fpga_device::reset_counter() {
	counter_current = counter_previous = counter_offset = 0;
	status_update();
}

void k573fpga_device::status_update() {
	auto cur_playback_status = get_mpeg_ctrl();
	is_timer_active = is_streaming() || ((cur_playback_status == last_playback_status && last_playback_status > PLAYBACK_STATE_IDLE) || cur_playback_status > last_playback_status);
	last_playback_status = cur_playback_status;

	if(!is_timer_active)
		counter_current = counter_previous = counter_offset = 0;
}

uint32_t k573fpga_device::get_counter() {
	status_update();

	counter_previous = counter_current;

	if(is_timer_active) {
		mas3507d->update_stream();
		counter_current = mas3507d->get_samples() - counter_offset;
	}

	return counter_current;
}

uint32_t k573fpga_device::get_counter_diff() {
	// Delta playback time since last counter update.
	// I couldn't find any active usages of this register but it exists in some code paths.
	// The functionality was tested using custom code running on real hardware.
	// When this is called, it will return the difference between the current counter value
	// and the last read counter value, and then reset the counter back to the previously read counter's value.
	auto diff = counter_current - counter_previous;
	counter_current -= diff;
	counter_previous = counter_current;
	get_counter();
	return diff;
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

uint16_t k573fpga_device::get_mpeg_ctrl()
{
	switch(mas3507d->get_status()) {
		case mas3507d_device::PLAYBACK_STATE_IDLE:
			return PLAYBACK_STATE_IDLE;

		case mas3507d_device::PLAYBACK_STATE_BUFFER_FULL:
			return PLAYBACK_STATE_BUFFER_FULL;

		case mas3507d_device::PLAYBACK_STATE_DEMAND_BUFFER:
			return PLAYBACK_STATE_DEMAND_BUFFER;
	}

	return PLAYBACK_STATE_IDLE;
}

bool k573fpga_device::is_mp3_playing()
{
	return get_mpeg_ctrl() > PLAYBACK_STATE_IDLE;
}

uint16_t k573fpga_device::get_fpga_ctrl()
{
	// 0x0000 Not Streaming
	// 0x1000 Streaming
	return is_streaming() << 12;
}

bool k573fpga_device::is_streaming()
{
	return is_stream_active && mp3_cur_addr < mp3_end_addr;
}

void k573fpga_device::set_mpeg_ctrl(uint16_t data)
{
	LOG("FPGA MPEG control %c%c%c | %04x\n",
				data & 0x8000 ? '#' : '.',
				data & 0x4000 ? '#' : '.', // "Active" flag. The FPGA will never start streaming data without this bit set
				data & 0x2000 ? '#' : '.',
				data);

	mas3507d->reset_playback();

	if(data == 0xa000) {
		is_stream_active = false;
		counter_current = counter_previous = 0;
		status_update();
	} else if(data == 0xe000) {
		is_stream_active = true;
		mp3_cur_addr = mp3_start_addr;

		reset_counter();

		if(!mas3507d->is_started) {
			mas3507d->start_playback();
			mas3507d->update_stream();

			// Audio should be buffered by this point.
			// The assumption is that the number of samples actually played can be
			// calculated by subtracting the base sample count when the song was started
			// from the current sample count when the counter register is read.
			// Otherwise, the sample count will always be ahead by the number of samples
			// that were in the buffered frames.
			counter_offset = mas3507d->get_samples();
		}
	}
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

uint16_t k573fpga_device::get_decrypted()
{
	if(!is_streaming()) {
		is_stream_active = false;
		return 0;
	}

	uint16_t src = ram[mp3_cur_addr >> 1];
	uint16_t result = use_ddrsbm_fpga ? decrypt_ddrsbm(src) : decrypt_default(src);
	mp3_cur_addr += 2;

	return result;
}

DEFINE_DEVICE_TYPE(KONAMI_573_DIGITAL_FPGA, k573fpga_device, "k573fpga", "Konami 573 Digital I/O FPGA")
