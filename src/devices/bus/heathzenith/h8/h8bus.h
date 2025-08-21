// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/*********************************************************************

  Heathkit H8 expansion slot

  Also referred to as Benton Harbor Bus.

**********************************************************************

     Slot          Cards
  --------------------------------------------------------
      P1           Reserved for control/front panel board
      P2           Reserved for CPU board
    P3 - P9        Available
      P10          Reserved for Expansion connector/HA-8-8 Extended Configuration Option


    pin         P1 - P10
  --------------------------------
    49  -------   +8V
    48  -------   +8V
    47  -------   +18V
    46  -------   /ROM DISABLE
    45  -------   /A15
    44  -------   /A14
    43  -------   /A13
    42  -------   /A12
    41  -------   /A11
    40  -------   /A10
    39  -------   /A9
    38  -------   /A8
    37  -------   /A7
    36  -------   /A6
    35  -------   /A5
    34  -------   /A4
    33  -------   /A3
    32  -------   /A2
    31  -------   /A1
    30  -------   /A0
    29  -------   /RESET
    28  -------   MEMR
    27  -------   /HOLD   *
    26  -------   I/O R
    25  -------   HLDA    *

    24  -------   GND     *
    23  -------   MEMW
    22  -------   O2
    21  -------   I/O W
    20  -------   RDYIN   *
    19  -------   M1
    18  -------   GND     *
    17  -------   /D7
    16  -------   /D6
    15  -------   /D5
    14  -------   /D4
    13  -------   /D3
    12  -------   /D2
    11  -------   /D1
    10  -------   /D0
     9  -------   /INT2   *
     8  -------   /INT1   *
     7  -------   /INT7
     6  -------   /INT6
     5  -------   /INT5
     4  -------   /INT4
     3  -------   /INT3
     2  -------   -18V
     1  -------   GND
     0  -------   GND

     Notes:
       *  -  Heath Company reserves the right to change these pin designations.


    Signal         Pin       Direction        Description
                             (wrt CPU board)
    --------------------------------------------------------

    A0-A15         30-45     Output           Address bus

    D0-D7          10-17     Input/Output     Data bus

    HOLD           27        Input            Hold - request CPU hold state

    HLDA           25        Output           Hold Acknowledge - CPU has gone into hold status, and Address bus and
                                              Data bus are in high impedance state

    I/O R          26        Output           I/O Read

    I/O W          21        Output           I/O Write

    INT1-INT2      8-9       Input            Interrupt 1-2 (10, 20)

    INT3-INT7      3-7       Input            Interrupt 3-7 (30, 40, 50, 60, 70)

    M1             19        Output           Machine cycle 1 - CPU fetch cycle of the first byte of an instruction

    MEMR           28        Output           Memory Read

    MEMW           23        Output           Memory Write

    ϕ2             22        Output           Phase 2 clock

    RDYIN          20        Input            Ready Input - Asynchronous wait request to the clock generator, generates
                                              the synchronous READY signal.

    RESET          29        Input            Reset - holds CPU in reset, also resets the INTE and HLDA flip-flops

    ROM DISABLE    46        Input            ROM Disable - Disable the on-board ROM

    +18 V          47                         +18 V Unregulated
    +8 V           48,49                       +8 V Unregulated
    -18 V          2                          -18 V Unregulated
    GND            0,1                        Ground
    CPU GND        18,24                      CPU Ground


***************************************************************************

  P201 Cable

  Between P1 and P2 is a cable labeled P201 in the schematics. This connects
  the Front Panel(P1) card to the CPU board(P2), providing dedicated signal lines
  between the two boards.

    Signal         Pin       Direction        Description
    --------------------------------------------------------

    GND             1                         Ground
    /INT2 (20)      2        P1 -> P2         Interrupt 20
    /INTE           3        P2 -> P1         Interrupts enabled.
    /INT1 (10)      4        P1 -> P2         Interrupt 10
    /RESIN          5        P1 -> P2         Reset in, allowed keypad to initiate a reset of the computer

***************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H8_H8BUS_H
#define MAME_BUS_HEATHZENITH_H8_H8BUS_H

#pragma once
#include <functional>
#include <utility>
#include <vector>


class h8bus_device;


class device_h8bus_card_interface : public device_interface
{
	friend class h8bus_device;
public:
	virtual ~device_h8bus_card_interface();

	// signals from the Bus
	virtual void int1_w(int state) {}
	virtual void int2_w(int state) {}
	virtual void int3_w(int state) {}
	virtual void int4_w(int state) {}
	virtual void int5_w(int state) {}
	virtual void int6_w(int state) {}
	virtual void int7_w(int state) {}

	virtual void m1_w(int state) {}
	virtual void reset_w(int state) {}
	virtual void hold_w(int state) {}
	virtual void rom_disable_w(int state) {}

	void set_h8bus_tag(h8bus_device *h8bus, const char *slottag) { m_h8bus = h8bus; m_h8bus_slottag = slottag; }
	void set_index(u8 index) { m_index = index; }

protected:
	device_h8bus_card_interface(const machine_config &mconfig, device_t &device);
	virtual void interface_pre_start() override;

	void set_slot_int1(int state);
	void set_slot_int2(int state);
	void set_slot_int3(int state);
	void set_slot_int4(int state);
	void set_slot_int5(int state);
	void set_slot_int6(int state);
	void set_slot_int7(int state);
	void set_slot_reset(int state);
	void set_slot_hold(int state);
	void set_slot_rom_disable(int state);
	void set_slot_m1(int state);

	h8bus_device &h8bus() { assert(m_h8bus); return *m_h8bus; }

	const char *m_h8bus_slottag;
	u8 m_index;

private:
	h8bus_device *m_h8bus;
};

class device_p201_p1_card_interface : public device_interface
{
public:
	virtual ~device_p201_p1_card_interface();

	auto p201_reset_cb() { return m_p201_reset.bind(); }
	auto p201_int1_cb() { return m_p201_int1.bind(); }
	auto p201_int2_cb() { return m_p201_int2.bind(); }

	virtual void p201_inte_w(int state) {};

protected:
	device_p201_p1_card_interface(device_t &device, const char *tag);

	devcb_write_line m_p201_reset;
	devcb_write_line m_p201_int1;
	devcb_write_line m_p201_int2;
};


class device_p201_p2_card_interface : public device_interface
{
public:
	virtual ~device_p201_p2_card_interface();

	auto p201_inte_cb() { return m_p201_inte.bind(); }

	virtual void p201_reset_w(int state) {};
	virtual void p201_int1_w(int state) {};
	virtual void p201_int2_w(int state) {};

protected:
	device_p201_p2_card_interface(device_t &device, const char *tag);

	devcb_write_line m_p201_inte;
};


class h8bus_slot_device : public device_t, public device_single_card_slot_interface<device_h8bus_card_interface>
{
public:
	template <typename T, typename U>
	h8bus_slot_device(const machine_config &mconfig, T &&tag, device_t *owner, const char *sltag, U &&opts, const char *dflt)
		: h8bus_slot_device(mconfig, tag, owner, (uint32_t) 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_h8bus_slot(std::forward<T>(sltag), tag);
	}

	h8bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	void set_h8bus_slot(T &&tag, const char *slottag)
	{
		m_h8bus.set_tag(std::forward<T>(tag));
		m_h8bus_slottag = slottag;
	}

protected:
	h8bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	required_device<h8bus_device> m_h8bus;
	const char *m_h8bus_slottag;
};

DECLARE_DEVICE_TYPE(H8BUS_SLOT, h8bus_slot_device)


class h8bus_device : public device_t, public device_memory_interface
{
	friend class h8bus_slot_device;
	friend class device_h8bus_card_interface;

public:
	h8bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~h8bus_device();

protected:
	h8bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	u8 add_h8bus_card(device_h8bus_card_interface &card);
	void set_int1_line(unsigned index, int state);
	void set_int2_line(unsigned index, int state);
	void set_int3_line(unsigned index, int state);
	void set_int4_line(unsigned index, int state);
	void set_int5_line(unsigned index, int state);
	void set_int6_line(unsigned index, int state);
	void set_int7_line(unsigned index, int state);
	void set_reset_line(unsigned index, int state);
	void set_hold_line(unsigned index, int state);
	void set_disable_rom_line(unsigned index, int state);
	void set_m1_line(unsigned index, int state);

	void set_p201_inte(int state);
	void set_p201_reset(int state);
	void set_p201_int1(int state);
	void set_p201_int2(int state);

	bool update_line_states(u32 &states, unsigned index, int state);

	void mem_map(address_map &map);
	void io_map(address_map &map);

private:
	u32 m_int1_slot_states;
	u32 m_int2_slot_states;
	u32 m_int3_slot_states;
	u32 m_int4_slot_states;
	u32 m_int5_slot_states;
	u32 m_int6_slot_states;
	u32 m_int7_slot_states;
	u32 m_reset_slot_states;
	u32 m_hold_slot_states;
	u32 m_disable_rom_slot_states;
	u32 m_m1_slot_states;

	address_space_config const m_mem_config;
	address_space_config const m_io_config;

	std::vector<std::reference_wrapper<device_h8bus_card_interface>> m_device_list;
};


inline void device_h8bus_card_interface::set_slot_int1(int state)
{
	h8bus().set_int1_line(m_index, state);
}

inline void device_h8bus_card_interface::set_slot_int2(int state)
{
	h8bus().set_int2_line(m_index, state);
}

inline void device_h8bus_card_interface::set_slot_int3(int state)
{
	h8bus().set_int3_line(m_index, state);
}

inline void device_h8bus_card_interface::set_slot_int4(int state)
{
	h8bus().set_int4_line(m_index, state);
}

inline void device_h8bus_card_interface::set_slot_int5(int state)
{
	h8bus().set_int5_line(m_index, state);
}

inline void device_h8bus_card_interface::set_slot_int6(int state)
{
	h8bus().set_int6_line(m_index, state);
}

inline void device_h8bus_card_interface::set_slot_int7(int state)
{
	h8bus().set_int7_line(m_index, state);
}

inline void device_h8bus_card_interface::set_slot_m1(int state)
{
	h8bus().set_m1_line(m_index, state);
}

inline void device_h8bus_card_interface::set_slot_reset(int state)
{
	h8bus().set_reset_line(m_index, state);
}

inline void device_h8bus_card_interface::set_slot_hold(int state)
{
	h8bus().set_hold_line(m_index, state);
}

inline void device_h8bus_card_interface::set_slot_rom_disable(int state)
{
	h8bus().set_disable_rom_line(m_index, state);
}

DECLARE_DEVICE_TYPE(H8BUS, h8bus_device)


#endif  // MAME_BUS_HEATHZENITH_H8_H8BUS_H
