// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    z88.h

    Z88 cartridge slots emulation

**********************************************************************

    pins    Slot 1  Slot 2  Slot 3

    1       A16     A16     A16
    2       A15     A15     A15
    3       A12     A12     A12
    4       A7      A7      A7
    5       A6      A6      A6
    6       A5      A5      A5
    7       A4      A4      A4
    8       A3      A3      A3
    9       A2      A2      A2
    10      A1      A1      A1
    11      A0      A0      A0
    12      D0      D0      D0
    13      D1      D1      D1
    14      D2      D2      D2
    15      SNSL    SNSL    SNSL
    16      GND     GND     GND
    17      GND     GND     GND
    18      A14     A14     A14
    19      VCC     VCC     VPP
    20      VCC     VCC     VCC
    21      VCC     VCC     VCC
    22      WEL     WEL     PGML
    23      A13     A13     A13
    24      A8      A8      A8
    25      A9      A9      A9
    26      A11     A11     A11
    27      POE     POE     POE
    28      ROE     ROE     EOE
    29      A10     A10     A10
    30      SE1     SE2     SE3
    31      D7      D7      D7
    32      D6      D6      D6
    33      D3      D3      D3
    34      D4      D4      D4
    35      D5      D5      D5
    36      A17     A17     A17
    37      A18     A18     A18
    38      A19     A19     A19

*********************************************************************/

#ifndef MAME_BUS_Z88_Z88_H
#define MAME_BUS_Z88_Z88_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> device_z88cart_interface

class device_z88cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_z88cart_interface();

	// reading and writing
	virtual uint8_t read(offs_t offset) { return 0xff; }
	virtual void write(offs_t offset, uint8_t data) { }
	virtual void vpp_w(int state) { }
	virtual uint8_t* get_cart_base() { return nullptr; }
	virtual uint32_t get_cart_size() { return 0; }

protected:
	device_z88cart_interface(const machine_config &mconfig, device_t &device);
};


// ======================> z88cart_slot_device

class z88cart_slot_device : public device_t,
							public device_cartrom_image_interface,
							public device_single_card_slot_interface<device_z88cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	z88cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: z88cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	z88cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto out_flp_callback() { return m_out_flp_cb.bind(); }

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "z88_cart"; }
	virtual const char *file_extensions() const noexcept override { return "epr,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void vpp_w(int state);
	uint8_t* get_cart_base();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(close_flap);

private:
	devcb_write_line            m_out_flp_cb;
	device_z88cart_interface*   m_cart;
	emu_timer *                 m_flp_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(Z88CART_SLOT, z88cart_slot_device)

#endif // MAME_BUS_Z88_Z88_H
