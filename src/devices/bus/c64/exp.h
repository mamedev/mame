// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 Expansion Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       _ROMH
                    +5V       3      C       _RESET
                   _IRQ       4      D       _NMI
                  _CR/W       5      E       Sphi2
                 DOTCLK       6      F       CA15
                  _I/O1       7      H       CA14
                  _GAME       8      J       CA13
                 _EXROM       9      K       CA12
                  _I/O2      10      L       CA11
                  _ROML      11      M       CA10
                     BA      12      N       CA9
                   _DMA      13      P       CA8
                    CD7      14      R       CA7
                    CD6      15      S       CA6
                    CD5      16      T       CA5
                    CD4      17      U       CA4
                    CD3      18      V       CA3
                    CD2      19      W       CA2
                    CD1      20      X       CA1
                    CD0      21      Y       CA0
                    GND      22      Z       GND

**********************************************************************/

#ifndef MAME_BUS_C64_EXP_H
#define MAME_BUS_C64_EXP_H

#pragma once

#include "imagedev/cartrom.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_expansion_slot_device

class device_c64_expansion_card_interface;

class c64_expansion_slot_device : public device_t,
									public device_single_card_slot_interface<device_c64_expansion_card_interface>,
									public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	c64_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: c64_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	c64_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return m_write_irq.bind(); }
	auto nmi_callback() { return m_write_nmi.bind(); }
	auto reset_callback() { return m_write_reset.bind(); }
	auto cd_input_callback() { return m_read_dma_cd.bind(); }
	auto cd_output_callback() { return m_write_dma_cd.bind(); }
	auto dma_callback() { return m_write_dma.bind(); }

	// computer interface
	uint8_t cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	void cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	int game_r(offs_t offset, int sphi2, int ba, int rw, int loram, int hiram);
	int exrom_r(offs_t offset, int sphi2, int ba, int rw, int loram, int hiram);

	// cartridge interface
	uint8_t dma_cd_r(offs_t offset) { return m_read_dma_cd(offset); }
	void dma_cd_w(offs_t offset, uint8_t data) { m_write_dma_cd(offset, data); }
	void irq_w(int state) { m_write_irq(state); }
	void nmi_w(int state) { m_write_nmi(state); }
	void dma_w(int state) { m_write_dma(state); }
	void reset_w(int state) { m_write_reset(state); }
	int phi2() { return clock(); }
	int dotclock() { return phi2() * 8; }
	int hiram() { return m_hiram; }
	int loram() { return m_loram; }

	void set_passthrough();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "c64_cart,vic10_cart"; }
	virtual const char *file_extensions() const noexcept override { return "80,a0,e0,crt"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	devcb_read8        m_read_dma_cd;
	devcb_write8       m_write_dma_cd;
	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_nmi;
	devcb_write_line   m_write_dma;
	devcb_write_line   m_write_reset;

	device_c64_expansion_card_interface *m_card;

	int m_hiram;
	int m_loram;
};


// ======================> device_c64_expansion_card_interface

class device_c64_expansion_card_interface : public device_interface
{
	friend class c64_expansion_slot_device;

public:
	// construction/destruction
	virtual ~device_c64_expansion_card_interface();

	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) { return data; }
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) { }
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) { return m_game; }
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) { return m_exrom; }

protected:
	device_c64_expansion_card_interface(const machine_config &mconfig, device_t &device);

	std::unique_ptr<uint8_t[]> m_roml;
	std::unique_ptr<uint8_t[]> m_romh;
	std::unique_ptr<uint8_t[]> m_romx;
	std::unique_ptr<uint8_t[]> m_nvram;

	size_t m_roml_size;
	size_t m_romh_size;

	int m_game;
	int m_exrom;

	c64_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_EXPANSION_SLOT, c64_expansion_slot_device)

void c64_expansion_cards(device_slot_interface &device);

#endif
