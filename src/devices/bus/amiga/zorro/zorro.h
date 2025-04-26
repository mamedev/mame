// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Amiga Zorro-II Slot

     2  Ground           1  Ground
     4  Ground           3  Ground
     6  +5VDC            5  +5VDC
     8  -5VDC            7  /OWN
    10  +12VDC           9  /SLAVEn
    12  CFGINn          11  CFGOUTn
    14  /C3 Clock       13  Ground
    16  /C1 Clock       15  CDAC
    18  XRDY            17  /OVR
    20  -12V            19  /INT2
    22  /INT6           21  A5
    24  A4              23  A6
    26  A3              25  Ground
    28  A7              27  A2
    30  A8              29  A1
    32  A9              31  FC0
    34  A10             33  FC1
    36  A11             35  FC2
    38  A12             37  Ground
    40  /EINT7          39  A13
    42  /EINT5          41  A14
    44  /EINT4          43  A15
    46  /BERR           45  A16
    48  /VPA            47  A17
    50  E Clock         49  Ground
    52  A18             51  /VMA
    54  A19             53  /RST
    56  A20             55  /HLT
    58  A21             57  A22
    60  /BRn            59  A23
    62  /BGACK          61  Ground
    64  /BGn            63  D15
    66  /DTACK          65  D14
    68  READ            67  D13
    70  /LDS            69  D12
    72  /UDS            71  D11
    74  /AS             73  Ground
    76  D10             75  D0
    78  D9              77  D1
    80  D8              79  D2
    82  D7              81  D3
    84  D6              83  D4
    86  D5              85  Ground
    87  Ground          88  Ground
    90  Ground          89  Ground
    92  7MHz            91  Ground
    94  /BUSRST         93  DOE
    96  /EINT1          95  /(C)BG
    98  N/C             97  N/C
   100  Ground          99  Ground

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_ZORRO_H
#define MAME_BUS_AMIGA_ZORRO_ZORRO_H

#pragma once

#include <functional>
#include <utility>
#include <vector>


// forward declaration of card interfaces
class device_zorro2_card_interface;


//**************************************************************************
//  BUS DEVICE
//**************************************************************************

class zorro2_bus_device : public device_t, public device_memory_interface
{
public:
	zorro2_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto eint1_handler() { return m_eint1_handler.bind(); }
	auto eint4_handler() { return m_eint4_handler.bind(); }
	auto eint5_handler() { return m_eint5_handler.bind(); }
	auto eint7_handler() { return m_eint7_handler.bind(); }
	auto int2_handler() { return m_int2_handler.bind(); }
	auto int6_handler() { return m_int6_handler.bind(); }
	auto ovr_handler() { return m_ovr_handler.bind(); }

	// device_memory_interface
	virtual space_config_vector memory_space_config() const override;

	virtual void add_card(device_zorro2_card_interface &card) ATTR_COLD;

	// interface (from slot device)
	void cfgout_w(int state);
	void eint1_w(int state);
	void eint4_w(int state);
	void eint5_w(int state);
	void eint7_w(int state);
	void int2_w(int state);
	void int6_w(int state);
	void ovr_w(int state);

	// interface (from host)
	uint16_t mem_r(offs_t offset, uint16_t mem_mask);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t io_r(offs_t offset, uint16_t mem_mask);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	void fc_w(int code);
	void busrst_w(int state);

	// access to the zorro2 space
	address_space &space() const { return device_memory_interface::space(AS_PROGRAM); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	using card_vector = std::vector<std::reference_wrapper<device_zorro2_card_interface>>;
	card_vector m_cards;

	address_space_config m_zorro_space_config;

	devcb_write_line m_eint1_handler;
	devcb_write_line m_eint4_handler;
	devcb_write_line m_eint5_handler;
	devcb_write_line m_eint7_handler;
	devcb_write_line m_int2_handler;
	devcb_write_line m_int6_handler;
	devcb_write_line m_ovr_handler;

	// the device which is currently configuring
	uint8_t m_autoconfig_device;
};

// device type declaration
DECLARE_DEVICE_TYPE(ZORRO2_BUS, zorro2_bus_device)


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

class zorro2_slot_device : public device_t, public device_single_card_slot_interface<device_zorro2_card_interface>
{
public:
	zorro2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	zorro2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt) :
		zorro2_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

protected:
	zorro2_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(ZORRO2_SLOT, zorro2_slot_device)


//**************************************************************************
//  INTERFACE
//**************************************************************************

class device_zorro2_card_interface : public device_interface
{
public:
	device_zorro2_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_zorro2_card_interface();

	void set_bus(zorro2_bus_device &device) ATTR_COLD;

	// interface (from device)
	void cfgout_w(int state) { m_zorro->cfgout_w(state); }

	// interface (from host)
	virtual void fc_w(int code);
	virtual void cfgin_w(int state);
	virtual void busrst_w(int state);

protected:
	zorro2_bus_device *m_zorro;
};


// include this here so that you don't need to include it into every driver that uses zorro slots
#include "cards.h"


#endif // MAME_BUS_AMIGA_ZORRO_ZORRO_H
