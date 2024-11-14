// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Colecovision Expansion Slot

    60-pin slot

     1  GND     60  -5V
     2  GND     59  +5V
     3  D3      58  +5V
     4  A14     57  +12V
     5  EN40    56  AUDOUT
     6  EN20    55  IORQ
     7  HALT    54  MREQ
     8  WR      53  RD
     9  NMI     52  BUSACK
    10  FX-9    51  INT
    11  BUSREQ  50  WAIT
    12  D1      49  RFSH
    13  RESET   48  D5
    14  D0      47  A0
    15  M1      46  D5
    16  D2      45  CPUCLK
    17  D6      44  A3
    18  A1      43  A15
    19  D4      42  N/C
    20  A2      41  N/C
    21  A4      40  CPUCLK
    22  A13     39  EXVDCRST
    23  A5      38  A12
    24  A8      37  A11
    25  A7      36  EXRES2
    26  A8      35  AUDCLK
    27  A9      34  EXRES1
    28  A10     33  EXVID
    29  N/C     32  EXVIDEN
    30  N/C     31  EXAUD

***************************************************************************/

#ifndef MAME_BUS_COlECO_EXPANSION_EXPANSION_H
#define MAME_BUS_COlECO_EXPANSION_EXPANSION_H

#pragma once

// include here so drivers don't need to
#include "cards.h"

// forward declaration
class device_coleco_expansion_interface;


//**************************************************************************
//  BUS DEVICE
//**************************************************************************

class coleco_expansion_device :
	public device_t,
	public device_single_card_slot_interface<device_coleco_expansion_interface>,
	public device_mixer_interface
{
public:
	// construction/destruction
	coleco_expansion_device(machine_config const &mconfig, char const *tag, device_t *owner, const char *dflt) :
		coleco_expansion_device(mconfig, tag, owner, uint32_t(0))
	{
		option_reset();
		coleco_expansion_cards(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	coleco_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~coleco_expansion_device();

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	// called from card device
	void int_w(int state) { m_int_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }

	address_space &mem_space() { return *m_program; }
	address_space &io_space() { return *m_io; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	required_address_space m_program;
	required_address_space m_io;

	devcb_write_line m_int_handler;
	devcb_write_line m_nmi_handler;

	device_coleco_expansion_interface *m_card;
};


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

class device_coleco_expansion_interface : public device_interface
{
public:
	virtual ~device_coleco_expansion_interface();

protected:
	device_coleco_expansion_interface(const machine_config &mconfig, device_t &device);

	coleco_expansion_device *m_expansion;
};


// device type declaration
DECLARE_DEVICE_TYPE(COLECO_EXPANSION, coleco_expansion_device)


#endif // MAME_BUS_COlECO_EXPANSION_EXPANSION_H
