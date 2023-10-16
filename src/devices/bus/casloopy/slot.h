// license:BSD-3-Clause
// copyright-holders:Phil Bennett
#ifndef MAME_BUS_CASLOOPY_SLOT_H
#define MAME_BUS_CASLOOPY_SLOT_H

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> device_casloopy_cart_interface

class device_casloopy_cart_interface : public device_interface
{
public:
	// construction/destruction
	device_casloopy_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_casloopy_cart_interface();

	// reading and writing
	virtual uint16_t read_rom(offs_t offset) { return 0xffff; }
	virtual uint8_t read_ram(offs_t offset) { return 0xff; }
	virtual void write_ram(offs_t offset, u8 data) { }

	void rom_alloc(uint32_t size);
	void nvram_alloc(uint32_t size);
	uint16_t* get_rom_base() const { return m_rom; }
	uint32_t get_rom_size() const { return m_rom_size; }

	uint8_t* get_nvram_base() { return &m_nvram[0]; }
	uint32_t get_nvram_size() { return m_nvram.size(); }

protected:
	uint16_t *m_rom;
	uint32_t m_rom_size;

	std::vector<uint8_t> m_nvram;
};


// ======================> casloopy_cart_slot_device

class casloopy_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_casloopy_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	casloopy_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: casloopy_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	casloopy_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~casloopy_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "casloopy_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	uint16_t read_rom(offs_t offset);
	uint8_t read_ram(offs_t offset);
	void write_ram(offs_t offset, u8 data);

protected:
	// device_t implementation
	virtual void device_start() override;

	device_casloopy_cart_interface *m_cart;
};

DECLARE_DEVICE_TYPE(CASLOOPY_CART_SLOT, casloopy_cart_slot_device)

#endif // MAME_BUS_CASLOOPY_SLOT_H
