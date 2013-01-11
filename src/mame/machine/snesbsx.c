/***************************************************************************

  snesbsx.c

  File to handle emulation of the SNES "BS-X Satellaview".

  Based on byuu's research.

  TODO: basically everything. The hardware can dynamically remap where
  the memory handlers read/write (this will probably require some more
  rethinking of the way we use snes_ram), see bsx_update_memory_map

***************************************************************************/


enum
{
	SNES_BSX_CARTROM = 0,
	SNES_BSX_PRAM,
	SNES_BSX_FLASH
};


struct snes_bsx_state
{
	// base regs
	// we don't emulate the base unit yet

	// cart regs
	UINT8 cart_regs[16];

	// flash regs
	UINT32 command;
	UINT8 write_old;
	UINT8 write_new;

	int flash_enable;
	int read_enable;
	int write_enable;

	UINT8 *pram;
	int ram_source;
};

static snes_bsx_state bsx_state;


static void bsx_update_memory_map(void)
{
	bsx_state.ram_source = BIT(bsx_state.cart_regs[0x01], 7) ? SNES_BSX_PRAM : SNES_BSX_FLASH;
//  UINT8 *RAM = (bsx_state.cart_regs[0x01] & 0x80) == 0x00 ? space.machine().root_device().memregion("flash")->base() : bsx_state.pram;

	logerror("BSX: updated memory map, current RAM: %d", bsx_state.ram_source);
	if (!BIT(bsx_state.cart_regs[0x02], 7))
	{
		//LoROM mapping
		// 0x00-0x7d:0x8000-0xfff -> RAM (either flash or pram)
		// 0x80-0xff:0x8000-0xfff -> RAM
	}
	else
	{
		//HiROM mapping
		// 0x00-0x3f:0x8000-0xfff -> RAM - 'shadowed'
		// 0x40-0x7d:0x0000-0xfff -> RAM
		// 0x80-0xbf:0x8000-0xfff -> RAM - 'shadowed'
		// 0xc0-0xff:0x0000-0xfff -> RAM
	}

	if (BIT(bsx_state.cart_regs[0x03], 7))
	{
		// 0x60-0x6f:0x0000-0xffff -> PRAM
	}

	if (!BIT(bsx_state.cart_regs[0x05], 7))
	{
		// 0x40-0x4f:0x0000-0xffff -> PRAM
	}

	if (!BIT(bsx_state.cart_regs[0x06], 7))
	{
		// 0x50-0x5f:0x0000-0xffff -> PRAM
	}

	if (BIT(bsx_state.cart_regs[0x07], 7))
	{
		// 0x00-0x1f:0x8000-0xffff -> CART
	}

	if (BIT(bsx_state.cart_regs[0x08], 7))
	{
		// 0x80-0x9f:0x8000-0xffff -> CART
	}

	// 0x20-0x3f:0x6000-0x7fff -> PRAM
	// 0x70-0x77:0x0000-0xffff -> PRAM
}

static READ8_HANDLER( bsx_read )
{
	if ((offset & 0xf0ffff) == 0x005000)
	{
		//$[00-0f]:5000 MMIO
		UINT8 n = (offset >> 16) & 0x0f;
		return bsx_state.cart_regs[n];
	}

	if ((offset & 0xf8f000) == 0x105000)
	{
		//$[10-17]:[5000-5fff] SRAM
		return bsx_state.pram[((offset >> 16) & 7) * 0x1000 + (offset & 0xfff)];
	}

	return 0x00;
}


static WRITE8_HANDLER( bsx_write )
{
	if ((offset & 0xf0ffff) == 0x005000)
	{
		//$[00-0f]:5000 MMIO
		UINT8 n = (offset >> 16) & 0x0f;
		bsx_state.cart_regs[n] = data;
		if ((n == 0x0e) && (data & 0x80))
			bsx_update_memory_map();
	}

	if ((offset & 0xf8f000) == 0x105000)
	{
		//$[10-17]:[5000-5fff] SRAM
		bsx_state.pram[((offset >> 16) & 7) * 0x1000 + (offset & 0xfff)] = data;
	}
}

static void bsx_init( running_machine &machine )
{
	memset(bsx_state.cart_regs, 0, ARRAY_LENGTH(bsx_state.cart_regs));

	bsx_state.cart_regs[0x07] = 0x80;
	bsx_state.cart_regs[0x08] = 0x80;

	bsx_state.pram = auto_alloc_array(machine, UINT8, 0x80000);

	bsx_update_memory_map();
	// saves
}

#ifdef UNUSED_FUNCTION
static READ8_HANDLER( bsx_flash_read )
{
	UINT8 *FLASH = space.machine().root_device().memregion("flash")->base();

	if (offset == 0x0002)
	{
		if (bsx_state.flash_enable)
			return 0x80;
	}

	if (offset == 0x5555)
	{
		if (bsx_state.flash_enable)
			return 0x80;
	}

	if (bsx_state.read_enable && offset >= 0xff00 && offset <= 0xff13)
	{
		//read flash cartridge vendor information
		switch(offset - 0xff00)
		{
			case 0x00: return 0x4d;
			case 0x01: return 0x00;
			case 0x02: return 0x50;
			case 0x03: return 0x00;
			case 0x04: return 0x00;
			case 0x05: return 0x00;
			case 0x06: return 0x2a;  //0x2a = 8mbit, 0x2b = 16mbit (not known to exist, though BIOS recognizes ID)
			case 0x07: return 0x00;
			default:   return 0x00;
		}
	}

	return FLASH[offset];
}

static WRITE8_HANDLER( bsx_flash_write )
{
	if ((offset & 0xff0000) == 0)
	{
		bsx_state.write_old = bsx_state.write_new;
		bsx_state.write_new = data;

		if (bsx_state.write_enable && bsx_state.write_old == bsx_state.write_new)
		{
//          currently we have the flashcart loaded in a rom_region -> we cannot write yet
		}
	}
	else
	{
		if (bsx_state.write_enable)
		{
			//          currently we have the flashcart loaded in a rom_region -> we cannot write yet
		}
	}

	if (offset == 0x0000)
	{
		bsx_state.command <<= 8;
		bsx_state.command |= data;

		if ((bsx_state.command & 0xffff) == 0x38d0)
		{
			bsx_state.flash_enable = 1;
			bsx_state.read_enable  = 1;
		}
	}

	if (offset == 0x2aaa)
	{
		bsx_state.command <<= 8;
		bsx_state.command |= data;
	}

	if (offset == 0x5555)
	{
		bsx_state.command <<= 8;
		bsx_state.command |= data;

		if ((bsx_state.command & 0xffffff) == 0xaa5570)
		{
			bsx_state.write_enable = 0;
		}

		if ((bsx_state.command & 0xffffff) == 0xaa55a0)
		{
			bsx_state.write_old = 0x00;
			bsx_state.write_new = 0x00;
			bsx_state.flash_enable = 1;
			bsx_state.write_enable = 1;
		}

		if ((bsx_state.command & 0xffffff) == 0xaa55f0)
		{
			bsx_state.flash_enable = 0;
			bsx_state.read_enable  = 0;
			bsx_state.write_enable = 0;
		}
	}
}

#endif
