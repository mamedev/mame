/***************************************************************************

Nintendo VS UniSystem and DualSystem - (c) 1984 Nintendo of America

    Portions of this code are heavily based on
    Brad Oliver's MESS implementation of the NES.

***************************************************************************/

#include "emu.h"
#include "video/ppu2c0x.h"
#include "includes/vsnes.h"


/* PPU notes */
/* nametable is, per Lord Nightmare, always 4K per PPU */
/* The vsnes system has relatively few banking options for CHR */
/* Each driver will use ROM or RAM for CHR, never both, and RAM is never banked */
/* This leads to the memory system being an optimal place to perform banking */

/* Prototypes for mapping board components to PPU bus */





static const char * const chr_banknames[] = { "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8", "bank9" };

/*************************************
 *
 *  Input Ports
 *
 *************************************/

WRITE8_MEMBER(vsnes_state::vsnes_in0_w)
{
	/* Toggling bit 0 high then low resets both controllers */
	if (data & 1)
	{
		/* load up the latches */
		m_input_latch[0] = input_port_read(machine(), "IN0");
		m_input_latch[1] = input_port_read(machine(), "IN1");
	}
}

READ8_MEMBER(vsnes_state::gun_in0_r)
{
	int ret = (m_input_latch[0]) & 1;

	/* shift */
	m_input_latch[0] >>= 1;

	ret |= input_port_read(machine(), "COINS");				/* merge coins, etc */
	ret |= (input_port_read(machine(), "DSW0") & 3) << 3;		/* merge 2 dipswitches */

/* The gun games expect a 1 returned on every 5th read after sound_fix is reset*/
/* Info Supplied by Ben Parnell <xodnizel@home.com> of FCE Ultra fame */

	if (m_sound_fix == 4)
	{
		ret = 1;
	}

	m_sound_fix++;

	return ret;

}

READ8_MEMBER(vsnes_state::vsnes_in0_r)
{

	int ret = (m_input_latch[0]) & 1;

	/* shift */
	m_input_latch[0] >>= 1;

	ret |= input_port_read(machine(), "COINS");				/* merge coins, etc */
	ret |= (input_port_read(machine(), "DSW0") & 3) << 3;		/* merge 2 dipswitches */

	return ret;

}

READ8_MEMBER(vsnes_state::vsnes_in1_r)
{
	int ret = (m_input_latch[1]) & 1;

	ret |= input_port_read(machine(), "DSW0") & ~3;			/* merge the rest of the dipswitches */

	/* shift */
	m_input_latch[1] >>= 1;

	return ret;
}

WRITE8_MEMBER(vsnes_state::vsnes_in0_1_w)
{
	/* Toggling bit 0 high then low resets both controllers */
	if (data & 1)
	{
		/* load up the latches */
		m_input_latch[2] = input_port_read(machine(), "IN2");
		m_input_latch[3] = input_port_read(machine(), "IN3");
	}
}

READ8_MEMBER(vsnes_state::vsnes_in0_1_r)
{
	int ret = (m_input_latch[2]) & 1;

	/* shift */
	m_input_latch[2] >>= 1;

	ret |= input_port_read(machine(), "COINS2");				/* merge coins, etc */
	ret |= (input_port_read(machine(), "DSW1") & 3) << 3;		/* merge 2 dipswitches */
	return ret;
}

READ8_MEMBER(vsnes_state::vsnes_in1_1_r)
{
	int ret = (m_input_latch[3]) & 1;

	ret |= input_port_read(machine(), "DSW1") & ~3;			/* merge the rest of the dipswitches */

	/* shift */
	m_input_latch[3] >>= 1;

	return ret;

}

/*************************************
 *
 *  Init machine
 *
 *************************************/

MACHINE_RESET( vsnes )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();

	state->m_last_bank = 0xff;
	state->m_sound_fix = 0;
	state->m_input_latch[0] = state->m_input_latch[1] = 0;
	state->m_input_latch[2] = state->m_input_latch[3] = 0;

}

/*************************************
 *
 *  Init machine
 *
 *************************************/

MACHINE_RESET( vsdual )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();

	state->m_input_latch[0] = state->m_input_latch[1] = 0;
	state->m_input_latch[2] = state->m_input_latch[3] = 0;

}

/*************************************
 *
 *  Machine start functions
 *
 *************************************/

static void v_set_videorom_bank( running_machine& machine, int start, int count, int vrom_start_bank )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	int i;

	assert(start + count <= 8);

	vrom_start_bank &= (state->m_vrom_banks - 1);
	assert(vrom_start_bank + count <= state->m_vrom_banks);

	/* bank_size_in_kb is used to determine how large the "bank" parameter is */
	/* count determines the size of the area mapped */
	for (i = 0; i < count; i++)
	{
		memory_set_bank(machine, chr_banknames[i + start], vrom_start_bank + i);
	}
}

MACHINE_START( vsnes )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	address_space *ppu1_space = machine.device("ppu1")->memory().space(AS_PROGRAM);
	int i;

	/* establish nametable ram */
	state->m_nt_ram[0] = auto_alloc_array(machine, UINT8, 0x1000);
	/* set mirroring */
	state->m_nt_page[0][0] = state->m_nt_ram[0];
	state->m_nt_page[0][1] = state->m_nt_ram[0] + 0x400;
	state->m_nt_page[0][2] = state->m_nt_ram[0] + 0x800;
	state->m_nt_page[0][3] = state->m_nt_ram[0] + 0xc00;

	ppu1_space->install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(vsnes_state::vsnes_nt0_r),state), write8_delegate(FUNC(vsnes_state::vsnes_nt0_w),state));

	state->m_vrom[0] = machine.region("gfx1")->base();
	state->m_vrom_size[0] = machine.region("gfx1")->bytes();
	state->m_vrom_banks = state->m_vrom_size[0] / 0x400;

	/* establish chr banks */
	/* bank 1 is used already! */
	/* DRIVER_INIT is called first - means we can handle this different for VRAM games! */
	if (NULL != state->m_vrom[0])
	{
		for (i = 0; i < 8; i++)
		{
			ppu1_space->install_read_bank(0x0400 * i, 0x0400 * i + 0x03ff, chr_banknames[i]);
			memory_configure_bank(machine, chr_banknames[i], 0, state->m_vrom_banks, state->m_vrom[0], 0x400);
		}
		v_set_videorom_bank(machine, 0, 8, 0);
	}
	else
	{
		ppu1_space->install_ram(0x0000, 0x1fff, state->m_vram);
	}
}

MACHINE_START( vsdual )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	state->m_vrom[0] = machine.region("gfx1")->base();
	state->m_vrom[1] = machine.region("gfx2")->base();
	state->m_vrom_size[0] = machine.region("gfx1")->bytes();
	state->m_vrom_size[1] = machine.region("gfx2")->bytes();

	/* establish nametable ram */
	state->m_nt_ram[0] = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_nt_ram[1] = auto_alloc_array(machine, UINT8, 0x1000);
	/* set mirroring */
	state->m_nt_page[0][0] = state->m_nt_ram[0];
	state->m_nt_page[0][1] = state->m_nt_ram[0] + 0x400;
	state->m_nt_page[0][2] = state->m_nt_ram[0] + 0x800;
	state->m_nt_page[0][3] = state->m_nt_ram[0] + 0xc00;
	state->m_nt_page[1][0] = state->m_nt_ram[1];
	state->m_nt_page[1][1] = state->m_nt_ram[1] + 0x400;
	state->m_nt_page[1][2] = state->m_nt_ram[1] + 0x800;
	state->m_nt_page[1][3] = state->m_nt_ram[1] + 0xc00;

	machine.device("ppu1")->memory().space(AS_PROGRAM)->install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(vsnes_state::vsnes_nt0_r),state), write8_delegate(FUNC(vsnes_state::vsnes_nt0_w),state));
	machine.device("ppu2")->memory().space(AS_PROGRAM)->install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(vsnes_state::vsnes_nt1_r),state), write8_delegate(FUNC(vsnes_state::vsnes_nt1_w),state));
	// read only!
	machine.device("ppu1")->memory().space(AS_PROGRAM)->install_read_bank(0x0000, 0x1fff, "bank2");
	// read only!
	machine.device("ppu2")->memory().space(AS_PROGRAM)->install_read_bank(0x0000, 0x1fff, "bank3");
	memory_configure_bank(machine, "bank2", 0, state->m_vrom_size[0] / 0x2000, state->m_vrom[0], 0x2000);
	memory_configure_bank(machine, "bank3", 0, state->m_vrom_size[1] / 0x2000, state->m_vrom[1], 0x2000);
	memory_set_bank(machine, "bank2", 0);
	memory_set_bank(machine, "bank3", 0);
}

/*************************************
 *
 *  External mappings for PPU bus
 *
 *************************************/

WRITE8_MEMBER(vsnes_state::vsnes_nt0_w)
{
	int page = ((offset & 0xc00) >> 10);
	m_nt_page[0][page][offset & 0x3ff] = data;
}

WRITE8_MEMBER(vsnes_state::vsnes_nt1_w)
{
	int page = ((offset & 0xc00) >> 10);
	m_nt_page[1][page][offset & 0x3ff] = data;
}

READ8_MEMBER(vsnes_state::vsnes_nt0_r)
{
	int page = ((offset&0xc00) >> 10);
	return m_nt_page[0][page][offset & 0x3ff];
}

READ8_MEMBER(vsnes_state::vsnes_nt1_r)
{
	int page = ((offset & 0xc00) >> 10);
	return m_nt_page[1][page][offset & 0x3ff];
}

void vsnes_state::v_set_mirroring(int ppu, int mirroring)
{
	switch (mirroring)
	{
	case PPU_MIRROR_LOW:
		m_nt_page[ppu][0] = m_nt_page[ppu][1] = m_nt_page[ppu][2] = m_nt_page[ppu][3] = m_nt_ram[ppu];
		break;
	case PPU_MIRROR_HIGH:
		m_nt_page[ppu][0] = m_nt_page[ppu][1] = m_nt_page[ppu][2] = m_nt_page[ppu][3] = m_nt_ram[ppu] + 0x400;
		break;
	case PPU_MIRROR_HORZ:
		m_nt_page[ppu][0] = m_nt_ram[ppu];
		m_nt_page[ppu][1] = m_nt_ram[ppu];
		m_nt_page[ppu][2] = m_nt_ram[ppu] + 0x400;
		m_nt_page[ppu][3] = m_nt_ram[ppu] + 0x400;
		break;
	case PPU_MIRROR_VERT:
		m_nt_page[ppu][0] = m_nt_ram[ppu];
		m_nt_page[ppu][1] = m_nt_ram[ppu] + 0x400;
		m_nt_page[ppu][2] = m_nt_ram[ppu];
		m_nt_page[ppu][3] = m_nt_ram[ppu] + 0x400;
		break;
	case PPU_MIRROR_NONE:
	default:
		m_nt_page[ppu][0] = m_nt_ram[ppu];
		m_nt_page[ppu][1] = m_nt_ram[ppu] + 0x400;
		m_nt_page[ppu][2] = m_nt_ram[ppu] + 0x800;
		m_nt_page[ppu][3] = m_nt_ram[ppu] + 0xc00;
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

WRITE8_MEMBER(vsnes_state::vsnormal_vrom_banking)
{
	/* switch vrom */
	v_set_videorom_bank(machine(), 0, 8, (data & 4) ? 8 : 0);

	/* bit 1 ( data & 2 ) enables writes to extra ram, we ignore it */

	/* move along */
	vsnes_in0_w(space, offset, data);
}

DRIVER_INIT( vsnormal )
{
	/* vrom switching is enabled with bit 2 of $4016 */
	vsnes_state *state = machine.driver_data<vsnes_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x4016, 0x4016, write8_delegate(FUNC(vsnes_state::vsnormal_vrom_banking),state));
}

/**********************************************************************************/
/* Gun games: VROM Banking in controller 0 write */

WRITE8_MEMBER(vsnes_state::gun_in0_w)
{
	ppu2c0x_device *ppu1 = machine().device<ppu2c0x_device>("ppu1");

	if (m_do_vrom_bank)
	{
		/* switch vrom */
		v_set_videorom_bank(machine(), 0, 8, (data & 4) ? 8 : 0);
	}

	/* here we do things a little different */
	if (data & 1)
	{

		/* load up the latches */
		m_input_latch[0] = input_port_read(machine(), "IN0");

		/* do the gun thing */
		int x = input_port_read(machine(), "GUNX");
		int y = input_port_read(machine(), "GUNY");
		UINT32 pix, color_base;

		/* get the pixel at the gun position */
		pix = ppu1->get_pixel(x, y);

		/* get the color base from the ppu */
		color_base = ppu1->get_colorbase();

		/* look at the screen and see if the cursor is over a bright pixel */
		if ((pix == color_base + 0x20 ) || (pix == color_base + 0x30) ||
			(pix == color_base + 0x33 ) || (pix == color_base + 0x34))
		{
			m_input_latch[0] |= 0x40;
		}

		m_input_latch[1] = input_port_read(machine(), "IN1");
	}

    if ((m_zapstore & 1) && (!(data & 1)))
	/* reset sound_fix to keep sound from hanging */
    {
		m_sound_fix = 0;
	}

    m_zapstore = data;
}

DRIVER_INIT( vsgun )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	/* VROM switching is enabled with bit 2 of $4016 */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0x4016, 0x4016, read8_delegate(FUNC(vsnes_state::gun_in0_r),state), write8_delegate(FUNC(vsnes_state::gun_in0_w),state));
	state->m_do_vrom_bank = 1;
}

/**********************************************************************************/
/* Konami games: ROMs bankings at $8000-$ffff */

WRITE8_MEMBER(vsnes_state::vskonami_rom_banking)
{
	int reg = (offset >> 12) & 0x07;
	int bankoffset = (data & 7) * 0x2000 + 0x10000;

	switch (reg)
	{
		case 0: /* code bank 0 */
		case 2: /* code bank 1 */
		case 4: /* code bank 2 */
		{
			UINT8 *prg = machine().region("maincpu")->base();
			memcpy(&prg[0x08000 + reg * 0x1000], &prg[bankoffset], 0x2000);
		}
		break;

		case 6: /* vrom bank 0 */
			v_set_videorom_bank(machine(), 0, 4, data * 4);
		break;

		case 7: /* vrom bank 1 */
			v_set_videorom_bank(machine(), 4, 4, data * 4);
		break;
	}
}

DRIVER_INIT( vskonami )
{
	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	UINT8 *prg = machine.region("maincpu")->base();
	memcpy(&prg[0x08000], &prg[0x18000], 0x8000);

	/* banking is done with writes to the $8000-$ffff area */
	vsnes_state *state = machine.driver_data<vsnes_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(vsnes_state::vskonami_rom_banking),state));
}

/***********************************************************************/
/* Vs. Gumshoe */

WRITE8_MEMBER(vsnes_state::vsgshoe_gun_in0_w)
{
	int addr;
	if((data & 0x04) != m_old_bank)
	{
		UINT8 *prg = machine().region("maincpu")->base();
		m_old_bank = data & 0x04;
		addr = m_old_bank ? 0x12000: 0x10000;
		memcpy(&prg[0x08000], &prg[addr], 0x2000);
	}

	gun_in0_w(space, offset, data);
}

DRIVER_INIT( vsgshoe )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	/* set up the default bank */
	UINT8 *prg = machine.region("maincpu")->base();
	memcpy (&prg[0x08000], &prg[0x12000], 0x2000);

	/* vrom switching is enabled with bit 2 of $4016 */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0x4016, 0x4016, read8_delegate(FUNC(vsnes_state::gun_in0_r),state), write8_delegate(FUNC(vsnes_state::vsgshoe_gun_in0_w),state));

	state->m_do_vrom_bank = 1;
}

/**********************************************************************************/
/* Dr Mario: ROMs bankings at $8000-$ffff */


WRITE8_MEMBER(vsnes_state::drmario_rom_banking)
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
			case 0:		/* mirroring and options */
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

			case 1:	/* video rom banking - bank 0 - 4k or 8k */
				if (!m_vram)
					v_set_videorom_bank(machine(), 0, (m_vrom4k) ? 4 : 8, m_drmario_shiftreg * 4);
			break;

			case 2: /* video rom banking - bank 1 - 4k only */
				if (m_vrom4k && !m_vram)
					v_set_videorom_bank(machine(), 4, 4, m_drmario_shiftreg * 4);
			break;

			case 3:	/* program banking */
				{
					int bank = (m_drmario_shiftreg & 0x03) * 0x4000;
					UINT8 *prg = machine().region("maincpu")->base();

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

DRIVER_INIT( drmario )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	UINT8 *prg = machine.region("maincpu")->base();
	memcpy(&prg[0x08000], &prg[0x10000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x1c000], 0x4000);

	/* MMC1 mapper at writes to $8000-$ffff */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(vsnes_state::drmario_rom_banking),state));

	state->m_drmario_shiftreg = 0;
	state->m_drmario_shiftcount = 0;
}

/**********************************************************************************/
/* Games with VRAM instead of graphics ROMs: ROMs bankings at $8000-$ffff */

WRITE8_MEMBER(vsnes_state::vsvram_rom_banking)
{
	int rombank = 0x10000 + (data & 7) * 0x4000;
	UINT8 *prg = machine().region("maincpu")->base();

	memcpy(&prg[0x08000], &prg[rombank], 0x4000);
}

DRIVER_INIT( vsvram )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	/* when starting the game, the 1st 16k and the last 16k are loaded into the 2 banks */
	UINT8 *prg = machine.region("maincpu")->base();
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	/* banking is done with writes to the $8000-$ffff area */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(vsnes_state::vsvram_rom_banking),state));

	/* allocate state->m_vram */
	state->m_vram = auto_alloc_array(machine, UINT8, 0x2000);
}

/**********************************************************************************/


static void mapper4_set_prg( running_machine &machine )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	UINT8 *prg = machine.region("maincpu")->base();
	UINT8 prg_flip = (state->m_MMC3_cmd & 0x40) ? 2 : 0;

	memcpy(&prg[0x8000], &prg[0x2000 * (state->m_MMC3_prg_bank[0 ^ prg_flip] & state->m_MMC3_prg_mask) + 0x10000], 0x2000);
	memcpy(&prg[0xa000], &prg[0x2000 * (state->m_MMC3_prg_bank[1] & state->m_MMC3_prg_mask) + 0x10000], 0x2000);
	memcpy(&prg[0xc000], &prg[0x2000 * (state->m_MMC3_prg_bank[2 ^ prg_flip] & state->m_MMC3_prg_mask) + 0x10000], 0x2000);
	memcpy(&prg[0xe000], &prg[0x2000 * (state->m_MMC3_prg_bank[3] & state->m_MMC3_prg_mask) + 0x10000], 0x2000);
}

static void mapper4_set_chr( running_machine &machine )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	UINT8 chr_page = (state->m_MMC3_cmd & 0x80) >> 5;

	v_set_videorom_bank(machine, chr_page ^ 0, 1, state->m_MMC3_chr_bank[0] & ~0x01);
	v_set_videorom_bank(machine, chr_page ^ 1, 1, state->m_MMC3_chr_bank[0] |  0x01);
	v_set_videorom_bank(machine, chr_page ^ 2, 1, state->m_MMC3_chr_bank[1] & ~0x01);
	v_set_videorom_bank(machine, chr_page ^ 3, 1, state->m_MMC3_chr_bank[1] |  0x01);
	v_set_videorom_bank(machine, chr_page ^ 4, 1, state->m_MMC3_chr_bank[2]);
	v_set_videorom_bank(machine, chr_page ^ 5, 1, state->m_MMC3_chr_bank[3]);
	v_set_videorom_bank(machine, chr_page ^ 6, 1, state->m_MMC3_chr_bank[4]);
	v_set_videorom_bank(machine, chr_page ^ 7, 1, state->m_MMC3_chr_bank[5]);
}

#define BOTTOM_VISIBLE_SCANLINE	239		/* The bottommost visible scanline */
#define NUM_SCANLINE 262

static void mapper4_irq( device_t *device, int scanline, int vblank, int blanked )
{
	vsnes_state *state = device->machine().driver_data<vsnes_state>();
	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		int priorCount = state->m_IRQ_count;
		if ((state->m_IRQ_count == 0))
		{
			state->m_IRQ_count = state->m_IRQ_count_latch;
		}
		else
			state->m_IRQ_count--;

		if (state->m_IRQ_enable && !blanked && (state->m_IRQ_count == 0) && priorCount)
		{
			cputag_set_input_line(device->machine(), "maincpu", 0, HOLD_LINE);
		}
	}
}

WRITE8_MEMBER(vsnes_state::mapper4_w)
{
	ppu2c0x_device *ppu1 = machine().device<ppu2c0x_device>("ppu1");
	UINT8 MMC3_helper, cmd;

	switch (offset & 0x6001)
	{
		case 0x0000: /* $8000 */
			MMC3_helper = m_MMC3_cmd ^ data;
			m_MMC3_cmd = data;

			/* Has PRG Mode changed? */
			if (MMC3_helper & 0x40)
				mapper4_set_prg(machine());

			/* Has CHR Mode changed? */
			if (MMC3_helper & 0x80)
				mapper4_set_chr(machine());
			break;

		case 0x0001: /* $8001 */
			cmd = m_MMC3_cmd & 0x07;
			switch (cmd)
			{
				case 0: case 1:	// these do not need to be separated: we take care of them in set_chr!
				case 2: case 3: case 4: case 5:
					m_MMC3_chr_bank[cmd] = data;
					mapper4_set_chr(machine());
					break;
				case 6:
				case 7:
					m_MMC3_prg_bank[cmd - 6] = data;
					mapper4_set_prg(machine());
					break;
			}
			break;

		case 0x2000: /* $a000 */
			if (data & 0x40)
				v_set_mirroring(1, PPU_MIRROR_HIGH);
			else
			{
				if (data & 0x01)
					v_set_mirroring(1, PPU_MIRROR_HORZ);
				else
					v_set_mirroring(1, PPU_MIRROR_VERT);
			}
			break;

		case 0x2001: /* $a001 - extra RAM enable/disable */
			/* ignored - we always enable it */

			break;
		case 0x4000: /* $c000 - IRQ scanline counter */
			m_IRQ_count = data;

			break;

		case 0x4001: /* $c001 - IRQ scanline latch */
			m_IRQ_count_latch = data;

			break;

		case 0x6000: /* $e000 - Disable IRQs */
			m_IRQ_enable = 0;
			m_IRQ_count = m_IRQ_count_latch;

			ppu1->set_scanline_callback(0);

			break;

		case 0x6001: /* $e001 - Enable IRQs */
			m_IRQ_enable = 1;
			ppu1->set_scanline_callback(mapper4_irq);

			break;

		default:
			logerror("mapper4_w uncaught: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

/* Common init for MMC3 games */

DRIVER_INIT( MMC3 )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	UINT8 *prg = machine.region("maincpu")->base();
	state->m_IRQ_enable = state->m_IRQ_count = state->m_IRQ_count_latch = 0;
	int MMC3_prg_chunks = (machine.region("maincpu")->bytes() - 0x10000) / 0x4000;

	state->m_MMC3_prg_bank[0] = state->m_MMC3_prg_bank[2] = 0xfe;
	state->m_MMC3_prg_bank[1] = state->m_MMC3_prg_bank[3] = 0xff;
	state->m_MMC3_cmd = 0;

	state->m_MMC3_prg_mask = ((MMC3_prg_chunks << 1) - 1);

	memcpy(&prg[0x8000], &prg[(MMC3_prg_chunks - 1) * 0x4000 + 0x10000], 0x2000);
	memcpy(&prg[0xa000], &prg[(MMC3_prg_chunks - 1) * 0x4000 + 0x12000], 0x2000);
	memcpy(&prg[0xc000], &prg[(MMC3_prg_chunks - 1) * 0x4000 + 0x10000], 0x2000);
	memcpy(&prg[0xe000], &prg[(MMC3_prg_chunks - 1) * 0x4000 + 0x12000], 0x2000);

	/* MMC3 mapper at writes to $8000-$ffff */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(vsnes_state::mapper4_w),state));

	/* extra ram at $6000-$7fff */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_ram(0x6000, 0x7fff);
}

/* Vs. RBI Baseball */

READ8_MEMBER(vsnes_state::rbi_hack_r)
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

DRIVER_INIT( rbibb )
{
	DRIVER_INIT_CALL(MMC3);

	/* RBI Base ball hack */
	vsnes_state *state = machine.driver_data<vsnes_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0x5e00, 0x5e01, read8_delegate(FUNC(vsnes_state::rbi_hack_r),state)) ;
}

/* Vs. Super Xevious */


READ8_MEMBER(vsnes_state::supxevs_read_prot_1_r)
{
	return 0x05;
}

READ8_MEMBER(vsnes_state::supxevs_read_prot_2_r)
{
	if (m_supxevs_prot_index)
		return 0;
	else
		return 0x01;
}

READ8_MEMBER(vsnes_state::supxevs_read_prot_3_r)
{
	if (m_supxevs_prot_index)
		return 0xd1;
	else
		return 0x89;
}

READ8_MEMBER(vsnes_state::supxevs_read_prot_4_r)
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


DRIVER_INIT( supxevs )
{
	DRIVER_INIT_CALL(MMC3);

	/* Vs. Super Xevious Protection */
	vsnes_state *state = machine.driver_data<vsnes_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0x54ff, 0x54ff, read8_delegate(FUNC(vsnes_state::supxevs_read_prot_1_r),state));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0x5678, 0x5678, read8_delegate(FUNC(vsnes_state::supxevs_read_prot_2_r),state));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0x578f, 0x578f, read8_delegate(FUNC(vsnes_state::supxevs_read_prot_3_r),state));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0x5567, 0x5567, read8_delegate(FUNC(vsnes_state::supxevs_read_prot_4_r),state));
}

/* Vs. TKO Boxing */

READ8_MEMBER(vsnes_state::tko_security_r)
{
	static const UINT8 security_data[] = {
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

DRIVER_INIT( tkoboxng )
{
	DRIVER_INIT_CALL(MMC3);

	/* security device at $5e00-$5e01 */
	vsnes_state *state = machine.driver_data<vsnes_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0x5e00, 0x5e01, read8_delegate(FUNC(vsnes_state::tko_security_r),state));
}

/* Vs. Freedom Force */

DRIVER_INIT( vsfdf )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	DRIVER_INIT_CALL(MMC3);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0x4016, 0x4016, read8_delegate(FUNC(vsnes_state::gun_in0_r),state), write8_delegate(FUNC(vsnes_state::gun_in0_w),state));

	state->m_do_vrom_bank = 0;
}

/**********************************************************************************/
/* Platoon rom banking */

WRITE8_MEMBER(vsnes_state::mapper68_rom_banking)
{
	switch (offset & 0x7000)
	{
		case 0x0000:
		v_set_videorom_bank(machine(), 0, 2, data * 2);

		break;
		case 0x1000:
		v_set_videorom_bank(machine(), 2, 2, data * 2);

		break;
		case 0x2000:
		v_set_videorom_bank(machine(), 4, 2, data * 2);

		break;
		case 0x3000: /* ok? */
		v_set_videorom_bank(machine(), 6, 2, data * 2);

		break;

		case 0x7000:
		{
			UINT8 *prg = machine().region("maincpu")->base();
			memcpy(&prg[0x08000], &prg[0x10000 + data * 0x4000], 0x4000);
		}
		break;

	}

}

DRIVER_INIT( platoon )
{

	/* when starting a mapper 68 game  the first 16K ROM bank in the cart is loaded into $8000
    the LAST 16K ROM bank is loaded into $C000. The last 16K of ROM cannot be swapped. */

	UINT8 *prg = machine.region("maincpu")->base();
	memcpy(&prg[0x08000], &prg[0x10000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x2c000], 0x4000);

	vsnes_state *state = machine.driver_data<vsnes_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(vsnes_state::mapper68_rom_banking),state));
}

/**********************************************************************************/
/* Vs. Raid on Bungeling Bay (Japan) */

WRITE8_MEMBER(vsnes_state::set_bnglngby_irq_w)
{
	m_ret = data;
	cputag_set_input_line(machine(), "maincpu", 0, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
	/* other values ??? */
	/* 0, 4, 84 */
}

READ8_MEMBER(vsnes_state::set_bnglngby_irq_r)
{
	return m_ret;
}

DRIVER_INIT( bnglngby )
{
	vsnes_state *state = machine.driver_data<vsnes_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0x0231, 0x0231, read8_delegate(FUNC(vsnes_state::set_bnglngby_irq_r),state), write8_delegate(FUNC(vsnes_state::set_bnglngby_irq_w),state));

	/* extra ram */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_ram(0x6000, 0x7fff);

	state->m_ret = 0;

	/* normal banking */
	DRIVER_INIT_CALL(vsnormal);
}

/**********************************************************************************/
/* VS Dualsystem */

WRITE8_MEMBER(vsnes_state::vsdual_vrom_banking)
{
	device_t *other_cpu = (&space.device() == machine().device("maincpu")) ? machine().device("sub") : machine().device("maincpu");
	/* switch vrom */
	(&space.device() == machine().device("maincpu")) ? memory_set_bank(machine(), "bank2", BIT(data, 2)) : memory_set_bank(machine(), "bank3", BIT(data, 2));

	/* bit 1 ( data & 2 ) triggers irq on the other cpu */
	device_set_input_line(other_cpu, 0, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	/* move along */
	if (&space.device() == machine().device("maincpu"))
		vsnes_in0_w(space, offset, data);
	else
		vsnes_in0_1_w(space, offset, data);
}

DRIVER_INIT( vsdual )
{
	UINT8 *prg = machine.region("maincpu")->base();

	vsnes_state *state = machine.driver_data<vsnes_state>();
	/* vrom switching is enabled with bit 2 of $4016 */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x4016, 0x4016, write8_delegate(FUNC(vsnes_state::vsdual_vrom_banking),state));
	machine.device("sub")->memory().space(AS_PROGRAM)->install_write_handler(0x4016, 0x4016, write8_delegate(FUNC(vsnes_state::vsdual_vrom_banking),state));

	/* shared ram at $6000 */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_ram(0x6000, 0x7fff, &prg[0x6000]);
	machine.device("sub")->memory().space(AS_PROGRAM)->install_ram(0x6000, 0x7fff, &prg[0x6000]);
}

