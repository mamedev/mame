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

#ifndef MAME_BUS_BW2_EXP_H
#define MAME_BUS_BW2_EXP_H

#pragma once




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bw2_expansion_slot_device

class device_bw2_expansion_slot_interface;

class bw2_expansion_slot_device : public device_t,
									public device_single_card_slot_interface<device_bw2_expansion_slot_interface>
{
public:
	// construction/destruction
	template <typename T>
	bw2_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: bw2_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	bw2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~bw2_expansion_slot_device();

	// computer interface
	uint8_t cd_r(offs_t offset, uint8_t data, int ram2, int ram3, int ram4, int ram5, int ram6);
	void cd_w(offs_t offset, uint8_t data, int ram2, int ram3, int ram4, int ram5, int ram6);

	uint8_t slot_r(offs_t offset);
	void slot_w(offs_t offset, uint8_t data);

	uint8_t modsel_r(offs_t offset);
	void modsel_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	device_bw2_expansion_slot_interface *m_cart;
};


// ======================> device_bw2_expansion_slot_interface

// class representing interface-specific live bw2_expansion card
class device_bw2_expansion_slot_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_bw2_expansion_slot_interface();

	virtual uint8_t bw2_cd_r(offs_t offset, uint8_t data, int ram2, int ram3, int ram4, int ram5, int ram6) { return data; }
	virtual void bw2_cd_w(offs_t offset, uint8_t data, int ram2, int ram3, int ram4, int ram5, int ram6) { }

	virtual uint8_t bw2_slot_r(offs_t offset) { return 0xff; }
	virtual void bw2_slot_w(offs_t offset, uint8_t data) { }

	virtual uint8_t bw2_modsel_r(offs_t offset) { return 0xff; }
	virtual void bw2_modsel_w(offs_t offset, uint8_t data) { }

protected:
	device_bw2_expansion_slot_interface(const machine_config &mconfig, device_t &device);

	bw2_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BW2_EXPANSION_SLOT, bw2_expansion_slot_device)


void bw2_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_BW2_EXP_H
