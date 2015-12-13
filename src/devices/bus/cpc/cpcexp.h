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

#pragma once

#ifndef CPCEXP_H_
#define CPCEXP_H_

#include "emu.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define CPC_EXP_SLOT_TAG        "cpcexp"

enum
{
	MAP_LOWER = 0,  // special lower ROM handling
	MAP_UPPER,      // special upper ROM handling
	MAP_OTHER       // custom ROM handling (eg: Brunword MK4)
};

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CPC_EXPANSION_SLOT_OUT_IRQ_CB(_devcb) \
	devcb = &cpc_expansion_slot_device::set_out_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_CPC_EXPANSION_SLOT_OUT_NMI_CB(_devcb) \
	devcb = &cpc_expansion_slot_device::set_out_nmi_callback(*device, DEVCB_##_devcb);

#define MCFG_CPC_EXPANSION_SLOT_OUT_RESET_CB(_devcb) \
	devcb = &cpc_expansion_slot_device::set_out_reset_callback(*device, DEVCB_##_devcb);

#define MCFG_CPC_EXPANSION_SLOT_OUT_ROMDIS_CB(_devcb) \
	devcb = &cpc_expansion_slot_device::set_out_romdis_callback(*device, DEVCB_##_devcb);

#define MCFG_CPC_EXPANSION_SLOT_ROM_SELECT(_devcb) \
	devcb = &cpc_expansion_slot_device::set_out_rom_select_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_cpc_expansion_card_interface

// class representing interface-specific live cpc_expansion card
class device_cpc_expansion_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_cpc_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_cpc_expansion_card_interface();

	// reset
	virtual void cpc_reset_w() { };
	virtual WRITE_LINE_MEMBER( cursor_w ) { };
	virtual WRITE_LINE_MEMBER( romen_w ) { };

	void set_rom_bank(UINT8 sel) { m_rom_sel = sel; }  // tell device the currently selected ROM
	UINT8 get_rom_bank() { return m_rom_sel; }
	virtual void set_mapping(UINT8 type) { };

private:
	UINT8 m_rom_sel;  // currently selected ROM
};


// ======================> cpc_expansion_slot_device

class cpc_expansion_slot_device : public device_t,
									public device_slot_interface
{
public:
	// construction/destruction
	cpc_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~cpc_expansion_slot_device();

	template<class _Object> static devcb_base &set_out_irq_callback(device_t &device, _Object object) { return downcast<cpc_expansion_slot_device &>(device).m_out_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_nmi_callback(device_t &device, _Object object) { return downcast<cpc_expansion_slot_device &>(device).m_out_nmi_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_reset_callback(device_t &device, _Object object) { return downcast<cpc_expansion_slot_device &>(device).m_out_reset_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_romdis_callback(device_t &device, _Object object) { return downcast<cpc_expansion_slot_device &>(device).m_out_romdis_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rom_select_callback(device_t &device, _Object object) { return downcast<cpc_expansion_slot_device &>(device).m_out_rom_select.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );
	DECLARE_WRITE_LINE_MEMBER( romdis_w );
	DECLARE_WRITE8_MEMBER( rom_select );

	void set_rom_bank(UINT8 sel) { if(m_card) m_card->set_rom_bank(sel); }  // tell device the currently selected ROM
	void set_mapping(UINT8 type) { if(m_card) m_card->set_mapping(type); }  // tell device to enable any ROM or RAM mapping
	DECLARE_WRITE_LINE_MEMBER( cursor_w ) { if(m_card) m_card->cursor_w(state); }  // pass on CRTC Cursor signal
	DECLARE_WRITE_LINE_MEMBER( romen_w ) { if(m_card) m_card->romen_w(state); }  // pass on /ROMEN signal

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_reset_cb;
	devcb_write_line    m_out_romdis_cb;
	devcb_write8        m_out_rom_select;

	device_cpc_expansion_card_interface *m_card;

};


// device type definition
extern const device_type CPC_EXPANSION_SLOT;

#endif /* CPCEXP_H_ */
