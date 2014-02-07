#ifndef _NAOMIM4_H_
#define _NAOMIM4_H_

#include "naomibd.h"

#define MCFG_NAOMI_M4_BOARD_ADD(_tag, _key_tag, _eeprom_tag, _actel_tag, _irq_cb) \
	MCFG_NAOMI_BOARD_ADD(_tag, NAOMI_M4_BOARD, _eeprom_tag, _actel_tag, _irq_cb) \
	naomi_m4_board::static_set_tags(*device, _key_tag);

class naomi_m4_board : public naomi_board
{
public:
	naomi_m4_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_tags(device_t &device, const char *_key_tag);

	virtual DECLARE_ADDRESS_MAP(submap, 16);

	DECLARE_READ16_MEMBER(m4_id_r);
protected:
	virtual void device_start();
	virtual void device_reset();

	virtual void board_setup_address(UINT32 address, bool is_dma);
	virtual void board_get_buffer(UINT8 *&base, UINT32 &limit);
	virtual void board_advance(UINT32 size);
	virtual void board_write(offs_t offset, UINT16 data);

private:
	enum { BUFFER_SIZE = 32768 };

	static const UINT8 k_sboxes[4][16];

	const char *key_tag;
	UINT16 key, iv;
	UINT16 *one_round;

	UINT8 *buffer;
	UINT32 rom_cur_address, buffer_actual_size;
	UINT16 cur_iv;
	UINT8 counter;
	bool encryption;
	bool cfi_mode;

	void enc_init();
	void enc_reset();
	void enc_fill();
};

extern const device_type NAOMI_M4_BOARD;

#endif
