// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Bus Device

===========================================================================
 Standard Bus|        Extended Bus        |        RC80 RC2014 Bus
=============|============================|================================
 Pin Signal  |Pin Signal       Pin Signal |Pin #  Signal   Pin Signal
=============|============================|================================
 1   A15     |1   Not used     1   A15    |41  #41 (custom)  1   A15
 2   A14     |2   Not used     2   A14    |42  #42 (custom)  2   A14
 3   A13     |3   Not used     3   A13    |43  #43 (custom)  3   A13
 4   A12     |4   Not used     4   A12    |44  #44 (custom)  4   A12
 5   A11     |5   Not used     5   A11    |45  #45 (custom)  5   A11
 6   A10     |6   Not used     6   A10    |46  #46 (custom)  6   A10
 7   A9      |7   Not used     7   A9     |47  #47 (custom)  7   A9
 8   A8      |8   Not used     8   A8     |48  #48 (custom)  8   A8
 9   A7      |9   Not used     9   A7     |49  A23           9   A7
 10  A6      |10  Not used     10  A6     |50  A22           10  A6
 11  A5      |11  Not used     11  A5     |51  A21           11  A5
 12  A4      |12  Not used     12  A4     |52  A20           12  A4
 13  A3      |13  Not used     13  A3     |53  A19           13  A3
 14  A2      |14  Not used     14  A2     |54  A18           14  A2
 15  A1      |15  Not used     15  A1     |55  A17           15  A1
 16  A0      |16  Not used     16  A0     |56  A16           16  A0
 17  GND     |17  GND          17  GND    |57  GND           17  GND
 18  5V      |18  5V           18  5V     |58  5V            18  5V
 19  /M1     |19  /RFSH        19  /M1    |59  /RFSH         19  /M1
 20  /RESET  |20  PAGE(/RESET2)20  /RESET |60  PAGE          20  /RESET
 21  CLK     |21  CLK2         21  CLK    |61  CLK2          21  CLK
 22  /INT    |22  /BUSAK       22  /INT   |62  /BUSAK        22  /INT
 23  /MREQ   |23  /HALT        23  /MREQ  |63  /HALT         23  /MREQ
 24  /WR     |24  /BUSRQ       24  /WR    |64  /BUSRQ        24  /WR
 25  /RD     |25  /WAIT        25  /RD    |65  /WAIT         25  /RD
 26  /IORQ   |26  /NMI         26  /IORQ  |66  /NMI          26  /IORQ
 27  D0      |27  D8           27  D0     |67  #67 (custom)  27  D0
 28  D1      |28  D9           28  D1     |68  #68 (custom)  28  D1
 29  D2      |29  D10          29  D2     |69  #69 (custom)  29  D2
 30  D3      |30  D11          30  D3     |70  #70 (custom)  30  D3
 31  D4      |31  D12          31  D4     |71  #71 (custom)  31  D4
 32  D5      |32  D13          32  D5     |72  #72 (custom)  32  D5
 33  D6      |33  D14          33  D6     |73  #73 (custom)  33  D6
 34  D7      |34  D15          34  D7     |74  #74 (custom)  34  D7
 35  TX      |35  TX2          35  TX     |75  TX2           35  TX
 36  RX      |36  RX2          36  RX     |76  RX2           36  RX
 37  USER1   |37  USER5        37  USER1  |77  USER5         37  USER1
 38  USER2   |38  USER6        38  USER2  |78  USER6         38  USER2
 39  USER3   |39  USER7        39  USER3  |79  USER7         39  USER3
 40  USER4   |40  USER8        40  USER4  |80  USER8(IEI)    40  USER4(IEO)
===========================================================================

***************************************************************************/

#ifndef MAME_BUS_RC2014_RC2014_H
#define MAME_BUS_RC2014_RC2014_H

#pragma once

#include "machine/z80daisy.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

//**************************************************************************
//  RC2014 Standard Bus
//**************************************************************************

// ======================> rc2014_bus_device

class rc2014_bus_device : public device_t
{
public:
	// construction/destruction
	rc2014_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~rc2014_bus_device();

	auto clk_callback() { return m_clk.bind(); }
	auto int_callback() { return m_int.bind(); }
	auto tx_callback() { return m_tx.bind(); }
	auto rx_callback() { return m_rx.bind(); }
	auto user1_callback() { return m_user1.bind(); }
	auto user2_callback() { return m_user2.bind(); }
	auto user3_callback() { return m_user3.bind(); }
	auto user4_callback() { return m_user4.bind(); }

	DECLARE_WRITE_LINE_MEMBER( clk_w ) { m_clk(state); }
	DECLARE_WRITE_LINE_MEMBER( int_w ) { m_int(state); }
	DECLARE_WRITE_LINE_MEMBER( tx_w ) { m_tx(state); }
	DECLARE_WRITE_LINE_MEMBER( rx_w ) { m_rx(state); }
	DECLARE_WRITE_LINE_MEMBER( user1_w ) { m_user1(state); }
	DECLARE_WRITE_LINE_MEMBER( user2_w ) { m_user2(state); }
	DECLARE_WRITE_LINE_MEMBER( user3_w ) { m_user3(state); }
	DECLARE_WRITE_LINE_MEMBER( user4_w ) { m_user4(state); }

	void set_bus_clock(u32 clock);
	void set_bus_clock(const XTAL &xtal) { set_bus_clock(xtal.value()); }
	void assign_installer(int index, address_space_installer *installer);
	address_space_installer *installer(int index) const;
	void add_to_daisy_chain(std::string tag) { m_daisy.push_back(tag); }
	const z80_daisy_config* get_daisy_chain();

protected:
	rc2014_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	address_space_installer *m_installer[4];
	devcb_write_line m_clk;
	devcb_write_line m_int;
	devcb_write_line m_tx;
	devcb_write_line m_rx;
	devcb_write_line m_user1;
	devcb_write_line m_user2;
	devcb_write_line m_user3;
	devcb_write_line m_user4;
	std::vector<std::string> m_daisy;
	char **m_daisy_chain;
};

// ======================> device_rc2014_card_interface

class rc2014_slot_device;

class device_rc2014_card_interface : public device_interface
{
	friend class rc2014_slot_device;

public:
	// construction/destruction
	device_rc2014_card_interface(const machine_config &mconfig, device_t &device);
protected:
	void set_bus_device(rc2014_bus_device *bus_device);

	rc2014_bus_device  *m_bus;
};

// ======================> rc2014_slot_device

class rc2014_slot_device : public device_t, public device_slot_interface
{
public:
	rc2014_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename T, typename U>
	rc2014_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&bus_tag, U &&slot_options, char const *default_option, bool fixed = false)
		: rc2014_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1,1))
	{
		m_bus.set_tag(std::forward<T>(bus_tag));
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(fixed);
	}

protected:
	rc2014_slot_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);
	// device-level overrides
	virtual void device_start() override;
	virtual void device_resolve_objects() override;

	required_device<rc2014_bus_device> m_bus;
};

//**************************************************************************
//  RC2014 Extended Bus
//**************************************************************************

// ======================> rc2014_ext_bus_device

class rc2014_ext_bus_device : public rc2014_bus_device
{
public:
	// construction/destruction
	rc2014_ext_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto clk2_callback() { return m_clk2.bind(); }
	auto page_callback() { return m_page.bind(); }
	auto nmi_callback() { return m_nmi.bind(); }
	auto tx2_callback() { return m_tx2.bind(); }
	auto rx2_callback() { return m_rx2.bind(); }
	auto user5_callback() { return m_user5.bind(); }
	auto user6_callback() { return m_user6.bind(); }
	auto user7_callback() { return m_user7.bind(); }
	auto user8_callback() { return m_user8.bind(); }

	DECLARE_WRITE_LINE_MEMBER( clk2_w ) { m_clk2(state); }
	DECLARE_WRITE_LINE_MEMBER( page_w ) { m_page(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_nmi(state); }
	DECLARE_WRITE_LINE_MEMBER( tx2_w ) { m_tx2(state); }
	DECLARE_WRITE_LINE_MEMBER( rx2_w ) { m_rx2(state); }
	DECLARE_WRITE_LINE_MEMBER( user5_w ) { m_user5(state); }
	DECLARE_WRITE_LINE_MEMBER( user6_w ) { m_user6(state); }
	DECLARE_WRITE_LINE_MEMBER( user7_w ) { m_user7(state); }
	DECLARE_WRITE_LINE_MEMBER( user8_w ) { m_user8(state); }

protected:
	rc2014_ext_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	// device-level overrides
	virtual void device_start() override;

private:
	devcb_write_line m_clk2;
	devcb_write_line m_page;
	devcb_write_line m_nmi;
	devcb_write_line m_tx2;
	devcb_write_line m_rx2;
	devcb_write_line m_user5;
	devcb_write_line m_user6;
	devcb_write_line m_user7;
	devcb_write_line m_user8;
};

// ======================> device_rc2014_ext_card_interface

class rc2014_ext_slot_device;

class device_rc2014_ext_card_interface : public device_rc2014_card_interface
{
	friend class rc2014_ext_slot_device;

protected:
	// construction/destruction
	device_rc2014_ext_card_interface(const machine_config &mconfig, device_t &device);

	void set_bus_device(rc2014_ext_bus_device *bus_device);

	rc2014_ext_bus_device  *m_bus;
};

// ======================> rc2014_ext_slot_device

class rc2014_ext_slot_device : public rc2014_slot_device
{
public:
	rc2014_ext_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename T, typename U>
	rc2014_ext_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&bus_tag, U &&slot_options, char const *default_option, bool fixed = false)
		: rc2014_ext_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1,1))
	{
		m_bus.set_tag(std::forward<T>(bus_tag));
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(fixed);
	}

protected:
	rc2014_ext_slot_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);
	// device-level overrides
	virtual void device_start() override;
	virtual void device_resolve_objects() override;
};

//**************************************************************************
//  RC2014 RC80 Bus
//**************************************************************************

// ======================> rc2014_rc80_bus_device

class rc2014_rc80_bus_device : public rc2014_ext_bus_device
{
public:
	// construction/destruction
	rc2014_rc80_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	rc2014_rc80_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	// device-level overrides
	virtual void device_start() override;
};

// ======================> device_rc2014_rc80_card_interface

class rc2014_rc80_slot_device;

class device_rc2014_rc80_card_interface : public device_rc2014_ext_card_interface
{
	friend class rc2014_rc80_slot_device;

protected:
	// construction/destruction
	device_rc2014_rc80_card_interface(const machine_config &mconfig, device_t &device);

	void set_bus_device(rc2014_rc80_bus_device *bus_device);

	rc2014_rc80_bus_device  *m_bus;
};

// ======================> rc2014_rc80_slot_device

class rc2014_rc80_slot_device : public rc2014_ext_slot_device
{
public:
	rc2014_rc80_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename T, typename U>
	rc2014_rc80_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&bus_tag, U &&slot_options, char const *default_option)
		: rc2014_rc80_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1,1))
	{
		m_bus.set_tag(std::forward<T>(bus_tag));
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_resolve_objects() override;
};

// device type definition
DECLARE_DEVICE_TYPE(RC2014_BUS,  rc2014_bus_device)
DECLARE_DEVICE_TYPE(RC2014_SLOT, rc2014_slot_device)

DECLARE_DEVICE_TYPE(RC2014_EXT_BUS,  rc2014_ext_bus_device)
DECLARE_DEVICE_TYPE(RC2014_EXT_SLOT, rc2014_ext_slot_device)

DECLARE_DEVICE_TYPE(RC2014_RC80_BUS,  rc2014_rc80_bus_device)
DECLARE_DEVICE_TYPE(RC2014_RC80_SLOT, rc2014_rc80_slot_device)

#endif // MAME_BUS_RC2014_RC2014_H
