/*

Penguin Adventure bootleg

Driver by Mariusz Wojcieszek

This seems to be the MSX version possibly hacked
to run on cheap Korean bootleg hardware.

Basic components include.....
Z80 @ 3.579533MHz [10.7386/3]
TMS9128 @ 10.7386MHz
AY-3-8910 @ 1.789766MHz [10.7386/6]
8255
4416 RAM x2
4164 RAM x8
10.7386 XTAL
10 position DIPSW
NOTE! switches 1, 3 & 5 must be ON or the game will not boot.
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
#include "machine/i8255a.h"

static UINT8 *main_mem = NULL;
static UINT8 mem_map = 0;
static UINT8 mem_banks[4];

static void mem_map_banks(running_machine *machine)
{
	int slot_select;

	// page 0
	slot_select = (mem_map >> 0) & 0x03;
	switch(slot_select)
	{
		case 0:
		{
			memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0000, 0x3fff, 0, 0, "bank1" );
			memory_set_bankptr(machine, "bank1", memory_region(machine, "maincpu"));
			break;
		};
		case 1:
		case 2:
		case 3:
		{
			memory_unmap_read(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0000, 0x3fff, 0, 0);
			break;
		}
	}

	// page 1
	slot_select = (mem_map >> 2) & 0x03;
	switch(slot_select)
	{
		case 0:
		{
			memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4000, 0x5fff, 0, 0, "bank21" );
			memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6000, 0x7fff, 0, 0, "bank22" );
			memory_set_bankptr(machine, "bank21", memory_region(machine, "maincpu") + 0x4000);
			memory_set_bankptr(machine, "bank22", memory_region(machine, "maincpu") + 0x4000 + 0x2000);
			break;
		}
		case 1:
		{
			memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4000, 0x5fff, 0, 0, "bank21" );
			memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6000, 0x7fff, 0, 0, "bank22" );
			memory_set_bankptr(machine, "bank21", memory_region(machine, "game") + mem_banks[0]*0x2000);
			memory_set_bankptr(machine, "bank22", memory_region(machine, "game") + mem_banks[1]*0x2000);
			break;
		}
		case 2:
		case 3:
		{
			memory_unmap_read(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4000, 0x7fff, 0, 0);
			break;
		}
	}

	// page 2
	slot_select = (mem_map >> 4) & 0x03;
	switch(slot_select)
	{
		case 1:
		{
			memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x8000, 0x9fff, 0, 0, "bank31" );
			memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa000, 0xbfff, 0, 0, "bank32" );
			memory_set_bankptr(machine, "bank31", memory_region(machine, "game") + mem_banks[2]*0x2000);
			memory_set_bankptr(machine, "bank32", memory_region(machine, "game") + mem_banks[3]*0x2000);
			break;
		}
		case 0:
		case 2:
		case 3:
		{
			memory_unmap_read(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x8000, 0xbfff, 0, 0);
			break;
		}
	}

	// page 3
	slot_select = (mem_map >> 6) & 0x03;

	switch(slot_select)
	{
		case 0:
		case 1:
		case 2:
		{
			memory_unmap_read(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc000, 0xffff, 0, 0);
			break;
		}
		case 3:
		{
			memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc000, 0xffff, 0, 0, "bank4" );
			memory_set_bankptr(machine, "bank4", main_mem);
			break;
		}
	}

}

static WRITE8_HANDLER(mem_w)
{
	if (offset >= 0xc000)
	{
		int slot_select = (mem_map >> 6) & 0x03;

		if ( slot_select == 3 )
		{
			main_mem[offset - 0xc000] = data;
		}
	}
	else
	{
		switch(offset)
		{
			case 0x4000: mem_banks[0] = data; mem_map_banks(space->machine); break;
			case 0x6000: mem_banks[1] = data; mem_map_banks(space->machine); break;
			case 0x8000: mem_banks[2] = data; mem_map_banks(space->machine); break;
			case 0xa000: mem_banks[3] = data; mem_map_banks(space->machine); break;
		}
	}
}


static ADDRESS_MAP_START( program_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank21")
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank22")
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank31")
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank32")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank4")
	AM_RANGE(0x0000, 0xffff) AM_WRITE(mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_mem, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x98, 0x98) AM_READWRITE( TMS9928A_vram_r, TMS9928A_vram_w )
	AM_RANGE(0x99, 0x99) AM_READWRITE( TMS9928A_register_r, TMS9928A_register_w )
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xa2, 0xa2) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0xa8, 0xab) AM_DEVREADWRITE("ppi8255", i8255a_r, i8255a_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( pengadvb )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(1)
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


static READ8_DEVICE_HANDLER( pengadvb_psg_port_a_r )
{
	return input_port_read(device->machine, "IN0");
}

static const ay8910_interface pengadvb_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(pengadvb_psg_port_a_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static WRITE8_DEVICE_HANDLER ( pengadvb_ppi_port_a_w )
{
	mem_map = data;
	mem_map_banks(device->machine);
}

static READ8_DEVICE_HANDLER( pengadvb_ppi_port_b_r )
{
	if ((i8255a_r(device, 2) & 0x0f) == 0)
		return input_port_read(device->machine, "IN1");

	return 0xff;
}

static I8255A_INTERFACE(pengadvb_ppi8255_interface)
{
	DEVCB_NULL,
	DEVCB_HANDLER(pengadvb_ppi_port_b_r),
	DEVCB_NULL,
	DEVCB_HANDLER(pengadvb_ppi_port_a_w),
	DEVCB_NULL,
	DEVCB_NULL
};

static void vdp_interrupt(running_machine *machine, int i)
{
	cputag_set_input_line(machine, "maincpu", 0, (i ? HOLD_LINE : CLEAR_LINE));
}

static const TMS9928a_interface tms9928a_interface =
{
	TMS99x8A,
	0x4000,
	0, 0,
	vdp_interrupt
};

static STATE_POSTLOAD ( pengadvb )
{
	TMS9928A_post_load(machine);
	mem_map_banks(machine);
}

static MACHINE_START( pengadvb )
{
	TMS9928A_configure(&tms9928a_interface);

	state_save_register_global_pointer(machine, main_mem, 0x4000);
	state_save_register_global(machine, mem_map);
	state_save_register_global_array(machine, mem_banks);
	state_save_register_postload(machine, pengadvb, NULL);
}

static MACHINE_RESET( pengadvb )
{
	TMS9928A_reset();

	mem_map = 0;
	mem_banks[0] = mem_banks[1] = mem_banks[2] = mem_banks[3] = 0;
	mem_map_banks(machine);
}

static INTERRUPT_GEN( pengadvb_interrupt )
{
	TMS9928A_interrupt(device->machine);
}


static MACHINE_CONFIG_START( pengadvb, driver_device )

	MDRV_CPU_ADD("maincpu", Z80, XTAL_10_738635MHz/3)		  /* 3.579545 Mhz */
	MDRV_CPU_PROGRAM_MAP(program_mem)
	MDRV_CPU_IO_MAP(io_mem)
	MDRV_CPU_VBLANK_INT("screen",pengadvb_interrupt)

	MDRV_MACHINE_START( pengadvb )
	MDRV_MACHINE_RESET( pengadvb )

    MDRV_I8255A_ADD( "ppi8255", pengadvb_ppi8255_interface)

	/* video hardware */
	MDRV_FRAGMENT_ADD(tms9928a)
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_REFRESH_RATE((float)XTAL_10_738635MHz/2/342/262)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(4395)) /* 69 lines */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, (float)XTAL_10_738635MHz/6)
	MDRV_SOUND_CONFIG(pengadvb_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static void pengadvb_decrypt(running_machine *machine, const char* region)
{
	UINT8 *mem = memory_region(machine, region);
	int memsize = memory_region_length(machine, region);
	UINT8 *buf;
	int i;

	// data lines swap
	for ( i = 0; i < memsize; i++ )
	{
		mem[i] = BITSWAP8(mem[i],7,6,5,3,4,2,1,0);
	}

	// address line swap
	buf = auto_alloc_array(machine, UINT8, memsize);
	memcpy(buf, mem, memsize);
	for ( i = 0; i < memsize; i++ )
	{
		mem[i] = buf[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,5,11,10,9,8,7,6,12,4,3,2,1,0)];
	}
	auto_free(machine, buf);

}

static DRIVER_INIT(pengadvb)
{
	pengadvb_decrypt(machine, "maincpu");
	pengadvb_decrypt(machine, "game");

	main_mem = auto_alloc_array(machine, UINT8, 0x4000);
}

ROM_START( pengadvb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rom.u5", 0x00000, 0x8000, CRC(d21950d2) SHA1(0b1815677f17a680ba63c3839bea2d451813eec8) )

	ROM_REGION( 0x20000, "game", 0 )
	ROM_LOAD( "rom.u7",  0x00000, 0x8000, CRC(d4b4a4a4) SHA1(59f9299182fd8aedc7a4e9b0ddd685f2a71c033f) )
	ROM_LOAD( "rom.u8",  0x08000, 0x8000, CRC(eada2232) SHA1(f4182f0921b621acd8be6077eb9639b31a97e907) )
	ROM_LOAD( "rom.u9",  0x10000, 0x8000, CRC(6478c561) SHA1(6f9a794a5bb51e96552f6d1e9fa6515659d25933) )
	ROM_LOAD( "rom.u10", 0x18000, 0x8000, CRC(5c48360f) SHA1(0866e20969f57b7b7c59df8f7ca203f18c7c9870) )

ROM_END

GAME( 1988, pengadvb, 0, pengadvb, pengadvb, pengadvb, ROT0, "bootleg / Konami", "Penguin Adventure (bootleg of MSX version)", GAME_SUPPORTS_SAVE )
