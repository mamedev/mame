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
      P10          Reserved for Expansion connector



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
    27  -------   /HOLD
    26  -------   I/O R
    25  -------   HLDA

    24  -------   GND
    23  -------   MEMW
    22  -------   O2
    21  -------   I/O W
    20  -------   RDYIN
    19  -------   M1
    18  -------   GND
    17  -------   /D7
    16  -------   /D6
    15  -------   /D5
    14  -------   /D4
    13  -------   /D3
    12  -------   /D2
    11  -------   /D1
    10  -------   /D0
     9  -------   /INT2
     8  -------   /INT1
     7  -------   /INT7
     6  -------   /INT6
     5  -------   /INT5
     4  -------   /INT4
     3  -------   /INT3
     2  -------   -18V
     1  -------   GND
     0  -------   GND



    Signal         Pin       Direction        Description
    --------------------------------------------------------

    A0-A15         30-45     Output           Address bus

    D0-D7          10-17     Input/Output     Data bus

    HOLD           27        Input            Hold - request CPU hold state

    HLDA           25        Output           Hold Acknowledge - CPU has gone into hold status, and Address bus and
                                              Data bus are in high impedance state

    I/O R          26        Output           I/O Read

    I/O W          21        Output           I/O Write

    INT1-INT7      8-9,3-7   Input            Interrupt 1-7

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

***************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H8_H8BUS_H
#define MAME_BUS_HEATHZENITH_H8_H8BUS_H

#pragma once
#include "emu.h"
#include <functional>
#include <utility>
#include <vector>
#include <string.h>


class h8bus_device;


class device_h8bus_card_interface : public device_interface
{
	friend class h8bus_device;
public:
	// construction/destruction
	virtual ~device_h8bus_card_interface();

	void set_slot_int0(int state);
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

	virtual void set_hold_ack(int state) {}

	void set_h8bus_tag(h8bus_device *h8bus, const char *slottag) { m_h8bus = h8bus; m_h8bus_slottag = slottag; }

protected:
	device_h8bus_card_interface(const machine_config &mconfig, device_t &device);
	virtual void interface_pre_start() override;

	h8bus_device &h8bus() { assert(m_h8bus); return *m_h8bus; }

	const char *m_h8bus_slottag;

private:
	h8bus_device *m_h8bus;
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


class h8bus_device : public device_t
{
	friend class h8bus_slot_device;
	friend class device_h8bus_card_interface;

public:
	h8bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~h8bus_device();

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program_space.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io_space.set_tag(std::forward<T>(tag), spacenum); }

	void install_mem_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler);

	void install_io_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler);
	void install_io_device(offs_t start, offs_t end, read8smo_delegate rhandler, write8smo_delegate whandler);
	void install_io_device_w(offs_t start, offs_t end, write8smo_delegate whandler);

	auto out_int0_callback() { return m_out_int0_cb.bind(); }
	auto out_int1_callback() { return m_out_int1_cb.bind(); }
	auto out_int2_callback() { return m_out_int2_cb.bind(); }
	auto out_int3_callback() { return m_out_int3_cb.bind(); }
	auto out_int4_callback() { return m_out_int4_cb.bind(); }
	auto out_int5_callback() { return m_out_int5_cb.bind(); }
	auto out_int6_callback() { return m_out_int6_cb.bind(); }
	auto out_int7_callback() { return m_out_int7_cb.bind(); }
	auto out_reset_callback() { return m_out_reset_cb.bind(); }
	auto out_hold_callback() { return m_out_hold_cb.bind(); }
	auto out_rom_disable_callback() { return m_out_rom_disable_cb.bind(); }

	void set_hold_ack(u8 val);

protected:
	h8bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	//virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void add_h8bus_card(device_h8bus_card_interface &card);
	void set_int0_line(int state);
	void set_int1_line(int state);
	void set_int2_line(int state);
	void set_int3_line(int state);
	void set_int4_line(int state);
	void set_int5_line(int state);
	void set_int6_line(int state);
	void set_int7_line(int state);
	void set_reset_line(int state);
	void set_hold_line(int state);
	void set_disable_rom_line(int state);

private:
	required_address_space m_program_space, m_io_space;
	devcb_write_line m_out_int0_cb, m_out_int1_cb, m_out_int2_cb, m_out_int3_cb,
					 m_out_int4_cb, m_out_int5_cb, m_out_int6_cb, m_out_int7_cb;
	devcb_write_line m_out_reset_cb, m_out_hold_cb, m_out_rom_disable_cb;

	std::vector<std::reference_wrapper<device_h8bus_card_interface>> m_device_list;
};

inline void device_h8bus_card_interface::set_slot_int0(int state)
{
	h8bus().set_int0_line(state);
}

inline void device_h8bus_card_interface::set_slot_int1(int state)
{
	h8bus().set_int1_line(state);
}

inline void device_h8bus_card_interface::set_slot_int2(int state)
{
	h8bus().set_int2_line(state);
}

inline void device_h8bus_card_interface::set_slot_int3(int state)
{
	h8bus().set_int3_line(state);
}

inline void device_h8bus_card_interface::set_slot_int4(int state)
{
	h8bus().set_int4_line(state);
}

inline void device_h8bus_card_interface::set_slot_int5(int state)
{
	h8bus().set_int5_line(state);
}

inline void device_h8bus_card_interface::set_slot_int6(int state)
{
	h8bus().set_int6_line(state);
}

inline void device_h8bus_card_interface::set_slot_int7(int state)
{
	h8bus().set_int7_line(state);
}

inline void device_h8bus_card_interface::set_slot_reset(int state)
{
	h8bus().set_reset_line(state);
}

inline void device_h8bus_card_interface::set_slot_hold(int state)
{
	h8bus().set_hold_line(state);
}

inline void device_h8bus_card_interface::set_slot_rom_disable(int state)
{
	h8bus().set_disable_rom_line(state);
}

DECLARE_DEVICE_TYPE(H8BUS, h8bus_device)


#endif  // MAME_BUS_HEATHZENITH_H8_H8BUS_H
