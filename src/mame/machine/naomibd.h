// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef _NAOMIBD_H_
#define _NAOMIBD_H_

#include "machine/naomig1.h"

#define MCFG_NAOMI_BOARD_ADD(_tag, type, _eeprom_tag, _actel_tag, _irq_cb)    \
	MCFG_NAOMI_G1_ADD(_tag, type, _irq_cb)                        \
	naomi_board::static_set_eeprom_tag(*device, _eeprom_tag, _actel_tag);

class naomi_board : public naomi_g1_device
{
public:
	naomi_board(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	static void static_set_eeprom_tag(device_t &device, const char *_eeprom_tag, const char *_actel_tag);

	// Can be patched in the underlying class
	virtual DECLARE_ADDRESS_MAP(submap, 16);

	DECLARE_WRITE16_MEMBER(rom_offseth_w);          // 5f7000
	DECLARE_WRITE16_MEMBER(rom_offsetl_w);          // 5f7004
	DECLARE_READ16_MEMBER( rom_data_r);             // 5f7008
	DECLARE_WRITE16_MEMBER(rom_data_w);             // 5f7008
	DECLARE_WRITE16_MEMBER(dma_offseth_w);          // 5f700c
	DECLARE_WRITE16_MEMBER(dma_offsetl_w);          // 5f7010
	DECLARE_WRITE16_MEMBER(dma_count_w);            // 5f7014

	DECLARE_WRITE16_MEMBER(boardid_w);              // 5f7078
	DECLARE_READ16_MEMBER( boardid_r);              // 5f707c

	DECLARE_READ16_MEMBER( default_r);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void dma_get_position(UINT8 *&base, UINT32 &limit, bool to_mainram) override;
	virtual void dma_advance(UINT32 size) override;

	// To be defined in the underlying class
	virtual void board_setup_address(UINT32 address, bool is_dma) = 0;
	virtual void board_get_buffer(UINT8 *&base, UINT32 &limit) = 0;
	virtual void board_advance(UINT32 size) = 0;

	// To be optionally defined in the underlying class
	virtual void board_write(offs_t offset, UINT16 data);

	UINT32 rom_offset;
	const char *rombdid_tag;
private:
	UINT32 dma_offset, dma_cur_offset;
	UINT16 dma_count;
	bool pio_ready, dma_ready;

	const char *eeprom_tag;
	class x76f100_device *eeprom;
};

#endif
