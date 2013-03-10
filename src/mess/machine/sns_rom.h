#ifndef __SNS_ROM_H
#define __SNS_ROM_H

#include "machine/sns_slot.h"


// ======================> sns_rom_device

class sns_rom_device : public device_t,
						public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	sns_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "sns_rom"; }

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_h);
};

// ======================> sns_rom_pokemon_device

class sns_rom_pokemon_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_pokemon_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "sns_rom_pokemon"; }

	// reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read);    // protection device
	virtual DECLARE_WRITE8_MEMBER(chip_write);  // protection device
	UINT8 m_latch;
};

// ======================> sns_rom_obc1_device

class sns_rom_obc1_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_obc1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "sns_rom_obc1"; }

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read);
	virtual DECLARE_WRITE8_MEMBER(chip_write);

	int m_address;
	int m_offset;
	int m_shift;
	UINT8 m_ram[0x2000];
};


// device type definition
extern const device_type SNS_LOROM;
extern const device_type SNS_LOROM_OBC1;
extern const device_type SNS_LOROM_POKEMON;

#endif
