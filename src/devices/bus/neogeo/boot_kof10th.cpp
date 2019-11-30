// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 The King of Fighters 10th Anniversary Bootleg cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_kof10th.h"


//-------------------------------------------------
//  neogeo_kof10th_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_KOF10TH_CART, neogeo_kof10th_cart_device, "neocart_kof10th", "Neo Geo KoF 10th Ann. Bootleg Cart")


neogeo_kof10th_cart_device::neogeo_kof10th_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KOF10TH_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_kof10th_cart_device::device_start()
{
	save_item(NAME(m_special_bank));
	save_item(NAME(m_cart_ram));
	save_item(NAME(m_cart_ram2));
}

void neogeo_kof10th_cart_device::device_reset()
{
	m_special_bank = 0;
	memset(m_cart_ram, 0x00, 0x2000);
	memset(m_cart_ram2, 0x00, 0x20000);
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_kof10th_cart_device::device_add_mconfig(machine_config &config)
{
	NEOBOOT_PROT(config, m_prot);
}


void neogeo_kof10th_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kof10th_decrypt(cpuregion, cpuregion_size);
	memcpy(m_cart_ram2, (uint8_t *)cpuregion + 0xe0000, 0x20000);
	m_fixed = (get_fixed_size()) ? get_fixed_base() : get_region_fixed_base();
	save_pointer(NAME(m_fixed), 0x40000);
}


/* this uses RAM based tiles for the text layer, however the implementation
 is incomplete, at the moment the S data is copied from the program rom on
 start-up instead */

uint32_t neogeo_kof10th_cart_device::get_bank_base(uint16_t sel)
{
	uint32_t bank = 0x100000 + ((sel & 7) << 20);
	if (bank >= 0x700000)
		bank = 0x100000;
	return bank;
}

uint16_t neogeo_kof10th_cart_device::get_helper()
{
	return m_cart_ram[0xffc];
}

uint32_t neogeo_kof10th_cart_device::get_special_bank()
{
	return m_special_bank;
}

READ16_MEMBER(neogeo_kof10th_cart_device::protection_r)
{
	return m_cart_ram[offset];
}

READ16_MEMBER(neogeo_kof10th_cart_device::addon_r)
{
	//  printf("kof10th_RAM2_r\n");
	return m_cart_ram2[offset];
}

WRITE16_MEMBER(neogeo_kof10th_cart_device::protection_w)
{
	if (offset < 0x40000/2)
	{
		if (!m_cart_ram[0xffe])
			COMBINE_DATA(&m_cart_ram2[(0x00000/2) + (offset & 0xffff)]);    // Write to RAM bank A
		else
			m_fixed[offset] = bitswap<8>(data, 7,6,0,4,3,2,1,5);  // Write S data on-the-fly
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
