// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MEGADRIVE_ROM_H
#define MAME_BUS_MEGADRIVE_ROM_H

#pragma once

#include "md_slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> md_std_rom_device

class md_std_rom_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_std_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override { if (offset < 0x400000/2) return m_rom[MD_ADDR(offset)]; else return 0xffff; }
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { }

protected:
	md_std_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
};

// ======================> md_rom_sram_device

class md_rom_sram_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	md_rom_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

public:
	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;
};

// ======================> md_rom_sram_arg96_device

class md_rom_sram_arg96_device : public md_rom_sram_device
{
public:
	md_rom_sram_arg96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

// ======================> md_rom_fram_device

class md_rom_fram_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_fram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual uint16_t read_a13(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;
};

// ======================> md_rom_ssf2_device

class md_rom_ssf2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_ssf2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank[16];
	int m_lastoff, m_lastdata;
};

// ======================> md_rom_cm2in1_device

class md_rom_cm2in1_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_cm2in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int m_base;
};


// ======================> md_rom_mcpirate_device

class md_rom_mcpirate_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_mcpirate_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank;
};


// ======================> md_rom_bugslife_device

class md_rom_bugslife_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_bugslife_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_chinf3_device

class md_rom_chinf3_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_chinf3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int m_bank;
};

// ======================> md_rom_16mj2_device

class md_rom_16mj2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_16mj2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_elfwor_device

class md_rom_elfwor_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_elfwor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_yasech_device

class md_rom_yasech_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_yasech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_kof98_device

class md_rom_kof98_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_kof98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_kof99_device

class md_rom_kof99_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_kof99_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_lion2_device

class md_rom_lion2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_lion2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_prot1_data, m_prot2_data;
};

// ======================> md_rom_lion3_device

class md_rom_lion3_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_lion3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_reg[3];
	uint16_t m_bank;
};

// ======================> md_rom_mjlov_device

class md_rom_mjlov_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_mjlov_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_cjmjclub_device

class md_rom_cjmjclub_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_cjmjclub_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_pokea_device

class md_rom_pokea_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_pokea_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_pokestad_device

class md_rom_pokestad_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_pokestad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank;
};

// ======================> md_rom_realtec_device

class md_rom_realtec_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_realtec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_bank_addr, m_bank_size, m_old_bank_addr;
};

// ======================> md_rom_redcl_device

class md_rom_redcl_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_redcl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_rx3_device

class md_rom_rx3_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_rx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_sbubl_device

class md_rom_sbubl_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_sbubl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_smb_device

class md_rom_smb_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_smb2_device

class md_rom_smb2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_smw64_device

class md_rom_smw64_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smw64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint32_t m_latch0, m_latch1;
	uint16_t m_reg[6];
	uint16_t m_ctrl[3];
};

// ======================> md_rom_smouse_device

class md_rom_smouse_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};


// ======================> md_rom_soulb_device

class md_rom_soulb_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_soulb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_squir_device

class md_rom_squir_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_squir_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_latch;
};

// ======================> md_rom_tc2000_device

class md_rom_tc2000_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_tc2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_retvalue;
};


// ======================> md_rom_tekkensp_device

class md_rom_tekkensp_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_tekkensp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_reg;
};

// ======================> md_rom_topf_device

class md_rom_topf_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_topf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_latch;
	uint8_t m_bank[3];
};

// ======================> md_rom_radica_device

class md_rom_radica_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_radica_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual uint16_t read_a13(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank;
};

// ======================> md_rom_beggarp_device

class md_rom_beggarp_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_beggarp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_mode, m_lock;
};

// ======================> md_rom_wukong_device

class md_rom_wukong_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_wukong_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_mode;
};

// ======================> md_rom_starodys_device

class md_rom_starodys_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_starodys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual uint16_t read_a13(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_mode, m_lock, m_ram_enable, m_base;
};



// device type definition
DECLARE_DEVICE_TYPE(MD_STD_ROM,      md_std_rom_device)
DECLARE_DEVICE_TYPE(MD_ROM_SRAM,     md_rom_sram_device)
DECLARE_DEVICE_TYPE(MD_ROM_FRAM,     md_rom_fram_device)
DECLARE_DEVICE_TYPE(MD_ROM_CM2IN1,   md_rom_cm2in1_device)
DECLARE_DEVICE_TYPE(MD_ROM_SSF2,     md_rom_ssf2_device)
DECLARE_DEVICE_TYPE(MD_ROM_BUGSLIFE, md_rom_bugslife_device)
DECLARE_DEVICE_TYPE(MD_ROM_SMOUSE,   md_rom_smouse_device)
DECLARE_DEVICE_TYPE(MD_ROM_SMW64,    md_rom_smw64_device)
DECLARE_DEVICE_TYPE(MD_ROM_SMB,      md_rom_smb_device)
DECLARE_DEVICE_TYPE(MD_ROM_SMB2,     md_rom_smb2_device)
DECLARE_DEVICE_TYPE(MD_ROM_SBUBL,    md_rom_sbubl_device)
DECLARE_DEVICE_TYPE(MD_ROM_RX3,      md_rom_rx3_device)
DECLARE_DEVICE_TYPE(MD_ROM_MJLOV,    md_rom_mjlov_device)
DECLARE_DEVICE_TYPE(MD_ROM_CJMJCLUB, md_rom_cjmjclub_device)
DECLARE_DEVICE_TYPE(MD_ROM_KOF98,    md_rom_kof98_device)
DECLARE_DEVICE_TYPE(MD_ROM_KOF99,    md_rom_kof99_device)
DECLARE_DEVICE_TYPE(MD_ROM_SOULB,    md_rom_soulb_device)
DECLARE_DEVICE_TYPE(MD_ROM_CHINF3,   md_rom_chinf3_device)
DECLARE_DEVICE_TYPE(MD_ROM_16MJ2,    md_rom_16mj2_device)
DECLARE_DEVICE_TYPE(MD_ROM_ELFWOR,   md_rom_elfwor_device)
DECLARE_DEVICE_TYPE(MD_ROM_YASECH,   md_rom_yasech_device)
DECLARE_DEVICE_TYPE(MD_ROM_LION2,    md_rom_lion2_device)
DECLARE_DEVICE_TYPE(MD_ROM_LION3,    md_rom_lion3_device)
DECLARE_DEVICE_TYPE(MD_ROM_MCPIR,    md_rom_mcpirate_device)
DECLARE_DEVICE_TYPE(MD_ROM_POKEA,    md_rom_pokea_device)
DECLARE_DEVICE_TYPE(MD_ROM_POKESTAD, md_rom_pokestad_device)
DECLARE_DEVICE_TYPE(MD_ROM_REALTEC,  md_rom_realtec_device)
DECLARE_DEVICE_TYPE(MD_ROM_REDCL,    md_rom_redcl_device)
DECLARE_DEVICE_TYPE(MD_ROM_SQUIR,    md_rom_squir_device)
DECLARE_DEVICE_TYPE(MD_ROM_SRAM_ARG96, md_rom_sram_arg96_device)
DECLARE_DEVICE_TYPE(MD_ROM_TC2000,   md_rom_tc2000_device)
DECLARE_DEVICE_TYPE(MD_ROM_TEKKENSP, md_rom_tekkensp_device)
DECLARE_DEVICE_TYPE(MD_ROM_TOPF,     md_rom_topf_device)
DECLARE_DEVICE_TYPE(MD_ROM_RADICA,   md_rom_radica_device)
DECLARE_DEVICE_TYPE(MD_ROM_BEGGARP,  md_rom_beggarp_device)
DECLARE_DEVICE_TYPE(MD_ROM_WUKONG,   md_rom_wukong_device)
DECLARE_DEVICE_TYPE(MD_ROM_STARODYS, md_rom_starodys_device)

#endif // MAME_BUS_MEGADRIVE_ROM_H
