// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    International Memories Incorporated IMI 7000 Series bus emulation

**********************************************************************

                    GND       1      2       GND
                  SPARE       3      4       SPARE
       -SEL UNIT ADDR 3       5      6       -SEL UNIT ADDR 2
              +R/W DATA       7      8       -R/W DATA
       -SEL UNIT ADDR 1       9      10      -SEL UNIT ADDR 0
             +SYS CLOCK      11      12      -SYS CLOCK
                    GND      13      14      GND
                -SECTOR      15      16      -INDEX
         -SEEK COMPLETE      17      18      -FAULT
            -CMD STROBE      19      20      -CMD R/W
          -CMD SELECT 0      21      22      -CMD SELECT 1
                  SPARE      23      24      SPARE
               -CMD ACK      25      26      SPARE
             -CMD BUS 6      27      28      -CMD BUS 7
             -CMD BUS 4      29      30      -CMD BUS 5
             -CMD BUS 2      31      32      -CMD BUS 3
             -CMD BUS 0      33      34      -CMD BUS 1

**********************************************************************/

#pragma once

#ifndef __IMI7000_BUS__
#define __IMI7000_BUS__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define IMI7000_BUS_TAG      "imi7000"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_IMI7000_BUS_ADD(_def_slot1, _def_slot2, _def_slot3, _def_slot4) \
	MCFG_DEVICE_ADD(IMI7000_BUS_TAG, IMI7000_BUS, 0) \
	MCFG_DEVICE_ADD(IMI7000_BUS_TAG":0", IMI7000_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(imi7000_devices, _def_slot1, false) \
	MCFG_DEVICE_ADD(IMI7000_BUS_TAG":1", IMI7000_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(imi7000_devices, _def_slot2, false) \
	MCFG_DEVICE_ADD(IMI7000_BUS_TAG":2", IMI7000_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(imi7000_devices, _def_slot3, false) \
	MCFG_DEVICE_ADD(IMI7000_BUS_TAG":3", IMI7000_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(imi7000_devices, _def_slot4, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class imi7000_slot_device;
class device_imi7000_interface;


// ======================> imi7000_bus_device

class imi7000_bus_device : public device_t
{
public:
	// construction/destruction
	imi7000_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void add_device(imi7000_slot_device *slot, device_t *target);

protected:
	// device-level overrides
	virtual void device_start() override;

	imi7000_slot_device *m_unit[4];
};


// ======================> imi7000_slot_device

class imi7000_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	imi7000_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	device_imi7000_interface *m_card;
};


// ======================> device_imi7000_interface

class device_imi7000_interface : public device_slot_card_interface
{
	friend class imi7000_slot_device;

public:
	// construction/destruction
	device_imi7000_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_imi7000_interface() { }

protected:
	imi7000_slot_device *m_slot;
};


// device type definition
extern const device_type IMI7000_BUS;
extern const device_type IMI7000_SLOT;


// slot interface
SLOT_INTERFACE_EXTERN( imi7000_devices );



#endif
