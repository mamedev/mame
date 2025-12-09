// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    86-pin expansion slot (A500, A1000)
    Coprocessor slot (A2000, B2000)

     2  Ground           1  Ground
     4  Ground           3  Ground
     6  +5VDC            5  +5VDC
     8  -5VDC            7  N/C
    10  +12VDC           9  N/C *1
    12  CFGIN           11  N/C *2
    14  /C3 Clock       13  Ground
    16  /C1 Clock       15  CDAC
    18  RDY             17  /OVR
    20  N/C *3          19  /INT2
    22  /INT6           21  A5
    24  A4              23  A6
    26  A3              25  Ground
    28  A7              27  A2
    30  A8              29  A1
    32  A9              31  FC0
    34  A10             33  FC1
    36  A11             35  FC2
    38  A12             37  Ground
    40  /IPL0           39  A13
    42  /IPL1           41  A14
    44  /IPL2           43  A15
    46  /BERR           45  A16
    48  /VPA            47  A17
    50  E Clock         49  Ground
    52  A18             51  /VMA
    54  A19             53  /RST
    56  A20             55  /HLT
    58  A21             57  A22
    60  /BR *4          59  A23
    62  /BGACK          61  Ground
    64  /BG *5          63  D15
    66  /DTACK          65  D14
    68  R/W             67  D13
    70  /LDS            69  D12
    72  /UDS            71  D11
    74  /AS             73  Ground
    76  D10             75  D0
    78  D9              77  D1
    80  D8              79  D2
    82  D7              81  D3
    84  D6              83  D4
    86  D5              85  Ground

    *1  28 MHz Clock on A2000 and B2000
    *2  /COPCFG on B2000
    *3  /PALOPE on A1000, /BOSS on B2000
    *4  /CBR on B2000
    *5  /CBG on B2000

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CPUSLOT_CPUSLOT_H
#define MAME_BUS_AMIGA_CPUSLOT_CPUSLOT_H

#pragma once


// forward declaration
class device_amiga_cpuslot_interface;


//**************************************************************************
//  DEVICE
//**************************************************************************

class amiga_cpuslot_device : public device_t, public device_single_card_slot_interface<device_amiga_cpuslot_interface>
{
public:
	template <typename T>
	amiga_cpuslot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt) :
		amiga_cpuslot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	amiga_cpuslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration helpers
	template <class T>
	void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }

	auto cfgout_cb() { return m_cfgout_cb.bind(); }
	auto ovr_cb() { return m_ovr_cb.bind(); }
	auto int2_cb() { return m_int2_cb.bind(); }
	auto int6_cb() { return m_int6_cb.bind(); }
	auto ipl7_cb() { return m_ipl7_cb.bind(); }

	// from host
	void cfgin_w(int state);
	void rst_w(int state);

	// from slot device
	void int2_w(int state) { m_int2_cb(state); }
	void int6_w(int state) { m_int6_cb(state); }
	void ovr_w(int state) { m_ovr_cb(state); }
	void cfgout_w(int state) { m_cfgout_cb(state); }
	void ipl7_w(int state) { m_ipl7_cb(state); }

	address_space &space() const { return *m_space; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	device_amiga_cpuslot_interface *m_card;

	required_address_space m_space;

	devcb_write_line m_cfgout_cb;
	devcb_write_line m_ovr_cb;
	devcb_write_line m_int2_cb;
	devcb_write_line m_int6_cb;
	devcb_write_line m_ipl7_cb;
};


//**************************************************************************
//  INTERFACE
//**************************************************************************

class device_amiga_cpuslot_interface : public device_interface
{
public:
	// interface (from host)
	virtual void cfgin_w(int state) { }
	virtual void rst_w(int state) { }

protected:
	device_amiga_cpuslot_interface(const machine_config &mconfig, device_t &device);

	amiga_cpuslot_device *m_host;
};


// device type declarations
DECLARE_DEVICE_TYPE(AMIGA_CPUSLOT, amiga_cpuslot_device)


// include this here so that you don't need to include it into every driver that uses zorro slots
#include "cards.h"


#endif // MAME_BUS_AMIGA_CPUSLOT_CPUSLOT_H
