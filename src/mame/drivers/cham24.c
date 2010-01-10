/*
    Chameleon 24

    driver by Mariusz Wojcieszek
    uses NES emulaton by Brad Olivier

    Notes:
    - NES hardware is probably implemented on FPGA
    - Atmel mcu probably controls coins and timer - since these are not emulated
      game is marked as 'not working'
    - 72-in-1 mapper (found on NES pirate carts) is used for bank switching
    - code at 0x0f8000 in 24-2.u2 contains English version of menu, code at 0x0fc000 contains
    other version (Asian language), is this controlled by mcu?

PCB is small and newly manufactured. There's 24 games which can be chosen
from a text menu after coin-up.
The games appear to be old NES games (i.e. very poor quality for an arcade product)
Screenshots can be found here....
http://www.coinopexpress.com/products/pcbs/pcb/Chameleon_24_2839.html

PCB Layout
----------


|------------------------------------|
|       LM380    --------            |
|                |NTA0002|           |
|                |(QFP80)|   24-1.U1 |
|                --------            |
|   2003        -----------          |
|              |LATTICE  |           |
|      DSW1    |PLSI 1016|           |
|J             |(PLCC44) |  24-2.U2  |
|A    AT89C51  -----------           |
|M                                   |
|M    SW1   21.4771MHz               |
|A                                   |
| GW6582  LS02                       |
|          |-----------| 4040        |
|  74HC245 |Phillps    | 4040        |
|          |SAA71111AH2|             |
|          |20505650   |             |
|          |bP0219     | 24-3.U3     |
| 24.576MHz|-----------|             |
|             (QFP64)                |
|------------------------------------|

Notes:
       All components are listed.
       DSW1 has 2 switches only
       SW1 is a push button switch
       U1 is 27C040 EPROM
       U2 is 27C080 EPROM
       U3 is 27C512 EPROM
*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/dac.h"
#include "sound/nes_apu.h"
#include "video/ppu2c0x.h"

static UINT8* nt_ram;
static UINT8* nt_page[4];

static void cham24_set_mirroring( int mirroring )
{
	switch(mirroring)
	{
	case PPU_MIRROR_LOW:
		nt_page[0] = nt_page[1] = nt_page[2] = nt_page[3] = nt_ram;
		break;
	case PPU_MIRROR_HIGH:
		nt_page[0] = nt_page[1] = nt_page[2] = nt_page[3] = nt_ram + 0x400;
		break;
	case PPU_MIRROR_HORZ:
		nt_page[0] = nt_ram;
		nt_page[1] = nt_ram;
		nt_page[2] = nt_ram + 0x400;
		nt_page[3] = nt_ram + 0x400;
		break;
	case PPU_MIRROR_VERT:
		nt_page[0] = nt_ram;
		nt_page[1] = nt_ram + 0x400;
		nt_page[2] = nt_ram;
		nt_page[3] = nt_ram + 0x400;
		break;
	case PPU_MIRROR_NONE:
	default:
		nt_page[0] = nt_ram;
		nt_page[1] = nt_ram + 0x400;
		nt_page[2] = nt_ram + 0x800;
		nt_page[3] = nt_ram + 0xc00;
		break;
	}
}

static WRITE8_HANDLER( nt_w )
{
	int page = ((offset & 0xc00) >> 10);
	nt_page[page][offset & 0x3ff] = data;
}

static READ8_HANDLER( nt_r )
{
	int page = ((offset & 0xc00) >> 10);
	return nt_page[page][offset & 0x3ff];

}

static WRITE8_HANDLER( sprite_dma_w )
{
	int source = (data & 7);
	ppu2c0x_spriteram_dma(space, devtag_get_device(space->machine, "ppu"), source);
}

static READ8_DEVICE_HANDLER( psg_4015_r )
{
	return nes_psg_r(device,0x15);
}

static WRITE8_DEVICE_HANDLER( psg_4015_w )
{
	nes_psg_w(device,0x15, data);
}

static WRITE8_DEVICE_HANDLER( psg_4017_w )
{
	nes_psg_w(device,0x17, data);
}

static UINT32 in_0;
static UINT32 in_1;
static UINT32 in_0_shift;
static UINT32 in_1_shift;

static READ8_HANDLER( cham24_IN0_r )
{
	return ((in_0 >> in_0_shift++) & 0x01) | 0x40;
}

static WRITE8_HANDLER( cham24_IN0_w )
{
	if (data & 0xfe)
	{
		//logerror("Unhandled cham24_IN0_w write: data = %02X\n", data);
	}

	if (data & 0x01)
	{
		return;
	}

	in_0_shift = 0;
	in_1_shift = 0;

	in_0 = input_port_read(space->machine, "P1");
	in_1 = input_port_read(space->machine, "P2");

}

static READ8_HANDLER( cham24_IN1_r )
{
	return ((in_1 >> in_1_shift++) & 0x01) | 0x40;
}

static WRITE8_HANDLER( cham24_mapper_w )
{
	UINT32 gfx_bank = offset & 0x3f;
	UINT32 prg_16k_bank_page = (offset >> 6) & 0x01;
	UINT32 prg_bank = (offset >> 7) & 0x1f;
	UINT32 prg_bank_page_size = (offset >> 12) & 0x01;
	UINT32 gfx_mirroring = (offset >> 13) & 0x01;

	UINT8* dst = memory_region(space->machine, "maincpu");
	UINT8* src = memory_region(space->machine, "user1");

	// switch PPU VROM bank
	memory_set_bankptr(space->machine, "bank1", memory_region(space->machine, "gfx1") + (0x2000 * gfx_bank));

	// set gfx mirroring
	cham24_set_mirroring(gfx_mirroring != 0 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	// switch PRG bank
	if (prg_bank_page_size == 0)
	{
		// 32K
		memcpy(&dst[0x8000], &src[prg_bank * 0x8000], 0x8000);
	}
	else
	{
		if (prg_16k_bank_page == 1)
		{
			// upper half of 32K page
			memcpy(&dst[0x8000], &src[(prg_bank * 0x8000) + 0x4000], 0x4000);
			memcpy(&dst[0xC000], &src[(prg_bank * 0x8000) + 0x4000], 0x4000);
		}
		else
		{
			// lower half of 32K page
			memcpy(&dst[0x8000], &src[(prg_bank * 0x8000)], 0x4000);
			memcpy(&dst[0xC000], &src[(prg_bank * 0x8000)], 0x4000);
		}
	}
}

static ADDRESS_MAP_START( cham24_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM	/* NES RAM */
	AM_RANGE(0x2000, 0x3fff) AM_DEVREADWRITE("ppu", ppu2c0x_r, ppu2c0x_w)
	AM_RANGE(0x4000, 0x4013) AM_DEVREADWRITE("nes", nes_psg_r, nes_psg_w)			/* PSG primary registers */
	AM_RANGE(0x4014, 0x4014) AM_WRITE(sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_DEVREADWRITE("nes", psg_4015_r, psg_4015_w)			/* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(cham24_IN0_r,        cham24_IN0_w)			/* IN0 - input port 1 */
	AM_RANGE(0x4017, 0x4017) AM_READ(cham24_IN1_r) AM_DEVWRITE("nes", psg_4017_w)		/* IN1 - input port 2 / PSG second control register */
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_WRITE(cham24_mapper_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( cham24 )
	PORT_START("P1") /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	/* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2") /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)	/* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END

static const nes_interface cham24_interface_1 =
{
	"maincpu"
};

static MACHINE_RESET( cham24 )
{
}

static PALETTE_INIT( cham24 )
{
	ppu2c0x_init_palette(machine, 0);
}

static void ppu_irq( const device_config *device, int *ppu_regs )
{
	cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);
}

/* our ppu interface                                            */
static const ppu2c0x_interface ppu_interface =
{
	0,					/* gfxlayout num */
	0,					/* color base */
	PPU_MIRROR_NONE,	/* mirroring */
	ppu_irq				/* irq */
};

static VIDEO_START( cham24 )
{
}

static VIDEO_UPDATE( cham24 )
{
	/* render the ppu */
	ppu2c0x_render(devtag_get_device(screen->machine, "ppu"), bitmap, 0, 0, 0, 0);
	return 0;
}


static MACHINE_START( cham24 )
{
	/* switch PRG rom */
	UINT8* dst = memory_region(machine, "maincpu");
	UINT8* src = memory_region(machine, "user1");

	memcpy(&dst[0x8000], &src[0x0f8000], 0x4000);
	memcpy(&dst[0xc000], &src[0x0f8000], 0x4000);

	/* uses 8K swapping, all ROM!*/
	memory_install_read_bank(cpu_get_address_space(devtag_get_device(machine, "ppu"), ADDRESS_SPACE_PROGRAM), 0x0000, 0x1fff, 0, 0, "bank1");
	memory_set_bankptr(machine, "bank1", memory_region(machine, "gfx1"));

	/* need nametable ram, though. I doubt this uses more than 2k, but it starts up configured for 4 */
	nt_ram = auto_alloc_array(machine, UINT8, 0x1000);
	nt_page[0] = nt_ram;
	nt_page[1] = nt_ram + 0x400;
	nt_page[2] = nt_ram + 0x800;
	nt_page[3] = nt_ram + 0xc00;

	/* and read/write handlers */
	memory_install_readwrite8_handler(cpu_get_address_space(devtag_get_device(machine, "ppu"), ADDRESS_SPACE_PROGRAM), 0x2000, 0x3eff, 0, 0, nt_r, nt_w);
}

static DRIVER_INIT( cham24 )
{
}

static GFXDECODE_START( cham24 )
	/* none, the ppu generates one */
GFXDECODE_END

static MACHINE_DRIVER_START( cham24 )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", N2A03, N2A03_DEFAULTCLOCK)
	MDRV_CPU_PROGRAM_MAP(cham24_map)

	MDRV_MACHINE_RESET( cham24 )
	MDRV_MACHINE_START( cham24 )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 262)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(cham24)
	MDRV_PALETTE_LENGTH(8*4*16)

	MDRV_PALETTE_INIT(cham24)
	MDRV_VIDEO_START(cham24)
	MDRV_VIDEO_UPDATE(cham24)

	MDRV_PPU2C04_ADD("ppu", ppu_interface)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("nes", NES, N2A03_DEFAULTCLOCK)
	MDRV_SOUND_CONFIG(cham24_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

ROM_START( cham24 )
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASE00)

	ROM_REGION(0x100000, "user1", 0)
	ROM_LOAD( "24-2.u2", 0x000000, 0x100000, CRC(686e9d05) SHA1(a55b9850a4b47f1b4495710e71534ca0287b05ee) )

	ROM_REGION(0x080000, "gfx1", 0)
	ROM_LOAD( "24-1.u1", 0x000000, 0x080000, CRC(43c43d58) SHA1(3171befaca28acc80fb70226748d9abde76a1b56) )

	ROM_REGION(0x10000, "user2", 0)
	ROM_LOAD( "24-3.u3", 0x0000, 0x10000, CRC(e97955fa) SHA1(6d686c5d0967c9c2f40dbd8e6a0c0907606f2c7d) ) // unknown rom
ROM_END

GAME( 2002, cham24, 0, cham24, cham24, cham24, ROT0, "bootleg", "Chameleon 24", GAME_NOT_WORKING )
