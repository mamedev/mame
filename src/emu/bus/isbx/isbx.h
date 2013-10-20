// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel iSBX bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                              1      2       
                    GND       3      4       +5V
                  RESET       5      6       
                    MA2       7      8       MPST/
                    MA1       9      10      
                    MA0      11      12      MINTR1
                 /IOWRT      13      14      
                  /IORD      15      16      MWAIT/
                    GND      17      18      +5V
                    MD7      19      20      MCS1/
                    MD6      21      22      MCS0/
                    MD5      23      24      
                    MD4      25      26      TDMA
                    MD3      27      28      OPT1
                    MD2      29      30      OPT0
                    MD1      31      32      MDACK/
                    MD0      33      34      MDRQT
                    GND      35      36      +5V
                    MDE      37      38      MDF
                    MDC      39      40      MDD
                    MDA      41      42      MDB
                    MD8      43      44      MD9

**********************************************************************/

#pragma once

#ifndef __ISBX_SLOT__
#define __ISBX_SLOT__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ISBX_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, ISBX_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isbx_slot_device

class device_isbx_card_interface;

class isbx_slot_device : public device_t,
						 public device_slot_interface
{
public:
	// construction/destruction
	isbx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	device_isbx_card_interface *m_card;
};


// ======================> device_isbx_card_interface

class device_isbx_card_interface : public device_slot_card_interface
{
	friend class isbx_slot_device;

public:
	// construction/destruction
	device_isbx_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_isbx_card_interface();

protected:
	isbx_slot_device *m_slot;
};


// device type definition
extern const device_type ISBX_SLOT;


// slot devices
SLOT_INTERFACE_EXTERN( isbx_cards );



#endif
