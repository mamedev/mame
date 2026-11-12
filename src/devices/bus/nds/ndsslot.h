// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Nintendo DS game card slot
	Based on gba_slot by Ryan Holtz and Fabio Priuli

***************************************************************************/
#ifndef MAME_BUS_NDS_NDSSLOT_H
#define MAME_BUS_NDS_NDSSLOT_H

#pragma once

#include "imagedev/cartrom.h"


// ======================> device_nds_cart_interface

class device_nds_cart_interface : public device_interface
{
public:
	virtual ~device_nds_cart_interface();

	// an 8 byte command has been latched and 'length' bytes of response are expected
	virtual void command_start(const uint8_t *cmd, uint32_t length) { }
	virtual uint32_t data_r() { return 0xffffffff; }

	// the backup chip on the AUXSPI bus
	virtual uint8_t spi_transfer(uint8_t data) { return 0xff; }
	virtual void spi_deselect() { }

	// KEY1 needs the Blowfish tables at offset 0x30 of the ARM7 BIOS
	void set_key1_table(const uint8_t *table) { m_key1_table = table; }

	void rom_alloc(uint32_t size, const char *tag);
	uint8_t *get_rom_base() { return m_rom; }
	uint32_t get_rom_size() const { return m_rom_size; }
	uint8_t *get_nvram_base() { return m_nvram.empty() ? nullptr : &m_nvram[0]; }
	uint32_t get_nvram_size() const { return m_nvram.size(); }

protected:
	device_nds_cart_interface(const machine_config &mconfig, device_t &device);

	void nvram_alloc(uint32_t size) { m_nvram.resize(size); }

	uint8_t *m_rom;
	uint32_t m_rom_size;
	const uint8_t *m_key1_table;
	std::vector<uint8_t> m_nvram;
};


// ======================> nds_cart_slot_device

class nds_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_nds_cart_interface>
{
public:
	template <typename T>
	nds_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: nds_cart_slot_device(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}

	nds_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~nds_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "nds_cart"; }
	virtual const char *file_extensions() const noexcept override { return "nds,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	void set_key1_table(const uint8_t *table) { if (m_cart) m_cart->set_key1_table(table); }

	void command_start(const uint8_t *cmd, uint32_t length) { if (m_cart) m_cart->command_start(cmd, length); }
	uint32_t data_r() { return m_cart ? m_cart->data_r() : 0xffffffff; }
	uint8_t spi_transfer(uint8_t data) { return m_cart ? m_cart->spi_transfer(data) : 0xff; }
	void spi_deselect() { if (m_cart) m_cart->spi_deselect(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	device_nds_cart_interface *m_cart;
};


// device type declaration
DECLARE_DEVICE_TYPE(NDS_CART_SLOT, nds_cart_slot_device)

#define NDSSLOT_ROM_REGION_TAG ":cart:rom"

#endif // MAME_BUS_NDS_NDSSLOT_H
