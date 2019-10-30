// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
#ifndef MAME_BUS_GAMEBOY_MBC_H
#define MAME_BUS_GAMEBOY_MBC_H

#include "gb_slot.h"


// ======================> gb_rom_mbc_device

class gb_rom_mbc_device : public device_t,
						public device_gb_cart_interface
{
public:
	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// construction/destruction
	gb_rom_mbc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }

	void shared_start();
	void shared_reset();

	uint8_t m_ram_enable;
};

// ======================> gb_rom_mbc1_device

class gb_rom_mbc1_device : public gb_rom_mbc_device
{
public:

	// construction/destruction
	gb_rom_mbc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	enum {
		MODE_16M_64k = 0, /// 16Mbit ROM, 64kBit RAM
		MODE_4M_256k = 1  /// 4Mbit ROM, 256kBit RAM
	};

	gb_rom_mbc1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); save_item(NAME(m_mode)); }
	virtual void device_reset() override { shared_reset(); m_mode = MODE_16M_64k; }
	virtual void set_additional_wirings(uint8_t mask, int shift) override { m_mask = mask; m_shift = shift; }  // these get set at cart loading

	uint8_t m_mode, m_mask;
	int m_shift;
};

// ======================> gb_rom_mbc2_device

class gb_rom_mbc2_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
};

// ======================> gb_rom_mbc3_device

class gb_rom_mbc3_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

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
	gb_rom_mbc5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	gb_rom_mbc5_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); m_rumble.resolve(); }
	virtual void device_reset() override { shared_reset(); }

	output_finder<> m_rumble;
};

// ======================> gb_rom_mbc6_device

class gb_rom_mbc6_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	uint16_t m_latch1, m_latch2;
	uint8_t m_bank_4000, m_bank_6000;
};

// ======================> gb_rom_mbc7_device

class gb_rom_mbc7_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mbc7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
};

// ======================> gb_rom_m161_device

class gb_rom_m161_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_m161_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override { return 0xff; }
	virtual void write_ram(offs_t offset, uint8_t data) override { }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t m_base_bank;
	uint8_t m_load_disable;
};

// ======================> gb_rom_mmm01_device
class gb_rom_mmm01_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_mmm01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

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
	// construction/destruction
	gb_rom_sachen_mmc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override { return 0xff; }
	virtual void write_ram(offs_t offset, uint8_t data) override { }

protected:
	enum {
		MODE_LOCKED,
		MODE_UNLOCKED
	};

	gb_rom_sachen_mmc1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t m_base_bank, m_mask, m_mode, m_unlock_cnt;
};

// ======================> gb_rom_sachen_mmc2_device

class gb_rom_sachen_mmc2_device : public gb_rom_sachen_mmc1_device
{
public:
	// construction/destruction
	gb_rom_sachen_mmc2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	enum {
		MODE_LOCKED_DMG,
		MODE_LOCKED_CGB,
		MODE_UNLOCKED
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// ======================> gb_rom_188in1_device
class gb_rom_188in1_device : public gb_rom_mbc1_device
{
public:
	// construction/destruction
	gb_rom_188in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); save_item(NAME(m_game_base)); }
	virtual void device_reset() override { shared_reset(); m_game_base = 0; }

private:
	uint32_t m_game_base;
};

// ======================> gb_rom_sintax_device
class gb_rom_sintax_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_sintax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	void set_xor_for_bank(uint8_t bank);

	uint8_t m_bank_mask, m_bank, m_reg;

	uint8_t m_currentxor, m_xor2, m_xor3, m_xor4, m_xor5, m_sintax_mode;
};

// ======================> gb_rom_chongwu_device

class gb_rom_chongwu_device : public gb_rom_mbc5_device
{
public:
	// construction/destruction
	gb_rom_chongwu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t m_protection_checked;
};

// ======================> gb_rom_licheng_device

class gb_rom_licheng_device : public gb_rom_mbc5_device
{
public:
	// construction/destruction
	gb_rom_licheng_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_bank(offs_t offset, uint8_t data) override;
};

// ======================> gb_rom_digimon_device

class gb_rom_digimon_device : public gb_rom_mbc5_device
{
public:
	// construction/destruction
	gb_rom_digimon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
	virtual void write_ram(offs_t offset, uint8_t data) override;
};

// ======================> gb_rom_rockman8_device
class gb_rom_rockman8_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_rockman8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }

	uint8_t m_bank_mask, m_bank, m_reg;
};

// ======================> gb_rom_sm3sp_device
class gb_rom_sm3sp_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_sm3sp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }

	uint8_t m_bank_mask, m_bank, m_reg, m_mode;
};

// ======================> gb_rom_camera_device
class gb_rom_camera_device : public gb_rom_mbc_device
{
public:
	// construction/destruction
	gb_rom_camera_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void update_camera();
	uint8_t m_camera_regs[54];
};



// device type definition
DECLARE_DEVICE_TYPE(GB_ROM_MBC1,     gb_rom_mbc1_device)
DECLARE_DEVICE_TYPE(GB_ROM_MBC2,     gb_rom_mbc2_device)
DECLARE_DEVICE_TYPE(GB_ROM_MBC3,     gb_rom_mbc3_device)
DECLARE_DEVICE_TYPE(GB_ROM_MBC5,     gb_rom_mbc5_device)
DECLARE_DEVICE_TYPE(GB_ROM_MBC6,     gb_rom_mbc6_device)
DECLARE_DEVICE_TYPE(GB_ROM_MBC7,     gb_rom_mbc7_device)
DECLARE_DEVICE_TYPE(GB_ROM_M161,     gb_rom_m161_device)
DECLARE_DEVICE_TYPE(GB_ROM_MMM01,    gb_rom_mmm01_device)
DECLARE_DEVICE_TYPE(GB_ROM_SACHEN1,  gb_rom_sachen_mmc1_device)
DECLARE_DEVICE_TYPE(GB_ROM_SACHEN2,  gb_rom_sachen_mmc2_device)
DECLARE_DEVICE_TYPE(GB_ROM_188IN1,   gb_rom_188in1_device)
DECLARE_DEVICE_TYPE(GB_ROM_SINTAX,   gb_rom_sintax_device)
DECLARE_DEVICE_TYPE(GB_ROM_CHONGWU,  gb_rom_chongwu_device)
DECLARE_DEVICE_TYPE(GB_ROM_LICHENG,  gb_rom_licheng_device)
DECLARE_DEVICE_TYPE(GB_ROM_DIGIMON,  gb_rom_digimon_device)
DECLARE_DEVICE_TYPE(GB_ROM_ROCKMAN8, gb_rom_rockman8_device)
DECLARE_DEVICE_TYPE(GB_ROM_SM3SP,    gb_rom_sm3sp_device)
DECLARE_DEVICE_TYPE(GB_ROM_CAMERA,   gb_rom_camera_device)

#endif // MAME_BUS_GAMEBOY_MBC_H
