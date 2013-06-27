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
 */

#pragma once

#ifndef CPCEXP_H_
#define CPCEXP_H_

#include "emu.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define CPC_EXP_SLOT_TAG        "cpcexp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define CPC_EXPANSION_INTERFACE(_name) \
	const cpc_expansion_slot_interface (_name) =


#define MCFG_CPC_EXPANSION_SLOT_ADD(_tag, _config, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, CPC_EXPANSION_SLOT, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// expansion slot interface

struct cpc_expansion_slot_interface
{
	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_reset_cb;
	devcb_write_line    m_out_romdis_cb;
	devcb_write_line    m_out_romen_cb;
};


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
};


// ======================> cpc_expansion_slot_device

class cpc_expansion_slot_device : public device_t,
									public cpc_expansion_slot_interface,
									public device_slot_interface
{
public:
	// construction/destruction
	cpc_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~cpc_expansion_slot_device();

	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );
	DECLARE_WRITE_LINE_MEMBER( romdis_w );
	DECLARE_WRITE_LINE_MEMBER( romen_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	devcb_resolved_write_line   m_out_irq_func;
	devcb_resolved_write_line   m_out_nmi_func;
	devcb_resolved_write_line   m_out_reset_func;
	devcb_resolved_write_line   m_out_romdis_func;
	devcb_resolved_write_line   m_out_romen_func;

	device_cpc_expansion_card_interface *m_card;
};


// device type definition
extern const device_type CPC_EXPANSION_SLOT;

#endif /* CPCEXP_H_ */
