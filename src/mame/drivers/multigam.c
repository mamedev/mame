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

    Multi Game III: 21 games included, hardware features MMC3 NES mapper and additional
    RAM used by Super Mario Bros 3.
*/

#include "driver.h"
#include "deprecat.h"
#include "video/ppu2c0x.h"
#include "sound/nes_apu.h"
#include "cpu/m6502/m6502.h"

/******************************************************

   NES interface

*******************************************************/

static WRITE8_HANDLER( sprite_dma_w )
{
	int source = ( data & 7 );
	ppu2c0x_spriteram_dma( 0, source );
}

static READ8_HANDLER( psg_4015_r )
{
	return nes_psg_0_r(space, 0x15);
}

static WRITE8_HANDLER( psg_4015_w )
{
	nes_psg_0_w(space, 0x15, data);
}

static WRITE8_HANDLER( psg_4017_w )
{
	nes_psg_0_w(space, 0x17, data);
}

/******************************************************

   Inputs

*******************************************************/

static UINT32 in_0;
static UINT32 in_1;
static UINT32 in_0_shift;
static UINT32 in_1_shift;
static UINT32 multigam_in_dsw;
static UINT32 multigam_in_dsw_shift;

static READ8_HANDLER( multigam_IN0_r )
{
	return ((in_0 >> in_0_shift++) & 0x01) | 0x40;
}

static WRITE8_HANDLER( multigam_IN0_w )
{
	if ( data & 0x01 )
	{
		return;
	}

	in_0_shift = 0;
	in_1_shift = 0;

	in_0 = input_port_read(space->machine, "P1");
	in_1 = input_port_read(space->machine, "P2");

	multigam_in_dsw_shift = 0;
	multigam_in_dsw = input_port_read(space->machine, "DSW");
}

static READ8_HANDLER( multigam_IN1_r )
{
	return ((in_1 >> in_1_shift++) & 0x01) | 0x40;
}

static CUSTOM_INPUT( multigam_inputs_r )
{
	/* bit 0: serial input (dsw)
       bit 1: coin */
	return (multigam_in_dsw >> multigam_in_dsw_shift++) & 0x01;
}


/******************************************************

   ROM/VROM banking (Multi Game)

*******************************************************/

static int multigam_game_gfx_bank = 0;

static WRITE8_HANDLER(multigam_switch_prg_rom)
{
	/* switch PRG rom */
	UINT8* dst = memory_region( space->machine, "main" );
	UINT8* src = memory_region( space->machine, "user1" );

	if ( data & 0x80 )
	{
		if ( data & 0x01 )
		{
			data &= ~0x01;
		}
		memcpy( &dst[0x8000], &src[(data & 0x7f) * 0x4000], 0x8000 );
	}
	else
	{
		memcpy( &dst[0x8000], &src[data*0x4000], 0x4000 );
		memcpy( &dst[0xc000], &src[data*0x4000], 0x4000 );
	}
};

static WRITE8_HANDLER(multigam_switch_gfx_rom)
{
	ppu2c0x_set_videorom_bank( 0, 0, 8, data /*& 0x3f*/, 512 );
	ppu2c0x_set_mirroring( 0, data & 0x40 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT );
	multigam_game_gfx_bank = data;
};


static WRITE8_HANDLER(multigam_mapper2_w)
{
	if ( multigam_game_gfx_bank & 0x80 )
	{
		ppu2c0x_set_videorom_bank( 0, 0, 8, multigam_game_gfx_bank + (data & 0xf), 512 );
	}
	else
	{
		logerror( "Unmapped multigam_mapper2_w: offset = %04X, data = %02X\n", offset, data );
	}
}

/******************************************************

   Memory map (Multi Game)

*******************************************************/

static ADDRESS_MAP_START( multigam_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM	/* NES RAM */
	AM_RANGE(0x0800, 0x0fff) AM_RAM /* additional RAM */
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(ppu2c0x_0_r, ppu2c0x_0_w)
	AM_RANGE(0x4000, 0x4013) AM_READWRITE(nes_psg_0_r, nes_psg_0_w)			/* PSG primary registers */
	AM_RANGE(0x4014, 0x4014) AM_WRITE(sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_READWRITE(psg_4015_r, psg_4015_w)			/* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(multigam_IN0_r, multigam_IN0_w)	/* IN0 - input port 1 */
	AM_RANGE(0x4017, 0x4017) AM_READWRITE(multigam_IN1_r, psg_4017_w)		/* IN1 - input port 2 / PSG second control register */
	AM_RANGE(0x5002, 0x5002) AM_WRITENOP
	AM_RANGE(0x5000, 0x5ffe) AM_ROM
	AM_RANGE(0x5fff, 0x5fff) AM_READ_PORT("IN0")
	AM_RANGE(0x6000, 0x7fff) AM_ROM
	AM_RANGE(0x6fff, 0x6fff) AM_WRITE(multigam_switch_prg_rom)
	AM_RANGE(0x7fff, 0x7fff) AM_WRITE(multigam_switch_gfx_rom)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_WRITE(multigam_mapper2_w)
ADDRESS_MAP_END

/******************************************************

   MMC3 (Multi Game III)

*******************************************************/

static int multigam3_mmc3_scanline_counter;
static int multigam3_mmc3_scanline_latch;
static int multigam3_mmc3_banks[2];
static int multigam3_mmc3_4screen;
static int multigam3_mmc3_last_bank;
static UINT8* multigmc_mmc3_6000_ram;

static void multigam3_mmc3_scanline_cb( int num, int scanline, int vblank, int blanked )
{
	if ( !vblank && !blanked )
	{
		if ( --multigam3_mmc3_scanline_counter == -1 )
		{
			multigam3_mmc3_scanline_counter = multigam3_mmc3_scanline_latch;
			cpu_set_input_line(Machine->cpu[0], 0, PULSE_LINE );
		}
	}
}

static WRITE8_HANDLER( multigam3_mmc3_rom_switch_w )
{
	/* basically, a MMC3 mapper from the nes */
	static int multigam3_mmc3_command;

	switch( offset & 0x7001 )
	{
		case 0x0000:
			multigam3_mmc3_command = data;

			if ( multigam3_mmc3_last_bank != ( data & 0xc0 ) )
			{
				int bank;
				UINT8 *prg = memory_region( space->machine, "main" );

				/* reset the banks */
				if ( multigam3_mmc3_command & 0x40 )
				{
					/* high bank */
					bank = multigam3_mmc3_banks[0] * 0x2000 + 0xa0000;

					memcpy( &prg[0x0c000], &prg[bank], 0x2000 );
					memcpy( &prg[0x08000], &prg[0xa0000 + 0x3c000], 0x2000 );
				}
				else
				{
					/* low bank */
					bank = multigam3_mmc3_banks[0] * 0x2000 + 0xa0000;

					memcpy( &prg[0x08000], &prg[bank], 0x2000 );
					memcpy( &prg[0x0c000], &prg[0xa0000 + 0x3c000], 0x2000 );
				}

				/* mid bank */
				bank = multigam3_mmc3_banks[1] * 0x2000 + 0xa0000;
				memcpy( &prg[0x0a000], &prg[bank], 0x2000 );

				multigam3_mmc3_last_bank = data & 0xc0;
			}
		break;

		case 0x0001:
			{
				UINT8 cmd = multigam3_mmc3_command & 0x07;
				int page = ( multigam3_mmc3_command & 0x80 ) >> 5;
				int bank;

				switch( cmd )
				{
					case 0:	/* char banking */
					case 1: /* char banking */
						data &= 0xfe;
						page ^= ( cmd << 1 );
						ppu2c0x_set_videorom_bank( 0, page, 2, 0x100 + data, 64 );
					break;

					case 2: /* char banking */
					case 3: /* char banking */
					case 4: /* char banking */
					case 5: /* char banking */
						page ^= cmd + 2;
						ppu2c0x_set_videorom_bank( 0, page, 1, 0x100 + data, 64 );
					break;

					case 6: /* program banking */
					{
						UINT8 *prg = memory_region( space->machine, "main" );
						if ( multigam3_mmc3_command & 0x40 )
						{
							/* high bank */
							multigam3_mmc3_banks[0] = data & 0x1f;
							bank = ( multigam3_mmc3_banks[0] ) * 0x2000 + 0xa0000;

							memcpy( &prg[0x0c000], &prg[bank], 0x2000 );
							memcpy( &prg[0x08000], &prg[0xa0000 + 0x3c000], 0x2000 );
						}
						else
						{
							/* low bank */
							multigam3_mmc3_banks[0] = data & 0x1f;
							bank = ( multigam3_mmc3_banks[0] ) * 0x2000 + 0xa0000;

							memcpy( &prg[0x08000], &prg[bank], 0x2000 );
							memcpy( &prg[0x0c000], &prg[0xa0000 + 0x3c000], 0x2000 );
						}
					}
					break;

					case 7: /* program banking */
						{
							/* mid bank */
							UINT8 *prg = memory_region( space->machine, "main" );
							multigam3_mmc3_banks[1] = data & 0x1f;
							bank = multigam3_mmc3_banks[1] * 0x2000 + 0xa0000;

							memcpy( &prg[0x0a000], &prg[bank], 0x2000 );
						}
					break;
				}
			}
		break;

		case 0x2000: /* mirroring */
			if( !multigam3_mmc3_4screen )
			{
				if ( data & 0x40 )
					ppu2c0x_set_mirroring( 0, PPU_MIRROR_HIGH );
				else
					ppu2c0x_set_mirroring( 0, ( data & 1 ) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT );
			}
		break;

		case 0x2001: /* enable ram at $6000 */
			/* ignored - we always enable it */
		break;

		case 0x4000: /* scanline counter */
			multigam3_mmc3_scanline_counter = data;
		break;

		case 0x4001: /* scanline latch */
			multigam3_mmc3_scanline_latch = data;
		break;

		case 0x6000: /* disable irqs */
			ppu2c0x_set_scanline_callback( 0, 0 );
		break;

		case 0x6001: /* enable irqs */
			ppu2c0x_set_scanline_callback( 0, multigam3_mmc3_scanline_cb );
		break;
	}
}

static void multigam_init_smb3(running_machine *machine)
{
	UINT8* dst = memory_region( machine, "main" );
	UINT8* src = memory_region( machine, "user1" );

	memcpy(&dst[0x8000], &src[0xa0000 + 0x3c000], 0x4000);
	memcpy(&dst[0xc000], &src[0xa0000 + 0x3c000], 0x4000);

	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, multigam3_mmc3_rom_switch_w );

	memory_set_bankptr(1, multigmc_mmc3_6000_ram);

	multigam3_mmc3_banks[0] = 0x1e;
	multigam3_mmc3_banks[1] = 0x1f;
	multigam3_mmc3_scanline_counter = 0;
	multigam3_mmc3_scanline_latch = 0;
	multigam3_mmc3_4screen = 0;
	multigam3_mmc3_last_bank = 0xff;

};

static WRITE8_HANDLER(multigm3_mapper2_w)
{
	if ( multigam_game_gfx_bank & 0x80 )
	{
		ppu2c0x_set_videorom_bank( 0, 0, 8, (multigam_game_gfx_bank & 0xfc)  + (data & 0x3), 512 );
	}
	else
	{
		logerror( "Unmapped multigam_mapper2_w: offset = %04X, data = %02X\n", offset, data );
	}
};

static WRITE8_HANDLER(multigm3_switch_prg_rom)
{
	/* switch PRG rom */
	UINT8* dst = memory_region( space->machine, "main" );
	UINT8* src = memory_region( space->machine, "user1" );

	if ( data == 0xa8 )
	{
		multigam_init_smb3(space->machine);
		return;
	}
	else
	{
		memory_install_write8_handler(space->machine, 0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, multigm3_mapper2_w );
		memory_set_bankptr(1, memory_region(space->machine, "main") + 0x6000);
	}

	if ( data & 0x80 )
	{
		if ( data & 0x01 )
		{
			data &= ~0x01;
		}
		memcpy( &dst[0x8000], &src[(data & 0x7f) * 0x4000], 0x8000 );
	}
	else
	{
		memcpy( &dst[0x8000], &src[data*0x4000], 0x4000 );
		memcpy( &dst[0xc000], &src[data*0x4000], 0x4000 );
	}
};

/******************************************************

   Memory map (Multi Game III)

*******************************************************/

static ADDRESS_MAP_START( multigm3_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM	/* NES RAM */
	AM_RANGE(0x0800, 0x0fff) AM_RAM /* additional RAM */
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(ppu2c0x_0_r, ppu2c0x_0_w)
	AM_RANGE(0x4000, 0x4013) AM_READWRITE(nes_psg_0_r, nes_psg_0_w)			/* PSG primary registers */
	AM_RANGE(0x4014, 0x4014) AM_WRITE(sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_READWRITE(psg_4015_r, psg_4015_w)			/* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(multigam_IN0_r, multigam_IN0_w)	/* IN0 - input port 1 */
	AM_RANGE(0x4017, 0x4017) AM_READWRITE(multigam_IN1_r, psg_4017_w)		/* IN1 - input port 2 / PSG second control register */
	AM_RANGE(0x5001, 0x5001) AM_WRITE(multigm3_switch_prg_rom)
	AM_RANGE(0x5002, 0x5002) AM_WRITENOP
	AM_RANGE(0x5003, 0x5003) AM_WRITE(multigam_switch_gfx_rom)
	AM_RANGE(0x5000, 0x5ffe) AM_ROM
	AM_RANGE(0x5fff, 0x5fff) AM_READ_PORT("IN0")
	AM_RANGE(0x6000, 0x7fff) AM_RAMBANK(1)
	AM_RANGE(0x6fff, 0x6fff) AM_WRITENOP /* 0x00 in attract mode, 0xff during play */
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_WRITE(multigm3_mapper2_w)
ADDRESS_MAP_END

/******************************************************

   Input ports

*******************************************************/

static INPUT_PORTS_START( multigam_common )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	/* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)	/* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(multigam_inputs_r, NULL)
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

static INPUT_PORTS_START( multigm3 )
	PORT_INCLUDE( multigam_common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x06, 0x00, "Coin/Time" )
	PORT_DIPSETTING(    0x00, "15 min" )
	PORT_DIPSETTING(    0x04, "8 min" )
	PORT_DIPSETTING(    0x02, "11 min" )
	PORT_DIPSETTING(    0x06, "5 min" )
INPUT_PORTS_END


/******************************************************

   PPU/Video interface

*******************************************************/

static const nes_interface multigam_interface_1 =
{
	"main"
};

static PALETTE_INIT( multigam )
{
	ppu2c0x_init_palette(machine, 0 );
}

static void ppu_irq( int num, int *ppu_regs )
{
	cpu_set_input_line(Machine->cpu[num], INPUT_LINE_NMI, PULSE_LINE );
}

/* our ppu interface                                            */
static const ppu2c0x_interface ppu_interface =
{
	PPU_2C04,				/* type */
	1,						/* num */
	{ "gfx1" },				/* vrom gfx region */
	{ 0 },					/* gfxlayout num */
	{ 0 },					/* color base */
	{ PPU_MIRROR_NONE },	/* mirroring */
	{ ppu_irq }				/* irq */
};

static VIDEO_START( multigam )
{
	ppu2c0x_init(machine, &ppu_interface );
}

static VIDEO_UPDATE( multigam )
{
	/* render the ppu */
	ppu2c0x_render( 0, bitmap, 0, 0, 0, 0 );
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
	/* reset the ppu */
	ppu2c0x_reset( machine, 0, 1 );
}

static MACHINE_RESET( multigm3 )
{
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	/* reset the ppu */
	ppu2c0x_reset( machine, 0, 1 );
	multigm3_switch_prg_rom(space, 0, 0x01 );
};


static MACHINE_DRIVER_START( multigam )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", N2A03, N2A03_DEFAULTCLOCK)
	MDRV_CPU_PROGRAM_MAP(multigam_map, 0)

	MDRV_MACHINE_RESET( multigam )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 262)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(multigam)
	MDRV_PALETTE_LENGTH(8*4*16)

	MDRV_PALETTE_INIT(multigam)
	MDRV_VIDEO_START(multigam)
	MDRV_VIDEO_UPDATE(multigam)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("nes", NES, N2A03_DEFAULTCLOCK)
	MDRV_SOUND_CONFIG(multigam_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( multigm3 )
	MDRV_IMPORT_FROM(multigam)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(multigm3_map, 0)

	MDRV_MACHINE_RESET( multigm3 )
MACHINE_DRIVER_END

ROM_START( multigam )
	ROM_REGION( 0x10000, "main", 0 )
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
	ROM_REGION( 0x10000, "main", 0 )
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

ROM_START( multigm3 )
	ROM_REGION( 0x10000, "main", 0 )
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
	ROM_LOAD( "mg3-9.bin", 0x40000, 0x20000, CRC(a0ae2b4b) SHA1(5e026ad8a6b2a8120e386471d5178625bda04525) )
	ROM_FILL( 0x60000, 0x20000, 0x00 )
ROM_END

static DRIVER_INIT( multigam )
{
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	multigam_switch_prg_rom( space, 0x0, 0x01 );
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
	const UINT8 decode[16]  = { 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a };

	multigm3_decrypt(memory_region(machine, "main"), memory_region_length(machine, "main"), decode );
	multigm3_decrypt(memory_region(machine, "user1"), memory_region_length(machine, "user1"), decode );

	multigmc_mmc3_6000_ram = auto_malloc(0x2000);
}

GAME( 1992, multigam, 0,        multigam, multigam, multigam, ROT0, "unknown", "Multi Game (set 1)", 0 )
GAME( 1992, multigmb, multigam, multigam, multigam, multigam, ROT0, "unknown", "Multi Game (set 2)", 0 )
GAME( 19??, multigm3, 0,        multigm3, multigm3, multigm3, ROT0, "Seo Jin", "Multi Game III", 0 )
