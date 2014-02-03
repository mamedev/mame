#ifndef _NAOMIM2_H_
#define _NAOMIM2_H_

#include "naomibd.h"

#define MCFG_NAOMI_M2_BOARD_ADD(_tag, _key_tag, _eeprom_tag, _actel_tag, _irq_cb) \
	MCFG_NAOMI_BOARD_ADD(_tag, NAOMI_M2_BOARD, _eeprom_tag, _actel_tag, _irq_cb) \
	naomi_m2_board::static_set_tags(*device, _key_tag);

class naomi_m2_board : public naomi_board
{
public:
	naomi_m2_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_tags(device_t &device, const char *_key_tag);

protected:
	virtual void device_start();
	virtual void device_reset();

	virtual void board_setup_address(UINT32 address, bool is_dma);
	virtual void board_get_buffer(UINT8 *&base, UINT32 &limit);
	virtual void board_advance(UINT32 size);
	virtual void board_write(offs_t offset, UINT16 data);

private:
	enum {
		RAM_SIZE = 65536, BUFFER_SIZE = 32768, LINE_SIZE = 512,
		FLAG_COMPRESSED = 0x10000, FLAG_LINE_SIZE_512 = 0x20000
	};

	const char *key_tag;
	UINT32 key;

	UINT8 *ram, *buffer, *line_buffer, *line_buffer_prev;
	UINT32 rom_cur_address, prot_cur_address;
	UINT16 subkey, dec_hist;
	UINT32 dec_header;
	bool enc_ready;

	int buffer_pos, line_buffer_pos, line_buffer_size, buffer_bit;

	struct sbox {
		UINT8 table[64];
		int inputs[6];      // positions of the inputs bits, -1 means no input except from key
		int outputs[2];     // positions of the output bits
	};

	static const sbox fn1_sboxes[4][4];
	static const sbox fn2_sboxes[4][4];

	static const int fn1_game_key_scheduling[38][2];
	static const int fn2_game_key_scheduling[34][2];
	static const int fn1_sequence_key_scheduling[20][2];
	static const int fn2_sequence_key_scheduling[16];
	static const int fn2_middle_result_scheduling[16];

	static const UINT8 trees[9][2][32];

	int feistel_function(int input, const struct sbox *sboxes, UINT32 subkeys);
	UINT16 block_decrypt(UINT32 game_key, UINT16 sequence_key, UINT16 counter, UINT16 data);

	UINT16 get_decrypted_16();
	int get_compressed_bit();

	void enc_start();
	void enc_fill();
	void line_fill();
};

extern const device_type NAOMI_M2_BOARD;

#endif
