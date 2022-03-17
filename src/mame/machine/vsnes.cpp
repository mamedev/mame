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


/*************************************
 *
 *  Input Ports
 *
 *************************************/

void vsnes_state::vsnes_in0_w(uint8_t data)
{
	/* Toggling bit 0 high then low resets both controllers */
	if (m_input_strobe[0] & ~data & 1)
	{
		/* load up the latches */
		m_input_latch[0] = ioport("IN0")->read();
		m_input_latch[1] = ioport("IN1")->read();
	}

	m_input_strobe[0] = data;
}

uint8_t vsnes_state::vsnes_in0_r()
{
	if (m_input_strobe[0] & 1)
		m_input_latch[0] = ioport("IN0")->read();

	int ret = m_input_latch[0] & 1;
	m_input_latch[0] >>= 1;

	ret |= ioport("COINS")->read();             /* merge coins, etc */
	ret |= (ioport("DSW0")->read() & 3) << 3;       /* merge 2 dipswitches */

	return ret;
}

uint8_t vsnes_state::vsnes_in1_r()
{
	if (m_input_strobe[0] & 1)
		m_input_latch[1] = ioport("IN1")->read();

	int ret = m_input_latch[1] & 1;
	m_input_latch[1] >>= 1;

	ret |= ioport("DSW0")->read() & ~3;         /* merge the rest of the dipswitches */

	return ret;
}

void vsnes_state::vsnes_in0_1_w(uint8_t data)
{
	/* Toggling bit 0 high then low resets both controllers */
	if (m_input_strobe[1] & ~data & 1)
	{
		/* load up the latches */
		m_input_latch[2] = ioport("IN2")->read();
		m_input_latch[3] = ioport("IN3")->read();
	}

	m_input_strobe[1] = data;
}

uint8_t vsnes_state::vsnes_in0_1_r()
{
	if (m_input_strobe[1] & 1)
		m_input_latch[2] = ioport("IN2")->read();

	int ret = m_input_latch[2] & 1;
	m_input_latch[2] >>= 1;

	ret |= ioport("COINS2")->read();                /* merge coins, etc */
	ret |= (ioport("DSW1")->read() & 3) << 3;       /* merge 2 dipswitches */

	return ret;
}

uint8_t vsnes_state::vsnes_in1_1_r()
{
	if (m_input_strobe[1] & 1)
		m_input_latch[3] = ioport("IN3")->read();

	int ret = m_input_latch[3] & 1;
	m_input_latch[3] >>= 1;

	ret |= ioport("DSW1")->read() & ~3;         /* merge the rest of the dipswitches */

	return ret;
}

// all gun games except Freedom Force add VROM banking on top of this
void vsnes_state::gun_in0_w(u8 data)
{
	if (m_input_strobe[0] & ~data & 1)
	{
		// load up the latch
		m_input_latch[0] = ioport("IN0")->read();

		if (m_sensor->detect_light(ioport("GUNX")->read(), ioport("GUNY")->read()))
			m_input_latch[0] |= 0x40;
	}

	m_input_strobe[0] = data;
}

/*************************************
 *
 *  Init machine
 *
 *************************************/

MACHINE_RESET_MEMBER(vsnes_state,vsnes)
{
	m_input_latch[0] = m_input_latch[1] = 0;
	m_input_latch[2] = m_input_latch[3] = 0;
	m_input_strobe[0] = m_input_strobe[1] = 0;
}

/*************************************
 *
 *  Machine start functions
 *
 *************************************/

void vsnes_state::v_set_videorom_bank(int start, int count, int vrom_start_bank)
{
	assert(start + count <= 8);

	vrom_start_bank &= m_chr_chunks - 1;
	assert(vrom_start_bank + count <= m_chr_chunks);

	// count determines the size of the area mapped
	for (int i = 0; i < count; i++)
		m_chr_banks[i + start]->set_entry(vrom_start_bank + i);
}

MACHINE_START_MEMBER(vsnes_state, vsnes)
{
	// establish chr banks
	// DRIVER_INIT is called first - means we can handle this different for VRAM games!
	if (m_gfx1_rom != nullptr)
	{
		u8 *base = m_gfx1_rom->base();
		m_chr_chunks = m_gfx1_rom->bytes() / 0x400;

		for (int i = 0; i < 8; i++)
		{
			m_ppu1->space(AS_PROGRAM).install_read_bank(0x0400 * i, 0x0400 * i + 0x03ff, m_chr_banks[i]);
			m_chr_banks[i]->configure_entries(0, m_chr_chunks, base, 0x400);
		}
		v_set_videorom_bank(0, 8, 0);
	}
	else
		m_chr_view.select(0);
}

MACHINE_START_MEMBER(vsnes_state, vsdual)
{
	for (int i = 0; i < 2; i++)
	{
		const char *region = i ? "gfx2" : "gfx1";
		u8 *base = memregion(region)->base();
		int entries = memregion(region)->bytes() / 0x2000;
		m_chr_banks[i]->configure_entries(0, entries, base, 0x2000);
		m_chr_banks[i]->set_entry(0);
	}
}

MACHINE_START_MEMBER(vsnes_state, bootleg)
{
	u8 *base = m_gfx1_rom->base();
	int entries = m_gfx1_rom->bytes() / 0x2000;
	m_chr_banks[0]->configure_entries(0, entries, base, 0x2000);
	m_chr_banks[0]->set_entry(0);
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
// Gun games: VROM Banking in controller 0 write

void vsnes_state::vsnes_gun_in0_w(uint8_t data)
{
	// switch vrom
	v_set_videorom_bank(0, 8, (data & 4) ? 8 : 0);

	gun_in0_w(data);
}

void vsnes_state::init_vsgun()
{
	// VROM switching is enabled with bit 2 of $4016
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsnes_gun_in0_w)));
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
	if ((data & 0x04) != m_old_bank)
	{
		uint8_t *prg = memregion("maincpu")->base();
		m_old_bank = data & 0x04;
		int addr = m_old_bank ? 0x12000: 0x10000;
		memcpy(&prg[0x08000], &prg[addr], 0x2000);
	}

	vsnes_gun_in0_w(data);
}

void vsnes_state::init_vsgshoe()
{
	/* set up the default bank */
	uint8_t *prg = memregion("maincpu")->base();
	memcpy (&prg[0x08000], &prg[0x12000], 0x2000);

	/* vrom switching is enabled with bit 2 of $4016 */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsgshoe_gun_in0_w)));
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
					m_vrom4k = m_drmario_shiftreg & 0x10;
					m_size16k = m_drmario_shiftreg & 0x08;
					m_switchlow = m_drmario_shiftreg & 0x04;
					// 0x03: mirroring bits unused on VS
				}
			break;

			case 1: /* video rom banking - bank 0 - 4k or 8k */
				v_set_videorom_bank(0, (m_vrom4k) ? 4 : 8, m_drmario_shiftreg * 4);
			break;

			case 2: /* video rom banking - bank 1 - 4k only */
				if (m_vrom4k)
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

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::gun_in0_w)));
}

/**********************************************************************************/
/* Platoon rom banking */

void vsnes_state::sunsoft3_rom_banking(offs_t offset, uint8_t data)
{
	switch (offset & 0x7800)
	{
		case 0x0800:
		case 0x1800:
		case 0x2800:
		case 0x3800:
			v_set_videorom_bank((offset >> 11) & 0x06, 2, data * 2);
		break;

		case 0x7800:
		{
			uint8_t *prg = memregion("maincpu")->base();
			memcpy(&prg[0x08000], &prg[0x10000 + data * 0x4000], 0x4000);
		}
		break;

	}

}

void vsnes_state::init_platoon()
{
	/* when starting a mapper 67 game the first 16K ROM bank in the cart is loaded into $8000
	the LAST 16K ROM bank is loaded into $C000. The last 16K of ROM cannot be swapped. */

	uint8_t *prg = memregion("maincpu")->base();
	memcpy(&prg[0x08000], &prg[0x10000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x2c000], 0x4000);

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vsnes_state::sunsoft3_rom_banking)));
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

	m_ret = 0;

	/* normal banking */
	init_vsnormal();
}

/**********************************************************************************/
/* VS Dualsystem */

void vsnes_state::vsdual_vrom_banking_main(uint8_t data)
{
	/* switch vrom */
	m_chr_banks[0]->set_entry(BIT(data, 2));

	/* bit 1 ( data & 2 ) triggers irq on the other cpu */
	m_subcpu->set_input_line(0, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	/* move along */
	vsnes_in0_w(data);
}

void vsnes_state::vsdual_vrom_banking_sub(uint8_t data)
{
	/* switch vrom */
	m_chr_banks[1]->set_entry(BIT(data, 2));

	/* bit 1 ( data & 2 ) triggers irq on the other cpu */
	m_maincpu->set_input_line(0, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	/* move along */
	vsnes_in0_1_w(data);
}

void vsnes_state::init_vsdual()
{
	/* vrom switching is enabled with bit 2 of $4016 */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsdual_vrom_banking_main)));
	m_subcpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsdual_vrom_banking_sub)));
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
	m_chr_banks[0]->set_entry(1);
	uint8_t data = m_ppu1->read(0x2007);
	m_chr_banks[0]->set_entry(0);

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
