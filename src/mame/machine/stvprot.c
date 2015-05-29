// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
/* ST-V hookup for 315-5881 encryption/compression chip */

/*

 Known ST-V Games using this kind of protection

 Astra Superstars (text layer gfx transfer)
 Elandoree (gfx transfer of textures)
 Final Fight Revenge (boot vectors etc.)
 Radiant Silvergun (game start protection)
 Steep Slope Sliders (gfx transfer of character portraits)
 Tecmo World Cup '98 (tecmo logo, player movement)

*/

#include "emu.h"
#include "includes/stv.h"





/*************************************
*
* Common Handlers
*
*************************************/

READ32_MEMBER( stv_state::common_prot_r )
{
	UINT32 *ROM = (UINT32 *)space.machine().root_device().memregion("abus")->base();

	if(m_abus_protenable & 0x00010000)//protection calculation is activated
	{
		if(offset == 3)
		{
			UINT8* base;
			UINT16 res = m_cryptdevice->do_decrypt(base);
			UINT16 res2 = m_cryptdevice->do_decrypt(base);
			res = ((res & 0xff00) >> 8) | ((res & 0x00ff) << 8);
			res2 = ((res2 & 0xff00) >> 8) | ((res2 & 0x00ff) << 8);

			return res2 | (res << 16);
		}
		return m_a_bus[offset];
	}
	else
	{
		if(m_a_bus[offset] != 0) return m_a_bus[offset];
		else return ROM[(0x02fffff0/4)+offset];
	}
}


UINT16 stv_state::crypt_read_callback(UINT32 addr)
{
	UINT16 dat= m_maincpu->space().read_word((0x02000000+2*addr));
	return ((dat&0xff00)>>8)|((dat&0x00ff)<<8);
}

WRITE32_MEMBER ( stv_state::common_prot_w )
{
	COMBINE_DATA(&m_a_bus[offset]);

	if (offset == 0)
	{
		COMBINE_DATA(&m_abus_protenable);
	}
	else if(offset == 2)
	{
		if (mem_mask&0xffff0000) m_cryptdevice->set_addr_low(data >> 16);
		if (mem_mask&0x0000ffff) m_cryptdevice->set_addr_high(data&0xffff);

	}
	else if(offset == 3)
	{
		COMBINE_DATA(&m_abus_protkey);

		m_cryptdevice->set_subkey(m_abus_protkey>>16);
	}
}

void stv_state::install_common_protection()
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x4fffff0, 0x4ffffff, read32_delegate(FUNC(stv_state::common_prot_r), this), write32_delegate(FUNC(stv_state::common_prot_w), this));
}

void stv_state::stv_register_protection_savestates()
{
	save_item(NAME(m_a_bus));
}
