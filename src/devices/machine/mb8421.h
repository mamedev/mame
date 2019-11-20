// license:BSD-3-Clause
// copyright-holders:hap,AJR
/**********************************************************************

    Dual port RAM with Mailbox emulation

    Fujitsu MB8421/22/31/32-90/-90L/-90LL/-12/-12L/-12LL
    CMOS 16K-bit (2KB) dual-port SRAM (pinouts : see below)
	IDT 71321 16K-bit (2Kx8) dual port SRAM
	IDT 7130 8K-bit (1Kx8) dual port SRAM
	Cypress CY7C131 8K-bit (1Kx8) dual port SRAM

***********************************************************************
                            _____________
                 _CS(L)  1 |*    \_/     | 52  Vcc
                 _WE(L)  2 |             | 51 _CS(R)
               _BUSY(L)  3 |             | 50 _WE(R)
                _INT(L)  4 |             | 49 _BUSY(R)
                    NC   5 |             | 48 _INT(R)
                 A10(L)  6 |             | 47  NC
                 _OE(L)  7 |             | 46  A10(R)
                  A0(L)  8 |             | 45 _OE(R)
                  A1(L)  9 |             | 44  A0(R)
                  A2(L) 10 |             | 43  A1(R)
                  A3(L) 11 |             | 42  A2(R)
                  A4(L) 12 |             | 41  A3(R)
                  A5(L) 13 |    MB8421   | 40  A4(R)
                  A6(L) 14 |    MB8431   | 39  A5(R)
                  A7(L) 15 |             | 38  A6(R)
                  A8(L) 16 |             | 37  A7(R)
                  A9(L) 17 |             | 36  A8(R)
                I/O0(L) 18 |             | 35  A9(R)
                I/O1(L) 19 |             | 34  I/O7(R)
                I/O2(L) 20 |             | 33  I/O6(R)
                I/O3(L) 21 |             | 32  I/O5(R)
                I/O4(L) 22 |             | 31  I/O4(R)
                I/O5(L) 23 |             | 30  I/O3(R)
                I/O6(L) 24 |             | 29  I/O2(R)
                I/O7(L) 25 |             | 28  I/O1(R)
                   Vss  26 |_____________| 27  I/O0(R)

                            _____________
                 _CS(L)  1 |*    \_/     | 48  Vcc
                 _WE(L)  2 |             | 47 _CS(R)
               _BUSY(L)  3 |             | 46 _WE(R)
                 A10(L)  4 |             | 45 _BUSY(R)
                 _OE(L)  5 |             | 44  A10(R)
                  A0(L)  6 |             | 43 _OE(R)
                  A1(L)  7 |             | 42  A0(R)
                  A2(L)  8 |             | 41  A1(R)
                  A3(L)  9 |             | 40  A2(R)
                  A4(L) 10 |             | 39  A3(R)
                  A5(L) 11 |    MB8422   | 38  A4(R)
                  A6(L) 12 |    MB8432   | 37  A5(R)
                  A7(L) 13 |             | 36  A6(R)
                  A8(L) 14 |             | 35  A7(R)
                  A9(L) 15 |             | 34  A8(R)
                I/O0(L) 16 |             | 33  A9(R)
                I/O1(L) 17 |             | 32  I/O7(R)
                I/O2(L) 18 |             | 31  I/O6(R)
                I/O3(L) 19 |             | 30  I/O5(R)
                I/O4(L) 20 |             | 29  I/O4(R)
                I/O5(L) 21 |             | 28  I/O3(R)
                I/O6(L) 22 |             | 27  I/O2(R)
                I/O7(L) 23 |             | 26  I/O1(R)
                   Vss  24 |_____________| 25  I/O0(R)

**********************************************************************/

#ifndef MAME_MACHINE_MB8421_H
#define MAME_MACHINE_MB8421_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(CY7C131,             cy7c131_device)
DECLARE_DEVICE_TYPE(IDT7130,             idt7130_device)
DECLARE_DEVICE_TYPE(MB8421,              mb8421_device)
DECLARE_DEVICE_TYPE(IDT71321,            idt71321_device)
DECLARE_DEVICE_TYPE(MB8421_MB8431_16BIT, mb8421_mb8431_16_device)

// ======================> dual_port_mailbox_ram_base

template <typename Type, unsigned AddrBits, unsigned DataBits>
class dual_port_mailbox_ram_base : public device_t
{
public:
	// note: INT pins are only available on MB84x1
	// INTL is for the CPU on the left side, INTR for the one on the right
	auto intl_callback() { return m_intl_callback.bind(); }
	auto intr_callback() { return m_intr_callback.bind(); }

	Type peek(offs_t offset) const { return m_ram[offset & ADDR_MASK]; }

	//-------------------------------------------------
	//  left_w - write access for left-side bus
	//  (write to (max word size - 1) asserts INTR)
	//-------------------------------------------------

	void left_w(offs_t offset, Type data, Type mem_mask = ~Type(0))
	{
		offset &= ADDR_MASK;
		data &= DATA_MASK;
		COMBINE_DATA(&m_ram[offset]);
		update_intr(read_or_write::WRITE, false, offset);
	}

	//-------------------------------------------------
	//  left_r - read access for left-side bus
	//  (read from (max word size - 2) acknowledges INTL)
	//-------------------------------------------------

	Type left_r(offs_t offset)
	{
		offset &= ADDR_MASK;
		update_intr(read_or_write::READ, false, offset);
		return m_ram[offset];
	}

	//-------------------------------------------------
	//  right_w - write access for right-side bus
	//  (write to (max word size - 2) asserts INTL)
	//-------------------------------------------------

	void right_w(offs_t offset, Type data, Type mem_mask = ~Type(0))
	{
		offset &= ADDR_MASK;
		data &= DATA_MASK;
		COMBINE_DATA(&m_ram[offset]);
		update_intr(read_or_write::WRITE, true, offset);
	}

	//-------------------------------------------------
	//  right_r - read access for right-side bus
	//  (read from (max word size - 1) acknowledges INTR)
	//-------------------------------------------------

	Type right_r(offs_t offset)
	{
		offset &= ADDR_MASK;
		update_intr(read_or_write::READ, true, offset);
		return m_ram[offset];
	}

	DECLARE_READ_LINE_MEMBER(busy_r) { return 0; } // _BUSY pin - not emulated

protected:
	dual_port_mailbox_ram_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, m_intl_callback(*this)
		, m_intr_callback(*this)
		, m_ram(nullptr)
	{
	}

	// device-level overrides

	//-------------------------------------------------
	//  device_resolve_objects - resolve objects that
	//  may be needed for other devices to set
	//  initial conditions at start time
	//-------------------------------------------------

	virtual void device_resolve_objects() override
	{
		// resolve callbacks
		m_intl_callback.resolve_safe();
		m_intr_callback.resolve_safe();
	}

	//-------------------------------------------------
	//  device_start - device-specific startup
	//-------------------------------------------------

	virtual void device_start() override
	{
		m_ram = make_unique_clear<Type[]>(RAM_SIZE);

		// state save
		save_pointer(NAME(m_ram), RAM_SIZE);
	}

	//-------------------------------------------------
	//  device_reset - device-specific reset
	//-------------------------------------------------

	virtual void device_reset() override
	{
		m_intl_callback(CLEAR_LINE);
		m_intr_callback(CLEAR_LINE);
	}

private:
	// internal helpers
	static constexpr Type DATA_MASK = make_bitmask<Type>(DataBits); // for DPRAMs with 9/18/36bit data buses
	static constexpr size_t RAM_SIZE = make_bitmask<size_t>(AddrBits) + 1; // max RAM word size
	static constexpr offs_t ADDR_MASK = RAM_SIZE - 1;
	static constexpr offs_t INT_ADDR_LEFT = ADDR_MASK - 1; // max RAM word size - 2
	static constexpr offs_t INT_ADDR_RIGHT = ADDR_MASK; // max RAM word size - 1

	//-------------------------------------------------
	//  update_intr - update interrupt lines upon
	//  read or write accesses to special locations
	//-------------------------------------------------

	void update_intr(read_or_write row, bool is_right, offs_t offset)
	{
		if (machine().side_effects_disabled())
			return;

		if (row == read_or_write::WRITE && offset == (is_right ? INT_ADDR_LEFT : INT_ADDR_RIGHT))
			(is_right ? m_intl_callback : m_intr_callback)(ASSERT_LINE);
		else if (row == read_or_write::READ && offset == (is_right ? INT_ADDR_RIGHT : INT_ADDR_LEFT))
			(is_right ? m_intr_callback : m_intl_callback)(CLEAR_LINE);
	}

	devcb_write_line m_intl_callback;
	devcb_write_line m_intr_callback;
	std::unique_ptr<Type[]> m_ram;
};

// ======================> cy7c131_device

class cy7c131_device : public dual_port_mailbox_ram_base<u8, 10, 8>
{
public:
	cy7c131_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: dual_port_mailbox_ram_base<u8, 10, 8>(mconfig, CY7C131, tag, owner, clock) // 1kx8
	{
	}
};

// ======================> idt7130_device

class idt7130_device : public dual_port_mailbox_ram_base<u8, 10, 8>
{
public:
	idt7130_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: dual_port_mailbox_ram_base<u8, 10, 8>(mconfig, IDT7130, tag, owner, clock) // 1kx8
	{
	}
};

// ======================> idt71321_device

class idt71321_device : public dual_port_mailbox_ram_base<u8, 11, 8>
{
public:
	idt71321_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: dual_port_mailbox_ram_base<u8, 11, 8>(mconfig, IDT71321, tag, owner, clock) // 2kx8
	{
	}
};

// ======================> mb8421_device

class mb8421_device : public dual_port_mailbox_ram_base<u8, 11, 8>
{
public:
	mb8421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: dual_port_mailbox_ram_base<u8, 11, 8>(mconfig, MB8421, tag, owner, clock) // 2kx8
	{
	}
};

// ======================> mb8421_mb8431_16_device

class mb8421_mb8431_16_device : public dual_port_mailbox_ram_base<u16, 11, 16>
{
public:
	mb8421_mb8431_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: dual_port_mailbox_ram_base<u16, 11, 16>(mconfig, MB8421_MB8431_16BIT, tag, owner, clock) // 2kx16
	{
	}
};

//**************************************************************************
//  EXTERNAL TEMPLATE INSTANTIATIONS
//**************************************************************************

extern template class dual_port_mailbox_ram_base<u8, 10, 8>;
extern template class dual_port_mailbox_ram_base<u8, 11, 8>;
extern template class dual_port_mailbox_ram_base<u16, 11, 16>;

#endif // MAME_MACHINE_MB8421_H
