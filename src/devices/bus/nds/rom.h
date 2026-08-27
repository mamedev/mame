// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_NDS_ROM_H
#define MAME_BUS_NDS_ROM_H

#pragma once

#include "ndsslot.h"


// ======================> nds_rom_device

class nds_rom_device : public device_t,
						public device_nds_cart_interface
{
public:
	nds_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_nds_cart_interface implementation
	virtual void command_start(const uint8_t *cmd, uint32_t length) override;
	virtual uint32_t data_r() override;
	virtual uint8_t spi_transfer(uint8_t data) override;
	virtual void spi_deselect() override;

protected:
	nds_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum : uint8_t
	{
		MODE_RAW = 0,   // unencrypted commands, used by the BIOS before the KEY1 handshake
		MODE_KEY1,      // commands arrive Blowfish encrypted
		MODE_MAIN       // "main data mode": KEY2, which is transparent here
	};

	uint32_t idcode() const;
	uint32_t chip_id() const;
	void key1_init(uint32_t idcode, int level, int modulo);
	void key1_apply_keycode(uint32_t *keycode, int modulo);
	void key1_encrypt(uint32_t *data) const;
	void key1_decrypt(uint32_t *data) const;
	void key1_encrypt_block(uint8_t *block) const;
	void prepare_secure_area();

	void respond_rom(uint32_t addr, uint32_t mask);
	void respond_fixed(uint32_t value);

	uint32_t m_key1_buf[0x412];
	uint8_t m_mode;
	bool m_fixed_response;
	uint32_t m_fixed_value;
	uint32_t m_data_addr;
	uint32_t m_data_mask;
	uint32_t m_remaining;
	bool m_secure_area_ready;

	// backup chip on the AUXSPI bus
	static constexpr uint32_t BACKUP_SIZE = 0x100000;
	uint8_t m_bk_cmd;
	uint32_t m_bk_count;        // bytes seen in the current transaction, including the command
	uint32_t m_bk_addr;
	uint8_t m_bk_addr_bytes;
	bool m_bk_wel;
	bool m_bk_dirty;
};


// device type declaration
DECLARE_DEVICE_TYPE(NDS_ROM_STD, nds_rom_device)

#endif // MAME_BUS_NDS_ROM_H
