// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef _NAOMIM1_H_
#define _NAOMIM1_H_

#include "naomibd.h"

#define MCFG_NAOMI_M1_BOARD_ADD(_tag, _eeprom_tag, _actel_tag, _irq_cb) \
	MCFG_NAOMI_BOARD_ADD(_tag, NAOMI_M1_BOARD, _eeprom_tag, _actel_tag, _irq_cb)

class naomi_m1_board : public naomi_board
{
public:
	naomi_m1_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(submap, 16) override;

	DECLARE_READ16_MEMBER(actel_id_r);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void board_setup_address(UINT32 address, bool is_dma) override;
	virtual void board_get_buffer(UINT8 *&base, UINT32 &limit) override;
	virtual void board_advance(UINT32 size) override;

private:
	enum { BUFFER_SIZE = 32768 };
	UINT32 key;

	std::unique_ptr<UINT8[]> buffer;
	UINT8 dict[111], hist[2];
	UINT64 avail_val;
	UINT32 rom_cur_address, buffer_actual_size, avail_bits;
	bool encryption, stream_ended, has_history;

	required_memory_region m_region;

	void gb_reset();
	UINT32 lookb(int bits);
	void skipb(int bits);
	UINT32 getb(int bits);

	void enc_reset();
	void enc_fill();

	UINT32 get_decrypted_32b();

	void wb(UINT8 byte);
};

extern const device_type NAOMI_M1_BOARD;

#endif
