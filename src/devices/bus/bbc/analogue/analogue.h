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
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_analogue_slot_device

class device_bbc_analogue_interface;

class bbc_analogue_slot_device : public device_t, public device_single_card_slot_interface<device_bbc_analogue_interface>
{
public:
	// construction/destruction
	template <typename T>
	bbc_analogue_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: bbc_analogue_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	bbc_analogue_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto lpstb_handler() { return m_lpstb_handler.bind(); }

	void lpstb_w(int state) { m_lpstb_handler(state); }

	uint16_t ch_r(offs_t channel);
	uint8_t pb_r();
	void pb_w(uint8_t data);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	device_bbc_analogue_interface *m_card;

private:
	devcb_write_line m_lpstb_handler;
};


// ======================> device_bbc_analogue_interface

class device_bbc_analogue_interface : public device_interface
{
public:
	virtual uint16_t ch_r(offs_t channel) { return 0x00; }
	virtual uint8_t pb_r() { return 0x30; }
	virtual void pb_w(uint8_t data) { }

protected:
	device_bbc_analogue_interface(const machine_config &mconfig, device_t &device);

	bbc_analogue_slot_device *const m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_ANALOGUE_SLOT, bbc_analogue_slot_device)

void bbc_analogue_devices(device_slot_interface &device);
void bbc_analogue_devices_no_lightpen(device_slot_interface &device);

#endif // MAME_BUS_BBC_ANALOGUE_ANALOGUE_H
