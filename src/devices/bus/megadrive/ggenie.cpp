// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Game Genie pass-thorugh cart emulation


 based on Charles MacDonald's docs: http://cgfm2.emuviews.com/txt/genie.txt


 There is an interesting difference between Rev.0 and Rev.A
 After the codes has been entered, the former just performs
 a last write to the MODE register (m_gg_regs[0]) which both
 sets the enable bits for the 6 available cheats (in the low
 8 bits) and locks the GG so that later reads goes to the
 piggyback cart. The latter revision, instead, performs the
 same operations in two subsequent 8bit writes, accessing
 separately the low and high bits of the register.

 ***********************************************************************************************************/

#include "emu.h"
#include "ggenie.h"
#include "rom.h"


//-------------------------------------------------
//  md_rom_device - constructor
//-------------------------------------------------

const device_type MD_ROM_GAMEGENIE = &device_creator<md_rom_ggenie_device>;


md_rom_ggenie_device::md_rom_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, MD_ROM_GAMEGENIE, "MD Game Genie", tag, owner, clock, "md_ggenie", __FILE__),
						device_md_cart_interface( mconfig, *this ),
						m_exp(*this, "subslot"), m_gg_bypass(0), m_reg_enable(0)
				{
}


void md_rom_ggenie_device::device_start()
{
	save_item(NAME(m_gg_bypass));
	save_item(NAME(m_reg_enable));
	save_item(NAME(m_gg_regs));
	save_item(NAME(m_gg_addr));
	save_item(NAME(m_gg_data));
}

void md_rom_ggenie_device::device_reset()
{
	m_gg_bypass = 0;
	m_reg_enable = 0;
	memset(m_gg_regs, 0, sizeof(m_gg_regs));
	memset(m_gg_addr, 0, sizeof(m_gg_addr));
	memset(m_gg_data, 0, sizeof(m_gg_data));
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(md_rom_ggenie_device::read)
{
	if (!m_gg_bypass || !m_exp->m_cart)
	{
		if (m_reg_enable)
			return m_gg_regs[offset & 0x1f];
		else
			return m_rom[MD_ADDR(offset)];
	}

	if (m_exp->m_cart)
	{
		if (offset == m_gg_addr[0]/2 && BIT(m_gg_regs[0], 0))
			return m_gg_data[0];
		else if (offset == m_gg_addr[1]/2 && BIT(m_gg_regs[0], 1))
			return m_gg_data[1];
		else if (offset == m_gg_addr[2]/2 && BIT(m_gg_regs[0], 2))
			return m_gg_data[2];
		else if (offset == m_gg_addr[3]/2 && BIT(m_gg_regs[0], 3))
			return m_gg_data[3];
		else if (offset == m_gg_addr[4]/2 && BIT(m_gg_regs[0], 4))
			return m_gg_data[4];
		else if (offset == m_gg_addr[5]/2 && BIT(m_gg_regs[0], 5))
			return m_gg_data[5];
		else
			return m_exp->m_cart->read(space, offset);
	}
	else
		return 0xffff;
}

WRITE16_MEMBER(md_rom_ggenie_device::write)
{
	if (offset >= 0x40/2)
		return;

	if (ACCESSING_BITS_0_7)
		m_gg_regs[offset] = (m_gg_regs[offset] & 0xff00) | (data & 0x00ff);

	if (ACCESSING_BITS_8_15)
		m_gg_regs[offset] = (m_gg_regs[offset] & 0x00ff) | (data & 0xff00);

	//printf("write to 0x%X, data 0x%X\n", offset, data);

	// MODE
	if (offset == 0)
	{
		// bit10 set = read goes to piggyback cart
		if (data & 0x400)
			m_gg_bypass = 1;
		// bit10 unset = read goes to Game Genie ASIC/ROM
		else
		{
			m_gg_bypass = 0;

			// bit9 set = read goes to ASIC registers
			if (data & 0x200)
				m_reg_enable = 1;
			// bit9 unset = read goes to GG ROM
			else
				m_reg_enable = 0;
		}

		// LOCK bit
		if (data & 0x100)
		{
			// addresses
			m_gg_addr[0] = ((m_gg_regs[2]   & 0x3f) << 16) | m_gg_regs[3];
			m_gg_addr[1] = ((m_gg_regs[5]   & 0x3f) << 16) | m_gg_regs[6];
			m_gg_addr[2] = ((m_gg_regs[8]   & 0x3f) << 16) | m_gg_regs[9];
			m_gg_addr[3] = ((m_gg_regs[11]  & 0x3f) << 16) | m_gg_regs[12];
			m_gg_addr[4] = ((m_gg_regs[14]  & 0x3f) << 16) | m_gg_regs[15];
			m_gg_addr[5] = ((m_gg_regs[17]  & 0x3f) << 16) | m_gg_regs[18];

			// data
			m_gg_data[0] = m_gg_regs[4];
			m_gg_data[1] = m_gg_regs[7];
			m_gg_data[2] = m_gg_regs[10];
			m_gg_data[3] = m_gg_regs[13];
			m_gg_data[4] = m_gg_regs[16];
			m_gg_data[5] = m_gg_regs[19];

			//printf("mode %X\n", data);
			//for (int i = 0; i < 6; i++)
			//  printf("addr %d = 0x%X - data 0x%X\n", i, m_gg_addr[i], m_gg_data[i]);
		}
	}
	else if (offset == 1)
	{
		// RESET
		m_gg_regs[1] |= 1;
	}
}

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( ggenie_slot )
//-------------------------------------------------

static SLOT_INTERFACE_START(ggenie_sub_cart)
	SLOT_INTERFACE_INTERNAL("rom",  MD_STD_ROM)
	SLOT_INTERFACE_INTERNAL("rom_svp",  MD_STD_ROM)
	SLOT_INTERFACE_INTERNAL("rom_sram",  MD_ROM_SRAM)
	SLOT_INTERFACE_INTERNAL("rom_sramsafe",  MD_ROM_SRAM)
	SLOT_INTERFACE_INTERNAL("rom_fram",  MD_ROM_FRAM)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( ggenie_slot )
	MCFG_MD_CARTRIDGE_ADD("subslot", ggenie_sub_cart, nullptr)
	MCFG_MD_CARTRIDGE_NOT_MANDATORY
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor md_rom_ggenie_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ggenie_slot );
}
