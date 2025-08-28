// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 MegaDrive / Genesis cart emulation


 Here we emulate bankswitch / protection / NVRAM found on generic carts with no additional hardware


 Emulation of the pirate carts is heavily indebted to the reverse engineering efforts made
 by David Haywood (for HazeMD) and by EkeEke (for genplus-gx)

 ***********************************************************************************************************/

#include "emu.h"
#include "rom.h"
#include "cpu/m68000/m68000.h"


//-------------------------------------------------
//  md_rom_device - constructor
//-------------------------------------------------

// BASE CARTS + NVRAM
DEFINE_DEVICE_TYPE(MD_STD_ROM,      md_std_rom_device,      "md_std_rom",      "MD Standard cart")
DEFINE_DEVICE_TYPE(MD_ROM_SRAM,     md_rom_sram_device,     "md_rom_sram",     "MD Standard cart + SRAM")
DEFINE_DEVICE_TYPE(MD_ROM_FRAM,     md_rom_fram_device,     "md_rom_fram",     "MD Standard cart + FRAM")

// BASE CARTS + BANKSWITCH AT RESET
DEFINE_DEVICE_TYPE(MD_ROM_CM2IN1,   md_rom_cm2in1_device,   "md_rom_cm2in1",   "MD Codemasters 2in1")

// BASE CARTS + PROTECTION / BANKSWITCH
DEFINE_DEVICE_TYPE(MD_ROM_SSF2,     md_rom_ssf2_device,     "md_rom_ssf2",     "MD Super SF2")
DEFINE_DEVICE_TYPE(MD_ROM_BUGSLIFE, md_rom_bugslife_device, "md_rom_bugslife", "MD A Bug's Life")
DEFINE_DEVICE_TYPE(MD_ROM_SMOUSE,   md_rom_smouse_device,   "md_rom_smouse",   "MD Huan Le Tao Qi Shu / Smart Mouse")
DEFINE_DEVICE_TYPE(MD_ROM_SMW64,    md_rom_smw64_device,    "md_rom_smw64",    "MD Super Mario World 64")
DEFINE_DEVICE_TYPE(MD_ROM_SMB,      md_rom_smb_device,      "md_rom_smb",      "MD Super Mario Bros.")
DEFINE_DEVICE_TYPE(MD_ROM_SMB2,     md_rom_smb2_device,     "md_rom_smb2",     "MD Super Mario Bros. 2")
DEFINE_DEVICE_TYPE(MD_ROM_SBUBL,    md_rom_sbubl_device,    "md_rom_sbubl",    "MD Super Bubble Bobble")
DEFINE_DEVICE_TYPE(MD_ROM_RX3,      md_rom_rx3_device,      "md_rom_rx3",      "MD Rockman X3")
DEFINE_DEVICE_TYPE(MD_ROM_MJLOV,    md_rom_mjlov_device,    "md_rom_mjlov",    "MD Majiang Qingren / Mahjong Lover")
DEFINE_DEVICE_TYPE(MD_ROM_CJMJCLUB, md_rom_cjmjclub_device, "md_rom_cjmjclub", "MD Chaoji Majiang Club / Super Mahjong Club")
DEFINE_DEVICE_TYPE(MD_ROM_KOF98,    md_rom_kof98_device,    "md_rom_kof98",    "MD KOF 98")
DEFINE_DEVICE_TYPE(MD_ROM_KOF99,    md_rom_kof99_device,    "md_rom_kof99",    "MD KOF 99") // and others
DEFINE_DEVICE_TYPE(MD_ROM_SOULB,    md_rom_soulb_device,    "md_rom_soulb",    "MD Soul Blade")
DEFINE_DEVICE_TYPE(MD_ROM_CHINF3,   md_rom_chinf3_device,   "md_rom_chinf3",   "MD Chinese Fighter 3")
DEFINE_DEVICE_TYPE(MD_ROM_16MJ2,    md_rom_16mj2_device,    "md_rom_16mj2",    "MD 16 Majong Tiles II")
DEFINE_DEVICE_TYPE(MD_ROM_ELFWOR,   md_rom_elfwor_device,   "md_rom_elfwor",   "MD Linghuan Daoshi Super Magician / Elf Wor")
DEFINE_DEVICE_TYPE(MD_ROM_YASECH,   md_rom_yasech_device,   "md_rom_yasech",   "MD Ya Se Chuan Shuo")
DEFINE_DEVICE_TYPE(MD_ROM_LION2,    md_rom_lion2_device,    "md_rom_lion2",    "MD Lion King 2")
DEFINE_DEVICE_TYPE(MD_ROM_LION3,    md_rom_lion3_device,    "md_rom_lion3",    "MD Lion King 3")
DEFINE_DEVICE_TYPE(MD_ROM_MCPIR,    md_rom_mcpirate_device, "md_rom_mcpirate", "MD Pirate Multicarts (various)")
DEFINE_DEVICE_TYPE(MD_ROM_POKEA,    md_rom_pokea_device,    "md_rom_pokea",    "MD Pokemon (alt protection)")
DEFINE_DEVICE_TYPE(MD_ROM_POKESTAD, md_rom_pokestad_device, "md_rom_pokestad", "MD Pokemon Stadium")
DEFINE_DEVICE_TYPE(MD_ROM_REALTEC,  md_rom_realtec_device,  "md_rom_realtec",  "MD Realtec")
DEFINE_DEVICE_TYPE(MD_ROM_REDCL,    md_rom_redcl_device,    "md_rom_redcl",    "MD Redcliff")
DEFINE_DEVICE_TYPE(MD_ROM_SQUIR,    md_rom_squir_device,    "md_rom_squir",    "MD Squirrel King")
DEFINE_DEVICE_TYPE(MD_ROM_TC2000,   md_rom_tc2000_device,   "md_rom_tc2000",   "MD TC2000")
DEFINE_DEVICE_TYPE(MD_ROM_TEKKENSP, md_rom_tekkensp_device, "md_rom_tekkensp", "MD Tekken Special")
DEFINE_DEVICE_TYPE(MD_ROM_TOPF,     md_rom_topf_device,     "md_rom_topf",     "MD Top Fighter")
DEFINE_DEVICE_TYPE(MD_ROM_RADICA,   md_rom_radica_device,   "md_rom_radica",   "MD Radica TV games")
DEFINE_DEVICE_TYPE(MD_ROM_BEGGARP,  md_rom_beggarp_device,  "md_rom_beggarp",  "MD Beggar Prince")
DEFINE_DEVICE_TYPE(MD_ROM_WUKONG,   md_rom_wukong_device,   "md_rom_wukong",   "MD Legend of Wukong")
DEFINE_DEVICE_TYPE(MD_ROM_STARODYS, md_rom_starodys_device, "md_rom_starodys", "MD Star Odyssey")
DEFINE_DEVICE_TYPE(MD_ROM_SRAM_ARG96, md_rom_sram_arg96_device, "md_rom_sram_arg96", "MD Futbol Argentino 96")


md_std_rom_device::md_std_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_md_cart_interface(mconfig, *this)
{
}

md_std_rom_device::md_std_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_STD_ROM, tag, owner, clock)
{
}

md_rom_sram_device::md_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_rom_sram_device(mconfig, MD_ROM_SRAM, tag, owner, clock)
{
}

md_rom_sram_device::md_rom_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, type, tag, owner, clock)
{
}


md_rom_sram_arg96_device::md_rom_sram_arg96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_rom_sram_device(mconfig, MD_ROM_SRAM_ARG96, tag, owner, clock)
{
}


md_rom_fram_device::md_rom_fram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_FRAM, tag, owner, clock)
{
}

md_rom_cm2in1_device::md_rom_cm2in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_CM2IN1, tag, owner, clock), m_base(0)
{
}

md_rom_ssf2_device::md_rom_ssf2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_SSF2, tag, owner, clock), m_lastoff(0), m_lastdata(0)
{
}

md_rom_bugslife_device::md_rom_bugslife_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_BUGSLIFE, tag, owner, clock)
{
}

md_rom_smouse_device::md_rom_smouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_SMOUSE, tag, owner, clock)
{
}

md_rom_smw64_device::md_rom_smw64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_SMW64, tag, owner, clock), m_latch0(0), m_latch1(0)
{
}

md_rom_smb_device::md_rom_smb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_SMB, tag, owner, clock)
{
}

md_rom_smb2_device::md_rom_smb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_SMB2, tag, owner, clock)
{
}

md_rom_sbubl_device::md_rom_sbubl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_SBUBL, tag, owner, clock)
{
}

md_rom_rx3_device::md_rom_rx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_RX3, tag, owner, clock)
{
}

md_rom_mjlov_device::md_rom_mjlov_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_MJLOV, tag, owner, clock)
{
}

md_rom_cjmjclub_device::md_rom_cjmjclub_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_CJMJCLUB, tag, owner, clock)
{
}

md_rom_kof98_device::md_rom_kof98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_KOF98, tag, owner, clock)
{
}

md_rom_kof99_device::md_rom_kof99_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_KOF99, tag, owner, clock)
{
}

md_rom_soulb_device::md_rom_soulb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_SOULB, tag, owner, clock)
{
}

md_rom_chinf3_device::md_rom_chinf3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_CHINF3, tag, owner, clock), m_bank(0)
{
}

md_rom_16mj2_device::md_rom_16mj2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_16MJ2, tag, owner, clock)
{
}

md_rom_elfwor_device::md_rom_elfwor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_ELFWOR, tag, owner, clock)
{
}

md_rom_yasech_device::md_rom_yasech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_YASECH, tag, owner, clock)
{
}

md_rom_lion2_device::md_rom_lion2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_LION2, tag, owner, clock), m_prot1_data(0), m_prot2_data(0)
{
}

md_rom_lion3_device::md_rom_lion3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_LION3, tag, owner, clock), m_bank(0)
{
}

md_rom_mcpirate_device::md_rom_mcpirate_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_MCPIR, tag, owner, clock), m_bank(0)
{
}

md_rom_pokea_device::md_rom_pokea_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_POKEA, tag, owner, clock)
{
}

md_rom_pokestad_device::md_rom_pokestad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_POKESTAD, tag, owner, clock), m_bank(0)
{
}

md_rom_realtec_device::md_rom_realtec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_REALTEC, tag, owner, clock), m_bank_addr(0), m_bank_size(0), m_old_bank_addr(0)
{
}

md_rom_redcl_device::md_rom_redcl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_REDCL, tag, owner, clock)
{
}

md_rom_squir_device::md_rom_squir_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_SQUIR, tag, owner, clock), m_latch(0)
{
}

md_rom_tc2000_device::md_rom_tc2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_TC2000, tag, owner, clock)
{
}

md_rom_tekkensp_device::md_rom_tekkensp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_TEKKENSP, tag, owner, clock), m_reg(0)
{
}

md_rom_topf_device::md_rom_topf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_TOPF, tag, owner, clock), m_latch(0)
{
}

md_rom_radica_device::md_rom_radica_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_RADICA, tag, owner, clock), m_bank(0)
{
}

md_rom_beggarp_device::md_rom_beggarp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_BEGGARP, tag, owner, clock), m_mode(0), m_lock(0)
{
}

md_rom_wukong_device::md_rom_wukong_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_WUKONG, tag, owner, clock), m_mode(0)
{
}

md_rom_starodys_device::md_rom_starodys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_rom_device(mconfig, MD_ROM_STARODYS, tag, owner, clock), m_mode(0), m_lock(0), m_ram_enable(0), m_base(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void md_rom_ssf2_device::device_start()
{
	save_item(NAME(m_bank));
	save_item(NAME(m_lastoff));
	save_item(NAME(m_lastdata));
}

void md_rom_ssf2_device::device_reset()
{
	for (int i = 0; i < 7; i++)
		m_bank[i] = i;
	m_lastoff = -1;
	m_lastdata = -1;
}

void md_rom_cm2in1_device::device_start()
{
	m_base = -1;
	save_item(NAME(m_base));
}

void md_rom_cm2in1_device::device_reset()
{
	m_base++;
	m_base &= 1;
}

void md_rom_mcpirate_device::device_start()
{
	save_item(NAME(m_bank));
}

void md_rom_mcpirate_device::device_reset()
{
	m_bank = 0;
}

void md_rom_chinf3_device::device_start()
{
	m_bank = 0;
	save_item(NAME(m_bank));
}

void md_rom_chinf3_device::device_reset()
{
}

void md_rom_lion2_device::device_start()
{
	save_item(NAME(m_prot1_data));
	save_item(NAME(m_prot2_data));
}

void md_rom_lion2_device::device_reset()
{
	m_prot1_data = 0;
	m_prot2_data = 0;
}

void md_rom_lion3_device::device_start()
{
	save_item(NAME(m_reg));
	save_item(NAME(m_bank));
}

void md_rom_lion3_device::device_reset()
{
	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
	m_bank = 0;
}

void md_rom_pokestad_device::device_start()
{
	save_item(NAME(m_bank));
}

void md_rom_pokestad_device::device_reset()
{
	m_bank = 0;
}

void md_rom_realtec_device::device_start()
{
	save_item(NAME(m_bank_addr));
	save_item(NAME(m_bank_size));
	save_item(NAME(m_old_bank_addr));
}

void md_rom_realtec_device::device_reset()
{
	m_bank_addr = 0;
	m_bank_size = 0;
	m_old_bank_addr = -1;
}

void md_rom_squir_device::device_start()
{
	save_item(NAME(m_latch));
}

void md_rom_squir_device::device_reset()
{
	m_latch = 0;
}

void md_rom_smw64_device::device_start()
{
	save_item(NAME(m_latch0));
	save_item(NAME(m_latch1));
	save_item(NAME(m_reg));
	save_item(NAME(m_ctrl));
}

void md_rom_smw64_device::device_reset()
{
	m_latch0 = 0xf;
	m_latch1 = 0xf;
	memset(m_reg, 0, sizeof(m_reg));
	memset(m_ctrl, 0, sizeof(m_ctrl));
}

void md_rom_tekkensp_device::device_start()
{
	save_item(NAME(m_reg));
}

void md_rom_tekkensp_device::device_reset()
{
	m_reg = 0;
}

void md_rom_topf_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_bank));
}

void md_rom_topf_device::device_reset()
{
	m_latch = 0;
	m_bank[0] = m_bank[1] = m_bank[2] = 0;
}

void md_rom_radica_device::device_start()
{
	save_item(NAME(m_bank));
}

void md_rom_radica_device::device_reset()
{
	m_bank = 0;
}

void md_rom_beggarp_device::device_start()
{
	save_item(NAME(m_mode));
	save_item(NAME(m_lock));
}

void md_rom_beggarp_device::device_reset()
{
	m_mode = 0;
	m_lock = 0;
}

void md_rom_wukong_device::device_start()
{
	save_item(NAME(m_mode));
}

void md_rom_wukong_device::device_reset()
{
	m_mode = 0;
}

void md_rom_starodys_device::device_start()
{
	save_item(NAME(m_mode));
	save_item(NAME(m_lock));
	save_item(NAME(m_ram_enable));
	save_item(NAME(m_base));
}

void md_rom_starodys_device::device_reset()
{
	m_mode = 0;
	m_lock = 0;
	m_ram_enable = 1;
	m_base = 0;
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------
 CART + SRAM
 -------------------------------------------------*/

uint16_t md_rom_sram_device::read(offs_t offset)
{
	// since a lot of generic carts ends up here if loaded from fullpath
	// we access nvram only if m_nvram_handlers_installed has been turned on
	if (m_nvram_handlers_installed)
	{
		if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active)
			return m_nvram[offset - m_nvram_start/2];
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_rom_sram_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// since a lot of generic carts ends up here if loaded from fullpath
	// we access nvram only if m_nvram_handlers_installed has been turned on
	if (m_nvram_handlers_installed)
	{
		if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active && !m_nvram_readonly)
			m_nvram[offset - m_nvram_start/2] = data;
	}
}

void md_rom_sram_device::write_a13(offs_t offset, uint16_t data)
{
	if (offset == 0xf0/2)
	{
		/* unsure if this is actually supposed to toggle or just switch on? yet to encounter game that uses this */
		m_nvram_active = BIT(data, 0);
		m_nvram_readonly = BIT(data, 1);

		// since a lot of generic carts ends up here if loaded from fullpath
		// we turn on nvram (with m_nvram_handlers_installed) only if they toggle it on by writing here!
		if (m_nvram_active && !m_nvram_handlers_installed)
			m_nvram_handlers_installed = 1;
	}
}

/*-------------------------------------------------
 CART + FRAM [almost same as SRAM... merge common parts?]
 -------------------------------------------------*/

uint16_t md_rom_fram_device::read(offs_t offset)
{
	if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active)
		return m_nvram[offset - m_nvram_start/2];
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_rom_fram_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active)
			m_nvram[offset - m_nvram_start/2] = data;
}

void md_rom_fram_device::write_a13(offs_t offset, uint16_t data)
{
	if (offset == 0xf0/2)
		m_nvram_active = BIT(data, 0);
}


uint16_t md_rom_fram_device::read_a13(offs_t offset)
{
	if (offset == 0xf0/2)
		return m_nvram_active;
	else
		return 0xffff;
}

/*-------------------------------------------------
 SUPER STREET FIGHTER 2
 -------------------------------------------------*/

uint16_t md_rom_ssf2_device::read(offs_t offset)
{
	if (offset < 0x400000/2)
		return m_rom[offset];
	else
		return 0xffff;
}

// I'm not very fond of the code below...
void md_rom_ssf2_device::write_a13(offs_t offset, uint16_t data)
{
	if (offset >= 0xf0/2)
	{
		offset -= 0xf0/2;
		if ((m_lastoff != offset) || (m_lastdata != data))
		{
			m_lastoff = offset;
			m_lastdata = data;
			if (offset) // bank 0 is not modified
			{
				uint16_t *ROM = get_rom_base();
				m_bank[offset] = data & 0xf;
				memcpy(ROM + offset * 0x080000/2, ROM + 0x400000/2 + (m_bank[offset] * 0x080000)/2, 0x080000);
			}
		}
	}
}

/*-------------------------------------------------
 CODEMASTERS 2 IN 1 (RESET BASED)
 -------------------------------------------------*/

#define MD_ADDR_CM2IN1(a) (m_base == 0 ? ((a << 1) & 0x1fffff)/2 : (((a << 1) & 0x1fffff) + 0x200000)/2)

uint16_t md_rom_cm2in1_device::read(offs_t offset)
{
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR_CM2IN1(offset)];
	else
		return 0xffff;
}


/*-------------------------------------------------
 PIRATE MULTICARTS
 -------------------------------------------------*/

uint16_t md_rom_mcpirate_device::read(offs_t offset)
{
	if (offset < 0x400000/2)
		return m_rom[(((m_bank * 0x10000) + (offset << 1)) & (m_rom_size - 1))/2];
	else
		return read(offset - 0x400000/2);
}

void md_rom_mcpirate_device::write_a13(offs_t offset, uint16_t data)
{
	offset <<= 1;
	if (offset < 0x40)
		m_bank = offset;
}

/*-------------------------------------------------
 A BUG'S LIFE
 -------------------------------------------------*/

uint16_t md_rom_bugslife_device::read_a13(offs_t offset)
{
	if (offset == 0x00/2)   return 0x28;
	if (offset == 0x02/2)   return 0x01;
	if (offset == 0x3e/2)   return 0x1f;
	else    return 0xffff;
}

/*-------------------------------------------------
 CHINESE FIGHTER 3
 -------------------------------------------------*/

uint16_t md_rom_chinf3_device::read(offs_t offset)
{
	if (offset < 0x100000/2)
	{
		if (!m_bank)
			return m_rom[offset & 0xfffff/2];
		else
			return m_rom[(offset & 0xffff/2) + (m_bank * 0x10000)/2];
	}

	// PROTECTION in 0x400000 - 0x4fffff
	/* not 100% correct, there may be some relationship between the reads here
	 and the writes made at the start of the game.. */
	if (offset >= 0x400000/2 && offset < 0x500000/2)
	{
		uint32_t retdat;
		/*
		 04dc10 chifi3, prot_r? 2800
		 04cefa chifi3, prot_r? 65262
		 */
		if (machine().device<cpu_device>("maincpu")->pc() == 0x01782) // makes 'VS' screen appear
		{
			retdat = machine().device<cpu_device>("maincpu")->state_int(M68K_D3) & 0xff;
			retdat <<= 8;
			return retdat;
		}
		else if (machine().device<cpu_device>("maincpu")->pc() == 0x1c24) // background gfx etc.
		{
			retdat = machine().device<cpu_device>("maincpu")->state_int(M68K_D3) & 0xff;
			retdat <<= 8;
			return retdat;
		}
		else if (machine().device<cpu_device>("maincpu")->pc() == 0x10c4a) // unknown
		{
			return machine().rand();
		}
		else if (machine().device<cpu_device>("maincpu")->pc() == 0x10c50) // unknown
		{
			return machine().rand();
		}
		else if (machine().device<cpu_device>("maincpu")->pc() == 0x10c52) // relates to the game speed..
		{
			retdat = machine().device<cpu_device>("maincpu")->state_int(M68K_D4) & 0xff;
			retdat <<= 8;
			return retdat;
		}
		else if (machine().device<cpu_device>("maincpu")->pc() == 0x061ae)
		{
			retdat = machine().device<cpu_device>("maincpu")->state_int(M68K_D3) & 0xff;
			retdat <<= 8;
			return retdat;
		}
		else if (machine().device<cpu_device>("maincpu")->pc() == 0x061b0)
		{
			retdat = machine().device<cpu_device>("maincpu")->state_int(M68K_D3) & 0xff;
			retdat <<= 8;
			return retdat;
		}
		else
		{
			logerror("%06x chifi3, prot_r? %04x\n", machine().device<cpu_device>("maincpu")->pc(), offset);
		}
		return 0;
	}

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_rom_chinf3_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= 0x600000/2 && offset < 0x700000/2)
	{
		if (data == 0xf100) // *hit player
			m_bank = 1;
		else if (data == 0xd700) // title screen..
			m_bank = 7;
		else if (data == 0xd300) // character hits floor
			m_bank = 3;
		else if (data == 0x0000)
			m_bank = 0;
		else
			logerror("%06x chifi3, bankw? %04x %04x\n", machine().device<cpu_device>("maincpu")->pc(), offset, data);
	}
}

/*-------------------------------------------------
 16 MAHJONG II
 -------------------------------------------------*/

uint16_t md_rom_16mj2_device::read(offs_t offset)
{
	if (offset == 0x400004/2)   return 0xc900;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

/*-------------------------------------------------
 LINGHUAN DAOSHI SUPER MAGICIAN / ELF WOR
 -------------------------------------------------*/

uint16_t md_rom_elfwor_device::read(offs_t offset)
{
	// It returns (0x55 @ 0x400000 OR 0xc9 @ 0x400004) AND (0x0f @ 0x400002 OR 0x18 @ 0x400006).
	// It is probably best to add handlers for all 4 addresses
	if (offset == 0x400000/2)   return 0x5500;
	if (offset == 0x400002/2)   return 0x0f00;
	if (offset == 0x400004/2)   return 0xc900;
	if (offset == 0x400006/2)   return 0x1800;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

/*-------------------------------------------------
 HUAN LE TAO QI SHU / SMART MOUSE
 -------------------------------------------------*/

uint16_t md_rom_smouse_device::read(offs_t offset)
{
	if (offset == 0x400000/2)   return 0x5500;
	if (offset == 0x400002/2)   return 0x0f00;
	if (offset == 0x400004/2)   return 0xaa00;
	if (offset == 0x400006/2)   return 0xf000;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

/*-------------------------------------------------
 YA SE CHUAN SHUO
 -------------------------------------------------*/

uint16_t md_rom_yasech_device::read(offs_t offset)
{
	if (offset == 0x400000/2)   return 0x6300;
	if (offset == 0x400002/2)   return 0x9800;
	if (offset == 0x400004/2)   return 0xc900;
	if (offset == 0x400006/2)   return 0x1800;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

/*-------------------------------------------------
 KOF98
 -------------------------------------------------*/

uint16_t md_rom_kof98_device::read(offs_t offset)
{
	if (offset == 0x480000/2)   return 0xaa00;
	if (offset == 0x4800e0/2)   return 0xaa00;
	if (offset == 0x4824a0/2)   return 0xaa00;
	if (offset == 0x488880/2)   return 0xaa00;
	if (offset == 0x4a8820/2)   return 0x0a00;
	if (offset == 0x4f8820/2)   return 0x0000;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

/*-------------------------------------------------
 KOF 99
 -------------------------------------------------*/

uint16_t md_rom_kof99_device::read_a13(offs_t offset)
{
	if (offset == 0x00/2)   return 0x00;    // startup protection check, chinese message if != 0
	if (offset == 0x02/2)   return 0x01;    // write 02 to a13002.. shift right 1?
	if (offset == 0x3e/2)   return 0x1f;    // write 3e to a1303e.. shift right 1?
	else    return 0xffff;
}

/*-------------------------------------------------
 LION KING 2
 -------------------------------------------------*/

uint16_t md_rom_lion2_device::read(offs_t offset)
{
	if (offset == 0x400002/2)   return m_prot1_data;
	if (offset == 0x400006/2)   return m_prot2_data;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_rom_lion2_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x400000/2)   m_prot1_data = data;
	if (offset == 0x400004/2)   m_prot2_data = data;
}

/*-------------------------------------------------
 LION KING 3
 -------------------------------------------------*/

#define MD_LION3_ADDR(a)  (((offset << 1) | (m_bank << 15)) & (m_rom_size - 1))/2

uint16_t md_rom_lion3_device::read(offs_t offset)
{
	if (offset < 0x100000/2)
		return m_rom[MD_LION3_ADDR(offset)];
	else if (offset >= 0x600000/2 && offset < 0x700000/2)
	{
		switch (offset & 0x7)
		{
			case 0:
				return m_reg[0];
			case 1:
				return m_reg[1];
			case 2:
				return m_reg[2];
			default:
				logerror("protection read, unknown offset %x\n", offset & 0x7);
				break;
		}
		return 0;
	}

	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_rom_lion3_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= 0x600000/2 && offset < 0x700000/2)
	{
//      printf("protection write, offset %d data %d\n", offset & 0x7, data);
		switch (offset & 0x7)
		{
			case 0x0:
				m_reg[0] = data & 0xff;
				break;
			case 0x1:
				m_reg[1] = data & 0xff;
				break;
			default:
				logerror("protection write, unknown offset %d\n", offset & 0x7);
				break;
		}

		// update m_reg[2]
		switch (m_reg[1] & 3)
		{
			case 0x0:
				m_reg[2] = (m_reg[0] << 1);
				break;
			case 0x1:
				m_reg[2] = (m_reg[0] >> 1);
				break;
			case 0x2:
				m_reg[2] = (m_reg[0] >> 4) | ((m_reg[0] & 0x0f) << 4);
				break;
			case 0x3:
			default:
				m_reg[2] =  (BIT(m_reg[0], 7) << 0)
						| (BIT(m_reg[0], 6) << 1)
						| (BIT(m_reg[0], 5) << 2)
						| (BIT(m_reg[0], 4) << 3)
						| (BIT(m_reg[0], 3) << 4)
						| (BIT(m_reg[0], 2) << 5)
						| (BIT(m_reg[0], 1) << 6)
						| (BIT(m_reg[0], 0) << 7);
				break;
		}

	}
	if (offset >= 0x700000/2)
		m_bank = data & 0xff;
}

/*-------------------------------------------------
 MA JIANG QING REN / MAHJONG LOVER
 -------------------------------------------------*/

uint16_t md_rom_mjlov_device::read(offs_t offset)
{
	if (offset == 0x400000/2)   return 0x9000;
	if (offset == 0x401000/2)   return 0xd300;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}


/*-------------------------------------------------
 CHAOJI MAJIANG CLUB
 -------------------------------------------------*/

uint16_t md_rom_cjmjclub_device::read(offs_t offset)
{
	if (offset == 0x400000/2)   return 0x9000;
	if (offset == 0x400002/2)   return 0xd300;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}


/*-------------------------------------------------
 SUPER BUBBLE BOBBLE MD
 -------------------------------------------------*/

uint16_t md_rom_sbubl_device::read(offs_t offset)
{
	if (offset == 0x400000/2)   return 0x5500;
	if (offset == 0x400002/2)   return 0x0f00;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

/*-------------------------------------------------
 SOUL BLADE
 -------------------------------------------------*/

uint16_t md_rom_soulb_device::read(offs_t offset)
{
	if (offset == 0x400002/2)   return 0x9800;
	if (offset == 0x400004/2)   return 0xc900;
	if (offset == 0x400006/2)   return 0xf000;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

/*-------------------------------------------------
 POKEMON STADIUM / KAIJU
 -------------------------------------------------*/

#define MD_POKESTAD_ADDR(a)  (((offset << 1) | (m_bank << 15)) & (m_rom_size - 1))/2

uint16_t md_rom_pokestad_device::read(offs_t offset)
{
	if (offset < 0x100000/2)
		return m_rom[MD_POKESTAD_ADDR(offset)];

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_rom_pokestad_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= 0x700000/2 && offset < 0x800000/2)
		m_bank = data & 0x7f;
}

/*-------------------------------------------------
 POKEMON ALT
 -------------------------------------------------*/

uint16_t md_rom_pokea_device::read_a13(offs_t offset)
{
	if (offset == 0x00/2)   return 0x14;
	if (offset == 0x02/2)   return 0x01;
	if (offset == 0x3e/2)   return 0x1f;
	else    return 0xffff;
}

/*-------------------------------------------------
 REALTEC
 -------------------------------------------------*/

uint16_t md_rom_realtec_device::read(offs_t offset)
{
	if (offset < (m_bank_size * 0x20000))   // two banks of same (variable) size at the bottom of the rom
		return m_rom[MD_ADDR((offset + (m_bank_addr * 0x20000)/2))];
	return m_rom[MD_ADDR(((offset & 0x1fff/2) + 0x7e000/2))];  // otherwise it accesses the final 8k of the image
}

void md_rom_realtec_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x400000/2)
	{
		m_old_bank_addr = m_bank_addr;
		m_bank_addr = (m_bank_addr & 0x7) | ((data >> 9) & 0x7) << 3;
	}
	if (offset == 0x402000/2)
	{
		m_bank_addr = 0;
		m_bank_size = (data >> 8) & 0x1f;
	}
	if (offset == 0x404000/2)
	{
		m_old_bank_addr = m_bank_addr;
		m_bank_addr = (m_bank_addr & 0xf8) | ((data >> 8) & 0x3);
	}
}

/*-------------------------------------------------
 RED CLIFF
 -------------------------------------------------*/

uint16_t md_rom_redcl_device::read(offs_t offset)
{
	if (offset == 0x400000/2)   return 0x55 << 8;
	if (offset == 0x400004/2)   return 0xaa << 8;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

/*-------------------------------------------------
 ROCKMAN X3
 -------------------------------------------------*/

uint16_t md_rom_rx3_device::read_a13(offs_t offset)
{
	if (offset == 0)
		return 0x0c;
	else
		return 0xffff;
}

/*-------------------------------------------------
 SQUIRREL KING
 -------------------------------------------------*/

uint16_t md_rom_squir_device::read(offs_t offset)
{
	if ((offset >= 0x400000/2) && (offset < 0x400008/2))
		return m_latch;

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_rom_squir_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= 0x400000/2 && offset < 0x400008/2)
		m_latch = data;
}

/*-------------------------------------------------
 SUPER MARIO BROS
 -------------------------------------------------*/

uint16_t md_rom_smb_device::read_a13(offs_t offset)
{
	if (offset == 0)
		return 0x0c;
	else
		return 0xffff;
}

/*-------------------------------------------------
 SUPER MARIO BROS 2
 -------------------------------------------------*/

uint16_t md_rom_smb2_device::read_a13(offs_t offset)
{
	if (offset == 0)
		return 0x0a;
	else
		return 0xffff;
}

/*-------------------------------------------------
 SUPER MARIO WORLD 64
 -------------------------------------------------*/

uint16_t md_rom_smw64_device::read(offs_t offset)
{
	// 0x000000-0x0fffff: lower 512KB ROM (up to 0x07ffff) + mirror
	// 0x600000-0x6fffff: internal hardware (up to 0x67ffff) + mirror
	// Namely,
	//  * 60xxx = bank1 of the upper 512KB ROM
	//  * 61xxx = bank2 of the upper 512KB ROM
	//  * 62xxx = alternate 4KB chunks of 0x0000 ~ 0xffff
	//  * 63xxx = same as 62xxx
	//  * 64xxx = returns 0x0000
	//  * 65xxx = same as 64xxx
	//  * 66xxx = CTRL/DATA
	//  * 67xxx = CTRL/DATA
	if (offset < 0x100000/2)
		return m_rom[offset & 0x3ffff];

	if ((offset >= 0x600000/2) && (offset < 0x610000/2))
		return m_rom[(m_latch0 * 0x10000)/2 + (offset & 0x7fff)];
	if ((offset >= 0x610000/2) && (offset < 0x620000/2))
		return m_rom[(m_latch1 * 0x10000)/2 + (offset & 0x7fff)];

	if ((offset >= 0x620000/2) && (offset < 0x640000/2))
		return (offset & 0x1000/2) ? 0x0000 : 0xffff;
	if ((offset >= 0x640000/2) && (offset < 0x660000/2))
		return 0x0000;

	if ((offset >= 0x660000/2) && (offset < 0x670000/2))
	{
		offset &= 7;
		switch (offset)
		{
			case 0x0:
			case 0x2:
			case 0x4:
				return m_reg[offset/2]; // DATA1, DATA2, DATA3
			case 0x1:
			case 0x3:
			case 0x5:
				return m_reg[offset/2] + 1; // DATA1+1, DATA2+1, DATA3+1
			case 0x6:
				return m_reg[2] + 2;    // DATA3+2
			case 0x7:
				return m_reg[2] + 3;    // DATA3+3
		}
	}
	if ((offset >= 0x670000/2) && (offset < 0x680000/2))
	{
		uint16_t data = (m_ctrl[1] & 0x80) ? ((m_ctrl[2] & 0x40) ? (m_reg[4] & m_reg[5]) : (m_reg[4] ^ 0xff)) : 0x0000;
		if (offset & 0x1)   // odd offset, return lower 7 bits of the above
			return data & 0x7f;
		else    // even offset, return whole data above, but also update the regs if CTRL3 has 0x80 set
		{
			if (m_ctrl[2] & 0x80)   // update regs if CTRL3 has bit7 set
			{
				if (m_ctrl[2] & 0x20)
					m_reg[2] = (m_reg[5] << 2) & 0xfc;  // DATA3
				else
					m_reg[0] = ((m_reg[4] << 1) ^ m_reg[3]) & 0xfe; // DATA1
			}
			return data;
		}
	}
	return 0xffff;
}

void md_rom_smw64_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// 0x600000-0x6fffff: internal hardware (up to 0x67ffff) + mirror
	// Namely,
	//  * 62xxx/63xxx = unknown/unmapped
	//  * 65xxx/66xxx = unknown/unmapped
	//  * remaining ranges = CTRL/DATA
	if ((offset >= 0x600000/2) && (offset < 0x610000/2))
	{
		if (offset & 1)
		{
			if ((m_ctrl[0] & 7) == 0)
				m_reg[0] = ((m_reg[0] ^ m_reg[3]) ^ data) & 0xfe;   // DATA1
			if ((m_ctrl[0] & 7) == 1)
				m_reg[1] = data & 0xfe; // DATA2
			if ((m_ctrl[0] & 7) == 7)
				m_latch1 = 8 + ((data & 0x1c) >> 2);    // ROM BANKSWITCH $61
			m_reg[3] = data;    // DATA4
		}
		else
			m_ctrl[0] = data;   // CTRL1
	}
	if ((offset >= 0x610000/2) && (offset < 0x620000/2))
	{
		if (offset & 1)
			m_ctrl[1] = data;   // CTRL2
	}
	if ((offset >= 0x640000/2) && (offset < 0x650000/2))
	{
		if (offset & 1)
			m_reg[5] = data;    // DATA6
		else
			m_reg[4] = data;    // DATA5
	}
	if ((offset >= 0x670000/2) && (offset < 0x680000/2))
	{
		if (!(offset & 1))
		{
			m_ctrl[2] = data;   // CTRL3
			if (m_ctrl[1] & 0x80)
				m_latch0 = 8 + ((data & 0x1c) >> 2);    // ROM BANKSWITCH $60
		}
	}
}

/*-------------------------------------------------
 TC2000 / TRUCO 96
 -------------------------------------------------*/

void md_rom_tc2000_device::device_start()
{
	save_item(NAME(m_retvalue));
}

void md_rom_tc2000_device::device_reset()
{
	m_retvalue = 0;
}

uint16_t md_rom_tc2000_device::read(offs_t offset)
{
	if (offset < 0x100000 / 2)
	{
		return md_std_rom_device::read(offset);
	}
	else
	{
		// this works for game boot and starting a game, are there any further checks?
		logerror("protection read at offset %08x returning %04x\n", offset*2, m_retvalue);

		return m_retvalue;
	}
}

void md_rom_tc2000_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset < 0x100000/2)
	{
		md_std_rom_device::write(offset, data, mem_mask);
	}
	else
	{
		if (((offset * 2) & 0xf) == 0x0) // truco96a uses this case
		{
			m_retvalue = 0x0000;
		}
		else if (((offset * 2) & 0xf) == 0x8)
		{
			m_retvalue = 0x5000;
		}
		else if (((offset * 2) & 0xf) == 0xc)
		{
			m_retvalue = 0xa000;
		}

		logerror("protection write at offset %08x %04x %04x\n", offset*2, data, mem_mask);
	}
}


/*-------------------------------------------------
 TEKKEN SPECIAL
 -------------------------------------------------*/

uint16_t md_rom_tekkensp_device::read(offs_t offset)
{
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else if ((offset & 0x07) == 1 && m_reg)
		return (m_reg - 1) << 8;
	else
		return 0xffff;
}

void md_rom_tekkensp_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset < 0x400000/2)
		return;

	// thanks to EkeEke for the documentation
	switch (offset & 0x07)
	{
		case 0x00:
			// data output reset ? (game writes $FF before & after protection check)
			m_reg = 0;
			break;
		case 0x01:
			// read only ?
			break;
		case 0x06:
			// data output mode bit 0 ? (game writes $01)
			break;
		case 0x07:
			// data output mode bit 1 ? (never written by game)
			break;
		default:
			if (data & 0x100)   // data input (only connected to D0 ?)
			{
				// 4-bit hardware register ($400004 corresponds to bit0, $400006 to bit1, etc)
				int shift = (offset - 2) & 3;
				m_reg |= (1 << shift);
			}
			break;
	}
}

/*-------------------------------------------------
 TOP FIGHTER
 -------------------------------------------------*/

uint16_t md_rom_topf_device::read(offs_t offset)
{
	//cpu #0 (PC=0004CBAE): unmapped program memory word read from 006A35D4 & 00FF -- wants regD7
	if (offset == 0x645b44/2)
	{
		//cpu #0 (PC=0004DE00): unmapped program memory word write to 00689B80 = 004A & 00FF
		//cpu #0 (PC=0004DE08): unmapped program memory word write to 00 = 00B5 & 00FF
		//cpu #0 (PC=0004DE0C): unmapped program memory word read from 00645B44 & 00FF

		return 0x9f;//0x25;
	}
	if (offset == 0x6bd294/2)
	{
		/*
		 cpu #0 (PC=00177192): unmapped program memory word write to 006BD240 = 00A8 & 00FF
		 cpu #0 (PC=0017719A): unmapped program memory word write to 006BD2D2 = 0098 & 00FF
		 cpu #0 (PC=001771A2): unmapped program memory word read from 006BD294 & 00FF
		 */

		if (machine().device<cpu_device>("maincpu")->pc()==0x1771a2) return 0x50;
		else
		{
			m_latch++;
			logerror("%06x topfig_6BD294_r %04x\n",machine().device<cpu_device>("maincpu")->pc(), m_latch);
			return m_latch;
		}
	}
	if (offset == 0x6f5344/2)
	{
		if (machine().device<cpu_device>("maincpu")->pc()==0x4C94E)
			return machine().device<cpu_device>("maincpu")->state_int((M68K_D0)) & 0xff;
		else
		{
			m_latch++;
			logerror("%06x topfig_6F5344_r %04x\n", machine().device<cpu_device>("maincpu")->pc(), m_latch);
			return m_latch;
		}
	}

	if (offset >= 0x20000/2 && offset < 0x28000/2)
		return m_rom[offset + (m_bank[0] * 0x188000)/2];

	if (offset >= 0x58000/2 && offset < 0x60000/2)
		return m_rom[offset + (m_bank[1] * 0x20000)/2];

	if (offset >= 0x60000/2 && offset < 0x68000/2)
		return m_rom[offset + (m_bank[2] * 0x110000)/2];

	// non-protection accesses
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_rom_topf_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= 0x700000/2 && offset < 0x800000/2)
	{
		if (data == 0x002a)
			m_bank[2] = 1;  // == 0x2e*0x8000?!
		else if (data==0x0035) // characters ingame
			m_bank[0] = 1;  // == 0x35*0x8000
		else if (data==0x000f) // special moves
			m_bank[1] = 1; // == 0xf*0x8000
		else if (data==0x0000)
		{
			m_bank[0] = 0;
			m_bank[1] = 0;
			m_bank[2] = 0;
		}
		else
			logerror("%06x offset %06x, data %04x\n", machine().device<cpu_device>("maincpu")->pc(), offset, data);
	}
}

/*-------------------------------------------------
 RADICA TV GAMES [to be split...]
 -------------------------------------------------*/

uint16_t md_rom_radica_device::read(offs_t offset)
{
	return m_rom[(((m_bank * 0x10000) + (offset << 1)) & (m_rom_size - 1))/2];
}

uint16_t md_rom_radica_device::read_a13(offs_t offset)
{
	if (offset < 0x80)
		m_bank = offset & 0x3f;
	return 0;
}

/*-------------------------------------------------
 BEGGAR PRINCE
 This game uses cart which is the same as SEGA_SRAM
 + bankswitch mechanism for first 256KB of the image:
 depending on bit7 of the value written at 0xe00/2,
 accesses to 0x00000-0x3ffff go to the first 256KB
 of ROM, or to the second to last 256KB chunk (usually
 mapped to 0x380000-0x3bffff). SRAM is mapped at
 the end of ROM.
 -------------------------------------------------*/

uint16_t md_rom_beggarp_device::read(offs_t offset)
{
	if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active)
		return m_nvram[offset & 0x3fff];

	if (offset < 0x040000/2)
		return m_mode ? m_rom[offset + 0x380000/2] : m_rom[offset];
	else if (offset < 0x400000/2)
		return m_rom[offset & 0x1fffff];

	return 0xffff;
}

void md_rom_beggarp_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= 0x0e00/2 && offset < 0x0f00/2)
		m_mode = BIT(data, 7);

	if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active && !m_nvram_readonly)
		m_nvram[offset & 0x3fff] = data;
}

// this works the same as in standard SRAM carts
void md_rom_beggarp_device::write_a13(offs_t offset, uint16_t data)
{
	if (offset == 0xf0/2)
	{
		m_nvram_active = BIT(data, 0);
		m_nvram_readonly = BIT(data, 1);

		if (m_nvram_active)
			m_nvram_handlers_installed = 1;
	}
}

/*-------------------------------------------------
 LEGEND OF WUKONG
 This game uses cart which is the same as SEGA_SRAM
 + bankswitch mechanism for last 128KB of the image:
 first 2MB of ROM is loaded in 0-0x200000 and
 mirrored in 0x200000-0x400000, but depending on
 bit7 of the value written at 0xe00/2 accesses to
 0x200000-0x21ffff go either to the "physical" address
 (i.e. last 128K of ROM) or to the "memory" address
 (i.e. mirror of first 128K)
 -------------------------------------------------*/

uint16_t md_rom_wukong_device::read(offs_t offset)
{
	if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active)
		return m_nvram[offset - m_nvram_start/2];

	// here can access both last 128K of the ROM and the first 128K, depending of m_mode
	if (offset >= 0x200000/2 && offset < 0x220000/2)
		return !m_mode ? m_rom[offset] : m_rom[offset & 0xffff];
	else if (offset < 0x400000/2)
		return m_rom[offset & 0xfffff];
	else
		return 0xffff;
}

void md_rom_wukong_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset < 0x100000/2)    // it actually writes to 0xe00/2
		m_mode = BIT(data, 7);

	if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active && !m_nvram_readonly)
		m_nvram[offset - m_nvram_start/2] = data;
}

// this works the same as in standard SRAM carts
void md_rom_wukong_device::write_a13(offs_t offset, uint16_t data)
{
	if (offset == 0xf0/2)
	{
		m_nvram_active = BIT(data, 0);
		m_nvram_readonly = BIT(data, 1);

		if (m_nvram_active)
			m_nvram_handlers_installed = 1;
	}
}


/*-------------------------------------------------
 STAR ODYSSEY
 This game uses a slightly more complex mapper:
 not only RAM can be enabled / disabled, but also
 ROM can be mapped in three different modes.
 In what we call Mode0 the first 256K are mirrored
 into area 0x000000-0x1fffff; in Mode1 cart gives
 access to 5x256K banks into area 0x000000-0x13ffff
 and open bus into 0x140000-0x1fffff; in Mode2 the
 ROM is disabled and the whole 0x000000-0x1fffff
 gives open bus
-------------------------------------------------*/

uint16_t md_rom_starodys_device::read(offs_t offset)
{
	if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active && m_ram_enable)
		return m_nvram[offset & 0x3fff];

	if (offset < 0x200000/2)
	{
		if (m_mode == 0)
			return m_rom[offset & 0x3ffff];
		else if (m_mode == 1 && offset < 0x140000/2)
			return m_rom[m_base * 0x40000/2 + offset];
		else
			return 0xffff;
	}
	else if (offset < 0x400000/2)
		return m_rom[offset & 0xfffff];
	else
		return 0xffff;
}

void md_rom_starodys_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active && !m_nvram_readonly && m_ram_enable)
		m_nvram[offset & 0x3fff] = data;

	if (offset < 0x10000/2)
	{
		uint32_t prot_offs = (offset * 2) & 0xf00;
		if (!m_lock)
		{
			if (prot_offs == 0xd00)
			{
				//printf("RAM enable %s!\n", BIT(data, 7) ? "Yes" : "No");
				m_ram_enable = BIT(data, 7);
			}
			if (prot_offs == 0xe00)
			{
				if (BIT(data, 5))
					m_mode = 2;
				else if (BIT(data, 6))
					m_mode = 1;
				else
					m_mode = 0;
				//printf("ROM mode %d!\n", m_mode);

				if (!BIT(data, 7))
				{
					//printf("LOCK BANKSWITCH!\n");
					m_lock = 1;
				}
			}
			if (prot_offs == 0xf00)
			{
				m_base = ((data >> 4) & 7);
				m_mode = 1;
				//printf("ROM base %d!\n", m_base);
			}
		}
	}

}

uint16_t md_rom_starodys_device::read_a13(offs_t offset)
{
	return m_base << 4;
}

// this works the same as in standard SRAM carts
void md_rom_starodys_device::write_a13(offs_t offset, uint16_t data)
{
	if (offset == 0xf0/2)
	{
		m_nvram_active = BIT(data, 0);
		m_nvram_readonly = BIT(data, 1);

		if (m_nvram_active)
			m_nvram_handlers_installed = 1;
	}
}


/*-------------------------------------------------
 Futbol Argentino 96 (Argentina)
 -------------------------------------------------*/

uint16_t md_rom_sram_arg96_device::read(offs_t offset)
{
	if (offset < 0x400000 / 2)
	{
		return md_rom_sram_device::read(offset);
	}
	else
	{
		// these return values are probably connected somehow with the writes
		// but the game only ever looks for these results before doing DMA operations
		if ((offset * 2) == 0x4c6200)
			return 0xa;
		else if ((offset * 2) == 0x4c6600)
			return 0x9;
		else if ((offset * 2) == 0x4c6a00)
			return 0x7;
		else
			logerror("unhandled read at offset %08x\n", offset);

		return 0x0000;
	}
}

void md_rom_sram_arg96_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset < 0x400000/2)
	{
		md_rom_sram_device::write(offset, data, mem_mask);
	}
	else
	{
		logerror("unhandled write at offset %08x %04x %04x\n", offset, data, mem_mask);
	}
}
