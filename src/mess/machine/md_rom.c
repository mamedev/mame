/***********************************************************************************************************
 
 
 MegaDrive / Genesis cart emulation
 
 
 Here we emulate bankswitch / protection / NVRAM found on generic carts with no additional hardware


 ***********************************************************************************************************/

#include "emu.h"
#include "machine/md_rom.h"
#include "cpu/m68000/m68000.h"


//-------------------------------------------------
//  md_rom_device - constructor
//-------------------------------------------------

// BASE CARTS + NVRAM
const device_type MD_STD_ROM = &device_creator<md_std_rom_device>;
const device_type MD_ROM_SRAM = &device_creator<md_rom_sram_device>;
const device_type MD_ROM_FRAM = &device_creator<md_rom_fram_device>;

// BASE CARTS + PROTECTION / BANKSWITCH
const device_type MD_ROM_SSF2 = &device_creator<md_rom_ssf2_device>;
const device_type MD_ROM_BUGSLIFE = &device_creator<md_rom_bugslife_device>;
const device_type MD_ROM_SMOUSE = &device_creator<md_rom_smouse_device>;
const device_type MD_ROM_SMB = &device_creator<md_rom_smb_device>;
const device_type MD_ROM_SMB2 = &device_creator<md_rom_smb2_device>;
const device_type MD_ROM_SBUBL = &device_creator<md_rom_sbubl_device>;
const device_type MD_ROM_RX3 = &device_creator<md_rom_rx3_device>;
const device_type MD_ROM_MJLOV = &device_creator<md_rom_mjlov_device>;
const device_type MD_ROM_KOF98 = &device_creator<md_rom_kof98_device>;
const device_type MD_ROM_KOF99 = &device_creator<md_rom_kof99_device>;
const device_type MD_ROM_SOULB = &device_creator<md_rom_soulb_device>;
const device_type MD_ROM_CHINF3 = &device_creator<md_rom_chinf3_device>;
const device_type MD_ROM_ELFWOR = &device_creator<md_rom_elfwor_device>;
const device_type MD_ROM_YASECH = &device_creator<md_rom_yasech_device>;
const device_type MD_ROM_LION2 = &device_creator<md_rom_lion2_device>;
const device_type MD_ROM_LION3 = &device_creator<md_rom_lion3_device>;
const device_type MD_ROM_MCPIR = &device_creator<md_rom_mcpirate_device>;
const device_type MD_ROM_POKESTAD = &device_creator<md_rom_pokestad_device>;
const device_type MD_ROM_REALTEC = &device_creator<md_rom_realtec_device>;
const device_type MD_ROM_REDCL = &device_creator<md_rom_redcl_device>;
const device_type MD_ROM_SQUIR = &device_creator<md_rom_squir_device>;
const device_type MD_ROM_TOPF = &device_creator<md_rom_topf_device>;
const device_type MD_ROM_RADICA = &device_creator<md_rom_radica_device>;

// below ones are currently unused, because the protection is patched out
const device_type MD_ROM_MULAN = &device_creator<md_std_rom_device>;
const device_type MD_ROM_POKE = &device_creator<md_std_rom_device>;
const device_type MD_ROM_POKE2 = &device_creator<md_std_rom_device>;


md_std_rom_device::md_std_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, type, name, tag, owner, clock),
					device_md_cart_interface( mconfig, *this )
{
}

md_std_rom_device::md_std_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, MD_STD_ROM, "MD Standard cart", tag, owner, clock),
					device_md_cart_interface( mconfig, *this )
{
}

md_rom_sram_device::md_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_SRAM, "MD Standard cart + SRAM", tag, owner, clock)
{
}

md_rom_fram_device::md_rom_fram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_FRAM, "MD Standard cart + FRAM", tag, owner, clock)
{
}

md_rom_ssf2_device::md_rom_ssf2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_SSF2, "MD Super SF2", tag, owner, clock)
{
}

md_rom_mcpirate_device::md_rom_mcpirate_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_MCPIR, "MD Pirate Multicarts (Various)", tag, owner, clock)
{
}

md_rom_bugslife_device::md_rom_bugslife_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_BUGSLIFE, "MD A Bug's Life", tag, owner, clock)
{
}

md_rom_smouse_device::md_rom_smouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_SMOUSE, "MD Huan Le Tao Qi Shu / Smart Mouse", tag, owner, clock)
{
}

md_rom_smb_device::md_rom_smb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_SMB, "MD Super Mario Bros.", tag, owner, clock)
{
}

md_rom_smb2_device::md_rom_smb2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_SMB2, "MD Super Mario Bros. 2", tag, owner, clock)
{
}

md_rom_sbubl_device::md_rom_sbubl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_SBUBL, "MD Super Bubble Bobble", tag, owner, clock)
{
}

md_rom_rx3_device::md_rom_rx3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_SMB2, "MD Rockman X3", tag, owner, clock)
{
}

md_rom_mjlov_device::md_rom_mjlov_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_MJLOV, "MD Ma Jiang Qing Ren / Mahjong Lover", tag, owner, clock)
{
}

md_rom_kof98_device::md_rom_kof98_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_KOF98, "MD KOF 98", tag, owner, clock)
{
}

md_rom_kof99_device::md_rom_kof99_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_KOF99, "MD KOF 99", tag, owner, clock)
{
}

md_rom_soulb_device::md_rom_soulb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_SOULB, "MD Soul Blade", tag, owner, clock)
{
}

md_rom_chinf3_device::md_rom_chinf3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_CHINF3, "MD Chinese Fighter 3", tag, owner, clock)
{
}

md_rom_elfwor_device::md_rom_elfwor_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_ELFWOR, "MD Linghuan Daoshi Super Magician / Elf Wor", tag, owner, clock)
{
}

md_rom_yasech_device::md_rom_yasech_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_YASECH, "MD Ya Se Chuan Shuo", tag, owner, clock)
{
}

md_rom_lion2_device::md_rom_lion2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_LION2, "MD Lion King 2", tag, owner, clock)
{
}

md_rom_lion3_device::md_rom_lion3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_LION3, "MD Lion King 3", tag, owner, clock)
{
}

md_rom_pokestad_device::md_rom_pokestad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_POKESTAD, "MD Pokemon Stadium", tag, owner, clock)
{
}

md_rom_realtec_device::md_rom_realtec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_REALTEC, "MD Realtec", tag, owner, clock)
{
}

md_rom_redcl_device::md_rom_redcl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_REDCL, "MD Redcliff", tag, owner, clock)
{
}

md_rom_squir_device::md_rom_squir_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_SQUIR, "MD Squirrel King", tag, owner, clock)
{
}

md_rom_topf_device::md_rom_topf_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_TOPF, "MD Top Fighter", tag, owner, clock)
{
}

md_rom_radica_device::md_rom_radica_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: md_std_rom_device(mconfig, MD_ROM_RADICA, "MD Radica TV games", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void md_rom_ssf2_device::device_start()
{
	for (int i = 0; i < 7; i++)
		m_bank[i] = i;
	m_lastoff = -1;
	m_lastdata = -1;
	save_item(NAME(m_bank));
	save_item(NAME(m_lastoff));
	save_item(NAME(m_lastdata));
}

void md_rom_mcpirate_device::device_start()
{
	m_bank = 0;
	save_item(NAME(m_bank));
}

void md_rom_chinf3_device::device_start()
{
	m_bank = 0;
	save_item(NAME(m_bank));
}

void md_rom_lion2_device::device_start()
{
	m_prot1_data = 0;
	m_prot2_data = 0;
	save_item(NAME(m_prot1_data));
	save_item(NAME(m_prot2_data));
}

void md_rom_lion3_device::device_start()
{
	m_prot_data = 0;
	m_prot_cmd = 0;
	m_bank = 0;
	save_item(NAME(m_prot_data));
	save_item(NAME(m_prot_cmd));
	save_item(NAME(m_bank));
}

void md_rom_pokestad_device::device_start()
{
	m_bank = 0;
	save_item(NAME(m_bank));
}

void md_rom_realtec_device::device_start()
{
	m_bank_addr = 0; 
	m_bank_size = 0;
	m_old_bank_addr = -1;
	save_item(NAME(m_bank_addr));
	save_item(NAME(m_bank_size));
	save_item(NAME(m_old_bank_addr));
}

void md_rom_squir_device::device_start()
{
	m_latch = 0;
	save_item(NAME(m_latch));
}

void md_rom_topf_device::device_start()
{
	m_latch = 0;
	m_bank[0] = m_bank[1] = m_bank[2] = 0;
	save_item(NAME(m_latch));
	save_item(NAME(m_bank));
}

void md_rom_radica_device::device_start()
{
	m_bank = 0;
	save_item(NAME(m_bank));
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------
 CART + SRAM
 -------------------------------------------------*/

READ16_MEMBER(md_rom_sram_device::read)
{
	// since a lot of generic carts ends up here if loaded from fullpath
	// we access nvram only if m_nvram_handlers_installed has been turned on
	if (m_nvram_handlers_installed)
	{
		if (offset >= m_nvram_start/2 && offset < m_nvram_end/2 && m_nvram_active)
			return m_nvram[offset - m_nvram_start/2];
	}
	if (offset < 0x400000/2) 
		return m_rom[MD_ADDR(offset)]; 
	else 
		return 0xffff;
}

WRITE16_MEMBER(md_rom_sram_device::write)
{
	// since a lot of generic carts ends up here if loaded from fullpath
	// we access nvram only if m_nvram_handlers_installed has been turned on
	if (m_nvram_handlers_installed)
	{
		if (offset >= m_nvram_start/2 && offset <= m_nvram_end/2 && m_nvram_active && !m_nvram_readonly)
			m_nvram[offset - m_nvram_start/2] = data;
	}
}

WRITE16_MEMBER(md_rom_sram_device::write_a13)
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

READ16_MEMBER(md_rom_fram_device::read)
{
	if (offset >= m_nvram_start/2 && offset < m_nvram_end/2 && m_nvram_active)
		return m_nvram[offset - m_nvram_start/2];
	if (offset < 0x400000/2) 
		return m_rom[MD_ADDR(offset)]; 
	else 
		return 0xffff;
}

WRITE16_MEMBER(md_rom_fram_device::write_a13)
{
	if (offset == 0xf0/2)
		m_nvram_active = BIT(data, 0);
}


READ16_MEMBER(md_rom_fram_device::read_a13)
{
	if (offset == 0xf0/2)
		return m_nvram_active;
	else
		return 0xffff;
}

/*-------------------------------------------------
 SUPER STREET FIGHTERS 2
 -------------------------------------------------*/

READ16_MEMBER(md_rom_ssf2_device::read)
{
	if (offset < 0x400000/2)
		return m_rom[offset];
	else
		return 0xffff;
}

// I'm not very fond of the code below...
WRITE16_MEMBER(md_rom_ssf2_device::write_a13)
{
	if (offset >= 0xf0/2)
	{
		offset -= 0xf0/2;
		if ((m_lastoff != offset) || (m_lastdata != data))
		{
			m_lastoff = offset;
			m_lastdata = data;
			if (offset)	// bank 0 is not modified
			{
				UINT16 *ROM = get_rom_base();
				m_bank[offset] = data & 0xf;
				memcpy(ROM + offset * 0x080000/2, ROM + 0x400000/2 + (m_bank[offset] * 0x080000)/2, 0x080000);
			}
		}
	}
}

/*-------------------------------------------------
 PIRATE MULTICARTS
 -------------------------------------------------*/

READ16_MEMBER(md_rom_mcpirate_device::read)
{
	if (offset < 0x400000/2)
	{
		return m_rom[offset + (m_bank * 0x10000)/2];
	}
	else
	{
		return read(space, offset - 0x400000/2, 0xffff);
	}
}

WRITE16_MEMBER(md_rom_mcpirate_device::write_a13)
{
	offset <<= 1;
	if (offset < 0x40)
		m_bank = offset;
}

/*-------------------------------------------------
 A BUG'S LIFE
 -------------------------------------------------*/

READ16_MEMBER(md_rom_bugslife_device::read_a13)
{
	if (offset == 0)
		return 0x28;
	else
		return 0xffff;
}

/*-------------------------------------------------
 CHINESE FIGHTER 3
 -------------------------------------------------*/

READ16_MEMBER(md_rom_chinf3_device::read)
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
		UINT32 retdat = 0;
		/*
		 04dc10 chifi3, prot_r? 2800
		 04cefa chifi3, prot_r? 65262
		 */
		if (space.machine().device("maincpu")->safe_pc() == 0x01782) // makes 'VS' screen appear
		{
			retdat = space.machine().device("maincpu")->state().state_int(M68K_D3) & 0xff;
			retdat <<= 8;
			return retdat;
		}
		else if (space.machine().device("maincpu")->safe_pc() == 0x1c24) // background gfx etc.
		{
			retdat = space.machine().device("maincpu")->state().state_int(M68K_D3) & 0xff;
			retdat <<= 8;
			return retdat;
		}
		else if (space.machine().device("maincpu")->safe_pc() == 0x10c4a) // unknown
		{
			return space.machine().rand();
		}
		else if (space.machine().device("maincpu")->safe_pc() == 0x10c50) // unknown
		{
			return space.machine().rand();
		}
		else if (space.machine().device("maincpu")->safe_pc() == 0x10c52) // relates to the game speed..
		{
			retdat = space.machine().device("maincpu")->state().state_int(M68K_D4) & 0xff;
			retdat <<= 8;
			return retdat;
		}
		else if (space.machine().device("maincpu")->safe_pc() == 0x061ae)
		{
			retdat = space.machine().device("maincpu")->state().state_int(M68K_D3) & 0xff;
			retdat <<= 8;
			return retdat;
		}
		else if (space.machine().device("maincpu")->safe_pc() == 0x061b0)
		{
			retdat = space.machine().device("maincpu")->state().state_int(M68K_D3) & 0xff;
			retdat <<= 8;
			return retdat;
		}
		else
		{
			logerror("%06x chifi3, prot_r? %04x\n", space.machine().device("maincpu")->safe_pc(), offset);
		}
		return 0;
	}
	
	return m_rom[offset & 0x1fffff/2];
}

WRITE16_MEMBER(md_rom_chinf3_device::write)
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
			logerror("%06x chifi3, bankw? %04x %04x\n", space.device().safe_pc(), offset, data);
	}
}

/*-------------------------------------------------
 LINGHUAN DAOSHI SUPER MAGICIAN / ELF WOR
 -------------------------------------------------*/

READ16_MEMBER(md_rom_elfwor_device::read)
{
	/* It returns (0x55 @ 0x400000 OR 0xc9 @ 0x400004) AND (0x0f @ 0x400002 OR 0x18 @ 0x400006). 
	 It is probably best to add handlers for all 4 addresses. */
	if (offset == 0x400000/2)	return 0x5500;
	if (offset == 0x400002/2)	return 0x0f00;
	if (offset == 0x400004/2)	return 0xc900;
	if (offset == 0x400006/2)	return 0x1800;
	return m_rom[offset];
}

/*-------------------------------------------------
 HUAN LE TAO QI SHU / SMART MOUSE
 -------------------------------------------------*/

READ16_MEMBER(md_rom_smouse_device::read)
{
	if (offset == 0x400000/2)	return 0x5500;
	if (offset == 0x400002/2)	return 0x0f00;
	if (offset == 0x400004/2)	return 0xaa00;
	if (offset == 0x400006/2)	return 0xf000;
	return m_rom[offset];
}

/*-------------------------------------------------
 YA SE CHUAN SHUO
 -------------------------------------------------*/

READ16_MEMBER(md_rom_yasech_device::read)
{
	if (offset == 0x400000/2)	return 0x6300;
	if (offset == 0x400002/2)	return 0x9800;
	if (offset == 0x400004/2)	return 0xc900;
	if (offset == 0x400006/2)	return 0x1800;
	return m_rom[offset];
}

/*-------------------------------------------------
 KOF98
 -------------------------------------------------*/

READ16_MEMBER(md_rom_kof98_device::read)
{
	if (offset == 0x480000/2)	return 0xaa00;
	if (offset == 0x4800e0/2)	return 0xaa00;
	if (offset == 0x4824a0/2)	return 0xaa00;
	if (offset == 0x488880/2)	return 0xaa00;
	if (offset == 0x4a8820/2)	return 0x0a00;
	if (offset == 0x4f8820/2)	return 0x0000;
	return m_rom[offset];
}

/*-------------------------------------------------
 KOF 99
 -------------------------------------------------*/
// gfx glitch with the new code... uninitialized ram somewhere?
READ16_MEMBER(md_rom_kof99_device::read_a13)
{
	if (offset == 0x00/2)	return 0x00;	// startup protection check, chinese message if != 0
	if (offset == 0x02/2)	return 0x01;	// write 02 to a13002.. shift right 1?
	if (offset == 0x3e/2)	return 0x1f;	// write 3e to a1303e.. shift right 1?
	else	return 0xffff;
}

/*-------------------------------------------------
 LION KING 2
 -------------------------------------------------*/

READ16_MEMBER(md_rom_lion2_device::read)
{
	if (offset == 0x400002/2)	return m_prot1_data;
	if (offset == 0x400006/2)	return m_prot2_data;
	return m_rom[offset];
}

WRITE16_MEMBER(md_rom_lion2_device::write)
{
	if (offset == 0x400000/2)	m_prot1_data = data;
	if (offset == 0x400004/2)	m_prot2_data = data;
}

/*-------------------------------------------------
 LION KING 3
 -------------------------------------------------*/

READ16_MEMBER(md_rom_lion3_device::read)
{
	if (offset < 0x8000/2)
		return m_rom[offset + (m_bank * 0x8000)/2];
	else if (offset >= 0x600000/2 && offset < 0x700000/2)
	{	
		UINT16 retdata = 0;
		switch (offset & 0x7)
		{
			case 2:
				if (m_prot_cmd == 0)
					retdata = (m_prot_data << 1);
				else if (m_prot_cmd == 1)
					retdata = (m_prot_data >> 1);
				else if (m_prot_cmd == 2)
				{
					retdata = m_prot_data >> 4;
					retdata |= (m_prot_data & 0x0f) << 4;
				}
				else
				{
					/* printf("unk prot case %d\n", m_prot_cmd); */
					retdata =  (BIT(m_prot_data, 7) << 0);
					retdata |= (BIT(m_prot_data, 6) << 1);
					retdata |= (BIT(m_prot_data, 5) << 2);
					retdata |= (BIT(m_prot_data, 4) << 3);
					retdata |= (BIT(m_prot_data, 3) << 4);
					retdata |= (BIT(m_prot_data, 2) << 5);
					retdata |= (BIT(m_prot_data, 1) << 6);
					retdata |= (BIT(m_prot_data, 0) << 7);
				}
				break;
				
			default:
				logerror("protection read, unknown offset %x\n", offset & 0x7);
				break;
		}
		return retdata;
	}
	
	return m_rom[offset];
}

WRITE16_MEMBER(md_rom_lion3_device::write)
{
	if (offset >= 0x600000/2 && offset < 0x700000/2)
	{
		switch (offset & 0x7)
		{
			case 0x0:
				m_prot_data = data;
				break;
			case 0x1:
				m_prot_cmd = data;
				break;
			default:
				logerror("protection write, unknown offset %d\n", offset & 0x7);
				break;
		}
	}
	if (offset >= 0x700000/2 && offset < 0x800000/2)
	{
		switch (offset & 0x7)
		{
			case 0x0:
				m_bank = data & 0xffff;
				break;
			default:
				logerror("bank write, unknown offset %d\n", offset & 0x7);
				break;
		}
	}
}

/*-------------------------------------------------
 MA JIANG QING REN / MAHJONG LOVER
 -------------------------------------------------*/

READ16_MEMBER(md_rom_mjlov_device::read)
{
	if (offset == 0x400000/2)	return 0x9000;
	if (offset == 0x401000/2)	return 0xd300;
	return m_rom[offset];
}


/*-------------------------------------------------
 SUPER BUBBLE BOBBLE MD
 -------------------------------------------------*/

READ16_MEMBER(md_rom_sbubl_device::read)
{
	if (offset == 0x400000/2)	return 0x5500;
	if (offset == 0x400002/2)	return 0x0f00;
	return m_rom[offset];
}

/*-------------------------------------------------
 SOUL BLADE
 -------------------------------------------------*/

READ16_MEMBER(md_rom_soulb_device::read)
{
	if (offset == 0x400002/2)	return 0x9800;
	if (offset == 0x400004/2)	return 0xc900;
	if (offset == 0x400006/2)	return 0xf000;
	return m_rom[offset];
}

/*-------------------------------------------------
 POKEMON STADIUM / KAIJU
 -------------------------------------------------*/

READ16_MEMBER(md_rom_pokestad_device::read)
{
	if (offset < 0x8000/2)
		return m_rom[offset + (m_bank * 0x8000)/2];
	return m_rom[offset];
}

WRITE16_MEMBER(md_rom_pokestad_device::write)
{
	if (offset >= 0x700000/2 && offset < 0x800000/2)
		m_bank = data & 0x7f;
}

/*-------------------------------------------------
 REALTEC
 -------------------------------------------------*/

READ16_MEMBER(md_rom_realtec_device::read)
{
	if (offset < (m_bank_size * 0x20000))	// two banks of same (variable) size at the bottom of the rom
		return m_rom[offset + (m_bank_addr * 0x20000)/2];
	return m_rom[(offset & 0x1fff/2) + 0x7e000/2];	// otherwise it accesses the final 8k of the image
}

WRITE16_MEMBER(md_rom_realtec_device::write)
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

READ16_MEMBER(md_rom_redcl_device::read)
{
	if (offset == 0x400000/2)	return 0x55 << 8;
	if (offset == 0x400004/2)	return -0x56 << 8;
	return m_rom[offset];
}

/*-------------------------------------------------
 ROCKMAN X3
 -------------------------------------------------*/

READ16_MEMBER(md_rom_rx3_device::read_a13)
{
	if (offset == 0)
		return 0x0c;
	else
		return 0xffff;
}

/*-------------------------------------------------
 SQUIRREL KING
 -------------------------------------------------*/

READ16_MEMBER(md_rom_squir_device::read)
{
	if ((offset >= 0x400000/2) && (offset < 0x400008/2))
		return m_latch;
	return m_rom[offset];
}

WRITE16_MEMBER(md_rom_squir_device::write)
{
	if (offset >= 0x400000/2 && offset < 0x400008/2)
		m_latch = data;
}

/*-------------------------------------------------
 SUPER MARIO BROS
 -------------------------------------------------*/

READ16_MEMBER(md_rom_smb_device::read_a13)
{
	if (offset == 0)
		return 0x0c;
	else
		return 0xffff;
}

/*-------------------------------------------------
 SUPER MARIO BROS 2
 -------------------------------------------------*/

READ16_MEMBER(md_rom_smb2_device::read_a13)
{
	if (offset == 0)
		return 0x0a;
	else
		return 0xffff;
}

/*-------------------------------------------------
 TOP FIGHTER
 -------------------------------------------------*/

READ16_MEMBER(md_rom_topf_device::read)
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
		
		if (space.device().safe_pc()==0x1771a2) return 0x50;
		else
		{
			m_latch++;
			logerror("%06x topfig_6BD294_r %04x\n",space.device().safe_pc(), m_latch);
			return m_latch;
		}
	}
	if (offset == 0x6f5344/2)
	{
		if (space.device().safe_pc()==0x4C94E)
			return space.machine().device("maincpu")->state().state_int((M68K_D0)) & 0xff;
		else
		{
			m_latch++;
			logerror("%06x topfig_6F5344_r %04x\n", space.device().safe_pc(), m_latch);
			return m_latch;
		}
	}

	if (offset >= 0x20000/2 && offset < 0x28000/2)
		return m_rom[offset + (m_bank[0] * 0x188000)/2];

	if (offset >= 0x58000/2 && offset < 0x60000/2)
		return m_rom[offset + (m_bank[1] * 0x20000)/2];

	if (offset >= 0x60000/2 && offset < 0x68000/2)
		return m_rom[offset + (m_bank[2] * 0x110000)/2];

	return m_rom[offset];
}

WRITE16_MEMBER(md_rom_topf_device::write)
{
	if (offset >= 0x700000/2 && offset < 0x800000/2)
	{
		if (data == 0x002a)
			m_bank[2] = 1;	// == 0x2e*0x8000?!
		else if (data==0x0035) // characters ingame
			m_bank[0] = 1;	// == 0x35*0x8000
		else if (data==0x000f) // special moves
			m_bank[1] = 1; // == 0xf*0x8000
		else if (data==0x0000)
		{
			m_bank[0] = 0;
			m_bank[1] = 0;
			m_bank[2] = 0;
		}
		else
			logerror("%06x offset %06x, data %04x\n", space.device().safe_pc(), offset, data);
	}
}

/*-------------------------------------------------
 RADICA TV GAMES [to be split...]
 -------------------------------------------------*/

READ16_MEMBER(md_rom_radica_device::read)
{
	return m_rom[m_bank * 0x10000/2 + offset];
}

READ16_MEMBER(md_rom_radica_device::read_a13)
{
	if (offset < 0x80)
		m_bank = offset & 0x3f;
	return 0;
}
