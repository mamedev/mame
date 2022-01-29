// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Fabio Priuli
/***************************************************************************

Nintendo VS UniSystem and DualSystem - (c) 1984 Nintendo of America

    Portions of this code are heavily based on
    Brad Oliver's MESS implementation of the NES.

***************************************************************************/

#include "emu.h"
#include "screen.h"
#include "video/ppu2c0x.h"
#include "includes/vsnes.h"


/* PPU notes */
/* nametable is, per Lord Nightmare, always 4K per PPU */
/* The vsnes system has relatively few banking options for CHR */
/* Each driver will use ROM or RAM for CHR, never both, and RAM is never banked */
/* This leads to the memory system being an optimal place to perform banking */

/* Prototypes for mapping board components to PPU bus */


/*************************************
 *
 *  Input Ports
 *
 *************************************/

void vsnes_state::vsnes_in0_w(uint8_t data)
{
	/* Toggling bit 0 high then low resets both controllers */
	if (data & 1)
	{
		/* load up the latches */
		m_input_latch[0] = ioport("IN0")->read();
		m_input_latch[1] = ioport("IN1")->read();
	}
}

uint8_t vsnes_state::gun_in0_r()
{
	int ret = (m_input_latch[0]) & 1;

	/* shift */
	m_input_latch[0] >>= 1;

	ret |= ioport("COINS")->read();             /* merge coins, etc */
	ret |= (ioport("DSW0")->read() & 3) << 3;       /* merge 2 dipswitches */

/* The gun games expect a 1 returned on every 5th read after sound_fix is reset*/
/* Info Supplied by Ben Parnell <xodnizel@home.com> of FCE Ultra fame */

	if (m_sound_fix == 4)
	{
		ret = 1;
	}

	m_sound_fix++;

	return ret;

}

uint8_t vsnes_state::vsnes_in0_r()
{
	int ret = (m_input_latch[0]) & 1;

	/* shift */
	m_input_latch[0] >>= 1;

	ret |= ioport("COINS")->read();             /* merge coins, etc */
	ret |= (ioport("DSW0")->read() & 3) << 3;       /* merge 2 dipswitches */

	return ret;

}

uint8_t vsnes_state::vsnes_in1_r()
{
	int ret = (m_input_latch[1]) & 1;

	ret |= ioport("DSW0")->read() & ~3;         /* merge the rest of the dipswitches */

	/* shift */
	m_input_latch[1] >>= 1;

	return ret;
}

void vsnes_state::vsnes_in0_1_w(uint8_t data)
{
	/* Toggling bit 0 high then low resets both controllers */
	if (data & 1)
	{
		/* load up the latches */
		m_input_latch[2] = ioport("IN2")->read();
		m_input_latch[3] = ioport("IN3")->read();
	}
}

uint8_t vsnes_state::vsnes_in0_1_r()
{
	int ret = (m_input_latch[2]) & 1;

	/* shift */
	m_input_latch[2] >>= 1;

	ret |= ioport("COINS2")->read();                /* merge coins, etc */
	ret |= (ioport("DSW1")->read() & 3) << 3;       /* merge 2 dipswitches */
	return ret;
}

uint8_t vsnes_state::vsnes_in1_1_r()
{
	int ret = (m_input_latch[3]) & 1;

	ret |= ioport("DSW1")->read() & ~3;         /* merge the rest of the dipswitches */

	/* shift */
	m_input_latch[3] >>= 1;

	return ret;

}

/*************************************
 *
 *  Init machine
 *
 *************************************/

MACHINE_RESET_MEMBER(vsnes_state,vsnes)
{
	m_last_bank = 0xff;
	m_sound_fix = 0;
	m_input_latch[0] = m_input_latch[1] = 0;
	m_input_latch[2] = m_input_latch[3] = 0;

}

/*************************************
 *
 *  Init machine
 *
 *************************************/

MACHINE_RESET_MEMBER(vsnes_state,vsdual)
{
	m_input_latch[0] = m_input_latch[1] = 0;
	m_input_latch[2] = m_input_latch[3] = 0;

}

/*************************************
 *
 *  Machine start functions
 *
 *************************************/

void vsnes_state::v_set_videorom_bank(  int start, int count, int vrom_start_bank )
{
	int i;

	assert(start + count <= 8);

	vrom_start_bank &= (m_vrom_banks - 1);
	assert(vrom_start_bank + count <= m_vrom_banks);

	/* bank_size_in_kb is used to determine how large the "bank" parameter is */
	/* count determines the size of the area mapped */
	for (i = 0; i < count; i++)
	{
		m_chr_banks[i + start]->set_entry(vrom_start_bank + i);
	}
}

MACHINE_START_MEMBER(vsnes_state,vsnes)
{
	address_space &ppu1_space = m_ppu1->space(AS_PROGRAM);
	int i;

	/* establish nametable ram */
	m_nt_ram[0] = std::make_unique<uint8_t[]>(0x1000);
	/* set mirroring */
	m_nt_page[0][0] = m_nt_ram[0].get();
	m_nt_page[0][1] = m_nt_ram[0].get() + 0x400;
	m_nt_page[0][2] = m_nt_ram[0].get() + 0x800;
	m_nt_page[0][3] = m_nt_ram[0].get() + 0xc00;

	ppu1_space.install_readwrite_handler(0x2000, 0x3eff, read8sm_delegate(*this, FUNC(vsnes_state::vsnes_nt0_r)), write8sm_delegate(*this, FUNC(vsnes_state::vsnes_nt0_w)));

	if (m_gfx1_rom != nullptr)
	{
		m_vrom[0] = memregion("gfx1")->base();
		m_vrom_size[0] = memregion("gfx1")->bytes();
		m_vrom_banks = m_vrom_size[0] / 0x400;
	}
	else
	{
		m_vrom[0] = nullptr;
		m_vrom_size[0] = 0;
		m_vrom_banks = 0;
	}

	/* establish chr banks */
	/* bank 1 is used already! */
	/* DRIVER_INIT is called first - means we can handle this different for VRAM games! */
	if (m_vrom[0] != nullptr)
	{
		for (i = 0; i < 8; i++)
		{
			ppu1_space.install_read_bank(0x0400 * i, 0x0400 * i + 0x03ff, m_chr_banks[i]);
			m_chr_banks[i]->configure_entries(0, m_vrom_banks, m_vrom[0], 0x400);
		}
		v_set_videorom_bank(0, 8, 0);
	}
	else
	{
		ppu1_space.install_ram(0x0000, 0x1fff, m_vram.get());
	}
}

MACHINE_START_MEMBER(vsnes_state,vsdual)
{
	m_vrom[0] = memregion("gfx1")->base();
	m_vrom[1] = memregion("gfx2")->base();
	m_vrom_size[0] = memregion("gfx1")->bytes();
	m_vrom_size[1] = memregion("gfx2")->bytes();

	/* establish nametable ram */
	m_nt_ram[0] = std::make_unique<uint8_t[]>(0x1000);
	m_nt_ram[1] = std::make_unique<uint8_t[]>(0x1000);
	/* set mirroring */
	m_nt_page[0][0] = m_nt_ram[0].get();
	m_nt_page[0][1] = m_nt_ram[0].get() + 0x400;
	m_nt_page[0][2] = m_nt_ram[0].get() + 0x800;
	m_nt_page[0][3] = m_nt_ram[0].get() + 0xc00;
	m_nt_page[1][0] = m_nt_ram[1].get();
	m_nt_page[1][1] = m_nt_ram[1].get() + 0x400;
	m_nt_page[1][2] = m_nt_ram[1].get() + 0x800;
	m_nt_page[1][3] = m_nt_ram[1].get() + 0xc00;

	m_ppu1->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8sm_delegate(*this, FUNC(vsnes_state::vsnes_nt0_r)), write8sm_delegate(*this, FUNC(vsnes_state::vsnes_nt0_w)));
	m_ppu2->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8sm_delegate(*this, FUNC(vsnes_state::vsnes_nt1_r)), write8sm_delegate(*this, FUNC(vsnes_state::vsnes_nt1_w)));
	// read only!
	m_ppu1->space(AS_PROGRAM).install_read_bank(0x0000, 0x1fff, m_bank_vrom[0]);
	// read only!
	m_ppu2->space(AS_PROGRAM).install_read_bank(0x0000, 0x1fff, m_bank_vrom[1]);
	m_bank_vrom[0]->configure_entries(0, m_vrom_size[0] / 0x2000, m_vrom[0], 0x2000);
	m_bank_vrom[1]->configure_entries(0, m_vrom_size[1] / 0x2000, m_vrom[1], 0x2000);
	m_bank_vrom[0]->set_entry(0);
	m_bank_vrom[1]->set_entry(0);
}

MACHINE_START_MEMBER(vsnes_state, bootleg)
{
	address_space &ppu1_space = m_ppu1->space(AS_PROGRAM);

	/* establish nametable ram */
	m_nt_ram[0] = std::make_unique<uint8_t[]>(0x1000);
	/* set mirroring */
	v_set_mirroring(0, PPU_MIRROR_VERT);

	ppu1_space.install_readwrite_handler(0x2000, 0x3eff, read8sm_delegate(*this, FUNC(vsnes_state::vsnes_nt0_r)), write8sm_delegate(*this, FUNC(vsnes_state::vsnes_nt0_w)));

	m_vrom[0] = m_gfx1_rom->base();
	m_vrom_size[0] = m_gfx1_rom->bytes();
	m_vrom_banks = m_vrom_size[0] / 0x2000;

	/* establish chr banks */
	m_ppu1->space(AS_PROGRAM).install_read_bank(0x0000, 0x1fff, m_bank_vrom[0]);
	m_bank_vrom[0]->configure_entries(0, m_vrom_banks, m_vrom[0], 0x2000);
	m_bank_vrom[0]->set_entry(0);
}

/*************************************
 *
 *  External mappings for PPU bus
 *
 *************************************/

void vsnes_state::vsnes_nt0_w(offs_t offset, uint8_t data)
{
	int page = ((offset & 0xc00) >> 10);
	m_nt_page[0][page][offset & 0x3ff] = data;
}

void vsnes_state::vsnes_nt1_w(offs_t offset, uint8_t data)
{
	int page = ((offset & 0xc00) >> 10);
	m_nt_page[1][page][offset & 0x3ff] = data;
}

uint8_t vsnes_state::vsnes_nt0_r(offs_t offset)
{
	int page = ((offset&0xc00) >> 10);
	return m_nt_page[0][page][offset & 0x3ff];
}

uint8_t vsnes_state::vsnes_nt1_r(offs_t offset)
{
	int page = ((offset & 0xc00) >> 10);
	return m_nt_page[1][page][offset & 0x3ff];
}

void vsnes_state::v_set_mirroring(int ppu, int mirroring)
{
	switch (mirroring)
	{
	case PPU_MIRROR_LOW:
		m_nt_page[ppu][0] = m_nt_page[ppu][1] = m_nt_page[ppu][2] = m_nt_page[ppu][3] = m_nt_ram[ppu].get();
		break;
	case PPU_MIRROR_HIGH:
		m_nt_page[ppu][0] = m_nt_page[ppu][1] = m_nt_page[ppu][2] = m_nt_page[ppu][3] = m_nt_ram[ppu].get() + 0x400;
		break;
	case PPU_MIRROR_HORZ:
		m_nt_page[ppu][0] = m_nt_ram[ppu].get();
		m_nt_page[ppu][1] = m_nt_ram[ppu].get();
		m_nt_page[ppu][2] = m_nt_ram[ppu].get() + 0x400;
		m_nt_page[ppu][3] = m_nt_ram[ppu].get() + 0x400;
		break;
	case PPU_MIRROR_VERT:
		m_nt_page[ppu][0] = m_nt_ram[ppu].get();
		m_nt_page[ppu][1] = m_nt_ram[ppu].get() + 0x400;
		m_nt_page[ppu][2] = m_nt_ram[ppu].get();
		m_nt_page[ppu][3] = m_nt_ram[ppu].get() + 0x400;
		break;
	case PPU_MIRROR_NONE:
	default:
		m_nt_page[ppu][0] = m_nt_ram[ppu].get();
		m_nt_page[ppu][1] = m_nt_ram[ppu].get() + 0x400;
		m_nt_page[ppu][2] = m_nt_ram[ppu].get() + 0x800;
		m_nt_page[ppu][3] = m_nt_ram[ppu].get() + 0xc00;
		break;
	}

}

/**********************************************************************************
 *
 *  Game and Board-specific initialization
 *
 **********************************************************************************/

/**********************************************************************************/
/* Most games: VROM Banking in controller 0 write */

void vsnes_state::vsnormal_vrom_banking(uint8_t data)
{
	/* switch vrom */
	v_set_videorom_bank(0, 8, (data & 4) ? 8 : 0);

	/* bit 1 ( data & 2 ) enables writes to extra ram, we ignore it */

	/* move along */
	vsnes_in0_w(data);
}

void vsnes_state::init_vsnormal()
{
	/* vrom switching is enabled with bit 2 of $4016 */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsnormal_vrom_banking)));
}

/**********************************************************************************/
/* Gun games: VROM Banking in controller 0 write */

void vsnes_state::gun_in0_w(uint8_t data)
{
	if (m_do_vrom_bank)
	{
		/* switch vrom */
		v_set_videorom_bank(0, 8, (data & 4) ? 8 : 0);
	}

	/* here we do things a little different */
	if (data & 1)
	{
		/* load up the latches */
		m_input_latch[0] = ioport("IN0")->read();

		/* do the gun thing */
		int x = ioport("GUNX")->read();
		float y = ioport("GUNY")->read();

		y = y * 0.9375f; // scale 256 (our gun input range is 0 - 255) to 240 (screen visible area / bitmap we're using is 0 - 239)

		uint8_t realy = (int)y;

		// update the screen if necessary
		if (!m_ppu1->screen().vblank())
		{
			int vpos = m_ppu1->screen().vpos();
			int hpos = m_ppu1->screen().hpos();

			if (vpos > realy || (vpos == realy && hpos >= x))
				m_ppu1->screen().update_now();
		}

		/* get the pixel at the gun position */
		rgb_t col = m_ppu1->screen().pixel(x, realy);
		uint8_t bright = col.brightness();
		// todo, calculate how bright it is with pix.r * 0.3 + pix.g * 0.59 + pix.b * 0.11 ?
		// the mame calc above is uint8_t brightness() const { return (r() * 222 + g() * 707 + b() * 71) / 1000; }  (from lib/util/palette.h)
#if 0
		uint8_t r = col.r();
		uint8_t g = col.g();
		uint8_t b = col.b();
		printf("pix is %02x %02x %02x | %02x\n", r,g,b,bright);
#endif
		if (bright == 0xff)
		{
			m_input_latch[0] |= 0x40;
		}



#if 0 // this is junk code, only works for NES palette..
		/* get the color base from the ppu */
		uint32_t color_base = 0;

		/* look at the screen and see if the cursor is over a bright pixel */
		if ((pix == color_base + 0x20 ) || (pix == color_base + 0x30) ||
			(pix == color_base + 0x33 ) || (pix == color_base + 0x34))
		{
			m_input_latch[0] |= 0x40;
		}
#endif

		m_input_latch[1] = ioport("IN1")->read();
	}

	if ((m_zapstore & 1) && (!(data & 1)))
	/* reset sound_fix to keep sound from hanging */
	{
		m_sound_fix = 0;
	}

	m_zapstore = data;
}

void vsnes_state::init_vsgun()
{
	/* VROM switching is enabled with bit 2 of $4016 */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x4016, 0x4016, read8smo_delegate(*this, FUNC(vsnes_state::gun_in0_r)), write8smo_delegate(*this, FUNC(vsnes_state::gun_in0_w)));
	m_do_vrom_bank = 1;
}

/**********************************************************************************/
/* Konami games: ROMs bankings at $8000-$ffff */

void vsnes_state::vskonami_rom_banking(offs_t offset, uint8_t data)
{
	int reg = (offset >> 12) & 0x07;
	int bankoffset = (data & 7) * 0x2000 + 0x10000;

	switch (reg)
	{
		case 0: /* code bank 0 */
		case 2: /* code bank 1 */
		case 4: /* code bank 2 */
		{
			uint8_t *prg = memregion("maincpu")->base();
			memcpy(&prg[0x08000 + reg * 0x1000], &prg[bankoffset], 0x2000);
		}
		break;

		case 6: /* vrom bank 0 */
			v_set_videorom_bank(0, 4, data * 4);
		break;

		case 7: /* vrom bank 1 */
			v_set_videorom_bank(4, 4, data * 4);
		break;
	}
}

void vsnes_state::init_vskonami()
{
	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	uint8_t *prg = memregion("maincpu")->base();
	memcpy(&prg[0x08000], &prg[0x18000], 0x8000);

	/* banking is done with writes to the $8000-$ffff area */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vsnes_state::vskonami_rom_banking)));
}

/***********************************************************************/
/* Vs. Gumshoe */

void vsnes_state::vsgshoe_gun_in0_w(uint8_t data)
{
	int addr;
	if((data & 0x04) != m_old_bank)
	{
		uint8_t *prg = memregion("maincpu")->base();
		m_old_bank = data & 0x04;
		addr = m_old_bank ? 0x12000: 0x10000;
		memcpy(&prg[0x08000], &prg[addr], 0x2000);
	}

	gun_in0_w(data);
}

void vsnes_state::init_vsgshoe()
{
	/* set up the default bank */
	uint8_t *prg = memregion("maincpu")->base();
	memcpy (&prg[0x08000], &prg[0x12000], 0x2000);

	/* vrom switching is enabled with bit 2 of $4016 */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x4016, 0x4016, read8smo_delegate(*this, FUNC(vsnes_state::gun_in0_r)), write8smo_delegate(*this, FUNC(vsnes_state::vsgshoe_gun_in0_w)));

	m_do_vrom_bank = 1;
}

/**********************************************************************************/
/* Dr Mario: ROMs bankings at $8000-$ffff */


void vsnes_state::drmario_rom_banking(offs_t offset, uint8_t data)
{
	/* basically, a MMC1 mapper from the nes */

	int reg = (offset >> 13);

	/* reset mapper */
	if (data & 0x80)
	{
		m_drmario_shiftreg = m_drmario_shiftcount = 0;

		m_size16k = 1;

		m_switchlow = 1;
		m_vrom4k = 0;

		return;
	}

	/* see if we need to clock in data */
	if (m_drmario_shiftcount < 5)
	{
		m_drmario_shiftreg >>= 1;
		m_drmario_shiftreg |= (data & 1) << 4;
		m_drmario_shiftcount++;
	}

	/* are we done shifting? */
	if (m_drmario_shiftcount == 5)
	{
		/* reset count */
		m_drmario_shiftcount = 0;

		/* apply data to registers */
		switch (reg)
		{
			case 0:     /* mirroring and options */
				{
					int mirroring;

					m_vrom4k = m_drmario_shiftreg & 0x10;
					m_size16k = m_drmario_shiftreg & 0x08;
					m_switchlow = m_drmario_shiftreg & 0x04;

					switch (m_drmario_shiftreg & 3)
					{
						case 0:
							mirroring = PPU_MIRROR_LOW;
						break;

						case 1:
							mirroring = PPU_MIRROR_HIGH;
						break;

						case 2:
							mirroring = PPU_MIRROR_VERT;
						break;

						default:
						case 3:
							mirroring = PPU_MIRROR_HORZ;
						break;
					}

					/* apply mirroring */
					v_set_mirroring(1, mirroring);
				}
			break;

			case 1: /* video rom banking - bank 0 - 4k or 8k */
				if (!m_vram)
					v_set_videorom_bank(0, (m_vrom4k) ? 4 : 8, m_drmario_shiftreg * 4);
			break;

			case 2: /* video rom banking - bank 1 - 4k only */
				if (m_vrom4k && !m_vram)
					v_set_videorom_bank(4, 4, m_drmario_shiftreg * 4);
			break;

			case 3: /* program banking */
				{
					int bank = (m_drmario_shiftreg & 0x03) * 0x4000;
					uint8_t *prg = memregion("maincpu")->base();

					if (!m_size16k)
					{
						/* switch 32k */
						memcpy(&prg[0x08000], &prg[0x010000 + bank], 0x8000);
					}
					else
					{
						/* switch 16k */
						if (m_switchlow)
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

		m_drmario_shiftreg = 0;
	}
}

void vsnes_state::init_drmario()
{
	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	uint8_t *prg = memregion("maincpu")->base();
	memcpy(&prg[0x08000], &prg[0x10000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x1c000], 0x4000);

	/* MMC1 mapper at writes to $8000-$ffff */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vsnes_state::drmario_rom_banking)));

	m_drmario_shiftreg = 0;
	m_drmario_shiftcount = 0;
}

/**********************************************************************************/
/* Games with VRAM instead of graphics ROMs: ROMs bankings at $8000-$ffff */

void vsnes_state::vsvram_rom_banking(uint8_t data)
{
	int rombank = 0x10000 + (data & 7) * 0x4000;
	uint8_t *prg = memregion("maincpu")->base();

	memcpy(&prg[0x08000], &prg[rombank], 0x4000);
}

void vsnes_state::init_vsvram()
{
	/* when starting the game, the 1st 16k and the last 16k are loaded into the 2 banks */
	uint8_t *prg = memregion("maincpu")->base();
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	/* banking is done with writes to the $8000-$ffff area */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8smo_delegate(*this, FUNC(vsnes_state::vsvram_rom_banking)));

	/* allocate m_vram */
	m_vram = std::make_unique<uint8_t[]>(0x2000);
}

/**********************************************************************************/


void vsnes_state::vs108_rom_banking(offs_t offset, uint8_t data)
{
	switch (offset & 0x6001)
	{
		case 0x0000: // $8000
			m_108_reg = data & 0x07;
			break;

		case 0x0001: // $8001
			switch (m_108_reg)
			{
				case 0: case 1:
					v_set_videorom_bank(m_108_reg * 2, 2, data);
					break;
				case 2: case 3: case 4: case 5:
					v_set_videorom_bank(m_108_reg + 2, 1, data);
					break;
				case 6: case 7:
					{
						uint8_t *prg = memregion("maincpu")->base();
						int addr = m_108_reg == 6 ? 0x8000 : 0xa000;
						memcpy(&prg[addr], &prg[0x2000 * (data & m_108_prg_mask) + 0x10000], 0x2000);
					}
					break;
			}
			break;

		default:
			logerror("vs108_rom_banking uncaught: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

// Common init for (Namco) 108 games

void vsnes_state::init_vs108()
{
	uint8_t *prg = memregion("maincpu")->base();
	int prg_chunks = (memregion("maincpu")->bytes() - 0x10000) / 0x4000;

	m_108_reg = 0;
	m_108_prg_mask = (prg_chunks << 1) - 1;

	memcpy(&prg[0x8000], &prg[(prg_chunks - 1) * 0x4000 + 0x10000], 0x2000);
	memcpy(&prg[0xa000], &prg[(prg_chunks - 1) * 0x4000 + 0x12000], 0x2000);
	memcpy(&prg[0xc000], &prg[(prg_chunks - 1) * 0x4000 + 0x10000], 0x2000);
	memcpy(&prg[0xe000], &prg[(prg_chunks - 1) * 0x4000 + 0x12000], 0x2000);

	// 108 chip at $8000-$9fff
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vsnes_state::vs108_rom_banking)));

	// extra ram at $6000-$7fff
	m_extraram = std::make_unique<uint8_t[]>(0x2000);
	save_pointer(NAME(m_extraram.get()), 0x2000);
	m_maincpu->space(AS_PROGRAM).install_ram(0x6000, 0x7fff, m_extraram.get());
}

/* Vs. RBI Baseball */

uint8_t vsnes_state::rbi_hack_r(offs_t offset)
{
	/* Supplied by Ben Parnell <xodnizel@home.com> of FCE Ultra fame */


	if (offset == 0)
	{
		m_VSindex=0;
		return 0xFF;

	}
	else
	{
		switch(m_VSindex++)
		{
			case 9:
				return 0x6F;

			case 14:
				return 0x94;

			default:
				return 0xB4;
		}
	}
}

void vsnes_state::init_rbibb()
{
	init_vs108();

	/* RBI Base ball hack */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5e00, 0x5e01, read8sm_delegate(*this, FUNC(vsnes_state::rbi_hack_r)));
}

/* Vs. Super Xevious */


uint8_t vsnes_state::supxevs_read_prot_1_r()
{
	return 0x05;
}

uint8_t vsnes_state::supxevs_read_prot_2_r()
{
	if (m_supxevs_prot_index)
		return 0;
	else
		return 0x01;
}

uint8_t vsnes_state::supxevs_read_prot_3_r()
{
	if (m_supxevs_prot_index)
		return 0xd1;
	else
		return 0x89;
}

uint8_t vsnes_state::supxevs_read_prot_4_r()
{
	if (m_supxevs_prot_index)
	{
		m_supxevs_prot_index = 0;
		return 0x3e;
	}
	else
	{
		m_supxevs_prot_index = 1;
		return 0x37;
	}
}


void vsnes_state::init_supxevs()
{
	init_vs108();

	/* Vs. Super Xevious Protection */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x54ff, 0x54ff, read8smo_delegate(*this, FUNC(vsnes_state::supxevs_read_prot_1_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5678, 0x5678, read8smo_delegate(*this, FUNC(vsnes_state::supxevs_read_prot_2_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x578f, 0x578f, read8smo_delegate(*this, FUNC(vsnes_state::supxevs_read_prot_3_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5567, 0x5567, read8smo_delegate(*this, FUNC(vsnes_state::supxevs_read_prot_4_r)));

	m_supxevs_prot_index = 0;
}

/* Vs. TKO Boxing */

uint8_t vsnes_state::tko_security_r(offs_t offset)
{
	static const uint8_t security_data[] = {
		0xff, 0xbf, 0xb7, 0x97, 0x97, 0x17, 0x57, 0x4f,
		0x6f, 0x6b, 0xeb, 0xa9, 0xb1, 0x90, 0x94, 0x14,
		0x56, 0x4e, 0x6f, 0x6b, 0xeb, 0xa9, 0xb1, 0x90,
		0xd4, 0x5c, 0x3e, 0x26, 0x87, 0x83, 0x13, 0x00
	};

	if (offset == 0)
	{
		m_security_counter = 0;
		return 0;
	}

	return security_data[(m_security_counter++)];

}

void vsnes_state::init_tkoboxng()
{
	init_vs108();

	/* security device at $5e00-$5e01 */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5e00, 0x5e01, read8sm_delegate(*this, FUNC(vsnes_state::tko_security_r)));
}

/* Vs. Freedom Force */

void vsnes_state::init_vsfdf()
{
	init_vs108();

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x4016, 0x4016, read8smo_delegate(*this, FUNC(vsnes_state::gun_in0_r)), write8smo_delegate(*this, FUNC(vsnes_state::gun_in0_w)));

	m_do_vrom_bank = 0;
}

/**********************************************************************************/
/* Platoon rom banking */

void vsnes_state::mapper68_rom_banking(offs_t offset, uint8_t data)
{
	switch (offset & 0x7000)
	{
		case 0x0000:
		v_set_videorom_bank(0, 2, data * 2);

		break;
		case 0x1000:
		v_set_videorom_bank(2, 2, data * 2);

		break;
		case 0x2000:
		v_set_videorom_bank(4, 2, data * 2);

		break;
		case 0x3000: /* ok? */
		v_set_videorom_bank(6, 2, data * 2);

		break;

		case 0x7000:
		{
			uint8_t *prg = memregion("maincpu")->base();
			memcpy(&prg[0x08000], &prg[0x10000 + data * 0x4000], 0x4000);
		}
		break;

	}

}

void vsnes_state::init_platoon()
{
	/* when starting a mapper 68 game  the first 16K ROM bank in the cart is loaded into $8000
	the LAST 16K ROM bank is loaded into $C000. The last 16K of ROM cannot be swapped. */

	uint8_t *prg = memregion("maincpu")->base();
	memcpy(&prg[0x08000], &prg[0x10000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x2c000], 0x4000);

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vsnes_state::mapper68_rom_banking)));
}

/**********************************************************************************/
/* Vs. Raid on Bungeling Bay (Japan) */

void vsnes_state::set_bnglngby_irq_w(uint8_t data)
{
	m_ret = data;
	m_maincpu->set_input_line(0, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
	/* other values ??? */
	/* 0, 4, 84 */
}

uint8_t vsnes_state::set_bnglngby_irq_r()
{
	return m_ret;
}

void vsnes_state::init_bnglngby()
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0231, 0x0231, read8smo_delegate(*this, FUNC(vsnes_state::set_bnglngby_irq_r)), write8smo_delegate(*this, FUNC(vsnes_state::set_bnglngby_irq_w)));

	/* extra ram */
	m_extraram = std::make_unique<uint8_t[]>(0x2000);
	save_pointer(NAME(m_extraram.get()), 0x2000);
	m_maincpu->space(AS_PROGRAM).install_ram(0x6000, 0x7fff, m_extraram.get());

	m_ret = 0;

	/* normal banking */
	init_vsnormal();
}

/**********************************************************************************/
/* VS Dualsystem */

void vsnes_state::vsdual_vrom_banking_main(uint8_t data)
{
	/* switch vrom */
	m_bank_vrom[0]->set_entry(BIT(data, 2));

	/* bit 1 ( data & 2 ) triggers irq on the other cpu */
	m_subcpu->set_input_line(0, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	/* move along */
	vsnes_in0_w(data);
}

void vsnes_state::vsdual_vrom_banking_sub(uint8_t data)
{
	/* switch vrom */
	m_bank_vrom[1]->set_entry(BIT(data, 2));

	/* bit 1 ( data & 2 ) triggers irq on the other cpu */
	m_maincpu->set_input_line(0, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	/* move along */
	vsnes_in0_1_w(data);
}

void vsnes_state::init_vsdual()
{
	uint8_t *prg = memregion("maincpu")->base();

	/* vrom switching is enabled with bit 2 of $4016 */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsdual_vrom_banking_main)));
	m_subcpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsdual_vrom_banking_sub)));

	/* shared ram at $6000 */
	m_maincpu->space(AS_PROGRAM).install_ram(0x6000, 0x7fff, &prg[0x6000]);
	m_subcpu->space(AS_PROGRAM).install_ram(0x6000, 0x7fff, &prg[0x6000]);
}

/**********************************************************************************/
/* Vs. Super Mario Bros (Bootleg) */

void vsnes_state::vsnes_bootleg_scanline(int scanline, int vblank, int blanked)
{
	// Z80 IRQ is controlled by two factors:
	// - bit 6 of current (next) scanline number
	// - bit 6 of latched scanline number from Z80 reading $4000
	if (!(m_bootleg_latched_scanline & 0x40))
	{
		m_subcpu->set_input_line(INPUT_LINE_IRQ0, ((scanline + 1) & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	}
}

uint8_t vsnes_state::vsnes_bootleg_ppudata()
{
	// CPU always reads higher CHR ROM banks from $2007, PPU always reads lower ones
	m_bank_vrom[0]->set_entry(1);
	uint8_t data = m_ppu1->read(0x2007);
	m_bank_vrom[0]->set_entry(0);

	return data;
}

void vsnes_state::init_bootleg()
{
	m_bootleg_sound_offset = 0;
	m_bootleg_sound_data = 0;
	m_bootleg_latched_scanline = 0;

	m_ppu1->set_scanline_callback(*this, FUNC(vsnes_state::vsnes_bootleg_scanline));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2007, 0x2007, read8smo_delegate(*this, FUNC(vsnes_state::vsnes_bootleg_ppudata)));
}
