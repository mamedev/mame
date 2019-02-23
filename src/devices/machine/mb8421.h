// license:BSD-3-Clause
// copyright-holders:hap,AJR
/**********************************************************************

    Fujitsu MB8421/22/31/32-90/-90L/-90LL/-12/-12L/-12LL
    CMOS 16K-bit (2KB) dual-port SRAM

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

// ======================> mb8421_device

class mb8421_master_device : public device_t
{
public:
	// note: INT pins are only available on MB84x1
	// INTL is for the CPU on the left side, INTR for the one on the right
	auto intl_callback() { return m_intl_callback.bind(); }
	auto intr_callback() { return m_intr_callback.bind(); }

	DECLARE_READ_LINE_MEMBER(busy_r) { return 0; } // _BUSY pin - not emulated

protected:
	mb8421_master_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_reset() override;

	// internal helpers
	template<read_or_write row, bool is_right> void update_intr(offs_t offset);

private:
	devcb_write_line m_intl_callback;
	devcb_write_line m_intr_callback;
};

// ======================> mb8421_device

class mb8421_device : public mb8421_master_device
{
public:
	mb8421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u8 peek(offs_t offset) const { return m_ram[offset & 0x7ff]; }

	void left_w(offs_t offset, u8 data);
	u8 left_r(offs_t offset);
	void right_w(offs_t offset, u8 data);
	u8 right_r(offs_t offset);

protected:
	mb8421_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;

private:
	std::unique_ptr<u8[]> m_ram;
};

// ======================> mb8421_device

class idt71321_device : public mb8421_device
{
public:
	idt71321_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// ======================> mb8421_mb8431_16_device

class mb8421_mb8431_16_device : public mb8421_master_device
{
public:
	mb8421_mb8431_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u16 peek(offs_t offset) const { return m_ram[offset & 0x7ff]; }

	void left_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	u16 left_r(offs_t offset, u16 mem_mask = 0xffff);
	void right_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	u16 right_r(offs_t offset, u16 mem_mask = 0xffff);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	std::unique_ptr<u16[]> m_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(MB8421, mb8421_device)
DECLARE_DEVICE_TYPE(IDT71321, idt71321_device)
DECLARE_DEVICE_TYPE(MB8421_MB8431_16BIT, mb8421_mb8431_16_device)

#endif // MAME_MACHINE_MB8421_H
