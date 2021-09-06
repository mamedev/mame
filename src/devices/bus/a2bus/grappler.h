// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************

    Orange Micro Grappler/Grappler+ Printer Interface

    With references to schematics from The Grappler™ Interface
    Operators Manual (Copyright 1982 Orance Micro, Inc.) and
    Grappler™+ Printer Interface Series Operators Manual (© Orange
    Micro, Inc. 1982).  This uses the IC locations on the "long"
    version of the Grappler+ card - the newer "short" version of the
    card has slightly different circuitry and completely different IC
    locations.

    26-pin two-row header to printer:

        STB    1   2  GND
        D0     3   4  GND
        D1     5   6  GND
        D2     7   8  GND
        D3     9  10  GND
        D4    11  12  GND
        D5    23  14  GND
        D6    15  16  GND
        D7    17  18  GND
        ACK   19  20  GND
        BUSY  21  22  GND
        P.E.  23  24  GND
        SLCT  25  26  GND

    Orange Micro Buffered Grappler+ Printer Interface

    26-pin two-row header to printer:

        STB    1   2  GND
        D0     3   4  GND
        D1     5   6  GND
        D2     7   8  GND
        D3     9  10  GND
        D4    11  12  GND
        D5    23  14  GND
        D6    15  16  GND
        D7    17  18  GND
        ACK   19  20  GND
        BUSY  21  22  GND
        P.E.  23  24  GND
        SLCT  25  26  N/C

***********************************************************************/
#ifndef MAME_BUS_A2BUS_GRAPPLERPLUS_H
#define MAME_BUS_A2BUS_GRAPPLERPLUS_H

#pragma once

#include "a2bus.h"

#include "bus/centronics/ctronics.h"
#include "cpu/mcs48/mcs48.h"


DECLARE_DEVICE_TYPE(A2BUS_GRAPPLER, a2bus_grappler_device)
DECLARE_DEVICE_TYPE(A2BUS_GRAPPLERPLUS, a2bus_grapplerplus_device)
DECLARE_DEVICE_TYPE(A2BUS_BUFGRAPPLERPLUS, a2bus_buf_grapplerplus_device)
DECLARE_DEVICE_TYPE(A2BUS_BUFGRAPPLERPLUSA, a2bus_buf_grapplerplus_reva_device)


class a2bus_grappler_device_base : public device_t, public device_a2bus_card_interface
{
public:
	// device_a2bus_card_interface implementation
	virtual u8 read_c800(u16 offset) override;

protected:
	a2bus_grappler_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// signal state
	u8 busy_in() const { return m_busy_in; }
	u8 pe_in() const { return m_pe_in; }
	u8 slct_in() const { return m_slct_in; }

	required_device<centronics_device>      m_printer_conn;
	required_device<output_latch_device>    m_printer_out;
	required_region_ptr<u8>                 m_rom;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	// helpers
	void set_rom_bank(u16 rom_bank);

private:
	// printer status inputs
	DECLARE_WRITE_LINE_MEMBER(busy_w);
	DECLARE_WRITE_LINE_MEMBER(pe_w);
	DECLARE_WRITE_LINE_MEMBER(slct_w);

	// synchronised printer status inputs
	void set_busy_in(void *ptr, s32 param);
	void set_pe_in(void *ptr, s32 param);
	void set_slct_in(void *ptr, s32 param);

	u16 m_rom_bank;     // U2D (pin 13)
	u8  m_busy_in;      // printer connector pin 21 (synchronised)
	u8  m_pe_in;        // printer connector pin 23 (synchronised)
	u8  m_slct_in;      // printer connector pin 25 (synchronised)
};


class a2bus_grappler_device : public a2bus_grappler_device_base
{
public:
	a2bus_grappler_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// printer status inputs
	DECLARE_WRITE_LINE_MEMBER(ack_w);

	// synchronised signals
	void set_data(void *ptr, s32 param);
	void set_strobe(void *ptr, s32 param);
	void set_ack_in(void *ptr, s32 param);

	u8  m_strobe;       // U3 (pin 4)
	u8  m_ack_latch;    // U3 (pin 13)
	u8  m_ack_in;       // printer connector pin 19 (synchronised)
};


class a2bus_grapplerplus_device_base : public a2bus_grappler_device_base
{
public:
	// DIP switch handlers
	virtual DECLARE_INPUT_CHANGED_MEMBER(sw_msb) { }

	// device_a2bus_card_interface implementation
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;

protected:
	a2bus_grapplerplus_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// ACK latch set input
	DECLARE_WRITE_LINE_MEMBER(ack_w);

	// signal state
	u8 ack_latch() const { return m_ack_latch; }

	required_ioport m_s1;

private:
	// synchronised printer status inputs
	void set_ack_in(void *ptr, s32 param);

	// for derived devices to implement
	virtual void data_latched(u8 data) = 0;
	virtual void ack_latch_set() { }
	virtual void ack_latch_cleared() { }

	u8  m_ack_latch;    // U2C (pin 9)
	u8  m_ack_in;       // printer connector pin 19 (synchronised)
};


class a2bus_grapplerplus_device : public a2bus_grapplerplus_device_base
{
public:
	a2bus_grapplerplus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// DIP switch handlers
	virtual DECLARE_INPUT_CHANGED_MEMBER(sw_msb) override;

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// a2bus_grapplerplus_device_base implementation
	virtual void data_latched(u8 data) override;
	virtual void ack_latch_set() override;
	virtual void ack_latch_cleared() override;

	// timer handlers
	TIMER_CALLBACK_MEMBER(update_strobe);

	emu_timer * m_strobe_timer;

	u8  m_data_latch;   // U10
	u8  m_irq_disable;  // U2A (pin 4)
	u8  m_irq;          // U3D (pin 13)
	u8  m_next_strobe;  // U5A (pin 5)
};


class a2bus_buf_grapplerplus_device : public a2bus_grapplerplus_device_base
{
public:
	a2bus_buf_grapplerplus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;

protected:
	a2bus_buf_grapplerplus_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// helpers
	template <typename T> void device_add_mconfig(machine_config &config, T &&mcu_clock);

private:
	// a2bus_grapplerplus_device_base implementation
	virtual void data_latched(u8 data) override;

	// printer status inputs
	DECLARE_WRITE_LINE_MEMBER(buf_ack_w);

	// MCU I/O handlers
	void mcu_io(address_map &map);
	void mcu_p2_w(u8 data);
	u8 mcu_bus_r();
	void mcu_bus_w(u8 data);

	// synchronised signals
	void set_buf_data(void *ptr, s32 param);
	void set_buf_ack_in(void *ptr, s32 param);
	void clear_ibusy(void *ptr, s32 param);

	required_device<mcs48_cpu_device>   m_mcu;
	std::unique_ptr<u8 []>              m_ram;

	u16 m_ram_row;          // U1-U8
	u8  m_ram_mask;         // mask out chips that are not installed
	u8  m_mcu_p2;           // U10
	u8  m_data_latch;       // U14 (synchronised)
	u8  m_ibusy;            // U12
	u8  m_buf_ack_latch;    // U12
	u8  m_buf_ack_in;       // printer connector pin 19 (synchronised)
};


class a2bus_buf_grapplerplus_reva_device : public a2bus_buf_grapplerplus_device
{
public:
	a2bus_buf_grapplerplus_reva_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	static auto parent_rom_device_type() { return &A2BUS_BUFGRAPPLERPLUS; }

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override;
};

#endif // MAME_BUS_A2BUS_GRAPPLERPLUS_H
