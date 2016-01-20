// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Andreas Naive
#ifndef _AWBOARD_H_
#define _AWBOARD_H_

#include "naomig1.h"

#define MCFG_AW_ROM_BOARD_ADD(_tag, _keyregion, _irq_cb)  \
	MCFG_NAOMI_G1_ADD(_tag, AW_ROM_BOARD, _irq_cb)        \
	aw_rom_board::static_set_keyregion(*device, _keyregion);

class aw_rom_board : public naomi_g1_device
{
public:
	aw_rom_board(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void static_set_keyregion(device_t &device, const char *_keyregion);

	DECLARE_ADDRESS_MAP(submap, 16);

	DECLARE_WRITE16_MEMBER(epr_offsetl_w);          // 5f7000
	DECLARE_WRITE16_MEMBER(epr_offseth_w);          // 5f7004
	DECLARE_WRITE16_MEMBER(mpr_record_index_w);     // 5f700c
	DECLARE_WRITE16_MEMBER(mpr_first_file_index_w); // 5f7010
	DECLARE_WRITE16_MEMBER(mpr_file_offsetl_w);     // 5f7014
	DECLARE_WRITE16_MEMBER(mpr_file_offseth_w);     // 5f7018
	DECLARE_READ16_MEMBER(pio_r);                   // 5f7080
	DECLARE_WRITE16_MEMBER(pio_w);                  // 5f7080

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void dma_get_position(UINT8 *&base, UINT32 &limit, bool to_mainram) override;
	virtual void dma_advance(UINT32 size) override;

private:
	enum { EPR, MPR_RECORD, MPR_FILE };

	const char *keyregion;
	UINT32 rombd_key;
	UINT32 mpr_offset, mpr_bank;
	UINT32 epr_offset, mpr_file_offset;
	UINT16 mpr_record_index, mpr_first_file_index;
	UINT16 decrypted_buf[16];

	UINT32 dma_offset, dma_limit;

	struct sbox_set {
		UINT8 S0[32];
		UINT8 S1[16];
		UINT8 S2[16];
		UINT8 S3[8];
	};

	static const int permutation_table[4][16];
	static const sbox_set sboxes_table[4];
	static UINT16 decrypt(UINT16 cipherText, UINT32 address, const UINT32 key);
	UINT16 decrypt16(UINT32 address) { return decrypt(m_region->u16(address), address, rombd_key); }

	void set_key();
	void recalc_dma_offset(int mode);
};

extern const device_type AW_ROM_BOARD;

#endif
