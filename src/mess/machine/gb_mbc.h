#ifndef __GB_MBC_H
#define __GB_MBC_H

#include "machine/gb_slot.h"


// ======================> gb_rom_mbc_device

class gb_rom_mbc_device : public device_t,
						public device_gb_cart_interface
{
public:
	// construction/destruction
	gb_rom_mbc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };
	virtual void device_config_complete() { m_shortname = "gb_rom_mbc_base"; }

	void shared_start();
	void shared_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);

	UINT8 m_ram_enable;
	UINT8 m_mode;
};

// ======================> gb_rom_mbc1_device

class gb_rom_mbc1_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };
	virtual void device_config_complete() { m_shortname = "gb_rom_mbc1"; }

	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
};

// ======================> gb_rom_mbc1col_device

class gb_rom_mbc1col_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc1col_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };
	virtual void device_config_complete() { m_shortname = "gb_rom_mbc1col"; }

	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
};

// ======================> gb_rom_mbc2_device

class gb_rom_mbc2_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };
	virtual void device_config_complete() { m_shortname = "gb_rom_mbc2"; }

	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
};

// ======================> gb_rom_mbc3_device

class gb_rom_mbc3_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "gb_rom_mbc3"; }

	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	UINT8 m_rtc_map[5];
};

// ======================> gb_rom_mbc5_device

class gb_rom_mbc5_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc5_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	gb_rom_mbc5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };
	virtual void device_config_complete() { m_shortname = "gb_rom_mbc5"; }

	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
};

// ======================> gb_rom_mbc6_device

class gb_rom_mbc6_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc6_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "gb_rom_mbc6"; }

	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	UINT16 m_latch1, m_latch2;
	UINT8 m_bank_4000, m_bank_6000;
};

// ======================> gb_rom_mbc7_device

class gb_rom_mbc7_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc7_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };
	virtual void device_config_complete() { m_shortname = "gb_rom_mbc7"; }

	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
};

// ======================> gb_rom_mmm01_device
class gb_rom_mmm01_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mmm01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "gb_rom_mmm01"; }

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	UINT8 m_bank_mask, m_bank, m_reg;
};

// ======================> gb_rom_sintax_device
class gb_rom_sintax_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_sintax_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "gb_rom_sintax"; }
	void set_xor_for_bank(UINT8 bank);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	UINT8 m_bank_mask, m_bank, m_reg;

	UINT8 m_currentxor, m_xor2, m_xor3, m_xor4, m_xor5, m_sintax_mode;
};

// ======================> gb_rom_chongwu_device

class gb_rom_chongwu_device : public gb_rom_mbc5_device
{
public:
	// construction/destruction
	gb_rom_chongwu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "gb_rom_chongwu"; }

	virtual DECLARE_READ8_MEMBER(read_rom);
	UINT8 m_protection_checked;
};

// ======================> gb_rom_digimon_device

class gb_rom_digimon_device : public gb_rom_mbc5_device
{
public:
	// construction/destruction
	gb_rom_digimon_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };
	virtual void device_config_complete() { m_shortname = "gb_rom_digimon"; }

	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
};

// ======================> gb_rom_rockman8_device
class gb_rom_rockman8_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_rockman8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };
	virtual void device_config_complete() { m_shortname = "gb_rom_rockman8"; }

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	UINT8 m_bank_mask, m_bank, m_reg;
};

// ======================> gb_rom_sm3sp_device
class gb_rom_sm3sp_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_sm3sp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { shared_start(); };
	virtual void device_reset() { shared_reset(); };
	virtual void device_config_complete() { m_shortname = "gb_rom_sm3sp"; }

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	UINT8 m_bank_mask, m_bank, m_reg;
};



// device type definition
extern const device_type GB_ROM_MBC1;
extern const device_type GB_ROM_MBC1_COL;
extern const device_type GB_ROM_MBC2;
extern const device_type GB_ROM_MBC3;
extern const device_type GB_ROM_MBC4;
extern const device_type GB_ROM_MBC5;
extern const device_type GB_ROM_MBC6;
extern const device_type GB_ROM_MBC7;
extern const device_type GB_ROM_MMM01;
extern const device_type GB_ROM_SINTAX;
extern const device_type GB_ROM_CHONGWU;
extern const device_type GB_ROM_DIGIMON;
extern const device_type GB_ROM_ROCKMAN8;
extern const device_type GB_ROM_SM3SP;

#endif
