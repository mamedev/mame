// license:BSD-3-Clause
// copyright-holders:hap,AJR
/**********************************************************************

    Fujitsu MB8421/22/31/32-90/-90L/-90LL/-12/-12L/-12LL
    CMOS 16K-bit (2KB) dual-port SRAM

**********************************************************************/

#ifndef MAME_MACHINE_MB8421_H
#define MAME_MACHINE_MB8421_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

// note: INT pins are only available on MB84x1
// INTL is for the CPU on the left side, INTR for the one on the right
#define MCFG_MB8421_INTL_HANDLER(_devcb) \
	devcb = &downcast<mb8421_master_device &>(*device).set_intl_handler(DEVCB_##_devcb);

#define MCFG_MB8421_INTR_HANDLER(_devcb) \
	devcb = &downcast<mb8421_master_device &>(*device).set_intr_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb8421_device

class mb8421_master_device : public device_t
{
public:
	// configuration helpers
	template <class Object> devcb_base &set_intl_handler(Object &&cb) { return m_intl_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_intr_handler(Object &&cb) { return m_intr_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ_LINE_MEMBER(busy_r) { return 0; } // _BUSY pin - not emulated

protected:
	mb8421_master_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal helpers
	template<read_or_write row, bool is_right> void update_intr(offs_t offset);

private:
	devcb_write_line m_intl_handler;
	devcb_write_line m_intr_handler;
};

// ======================> mb8421_device

class mb8421_device : public mb8421_master_device
{
public:
	mb8421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 peek(offs_t offset) { return m_ram[offset & 0x7ff]; }

	DECLARE_WRITE8_MEMBER(left_w);
	DECLARE_READ8_MEMBER(left_r);
	DECLARE_WRITE8_MEMBER(right_w);
	DECLARE_READ8_MEMBER(right_r);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	std::unique_ptr<u8[]> m_ram;
};

// ======================> mb8421_mb8431_16_device

class mb8421_mb8431_16_device : public mb8421_master_device
{
public:
	mb8421_mb8431_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u16 peek(offs_t offset) { return m_ram[offset & 0x7ff]; }

	DECLARE_WRITE16_MEMBER(left_w);
	DECLARE_READ16_MEMBER(left_r);
	DECLARE_WRITE16_MEMBER(right_w);
	DECLARE_READ16_MEMBER(right_r);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	std::unique_ptr<u16[]> m_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(MB8421, mb8421_device)
DECLARE_DEVICE_TYPE(MB8421_MB8431_16BIT, mb8421_mb8431_16_device)

#endif // MAME_MACHINE_MB8421_H
