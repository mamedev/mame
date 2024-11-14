// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpcexp.h  --  Amstrad CPC Expansion port
 *
 *  Created on: 16/07/2011
 *
 *  Pinout from CPC6128 User's Manual
 *    SOUND   1  2   GND
 *      A15   3  4   A14
 *      A13   5  6   A12
 *      A11   7  8   A10
 *       A9   9  10  A8
 *       A7  11  12  A6
 *       A5  13  14  A4
 *       A3  15  16  A2
 *       A1  17  18  A0
 *       D7  19  20  D6
 *       D5  21  22  D4
 *       D3  23  24  D2
 *       D1  25  26  D0
 *      +5v  27  28  _MREQ
 *      _M1  29  30  _RFSH
 *    _IORQ  31  32  _RD
 *      _WR  33  34  _HALT
 *     _INT  35  36  _NMI
 *   _BUSR2  37  38  _BUSAK
 *    READY  39  40  _BUS RESET
 *   _RESET  41  42  _ROMEN
 *   ROMDIS  43  44  _RAMRD
 *   RAMDIS  45  46  CURSOR
 *    L.PEN  47  48  _EXP
 *      GND  49  50  CLOCK
 *
 *  Aleste 520EX expansion port is 62-pin.  Same as the CPC above, except that pin 40 is not connected, plus the following:
 *
 *    MAP14  A26  B26  MAP15
 *    MAP16  A27  B27  MAP17
 *    MAP18  A28  B28  MAPBLK
 *    _INTA  A29  B29  _DISP
 *     Agnd  A30  B30  _CPU
 *     Aucc  A31  B31  HIGH
 *
 */

#ifndef MAME_BUS_CPC_CPCEXP_H
#define MAME_BUS_CPC_CPCEXP_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_cpc_expansion_card_interface

// class representing interface-specific live cpc_expansion card
class device_cpc_expansion_card_interface : public device_interface
{
public:
	enum
	{
		MAP_LOWER = 0,  // special lower ROM handling
		MAP_UPPER,      // special upper ROM handling
		MAP_OTHER       // custom ROM handling (eg: Brunword MK4)
	};

	// construction/destruction
	virtual ~device_cpc_expansion_card_interface();

	// reset
	virtual void cpc_reset_w() { }
	virtual void cursor_w(int state) { }
	virtual void romen_w(int state) { }

	void set_rom_bank(uint8_t sel) { m_rom_sel = sel; }  // tell device the currently selected ROM
	uint8_t get_rom_bank() { return m_rom_sel; }
	virtual void set_mapping(uint8_t type) { }

protected:
	device_cpc_expansion_card_interface(const machine_config &mconfig, device_t &device);

private:
	uint8_t m_rom_sel;  // currently selected ROM
};


// ======================> cpc_expansion_slot_device

class cpc_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_cpc_expansion_card_interface>
{
public:
	// construction/destruction
	cpc_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T>
	cpc_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: cpc_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	virtual ~cpc_expansion_slot_device();

	template <class Object> void set_cpu_tag(Object &&tag) { m_cpu.set_tag(std::forward<Object>(tag)); }

	auto irq_callback() { return m_out_irq_cb.bind(); }
	auto nmi_callback() { return m_out_nmi_cb.bind(); }
	auto reset_callback() { return m_out_reset_cb.bind(); }
	auto romdis_callback() { return m_out_romdis_cb.bind(); }
	auto rom_select_callback() { return m_out_rom_select.bind(); }

	void irq_w(int state);
	void nmi_w(int state);
	void reset_w(int state);
	void romdis_w(int state);
	void rom_select(uint8_t data);

	void set_rom_bank(uint8_t sel) { if(m_card) m_card->set_rom_bank(sel); }  // tell device the currently selected ROM
	void set_mapping(uint8_t type) { if(m_card) m_card->set_mapping(type); }  // tell device to enable any ROM or RAM mapping
	void cursor_w(int state) { if(m_card) m_card->cursor_w(state); }  // pass on CRTC Cursor signal
	void romen_w(int state) { if(m_card) m_card->romen_w(state); }  // pass on /ROMEN signal

	cpu_device &cpu() const { return *m_cpu; }

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

	required_device<cpu_device> m_cpu;

	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_reset_cb;
	devcb_write_line    m_out_romdis_cb;
	devcb_write8        m_out_rom_select;

	device_cpc_expansion_card_interface *m_card;

};


// device type definition
DECLARE_DEVICE_TYPE(CPC_EXPANSION_SLOT, cpc_expansion_slot_device)

#endif // MAME_BUS_CPC_CPCEXP_H
