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
DEFINE_DEVICE_TYPE(MD_ROM_PAPRIUM, md_rom_paprium_device, "md_rom_paprium", "MD Paprium")


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
 SUPER STREET FIGHTERS 2
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

/*-------------------------------------------------
 Paprium
 Cortex-M4 CPU code isn't dumped, we have to make
 up for the missing features with best guesses of
 internal workings.

 Current issues:
 - First boot will launch a fake minigame, this is normal
 when save is empty.
 - Still WIP/incomplete, no sound, reset not working, hacks, etc...
 - Implementation is a bit too much pointer happy
 for save state atm.
 -------------------------------------------------*/


#define PPM_DEBUG 0

md_rom_paprium_device::md_rom_paprium_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : md_rom_sram_device(mconfig, MD_ROM_PAPRIUM, tag, owner, clock)
{
}

void md_rom_paprium_device::device_start()
{
	logerror("sizeof(ppm_interface_struct): 0x%x\n", (unsigned int)sizeof(ppm_interface_struct));
	ppm_interface = (ppm_interface_struct *)ppm_dual_port_ram;

	save_item(NAME(ppm_dual_port_ram));
	save_item(NAME(ppm_sdram));

	//	save_item(NAME(ppm_interface));

	save_item(NAME(ppm_scale_stamp));

	//	save_item(NAME(ppm_sdram_pointer));
	save_item(NAME(ppm_sdram_window_enabled));

	//	save_item(NAME(ppm_object_handles));
	save_pointer((uint16_t *)&ppm_object_handles[0], "ppm_object_handles", 0);
	save_item(NAME(ppm_drawList));
	//	save_item(NAME(ppm_drawListPtr));

	//	save_item(NAME(ppm_vram_slots));
	save_pointer((uint16_t *)&ppm_vram_slots[0], "ppm_vram_slots", 0);
	save_item(NAME(ppm_vram_max_slot));
	save_item(NAME(ppm_block_unpack_addr));

	save_item(NAME(ppm_anim_data_base_addr));
	//	save_item(NAME(ppm_anim_data));
	save_item(NAME(ppm_anim_max_index));

	save_item(NAME(ppm_unk_data_addr));
	save_item(NAME(ppm_unk_data2_addr));

	//	save_item(NAME(ppm_bgm_tracks_offsets));
	save_item(NAME(ppm_bgm_tracks_base_addr));
	save_item(NAME(ppm_bgm_unpack_addr));

	//	save_item(NAME(ppm_sfx_data));
	save_item(NAME(ppm_sfx_base_addr));

	//	save_item(NAME(ppm_gfx_blocks_offsets));
	save_item(NAME(ppm_gfx_blocks_base_addr));
	save_item(NAME(ppm_max_gfx_block));

	save_item(NAME(ppm_audio_bgm_volume));
	save_item(NAME(ppm_audio_sfx_volume));
	save_item(NAME(ppm_audio_config));
}

void md_rom_paprium_device::device_reset()
{
	// currently loading MAX10 data over rom area then copies it to ram block,
	// this should be changed later on to add proper dataarea for this to load into

	// did we decode yet?
	if (m_rom[0x8000 / 2])
	{
		uint16_t key1 = m_rom[0x8000 / 2];  // 0x0000 reset stack ptr
		uint16_t key2 = m_rom[0xbd000 / 2]; // blank area

		// top 8K overlayed with MAX10 dual port ram module
		// for (uint32_t addr = 0; addr < 0x2000 / 2; addr++)
		//	m_rom[addr] ^= (key2 | bitswap<16>(addr & 0xff, 15, 1, 14, 6, 13, 2, 12, 0, 11, 3, 10, 4, 9, 7, 8, 5));
		for (uint32_t addr = 0x2000 / 2; addr < 0x10000 / 2; addr++)
			m_rom[addr] ^= (key1 | bitswap<16>(addr & 0xff, 15, 1, 14, 6, 13, 2, 12, 0, 11, 3, 10, 4, 9, 7, 8, 5));
		for (uint32_t addr = 0x10000 / 2; addr < 0x800000 / 2; addr++)
			m_rom[addr] ^= (key2 | bitswap<16>(addr & 0xff, 15, 1, 14, 6, 13, 2, 12, 0, 11, 3, 10, 4, 9, 7, 8, 5));

		// copy data to dual port ram
		for (uint16_t x = 0; x < 0x2000 / 2; x++)
			ppm_dual_port_ram[x] = m_rom[x];

		// check ROM version & patch a few things
		// (hold B on startup to display version ingame)
		switch (m_rom[0x1000a / 2])
		{
		case 0x2e7f:
			// boot check
			ppm_dual_port_ram[0x1d1c / 2] = 0x0004;	 // 0xc00004
			ppm_dual_port_ram[0x1d2c / 2] |= 0x0100; // switch to beq.s (0x670e)

			// post splash jump location
			ppm_dual_port_ram[0x1562 / 2] = 0x0001;
			ppm_dual_port_ram[0x1564 / 2] = 0x0100;

			// emulator check
			m_rom[0x81104 / 2] = 0x4e71;

			// mame pretends to be version 1 megadrive but does not implement
			// proper TMSS register & rom, this will fix the issue being detected
			m_rom[0xbca5e / 2] = 0x6006; // bra.s to the correct location
			break;
		default:
			printf("UNKNOWN PAPRIUM VERSION 0x%04X\n", m_rom[0x1000a / 2]);
			break;
		}
	}

	// clear/setup registers
	ppm_interface->reg_command = 0;
	ppm_interface->reg_status_1 = 0;
	ppm_interface->reg_status_2.word = 0;
	ppm_interface->reg_status_2.bits.megawire_status = 7; // let's pretend MW is plugged & connected

	ppm_sdram_pointer = ppm_sdram;
	ppm_sdram_window_enabled = false;

	ppm_vram_max_slot = 0;
}

uint16_t md_rom_paprium_device::read(offs_t offset)
{
	if (offset < sizeof(ppm_dual_port_ram) / 2)
		return ppm_dual_port_ram[offset];

	if ((offset >= 0xc000 / 2) && (offset < 0x10000 / 2) && ppm_sdram_window_enabled)
		return *ppm_sdram_pointer++;

	if (offset < 0x400000 / 2)
		return m_rom[offset];
	else
		return 0xffff;
}

void md_rom_paprium_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// can only write to dual port ram
	if (offset < sizeof(ppm_dual_port_ram) / 2)
	{
		ppm_dual_port_ram[offset] = (ppm_dual_port_ram[offset] & ~mem_mask) | (data & mem_mask);
		switch (offset)
		{
		case offsetof(ppm_interface_struct, reg_status_1) / 2:
			printf("Status register 1 write? (0x%04x)\n", data);
			break;
		case offsetof(ppm_interface_struct, reg_status_2) / 2:
			printf("Status register 2 write? (0x%04x)\n", data);
			break;
		case offsetof(ppm_interface_struct, reg_command) / 2:
			ppm_process_command();
			break;
		case offsetof(ppm_interface_struct, reg_watchdog) / 2:
			if (data != 0x9494)
				printf("Watchdog? / unknown value: 0x%04x\n", data);
			break;
		case offsetof(ppm_interface_struct, reg_unk0) / 2:
		case offsetof(ppm_interface_struct, reg_unk1) / 2:
		case offsetof(ppm_interface_struct, reg_unk4) / 2:
		case offsetof(ppm_interface_struct, reg_unk7) / 2:
		case offsetof(ppm_interface_struct, reg_unk8) / 2:
		case offsetof(ppm_interface_struct, reg_unk9) / 2:
		case offsetof(ppm_interface_struct, reg_unkA) / 2:
		case offsetof(ppm_interface_struct, reg_unkB) / 2:
		case offsetof(ppm_interface_struct, reg_unkC) / 2:
		case offsetof(ppm_interface_struct, reg_unkD) / 2:
		case offsetof(ppm_interface_struct, reg_unkE) / 2:
		case offsetof(ppm_interface_struct, reg_unkF) / 2:
			printf("Unknown register write? (0x%04x, 0x%04x)\n", offset * 2, data);
			break;
		}
	}
}

void md_rom_paprium_device::ppm_process_command()
{
	uint8_t command_id = ppm_interface->reg_command >> 8;
	uint8_t command_arg = ppm_interface->reg_command; // & 0xff

	switch (command_id)
	{
	// special challenge command used on startup.
	// command 0x0055 takes quite some time to execute
	case 0x00:
		switch (command_arg)
		{
		case 0xaa:
			ppm_interface->reg_command = 0x00ff;
			return;
		case 0x55:
			ppm_interface->reg_command = 0x0000; // explicit 0 return
			return;
		default:
			#if PPM_DEBUG
			printf("CMD 0x00, unknown challenge 0x%02x\n", command_arg);
			#endif
			break;
		}
		break;
	// initial startup/reset? - 3 0w8100 writes on startup
	// finding arcade mode sends 0x810f
	case 0x81:
		#if PPM_DEBUG
		printf("CMD 0x%04x\n", ppm_interface->reg_command);
		#endif
		ppm_sdram_window_enabled = true;
		ppm_vram_set_budget(0);
		ppm_obj_reset(); // spr system reset
		break;
	// unk startup thing
	// 8300/8302 toggle *12 (24 total writes) on startup
	// couple toggles between stages
	case 0x83:
		#if PPM_DEBUG
		printf("CMD 0x%04x\n", ppm_interface->reg_command);
		#endif
		break;
	// disable sdram window
	case 0x84:
		ppm_sdram_window_enabled = false;
		break;
	// set audio config
	case 0x88:
		ppm_audio_config = command_arg;
		if (command_arg & ~0x0b) // setting unknown bits ?
			logerror("audio config: unknown bits (0x%02x)\n", command_arg);
		break;
	// bgm play, check & 0x80
	case 0x8c:
		logerror("BGM play 0x%02x (code %02x)\n", command_arg & 0x7f, command_arg);
		if (command_arg & 0x80)
		{
			// ?
		}
		ppm_unpack(ppm_bgm_addr(command_arg & 0x7f), ppm_bgm_unpack_addr);
		break;

		//	case 0x8d:
		//	// sound related

		//	case 0x8e:
		//		// bgm stop/pause?
		//		break;

	// (bgm related)
	case 0x95:
		break;
	// (bgm related)
	case 0x96:
		break;

	// megawire settings
	case 0xa4:
		switch (command_arg)
		{
		default:
			logerror("Megawire unk. option (0x%02x)\n", command_arg);
		case 0x00: // reset conn.
		case 0x40: // ?delete credentials?
		case 0x80: // power profile off
		case 0x81: // power profile auto
		case 0x82: // power profile forced
			break;
		}
		break;
	// a858 MW reboot (update screen)
	// a900 MW interrupt/reset? sub_9B738

	// add object to draw list
	case 0xad:
		ppm_obj_add(command_arg);
		break;
	// frame begin
	case 0xae:
		ppm_obj_frame_start();
		break;
	// frame finish
	case 0xaf:
		// af01: no scaling ongoing
		// af02: must reserve bandwith for scaling buffer?
		ppm_obj_frame_end();
		break;
		// ?unk?
		//	case 0xb0:
		//		break;
		// sent by waitvbl under condition, instead of 0xaf
		// end frame without obj rendering ?
	case 0xb1:
		// seems used to keep previous SAT data in VRAM, do nothing?
		break;
	// post scale command
	case 0xb6:
		// likely instructs to restore boot code to allow back HW reset
		break;
	// load/setup base data
	case 0xc6:
		ppm_setup_data(
		    swapshorts(ppm_interface->command_args_long[0]),
		    swapshorts(ppm_interface->command_args_long[1]),
		    swapshorts(ppm_interface->command_args_long[2]),
		    swapshorts(ppm_interface->command_args_long[3]),
		    swapshorts(ppm_interface->command_args_long[4]),
		    swapshorts(ppm_interface->command_args_long[5]),
		    swapshorts(ppm_interface->command_args_long[6]));
		break;
	// set BGM volume
	case 0xc9:
		ppm_audio_bgm_volume = command_arg;
		break;
	// set SFX volume
	case 0xca:
		ppm_audio_sfx_volume = command_arg;
		break;

	// d0

	// SFX play, 4 additional args
	case 0xd1:
		#if PPM_DEBUG
		printf("SFX 0x%02x - 0x%04x / 0x%04x / 0x%04x / 0x%04x\n", command_arg, ppm_interface->command_args[0], ppm_interface->command_args[1], ppm_interface->command_args[2], ppm_interface->command_args[3]);
		#endif
		break;
		// ?unk?
		//	case 0xd2:
		// 0xd240 0xd280
		//		break;

	// d6
	case 0xd6:
		#if PPM_DEBUG
		printf("CMD 0x%04x - 0x%04x\n", ppm_interface->reg_command, ppm_interface->command_args[0]);
		#endif
		break;

	// unpack request
	case 0xda:
		logerror("Unpacking 0x%06x @0x%04x (0x%02x)\n", (ppm_interface->command_args[1] << 16) + ppm_interface->command_args[2], ppm_interface->command_args[0], command_arg);
		ppm_unpack((ppm_interface->command_args[1] << 16) + ppm_interface->command_args[2], ppm_interface->command_args[0]);
		ppm_sdram_pointer = &ppm_sdram[ppm_interface->command_args[0] / 2]; // optional ?
		ppm_interface->reg_status_1 &= ~0x0004;
		ppm_interface->reg_status_2.bits.busy = 0;
		break;
	// setup sdram pointer for read
	case 0xdb:
		logerror("Setup sdram read @0x%06x, size 0x%04x (0x%02x)\n", swapshorts(ppm_interface->command_args_long[0]), ppm_interface->command_args[2], command_arg);
		ppm_sdram_pointer = &ppm_sdram[swapshorts(ppm_interface->command_args_long[0]) / 2];
		// read size = ppm_interface->command_args[2];
		break;
	// eeprom load
	case 0xdf:
		switch (command_arg)
		{
		case 1:
		case 2:
		case 3:
			memcpy(&ppm_dual_port_ram[ppm_interface->command_args[0] / 2], &m_nvram[(0x200 + command_arg * 0x200) / 2], 0x100);
			break;
		case 4:
			memcpy(&ppm_dual_port_ram[ppm_interface->command_args[0] / 2], &m_nvram[0], 0x200);
			break;
		}
		break;
	// eeprom save (command_args[0]=0xbeef)
	case 0xe0:
		switch (command_arg)
		{
		case 1:
		case 2:
		case 3:
			memcpy(&m_nvram[(0x200 + command_arg * 0x200) / 2], &ppm_dual_port_ram[ppm_interface->command_args[1] / 2], 0x100);
			break;
		case 4:
			memcpy(&m_nvram[0], &ppm_dual_port_ram[ppm_interface->command_args[1] / 2], 0x200);
			break;
		}
		ppm_interface->reg_status_2.bits.eeprom_error1 = 0;
		ppm_interface->reg_status_2.bits.eeprom_error2 = 0;
		break;

	case 0xe7: // send some data over network
		#if PPM_DEBUG
		printf("CMD 0x%04x (0x%04x / 0x%04x):", ppm_interface->reg_command, ppm_interface->command_args[0], ppm_interface->command_args[1]);
		for (int i = 0; i < (ppm_interface->command_args[1] + 1) / 2; i++)
			printf(" 0x%04x", ppm_interface->command_args[2 + i]);
		printf("\n");
		#endif
		ppm_interface->reg_status_2.bits.megawire_data_in = 1;			     // pretend data is in
		ppm_interface->network_data[0x10 / 2] = ppm_interface->command_args[0] + 16; // pretend 16 bytes in?
		// 0x24: ranking fetch
		// 0x82: list access points
		// 0x84: set account/password (16 bytes each, null terminated if less than 16)
		break;

	// set vram block budget
	case 0xec:
		ppm_vram_set_budget(ppm_interface->command_args[1]);
		if (ppm_interface->command_args[0])
			logerror("PPM command 0xec: unk arg 0 (0x%04x)", ppm_interface->command_args[0]);
		break;
	// load single block (used in obj viewer)
	case 0xf2:
		// todo: check block range
		ppm_unpack(ppm_block_addr(ppm_interface->command_args[0]), 0x9000);
		ppm_unpack(ppm_block_addr(ppm_interface->command_args[0]), 0x9200);
		ppm_sdram_pointer = &ppm_sdram[0x9000 / 2];
		break;
	// load a stamp for scaling
	case 0xf4:
		ppm_unpack(swapshorts(ppm_interface->command_args_long[0]), 0, true);
		break;
	// scale current stamp to desired size
	case 0xf5:
		// logerror("stamp resize - 0x%04x / 0x%04x / 0x%04x / 0x%04x\n", ppm_interface->command_args[0], ppm_interface->command_args[1], ppm_interface->command_args[2], ppm_interface->command_args[3]);
		ppm_stamp_rescale(ppm_interface->command_args[0], ppm_interface->command_args[1], ppm_interface->command_args[2], ppm_interface->command_args[3]);
		break;
	default:
		#if PPM_DEBUG
		printf("Unprocessed command 0x%04x\n", ppm_interface->reg_command);
		#endif
		break;
	}
	// ack command
	// cart returns 0 tho only MSB is checked by the game
	ppm_interface->reg_command = 0;
}

uint32_t md_rom_paprium_device::ppm_sdram_read_u32(uint32_t addr)
{
	// fetching non aligned U32
	return ((ppm_sdram[addr / 2 + 1] << 16) + ppm_sdram[addr / 2]);
}

uint32_t md_rom_paprium_device::ppm_unpack(uint32_t source_addr, uint32_t dest_addr, bool is_scaling_stamp)
{
	// packed data is byte width, ^1 on all addresses to fix endian mumbo jumbo
	uint8_t code, count, data_byte;
	uint32_t copy_addr;
	uint32_t initial_dest_addr = dest_addr;
	uint8_t *packed_data = (uint8_t *)m_rom;
	uint8_t *unpacked_data = is_scaling_stamp ? (uint8_t *)ppm_scale_stamp : (uint8_t *)ppm_sdram;

	switch (packed_data[source_addr++ ^ 1])
	{
	case 0x80:
		while ((code = packed_data[source_addr++ ^ 1]))
		{
			switch (count = code & 0x3f, code >> 6)
			{
			case 0:
				while (count--)
					unpacked_data[dest_addr++ ^ 1] = packed_data[source_addr++ ^ 1];
				break;
			case 1:
				data_byte = packed_data[source_addr++ ^ 1];
				while (count--)
					unpacked_data[dest_addr++ ^ 1] = data_byte;
				break;
			case 2:
				copy_addr = dest_addr - packed_data[source_addr++ ^ 1];
				while (count--)
					unpacked_data[dest_addr++ ^ 1] = unpacked_data[copy_addr++ ^ 1];
				break;
			case 3:
				while (count--)
					unpacked_data[dest_addr++ ^ 1] = 0;
				break;
			}
		}
		break;
	case 0x81:
		uint16_t copy_size, literal_size;

		while ((code = packed_data[source_addr++ ^ 1]) != 0x11) // unconfirmed end code
		{
			switch (code >> 4)
			{
			case 0:
				copy_size = 0;
				literal_size = code ? (3 + (code & 0x1f)) : (0x12 + packed_data[source_addr++ ^ 1]);
				break;
			case 1:
				if ((copy_size = 2 + (code & 0x7)) == 2)
					copy_size = 9 + packed_data[source_addr++ ^ 1];
				literal_size = packed_data[source_addr ^ 1] & 0x3;
				copy_addr = dest_addr - 0x4000 - (((packed_data[(source_addr + 1) ^ 1] << 8) + packed_data[source_addr ^ 1]) >> 2);
				source_addr += 2;
				break;
			case 2:
			case 3:
				if ((copy_size = (code & 0x1f)))
					copy_size += 2;
				else
				{
					copy_size = 0x21;
					while (!packed_data[source_addr++ ^ 1])
						copy_size += 0xff;
					copy_size += packed_data[(source_addr - 1) ^ 1];
				}
				literal_size = packed_data[source_addr ^ 1] & 0x3;
				copy_addr = dest_addr - 1 - (((packed_data[(source_addr + 1) ^ 1] << 8) + packed_data[source_addr ^ 1]) >> 2);
				source_addr += 2;
				break;
			default:
				copy_size = (code >> 5) + 1;
				literal_size = code & 0x3;
				copy_addr = dest_addr - 1 - (((code >> 2) & 0x7) + (packed_data[source_addr ^ 1] << 3));
				source_addr++;
				break;
			}
			while (copy_size--)
				unpacked_data[dest_addr++ ^ 1] = unpacked_data[copy_addr++ ^ 1];
			while (literal_size--)
				unpacked_data[dest_addr++ ^ 1] = packed_data[source_addr++ ^ 1];
		}
		break;
	default:
		logerror("unknown packer format, wrong address? (0x%06x)\n", source_addr - 1);
		break;
	}
	return dest_addr - initial_dest_addr;
}

void md_rom_paprium_device::ppm_setup_data(uint32_t bgm_file, uint32_t unk1_file, uint32_t smp_file, uint32_t unk2_file, uint32_t sfx_file, uint32_t anm_file, uint32_t blk_file)
{
	uint32_t unpack_addr = PPM_DATA_START_ADDR;

	logerror("=== data setup ===\n");
	// setup misc data (unpack some of thoses to sdram):
	// bgm data
	logerror("BGM data: 0x%06x\n", bgm_file);
	ppm_bgm_tracks_base_addr = bgm_file;
	ppm_bgm_tracks_offsets = (uint32_t *)&m_rom[bgm_file / 2];
	// ?? data
	logerror("uk1 data: 0x%06x > 0x%06x\n", unk1_file, unpack_addr);
	ppm_unk_data_addr = unpack_addr;
	unpack_addr += ppm_unpack(unk1_file, unpack_addr);
	++unpack_addr &= 0xfffffffeu; // word align
	// samples data (WavPack file)
	logerror("SMP data: 0x%06x >  0x%06x\n", smp_file, unpack_addr);
	// unpacked data is about 1.4MB
	// ?? data
	logerror("uk2 data: 0x%06x > 0x%06x\n", unk2_file, unpack_addr);
	ppm_unk_data2_addr = unpack_addr;
	unpack_addr += ppm_unpack(unk2_file, unpack_addr);
	++unpack_addr &= 0xfffffffeu; // word align
	// sfx data
	logerror("SFX data: 0x%06x\n", sfx_file);
	ppm_sfx_base_addr = sfx_file;
	ppm_sfx_data = (ppm_sfx_struct *)&m_rom[sfx_file / 2];
	ppm_sfx_struct *fx = &ppm_sfx_data[1];
	for (int x = 1; x <= ppm_sfx_data[0].length; fx++, x++)
		logerror("\tSFX 0x%02x: 0x%06x / 0x%04x / 0x%04x\n", x, swapshorts(fx->offset), fx->attributes, fx->length);
	// animation data
	logerror("ANM data: 0x%06x > 0x%06x\n", anm_file, unpack_addr);
	ppm_anim_data_base_addr = unpack_addr;
	unpack_addr += ppm_unpack(anm_file, unpack_addr);
	++unpack_addr &= 0xfffffffeu; // word align
	ppm_anim_data = (uint32_t *)&ppm_sdram[ppm_anim_data_base_addr / 2];
	logerror("anim data for 0x%x objs\n", ppm_anim_data[0]);
	for (uint16_t x = 1; x <= ppm_anim_data[0]; x++)
	{
		uint32_t anim_offset = ppm_anim_data[x];
		uint16_t anim_count = 0;
		while (ppm_anim_data[anim_count + (anim_offset / 4)] != 0xffffffffu)
			anim_count++;
		ppm_anim_max_index[x - 1] = anim_count - 1;
		logerror("\tobj 0x%02x (0x%06x): %d anims\n", x, anim_offset, anim_count);
		if (anim_offset & 0x3)
			printf("(unaligned pointers)\n");
	}
	// blocks data
	logerror("BLK data: 0x%06x\n", blk_file);
	ppm_gfx_blocks_base_addr = blk_file;
	ppm_gfx_blocks_offsets = (uint32_t *)&m_rom[blk_file / 2];
	ppm_max_gfx_block = swapshorts(ppm_gfx_blocks_offsets[0]) - 1;
	logerror("blocks addr: %06x, count: %d\n", ppm_gfx_blocks_base_addr, swapshorts(ppm_gfx_blocks_offsets[0]));

	logerror("unpack addr finish: 0x%06x / free: 0x%06x\n", unpack_addr, 0x200000 - unpack_addr);
	ppm_bgm_unpack_addr = unpack_addr;
}

void md_rom_paprium_device::ppm_vram_set_budget(uint16_t blocks)
{
	// temp failsafe
	if (blocks > 0x35)
	{
		logerror("Allocation error (0x%04x blocks)\n", blocks);
		blocks = 0x35;
	}
	ppm_vram_reset_blocks((ppm_vram_max_slot = blocks));
}

void md_rom_paprium_device::ppm_vram_reset_blocks(uint16_t last_block)
{
	ppm_vram_slot_struct *slot = &ppm_vram_slots[last_block];
	for (uint16_t i = last_block; i < PPM_MAX_VRAM_SLOTS; slot++, i++)
	{
		slot->block_num = 0;
		slot->usage = 0;
		slot->age = 0;
	}
}

uint16_t md_rom_paprium_device::ppm_vram_find_block(uint16_t num)
{ // return tile index
	ppm_vram_slot_struct *slot = ppm_vram_slots;
	for (uint16_t x = 0; x < ppm_vram_max_slot; slot++, x++)
		if (slot->block_num == num)
			return ((x + (x <= 0x30 ? 1 : 0x4b)) << 4);
	return 0;
}

uint16_t md_rom_paprium_device::ppm_vram_load_block(uint16_t num)
{
	if (!num)
		return 0;

	// is block already in VRAM?
	ppm_vram_slot_struct *slot = ppm_vram_slots;
	for (uint16_t x = 0; x < ppm_vram_max_slot; slot++, x++)
		if (slot->block_num == num)
		{
			slot->usage++;
			slot->age = 0;
			return ((x + (x <= 0x30 ? 1 : 0x4b)) << 4);
		}

	// enough DMA budget?
	if (ppm_interface->dma_remaining < 0x110)
		return 0;

	// find oldest slot to load into
	uint32_t max_age = 0;
	uint16_t block_index = 0xffff;
	slot = ppm_vram_slots;

	for (uint16_t x = 0; x < ppm_vram_max_slot; slot++, x++)
		if ((!slot->usage) && (slot->age > max_age))
		{
			max_age = slot->age;
			block_index = x;
		}

	// no slot available?
	if (block_index == 0xffff)
		return 0;

	// found slot & have budget, unpack and DMA the block
	slot = &ppm_vram_slots[block_index];
	slot->block_num = num;
	slot->usage++;
	slot->age = 0;

	ppm_unpack(ppm_block_addr(num), ppm_block_unpack_addr);
	ppm_block_unpack_addr += 0x200;

	ppm_dma_command_struct *dma_entry = &ppm_interface->dma_commands[ppm_interface->dma_commands_count++];
	ppm_interface->dma_remaining -= 0x110;
	dma_entry->autoinc = 0x8f02;
	dma_entry->lenH = 0x9401;
	dma_entry->lenL = 0x9300; // 0x200 words size
	dma_entry->srcH = 0x9700;
	dma_entry->srcM = 0x9660;
	dma_entry->srcL = 0x9500; // 0xc000 source

	block_index += (block_index <= 0x30 ? 1 : 0x4b); // translate index
	uint32_t command = (((block_index << 25) | (block_index >> 5)) & 0x3fff0003) | 0x40000080;
	dma_entry->cmdH = command >> 16;
	dma_entry->cmdL = command;

	return (block_index << 4);
}

void md_rom_paprium_device::ppm_obj_reset()
{
	ppm_vram_reset_blocks(0);

	memset(ppm_interface->obj_data, 0, sizeof(ppm_interface->obj_data));
	ppm_drawListPtr = ppm_drawList;
}

void md_rom_paprium_device::ppm_obj_add(uint8_t num)
{
	*ppm_drawListPtr++ = num;
}

void md_rom_paprium_device::ppm_obj_frame_start()
{
	ppm_drawListPtr = ppm_drawList;

	// all blocks unused
	ppm_vram_slot_struct *slot = ppm_vram_slots;
	for (uint16_t x = 0; x < PPM_MAX_VRAM_SLOTS; slot++, x++)
		slot->usage = 0;
}

void md_rom_paprium_device::ppm_obj_frame_end()
{
	ppm_block_unpack_addr = 0x9000;
	ppm_interface->dma_remaining = ppm_interface->dma_budget - ppm_interface->dma_total;

	uint8_t *ptr = ppm_drawList;
	while (ptr != ppm_drawListPtr)
		ppm_obj_render(*ptr++);

	ppm_vram_slot_struct *slot = ppm_vram_slots;
	for (uint16_t x = 0; x < PPM_MAX_VRAM_SLOTS; slot++, x++)
		if (!slot->usage)
			slot->age++;

	ppm_close_sprite_table();
	ppm_sdram_pointer = &ppm_sdram[0x9000 / 2];
}

void md_rom_paprium_device::ppm_close_sprite_table()
{
	ppm_sat_item_struct *sat_entry = &ppm_interface->sat_data[ppm_interface->sat_count];

	if (!ppm_interface->sat_count)
	{
		sat_entry->posY = 0x10;
		sat_entry->sizeNext = 0;
		sat_entry->attrs = 0;
		sat_entry->posX = 0x10;
		ppm_interface->sat_count++;
	}
	else
		(--sat_entry)->sizeNext &= 0xff00;

	ppm_dma_command_struct *dma_entry = &ppm_interface->dma_commands[ppm_interface->dma_commands_count++];
	dma_entry->autoinc = 0x8f02;

	uint16_t word_size = ppm_interface->sat_count * (sizeof(ppm_sat_item_struct) / 2);
	dma_entry->lenH = 0x9400 + (word_size >> 8);
	dma_entry->lenL = 0x9300 + (word_size & 0xff);

	uint32_t sat_addr = offsetof(ppm_interface_struct, sat_data) / 2;
	dma_entry->srcH = 0x9700 + ((sat_addr >> 16) & 0xff);
	dma_entry->srcM = 0x9600 + ((sat_addr >> 8) & 0xff);
	dma_entry->srcL = 0x9500 + (sat_addr & 0xff);

	// xfer to SAT location in VRAM (0xf000)
	dma_entry->cmdH = 0x7000;
	dma_entry->cmdL = 0x0083;
}

void md_rom_paprium_device::ppm_obj_render(uint16_t obj_slot)
{
	ppm_intf_object_struct *intf_obj = &ppm_interface->obj_data[obj_slot]; // interface obj
	ppm_obj_struct *handle = &ppm_object_handles[obj_slot];		       // internal handle

	if ((intf_obj->anim & 0xff) > ppm_anim_max_index[intf_obj->objID & 0xff])
	{
		logerror("anim over for ID %02x: 0x%04x/0x%04x\n", intf_obj->objID & 0xff, intf_obj->anim & 0xff, ppm_anim_max_index[intf_obj->objID & 0xff]);
		return;
	}

	uint32_t offset, data_offset;
	uint32_t previous_offset = handle->anim_offset;
	uint16_t previous_counter = handle->counter;

	//	printf("Slot 0x%02x (ID 0x%04x): ", obj_slot, intf_obj->objID);

	// set / update animation
	if ((intf_obj->objID & 0x8000) || (intf_obj->anim != handle->crtAnim) || (intf_obj->animCounter != handle->counter))
	{
		// fresh obj?
		if (intf_obj->objID & 0x8000)
			previous_offset = 0, previous_counter = 1;

		offset = ppm_anim_data[(intf_obj->objID & 0xff) + 1];		 // obj offset
		offset = ppm_anim_data[(offset >> 2) + (intf_obj->anim & 0xff)]; // anim offset
		data_offset = ppm_anim_data[offset >> 2] & 0xffffffu;		 // data offset

		handle->anim_offset = offset;
		handle->crtAnim = intf_obj->anim;
		handle->counter = intf_obj->animCounter;
	}
	else
	{
		// move to next frame

		// get current offset (previous frame)
		if (!(offset = handle->anim_offset))
			return;
		// read data pointer
		data_offset = ppm_anim_data[offset >> 2];

		if (data_offset & 0x80000000u)
		{
			// previous frame was not last, just move to next
			offset += 4;
		}
		else
		{
			// anim is over, do we have fallback anim?
			if (intf_obj->nextAnim != 0xffff)
			{
				// yes, switch anims
				intf_obj->anim = intf_obj->nextAnim;
				intf_obj->nextAnim = 0xffff; // unverified
				ppm_obj_render(obj_slot);
				return;
			}
			else
			{
				// no, take anim loop
				offset = ppm_anim_data[(offset + 4) >> 2] & 0xffffffu;
			}
		}

		if (!(handle->anim_offset = offset)) // store offset
			return;
		data_offset = ppm_anim_data[offset >> 2] & 0xffffffu;
		intf_obj->animCounter++;
		handle->counter++;
	}

	// render sprite to SAT
	// > data might not be 4 bytes aligned, access via ppm_sdram <
	ppm_spr_data_header_struct *spr_info = (ppm_spr_data_header_struct *)&ppm_sdram[(ppm_anim_data_base_addr + data_offset) >> 1];
	ppm_sat_item_struct *satEntry = &ppm_interface->sat_data[ppm_interface->sat_count];
	int16_t posX = intf_obj->posX;
	int16_t posY = intf_obj->posY;

	bool blocks_available = true;
	// load new spr data
	ppm_spr_data_struct *spr_data = &spr_info->sprites[0];
	for (uint16_t x = 0; x < spr_info->count; spr_data++, x++)
	{
		if (!spr_data->blockNum)
			continue;
		if (!ppm_vram_load_block(spr_data->blockNum))
		{
			blocks_available = false;
			// break; need to iterate all to keep resevations?
		}
	}

	if (!blocks_available)
	{
		#if PPM_DEBUG
		printf(">>> block budget out / ");
		#endif
		if (previous_offset)
		{
			// restore offset/counter
			handle->anim_offset = previous_offset;
			handle->counter = previous_counter;
			intf_obj->animCounter = previous_counter;
			data_offset = ppm_anim_data[previous_offset >> 2] & 0xffffff;
			spr_info = (ppm_spr_data_header_struct *)&ppm_sdram[(ppm_anim_data_base_addr + data_offset) >> 1];
			#if PPM_DEBUG
			printf("Obj #%02x: keep previous frame\n", obj_slot);
			#endif
		}
		else
		{
			#if PPM_DEBUG
			printf("Obj #%02x: no render\n", obj_slot);
			#endif
			return;
		}
	}

	spr_data = &spr_info->sprites[0];
	for (uint16_t x = 0; x < spr_info->count; spr_data++, x++)
	{
		posX += (intf_obj->attrs & 0x0800) ? spr_data->flipPosX : spr_data->posX;
		posY += spr_data->posY;
		if (!spr_data->blockNum)
			continue;

		// clipping
		if ((posX >= 320 + 128) ||
		    (posY >= 240 + 128) ||
		    (posX < 128 - ((((spr_data->size >> 2) & 0x3) + 1) * 8)) ||
		    (posY < 128 - (((spr_data->size & 0x3) + 1) * 8)))
			continue;

		ppm_interface->sat_count++;
		satEntry->posX = posX & 0x1ff;
		satEntry->posY = posY & 0x3ff;
		satEntry->sizeNext = ((spr_data->size & 0xf) << 8) + (ppm_interface->sat_count & 0xff);
		// attributes aren't quite right yet, some objects have wrong palette
//#		satEntry->attrs = ((spr_data->attrs & 0x98) << 8) ^ intf_obj->attrs ^ (ppm_vram_find_block(spr_data->blockNum) + spr_data->offset); // whole attr word?
	satEntry->attrs = ((spr_data->attrs & 0xf8) << 8) ^ intf_obj->attrs ^ (ppm_vram_find_block(spr_data->blockNum) + spr_data->offset); // whole attr word?
		// satEntry->attrs = ppm_dual_port_ram[0] ^ (spr_data->attrs << 8) ^ intf_obj->attrs ^ (ppm_vram_find_block(spr_data->blockNum) + spr_data->offset); // debug
		// satEntry->attrs = ((spr_data->attrs & ppm_dual_port_ram[0]) << 8) ^ intf_obj->attrs ^ (ppm_vram_find_block(spr_data->blockNum) + spr_data->offset); //whole attr word?
		// satEntry->attrs ^= (spr_info->flags & ppm_dual_port_ram[1]) << 8;
		satEntry++;
	}
	intf_obj->objID &= 0x7fff;
}

void md_rom_paprium_device::ppm_stamp_rescale(uint16_t window_start, uint16_t window_end, uint16_t factor, uint16_t stamp_offset)
{
	// prepare temp buffer
	uint8_t scaled_stamp[128][32];
	memset(scaled_stamp, 0, sizeof(scaled_stamp));

	// scale the stamp in temp buffer
	float offset = (float)stamp_offset;
	float adder = (float)factor / 64;
	for (uint16_t y = window_start; y < window_end; offset += adder, y++)
		memcpy(&scaled_stamp[y][0], &ppm_scale_stamp[(int)offset][0], 32);

	// translate data: 128*32px block, as 4*32 px strips
	// layout is weird because... reasons?
	for (uint16_t s = 0; s < 32; s++) // 4px strips
	{
		// uint16_t clmn = ((s << 4) & 0xffe0) + (s & 1 ? 0x200 : 0);
		uint16_t clmn = ((s & 0xfe) << 4) + ((s & 1) << 9);
		for (uint16_t y = 0; y < 32; y++)
		{
			// 1 word contains 4 pixels
			ppm_interface->scaling_buffer[clmn + y] =
			    (((scaled_stamp[(s << 2) + 0][y ^ 1] & 0xf0) |
			      (scaled_stamp[(s << 2) + 1][y ^ 1] & 0x0f))
			     << 8) +
			    (((scaled_stamp[(s << 2) + 2][y ^ 1] & 0xf0) |
			      (scaled_stamp[(s << 2) + 3][y ^ 1] & 0x0f)));
		}
	}
}
