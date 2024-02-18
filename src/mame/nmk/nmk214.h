// license:BSD-3-Clause
// copyright-holders:Sergio Galiano
#ifndef MAME_NMK_NMK214_H
#define MAME_NMK_NMK214_H

#pragma once


class nmk214_device : public device_t
{
public:
	nmk214_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_mode(const u8 mode) { m_mode = mode; }
	void set_input_address_bitswap(const std::array<u8, 13> &input_address_bitswap) { m_input_address_bitswap = input_address_bitswap; }

	void set_init_config(u8 init_config);
	bool is_device_initialized() { return m_device_initialized; }

	u16 decode_word(u32 addr, u16 data);
	u8 decode_byte(u32 addr, u8 data);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// Operation mode. In practice, only LSB is used. Allowed values: 0 or 1. This value is hardwired for each NM214 out of 2 ones in the
	// game board, each device having a different configuration than the other.
	u8 m_mode;

	// Input address lines bitswap. These represents the way the 13 address lines on NMK214 (from A0 to A12) are hooked up externally
	// related to the GFX ROMs address bus, that could be in different order/position
	std::array<u8, 13> m_input_address_bitswap;

	// An init config out of 8 existing ones in NMK214 to be used while descrambling GFX.
	// In practice, only lower 3 bits (From 0 to 2) are used, allowed values: 0 to 7.
	// Bit 3 is used to match with operation mode and let the init config to be stored on the device or not.
	u8 m_init_config;

	// Flag to mark the device already received the init configuration and it can perform descrambling from now on.
	bool m_device_initialized;


	// Gets the value to select the output bitswap to apply to input data based on the address where the data is located
	u8 get_bitswap_select_value(u32 addr);  
};

DECLARE_DEVICE_TYPE(NMK214, nmk214_device)

#endif // MAME_NMK_NMK214_H
