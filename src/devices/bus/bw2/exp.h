// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Bondwell 2 Expansion Port emulation

**********************************************************************

                     5V       1      26      12V
                     D3       2      27      12V
                     A1       3      28      A3
                     A2       4      29      A4
                  _CTSB       5      30      A5
                   _RST       6      31      A6
                _MODSEL       7      32      A7
                  16MHZ       8      33      A8
                  _IORQ       9      34      A9
                    _RD      10      35      A10
                     D0      11      36      A11
                     D1      12      37      A12
                     D2      13      38      A13
                     A0      14      39      A14
                     D4      15      40      _RAM6
                     D5      16      41      _RAM5
                     D6      17      42      _RFSH
                     D7      18      43      _WR
                   DCDB      19      44      SELECT
                  _DTRB      20      45      _RAM2
                  _RTSB      21      46      _RAM3
                  _DSRB      22      47      _RAM4
                   TXDB      23      48      _SLOT
                   RXDB      24      49      GND
                    GND      25      50      5V

**********************************************************************/

#pragma once

#ifndef __BW2_EXPANSION_SLOT__
#define __BW2_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define BW2_EXPANSION_SLOT_TAG      "exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BW2_EXPANSION_SLOT_ADD(_tag, _clock, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, BW2_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bw2_expansion_slot_device

class device_bw2_expansion_slot_interface;

class bw2_expansion_slot_device : public device_t,
									public device_slot_interface
{
public:
	// construction/destruction
	bw2_expansion_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~bw2_expansion_slot_device();

	// computer interface
	UINT8 cd_r(address_space &space, offs_t offset, UINT8 data, int ram2, int ram3, int ram4, int ram5, int ram6);
	void cd_w(address_space &space, offs_t offset, UINT8 data, int ram2, int ram3, int ram4, int ram5, int ram6);

	DECLARE_READ8_MEMBER( slot_r );
	DECLARE_WRITE8_MEMBER( slot_w );

	DECLARE_READ8_MEMBER( modsel_r );
	DECLARE_WRITE8_MEMBER( modsel_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	device_bw2_expansion_slot_interface *m_cart;
};


// ======================> device_bw2_expansion_slot_interface

// class representing interface-specific live bw2_expansion card
class device_bw2_expansion_slot_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_bw2_expansion_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_bw2_expansion_slot_interface();

	virtual UINT8 bw2_cd_r(address_space &space, offs_t offset, UINT8 data, int ram2, int ram3, int ram4, int ram5, int ram6) { return data; };
	virtual void bw2_cd_w(address_space &space, offs_t offset, UINT8 data, int ram2, int ram3, int ram4, int ram5, int ram6) { };

	virtual UINT8 bw2_slot_r(address_space &space, offs_t offset) { return 0xff; }
	virtual void bw2_slot_w(address_space &space, offs_t offset, UINT8 data) { }

	virtual UINT8 bw2_modsel_r(address_space &space, offs_t offset) { return 0xff; }
	virtual void bw2_modsel_w(address_space &space, offs_t offset, UINT8 data) { }

protected:
	bw2_expansion_slot_device *m_slot;
};


// device type definition
extern const device_type BW2_EXPANSION_SLOT;


SLOT_INTERFACE_EXTERN( bw2_expansion_cards );



#endif
