// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver
#include "emu.h"
#include "video/ppu2c0x.h"
#include "machine/nvram.h"
#include "includes/playch10.h"

/*************************************
 *
 *  Init machine
 *
 *************************************/

void playch10_state::machine_reset()
{
	/* initialize latches and flip-flops */
	m_pc10_nmi_enable = m_pc10_dog_di = m_pc10_dispmask = m_pc10_sdcs = m_pc10_int_detect = 0;

	m_pc10_game_mode = m_pc10_dispmask_old = 0;

	m_cart_sel = 0;
	m_cntrl_mask = 1;

	m_input_latch[0] = m_input_latch[1] = 0;

	/* variables used only in MMC2 game (mapper 9)  */
	m_MMC2_bank[0] = m_MMC2_bank[1] = m_MMC2_bank[2] = m_MMC2_bank[3] = 0;
	m_MMC2_bank_latch[0] = m_MMC2_bank_latch[1] = 0xfe;

	/* reset the security chip */
	m_rp5h01->enable_w(1);
	m_rp5h01->enable_w(0);
	m_rp5h01->reset_w(0);
	m_rp5h01->reset_w(1);

	pc10_set_mirroring(m_mirroring);
}

void playch10_state::machine_start()
{
	m_vrom = memregion("gfx2")->base();

	/* allocate 4K of nametable ram here */
	/* move to individual boards as documentation of actual boards allows */
	m_nt_ram = std::make_unique<UINT8[]>(0x1000);

	machine().device("ppu")->memory().space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8_delegate(FUNC(playch10_state::pc10_chr_r),this), write8_delegate(FUNC(playch10_state::pc10_chr_w),this));
	machine().device("ppu")->memory().space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(playch10_state::pc10_nt_r),this),write8_delegate(FUNC(playch10_state::pc10_nt_w),this));

	if (nullptr != m_vram)
		set_videoram_bank(0, 8, 0, 8);
	else pc10_set_videorom_bank(0, 8, 0, 8);

	nvram_device *nvram = machine().device<nvram_device>("nvram");
	if (nvram != nullptr)
		nvram->set_base(memregion("cart" )->base() + 0x6000, 0x1000);
}

MACHINE_START_MEMBER(playch10_state,playch10_hboard)
{
	m_vrom = memregion("gfx2")->base();

	/* allocate 4K of nametable ram here */
	/* move to individual boards as documentation of actual boards allows */
	m_nt_ram = std::make_unique<UINT8[]>(0x1000);
	/* allocate vram */

	m_vram = std::make_unique<UINT8[]>(0x2000);

	machine().device("ppu")->memory().space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8_delegate(FUNC(playch10_state::pc10_chr_r),this), write8_delegate(FUNC(playch10_state::pc10_chr_w),this));
	machine().device("ppu")->memory().space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(playch10_state::pc10_nt_r),this), write8_delegate(FUNC(playch10_state::pc10_nt_w),this));
}

/*************************************
 *
 *  BIOS ports handling
 *
 *************************************/

CUSTOM_INPUT_MEMBER(playch10_state::pc10_int_detect_r)
{
	return ~m_pc10_int_detect & 1;
}

WRITE8_MEMBER(playch10_state::pc10_SDCS_w)
{
	/*
	    Hooked to CLR on LS194A - Sheet 2, bottom left.
	    Drives character and color code to 0.
	    It's used to keep the screen black during redraws.
	    Also hooked to the video sram. Prevent writes.
	*/
	m_pc10_sdcs = ~data & 1;
}

WRITE8_MEMBER(playch10_state::pc10_CNTRLMASK_w)
{
	m_cntrl_mask = ~data & 1;
}

WRITE8_MEMBER(playch10_state::pc10_DISPMASK_w)
{
	m_pc10_dispmask = ~data & 1;
}

WRITE8_MEMBER(playch10_state::pc10_SOUNDMASK_w)
{
	/* should mute the APU - unimplemented yet */
}

WRITE8_MEMBER(playch10_state::pc10_NMIENABLE_w)
{
	m_pc10_nmi_enable = data & 1;
}

WRITE8_MEMBER(playch10_state::pc10_DOGDI_w)
{
	m_pc10_dog_di = data & 1;
}

WRITE8_MEMBER(playch10_state::pc10_GAMERES_w)
{
	machine().device("cart")->execute().set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE );
}

WRITE8_MEMBER(playch10_state::pc10_GAMESTOP_w)
{
	machine().device("cart")->execute().set_input_line(INPUT_LINE_HALT, (data & 1) ? CLEAR_LINE : ASSERT_LINE );
}

WRITE8_MEMBER(playch10_state::pc10_PPURES_w)
{
	if (data & 1)
		machine().device("ppu")->reset();
}

READ8_MEMBER(playch10_state::pc10_detectclr_r)
{
	m_pc10_int_detect = 0;

	return 0;
}

WRITE8_MEMBER(playch10_state::pc10_CARTSEL_w)
{
	m_cart_sel &= ~(1 << offset);
	m_cart_sel |= (data & 1) << offset;
}


/*************************************
 *
 *  RP5H01 handling
 *
 *************************************/

READ8_MEMBER(playch10_state::pc10_prot_r)
{
	int data = 0xe7;

	/* we only support a single cart connected at slot 0 */
	if (m_cart_sel == 0)
	{
		data |= ((~m_rp5h01->counter_r()) << 4) & 0x10;  /* D4 */
		data |= (m_rp5h01->data_r() << 3) & 0x08;        /* D3 */
	}
	return data;
}

WRITE8_MEMBER(playch10_state::pc10_prot_w)
{
	/* we only support a single cart connected at slot 0 */
	if (m_cart_sel == 0)
	{
		m_rp5h01->test_w(data & 0x10);       /* D4 */
		m_rp5h01->clock_w(data & 0x08);      /* D3 */
		m_rp5h01->reset_w(~data & 0x01);     /* D0 */
	}
}


/*************************************
 *
 *  Input Ports
 *
 *************************************/

WRITE8_MEMBER(playch10_state::pc10_in0_w)
{
	/* Toggling bit 0 high then low resets both controllers */
	if (data & 1)
		return;

	/* load up the latches */
	m_input_latch[0] = ioport("P1")->read();
	m_input_latch[1] = ioport("P2")->read();

	/* apply any masking from the BIOS */
	if (m_cntrl_mask)
	{
		/* mask out select and start */
		m_input_latch[0] &= ~0x0c;
	}
}

READ8_MEMBER(playch10_state::pc10_in0_r)
{
	int ret = (m_input_latch[0]) & 1;

	/* shift */
	m_input_latch[0] >>= 1;

	/* some games expect bit 6 to be set because the last entry on the data bus shows up */
	/* in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there. */
	ret |= 0x40;

	return ret;
}

READ8_MEMBER(playch10_state::pc10_in1_r)
{
	int ret = (m_input_latch[1]) & 1;

	/* shift */
	m_input_latch[1] >>= 1;

	/* do the gun thing */
	if (m_pc10_gun_controller)
	{
		int trigger = ioport("P1")->read();
		int x = ioport("GUNX")->read();
		int y = ioport("GUNY")->read();
		UINT32 pix, color_base;

		/* no sprite hit (yet) */
		ret |= 0x08;

		/* get the pixel at the gun position */
		pix = m_ppu->get_pixel(x, y);

		/* get the color base from the ppu */
		color_base = m_ppu->get_colorbase();

		/* look at the screen and see if the cursor is over a bright pixel */
		if ((pix == color_base + 0x20) || (pix == color_base + 0x30) ||
			(pix == color_base + 0x33) || (pix == color_base + 0x34))
		{
			ret &= ~0x08; /* sprite hit */
		}

		/* now, add the trigger if not masked */
		if (!m_cntrl_mask)
		{
			ret |= (trigger & 2) << 3;
		}
	}

	/* some games expect bit 6 to be set because the last entry on the data bus shows up */
	/* in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there. */
	ret |= 0x40;

	return ret;
}
/*************************************
 *
 *  PPU External bus handlers
 *
 *************************************/

WRITE8_MEMBER(playch10_state::pc10_nt_w)
{
	int page = ((offset & 0xc00) >> 10);
	m_nametable[page][offset & 0x3ff] = data;
}

READ8_MEMBER(playch10_state::pc10_nt_r)
{
	int page = ((offset & 0xc00) >> 10);
	return m_nametable[page][offset & 0x3ff];
}

WRITE8_MEMBER(playch10_state::pc10_chr_w)
{
	int bank = offset >> 10;
	if (m_chr_page[bank].writable)
	{
		m_chr_page[bank].chr[offset & 0x3ff] = data;
	}
}

READ8_MEMBER(playch10_state::pc10_chr_r)
{
	int bank = offset >> 10;
	return m_chr_page[bank].chr[offset & 0x3ff];
}

void playch10_state::pc10_set_mirroring(int mirroring )
{
	switch (mirroring)
	{
	case PPU_MIRROR_LOW:
		m_nametable[0] = m_nametable[1] = m_nametable[2] = m_nametable[3] = m_nt_ram.get();
		break;
	case PPU_MIRROR_HIGH:
		m_nametable[0] = m_nametable[1] = m_nametable[2] = m_nametable[3] = m_nt_ram.get() + 0x400;
		break;
	case PPU_MIRROR_HORZ:
		m_nametable[0] = m_nt_ram.get();
		m_nametable[1] = m_nt_ram.get();
		m_nametable[2] = m_nt_ram.get() + 0x400;
		m_nametable[3] = m_nt_ram.get() + 0x400;
		break;
	case PPU_MIRROR_VERT:
		m_nametable[0] = m_nt_ram.get();
		m_nametable[1] = m_nt_ram.get() + 0x400;
		m_nametable[2] = m_nt_ram.get();
		m_nametable[3] = m_nt_ram.get()+ 0x400;
		break;
	case PPU_MIRROR_NONE:
	default:
		m_nametable[0] = m_nt_ram.get();
		m_nametable[1] = m_nt_ram.get() + 0x400;
		m_nametable[2] = m_nt_ram.get() + 0x800;
		m_nametable[3] = m_nt_ram.get() + 0xc00;
		break;
	}
}

/* SIZE MAPPINGS *\
 * old       new *
 * 512         8 *
 * 256         4 *
 * 128         2 *
 *  64         1 *
\*****************/

void playch10_state::pc10_set_videorom_bank( int first, int count, int bank, int size )
{
	int i, len;
	/* first = first bank to map */
	/* count = number of 1K banks to map */
	/* bank = index of the bank */
	/* size = size of indexed banks (in KB) */
	/* note that this follows the original PPU banking and might be overly complex */

	/* yeah, this is probably a horrible assumption to make.*/
	/* but the driver is 100% consistant */

	len = memregion("gfx2")->bytes();
	len /= 0x400;   // convert to KB
	len /= size;    // convert to bank resolution
	len--;          // convert to mask
	bank &= len;    // should be the right mask

	for (i = 0; i < count; i++)
	{
		m_chr_page[i + first].writable = 0;
		m_chr_page[i + first].chr=m_vrom + (i * 0x400) + (bank * size * 0x400);
	}
}

void playch10_state::set_videoram_bank( int first, int count, int bank, int size )
{
	int i;
	/* first = first bank to map */
	/* count = number of 1K banks to map */
	/* bank = index of the bank */
	/* size = size of indexed banks (in KB) */
	/* note that this follows the original PPU banking and might be overly complex */

	/* assumes 8K of vram */
	/* need 8K to fill address space */
	/* only pinbot (8k) banks at all */

	for (i = 0; i < count; i++)
	{
		m_chr_page[i + first].writable = 1;
		m_chr_page[i + first].chr = m_vram.get() + (((i * 0x400) + (bank * size * 0x400)) & 0x1fff);
	}
}

/*************************************
 *
 *  Common init for all games
 *
 *************************************/

DRIVER_INIT_MEMBER(playch10_state,playch10)
{
	m_vram = nullptr;

	/* set the controller to default */
	m_pc10_gun_controller = 0;

	/* default mirroring */
	m_mirroring = PPU_MIRROR_NONE;
}

/**********************************************************************************
 *
 *  Game and Board-specific initialization
 *
 **********************************************************************************/

/* Gun games */

DRIVER_INIT_MEMBER(playch10_state,pc_gun)
{
	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* we have no vram, make sure switching games doesn't point to an old allocation */
	m_vram = nullptr;

	/* set the control type */
	m_pc10_gun_controller = 1;
}


/* Horizontal mirroring */

DRIVER_INIT_MEMBER(playch10_state,pc_hrz)
{
	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* setup mirroring */
	m_mirroring = PPU_MIRROR_HORZ;
}

/* MMC1 mapper, used by D and F boards */


WRITE8_MEMBER(playch10_state::mmc1_rom_switch_w)
{
	/* basically, a MMC1 mapper from the nes */
	static int size16k, switchlow, vrom4k;

	int reg = (offset >> 13);

	/* reset mapper */
	if (data & 0x80)
	{
		m_mmc1_shiftreg = m_mmc1_shiftcount = 0;

		size16k = 1;
		switchlow = 1;
		vrom4k = 0;

		return;
	}

	/* see if we need to clock in data */
	if (m_mmc1_shiftcount < 5)
	{
		m_mmc1_shiftreg >>= 1;
		m_mmc1_shiftreg |= (data & 1) << 4;
		m_mmc1_shiftcount++;
	}

	/* are we done shifting? */
	if (m_mmc1_shiftcount == 5)
	{
		/* reset count */
		m_mmc1_shiftcount = 0;

		/* apply data to registers */
		switch (reg)
		{
			case 0:     /* mirroring and options */
				{
					int _mirroring;

					vrom4k = m_mmc1_shiftreg & 0x10;
					size16k = m_mmc1_shiftreg & 0x08;
					switchlow = m_mmc1_shiftreg & 0x04;

					switch (m_mmc1_shiftreg & 3)
					{
						case 0:
							_mirroring = PPU_MIRROR_LOW;
						break;

						case 1:
							_mirroring = PPU_MIRROR_HIGH;
						break;

						case 2:
							_mirroring = PPU_MIRROR_VERT;
						break;

						default:
						case 3:
							_mirroring = PPU_MIRROR_HORZ;
						break;
					}

					/* apply mirroring */
					pc10_set_mirroring(_mirroring);
				}
			break;

			case 1: /* video rom banking - bank 0 - 4k or 8k */
				if (m_vram)
					set_videoram_bank(0, (vrom4k) ? 4 : 8, (m_mmc1_shiftreg & 0x1f), 4);
				else
					pc10_set_videorom_bank(0, (vrom4k) ? 4 : 8, (m_mmc1_shiftreg & 0x1f), 4);
			break;

			case 2: /* video rom banking - bank 1 - 4k only */
				if (vrom4k)
				{
					if (m_vram)
						set_videoram_bank(0, (vrom4k) ? 4 : 8, (m_mmc1_shiftreg & 0x1f), 4);
					else
						pc10_set_videorom_bank(4, 4, (m_mmc1_shiftreg & 0x1f), 4);
				}
			break;

			case 3: /* program banking */
				{
					int bank = (m_mmc1_shiftreg & m_mmc1_rom_mask) * 0x4000;
					UINT8 *prg = memregion("cart")->base();

					if (!size16k)
					{
						/* switch 32k */
						memcpy(&prg[0x08000], &prg[0x010000 + bank], 0x8000);
					}
					else
					{
						/* switch 16k */
						if (switchlow)
						{
							/* low */
							memcpy(&prg[0x08000], &prg[0x010000 + bank], 0x4000);
						}
						else
						{
							/* high */
							memcpy(&prg[0x0c000], &prg[0x010000 + bank], 0x4000);
						}
					}
				}
			break;
		}
	}
}

/**********************************************************************************/
/* A Board games (Track & Field, Gradius) */

WRITE8_MEMBER(playch10_state::aboard_vrom_switch_w)
{
	pc10_set_videorom_bank(0, 8, (data & 3), 8);
}

DRIVER_INIT_MEMBER(playch10_state,pcaboard)
{
	/* switches vrom with writes to the $803e-$8041 area */
	machine().device("cart")->memory().space(AS_PROGRAM).install_write_handler(0x8000, 0x8fff, write8_delegate(FUNC(playch10_state::aboard_vrom_switch_w),this));

	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* set the mirroring here */
	m_mirroring = PPU_MIRROR_VERT;

	/* we have no vram, make sure switching games doesn't point to an old allocation */
	m_vram = nullptr;
}

/**********************************************************************************/
/* B Board games (Contra, Rush N' Attach, Pro Wrestling) */

WRITE8_MEMBER(playch10_state::bboard_rom_switch_w)
{
	int bankoffset = 0x10000 + ((data & 7) * 0x4000);
	UINT8 *prg = memregion("cart")->base();

	memcpy(&prg[0x08000], &prg[bankoffset], 0x4000);
}

DRIVER_INIT_MEMBER(playch10_state,pcbboard)
{
	UINT8 *prg = memregion("cart")->base();

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	/* Roms are banked at $8000 to $bfff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(playch10_state::bboard_rom_switch_w),this));

	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* allocate vram */
	m_vram = std::make_unique<UINT8[]>(0x2000);

	/* set the mirroring here */
	m_mirroring = PPU_MIRROR_VERT;
	/* special init */
	set_videoram_bank(0, 8, 0, 8);
}

/**********************************************************************************/
/* C Board games (The Goonies) */

WRITE8_MEMBER(playch10_state::cboard_vrom_switch_w)
{
	pc10_set_videorom_bank(0, 8, ((data >> 1) & 1), 8);
}

DRIVER_INIT_MEMBER(playch10_state,pccboard)
{
	/* switches vrom with writes to $6000 */
	machine().device("cart")->memory().space(AS_PROGRAM).install_write_handler(0x6000, 0x6000, write8_delegate(FUNC(playch10_state::cboard_vrom_switch_w),this));

	/* we have no vram, make sure switching games doesn't point to an old allocation */
	m_vram = nullptr;

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/**********************************************************************************/
/* D Board games (Rad Racer) */

DRIVER_INIT_MEMBER(playch10_state,pcdboard)
{
	UINT8 *prg = memregion("cart")->base();

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	m_mmc1_rom_mask = 0x07;

	/* MMC mapper at writes to $8000-$ffff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(playch10_state::mmc1_rom_switch_w),this));


	/* common init */
	DRIVER_INIT_CALL(playch10);
	/* allocate vram */
	m_vram = std::make_unique<UINT8[]>(0x2000);
	/* special init */
	set_videoram_bank(0, 8, 0, 8);
}

/* D Board games with extra ram (Metroid) */

DRIVER_INIT_MEMBER(playch10_state,pcdboard_2)
{
	/* extra ram at $6000-$7fff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_ram(0x6000, 0x7fff);

	/* common init */
	DRIVER_INIT_CALL(pcdboard);

	/* allocate vram */
	m_vram = std::make_unique<UINT8[]>(0x2000);
	/* special init */
	set_videoram_bank(0, 8, 0, 8);
}

/**********************************************************************************/
/* E Board games (Mike Tyson's Punchout) - BROKEN - FIX ME */

/* callback for the ppu_latch */
void playch10_state::mapper9_latch(offs_t offset )
{
	if((offset & 0x1ff0) == 0x0fd0 && m_MMC2_bank_latch[0] != 0xfd)
	{
		m_MMC2_bank_latch[0] = 0xfd;
		pc10_set_videorom_bank(0, 4, m_MMC2_bank[0], 4);
	}
	else if((offset & 0x1ff0) == 0x0fe0 && m_MMC2_bank_latch[0] != 0xfe)
	{
		m_MMC2_bank_latch[0] = 0xfe;
		pc10_set_videorom_bank(0, 4, m_MMC2_bank[1], 4);
	}
	else if((offset & 0x1ff0) == 0x1fd0 && m_MMC2_bank_latch[1] != 0xfd)
	{
		m_MMC2_bank_latch[1] = 0xfd;
		pc10_set_videorom_bank(4, 4, m_MMC2_bank[2], 4);
	}
	else if((offset & 0x1ff0) == 0x1fe0 && m_MMC2_bank_latch[1] != 0xfe)
	{
		m_MMC2_bank_latch[1] = 0xfe;
		pc10_set_videorom_bank(4, 4, m_MMC2_bank[3], 4);
	}
}

WRITE8_MEMBER(playch10_state::eboard_rom_switch_w)
{
	/* a variation of mapper 9 on a nes */
	switch (offset & 0x7000)
	{
		case 0x2000: /* code bank switching */
			{
				int bankoffset = 0x10000 + (data & 0x0f) * 0x2000;
				UINT8 *prg = memregion("cart")->base();
				memcpy(&prg[0x08000], &prg[bankoffset], 0x2000);
			}
		break;

		case 0x3000: /* gfx bank 0 - 4k */
			m_MMC2_bank[0] = data;
			if (m_MMC2_bank_latch[0] == 0xfd)
				pc10_set_videorom_bank(0, 4, data, 4);
		break;

		case 0x4000: /* gfx bank 0 - 4k */
			m_MMC2_bank[1] = data;
			if (m_MMC2_bank_latch[0] == 0xfe)
				pc10_set_videorom_bank(0, 4, data, 4);
		break;

		case 0x5000: /* gfx bank 1 - 4k */
			m_MMC2_bank[2] = data;
			if (m_MMC2_bank_latch[1] == 0xfd)
				pc10_set_videorom_bank(4, 4, data, 4);
		break;

		case 0x6000: /* gfx bank 1 - 4k */
			m_MMC2_bank[3] = data;
			if (m_MMC2_bank_latch[1] == 0xfe)
				pc10_set_videorom_bank(4, 4, data, 4);
		break;

		case 0x7000: /* mirroring */
			pc10_set_mirroring(data ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

		break;
	}
}

DRIVER_INIT_MEMBER(playch10_state,pceboard)
{
	ppu2c0x_device *ppu = machine().device<ppu2c0x_device>("ppu");
	UINT8 *prg = memregion("cart")->base();

	/* we have no vram, make sure switching games doesn't point to an old allocation */
	m_vram = nullptr;

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	/* basically a mapper 9 on a nes */
	machine().device("cart")->memory().space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(playch10_state::eboard_rom_switch_w),this));

	/* ppu_latch callback */
	ppu->set_latch(ppu2c0x_latch_delegate(FUNC(playch10_state::mapper9_latch),this));

	/* nvram at $6000-$6fff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_ram(0x6000, 0x6fff);

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/**********************************************************************************/
/* F Board games (Ninja Gaiden, Double Dragon) */

DRIVER_INIT_MEMBER(playch10_state,pcfboard)
{
	UINT8 *prg = memregion("cart")->base();
	UINT32 len = memregion("cart")->bytes();

	/* we have no vram, make sure switching games doesn't point to an old allocation */
	m_vram = nullptr;

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	m_mmc1_rom_mask = ((len - 0x10000) / 0x4000) - 1;

	/* MMC mapper at writes to $8000-$ffff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(playch10_state::mmc1_rom_switch_w),this));

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/* F Board games with extra ram (Baseball Stars) */

DRIVER_INIT_MEMBER(playch10_state,pcfboard_2)
{
	/* extra ram at $6000-$6fff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_ram(0x6000, 0x6fff);

	m_vram = nullptr;

	/* common init */
	DRIVER_INIT_CALL(pcfboard);
}

/**********************************************************************************/
/* G Board games (Super Mario Bros. 3) */


void playch10_state::gboard_scanline_cb( int scanline, int vblank, int blanked )
{
	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		int priorCount = m_IRQ_count;
		if (m_IRQ_count == 0)
			m_IRQ_count = m_IRQ_count_latch;
		else
			m_IRQ_count--;

		if (m_IRQ_enable && !blanked && (m_IRQ_count == 0) && priorCount) // according to blargg the latter should be present as well, but it breaks Rampart and Joe & Mac US: they probably use the alt irq!
		{
			machine().device("cart")->execute().set_input_line(0, HOLD_LINE);
		}
	}
}

WRITE8_MEMBER(playch10_state::gboard_rom_switch_w)
{
	/* basically, a MMC3 mapper from the nes */

	switch (offset & 0x6001)
	{
		case 0x0000:
			m_gboard_command = data;

			if (m_gboard_last_bank != (data & 0xc0))
			{
				int bank;
				UINT8 *prg = memregion("cart")->base();

				/* reset the banks */
				if (m_gboard_command & 0x40)
				{
					/* high bank */
					bank = m_gboard_banks[0] * 0x2000 + 0x10000;

					memcpy(&prg[0x0c000], &prg[bank], 0x2000);
					memcpy(&prg[0x08000], &prg[0x4c000], 0x2000);
				}
				else
				{
					/* low bank */
					bank = m_gboard_banks[0] * 0x2000 + 0x10000;

					memcpy(&prg[0x08000], &prg[bank], 0x2000);
					memcpy(&prg[0x0c000], &prg[0x4c000], 0x2000);
				}

				/* mid bank */
				bank = m_gboard_banks[1] * 0x2000 + 0x10000;
				memcpy(&prg[0x0a000], &prg[bank], 0x2000);

				m_gboard_last_bank = data & 0xc0;
			}
		break;

		case 0x0001:
			{
				UINT8 cmd = m_gboard_command & 0x07;
				int page = (m_gboard_command & 0x80) >> 5;
				int bank;

				switch (cmd)
				{
					case 0: /* char banking */
					case 1: /* char banking */
						data &= 0xfe;
						page ^= (cmd << 1);
						pc10_set_videorom_bank(page, 2, data, 1);
					break;

					case 2: /* char banking */
					case 3: /* char banking */
					case 4: /* char banking */
					case 5: /* char banking */
						page ^= cmd + 2;
						pc10_set_videorom_bank(page, 1, data, 1);
					break;

					case 6: /* program banking */
					{
						UINT8 *prg = memregion("cart")->base();
						if (m_gboard_command & 0x40)
						{
							/* high bank */
							m_gboard_banks[0] = data & 0x1f;
							bank = (m_gboard_banks[0]) * 0x2000 + 0x10000;

							memcpy(&prg[0x0c000], &prg[bank], 0x2000);
							memcpy(&prg[0x08000], &prg[0x4c000], 0x2000);
						}
						else
						{
							/* low bank */
							m_gboard_banks[0] = data & 0x1f;
							bank = (m_gboard_banks[0]) * 0x2000 + 0x10000;

							memcpy(&prg[0x08000], &prg[bank], 0x2000);
							memcpy(&prg[0x0c000], &prg[0x4c000], 0x2000);
						}
					}
					break;

					case 7: /* program banking */
						{
							/* mid bank */
							UINT8 *prg = memregion("cart")->base();
							m_gboard_banks[1] = data & 0x1f;
							bank = m_gboard_banks[1] * 0x2000 + 0x10000;

							memcpy(&prg[0x0a000], &prg[bank], 0x2000);
						}
					break;
				}
			}
		break;

		case 0x2000: /* mirroring */
			if (!m_gboard_4screen)
			{
				if (data & 0x40)
					pc10_set_mirroring(PPU_MIRROR_HIGH);
				else
					pc10_set_mirroring((data & 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			}
		break;

		case 0x2001: /* enable ram at $6000 */
			/* ignored - we always enable it */
		break;

		case 0x4000: /* scanline counter */
			m_IRQ_count_latch = data;
		break;

		case 0x4001: /* scanline latch */
			m_IRQ_count = 0;
		break;

		case 0x6000: /* disable irqs */
			m_IRQ_enable = 0;
		break;

		case 0x6001: /* enable irqs */
			m_IRQ_enable = 1;
		break;
	}
}

DRIVER_INIT_MEMBER(playch10_state,pcgboard)
{
	ppu2c0x_device *ppu = machine().device<ppu2c0x_device>("ppu");
	UINT8 *prg = memregion("cart")->base();
	m_vram = nullptr;

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x4c000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x4c000], 0x4000);

	/* MMC3 mapper at writes to $8000-$ffff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(playch10_state::gboard_rom_switch_w),this));

	/* extra ram at $6000-$7fff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_ram(0x6000, 0x7fff);

	m_gboard_banks[0] = 0x1e;
	m_gboard_banks[1] = 0x1f;
	m_gboard_scanline_counter = 0;
	m_gboard_scanline_latch = 0;
	m_gboard_4screen = 0;
	m_IRQ_enable = 0;
	m_IRQ_count = m_IRQ_count_latch = 0;

	/* common init */
	DRIVER_INIT_CALL(playch10);

	ppu->set_scanline_callback(ppu2c0x_scanline_delegate(FUNC(playch10_state::gboard_scanline_cb),this));
}

DRIVER_INIT_MEMBER(playch10_state,pcgboard_type2)
{
	m_vram = nullptr;
	/* common init */
	DRIVER_INIT_CALL(pcgboard);

	/* enable 4 screen mirror */
	m_gboard_4screen = 1;
}

/**********************************************************************************/
/* i Board games (Captain Sky Hawk, Solar Jetman) */

WRITE8_MEMBER(playch10_state::iboard_rom_switch_w)
{
	int bank = data & 7;
	UINT8 *prg = memregion("cart")->base();

	if (data & 0x10)
		pc10_set_mirroring(PPU_MIRROR_HIGH);
	else
		pc10_set_mirroring(PPU_MIRROR_LOW);

	memcpy(&prg[0x08000], &prg[bank * 0x8000 + 0x10000], 0x8000);
}

DRIVER_INIT_MEMBER(playch10_state,pciboard)
{
	UINT8 *prg = memregion("cart")->base();

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x10000], 0x8000);

	/* Roms are banked at $8000 to $bfff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(playch10_state::iboard_rom_switch_w),this));

	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* allocate vram */
	m_vram = std::make_unique<UINT8[]>(0x2000);
	/* special init */
	set_videoram_bank(0, 8, 0, 8);
}

/**********************************************************************************/
/* H Board games (PinBot) */

WRITE8_MEMBER(playch10_state::hboard_rom_switch_w)
{
	switch (offset & 0x7001)
	{
		case 0x0001:
			{
				UINT8 cmd = m_gboard_command & 0x07;
				int page = (m_gboard_command & 0x80) >> 5;

				switch (cmd)
				{
					case 0: /* char banking */
					case 1: /* char banking */
						data &= 0xfe;
						page ^= (cmd << 1);
						if (data & 0x40)
						{
							set_videoram_bank(page, 2, data, 1);
						}
						else
						{
							pc10_set_videorom_bank(page, 2, data, 1);
						}
					return;

					case 2: /* char banking */
					case 3: /* char banking */
					case 4: /* char banking */
					case 5: /* char banking */
						page ^= cmd + 2;
						if (data & 0x40)
						{
							set_videoram_bank(page, 1, data, 1);
						}
						else
						{
							pc10_set_videorom_bank(page, 1, data, 1);
						}
					return;
				}
			}
	};
	gboard_rom_switch_w(space,offset,data);
}


DRIVER_INIT_MEMBER(playch10_state,pchboard)
{
	UINT8 *prg = memregion("cart")->base();
	memcpy(&prg[0x08000], &prg[0x4c000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x4c000], 0x4000);

	/* Roms are banked at $8000 to $bfff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(playch10_state::hboard_rom_switch_w),this));

	/* extra ram at $6000-$7fff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_ram(0x6000, 0x7fff);

	m_gboard_banks[0] = 0x1e;
	m_gboard_banks[1] = 0x1f;
	m_gboard_scanline_counter = 0;
	m_gboard_scanline_latch = 0;
	m_gboard_last_bank = 0xff;
	m_gboard_command = 0;

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/**********************************************************************************/
/* K Board games (Mario Open Golf) */

DRIVER_INIT_MEMBER(playch10_state,pckboard)
{
	UINT8 *prg = memregion("cart")->base();

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x48000], 0x8000);

	m_mmc1_rom_mask = 0x0f;

	/* extra ram at $6000-$7fff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_ram(0x6000, 0x7fff);

	/* Roms are banked at $8000 to $bfff */
	machine().device("cart")->memory().space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(playch10_state::mmc1_rom_switch_w),this));

	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* allocate vram */
	m_vram = std::make_unique<UINT8[]>(0x2000);
	/* special init */
	set_videoram_bank(0, 8, 0, 8);
}
