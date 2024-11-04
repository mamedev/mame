// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_SUPERACAN_SLOT_H
#define MAME_BUS_SUPERACAN_SLOT_H

#include "imagedev/cartrom.h"

#include <system_error>
#include <utility>


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

class superacan_cart_slot_device;


class device_superacan_cart_interface : public device_interface
{
public:
	virtual ~device_superacan_cart_interface();

	// load/unload
	virtual std::error_condition load() = 0;
	virtual void unload() = 0;

	// read/write
	virtual u16 rom_r(offs_t offset) = 0;
	virtual u8 nvram_r(offs_t offset) = 0;
	virtual void nvram_w(offs_t offset, u8 data) = 0;

protected:
	// construction/destruction
	device_superacan_cart_interface(const machine_config &mconfig, device_t &device);

	// helpers for slot stuff
	void battery_load(void *buffer, int length, int fill);
	void battery_load(void *buffer, int length, void *def_buffer);
	void battery_save(const void *buffer, int length);

private:
	superacan_cart_slot_device *const m_slot;
};


class superacan_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_superacan_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	superacan_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, T &&opts, char const *dflt)
		: superacan_cart_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	superacan_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~superacan_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "superacan_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	u16 rom_r(offs_t offset) { return m_cart ? m_cart->rom_r(offset) : 0xffff; }
	u8 nvram_r(offs_t offset) { return m_cart ? m_cart->nvram_r(offset) : 0xff; }
	void nvram_w(offs_t offset, u8 data) { if (m_cart) m_cart->nvram_w(offset, data); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	device_superacan_cart_interface *m_cart;
};

DECLARE_DEVICE_TYPE(SUPERACAN_CART_SLOT, superacan_cart_slot_device)

#endif // MAME_BUS_SUPERACAN_SLOT_H
