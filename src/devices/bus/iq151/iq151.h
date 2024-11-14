// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    iq151cart.h

    IQ151 cartridge slot pinout:

                     +--------+
        +12V         | 01  32 |   IO     /MW
        +12V         | 02  33 |   IO     /IOR
        +5V          | 03  34 |   IO     /IOW
        +5V          | 04  35 |    O     /NRDY
        GND          | 05  36 |    O     /HOLD
        GND          | 06  37 |   I      HLDA
        A0      IO   | 07  38 |    O     /RAM
        A1      IO   | 08  39 |    O     /INT0
        A2      IO   | 09  40 |    O     /INT1
        A3      IO   | 10  41 |    O     /INT2
        A4      IO   | 11  42 |    O     /INT3
        A5      IO   | 12  43 |    O     /INT4
        A6      IO   | 13  44 |    O     /VID
        A7      IO   | 14  45 |   I       OSC
        A8      IO   | 15  46 |   I       TTL
        A9      IO   | 16  47 |    O      NF
        A10     IO   | 17  48 |           N.C.
        A11     IO   | 18  49 |           N.C.
        A12     IO   | 19  50 |   I       /INIT
        A13     IO   | 20  51 |   IO      /SS
        A14     IO   | 21  52 |   IO      /SR
        A15     IO   | 22  53 |           N.C.
        D0      IO   | 23  54 |   IO      /ZS
        D1      IO   | 24  55 |   IO      /ZR
        D2      IO   | 25  56 |    O      /DMA
        D3      IO   | 26  57 |           GND
        D4      IO   | 27  58 |           GND
        D5      IO   | 28  59 |           -5V
        D6      IO   | 29  60 |           -5V
        D7      IO   | 30  61 |           -12V
        /MR     IO   | 31  62 |           -12V
                     +--------+

*********************************************************************/

#ifndef MAME_BUS_IQ151_IQ151_H
#define MAME_BUS_IQ151_IQ151_H

#pragma once

#include "imagedev/cartrom.h"
#include "screen.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> device_iq151cart_interface

class device_iq151cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_iq151cart_interface();

	// reading and writing
	virtual void read(offs_t offset, uint8_t &data) { }
	virtual void write(offs_t offset, uint8_t data) { }
	virtual void io_read(offs_t offset, uint8_t &data) { }
	virtual void io_write(offs_t offset, uint8_t data) { }
	virtual uint8_t* get_cart_base() { return nullptr; }
	virtual void set_screen_device(screen_device &screen) { m_screen = &screen; }

	// video update
	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect) { }

protected:
	device_iq151cart_interface(const machine_config &mconfig, device_t &device);

	screen_device *m_screen;
};

// ======================> iq151cart_slot_device

class iq151cart_slot_device : public device_t,
								public device_single_card_slot_interface<device_iq151cart_interface>,
								public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	iq151cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: iq151cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	iq151cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~iq151cart_slot_device();

	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	auto out_irq0_callback() { return m_out_irq0_cb.bind(); }
	auto out_irq1_callback() { return m_out_irq1_cb.bind(); }
	auto out_irq2_callback() { return m_out_irq2_cb.bind(); }
	auto out_irq3_callback() { return m_out_irq3_cb.bind(); }
	auto out_irq4_callback() { return m_out_irq4_cb.bind(); }
	auto out_drq_callback() { return m_out_drq_cb.bind(); }

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "iq151_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	virtual void read(offs_t offset, uint8_t &data);
	virtual void write(offs_t offset, uint8_t data);
	virtual void io_read(offs_t offset, uint8_t &data);
	virtual void io_write(offs_t offset, uint8_t data);
	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	devcb_write_line                m_out_irq0_cb;
	devcb_write_line                m_out_irq1_cb;
	devcb_write_line                m_out_irq2_cb;
	devcb_write_line                m_out_irq3_cb;
	devcb_write_line                m_out_irq4_cb;
	devcb_write_line                m_out_drq_cb;

	device_iq151cart_interface*     m_cart;
	required_device<screen_device>  m_screen;
};


// device type definition
DECLARE_DEVICE_TYPE(IQ151CART_SLOT, iq151cart_slot_device)

#endif // MAME_BUS_IQ151_IQ151_H
