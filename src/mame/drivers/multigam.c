/*
    Multi Game

    driver by Mariusz Wojcieszek
    uses NES emulation by Brad Olivier

    Hardware is based around Nintendo NES. After coin up, it allows to play one of
    35 NES games (choosen from text menu). Manufacturer is unknown, code for title
    screen and game selection contains string:
    "Programed by SANG  HO  ENG. K. B. KIM 1986/1/ 1"

    Notes:
    - rom at 0x6000-0x7fff contains control program, which handles coins and timer
    - each game rom has NMI (VBLANK) vector changed to 0x6100 (control program). Then,
      game NMI is called through IRQ vector
    - there is additional RAM (0x800-0xfff) used by control program
    - ROM banked at 0x5000-0x5ffe seems to contain patches for games. At least
      Ninja code jumps into this area
    - game roms are banked in 16KB units. Bank switching of game rom is handled by
      writing to 0x6fff. If bank select has 7th bit turned on, then 32KB rom is
      switched into $8000 - $ffff. Otherwise, 16KB rom is switched on.
    - PPU VROMS are banked in 8KB units. Write to 0x7fff controls PPU VROM banking.
    - some games use additional banking on PPU VROMS. This is enabled by writing
      value with bit 7 on to 0x7fff and then selecting a vrom by writing into
      0x8000 - 0xffff range.
    - there is a register at 0x5002. 0xff is written there when game is played,
      0x00 otherwise. Its purpose is unknown.


    Notes by Roberto Fresca:

    - Added multigame (set2).
    - Main differences are:
         - Control program: Offset 0x79b4 = JMP $c4c4 was changed to JMP $c4c6.
         - ROM at 0x80000:  Several things patched and/or relocated.
                            Most original NOPs were replaced by proper code.

    Multi Game 2 & III: 21 games included, hardware features MMC3 NES mapper and additional
    RAM used by Super Mario Bros 3.

    Multi Game (Tung Sheng Electronics): 10 games included, selectable by dip switches.

    Super Game III:

PCB Layout
----------

|------------------------------------|
|UPC1242H 3.579545MHz  6264          |
|  4066        UPC1352 6116     ROM14|
|                               ROM15|
|J                              ROM16|
|A     UA6527  UA6528           ROM17|
|M                                   |
|M       PAL   DIP40*           IC36 |
|A             DIP24*           ROM18|
| 21.47727MHz                   ROM10|
|                               ROM11|
|                 PLSI1016      ROM12|
|DIPSW(8)  6264          6264   ROM13|
|------------------------------------|
* - chip surface scratched

    Super Game III features 15 games, updated hardware with several NES mappers allowing newer NES
    games to run.
*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/dac.h"
#include "sound/nes_apu.h"
#include "video/ppu2c0x.h"


class multigam_state : public driver_device
{
public:
	multigam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8* m_nt_ram;
	UINT8* m_vram;
	UINT8* m_nt_page[4];
	UINT32 m_in_0;
	UINT32 m_in_1;
	UINT32 m_in_0_shift;
	UINT32 m_in_1_shift;
	UINT32 m_in_dsw;
	UINT32 m_in_dsw_shift;
	int m_game_gfx_bank;
	int m_multigam3_mmc3_scanline_counter;
	int m_multigam3_mmc3_scanline_latch;
	int m_multigam3_mmc3_banks[2];
	int m_multigam3_mmc3_4screen;
	int m_multigam3_mmc3_last_bank;
	UINT8* m_multigmc_mmc3_6000_ram;
	UINT8* m_multigam3_mmc3_prg_base;
	int m_multigam3_mmc3_prg_size;
	int m_multigam3_mmc3_chr_bank_base;
	int m_multigam3_mmc3_command;
	UINT8* m_mapper02_prg_base;
	int m_mapper02_prg_size;
	int m_mmc1_shiftreg;
	int m_mmc1_shiftcount;
	int m_mmc1_rom_mask;
	UINT8* m_mmc1_prg_base;
	int m_mmc1_prg_size;
	int m_mmc1_chr_bank_base;
	int m_mmc1_reg_write_enable;
	int m_size16k;
	int m_switchlow;
	int m_vrom4k;
	UINT8 m_supergm3_prg_bank;
	UINT8 m_supergm3_chr_bank;
	DECLARE_CUSTOM_INPUT_MEMBER(multigam_inputs_r);
	DECLARE_WRITE8_MEMBER(multigam_nt_w);
	DECLARE_READ8_MEMBER(multigam_nt_r);
	DECLARE_WRITE8_MEMBER(sprite_dma_w);
	DECLARE_READ8_MEMBER(multigam_IN0_r);
	DECLARE_WRITE8_MEMBER(multigam_IN0_w);
	DECLARE_READ8_MEMBER(multigam_IN1_r);
	DECLARE_WRITE8_MEMBER(multigam_switch_prg_rom);
	DECLARE_WRITE8_MEMBER(multigam_switch_gfx_rom);
	DECLARE_WRITE8_MEMBER(multigam_mapper2_w);
	DECLARE_WRITE8_MEMBER(multigam3_mmc3_rom_switch_w);
	DECLARE_WRITE8_MEMBER(multigm3_mapper2_w);
	DECLARE_WRITE8_MEMBER(multigm3_switch_gfx_rom);
	DECLARE_WRITE8_MEMBER(multigm3_switch_prg_rom);
	DECLARE_WRITE8_MEMBER(multigam3_mapper02_rom_switch_w);
	DECLARE_WRITE8_MEMBER(mmc1_rom_switch_w);
	DECLARE_WRITE8_MEMBER(supergm3_prg_bank_w);
	DECLARE_WRITE8_MEMBER(supergm3_chr_bank_w);
	void set_mirroring(int mirroring);
};


/******************************************************

   PPU external bus interface

*******************************************************/


void multigam_state::set_mirroring(int mirroring)
{
	switch(mirroring)
	{
	case PPU_MIRROR_LOW:
		m_nt_page[0] = m_nt_page[1] = m_nt_page[2] = m_nt_page[3] = m_nt_ram;
		break;
	case PPU_MIRROR_HIGH:
		m_nt_page[0] = m_nt_page[1] = m_nt_page[2] = m_nt_page[3] = m_nt_ram + 0x400;
		break;
	case PPU_MIRROR_HORZ:
		m_nt_page[0] = m_nt_ram;
		m_nt_page[1] = m_nt_ram;
		m_nt_page[2] = m_nt_ram + 0x400;
		m_nt_page[3] = m_nt_ram + 0x400;
		break;
	case PPU_MIRROR_VERT:
		m_nt_page[0] = m_nt_ram;
		m_nt_page[1] = m_nt_ram + 0x400;
		m_nt_page[2] = m_nt_ram;
		m_nt_page[3] = m_nt_ram + 0x400;
		break;
	case PPU_MIRROR_NONE:
	default:
		m_nt_page[0] = m_nt_ram;
		m_nt_page[1] = m_nt_ram + 0x400;
		m_nt_page[2] = m_nt_ram + 0x800;
		m_nt_page[3] = m_nt_ram + 0xc00;
		break;
	}
}

WRITE8_MEMBER(multigam_state::multigam_nt_w)
{
	int page = ((offset & 0xc00) >> 10);
	m_nt_page[page][offset & 0x3ff] = data;
}


READ8_MEMBER(multigam_state::multigam_nt_r)
{
	int page = ((offset & 0xc00) >> 10);
	return m_nt_page[page][offset & 0x3ff];
}

static const char * const banknames[] = { "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8", "bank9" };

static void set_videorom_bank(running_machine& machine, int start, int count, int bank, int bank_size_in_kb)
{
	int i;
	int offset = bank * (bank_size_in_kb * 0x400);
	/* bank_size_in_kb is used to determine how large the "bank" parameter is */
	/* count determines the size of the area mapped in KB */
	for (i = 0; i < count; i++, offset += 0x400)
	{
		machine.root_device().membank(banknames[i + start])->set_base(machine.root_device().memregion("gfx1")->base() + offset);
	}
}

static void set_videoram_bank(running_machine& machine, int start, int count, int bank, int bank_size_in_kb)
{
	multigam_state *state = machine.driver_data<multigam_state>();
	int i;
	int offset = bank * (bank_size_in_kb * 0x400);
	/* bank_size_in_kb is used to determine how large the "bank" parameter is */
	/* count determines the size of the area mapped in KB */
	for (i = 0; i < count; i++, offset += 0x400)
	{
		state->membank(banknames[i + start])->set_base(state->m_vram + offset);
	}
}

/******************************************************

   NES interface

*******************************************************/

WRITE8_MEMBER(multigam_state::sprite_dma_w)
{
	int source = (data & 7);
	ppu2c0x_device *ppu = machine().device<ppu2c0x_device>("ppu");
	ppu->spriteram_dma(&space, source);
}

static READ8_DEVICE_HANDLER( psg_4015_r )
{
	return nes_psg_r(device, 0x15);
}

static WRITE8_DEVICE_HANDLER( psg_4015_w )
{
	nes_psg_w(device, 0x15, data);
}

static WRITE8_DEVICE_HANDLER( psg_4017_w )
{
	nes_psg_w(device, 0x17, data);
}

/******************************************************

   Inputs

*******************************************************/


READ8_MEMBER(multigam_state::multigam_IN0_r)
{
	return ((m_in_0 >> m_in_0_shift++) & 0x01) | 0x40;
}

WRITE8_MEMBER(multigam_state::multigam_IN0_w)
{
	if (data & 0x01)
	{
		return;
	}

	m_in_0_shift = 0;
	m_in_1_shift = 0;

	m_in_0 = ioport("P1")->read();
	m_in_1 = ioport("P2")->read();

	m_in_dsw_shift = 0;
	m_in_dsw = ioport("DSW")->read_safe(0);
}

READ8_MEMBER(multigam_state::multigam_IN1_r)
{
	return ((m_in_1 >> m_in_1_shift++) & 0x01) | 0x40;
}

CUSTOM_INPUT_MEMBER(multigam_state::multigam_inputs_r)
{
	/* bit 0: serial input (dsw)
       bit 1: coin */
	return (m_in_dsw >> m_in_dsw_shift++) & 0x01;
}


/******************************************************

   ROM/VROM banking (Multi Game)

*******************************************************/


WRITE8_MEMBER(multigam_state::multigam_switch_prg_rom)
{
	/* switch PRG rom */
	UINT8* dst = memregion("maincpu")->base();
	UINT8* src = memregion("user1")->base();

	if (data & 0x80)
	{
		if (data & 0x01)
		{
			data &= ~0x01;
		}
		memcpy(&dst[0x8000], &src[(data & 0x7f) * 0x4000], 0x8000);
	}
	else
	{
		memcpy(&dst[0x8000], &src[data*0x4000], 0x4000);
		memcpy(&dst[0xc000], &src[data*0x4000], 0x4000);
	}
};

WRITE8_MEMBER(multigam_state::multigam_switch_gfx_rom)
{
	membank("bank1")->set_base(machine().root_device().memregion("gfx1")->base() + (0x2000 * (data & 0x3f)));
	set_mirroring(data & 0x40 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_game_gfx_bank = data;
};


WRITE8_MEMBER(multigam_state::multigam_mapper2_w)
{
	if (m_game_gfx_bank & 0x80)
	{
		membank("bank1")->set_base(machine().root_device().memregion("gfx1")->base() + (0x2000 * ((data & 0x3) + (m_game_gfx_bank & 0x3c))));
	}
	else
	{
		logerror("Unmapped multigam_mapper2_w: offset = %04X, data = %02X\n", offset, data);
	}
}

/******************************************************

   Memory map (Multi Game)

*******************************************************/

static ADDRESS_MAP_START( multigam_map, AS_PROGRAM, 8, multigam_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM	/* NES RAM */
	AM_RANGE(0x0800, 0x0fff) AM_RAM /* additional RAM */
	AM_RANGE(0x2000, 0x3fff) AM_DEVREADWRITE("ppu", ppu2c0x_device, read, write)
	AM_RANGE(0x4000, 0x4013) AM_DEVREADWRITE_LEGACY("nes", nes_psg_r, nes_psg_w)			/* PSG primary registers */
	AM_RANGE(0x4014, 0x4014) AM_WRITE(sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_DEVREADWRITE_LEGACY("nes", psg_4015_r, psg_4015_w)			/* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(multigam_IN0_r, multigam_IN0_w)	/* IN0 - input port 1 */
	AM_RANGE(0x4017, 0x4017) AM_READ(multigam_IN1_r) AM_DEVWRITE_LEGACY("nes", psg_4017_w)		/* IN1 - input port 2 / PSG second control register */
	AM_RANGE(0x5002, 0x5002) AM_WRITENOP
	AM_RANGE(0x5000, 0x5ffe) AM_ROM
	AM_RANGE(0x5fff, 0x5fff) AM_READ_PORT("IN0")
	AM_RANGE(0x6000, 0x7fff) AM_ROM
	AM_RANGE(0x6fff, 0x6fff) AM_WRITE(multigam_switch_prg_rom)
	AM_RANGE(0x7fff, 0x7fff) AM_WRITE(multigam_switch_gfx_rom)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_WRITE(multigam_mapper2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( multigmt_map, AS_PROGRAM, 8, multigam_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM	/* NES RAM */
	AM_RANGE(0x0800, 0x0fff) AM_RAM /* additional RAM */
	AM_RANGE(0x3000, 0x3000) AM_WRITE(multigam_switch_prg_rom)
	AM_RANGE(0x3fff, 0x3fff) AM_WRITE(multigam_switch_gfx_rom)
	AM_RANGE(0x2000, 0x3fff) AM_DEVREADWRITE("ppu", ppu2c0x_device, read, write)
	AM_RANGE(0x4000, 0x4013) AM_DEVREADWRITE_LEGACY("nes", nes_psg_r, nes_psg_w)			/* PSG primary registers */
	AM_RANGE(0x4014, 0x4014) AM_WRITE(sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_DEVREADWRITE_LEGACY("nes", psg_4015_r, psg_4015_w)			/* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(multigam_IN0_r, multigam_IN0_w)	/* IN0 - input port 1 */
	AM_RANGE(0x4017, 0x4017) AM_READ(multigam_IN1_r) AM_DEVWRITE_LEGACY("nes", psg_4017_w)		/* IN1 - input port 2 / PSG second control register */
	AM_RANGE(0x5002, 0x5002) AM_WRITENOP
	AM_RANGE(0x5000, 0x5ffe) AM_ROM
	AM_RANGE(0x5fff, 0x5fff) AM_READ_PORT("IN0")
	AM_RANGE(0x6000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_WRITE(multigam_mapper2_w)
ADDRESS_MAP_END

/******************************************************

   MMC3 (Multi Game III)

*******************************************************/


static void multigam3_mmc3_scanline_cb( device_t *device, int scanline, int vblank, int blanked )
{
	multigam_state *state = device->machine().driver_data<multigam_state>();
	if (!vblank && !blanked)
	{
		if (--state->m_multigam3_mmc3_scanline_counter == -1)
		{
			state->m_multigam3_mmc3_scanline_counter = state->m_multigam3_mmc3_scanline_latch;
			generic_pulse_irq_line(device->machine().device("maincpu"), 0, 1);
		}
	}
}

WRITE8_MEMBER(multigam_state::multigam3_mmc3_rom_switch_w)
{
	ppu2c0x_device *ppu = machine().device<ppu2c0x_device>("ppu");

	/* basically, a MMC3 mapper from the nes */
	int bankmask = m_multigam3_mmc3_prg_size == 0x40000 ? 0x1f : 0x0f;

	switch(offset & 0x7001)
	{
		case 0x0000:
			m_multigam3_mmc3_command = data;

			if (m_multigam3_mmc3_last_bank != (data & 0xc0))
			{
				int bank;
				UINT8 *prg = memregion("maincpu")->base();

				/* reset the banks */
				if (m_multigam3_mmc3_command & 0x40)
				{
					/* high bank */
					bank = (m_multigam3_mmc3_banks[0] & bankmask) * 0x2000;

					memcpy(&prg[0x0c000], &m_multigam3_mmc3_prg_base[bank], 0x2000);
					memcpy(&prg[0x08000], &m_multigam3_mmc3_prg_base[m_multigam3_mmc3_prg_size - 0x4000], 0x2000);
				}
				else
				{
					/* low bank */
					bank = (m_multigam3_mmc3_banks[0] & bankmask) * 0x2000;

					memcpy(&prg[0x08000], &m_multigam3_mmc3_prg_base[bank], 0x2000);
					memcpy(&prg[0x0c000], &m_multigam3_mmc3_prg_base[m_multigam3_mmc3_prg_size - 0x4000], 0x2000);
				}

				/* mid bank */
				bank = (m_multigam3_mmc3_banks[1] & bankmask) * 0x2000;
				memcpy(&prg[0x0a000], &m_multigam3_mmc3_prg_base[bank], 0x2000);

				m_multigam3_mmc3_last_bank = data & 0xc0;
			}
		break;

		case 0x0001:
			{
				UINT8 cmd = m_multigam3_mmc3_command & 0x07;
				int page = (m_multigam3_mmc3_command & 0x80) >> 5;
				int bank;

				switch (cmd)
				{
					case 0:	/* char banking */
					case 1: /* char banking */
						data &= 0xfe;
						page ^= (cmd << 1);
						set_videorom_bank(machine(), page, 2, m_multigam3_mmc3_chr_bank_base + data, 1);
					break;

					case 2: /* char banking */
					case 3: /* char banking */
					case 4: /* char banking */
					case 5: /* char banking */
						page ^= cmd + 2;
						set_videorom_bank(machine(), page, 1, m_multigam3_mmc3_chr_bank_base + data, 1);
					break;

					case 6: /* program banking */
					{
						UINT8 *prg = memregion("maincpu")->base();
						if (m_multigam3_mmc3_command & 0x40)
						{
							/* high bank */
							m_multigam3_mmc3_banks[0] = data & bankmask;
							bank = (m_multigam3_mmc3_banks[0]) * 0x2000;

							memcpy(&prg[0x0c000], &m_multigam3_mmc3_prg_base[bank], 0x2000);
							memcpy(&prg[0x08000], &m_multigam3_mmc3_prg_base[m_multigam3_mmc3_prg_size - 0x4000], 0x2000);
						}
						else
						{
							/* low bank */
							m_multigam3_mmc3_banks[0] = data & bankmask;
							bank = (m_multigam3_mmc3_banks[0]) * 0x2000;

							memcpy(&prg[0x08000], &m_multigam3_mmc3_prg_base[bank], 0x2000);
							memcpy(&prg[0x0c000], &m_multigam3_mmc3_prg_base[m_multigam3_mmc3_prg_size - 0x4000], 0x2000);
						}
					}
					break;

					case 7: /* program banking */
						{
							/* mid bank */
							UINT8 *prg = memregion("maincpu")->base();

							m_multigam3_mmc3_banks[1] = data & bankmask;
							bank = m_multigam3_mmc3_banks[1] * 0x2000;

							memcpy(&prg[0x0a000], &m_multigam3_mmc3_prg_base[bank], 0x2000);
						}
					break;
				}
			}
		break;

		case 0x2000: /* mirroring */
			if (!m_multigam3_mmc3_4screen)
			{
				if (data & 0x40)
					set_mirroring(PPU_MIRROR_HIGH);
				else
					set_mirroring((data & 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			}
		break;

		case 0x2001: /* enable ram at $6000 */
			if (data & 0x80)
			{
				membank("bank10")->set_base(m_multigmc_mmc3_6000_ram);
			}
			else
			{
				membank("bank10")->set_base(machine().root_device().memregion("maincpu")->base() + 0x6000);
			}
			if (data & 0x40)
			{
				logerror("Write protect for 6000 enabled\n");
			}
		break;

		case 0x4000: /* scanline counter */
			m_multigam3_mmc3_scanline_counter = data;
		break;

		case 0x4001: /* scanline latch */
			m_multigam3_mmc3_scanline_latch = data;
		break;

		case 0x6000: /* disable irqs */
			ppu->set_scanline_callback(0);
		break;

		case 0x6001: /* enable irqs */
			ppu->set_scanline_callback(multigam3_mmc3_scanline_cb);
		break;
	}
}

static void multigam_init_mmc3(running_machine &machine, UINT8 *prg_base, int prg_size, int chr_bank_base)
{
	multigam_state *state = machine.driver_data<multigam_state>();
	UINT8* dst = state->memregion("maincpu")->base();

	// Tom & Jerry in Super Game III enables 6000 ram, but does not read/write it
	// however, it expects ROM from 6000 there (code jumps to $6xxx)
	memcpy(state->m_multigmc_mmc3_6000_ram, dst + 0x6000, 0x2000);

	memcpy(&dst[0x8000], prg_base + (prg_size - 0x4000), 0x4000);
	memcpy(&dst[0xc000], prg_base + (prg_size - 0x4000), 0x4000);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(multigam_state::multigam3_mmc3_rom_switch_w),state));

	state->m_multigam3_mmc3_banks[0] = 0x1e;
	state->m_multigam3_mmc3_banks[1] = 0x1f;
	state->m_multigam3_mmc3_scanline_counter = 0;
	state->m_multigam3_mmc3_scanline_latch = 0;
	state->m_multigam3_mmc3_4screen = 0;
	state->m_multigam3_mmc3_last_bank = 0xff;
	state->m_multigam3_mmc3_prg_base = prg_base;
	state->m_multigam3_mmc3_chr_bank_base = chr_bank_base;
	state->m_multigam3_mmc3_prg_size = prg_size;
};

WRITE8_MEMBER(multigam_state::multigm3_mapper2_w)
{
	if (m_game_gfx_bank & 0x80)
	{
		set_videorom_bank(machine(), 0, 8, (m_game_gfx_bank & 0x3c)  + (data & 0x3), 8);
	}
	else
	{
		logerror("Unmapped multigam_mapper2_w: offset = %04X, data = %02X\n", offset, data);
	}
};

WRITE8_MEMBER(multigam_state::multigm3_switch_gfx_rom)
{
	set_videorom_bank(machine(), 0, 8, data & 0x3f, 8);
	set_mirroring(data & 0x40 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_game_gfx_bank = data;
};

WRITE8_MEMBER(multigam_state::multigm3_switch_prg_rom)
{
	/* switch PRG rom */
	UINT8* dst = memregion("maincpu")->base();
	UINT8* src = memregion("user1")->base();

	if (data == 0xa8)
	{
		multigam_init_mmc3(machine(), src + 0xa0000, 0x40000, 0x180);
		return;
	}
	else
	{
		space.install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(multigam_state::multigm3_mapper2_w),this) );
		membank("bank10")->set_base(machine().root_device().memregion("maincpu")->base() + 0x6000);
	}

	if (data & 0x80)
	{
		if (data & 0x01)
		{
			data &= ~0x01;
		}
		memcpy(&dst[0x8000], &src[(data & 0x7f) * 0x4000], 0x8000);
	}
	else
	{
		memcpy(&dst[0x8000], &src[data*0x4000], 0x4000);
		memcpy(&dst[0xc000], &src[data*0x4000], 0x4000);
	}
};

/******************************************************

   Memory map (Multi Game III)

*******************************************************/

static ADDRESS_MAP_START( multigm3_map, AS_PROGRAM, 8, multigam_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM	/* NES RAM */
	AM_RANGE(0x0800, 0x0fff) AM_RAM /* additional RAM */
	AM_RANGE(0x2000, 0x3fff) AM_DEVREADWRITE("ppu", ppu2c0x_device, read, write)
	AM_RANGE(0x4000, 0x4013) AM_DEVREADWRITE_LEGACY("nes", nes_psg_r, nes_psg_w)			/* PSG primary registers */
	AM_RANGE(0x4014, 0x4014) AM_WRITE(sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_DEVREADWRITE_LEGACY("nes", psg_4015_r, psg_4015_w)			/* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(multigam_IN0_r, multigam_IN0_w)	/* IN0 - input port 1 */
	AM_RANGE(0x4017, 0x4017) AM_READ(multigam_IN1_r) AM_DEVWRITE_LEGACY("nes", psg_4017_w)		/* IN1 - input port 2 / PSG second control register */
	AM_RANGE(0x5001, 0x5001) AM_WRITE(multigm3_switch_prg_rom)
	AM_RANGE(0x5002, 0x5002) AM_WRITENOP
	AM_RANGE(0x5003, 0x5003) AM_WRITE(multigm3_switch_gfx_rom)
	AM_RANGE(0x5000, 0x5ffe) AM_ROM
	AM_RANGE(0x5fff, 0x5fff) AM_READ_PORT("IN0")
	AM_RANGE(0x6000, 0x7fff) AM_RAMBANK("bank10")
	AM_RANGE(0x6fff, 0x6fff) AM_WRITENOP /* 0x00 in attract mode, 0xff during play */
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_WRITE(multigm3_mapper2_w)
ADDRESS_MAP_END

/******************************************************

   Mapper 02

*******************************************************/


WRITE8_MEMBER(multigam_state::multigam3_mapper02_rom_switch_w)
{
	UINT8* mem = memregion("maincpu")->base();
	int bankmask = (m_mapper02_prg_size/0x4000) - 1;
	memcpy(mem + 0x8000, m_mapper02_prg_base + 0x4000*(data & bankmask), 0x4000);
}

static void multigam_init_mapper02(running_machine &machine, UINT8* prg_base, int prg_size)
{
	multigam_state *state = machine.driver_data<multigam_state>();
	ppu2c0x_device *ppu = machine.device<ppu2c0x_device>("ppu");
	UINT8* mem = state->memregion("maincpu")->base();
	memcpy(mem + 0x8000, prg_base + prg_size - 0x8000, 0x8000);
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(multigam_state::multigam3_mapper02_rom_switch_w),state));

	state->m_mapper02_prg_base = prg_base;
	state->m_mapper02_prg_size = prg_size;
	ppu->set_scanline_callback(0);
}

/******************************************************

   MMC1 mapper

*******************************************************/


static TIMER_CALLBACK( mmc1_resync_callback )
{
	multigam_state *state = machine.driver_data<multigam_state>();
	state->m_mmc1_reg_write_enable = 1;
}

WRITE8_MEMBER(multigam_state::mmc1_rom_switch_w)
{
	/* basically, a MMC1 mapper from the nes */

	if ( m_mmc1_reg_write_enable == 0 )
	{
		return;
	}
	else
	{
		m_mmc1_reg_write_enable = 0;
		machine().scheduler().synchronize(FUNC(mmc1_resync_callback));
	}

	int reg = (offset >> 13);

	/* reset mapper */
	if (data & 0x80)
	{
		m_mmc1_shiftreg = m_mmc1_shiftcount = 0;

		m_size16k = 1;
		m_switchlow = 1;
		m_vrom4k = 0;

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
			case 0:		/* mirroring and options */
				{
					int _mirroring;

					m_vrom4k = m_mmc1_shiftreg & 0x10;
					m_size16k = m_mmc1_shiftreg & 0x08;
					m_switchlow = m_mmc1_shiftreg & 0x04;

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
					set_mirroring(_mirroring);
				}
			break;

			case 1:	/* video rom banking - bank 0 - 4k or 8k */
				if (m_mmc1_chr_bank_base == 0)
					set_videoram_bank(machine(), 0, (m_vrom4k) ? 4 : 8, (m_mmc1_shiftreg & 0x1f), 4);
				else
					set_videorom_bank(machine(), 0, (m_vrom4k) ? 4 : 8, m_mmc1_chr_bank_base + (m_mmc1_shiftreg & 0x1f), 4);
			break;

			case 2: /* video rom banking - bank 1 - 4k only */
				if (m_vrom4k)
				{
					if (m_mmc1_chr_bank_base == 0)
						set_videoram_bank(machine(), 0, (m_vrom4k) ? 4 : 8, (m_mmc1_shiftreg & 0x1f), 4);
					else
						set_videorom_bank(machine(), 4, 4, m_mmc1_chr_bank_base + (m_mmc1_shiftreg & 0x1f), 4);
				}
			break;

			case 3:	/* program banking */
				{
					int bank = (m_mmc1_shiftreg & m_mmc1_rom_mask) * 0x4000;
					UINT8 *prg = memregion("maincpu")->base();

					if (!m_size16k)
					{
						bank = ((m_mmc1_shiftreg >> 1) & m_mmc1_rom_mask) * 0x4000;
						/* switch 32k */
						memcpy(&prg[0x08000], m_mmc1_prg_base + bank, 0x8000);
					}
					else
					{
						/* switch 16k */
						if (m_switchlow)
						{
							/* low */
							memcpy(&prg[0x08000], m_mmc1_prg_base + bank, 0x4000);
							memcpy(&prg[0x0c000], m_mmc1_prg_base + (0x0f & m_mmc1_rom_mask) * 0x4000, 0x4000);
						}
						else
						{
							/* high */
							memcpy(&prg[0x08000], m_mmc1_prg_base + (0x00 & m_mmc1_rom_mask) * 0x4000, 0x4000);
							memcpy(&prg[0x0c000], m_mmc1_prg_base + bank, 0x4000);
						}
					}
				}
			break;
		}
	}
}

static void multigam_init_mmc1(running_machine &machine, UINT8 *prg_base, int prg_size, int chr_bank_base)
{
	multigam_state *state = machine.driver_data<multigam_state>();
	UINT8* dst = state->memregion("maincpu")->base();
	ppu2c0x_device *ppu = machine.device<ppu2c0x_device>("ppu");

	memcpy(&dst[0x8000], prg_base + (prg_size - 0x8000), 0x8000);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(multigam_state::mmc1_rom_switch_w),state));

	state->m_mmc1_reg_write_enable = 1;
	state->m_mmc1_rom_mask = (prg_size / 0x4000) - 1;
	state->m_mmc1_prg_base = prg_base;
	state->m_mmc1_prg_size = prg_size;
	state->m_mmc1_chr_bank_base = chr_bank_base;

	ppu->set_scanline_callback(0);
};


/******************************************************

   ROM/VROM banking (Super Game III)

*******************************************************/

/*
      76543210
5001: x         select rom at 0x8000: 0 - control program, 1 - game
       x        mapper for game: 0 - mapper 02, 1 - MMC1/MMC3
        x       game rom size: 0 - 256KB, 1 - 128KB
         xxxxx  rom bank, each bank has 0x20000 bytes (128KB)

      76543210
5002: x         unknown, always 0
       x        CHR rom size: 0 - 256KB, 1 - 128KB
        x       CHR rom flag: 0 - no CHR rom, 1 - CHR rom present
         x      mapper for game: 0 - MMC1, 1 - MMC3
          xxxx  CHR rom bank, each bank has 0x8000 bytes (32KB)
*/


static void supergm3_set_bank(running_machine &machine)
{
	multigam_state *state = machine.driver_data<multigam_state>();
	ppu2c0x_device *ppu = machine.device<ppu2c0x_device>("ppu");
	UINT8* mem = state->memregion("maincpu")->base();

	// video bank
	if (state->m_supergm3_chr_bank == 0x10 ||
		state->m_supergm3_chr_bank == 0x40 )
	{
		// VRAM
		ppu->memory().space(AS_PROGRAM)->install_read_bank(0x0000, 0x1fff, "bank1");
		ppu->memory().space(AS_PROGRAM)->install_write_bank(0x0000, 0x1fff, "bank1");
		state->membank("bank1")->set_base(state->m_vram);

		if (state->m_supergm3_chr_bank == 0x40)
			state->set_mirroring(PPU_MIRROR_VERT);
	}
	else
	{
		ppu->memory().space(AS_PROGRAM)->install_read_bank(0x0000, 0x03ff, "bank2");
		ppu->memory().space(AS_PROGRAM)->install_read_bank(0x0400, 0x07ff, "bank3");
		ppu->memory().space(AS_PROGRAM)->install_read_bank(0x0800, 0x0bff, "bank4");
		ppu->memory().space(AS_PROGRAM)->install_read_bank(0x0c00, 0x0fff, "bank5");
		ppu->memory().space(AS_PROGRAM)->install_read_bank(0x1000, 0x13ff, "bank6");
		ppu->memory().space(AS_PROGRAM)->install_read_bank(0x1400, 0x17ff, "bank7");
		ppu->memory().space(AS_PROGRAM)->install_read_bank(0x1800, 0x1bff, "bank8");
		ppu->memory().space(AS_PROGRAM)->install_read_bank(0x1c00, 0x1fff, "bank9");
		ppu->memory().space(AS_PROGRAM)->unmap_write(0x0000, 0x1fff);

		set_videorom_bank(machine, 0, 8, 0, 8);
	}

	// prg bank
	if ((state->m_supergm3_prg_bank & 0x80) == 0)
	{
		// title screen
		memcpy(mem + 0x8000, mem + 0x18000, 0x8000);
		state->membank("bank10")->set_base(mem + 0x6000);
		ppu->set_scanline_callback(0);
	}
	else if ((state->m_supergm3_prg_bank & 0x40) == 0)
	{
		// mapper 02
		multigam_init_mapper02(machine,
			machine.root_device().memregion("user1")->base() + (state->m_supergm3_prg_bank & 0x1f)*0x20000,
			0x20000);
	}
	else if (state->m_supergm3_chr_bank & 0x10)
	{
		// MMC3
		multigam_init_mmc3(machine,
			machine.root_device().memregion("user1")->base() + (state->m_supergm3_prg_bank & 0x1f)*0x20000,
			(state->m_supergm3_prg_bank & 0x20) ? 0x20000 : 0x40000,
			(state->m_supergm3_chr_bank & 0x0f)*0x80);
	}
	else
	{
		//MMC1
		multigam_init_mmc1(machine,
			machine.root_device().memregion("user1")->base() + (state->m_supergm3_prg_bank & 0x1f)*0x20000,
			0x20000,
			(state->m_supergm3_chr_bank & 0x0f)*0x80/4 );
	}
}

WRITE8_MEMBER(multigam_state::supergm3_prg_bank_w)
{
	m_supergm3_prg_bank = data;
}

WRITE8_MEMBER(multigam_state::supergm3_chr_bank_w)
{
	m_supergm3_chr_bank = data;
	supergm3_set_bank(machine());
}

/******************************************************

   Memory map (Super Game III)

*******************************************************/

static ADDRESS_MAP_START( supergm3_map, AS_PROGRAM, 8, multigam_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM	/* NES RAM */
	AM_RANGE(0x0800, 0x0fff) AM_RAM /* additional RAM */
	AM_RANGE(0x2000, 0x3fff) AM_DEVREADWRITE("ppu", ppu2c0x_device, read, write)
	AM_RANGE(0x4000, 0x4013) AM_DEVREADWRITE_LEGACY("nes", nes_psg_r, nes_psg_w)			/* PSG primary registers */
	AM_RANGE(0x4014, 0x4014) AM_WRITE(sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_DEVREADWRITE_LEGACY("nes", psg_4015_r, psg_4015_w)			/* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(multigam_IN0_r, multigam_IN0_w)	/* IN0 - input port 1 */
	AM_RANGE(0x4017, 0x4017) AM_READ(multigam_IN1_r) AM_DEVWRITE_LEGACY("nes", psg_4017_w)		/* IN1 - input port 2 / PSG second control register */
	AM_RANGE(0x4fff, 0x4fff) AM_READ_PORT("IN0")
	AM_RANGE(0x5000, 0x5fff) AM_ROM
	AM_RANGE(0x5000, 0x5000) AM_WRITENOP
	AM_RANGE(0x5001, 0x5001) AM_WRITE(supergm3_prg_bank_w)
	AM_RANGE(0x5002, 0x5002) AM_WRITE(supergm3_chr_bank_w)
	AM_RANGE(0x5fff, 0x5fff) AM_WRITENOP
	AM_RANGE(0x6000, 0x7fff) AM_RAMBANK("bank10")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

/******************************************************

   Input ports

*******************************************************/

static INPUT_PORTS_START( multigam_common )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	/* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)	/* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, multigam_state,multigam_inputs_r, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( multigam )
	PORT_INCLUDE( multigam_common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x06, 0x00, "Coin/Time" )
	PORT_DIPSETTING(    0x00, "3 min" )
	PORT_DIPSETTING(    0x04, "5 min" )
	PORT_DIPSETTING(    0x02, "7 min" )
	PORT_DIPSETTING(    0x06, "10 min" )
INPUT_PORTS_END

static INPUT_PORTS_START( multigm2 )
	PORT_INCLUDE( multigam_common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x06, 0x00, "Coin/Time" )
	PORT_DIPSETTING(    0x00, "5 min" )
	PORT_DIPSETTING(    0x04, "8 min" )
	PORT_DIPSETTING(    0x02, "11 min" )
	PORT_DIPSETTING(    0x06, "15 min" )
INPUT_PORTS_END

static INPUT_PORTS_START( multigm3 )
	PORT_INCLUDE( multigam_common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x06, 0x00, "Coin/Time" )
	PORT_DIPSETTING(    0x00, "15 min" )
	PORT_DIPSETTING(    0x04, "8 min" )
	PORT_DIPSETTING(    0x02, "11 min" )
	PORT_DIPSETTING(    0x06, "5 min" )
INPUT_PORTS_END

static INPUT_PORTS_START( multigmt )
	PORT_INCLUDE( multigam_common )

	PORT_START("DSW")
	PORT_DIPNAME( 0xf0, 0x00, "Game" )
	PORT_DIPSETTING( 0x00, "Pacman" )
	PORT_DIPSETTING( 0x10, "Super Mario Bros" )
	PORT_DIPSETTING( 0x20, "Tetris" )
	PORT_DIPSETTING( 0x30, "Road Fighter" )
	PORT_DIPSETTING( 0x40, "Gorilla 3" )
	PORT_DIPSETTING( 0x50, "Zippy Race" )
	PORT_DIPSETTING( 0x80, "Tennis" )
	PORT_DIPSETTING( 0x90, "Galaga" )
	PORT_DIPSETTING( 0xa0, "Track & Field" )
	PORT_DIPSETTING( 0xb0, "Bomb Jack" )

	PORT_DIPNAME( 0x06, 0x00, "Coin/Time" )
	PORT_DIPSETTING( 0x00, "3 min" )
	PORT_DIPSETTING( 0x04, "5 min" )
	PORT_DIPSETTING( 0x02, "7 min" )
	PORT_DIPSETTING( 0x06, "10 min" )
INPUT_PORTS_END

static INPUT_PORTS_START( supergm3 )
	PORT_INCLUDE( multigam_common )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x03, 0x00, "Play Time per Credit" )
	PORT_DIPSETTING(    0x00, "3 min" )
	PORT_DIPSETTING(    0x02, "5 min" )
	PORT_DIPSETTING(    0x01, "8 min" )
	PORT_DIPSETTING(    0x03, "10 min" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x00, "Enable 2 players" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END
/******************************************************

   PPU/Video interface

*******************************************************/

static const nes_interface multigam_interface_1 =
{
	"maincpu"
};

static PALETTE_INIT( multigam )
{
	ppu2c0x_device *ppu = machine.device<ppu2c0x_device>("ppu");
	ppu->init_palette(machine, 0);
}

static void ppu_irq( device_t *device, int *ppu_regs )
{
	cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_NMI, PULSE_LINE);
}

/* our ppu interface                                            */
static const ppu2c0x_interface ppu_interface =
{
	"maincpu",
	"screen",
	0,					/* gfxlayout num */
	0,					/* color base */
	PPU_MIRROR_NONE,	/* mirroring */
	ppu_irq				/* irq */
};

static VIDEO_START( multigam )
{
}

static SCREEN_UPDATE_IND16( multigam )
{
	/* render the ppu */
	ppu2c0x_device *ppu = screen.machine().device<ppu2c0x_device>("ppu");
	ppu->render(bitmap, 0, 0, 0, 0);
	return 0;
}

static GFXDECODE_START( multigam )
	/* none, the ppu generates one */
GFXDECODE_END

/******************************************************

   Machine

*******************************************************/

static MACHINE_RESET( multigam )
{
}

static MACHINE_RESET( multigm3 )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	/* reset the ppu */
	multigam_state *state = machine.driver_data<multigam_state>();
	state->multigm3_switch_prg_rom(*space, 0, 0x01 );
};

static MACHINE_START( multigam )
{
	multigam_state *state = machine.driver_data<multigam_state>();
	state->m_nt_ram = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_nt_page[0] = state->m_nt_ram;
	state->m_nt_page[1] = state->m_nt_ram + 0x400;
	state->m_nt_page[2] = state->m_nt_ram + 0x800;
	state->m_nt_page[3] = state->m_nt_ram + 0xc00;

	machine.device("ppu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(multigam_state::multigam_nt_r),state), write8_delegate(FUNC(multigam_state::multigam_nt_w),state));
	machine.device("ppu")->memory().space(AS_PROGRAM)->install_read_bank(0x0000, 0x1fff, "bank1");
	state->membank("bank1")->set_base(state->memregion("gfx1")->base());
}

static MACHINE_START( multigm3 )
{
	multigam_state *state = machine.driver_data<multigam_state>();
	state->m_nt_ram = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_nt_page[0] = state->m_nt_ram;
	state->m_nt_page[1] = state->m_nt_ram + 0x400;
	state->m_nt_page[2] = state->m_nt_ram + 0x800;
	state->m_nt_page[3] = state->m_nt_ram + 0xc00;

	machine.device("ppu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(multigam_state::multigam_nt_r),state), write8_delegate(FUNC(multigam_state::multigam_nt_w),state));

	machine.device("ppu")->memory().space(AS_PROGRAM)->install_read_bank(0x0000, 0x03ff, "bank2");
	machine.device("ppu")->memory().space(AS_PROGRAM)->install_read_bank(0x0400, 0x07ff, "bank3");
	machine.device("ppu")->memory().space(AS_PROGRAM)->install_read_bank(0x0800, 0x0bff, "bank4");
	machine.device("ppu")->memory().space(AS_PROGRAM)->install_read_bank(0x0c00, 0x0fff, "bank5");
	machine.device("ppu")->memory().space(AS_PROGRAM)->install_read_bank(0x1000, 0x13ff, "bank6");
	machine.device("ppu")->memory().space(AS_PROGRAM)->install_read_bank(0x1400, 0x17ff, "bank7");
	machine.device("ppu")->memory().space(AS_PROGRAM)->install_read_bank(0x1800, 0x1bff, "bank8");
	machine.device("ppu")->memory().space(AS_PROGRAM)->install_read_bank(0x1c00, 0x1fff, "bank9");

	set_videorom_bank(machine, 0, 8, 0, 8);
};

static MACHINE_START( supergm3 )
{
	multigam_state *state = machine.driver_data<multigam_state>();
	state->m_nt_ram = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_nt_page[0] = state->m_nt_ram;
	state->m_nt_page[1] = state->m_nt_ram + 0x400;
	state->m_nt_page[2] = state->m_nt_ram + 0x800;
	state->m_nt_page[3] = state->m_nt_ram + 0xc00;

	machine.device("ppu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(multigam_state::multigam_nt_r),state), write8_delegate(FUNC(multigam_state::multigam_nt_w),state));

	state->m_vram = auto_alloc_array(machine, UINT8, 0x2000);
	state->m_multigmc_mmc3_6000_ram = auto_alloc_array(machine, UINT8, 0x2000);
}

static MACHINE_CONFIG_START( multigam, multigam_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", N2A03, N2A03_DEFAULTCLOCK)
	MCFG_CPU_PROGRAM_MAP(multigam_map)

	MCFG_MACHINE_START( multigam )
	MCFG_MACHINE_RESET( multigam )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(multigam)

	MCFG_GFXDECODE(multigam)
	MCFG_PALETTE_LENGTH(8*4*16)

	MCFG_PALETTE_INIT(multigam)
	MCFG_VIDEO_START(multigam)

	MCFG_PPU2C04_ADD("ppu", ppu_interface)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("nes", NES, N2A03_DEFAULTCLOCK)
	MCFG_SOUND_CONFIG(multigam_interface_1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( multigm3, multigam )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(multigm3_map)

	MCFG_MACHINE_START( multigm3 )
	MCFG_MACHINE_RESET( multigm3 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( multigmt, multigam )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(multigmt_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( supergm3, multigam )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(supergm3_map)

	MCFG_MACHINE_START(supergm3)
MACHINE_CONFIG_END

ROM_START( multigam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7.bin", 0x0000, 0x8000, CRC(f0fa7cf2) SHA1(7f3b3dca796b964893197aef7f0f31dfd7a2c1a4) )

	ROM_REGION( 0xC0000, "user1", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x20000, CRC(e0bb14a5) SHA1(74026f59dfb08456183adaaf381bb28830212a1c) )
	ROM_LOAD( "2.bin", 0x20000, 0x20000, CRC(f52c07ad) SHA1(51be288bcf5aeab5bdd95ee93a6d807867e30e97) )
	ROM_LOAD( "3.bin", 0x40000, 0x20000, CRC(92e8303e) SHA1(4a735fce22cdea65801aa7e4e00fa8c15a022ea4) )
	ROM_LOAD( "4.bin", 0x60000, 0x20000, CRC(f5c50f29) SHA1(6f3bd969d9d82a0d92aa6119375a607ccdb5d8b9) )
	ROM_LOAD( "5.bin", 0x80000, 0x20000, CRC(e1232243) SHA1(fa864b255b7f3cb195d3789f8a2a7b3848b255a2) )
	ROM_LOAD( "6.bin", 0xa0000, 0x20000, CRC(fb129b09) SHA1(b677937d6cf7800359dc6d35dd2de3ec27919d71) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "8.bin", 0x00000, 0x20000, CRC(509c0e94) SHA1(9e87c74e76afe6c3a7ba194439434f54e2c879eb) )
	ROM_LOAD( "9.bin", 0x20000, 0x20000, CRC(48416f20) SHA1(f461fcbff6d2fa4774d64c26475089d1aeea7fb5) )
	ROM_LOAD( "10.bin", 0x40000, 0x20000, CRC(869d1661) SHA1(ac155bd24ebfea8e064859e1a05317001286f9ae) )
	ROM_FILL( 0x60000, 0x20000, 0x00 )
ROM_END

ROM_START( multigmb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mg_4.bin", 0x0000, 0x8000, CRC(079226c1) SHA1(f4ceedd9b6cc3454550be7421dc75904ad664545) )

	ROM_REGION( 0xC0000, "user1", 0 )
	ROM_LOAD( "1.bin",    0x00000, 0x20000, CRC(e0bb14a5) SHA1(74026f59dfb08456183adaaf381bb28830212a1c) )
	ROM_LOAD( "2.bin",    0x20000, 0x20000, CRC(f52c07ad) SHA1(51be288bcf5aeab5bdd95ee93a6d807867e30e97) )
	ROM_LOAD( "3.bin",    0x40000, 0x20000, CRC(92e8303e) SHA1(4a735fce22cdea65801aa7e4e00fa8c15a022ea4) )
	ROM_LOAD( "4.bin",    0x60000, 0x20000, CRC(f5c50f29) SHA1(6f3bd969d9d82a0d92aa6119375a607ccdb5d8b9) )
	ROM_LOAD( "mg_9.bin", 0x80000, 0x20000, CRC(47f968af) SHA1(ed60bda060984e5acc2c6b6cac5b36eafbb2b631) )
	ROM_LOAD( "6.bin",    0xa0000, 0x20000, CRC(fb129b09) SHA1(b677937d6cf7800359dc6d35dd2de3ec27919d71) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "8.bin", 0x00000, 0x20000, CRC(509c0e94) SHA1(9e87c74e76afe6c3a7ba194439434f54e2c879eb) )
	ROM_LOAD( "9.bin", 0x20000, 0x20000, CRC(48416f20) SHA1(f461fcbff6d2fa4774d64c26475089d1aeea7fb5) )
	ROM_LOAD( "10.bin", 0x40000, 0x20000, CRC(869d1661) SHA1(ac155bd24ebfea8e064859e1a05317001286f9ae) )
	ROM_FILL( 0x60000, 0x20000, 0x00 )
ROM_END

ROM_START( multigm2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mg2-8u21.bin", 0x0000, 0x8000, CRC(e0588e11) SHA1(43b5f6bad7828b372c6b4251e9e24c748793bb2d) )

	ROM_REGION( 0x100000, "user1", 0 )
	ROM_LOAD( "mg2-4u10.bin", 0x00000, 0x20000, CRC(fc7b2461) SHA1(60cf296ba531495d577eac73220a7ae1dc897237) )
	ROM_LOAD( "mg2-6-u4.bin", 0x20000, 0x20000, CRC(f1ced4a6) SHA1(d94d7c9fc9270d0e5eaecf819e4048c20866ba2d) )
	ROM_LOAD( "mg2-3u15.bin", 0x40000, 0x20000, CRC(a1f84c4f) SHA1(c191677170c7d8907bbaee599a988c9adcc17449) )
	ROM_LOAD( "mg2-2u20.bin", 0x60000, 0x10000, CRC(8413c2bd) SHA1(54e9a8c2ccfcb1a70ea7aa6125116829e4c72855) )
	ROM_LOAD( "mg2-1u24.bin", 0x70000, 0x10000, CRC(53fd3238) SHA1(01095e6b853fa55783e9ac635fbf7c1450dc3eb1) )
	ROM_LOAD( "mg2-5-u6.bin", 0x80000, 0x20000, CRC(0b95a92d) SHA1(df2e30d537a6703cc614ecc4b93b828f1339807b) )
	ROM_LOAD( "mg2-7-u1.bin", 0xa0000, 0x40000, CRC(240ebac1) SHA1(83a8264d5efa261580c45112d718f6cd55957aa0) )


	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "mg2-9u27.bin", 0x00000, 0x20000, CRC(01ec6b04) SHA1(2e0e8b0ae8403070cae198555f66c044f48d19f4) )
	ROM_LOAD( "mg210u23.bin", 0x20000, 0x20000, CRC(a85f1104) SHA1(a447f265a683d471dba2f0ef88ca9260089274bf) )
	ROM_LOAD( "mg211u19.bin", 0x40000, 0x20000, CRC(44f21e4d) SHA1(e002004da5dd74ef30dc7ce95f426dd9a47fede5) )
	ROM_LOAD( "mg212u14.bin", 0x60000, 0x20000, CRC(a0ae2b4b) SHA1(5e026ad8a6b2a8120e386471d5178625bda04525) )
ROM_END

ROM_START( multigm3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mg3-6.bin", 0x0000, 0x8000, CRC(3c1e53f1) SHA1(2eaf84e592db58cd268738da2642c716895e4eaa) )

	ROM_REGION( 0x100000, "user1", 0 )
	ROM_LOAD( "mg3-2.bin", 0x00000, 0x20000, CRC(de6d2232) SHA1(30a5e6cd44cfbce2c5186dbc45c0c14c8b1510c4) )
	ROM_LOAD( "mg3-1.bin", 0x40000, 0x20000, CRC(b0dc8136) SHA1(6d7334aae9047ec2028790bd3b326458b5cdc737) )
	ROM_LOAD( "mg3-3.bin", 0x60000, 0x20000, CRC(6e96d642) SHA1(45eb954a0a905ad48ebc04ee3ed8858a1077817a) )
	ROM_LOAD( "mg3-5.bin", 0xa0000, 0x40000, CRC(44609e6f) SHA1(c12a63e34b379427a3c146f1f85688fedf55ed0f) )
	ROM_LOAD( "mg3-4.bin", 0xe0000, 0x20000, CRC(9b022e13) SHA1(96ad9c49cf72fe19a3e9944f8102d2d905266e92) )


	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "mg3-7.bin", 0x00000, 0x20000, CRC(d8308cdb) SHA1(7bfb864611c32cf3740cfd650bfe275513d511d7) )
	ROM_LOAD( "mg3-8.bin", 0x20000, 0x20000, CRC(b4a53b9d) SHA1(63d8901149d0d9b69f28bfc096f7932448542032) )
	ROM_FILL( 0x40000, 0x20000, 0x00 )
	ROM_LOAD( "mg3-9.bin", 0x60000, 0x20000, CRC(a0ae2b4b) SHA1(5e026ad8a6b2a8120e386471d5178625bda04525) )
ROM_END

ROM_START( multigmt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.24", 0x0000, 0x8000, CRC(fdc40548) SHA1(323dcfe04bff4afd7ea290c93ef466690214f2b0) )

	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "1.22",     0x00000, 0x40000, CRC(14d0509e) SHA1(9922b2bae7592f4942f5c1bb32746705d7298eb6) )
	ROM_RELOAD(           0x40000, 0x40000)

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "5.34",     0x00000, 0x40000, CRC(cdf70463) SHA1(79b7c399ebea245a0b06f461e5d8c3291b215878) )
	ROM_RELOAD(           0x40000, 0x40000)

	ROM_REGION( 0x40000, "unknown", 0 )
	ROM_LOAD( "r11.7", 0x00000, 0x20000, CRC(e6fdcabb) SHA1(9ff3adb61b81d5cac4e88520fe4281414d7e5311) )
	ROM_LOAD( "4.27",  0x20000, 0x10000, CRC(e6ed25fe) SHA1(e5723efab5652da3b276b13cd72ca5dc14f37a8b) )
	ROM_LOAD( "10.72", 0x30000, 0x10000, CRC(43f393ee) SHA1(4a307264bedfe77286a6dde47a7f1009a2698ab5) )

ROM_END

ROM_START( supergm3 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sg3.ic36", 0x0000, 0x10000, CRC(f79dd9ef) SHA1(31b84097f4f14bdb6e87a7a624e9974abe680c80) )
	ROM_COPY("maincpu", 0x0000, 0x10000, 0x10000)

	ROM_REGION( 0x400000, "user1", 0 )
	ROM_LOAD( "sg3.rom18", 0x000000, 0x80000, CRC(aa7b8631) SHA1(4d6695da2baef7cc5b0d3f9f620f6b49d8f89e23) )
	ROM_LOAD( "sg3.rom10", 0x080000, 0x80000, CRC(4581454f) SHA1(ce3b43e80e4893a7c9ad3adf7c1edb02e14b351a) )
	ROM_LOAD( "sg3.rom11", 0x100000, 0x80000, CRC(14490668) SHA1(d5076d2466ea974bf6867c3cf3052242b0242ffb) )
	ROM_LOAD( "sg3.rom12", 0x180000, 0x80000, CRC(6ce8781f) SHA1(cf75fccb3d22bbbcfbc7e8b8104eb79353a24c97) )
	ROM_LOAD( "sg3.rom13", 0x300000, 0x40000, CRC(bbc9ab4f) SHA1(4a833b4cc73330461ee519715b56ba0ba2e7ce50) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "sg3.rom14", 0x000000, 0x80000, CRC(197cd696) SHA1(a5a9ee2c5f63b730b9e93d46a1745fabfdc02db2) )
	ROM_LOAD( "sg3.rom15", 0x080000, 0x20000, CRC(e18eff5f) SHA1(d865b5a23a248b7d582fe3ccb3ff46fbbff9a2a9) )
	ROM_LOAD( "sg3.rom16", 0x100000, 0x80000, CRC(151f4ffa) SHA1(8055b9c275f539707aca45f3e57fd4b1fc2c3fc5) )
	ROM_LOAD( "sg3.rom17", 0x180000, 0x80000, CRC(7be7fbb8) SHA1(03cda9c098eaf21326b001d5c227ad85502b6378) )
ROM_END

static DRIVER_INIT( multigam )
{
	multigam_state *state = machine.driver_data<multigam_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	state->multigam_switch_prg_rom(*space, 0x0, 0x01);
}

static void multigm3_decrypt(UINT8* mem, int memsize, const UINT8* decode_nibble)
{
	int i;
	for (i = 0; i < memsize; i++)
	{
		mem[i] = decode_nibble[mem[i] & 0x0f] | (decode_nibble[mem[i] >> 4] << 4);
	}
};

static DRIVER_INIT(multigm3)
{
	multigam_state *state = machine.driver_data<multigam_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	const UINT8 decode[16]  = { 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a };

	multigm3_decrypt(machine.root_device().memregion("maincpu")->base(), machine.root_device().memregion("maincpu")->bytes(), decode );
	multigm3_decrypt(machine.root_device().memregion("user1")->base(), machine.root_device().memregion("user1")->bytes(), decode );

	state->m_multigmc_mmc3_6000_ram = auto_alloc_array(machine, UINT8, 0x2000);

	state->multigam_switch_prg_rom(*space, 0x0, 0x01);
}

static DRIVER_INIT(multigmt)
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	UINT8* buf = auto_alloc_array(machine, UINT8, 0x80000);
	UINT8 *rom;
	int size;
	int i;
	int addr;

	rom = machine.root_device().memregion("maincpu")->base();
	size = 0x8000;
	memcpy(buf, rom, size);
	for (i = 0; i < size; i++)
	{

		addr = BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,8,11,12,10,9,7,6,5,4,3,2,1,0);
		rom[i] = buf[addr];
	}

	rom = machine.root_device().memregion("user1")->base();
	size = 0x80000;
	memcpy(buf, rom, size);
	for (i = 0; i < size; i++)
	{
		addr = BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,8,11,12,10,9,7,6,5,4,3,2,1,0);
		rom[i] = buf[addr];
	}
	rom = machine.root_device().memregion("gfx1")->base();
	size = 0x80000;
	memcpy(buf, rom, size);
	for (i = 0; i < size; i++)
	{
		addr = BITSWAP24(i,23,22,21,20,19,18,17,15,16,11,10,12,13,14,8,9,1,3,5,7,6,4,2,0);
		rom[i] = BITSWAP8(buf[addr], 4, 7, 3, 2, 5, 1, 6, 0);
	}

	auto_free(machine, buf);
	multigam_state *state = machine.driver_data<multigam_state>();
	state->multigam_switch_prg_rom(*space, 0x0, 0x01);
};

GAME( 1992, multigam, 0,        multigam, multigam, multigam, ROT0, "<unknown>", "Multi Game (set 1)", 0 )
GAME( 1992, multigmb, multigam, multigam, multigam, multigam, ROT0, "<unknown>", "Multi Game (set 2)", 0 )
GAME( 1992, multigm2, 0,        multigm3, multigm2, multigm3, ROT0, "Seo Jin",   "Multi Game 2", 0 )
GAME( 1992, multigm3, 0,        multigm3, multigm3, multigm3, ROT0, "Seo Jin",   "Multi Game III", 0 )
GAME( 1992, multigmt, 0,        multigmt, multigmt, multigmt, ROT0, "Tung Sheng Electronics", "Multi Game (Tung Sheng Electronics)", 0 )
GAME( 1996, supergm3, 0,        supergm3, supergm3, 0,        ROT0, "<unknown>", "Super Game III", 0 )
