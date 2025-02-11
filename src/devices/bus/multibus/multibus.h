// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    multibus.h

    Intel Multibus

    P1 Multibus connector:

    Power supplies
    1   GND        2   GND
    3   +5Vdc      4   +5Vdc
    5   +5Vdc      6   +5Vdc
    7   +12Vdc     8   +12Vdc
    9   -5Vdc      10  -5Vdc
    11  GND        12  GND
    Bus controls
    13  BCLK/      14  INIT/
    15  BPRN/      16  BPRO/
    17  BUSY/      18  BREQ/
    19  MRDC/      20  MWTC/
    21  IORC/      22  IOWC/
    23  XACK/      24  INH1/
    Bus controls and address
    25  LOCK/      26  INH2/
    27  BHEN/      28  ADR10/
    29  CBRQ/      30  ADR11/
    31  CCLK/      32  ADR12/
    33  INTA/      34  ADR13/
    Interrupts
    35  INT6/      36  INT7/
    37  INT4/      38  INT5/
    39  INT2/      40  INT3/
    41  INT0/      42  INT1/
    Address
    43  ADRE/      44  ADRF/
    45  ADRC/      46  ADRD/
    47  ADRA/      48  ADRB/
    49  ADR8/      50  ADR9/
    51  ADR6/      52  ADR7/
    53  ADR4/      54  ADR5/
    55  ADR2/      56  ADR3/
    57  ADR0/      58  ADR1/
    Data
    59  DATE/      60  DATF/
    61  DATC/      62  DATD/
    63  DATA/      64  DATB/
    65  DAT8/      66  DAT9/
    67  DAT6/      68  DAT7/
    69  DAT4/      70  DAT5/
    71  DAT2/      72  DAT3/
    73  DAT0/      74  DAT1/
    Power supplies
    75  GND        76  GND
    77  reserved   78  reserved
    79  -12Vdc     80  -12Vdc
    81  +5Vdc      82  +5Vdc
    83  +5Vdc      84  +5Vdc
    85  GND        86  GND

    P2 Multibus connector:

    1-54 reserved for iLBX bus

    Address
    55  ADR16/     56  ADR17/
    57  ADR14/     58  ADR15/

    59-60 reserved for iLBx bus

*********************************************************************/

#ifndef MAME_BUS_MULTIBUS_MULTIBUS_H
#define MAME_BUS_MULTIBUS_MULTIBUS_H

#pragma once

class multibus_device
	: public device_t
	, public device_memory_interface
{
public:
	multibus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	enum flags : u16
	{
		FLAG_UNMAPPED = 0x0001,
	};

	// interrupt interface
	template <unsigned I> auto int_callback() { return m_int_cb[I].bind(); }
	template <unsigned I> void int_w(int state) { m_int_cb[I](state); }

	// XACK/
	auto xack_cb() { return m_xack_cb.bind(); }

	// Set XACK/ signal (this is meant for "device_multibus_interface" devices)
	void xack_w(int state) { m_xack_cb(state); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	void mem_map(address_map &map);
	void pio_map(address_map &map);

private:
	address_space_config const m_mem_config;
	address_space_config const m_pio_config;

	devcb_write_line::array<8> m_int_cb;
	devcb_write_line m_xack_cb;
};

class multibus_slot_device
	: public device_t
	, public device_slot_interface
{
public:
	multibus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = DERIVED_CLOCK(1, 1));

	template <typename T, typename U>
	multibus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&bus_tag, U &&slot_options, char const *default_option, bool const fixed)
		: multibus_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1,1))
	{
		m_bus.set_tag(std::forward<T>(bus_tag));

		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(fixed);
	}

	auto bus() const { return m_bus; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	required_device<multibus_device> m_bus;
};

class device_multibus_interface : public device_interface
{
protected:
	device_multibus_interface(machine_config const &mconfig, device_t &device);

	// configuration
	template <unsigned I> auto int_callback() { return m_int[I].bind(); }

	// device_interface implementation
	virtual void interface_config_complete() override ATTR_COLD;

	// runtime
	template <unsigned I> void int_w(int state) { m_bus->int_w<I>(state); }
	void int_w(unsigned number, int state);

	void xack_w(int state) { m_bus->xack_w(state); }
	void unmap(int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmirror = 0U);

	required_device<multibus_device> m_bus;

private:
	devcb_write_line::array<8> m_int;
};

// device type declaration
DECLARE_DEVICE_TYPE(MULTIBUS, multibus_device)
DECLARE_DEVICE_TYPE(MULTIBUS_SLOT, multibus_slot_device)

#endif // MAME_BUS_MULTIBUS_MULTIBUS_H
