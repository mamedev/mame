// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
#ifndef __GB_ROM_H
#define __GB_ROM_H

#include "gb_slot.h"


// ======================> gb_rom_device

class gb_rom_device : public device_t,
						public device_gb_cart_interface
{
public:
	// construction/destruction
	gb_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	gb_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };

	void shared_start();
	void shared_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
};

// ======================> gb_rom_tama5_device
class gb_rom_tama5_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_tama5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	UINT16 m_tama5_data, m_tama5_addr, m_tama5_cmd;
	UINT8 m_regs[32];
	UINT8 m_rtc_reg;
};

// ======================> gb_rom_wisdom_device
class gb_rom_wisdom_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_wisdom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
};

// ======================> gb_rom_yong_device
class gb_rom_yong_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_yong_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
};

// ======================> gb_rom_atvrac_device
class gb_rom_atvrac_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_atvrac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
};

// ======================> gb_rom_lasama_device
class gb_rom_lasama_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_lasama_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
};


// ======================> megaduck_rom_device
class megaduck_rom_device :public device_t,
						public device_gb_cart_interface
{
public:
	// construction/destruction
	megaduck_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	megaduck_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
};

// device type definition
extern const device_type GB_STD_ROM;
extern const device_type GB_ROM_TAMA5;
extern const device_type GB_ROM_WISDOM;
extern const device_type GB_ROM_YONG;
extern const device_type GB_ROM_ATVRAC;
extern const device_type GB_ROM_LASAMA;

extern const device_type MEGADUCK_ROM;

#endif
