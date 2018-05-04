// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC analogue port emulation

**********************************************************************

  Pinout:

            +5V   1
                      9  LPSTB
             0V   2
                     10  PB1
             0V   3
                     11  VREF
            CH3   4
                     12  CH2
   Analogue GND   5
                     13  PB0
             0V   6
                     14  VREF
            CH1   7
                     15  CH0
   Analogue GND   8

**********************************************************************/

#ifndef MAME_BUS_BBC_ANALOGUE_ANALOGUE_H
#define MAME_BUS_BBC_ANALOGUE_ANALOGUE_H

#pragma once



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define BBC_ANALOGUE_SLOT_TAG      "analogue"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BBC_ANALOGUE_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, BBC_ANALOGUE_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_analogue_slot_device

class device_bbc_analogue_interface;

class bbc_analogue_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	bbc_analogue_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ch_r(int channel);
	uint8_t pb_r();

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	device_bbc_analogue_interface *m_card;
};


// ======================> device_bbc_analogue_interface

class device_bbc_analogue_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_bbc_analogue_interface();

	virtual uint8_t ch_r(int channel) { return 0x00; };
	virtual uint8_t pb_r() { return 0x30; };

protected:
	device_bbc_analogue_interface(const machine_config &mconfig, device_t &device);

	bbc_analogue_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_ANALOGUE_SLOT, bbc_analogue_slot_device)

void bbc_analogue_devices(device_slot_interface &device);

#endif // MAME_BUS_BBC_ANALOGUE_ANALOGUE_H
