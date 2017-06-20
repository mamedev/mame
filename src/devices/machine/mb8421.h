// license:BSD-3-Clause
// copyright-holders:hap
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
	devcb = &mb8421_device::set_intl_handler(*device, DEVCB_##_devcb);

#define MCFG_MB8421_INTR_HANDLER(_devcb) \
	devcb = &mb8421_device::set_intr_handler(*device, DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb8421_device

class mb8421_device : public device_t
{
public:
	mb8421_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template <class Object> static devcb_base &set_intl_handler(device_t &device, Object &&cb) { return downcast<mb8421_device &>(device).m_intl_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_intr_handler(device_t &device, Object &&cb) { return downcast<mb8421_device &>(device).m_intr_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ_LINE_MEMBER( busy_r ) { return 0; } // _BUSY pin - not emulated
	uint8_t peek(offs_t offset) { return m_ram[offset & 0x7ff]; }

	DECLARE_WRITE8_MEMBER( left_w );
	DECLARE_READ8_MEMBER( left_r );
	DECLARE_WRITE8_MEMBER( right_w );
	DECLARE_READ8_MEMBER( right_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_ram[0x800];

	devcb_write_line m_intl_handler;
	devcb_write_line m_intr_handler;
};

// device type definition
extern const device_type MB8421;
DECLARE_DEVICE_TYPE(MB8421, mb8421_device)

#endif // MAME_MACHINE_MB8421_H
