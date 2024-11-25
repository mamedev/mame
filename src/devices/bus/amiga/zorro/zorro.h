// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Zorro Slots

    86-pin Expansion Slot (Zorro-I), Zorro-II, Zorro-III

    86-pin Expansion Slot

     2  Ground           1  Ground
     4  Ground           3  Ground
     6  +5VDC            5  +5VDC
     8  -5VDC            7  N/C
    10  +12VDC           9  N/C *1
    12  CFGIN           11  N/C *2
    14  /C3 Clock       13  Ground
    16  /C1 Clock       15  CDAC
    18  XRDY            17  /OVR
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
    46  /BEER           45  A16
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

    Zorro-II (differences only)

     7  /OWN
     9  /SLAVEn
    11  /CFGOUTn
    12  /CFGINn
    20  -12VDC
    40  /EINT7
    42  /EINT5
    44  /EINT4
    60  /BRn
    64  /BGn

    88  Ground          87  Ground
    90  Ground          89  Ground
    92  7 MHz           91  Ground
    94  /BURST          93  DOE
    96  /EINT1          95  /BG *5
    98  N/C             97  N/C
   100  Ground          99  Ground

    *6  /GBG on B2000

    Zorro-III

    The Zorro-III is a multiplexed Zorro-II bus with address and
    data phases. Signals changes as follows:

    17  /CINH
    18  /MTCR
    29  /LOCK
    30  AD8 (D0)
    32  AD9 (D1)
    34  AD10 (D2)
    36  AD11 (D3)
    38  AD12 (D4)
    39  AD13 (D5)
    40  Reserved
    41  AD14 (D6)
    42  Reserved
    43  AD15 (D7)
    44  Reserved
    45  AD16 (D8)
    47  AD17 (D9)
    48  /MTACK
    51  /DS0
    52  AD18 (D10)
    54  AD19 (D11)
    56  AD20 (D12)
    57  AD22 (D14)
    58  AD21 (D13)
    59  AD23 (D15)
    63  AD31
    65  AD30
    67  AD29
    69  AD28
    70  /DS2
    71  AD27
    72  /DS3
    74  /CCS
    75  SD0 (D16)
    76  AD26
    77  SD1 (D17)
    78  AD25
    79  SD2 (D18)
    80  AD24
    81  SD3 (D19)
    82  SD7 (D23)
    83  SD4 (D20)
    84  SD6 (D22)
    86  SD5 (D21)
    91  Sense Z3
    94  /IORST
    95  /BCLR
    97  /FCS
    98  /DS1


***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_ZORRO_H
#define MAME_BUS_AMIGA_ZORRO_ZORRO_H

#pragma once

#include <functional>
#include <utility>
#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration of card interfaces
class device_zorro_card_interface;
class device_exp_card_interface;
class device_zorro2_card_interface;
class zorro_bus_device_base;

// ======================> zorro slot device

class zorro_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	zorro_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T, typename O>
	zorro_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&zorrotag, O &&opts, const char *dflt)
		: zorro_slot_device(mconfig, tag, owner, 0)
	{
		set_zorro_slot(std::forward<T>(zorrotag));
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	// inline configuration
	template <class T> void set_zorro_slot(T &&zorro_tag) { m_zorro_bus.set_tag(zorro_tag); }

protected:
	zorro_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<zorro_bus_device_base> m_zorro_bus;
};

// device type definition
DECLARE_DEVICE_TYPE(ZORRO_SLOT, zorro_slot_device)

// ======================> base zorro bus device

class zorro_bus_device_base : public device_t
{
public:
	// configuration helpers
	template <class T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }

	auto int2_handler() { return m_int2_handler.bind(); }
	auto int6_handler() { return m_int6_handler.bind(); }
	auto ovr_handler() { return m_ovr_handler.bind(); }

	virtual void add_card(device_zorro_card_interface &card) ATTR_COLD = 0;

	// interface (from slot device)
	virtual void cfgout_w(int state) { }

	void int2_w(int state);
	void int6_w(int state);
	void ovr_w(int state);

	// interface (from host)
	virtual void fc_w(int code) = 0;

	// access to the host space
	address_space &space() const { return *m_space; }

protected:
	// construction/destruction
	zorro_bus_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	required_address_space m_space;

	devcb_write_line m_ovr_handler;
	devcb_write_line m_int2_handler;
	devcb_write_line m_int6_handler;
};

// ======================> expansion slot device

class exp_slot_device : public zorro_bus_device_base
{
public:
	// construction/destruction
	exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto ipl_handler() { return m_ipl_handler.bind(); }

	// the expansion slot can only have a single card
	virtual void add_card(device_zorro_card_interface &card) override ATTR_COLD;

	// interface (from slot device)
	void ipl_w(int interrupt);

	// interface (from host)
	virtual void fc_w(int code) override;

protected:
	exp_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write8 m_ipl_handler;

	device_exp_card_interface *m_dev;
};

// device type definition
DECLARE_DEVICE_TYPE(EXP_SLOT, exp_slot_device)

// ======================> zorro2 slot device

class zorro2_bus_device : public zorro_bus_device_base
{
public:
	// construction/destruction
	zorro2_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~zorro2_bus_device();

	auto eint1_handler() { return m_eint1_handler.bind(); }
	auto eint4_handler() { return m_eint4_handler.bind(); }
	auto eint5_handler() { return m_eint5_handler.bind(); }
	auto eint7_handler() { return m_eint7_handler.bind(); }

	// the zorro2 bus supports multiple cards
	virtual void add_card(device_zorro_card_interface &card) override ATTR_COLD;

	// interface (from slot device)
	virtual void cfgout_w(int state) override;

	void eint1_w(int state);
	void eint4_w(int state);
	void eint5_w(int state);
	void eint7_w(int state);

	// interface (from host)
	virtual void fc_w(int code) override;

protected:
	zorro2_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	using card_vector = std::vector<std::reference_wrapper<device_zorro2_card_interface> >;

	devcb_write_line m_eint1_handler;
	devcb_write_line m_eint4_handler;
	devcb_write_line m_eint5_handler;
	devcb_write_line m_eint7_handler;

	card_vector m_dev;

	// the device which is currently configuring
	uint8_t m_autoconfig_device;
};

// device type definition
DECLARE_DEVICE_TYPE(ZORRO2, zorro2_bus_device)


// ======================> base zorro card interface

class device_zorro_card_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_zorro_card_interface();

	void set_zorro_bus(zorro_bus_device_base &device) ATTR_COLD;

	// interface (from device)
	void cfgout_w(int state) { m_zorro->cfgout_w(state); }

	// interface (from host)
	virtual void fc_w(int code);
	virtual void cfgin_w(int state);

protected:
	device_zorro_card_interface(const machine_config &mconfig, device_t &device);

	zorro_bus_device_base *m_zorro;
};

// ======================> expansion slot card interface

class device_exp_card_interface : public device_zorro_card_interface
{
public:
	// construction/destruction
	virtual ~device_exp_card_interface();

protected:
	device_exp_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override ATTR_COLD;

	exp_slot_device *m_slot;
};

// ======================> zorro2 card interface

class device_zorro2_card_interface : public device_zorro_card_interface
{
public:
	// construction/destruction
	virtual ~device_zorro2_card_interface();

protected:
	device_zorro2_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override ATTR_COLD;

	zorro2_bus_device *m_slot;
};


// include this here so that you don't need to include it into every driver that uses zorro slots
#include "cards.h"


#endif // MAME_BUS_AMIGA_ZORRO_ZORRO_H
