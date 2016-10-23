// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
#ifndef __GB_MBC_H
#define __GB_MBC_H

#include "gb_slot.h"


// ======================> gb_rom_mbc_device

class gb_rom_mbc_device : public device_t,
						public device_gb_cart_interface
{
public:
	// construction/destruction
	gb_rom_mbc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override { shared_start(); };
	virtual void device_reset() override { shared_reset(); };

	void shared_start();
	void shared_reset();

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	uint8_t m_ram_enable;
};

// ======================> gb_rom_mbc1_device

class gb_rom_mbc1_device : public gb_rom_mbc_device
{
public:

	enum {
		MODE_16M_64k = 0, /// 16Mbit ROM, 64kBit RAM
		MODE_4M_256k = 1  /// 4Mbit ROM, 256kBit RAM
	};

	// construction/destruction
	gb_rom_mbc1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	gb_rom_mbc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); save_item(NAME(m_mode)); };
	virtual void device_reset() override { shared_reset(); m_mode = MODE_16M_64k; };
	virtual void set_additional_wirings(uint8_t mask, int shift) override { m_mask = mask; m_shift = shift; }  // these get set at cart loading

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	uint8_t m_mode, m_mask;
	int m_shift;
};

// ======================> gb_rom_mbc2_device

class gb_rom_mbc2_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); };
	virtual void device_reset() override { shared_reset(); };

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};

// ======================> gb_rom_mbc3_device

class gb_rom_mbc3_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	void update_rtc();
	uint8_t m_rtc_regs[5];
	int m_rtc_ready;
};

// ======================> gb_rom_mbc5_device

class gb_rom_mbc5_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc5_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	gb_rom_mbc5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); };
	virtual void device_reset() override { shared_reset(); };

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};

// ======================> gb_rom_mbc6_device

class gb_rom_mbc6_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	uint16_t m_latch1, m_latch2;
	uint8_t m_bank_4000, m_bank_6000;
};

// ======================> gb_rom_mbc7_device

class gb_rom_mbc7_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); };
	virtual void device_reset() override { shared_reset(); };

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};

// ======================> gb_rom_m161_device

class gb_rom_m161_device : public gb_rom_mbc_device
{
public:

	// construction/destruction
	gb_rom_m161_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override { return 0xff; }
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { }

	uint8_t m_base_bank;
	uint8_t m_load_disable;
};

// ======================> gb_rom_mmm01_device
class gb_rom_mmm01_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mmm01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	uint16_t m_romb;
	uint8_t  m_romb_nwe;
	uint8_t  m_ramb;
	uint8_t  m_ramb_nwe;
	uint8_t  m_mode;
	uint8_t  m_mode_nwe;
	uint8_t  m_map;
	uint8_t  m_mux;
};

// ======================> gb_rom_sachen_mmc1_device

class gb_rom_sachen_mmc1_device : public gb_rom_mbc_device
{
public:

	enum {
		MODE_LOCKED,
		MODE_UNLOCKED
	};

	// construction/destruction
	gb_rom_sachen_mmc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	gb_rom_sachen_mmc1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override { return 0xff; }
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { }

	uint8_t m_base_bank, m_mask, m_mode, m_unlock_cnt;
};

// ======================> gb_rom_sachen_mmc2_device

class gb_rom_sachen_mmc2_device : public gb_rom_sachen_mmc1_device
{
public:

	enum {
		MODE_LOCKED_DMG,
		MODE_LOCKED_CGB,
		MODE_UNLOCKED
	};

	// construction/destruction
	gb_rom_sachen_mmc2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

};

// ======================> gb_rom_188in1_device
class gb_rom_188in1_device : public gb_rom_mbc1_device
{
public:
	// construction/destruction
	gb_rom_188in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); save_item(NAME(m_game_base)); };
	virtual void device_reset() override { shared_reset(); m_game_base = 0; };

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	uint32_t m_game_base;
};

// ======================> gb_rom_sintax_device
class gb_rom_sintax_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_sintax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	void set_xor_for_bank(uint8_t bank);

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	uint8_t m_bank_mask, m_bank, m_reg;

	uint8_t m_currentxor, m_xor2, m_xor3, m_xor4, m_xor5, m_sintax_mode;
};

// ======================> gb_rom_chongwu_device

class gb_rom_chongwu_device : public gb_rom_mbc5_device
{
public:
	// construction/destruction
	gb_rom_chongwu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	uint8_t m_protection_checked;
};

// ======================> gb_rom_licheng_device

class gb_rom_licheng_device : public gb_rom_mbc5_device
{
public:
	// construction/destruction
	gb_rom_licheng_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};

// ======================> gb_rom_digimon_device

class gb_rom_digimon_device : public gb_rom_mbc5_device
{
public:
	// construction/destruction
	gb_rom_digimon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); };
	virtual void device_reset() override { shared_reset(); };

	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};

// ======================> gb_rom_rockman8_device
class gb_rom_rockman8_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_rockman8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); };
	virtual void device_reset() override { shared_reset(); };

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	uint8_t m_bank_mask, m_bank, m_reg;
};

// ======================> gb_rom_sm3sp_device
class gb_rom_sm3sp_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_sm3sp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); };
	virtual void device_reset() override { shared_reset(); };

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	uint8_t m_bank_mask, m_bank, m_reg, m_mode;
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
extern const device_type GB_ROM_M161;
extern const device_type GB_ROM_MMM01;
extern const device_type GB_ROM_SACHEN1;
extern const device_type GB_ROM_SACHEN2;
extern const device_type GB_ROM_188IN1;
extern const device_type GB_ROM_SINTAX;
extern const device_type GB_ROM_CHONGWU;
extern const device_type GB_ROM_LICHENG;
extern const device_type GB_ROM_DIGIMON;
extern const device_type GB_ROM_ROCKMAN8;
extern const device_type GB_ROM_SM3SP;

#endif
