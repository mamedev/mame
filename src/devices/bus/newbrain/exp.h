// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain Expansion Port emulation

**********************************************************************

                    GND       1      26      A6
                   1/8C       2      27      _RAMENB
                    A14       3      28      EXRM2
                    A13       4      29      EXRM1
                     D5       5      30      EXRM0
                   RMSL       6      31      _ROMOV
                     D4       7      32      _BUSRQ
                     D3       8      33      _M1
                     D6       9      34      _RST
                     D7      10      35      _RFRSH
                    A11      11      36      _WAIT
                    A10      12      37      A4
                     A8      13      38      _BUSAK
                     A9      14      39      A15
                    A12      15      40      _WR
                     A7      16      41      _INT
                     A3      17      42      _RD
                     A2      18      43      _NMI
                     A1      19      44      _HALT
                     A0      20      45      _MREQ
                     D0      21      46      _IORQ
                     D1      22      47      PRTOV
                  _FCTR      23      48      _RAMINH
                     D2      24      49      +5V
                     A5      25      50      .

**********************************************************************/

#pragma once

#ifndef __NEWBRAIN_EXPANSION_SLOT__
#define __NEWBRAIN_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define NEWBRAIN_EXPANSION_SLOT_TAG      "exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NEWBRAIN_EXPANSION_SLOT_ADD(_tag, _clock, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, NEWBRAIN_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> newbrain_expansion_slot_t

class device_newbrain_expansion_slot_interface;

class newbrain_expansion_slot_t : public device_t,
									public device_slot_interface
{
public:
	// construction/destruction
	newbrain_expansion_slot_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~newbrain_expansion_slot_t() { }

	// computer interface
	UINT8 mreq_r(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh);
	void mreq_w(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh);

	UINT8 iorq_r(address_space &space, offs_t offset, UINT8 data, bool &prtov);
	void iorq_w(address_space &space, offs_t offset, UINT8 data, bool &prtov);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	device_newbrain_expansion_slot_interface *m_card;
};


// ======================> device_newbrain_expansion_slot_interface

// class representing interface-specific live newbrain_expansion card
class device_newbrain_expansion_slot_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_newbrain_expansion_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_newbrain_expansion_slot_interface() { }

	// memory access
	virtual UINT8 mreq_r(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh) { return data; };
	virtual void mreq_w(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh) { };

	// I/O access
	virtual UINT8 iorq_r(address_space &space, offs_t offset, UINT8 data, bool &prtov) { return data; };
	virtual void iorq_w(address_space &space, offs_t offset, UINT8 data, bool &prtov) { };

protected:
	newbrain_expansion_slot_t *m_slot;
};


// device type definition
extern const device_type NEWBRAIN_EXPANSION_SLOT;


SLOT_INTERFACE_EXTERN( newbrain_expansion_cards );



#endif
