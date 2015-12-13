// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 BS-X Satellaview cartridge emulation (for SNES/SFC)

 ***********************************************************************************************************/


// TODO: Emulate FLASH memory... (possibly using flash device?)


#include "emu.h"
#include "bsx.h"


//-------------------------------------------------
//  sns_rom_bsx_device - constructor
//-------------------------------------------------

const device_type SNS_ROM_BSX = &device_creator<sns_rom_bsx_device>;
const device_type SNS_LOROM_BSX = &device_creator<sns_rom_bsxlo_device>;
const device_type SNS_HIROM_BSX = &device_creator<sns_rom_bsxhi_device>;
const device_type SNS_BSMEMPAK = &device_creator<sns_rom_bsmempak_device>;


sns_rom_bsx_device::sns_rom_bsx_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: sns_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_base_unit(nullptr),
	access_00_1f(0),
	access_80_9f(0),
	access_40_4f(0),
	access_50_5f(0),
	access_60_6f(0),
	rom_access(0),
	m_slot(*this, "bs_slot")
{
}

sns_rom_bsx_device::sns_rom_bsx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_ROM_BSX, "SNES BS-X Cart", tag, owner, clock, "sns_rom_bsx", __FILE__),
	m_base_unit(nullptr),
	access_00_1f(0),
	access_80_9f(0),
	access_40_4f(0),
	access_50_5f(0),
	access_60_6f(0),
	rom_access(0),
						m_slot(*this, "bs_slot")
{
}

sns_rom_bsxlo_device::sns_rom_bsxlo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_BSX, "SNES Cart (LoROM) +  BS-X slot", tag, owner, clock, "sns_rom_bsxlo", __FILE__),
						m_slot(*this, "bs_slot")
{
}

sns_rom_bsxhi_device::sns_rom_bsxhi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom21_device(mconfig, SNS_HIROM_BSX, "SNES Cart (HiROM) +  BS-X slot", tag, owner, clock, "sns_rom_bsxhi", __FILE__),
						m_slot(*this, "bs_slot")
{
}

sns_rom_bsmempak_device::sns_rom_bsmempak_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_BSMEMPAK, "SNES BS-X Memory packs", tag, owner, clock, "sns_bsmempak", __FILE__), m_command(0), m_write_old(0), m_write_new(0), m_flash_enable(0), m_read_enable(0), m_write_enable(0)
				{
}


void sns_rom_bsx_device::device_start()
{
	m_base_unit = auto_alloc(machine(), BSX_base(machine()));
	m_base_unit->init();

	memset(m_cart_regs, 0x00, sizeof(m_cart_regs));
	m_cart_regs[7] = 0x80;
	m_cart_regs[8] = 0x80;
	access_update();

	save_item(NAME(m_cart_regs));
	save_item(NAME(access_00_1f));
	save_item(NAME(access_80_9f));
	save_item(NAME(access_40_4f));
	save_item(NAME(access_50_5f));
	save_item(NAME(access_60_6f));
	save_item(NAME(rom_access));
	save_item(NAME(m_pram));
}

void sns_rom_bsx_device::device_reset()
{
	memset(m_pram, 0xff, sizeof(m_pram));
}

void sns_rom_bsxlo_device::device_start()
{
}

void sns_rom_bsxhi_device::device_start()
{
}

void sns_rom_bsmempak_device::device_start()
{
	save_item(NAME(m_command));
	save_item(NAME(m_write_old));
	save_item(NAME(m_write_new));
	save_item(NAME(m_flash_enable));
	save_item(NAME(m_read_enable));
	save_item(NAME(m_write_enable));
}

void sns_rom_bsmempak_device::device_reset()
{
	m_command = 0;
	m_write_old = 0;
	m_write_new = 0;

	m_flash_enable = 0;
	m_read_enable = 0;
	m_write_enable = 0;
}



// BS-X Base Unit emulation, to be device-fied ?

BSX_base::BSX_base(running_machine &machine)
			: r2192_minute(0), m_machine(machine)
{
	m_machine.save().save_item(regs, "SNES_BSX/regs");
	m_machine.save().save_item(r2192_counter, "SNES_BSX/r2192_counter");
	m_machine.save().save_item(r2192_hour, "SNES_BSX/r2192_hour");
	m_machine.save().save_item(r2192_second, "SNES_BSX/r2192_second");
}

void BSX_base::init()
{
	memset(regs, 0x00, sizeof(regs));
	r2192_counter = 0;
	r2192_hour = 0;
	r2192_minute = 0;
	r2192_second = 0;
}


UINT8 BSX_base::read(UINT32 offset)
{
	offset &= 0xffff;
	if (offset < 0x2188 || offset >= 0x21a0)
	{
		osd_printf_debug("BS-X Base Unit reg read outside correct range!\n");
		return 0x00;
	}

	switch (offset)
	{
		// no 218b? no 218d? no 2191? no 2195? no 219a-219f?
		case 0x2192:
		{
			UINT8 counter = r2192_counter++;
			if (r2192_counter >= 18)
				r2192_counter = 0;

			if (counter == 0)
			{
				system_time curtime, *systime = &curtime;
				m_machine.current_datetime(curtime);
				r2192_hour   = systime->local_time.hour;
				r2192_minute = systime->local_time.minute;
				r2192_second = systime->local_time.second;
			}

			switch (counter)
			{
				case  0: return 0x00;  //???
				case  1: return 0x00;  //???
				case  2: return 0x00;  //???
				case  3: return 0x00;  //???
				case  4: return 0x00;  //???
				case  5: return 0x01;
				case  6: return 0x01;
				case  7: return 0x00;
				case  8: return 0x00;
				case  9: return 0x00;
				case 10: return r2192_second;
				case 11: return r2192_minute;
				case 12: return r2192_hour;
				case 13: return 0x00;  //???
				case 14: return 0x00;  //???
				case 15: return 0x00;  //???
				case 16: return 0x00;  //???
				case 17: return 0x00;  //???
			}
		}
			break;

		case 0x2193:
			return regs[offset - 0x2188] & ~0x0c;

		default:
			return regs[offset - 0x2188];
	}

	return 0x00;
}


void BSX_base::write(UINT32 offset, UINT8 data)
{
	offset &= 0xffff;
	if (offset < 0x2188 || offset >= 0x21a0)
	{
		osd_printf_debug("BS-X Base Unit reg write outside correct range!\n");
		return;
	}

	switch(offset)
	{
		// no 218d? no 2190? no 2195? no 2196? no 2198? no 219a-219f?
		case 0x218f:
			regs[6] >>= 1;  // 0x218e
			regs[6] = regs[7] - regs[6];    // 0x218f - 0x218e
			regs[7] >>= 1;  // 0x218f
			break;

		case 0x2191:
			regs[offset - 0x2188] = data;
			r2192_counter = 0;
			break;

		case 0x2192:
			regs[8] = data; // sets 0x2190
			break;

		default:
			regs[offset - 0x2188] = data;
			break;
	}
}

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( bs_slot )
//-------------------------------------------------

static SLOT_INTERFACE_START(bsx_cart)
	SLOT_INTERFACE_INTERNAL("bsmempak",  SNS_BSMEMPAK)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( bs_slot )
	MCFG_SNS_BSX_CARTRIDGE_ADD("bs_slot", bsx_cart, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sns_rom_bsx_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bs_slot );
}

machine_config_constructor sns_rom_bsxlo_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bs_slot );
}

machine_config_constructor sns_rom_bsxhi_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bs_slot );
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

// BS-X base + cart

void sns_rom_bsx_device::access_update()
{
	access_00_1f = BIT(m_cart_regs[0x07], 7);
	access_40_4f = !BIT(m_cart_regs[0x05], 7);
	access_50_5f = !BIT(m_cart_regs[0x06], 7);
	access_60_6f = BIT(m_cart_regs[0x03], 7);
	access_80_9f = BIT(m_cart_regs[0x08], 7);
	if (BIT(m_cart_regs[0x01], 7))
		rom_access = 0;
	else
	{
//      rom_access = BIT(m_cart_regs[0x02], 7) + 1;
		rom_access = 1; // for whatever reason bsxsore changes access mode here and then fails to read the ROM properly!
		printf("rom_access %s\n", !BIT(m_cart_regs[0x02], 7) ? "Lo" : "Hi");
	}
}

READ8_MEMBER(sns_rom_bsx_device::read_l)
{
	if (offset < 0x200000 && access_00_1f)
	{
		// 0x00-0x1f:0x8000-0xffff -> CART
		if (m_slot->m_cart && m_slot->m_cart->get_rom_size())
			return m_slot->m_cart->read_l(space, offset);
	}
	if (offset >= 0x200000 && offset < 0x400000)
	{
		// 0x20-0x3f:0x6000-0x7fff -> PRAM
		if ((offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000)
			return m_pram[offset & 0xffff];
	}
	if (offset >= 0x400000 && offset < 0x500000 && access_40_4f)
	{
		// 0x40-0x4f:0x0000-0xffff -> PRAM
		return m_pram[offset & 0x7ffff];
	}
	if (offset >= 0x500000 && offset < 0x600000 && access_50_5f)
	{
		// 0x50-0x5f:0x0000-0xffff -> PRAM
		return m_pram[offset & 0x7ffff];
	}
	if (offset >= 0x600000 && offset < 0x700000 && access_60_6f)
	{
		// 0x60-0x6f:0x0000-0xffff -> PRAM
		return m_pram[offset & 0x7ffff];
	}
	if (offset >= 0x700000 && offset < 0x780000)
	{
		// 0x70-0x77:0x0000-0xffff -> PRAM
		return m_pram[offset & 0x7ffff];
	}

	// if not in any of the cases above...
	//$00-3f|80-bf:8000-ffff
	//$40-7f|c0-ff:0000-ffff
	if (!rom_access)
		return m_pram[offset & 0x7ffff];
	else
	{
		int bank = (rom_access == 1) ? (offset / 0x10000) : (offset / 0x8000);
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}

	// never executed
	//return 0x00;
}


READ8_MEMBER(sns_rom_bsx_device::read_h)
{
	if (offset < 0x200000 && access_80_9f)
	{
		// 0x80-0x9f:0x8000-0xffff -> CART
		if (m_slot->m_cart && m_slot->m_cart->get_rom_size())
			return m_slot->m_cart->read_l(space, offset);
	}

	// if not in any of the cases above...
	//$00-3f|80-bf:8000-ffff
	//$40-7f|c0-ff:0000-ffff
	if (!rom_access)
		return m_pram[offset & 0x7ffff];
	else
	{
		int bank = (rom_access == 1) ? (offset / 0x10000) : (offset / 0x8000);
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}

	// never executed
	//return 0x00;
}

WRITE8_MEMBER(sns_rom_bsx_device::write_l)
{
	if (offset < 0x200000 && access_00_1f)
	{
		// write to cart...
		return;
	}
	if (offset >= 0x200000 && offset < 0x400000)
	{
		// 0x20-0x3f:0x6000-0x7fff -> PRAM
		if ((offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000)
			m_pram[offset & 0xffff] = data;
	}
	if (offset >= 0x400000 && offset < 0x500000 && access_40_4f)
	{
		// 0x40-0x4f:0x0000-0xffff -> PRAM
		m_pram[offset & 0x7ffff] = data;
	}
	if (offset >= 0x500000 && offset < 0x600000 && access_50_5f)
	{
		// 0x50-0x5f:0x0000-0xffff -> PRAM
		m_pram[offset & 0x7ffff] = data;
	}
	if (offset >= 0x600000 && offset < 0x700000 && access_60_6f)
	{
		// 0x60-0x6f:0x0000-0xffff -> PRAM
		m_pram[offset & 0x7ffff] = data;
	}
	if (offset >= 0x700000 && offset < 0x780000)
	{
		// 0x70-0x77:0x0000-0xffff -> PRAM
		m_pram[offset & 0x7ffff] = data;
	}

	// if not in any of the cases above...
	//$00-3f|80-bf:8000-ffff
	//$40-7f|c0-ff:0000-ffff
	if (!rom_access)
		m_pram[offset & 0x7ffff] = data;
}


WRITE8_MEMBER(sns_rom_bsx_device::write_h)
{
	if (offset < 0x200000 && access_80_9f)
	{
		// write to cart...
		return;
	}

	// if not in any of the cases above...
	//$00-3f|80-bf:8000-ffff
	//$40-7f|c0-ff:0000-ffff
	if (!rom_access)
		m_pram[offset & 0x7ffff] = data;
}


READ8_MEMBER(sns_rom_bsx_device::chip_read)
{
	if ((offset & 0xffff) >= 0x2188 && (offset & 0xffff) < 0x21a0)
		return m_base_unit->read(offset & 0xffff);

	if ((offset & 0xf0ffff) == 0x005000)    //$[00-0f]:5000 reg access
	{
		UINT8 n = (offset >> 16) & 0x0f;
		return m_cart_regs[n];
	}

	if ((offset & 0xf8f000) == 0x105000)    //$[10-17]:[5000-5fff] SRAM access
	{
		return m_nvram[((offset >> 16) & 7) * 0x1000 + (offset & 0xfff)];
	}

	return 0x00;
}

WRITE8_MEMBER(sns_rom_bsx_device::chip_write)
{
	if ((offset & 0xffff) >= 0x2188 && (offset & 0xffff) < 0x21a0)
		m_base_unit->write(offset & 0xffff, data);

	if ((offset & 0xf0ffff) == 0x005000)    //$[00-0f]:5000 reg access
	{
		UINT8 n = (offset >> 16) & 0x0f;
		m_cart_regs[n] = data;
		if (n == 0x0e && data & 0x80)
			access_update();
	}

	if ((offset & 0xf8f000) == 0x105000)    //$[10-17]:[5000-5fff] SRAM access
	{
		m_nvram[((offset >> 16) & 7) * 0x1000 + (offset & 0xfff)] = data;
	}
}


// LoROM cart w/BS-X slot

READ8_MEMBER(sns_rom_bsxlo_device::read_l)
{
	if (offset < 0x400000)
	{
		int bank = offset / 0x10000;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	// nothing [40-6f]
	// RAM [70-7f]
	return 0x00;
}

READ8_MEMBER(sns_rom_bsxlo_device::read_h)
{
	if (offset < 0x400000)
	{
		int bank = offset / 0x10000;
		if (offset < 0x200000)
			bank += 64;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	else if (offset < 0x700000)
	{
		if (m_slot->m_cart && m_slot->m_cart->get_rom_size())
			return m_slot->m_cart->read_h(space, offset);
	}
	// RAM [70-7f]
	return 0x00;
}


// HiROM cart w/BS-X slot

READ8_MEMBER(sns_rom_bsxhi_device::read_l)
{
	return read_h(space, offset);
}

READ8_MEMBER(sns_rom_bsxhi_device::read_h)
{
	if (offset < 0x200000 && (offset & 0xffff) >= 0x8000)
	{
		int bank = offset / 0x8000;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	if (offset >= 0x200000 && offset < 0x400000)
	{
		if (m_slot->m_cart && m_slot->m_cart->get_rom_size() && (offset & 0xffff) >= 0x8000)
			return m_slot->m_cart->read_h(space, offset);
	}
	if (offset >= 0x400000 && offset < 0x600000)
	{
		// TODO: Ongaku Tsukuru Kanadeeru does not like accesses in 0x0000-0x8000 here... investigate...
		int bank = offset / 0x8000;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	if (offset >= 0x600000)
	{
		if (m_slot->m_cart && m_slot->m_cart->get_rom_size())
			return m_slot->m_cart->read_h(space, offset);
	}
	return 0xff;
}

/*-------------------------------------------------
 BS-X Memory Packs
 -------------------------------------------------*/

// Here we're cheating a bit, for the moment, to avoid the need of BSX mempacks as a completely different device
// which would require separate loading routines
// Hence, we use low read handler for ROM access in the 0x8000-0xffff range (i.e. mempack mapped as LoROM) and
// hi read handler for ROM access in the 0x0000-0xffff range (i.e. mempack mapped as HiROM)...

READ8_MEMBER(sns_rom_bsmempak_device::read_l)
{
	int bank = offset / 0x10000;
	return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
}

READ8_MEMBER(sns_rom_bsmempak_device::read_h)
{
	int bank = offset / 0x8000;
	return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
}

WRITE8_MEMBER(sns_rom_bsmempak_device::write_l)
{
}
