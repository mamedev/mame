// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Andreas Naive
#ifndef MAME_SEGA_NAOMIM4_H
#define MAME_SEGA_NAOMIM4_H

#include "naomibd.h"


class naomi_m4_board : public naomi_board
{
public:
	template <typename T, typename U>
	naomi_m4_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&eeprom_tag, U &&keyregion_tag)
		: naomi_m4_board(mconfig, tag, owner, clock)
	{
		eeprom.set_tag(std::forward<T>(eeprom_tag));
		m_key_data.set_tag(std::forward<U>(keyregion_tag));
	}

	naomi_m4_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void submap(address_map &map) override ATTR_COLD;

	uint16_t m4_id_r();
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void board_setup_address(uint32_t address, bool is_dma) override;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) override;
	virtual void board_advance(uint32_t size) override;
	virtual void board_write(offs_t offset, uint16_t data) override;

private:
	enum { BUFFER_SIZE = 32768 };

	static const uint8_t k_sboxes[4][16];

	uint16_t m4id;
	required_memory_region m_region;
	required_region_ptr<uint8_t> m_key_data;
	uint16_t subkey1, subkey2;
	std::unique_ptr<uint16_t[]> one_round;

	std::unique_ptr<uint8_t[]> buffer;
	uint32_t rom_cur_address, buffer_actual_size;
	uint16_t iv;
	uint8_t counter;
	bool encryption;
	bool cfi_mode;

	void enc_init();
	void enc_reset();
	void enc_fill();
	uint16_t decrypt_one_round(uint16_t word, uint16_t subkey);
};

DECLARE_DEVICE_TYPE(NAOMI_M4_BOARD, naomi_m4_board)

#endif // MAME_SEGA_NAOMIM4_H
