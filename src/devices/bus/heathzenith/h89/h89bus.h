// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  h89.h - Heath/Zenith H-89/Z-90 bus
  by R. Belmont

               Left-side slots of the H89                Right-side slots of the H89

    pin    P501         P502         P503            P504         P505         P506       pin

     1      GND----------GND----------GND-------------GND----------GND----------GND        1
     2       D0-----------D0-----------D0--------------D0-----------D0-----------D0        2
     3       D1-----------D1-----------D1--------------D1-----------D1-----------D1        3
     4       D2-----------D2-----------D2--------------D2-----------D2-----------D2        4
     5       D3-----------D3-----------D3--------------D3-----------D3-----------D3        5
     6       D4-----------D4-----------D4--------------D4-----------D4-----------D4        6
     7       D5-----------D5-----------D5--------------D5-----------D5-----------D5        7
     8       D6-----------D6-----------D6--------------D6-----------D6-----------D6        8
     9       D7-----------D7-----------D7--------------D7-----------D7-----------D7        9
    10      GND----------GND----------GND-------------GND----------GND----------GND       10

           P507         P508         P509            P510         P511         P512

     1      VCC----------VCC----------VCC-------------VCC----------VCC----------VCC        1
     2      GND----------GND----------GND-------------GND----------GND----------GND        2
     3       A0-----------A0-----------A0--------------A0-----------A0-----------A0        3
     4       A1-----------A1-----------A1--------------A1-----------A1-----------A1        4
     5       A2-----------A2-----------A2--------------A2-----------A2-----------A2        5
     6       A3-----------A3-----------A3             /BRD---------/BRD---------/BRD       6
     7       A4-----------A4-----------A4             /BWR---------/BWR---------/BWR       7
     8       A5-----------A5-----------A5            /WAIT--------/WAIT--------/WAIT       8
     9       A6-----------A6-----------A6            /SER0--------/SER0--------/SER0       9
    10       A7-----------A7-----------A7            /SER1--------/SER1--------/SER1      10
    11       A8-----------A8-----------A8             /LP1---------/LP1        /FLPY      11
    12       A9-----------A9-----------A9            /CASS--------/CASS        /FMWE      12
    13      A10----------A10----------A10             2MHz---------2MHz---------2MHz      13
    14      A11----------A11----------A11            1.8Mhz-------1.8MHz-------1.8MHz     14
    15      A12----------A12----------A12             /RST---------/RST---------/RST      15
    16     MEM 0--------MEM 0--------MEM 0            IO 0---------IO 0---------IO 0      16
    17     MEM 1--------MEM 1--------MEM 1            IO 1---------IO 1---------IO 1      17
    18      RD5----------RD5----------RD5            /INT3--------/INT3--------/INT3      18
    19      RD6----------RD6----------RD6            /INT4--------/INT4--------/INT4      19
    20      RD7----------RD7----------RD7            /INT5--------/INT5--------/INT5      20
    21    (+12V)-------(+12V)-------(+12V)-----------(+12V)-------(+12V)-------(+12V)     21
    22    (-12V)-------(-12V)-------(-12V)-----------(-12V)-------(-12V)-------(-12V)     22
    23     (-5V)--------(-5V)--------(-5V)------------(-5V)--------(-5V)--------(-5V)     23
    24    (+12V)-------(+12V)-------(+12V)-----------(+12V)-------(+12V)-------(+12V)     24
    25      GND----------GND----------GND--------------GND----------GND----------GND      25


        Signal          Description
        ------------------------------------------
        MEM 0 H         Controlled by the GPP port
        MEM 1 H         Controlled by the GPP port
        RD5             Tied to +5V when CPU jumpers configured as recommended by Heath's configuration
                        guide. Could be jumpered to RAS2 (bank select 2: address 32k-48k) (active low)
        RD6             Selects the 16k Expansion memory (active low)
        RD7             Reserved for future use, 444-66 PROM nevers sets this low. (active low)
        BRD L           Board Read (active low)
        BWR L           Board Write (active low)
        WAIT L          Wait state (active low)
        SER0 L          Select line for one of the serial ports on the HA-88-3 (active low)
        SER1 L          Select line for one of the serial ports on the HA-88-3 (active low)
        LP 1 L          Select line for one of the serial ports on the HA-88-3 (active low)
        CASS L          Select line originally for the Cassette Interface board, but when used with later PROM it
                        selected Floppy controllers/interfaces – Z-89-37, Z-89-47, Z-89-67 (active low)
        2 MHz           CPU Clock, actual speed 2.048 MHz
        1.8 MHz         Serial Port Clock, actual speed 1.8432 MHz
        RST L           Reset line (active low)
        IO 0 H          Controlled by the GPP port
        IO 1 H          Controlled by the GPP port
        INT3 L          Interrupt level 3 (active low)
        INT4 L          Interrupt level 4 (active low)
        INT5 L          Interrupt level 5 (active low)
        FLPY L          Select line originally for the H-88-1 hard-sectored controller, but also used for the later
                        Z-89-47 and Z-89-67 when installed in slot P506/P512 (active low)
        FMWE H          Floppy Memory Write Enable

***************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H89_H89BUS_H
#define MAME_BUS_HEATHZENITH_H89_H89BUS_H

#pragma once
#include "emu.h"
#include <functional>
#include <utility>
#include <vector>
#include <string.h>

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class h89bus_device;

// ======================> device_h89bus_card_interface

// class representing interface-specific live H-89/Z-89 bus card
class device_h89bus_card_interface : public device_interface
{
	friend class h89bus_device;
public:
	// construction/destruction
	virtual ~device_h89bus_card_interface();

	// inline configuration
	void set_h89bus_tag(h89bus_device *h89bus, const char *slottag) { m_h89bus = h89bus; m_h89bus_slottag = slottag; }

protected:
	device_h89bus_card_interface(const machine_config &mconfig, device_t &device);
	virtual void interface_pre_start() override;

	h89bus_device &h89bus() { assert(m_h89bus); return *m_h89bus; }

	const char *m_h89bus_slottag;

private:
	h89bus_device *m_h89bus;
};

class device_h89bus_left_card_interface : public device_h89bus_card_interface
{
public:
	// construction/destruction
	virtual ~device_h89bus_left_card_interface();

	int get_mem0();
	int get_mem1();

	virtual u8 read(u8 select_lines, u16 offset) { return 0; };
	virtual void write(u8 select_lines, u16 offset, u8 data) {};

protected:
	device_h89bus_left_card_interface(const machine_config &mconfig, device_t &device);

private:
};

class device_h89bus_right_card_interface : public device_h89bus_card_interface
{
	friend class h89bus_device;

public:
	// construction/destruction
	virtual ~device_h89bus_right_card_interface();

	void set_slot_int3(int state);
	void set_slot_int4(int state);
	void set_slot_int5(int state);
	void set_slot_fdcirq(int state);
	void set_slot_fdcdrq(int state);
	void set_slot_blockirq(int state);
	void set_slot_fmwe(int state);
	void set_slot_wait(int state);
	int get_io0();
	int get_io1();

	virtual u8 read(u8 select_lines, u8 offset) { return 0; };
	virtual void write(u8 select_lines, u8 offset, u8 data) {};

	void set_p506_signalling(bool val)
	{
		m_p506_signals = val;
	}

protected:
	device_h89bus_right_card_interface(const machine_config &mconfig, device_t &device);
	bool m_p506_signals;

private:
};

class h89bus_left_slot_device : public device_t, public device_single_card_slot_interface<device_h89bus_left_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	h89bus_left_slot_device(const machine_config &mconfig, T &&tag, device_t *owner, const char *sltag, U &&opts, const char *dflt)
		: h89bus_left_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_h89bus_slot(std::forward<T>(sltag), tag);
	}

	h89bus_left_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T>
	void set_h89bus_slot(T &&tag, const char *slottag)
	{
		m_h89bus.set_tag(std::forward<T>(tag));
		m_h89bus_slottag = slottag;
	}

protected:
	h89bus_left_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<h89bus_device> m_h89bus;
	const char *m_h89bus_slottag;
};

// device type definition
DECLARE_DEVICE_TYPE(H89BUS_LEFT_SLOT, h89bus_left_slot_device)

class h89bus_right_slot_device : public device_t, public device_single_card_slot_interface<device_h89bus_right_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	h89bus_right_slot_device(const machine_config &mconfig, T &&tag, device_t *owner, const char *sltag, U &&opts, const char *dflt)
		: h89bus_right_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_h89bus_slot(std::forward<T>(sltag), tag);
	}

	h89bus_right_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T>
	void set_h89bus_slot(T &&tag, const char *slottag)
	{
		m_h89bus.set_tag(std::forward<T>(tag));
		m_h89bus_slottag = slottag;
	}

	void set_p506_signalling(bool val)
	{
		m_p506_signals = val;
	}

protected:
	h89bus_right_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<h89bus_device> m_h89bus;
	const char *m_h89bus_slottag;
	bool m_p506_signals;
};

// device type definition
DECLARE_DEVICE_TYPE(H89BUS_RIGHT_SLOT, h89bus_right_slot_device)

// ======================> h89bus_device
class h89bus_device : public device_t
{
	friend class h89bus_left_slot_device;
	friend class h89bus_right_slot_device;
	friend class device_h89bus_left_card_interface;
	friend class device_h89bus_right_card_interface;

public:
	// left card select lines
	static constexpr u8 H89_RD5         = 0x01;
	static constexpr u8 H89_RD6         = 0x02;
	static constexpr u8 H89_RD7         = 0x04;
	// The Sigmasoft parallel card plugs into the left slot and has a jumper to
	// get the memory / I/O select signal from the motherboard.  This plus the left
	// slots' A0-A13 lines means it can claim arbitrary I/O ranges that the PROM doesn't
	// select anything at.
	static constexpr u8 H89_IO          = 0x80;

	// right card and on-board I/O space select lines
	static constexpr u8 H89_GPP         = 0x01;
	static constexpr u8 H89_NMI         = 0x02;
	static constexpr u8 H89_TERM        = 0x04;
	static constexpr u8 H89_SER1        = 0x08;
	static constexpr u8 H89_SER0        = 0x10;
	static constexpr u8 H89_LP          = 0x20;
	static constexpr u8 H89_CASS        = 0x40;
	static constexpr u8 H89_FLPY        = 0x80;

	// construction/destruction
	h89bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~h89bus_device();

	void set_io0(int state);
	void set_io1(int state);
	void set_mem0(int state);
	void set_mem1(int state);
	int get_io0();
	int get_io1();
	int get_mem0();
	int get_mem1();

	// inline configuration
	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program_space.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io_space.set_tag(std::forward<T>(tag), spacenum); }
	auto out_int3_callback() { return m_out_int3_cb.bind(); }
	auto out_int4_callback() { return m_out_int4_cb.bind(); }
	auto out_int5_callback() { return m_out_int5_cb.bind(); }
	auto out_fdcirq_callback() { return m_out_fdcirq_cb.bind(); }
	auto out_fdcdrq_callback() { return m_out_fdcdrq_cb.bind(); }
	auto out_blockirq_callback() { return m_out_blockirq_cb.bind(); }
	auto out_fmwe_callback() { return m_out_fmwe_cb.bind(); }
	auto out_wait_callback() { return m_out_wait_cb.bind(); }
	auto in_tlb_callback() { return m_in_tlb_cb.bind(); }
	auto in_nmi_callback() { return m_in_nmi_cb.bind(); }
	auto in_gpp_callback() { return m_in_gpp_cb.bind(); }
	auto out_tlb_callback() { return m_out_tlb_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }
	auto out_gpp_callback() { return m_out_gpp_cb.bind(); }

protected:
	h89bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// bus-internal handlers
	void add_h89bus_left_card(device_h89bus_left_card_interface &card);
	void add_h89bus_right_card(device_h89bus_right_card_interface &card);
	void set_int3_line(int state);
	void set_int4_line(int state);
	void set_int5_line(int state);
	void set_fdcirq_line(int state);
	void set_fdcdrq_line(int state);
	void set_blockirq_line(int state);
	void set_fmwe_line(int state);
	void set_wait_line(int state);
	u8 read_gpp();
	void write_gpp(u8 data);

	// internal state
	required_address_space m_program_space, m_io_space;
	required_region_ptr<uint8_t> m_decode_prom;
	int m_io0, m_io1, m_mem0, m_mem1;

private:
	devcb_write_line m_out_int3_cb, m_out_int4_cb, m_out_int5_cb, m_out_fdcirq_cb, m_out_fdcdrq_cb, m_out_blockirq_cb;
	devcb_write_line m_out_fmwe_cb, m_out_wait_cb;
	devcb_read8 m_in_tlb_cb, m_in_nmi_cb, m_in_gpp_cb;
	devcb_write8 m_out_tlb_cb, m_out_nmi_cb, m_out_gpp_cb;

	std::vector<std::reference_wrapper<device_h89bus_left_card_interface>> m_left_device_list;
	std::vector<std::reference_wrapper<device_h89bus_right_card_interface>> m_right_device_list;

	u8 io_dispatch_r(offs_t offset);
	void io_dispatch_w(offs_t offset, u8 data);
};

inline int device_h89bus_left_card_interface::get_mem0()
{
	return h89bus().get_mem0();
}

inline int device_h89bus_left_card_interface::get_mem1()
{
	return h89bus().get_mem1();
}

inline int device_h89bus_right_card_interface::get_io0()
{
	return h89bus().get_io0();
}

inline int device_h89bus_right_card_interface::get_io1()
{
	return h89bus().get_io1();
}

inline void device_h89bus_right_card_interface::set_slot_int3(int state)
{
	h89bus().set_int3_line(state);
}

inline void device_h89bus_right_card_interface::set_slot_int4(int state)
{
	h89bus().set_int4_line(state);
}

inline void device_h89bus_right_card_interface::set_slot_int5(int state)
{
	h89bus().set_int5_line(state);
}

inline void device_h89bus_right_card_interface::set_slot_fdcirq(int state)
{
	h89bus().set_fdcirq_line(state);
}

inline void device_h89bus_right_card_interface::set_slot_fdcdrq(int state)
{
	h89bus().set_fdcdrq_line(state);
}

inline void device_h89bus_right_card_interface::set_slot_blockirq(int state)
{
	h89bus().set_blockirq_line(state);
}

inline void device_h89bus_right_card_interface::set_slot_fmwe(int state)
{
	if (m_p506_signals)
	{
		h89bus().set_fmwe_line(state);
	}
}

inline void device_h89bus_right_card_interface::set_slot_wait(int state)
{
	h89bus().set_wait_line(state);
}

// device type definition
DECLARE_DEVICE_TYPE(H89BUS, h89bus_device)

#endif  // MAME_BUS_HEATHZENITH_H89_H89BUS_H
