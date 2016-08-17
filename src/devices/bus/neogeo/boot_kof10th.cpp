// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 The King of Fighters 10th Anniversary Bootleg cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_kof10th.h"


//-------------------------------------------------
//  neogeo_kof10th_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_KOF10TH_CART = &device_creator<neogeo_kof10th_cart>;


neogeo_kof10th_cart::neogeo_kof10th_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KOF10TH_CART, "Neo Geo KOF 10th Ann Bootleg Cart", tag, owner, clock, "neocart_kof10th", __FILE__)
{}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_kof10th_cart::device_start()
{
	save_pointer(NAME(m_fixed), 0x40000);
	save_item(NAME(m_special_bank));
	save_item(NAME(m_cart_ram));
	save_item(NAME(m_cart_ram2));
}

void neogeo_kof10th_cart::device_reset()
{
	m_special_bank = 0;
	memset(m_cart_ram, 0x00, 0x2000);
	memset(m_cart_ram2, 0x00, 0x20000);
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

static MACHINE_CONFIG_FRAGMENT( kof10th_cart )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_kof10th_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof10th_cart );
}


void neogeo_kof10th_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kof10th_decrypt(cpuregion, cpuregion_size);
	memcpy(m_cart_ram2, (UINT8 *)cpuregion + 0xe0000, 0x20000);
	m_fixed = (get_fixed_size()) ? get_fixed_base() : get_region_fixed_base();
}


/* this uses RAM based tiles for the text layer, however the implementation
 is incomplete, at the moment the S data is copied from the program rom on
 start-up instead */

UINT32 neogeo_kof10th_cart::get_bank_base(UINT16 sel)
{
	UINT32 bank = 0x100000 + ((sel & 7) << 20);
	if (bank >= 0x700000)
		bank = 0x100000;
	return bank;
}

UINT16 neogeo_kof10th_cart::get_helper()
{
	return m_cart_ram[0xffc];
}

UINT32 neogeo_kof10th_cart::get_special_bank()
{
	return m_special_bank;
}

READ16_MEMBER(neogeo_kof10th_cart::protection_r)
{
	return m_cart_ram[offset];
}

READ16_MEMBER(neogeo_kof10th_cart::addon_r)
{
	//  printf("kof10th_RAM2_r\n");
	return m_cart_ram2[offset];
}

WRITE16_MEMBER(neogeo_kof10th_cart::protection_w)
{
	if (offset < 0x40000/2)
	{
		if (!m_cart_ram[0xffe])
			COMBINE_DATA(&m_cart_ram2[(0x00000/2) + (offset & 0xffff)]);    // Write to RAM bank A
		else
			m_fixed[offset] = BITSWAP8(data, 7,6,0,4,3,2,1,5);  // Write S data on-the-fly
	}
	else if (offset >= 0xfe000/2)
	{
		if (offset == 0xffff0/2)
		{
			// Standard bankswitch
			//m_bankdev->neogeo_set_main_cpu_bank_address(get_bank_base(data));
		}
		else if (offset == 0xffff8/2 && m_cart_ram[0xffc] != data)
		{
			// Special bankswitch
			m_special_bank = (data & 1) ? 0x800000/2 : 0x700000/2;
		}
		COMBINE_DATA(&m_cart_ram[offset & 0xfff]);
	}
}
