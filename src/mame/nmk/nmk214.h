// license:BSD-3-Clause
// copyright-holders:Sergio Galiano
#ifndef MAME_NMK_NMK214_H
#define MAME_NMK_NMK214_H

#pragma once

#include <array>


class nmk214_device : public device_t
{
public:
	nmk214_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_mode(const u8 mode) { m_mode = mode; }
	void set_input_address_bitswap(const std::array<u8, 13> &input_address_bitswap) { m_input_address_bitswap = input_address_bitswap; }

	void set_init_config(u8 init_config) noexcept;
	bool is_device_initialized() const noexcept { return m_device_initialized; }

	u16 decode_word(u32 addr, u16 data) const noexcept;
	u8 decode_byte(u32 addr, u8 data) const noexcept;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// Operation mode - in practice, only LSB is used.
	// Allowed values: 0 or 1.
	// This is hard-wired on the PCB, with opposite values for the two devices.
	u8 m_mode;

	// Input address lines bitswap.
	// Represents how the 13 NMK214 address lines are wired to the graphics ROM address bus.
	std::array<u8, 13> m_input_address_bitswap;

	// Selects between eight internal configurations.
	// Bits 0 to 2 select the configuration.
	// Bit 3 must be set to match the operation mode for the configuration to take effect.
	u8 m_init_config;

	// Indicates that the device has been configured and can perform descrambling.
	bool m_device_initialized;


	// Gets the bitswap index for a given data address.
	u8 get_bitswap_select_value(u32 addr) const noexcept;
};

DECLARE_DEVICE_TYPE(NMK214, nmk214_device)

#endif // MAME_NMK_NMK214_H
