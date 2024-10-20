// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 Expansion Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       C1 LOW
                    +5V       3      C       _BRESET
                   _IRQ       4      D       _RAS
                   R/_W       5      E       phi0
                C1 HIGH       6      F       A15
                 C2 LOW       7      H       A14
                C2 HIGH       8      J       A13
                   _CS1       9      K       A12
                   _CS0      10      L       A11
                   _CAS      11      M       A10
                    MUX      12      N       A9
                     BA      13      P       A8
                     D7      14      R       A7
                     D6      15      S       A6
                     D5      16      T       A5
                     D4      17      U       A4
                     D3      18      V       A3
                     D2      19      W       A2
                     D1      20      X       A1
                     D0      21      Y       A0
                    AEC      22      Z       N.C. (RAMEN)
              EXT AUDIO      23      AA      N.C.
                   phi2      24      BB      N.C.
                    GND      25      CC      GND

**********************************************************************/

#ifndef MAME_BUS_PLUS4_EXP_H
#define MAME_BUS_PLUS4_EXP_H

#pragma once

#include "imagedev/cartrom.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> plus4_expansion_slot_device

class device_plus4_expansion_card_interface;

class plus4_expansion_slot_device : public device_t,
									public device_single_card_slot_interface<device_plus4_expansion_card_interface>,
									public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	plus4_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: plus4_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	plus4_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_wr_callback() { return m_write_irq.bind(); }
	auto cd_rd_callback() { return m_read_dma_cd.bind(); }
	auto cd_wr_callback() { return m_write_dma_cd.bind(); }
	auto aec_wr_callback() { return m_write_aec.bind(); }

	// computer interface
	uint8_t cd_r(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h);
	void cd_w(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h);

	// cartridge interface
	uint8_t dma_cd_r(offs_t offset) { return m_read_dma_cd(offset); }
	void dma_cd_w(offs_t offset, uint8_t data) { m_write_dma_cd(offset, data); }
	void irq_w(int state) { m_write_irq(state); }
	void aec_w(int state) { m_write_aec(state); }
	int phi2() { return clock(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "plus4_cart"; }
	virtual const char *file_extensions() const noexcept override { return "rom,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	devcb_write_line   m_write_irq;
	devcb_read8        m_read_dma_cd;
	devcb_write8       m_write_dma_cd;
	devcb_write_line   m_write_aec;

	device_plus4_expansion_card_interface *m_card;
};


// ======================> device_plus4_expansion_card_interface

class device_plus4_expansion_card_interface : public device_interface
{
	friend class plus4_expansion_slot_device;

public:
	// construction/destruction
	virtual ~device_plus4_expansion_card_interface();

	// runtime
	virtual uint8_t plus4_cd_r(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) { return data; }
	virtual void plus4_cd_w(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) { }

protected:
	device_plus4_expansion_card_interface(const machine_config &mconfig, device_t &device);

	std::unique_ptr<uint8_t[]> m_c1l;
	std::unique_ptr<uint8_t[]> m_c1h;
	std::unique_ptr<uint8_t[]> m_c2l;
	std::unique_ptr<uint8_t[]> m_c2h;

	size_t m_c1l_size;
	size_t m_c1h_size;
	size_t m_c2l_size;
	size_t m_c2h_size;

	plus4_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(PLUS4_EXPANSION_SLOT, plus4_expansion_slot_device)


void plus4_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_PLUS4_EXP_H
