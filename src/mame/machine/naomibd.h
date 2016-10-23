// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef _NAOMIBD_H_
#define _NAOMIBD_H_

#include "machine/naomig1.h"
#include "machine/x76f100.h"

#define MCFG_NAOMI_BOARD_ADD(_tag, type, _eeprom_tag, _irq_cb)    \
	MCFG_NAOMI_G1_ADD(_tag, type, _irq_cb)                        \
	naomi_board::static_set_eeprom_tag(*device, _eeprom_tag);

class naomi_board : public naomi_g1_device
{
public:
	naomi_board(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	static void static_set_eeprom_tag(device_t &device, const char *_eeprom_tag);

	// Can be patched in the underlying class
	virtual DECLARE_ADDRESS_MAP(submap, 16);

	void rom_offseth_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);          // 5f7000
	void rom_offsetl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);          // 5f7004
	uint16_t rom_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);             // 5f7008
	void rom_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);             // 5f7008
	void dma_offseth_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);          // 5f700c
	void dma_offsetl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);          // 5f7010
	void dma_count_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);            // 5f7014

	void boardid_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);              // 5f7078
	uint16_t boardid_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);              // 5f707c

	uint16_t default_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void dma_get_position(uint8_t *&base, uint32_t &limit, bool to_mainram) override;
	virtual void dma_advance(uint32_t size) override;

	// To be defined in the underlying class
	virtual void board_setup_address(uint32_t address, bool is_dma) = 0;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) = 0;
	virtual void board_advance(uint32_t size) = 0;

	// To be optionally defined in the underlying class
	virtual void board_write(offs_t offset, uint16_t data);

	uint32_t rom_offset;
private:
	uint32_t dma_offset, dma_cur_offset;
	uint16_t dma_count;
	bool pio_ready, dma_ready;

	const char *eeprom_tag;
	x76f100_device *eeprom;
};

#endif
