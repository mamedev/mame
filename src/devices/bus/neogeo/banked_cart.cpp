// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#include "emu.h"
#include "banked_cart.h"



extern const device_type NEOGEO_BANKED_CART = &device_creator<neogeo_banked_cart_device>;


neogeo_banked_cart_device::neogeo_banked_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NEOGEO_BANKED_CART, "NeoGeo Banked Cartridge", tag, owner, clock, "neogeo_banked_cart", __FILE__),
	m_bank_cartridge(0),
	m_main_cpu_bank_address(0),
	m_region(0),
	m_region_size(0)

{
}

void neogeo_banked_cart_device::postload()
{
	_set_main_cpu_bank_address();
}

void neogeo_banked_cart_device::device_start()
{
	save_item(NAME(m_main_cpu_bank_address));
	machine().save().register_postload(save_prepost_delegate(FUNC(neogeo_banked_cart_device::postload), this));
}

void neogeo_banked_cart_device::device_reset()
{
}






/*************************************
 *
 *  Main CPU banking
 *
 *************************************/

void neogeo_banked_cart_device::_set_main_cpu_bank_address(void)
{
	if (!m_region)
		printf("region is null\n");

	if (!m_bank_cartridge)
		printf("bank_cartridge is null\n");

	if (m_region && m_bank_cartridge) m_bank_cartridge->set_base(m_region + m_main_cpu_bank_address);
}


void neogeo_banked_cart_device::neogeo_set_main_cpu_bank_address( UINT32 bank_address )
{
//  if (LOG_MAIN_CPU_BANKING) logerror("MAIN CPU PC %06x: neogeo_set_main_cpu_bank_address %06x\n", m_maincpu->pc(), bank_address);

	m_main_cpu_bank_address = bank_address;

	_set_main_cpu_bank_address();
}


WRITE16_MEMBER(neogeo_banked_cart_device::main_cpu_bank_select_w)
{
	UINT32 bank_address;
	UINT32 len = m_region_size;

	if ((len <= 0x100000) && (data & 0x07))
		logerror("PC %06x: warning: bankswitch to %02x but no banks available\n", space.device().safe_pc(), data);
	else
	{
		bank_address = ((data & 0x07) + 1) * 0x100000;

		if (bank_address >= len)
		{
			logerror("PC %06x: warning: bankswitch to empty bank %02x\n", space.device().safe_pc(), data);
			bank_address = 0x100000;
		}

		neogeo_set_main_cpu_bank_address(bank_address);
	}
}

void neogeo_banked_cart_device::init_banks(void)
{
	/* set initial main CPU bank */
	if (m_region_size > 0x100000)
		neogeo_set_main_cpu_bank_address(0x100000);
	else
		neogeo_set_main_cpu_bank_address(0x000000);
}

void neogeo_banked_cart_device::install_banks(running_machine& machine, cpu_device* maincpu, UINT8* region, UINT32 region_size)
{
	maincpu->space(AS_PROGRAM).install_read_bank(0x200000, 0x2fffff, 0, 0, "cartridge");
	maincpu->space(AS_PROGRAM).install_write_handler(0x2ffff0, 0x2fffff, write16_delegate(FUNC(neogeo_banked_cart_device::main_cpu_bank_select_w),this));

	m_bank_cartridge = machine.root_device().membank("cartridge");
	m_region = region;
	m_region_size = region_size;

	init_banks();
}
