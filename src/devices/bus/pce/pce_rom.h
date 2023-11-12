// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Charles MacDonald, Wilbert Pol, Angelo Salese
#ifndef MAME_BUS_PCE_PCE_ROM_H
#define MAME_BUS_PCE_PCE_ROM_H

#pragma once

#include "pce_slot.h"
#include "machine/nvram.h"


// ======================> pce_rom_device

class pce_rom_device : public device_t,
						public device_pce_cart_interface
{
public:
	// construction/destruction
	pce_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;

protected:
	pce_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};

// ======================> pce_cdsys3_device

class pce_cdsys3_device : public pce_rom_device
{
public:
	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ex(offs_t offset) override;

protected:
	// construction/destruction
	pce_cdsys3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool region);

	bool m_region;
};

// ======================> pce_cdsys3j_device

class pce_cdsys3j_device : public pce_cdsys3_device
{
public:
	// construction/destruction
	pce_cdsys3j_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> pce_cdsys3u_device

class pce_cdsys3u_device : public pce_cdsys3_device
{
public:
	// construction/destruction
	pce_cdsys3u_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> pce_populous_device

class pce_populous_device : public pce_rom_device
{
public:
	// construction/destruction
	pce_populous_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;
};

// ======================> pce_sf2_device

class pce_sf2_device : public pce_rom_device
{
public:
	// construction/destruction
	pce_sf2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_bank_base;
};

// ======================> pce_tennokoe_device

class pce_tennokoe_device : public pce_rom_device,
							public device_nvram_interface
{
public:
	// construction/destruction
	pce_tennokoe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	const uint32_t m_bram_size = 0x800*4;
	uint8_t m_bram[0x800*4];

	uint8_t m_bram_locked;
};

// ======================> pce_acard_pro_device

class pce_acard_pro_device : public pce_cdsys3_device
{
public:
	// construction/destruction
	pce_acard_pro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;
	virtual uint8_t peripheral_r(offs_t offset) override;
	virtual void peripheral_w(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	/* Arcade Card specific */
	std::unique_ptr<uint8_t[]>  m_dram;
	uint8_t   m_ctrl[4];
	uint32_t  m_base_addr[4];
	uint16_t  m_addr_offset[4];
	uint16_t  m_addr_inc[4];
	uint32_t  m_shift = 0;
	uint8_t   m_shift_reg = 0;
	uint8_t   m_rotate_reg = 0;
};


// device type definition
DECLARE_DEVICE_TYPE(PCE_ROM_STD,       pce_rom_device)
DECLARE_DEVICE_TYPE(PCE_ROM_CDSYS3J,   pce_cdsys3j_device)
DECLARE_DEVICE_TYPE(PCE_ROM_CDSYS3U,   pce_cdsys3u_device)
DECLARE_DEVICE_TYPE(PCE_ROM_POPULOUS,  pce_populous_device)
DECLARE_DEVICE_TYPE(PCE_ROM_SF2,       pce_sf2_device)
DECLARE_DEVICE_TYPE(PCE_ROM_TENNOKOE,  pce_tennokoe_device)
DECLARE_DEVICE_TYPE(PCE_ROM_ACARD_PRO, pce_acard_pro_device)


#endif // MAME_BUS_PCE_PCE_ROM_H
