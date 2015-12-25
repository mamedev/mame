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

    The Zorro-III is a multiplexed Zorro-II bus with address- and
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

#pragma once

#ifndef __ZORRO_H__
#define __ZORRO_H__

#include "emu.h"

//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define EXP_SLOT_TAG "exp"
#define ZORROBUS_TAG "zorrobus"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ZORRO_SLOT_ADD(_zorrotag, _tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, ZORRO_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	zorro_slot_device::set_zorro_slot(*device, owner, _zorrotag);

// ======================> expansion slot

#define MCFG_EXPANSION_SLOT_ADD(_cputag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(EXP_SLOT_TAG, EXP_SLOT, 0) \
	zorro_device::set_cputag(*device, _cputag); \
	MCFG_ZORRO_SLOT_ADD(EXP_SLOT_TAG, "slot", _slot_intf, _def_slot)

// callbacks
#define MCFG_EXPANSION_SLOT_OVR_HANDLER(_devcb) \
	devcb = &zorro_device::set_ovr_handler(*device, DEVCB_##_devcb);

#define MCFG_EXPANSION_SLOT_INT2_HANDLER(_devcb) \
	devcb = &zorro_device::set_int2_handler(*device, DEVCB_##_devcb);

#define MCFG_EXPANSION_SLOT_INT6_HANDLER(_devcb) \
	devcb = &zorro_device::set_int6_handler(*device, DEVCB_##_devcb);

#define MCFG_EXPANSION_SLOT_IPL_HANDLER(_devcb) \
	devcb = &exp_slot_device::set_ipl_handler(*device, DEVCB_##_devcb);

// ======================> zorro 2 bus

#define MCFG_ZORRO2_ADD(_cputag) \
	MCFG_DEVICE_ADD(ZORROBUS_TAG, ZORRO2, 0) \
	zorro_device::set_cputag(*device, _cputag);

#define MCFG_ZORRO2_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_ZORRO_SLOT_ADD(ZORROBUS_TAG, _tag, _slot_intf, _def_slot)

#define MCFG_ZORRO2_OVR_HANDLER(_devcb) \
	devcb = &zorro_device::set_ovr_handler(*device, DEVCB_##_devcb);

#define MCFG_ZORRO2_INT2_HANDLER(_devcb) \
	devcb = &zorro_device::set_int2_handler(*device, DEVCB_##_devcb);

#define MCFG_ZORRO2_INT6_HANDLER(_devcb) \
	devcb = &zorro_device::set_int6_handler(*device, DEVCB_##_devcb);

#define MCFG_ZORRO2_EINT1_HANDLER(_devcb) \
	devcb = &zorro2_device::set_eint1_handler(*device, DEVCB_##_devcb);

#define MCFG_ZORRO2_EINT4_HANDLER(_devcb) \
	devcb = &zorro2_device::set_eint4_handler(*device, DEVCB_##_devcb);

#define MCFG_ZORRO2_EINT5_HANDLER(_devcb) \
	devcb = &zorro2_device::set_eint5_handler(*device, DEVCB_##_devcb);

#define MCFG_ZORRO2_EINT7_HANDLER(_devcb) \
	devcb = &zorro2_device::set_eint7_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration of card interfaces
class device_zorro_card_interface;
class device_exp_card_interface;
class device_zorro2_card_interface;

// ======================> zorro slot device

class zorro_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	zorro_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	zorro_slot_device(const machine_config &mconfig, device_type type, const char *name,
		const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	static void set_zorro_slot(device_t &device, device_t *owner, const char *zorro_tag);

protected:
	// configuration
	const char *m_zorro_tag;
};

// device type definition
extern const device_type ZORRO_SLOT;

// ======================> base zorro bus device

class zorro_device : public device_t
{
public:
	// construction/destruction
	zorro_device(const machine_config &mconfig, device_type type, const char *name,
		const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// static configuration helpers
	static void set_cputag(device_t &device, const char *tag);

	template<class _Object> static devcb_base &set_int2_handler(device_t &device, _Object object)
		{ return downcast<zorro_device &>(device).m_int2_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_int6_handler(device_t &device, _Object object)
		{ return downcast<zorro_device &>(device).m_int6_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_ovr_handler(device_t &device, _Object object)
		{ return downcast<zorro_device &>(device).m_ovr_handler.set_callback(object); }

	virtual void add_card(device_zorro_card_interface *card) = 0;

	// interface (from slot device)
	virtual DECLARE_WRITE_LINE_MEMBER( cfgout_w ) {};

	DECLARE_WRITE_LINE_MEMBER( int2_w );
	DECLARE_WRITE_LINE_MEMBER( int6_w );
	DECLARE_WRITE_LINE_MEMBER( ovr_w );

	// interface (from host)
	virtual void fc_w(int code) = 0;

	// access to the host space
	address_space *m_space;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	const char *m_cputag;

	devcb_write_line m_ovr_handler;
	devcb_write_line m_int2_handler;
	devcb_write_line m_int6_handler;
};

// ======================> expansion slot device

class exp_slot_device : public zorro_device
{
public:
	// construction/destruction
	exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	exp_slot_device(const machine_config &mconfig, device_type type, const char *name,
		const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	template<class _Object> static devcb_base &set_ipl_handler(device_t &device, _Object object)
		{ return downcast<exp_slot_device &>(device).m_ipl_handler.set_callback(object); }

	// the expansion slot can only have a single card
	virtual void add_card(device_zorro_card_interface *card) override;

	// interface (from slot device)
	void ipl_w(int interrupt);

	// interface (from host)
	virtual void fc_w(int code) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write8 m_ipl_handler;

	device_exp_card_interface *m_dev;
};

// device type definition
extern const device_type EXP_SLOT;

// ======================> zorro2 slot device

class zorro2_device : public zorro_device
{
public:
	// construction/destruction
	zorro2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	zorro2_device(const machine_config &mconfig, device_type type, const char *name,
		const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~zorro2_device();

	template<class _Object> static devcb_base &set_eint1_handler(device_t &device, _Object object)
		{ return downcast<zorro2_device &>(device).m_eint1_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_eint4_handler(device_t &device, _Object object)
		{ return downcast<zorro2_device &>(device).m_eint4_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_eint5_handler(device_t &device, _Object object)
		{ return downcast<zorro2_device &>(device).m_eint5_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_eint7_handler(device_t &device, _Object object)
		{ return downcast<zorro2_device &>(device).m_eint7_handler.set_callback(object); }

	// the zorro2 bus supports multiple cards
	virtual void add_card(device_zorro_card_interface *card) override;

	// interface (from slot device)
	virtual DECLARE_WRITE_LINE_MEMBER( cfgout_w ) override;

	DECLARE_WRITE_LINE_MEMBER( eint1_w );
	DECLARE_WRITE_LINE_MEMBER( eint4_w );
	DECLARE_WRITE_LINE_MEMBER( eint5_w );
	DECLARE_WRITE_LINE_MEMBER( eint7_w );

	// interface (from host)
	virtual void fc_w(int code) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_eint1_handler;
	devcb_write_line m_eint4_handler;
	devcb_write_line m_eint5_handler;
	devcb_write_line m_eint7_handler;

	simple_list<device_zorro2_card_interface> m_dev;

	// the device which is currently configuring
	device_zorro2_card_interface *m_autoconfig_device;
};

// device type definition
extern const device_type ZORRO2;


// ======================> base zorro card interface

class device_zorro_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_zorro_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_zorro_card_interface();

	virtual void set_zorro_device() = 0;

	void set_zorro_bus(zorro_device *device);

	// interface (from device)
	void cfgout_w(int state) { m_zorro->cfgout_w(state); }

	// interface (from host)
	virtual void fc_w(int code);
	virtual DECLARE_WRITE_LINE_MEMBER( cfgin_w );

protected:
	zorro_device *m_zorro;
};

// ======================> expansion slot card interface

class device_exp_card_interface : public device_zorro_card_interface
{
public:
	// construction/destruction
	device_exp_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_exp_card_interface();

	virtual void set_zorro_device() override;

protected:
	exp_slot_device *m_slot;
};

// ======================> zorro2 card interface

class device_zorro2_card_interface : public device_zorro_card_interface
{
public:
	// construction/destruction
	device_zorro2_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_zorro2_card_interface();

	virtual void set_zorro_device() override;

	device_zorro2_card_interface *next() const { return m_next; }
	device_zorro2_card_interface *m_next;

protected:
	zorro2_device *m_slot;
};


// include this here so that you don't need to include it into every
// driver that uses zorro slots
#include "cards.h"


#endif // __ZORRO_H__
