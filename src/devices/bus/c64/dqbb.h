// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Brown Boxes Double Quick Brown Box emulation

**********************************************************************/

#ifndef MAME_BUS_C64_DQBB_H
#define MAME_BUS_C64_DQBB_H

#pragma once


#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_dqbb_cartridge_device

class c64_dqbb_cartridge_device : public device_t,
									public device_c64_expansion_card_interface,
									public device_nvram_interface
{
public:
	// construction/destruction
	c64_dqbb_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER( mode_changed );

protected:
	c64_dqbb_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, size_t ram_size);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_c64_expansion_card_interface implementation
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	void update_exrom();

	required_ioport m_mode;

	size_t const m_ram_size;

	int m_cs;
	int m_we;
	bool m_exrom_bit;
	u8 m_bank;
};


// ======================> c64_dqbb32_cartridge_device

class c64_dqbb32_cartridge_device : public c64_dqbb_cartridge_device
{
public:
	// construction/destruction
	c64_dqbb32_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> c64_dqbb64_cartridge_device

class c64_dqbb64_cartridge_device : public c64_dqbb_cartridge_device
{
public:
	// construction/destruction
	c64_dqbb64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> c64_dqbb128_cartridge_device

class c64_dqbb128_cartridge_device : public c64_dqbb_cartridge_device
{
public:
	// construction/destruction
	c64_dqbb128_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> c64_dqbb256_cartridge_device

class c64_dqbb256_cartridge_device : public c64_dqbb_cartridge_device
{
public:
	// construction/destruction
	c64_dqbb256_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definitions
DECLARE_DEVICE_TYPE(C64_DQBB, c64_dqbb_cartridge_device)
DECLARE_DEVICE_TYPE(C64_DQBB32, c64_dqbb32_cartridge_device)
DECLARE_DEVICE_TYPE(C64_DQBB64, c64_dqbb64_cartridge_device)
DECLARE_DEVICE_TYPE(C64_DQBB128, c64_dqbb128_cartridge_device)
DECLARE_DEVICE_TYPE(C64_DQBB256, c64_dqbb256_cartridge_device)


#endif // MAME_BUS_C64_DQBB_H
