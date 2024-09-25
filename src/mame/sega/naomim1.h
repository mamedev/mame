// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_NAOMIM1_H
#define MAME_SEGA_NAOMIM1_H

#pragma once

#include "naomibd.h"


class naomi_m1_board : public naomi_board
{
public:
	template <typename T>
	naomi_m1_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&eeprom_tag)
		: naomi_m1_board(mconfig, tag, owner, clock)
	{
		eeprom.set_tag(std::forward<T>(eeprom_tag));
	}

	naomi_m1_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void submap(address_map &map) override ATTR_COLD;

	uint16_t actel_id_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void board_setup_address(uint32_t address, bool is_dma) override;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) override;
	virtual void board_advance(uint32_t size) override;

private:
	enum { BUFFER_SIZE = 32768 };
	uint32_t key;
	uint16_t actel_id;

	std::unique_ptr<uint8_t[]> buffer;
	uint8_t dict[111], hist[2];
	uint64_t avail_val;
	uint32_t rom_cur_address, buffer_actual_size, avail_bits;
	bool encryption, stream_ended, has_history;

	required_memory_region m_region;

	void gb_reset();
	uint32_t lookb(int bits);
	void skipb(int bits);
	uint32_t getb(int bits);

	void enc_reset();
	void enc_fill();

	uint32_t get_decrypted_32b();

	void wb(uint8_t byte);
};

DECLARE_DEVICE_TYPE(NAOMI_M1_BOARD, naomi_m1_board)

#endif // MAME_SEGA_NAOMIM1_H
