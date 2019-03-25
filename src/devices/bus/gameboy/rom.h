// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
#ifndef MAME_BUS_GAMEBOY_ROM_H
#define MAME_BUS_GAMEBOY_ROM_H

#include "gb_slot.h"


// ======================> gb_rom_device

class gb_rom_device : public device_t,
						public device_gb_cart_interface
{
public:
	// construction/destruction
	gb_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	gb_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }

	void shared_start();
	void shared_reset();
};

// ======================> gb_rom_tama5_device
class gb_rom_tama5_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_tama5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	uint16_t m_tama5_data, m_tama5_addr, m_tama5_cmd;
	uint8_t m_regs[32];
	uint8_t m_rtc_reg;
};

// ======================> gb_rom_wisdom_device
class gb_rom_wisdom_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_wisdom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
};

// ======================> gb_rom_yong_device
class gb_rom_yong_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_yong_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
};

// ======================> gb_rom_atvrac_device
class gb_rom_atvrac_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_atvrac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
};

// ======================> gb_rom_lasama_device
class gb_rom_lasama_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_lasama_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
};


// ======================> megaduck_rom_device
class megaduck_rom_device :public device_t,
						public device_gb_cart_interface
{
public:
	// construction/destruction
	megaduck_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	megaduck_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// device type definition
DECLARE_DEVICE_TYPE(GB_STD_ROM,    gb_rom_device)
DECLARE_DEVICE_TYPE(GB_ROM_TAMA5,  gb_rom_tama5_device)
DECLARE_DEVICE_TYPE(GB_ROM_WISDOM, gb_rom_wisdom_device)
DECLARE_DEVICE_TYPE(GB_ROM_YONG,   gb_rom_yong_device)
DECLARE_DEVICE_TYPE(GB_ROM_ATVRAC, gb_rom_atvrac_device)
DECLARE_DEVICE_TYPE(GB_ROM_LASAMA, gb_rom_lasama_device)

DECLARE_DEVICE_TYPE(MEGADUCK_ROM,  megaduck_rom_device)

#endif // MAME_BUS_GAMEBOY_ROM_H
