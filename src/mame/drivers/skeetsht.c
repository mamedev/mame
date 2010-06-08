/***************************************************************************

    Dynamo Skeet Shot

***************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "cpu/tms34010/tms34010.h"
#include "sound/ay8910.h"
#include "video/tlc34076.h"


/*************************************
 *
 *  Structs
 *
 *************************************/

class skeetsht_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, skeetsht_state(machine)); }

	skeetsht_state(running_machine &machine) { }

	UINT16 *tms_vram;
	UINT8 porta_latch;
	UINT8 ay_sel;
	UINT8 lastdataw;
	UINT16 lastdatar;
	running_device *ay;
	running_device *tms;
};


/*************************************
 *
 *  Initialisation
 *
 *************************************/

static MACHINE_RESET( skeetsht )
{
	skeetsht_state *state = (skeetsht_state *)machine->driver_data;

	state->ay = devtag_get_device(machine, "aysnd");
	state->tms = devtag_get_device(machine, "tms");

	/* Setup the Bt476 VGA RAMDAC palette chip */
	tlc34076_reset(6);
}


/*************************************
 *
 *  Video Functions
 *
 *************************************/

static VIDEO_START ( skeetsht )
{
}

static void skeetsht_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params)
{
	skeetsht_state *state = (skeetsht_state *)screen.machine->driver_data;
	const rgb_t *const pens = tlc34076_get_pens();
	UINT16 *vram = &state->tms_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = BITMAP_ADDR32(bitmap, scanline, 0);
	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = pens[pixels & 0xff];
		dest[x + 1] = pens[pixels >> 8];
	}
}

static READ16_HANDLER( ramdac_r )
{
	offset = (offset >> 12) & ~4;

	if (offset & 8)
		offset = (offset & ~8) | 4;

	return tlc34076_r(space, offset);
}

static WRITE16_HANDLER( ramdac_w )
{
	offset = (offset >> 12) & ~4;

	if (offset & 8)
		offset = (offset & ~8) | 4;

	tlc34076_w(space, offset, data);
}


/*************************************
 *
 *  CPU Communications
 *
 *************************************/

static void skeetsht_tms_irq(running_device *device, int state)
{
	cputag_set_input_line(device->machine, "68hc11", MC68HC11_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


static WRITE8_HANDLER( tms_w )
{
	skeetsht_state *state = (skeetsht_state *)space->machine->driver_data;

	if ((offset & 1) == 0)
		state->lastdataw = data;
	else
		tms34010_host_w(state->tms, offset >> 1, (state->lastdataw << 8) | data);
}

static READ8_HANDLER( tms_r )
{
	skeetsht_state *state = (skeetsht_state *)space->machine->driver_data;

	if ((offset & 1) == 0)
		state->lastdatar = tms34010_host_r(state->tms, offset >> 1);

	return state->lastdatar >> ((offset & 1) ? 0 : 8);
}


/*************************************
 *
 *  I/O
 *
 *************************************/

static READ8_HANDLER( hc11_porta_r )
{
	skeetsht_state *state = (skeetsht_state *)space->machine->driver_data;

	return state->porta_latch;
}

static WRITE8_HANDLER( hc11_porta_w )
{
	skeetsht_state *state = (skeetsht_state *)space->machine->driver_data;

	if (!(data & 0x8) && (state->porta_latch & 8))
		state->ay_sel = state->porta_latch & 0x10;

	state->porta_latch = data;
}

static WRITE8_HANDLER( ay8910_w )
{
	skeetsht_state *state = (skeetsht_state *)space->machine->driver_data;

	if (state->ay_sel)
		ay8910_data_w(state->ay, 0, data);
	else
		ay8910_address_w(state->ay, 0, data);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( hc11_pgm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x2800, 0x2807) AM_READWRITE(tms_r, tms_w)
	AM_RANGE(0x1800, 0x1800) AM_WRITE(ay8910_w)
	AM_RANGE(0xb600, 0xbdff) AM_RAM //internal EEPROM
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_REGION("68hc11", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hc11_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MC68HC11_IO_PORTA, MC68HC11_IO_PORTA) AM_READWRITE(hc11_porta_r, hc11_porta_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Video CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( tms_program_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE_MEMBER(skeetsht_state,tms_vram)
	AM_RANGE(0x00440000, 0x004fffff) AM_READWRITE(ramdac_r, ramdac_w)
	AM_RANGE(0xff800000, 0xffbfffff) AM_ROM AM_MIRROR(0x00400000) AM_REGION("tms", 0)
ADDRESS_MAP_END


/*************************************
 *
 *  Input definitions
 *
 *************************************/

static INPUT_PORTS_START( skeetsht )
INPUT_PORTS_END


/*************************************
 *
 *  68HC11A1 configuration
 *
 *************************************/

static const hc11_config skeetsht_hc11_config =
{
	0,
	0x100,	/* 256 bytes RAM */
//  0x200,  /* 512 bytes EEPROM */
};


/*************************************
 *
 *  TMS34010 configuration
 *
 *************************************/

static const tms34010_config tms_config =
{
	TRUE,                       /* halt on reset */
	"screen",                   /* the screen operated on */
	48000000 / 8,               /* pixel clock */
	1,                          /* pixels per clock */
	skeetsht_scanline_update,   /* scanline updater */
	skeetsht_tms_irq,           /* generate interrupt */
	NULL,                       /* write to shiftreg function */
	NULL                        /* read from shiftreg function */
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( skeetsht )

	MDRV_DRIVER_DATA( skeetsht_state )

	MDRV_CPU_ADD("68hc11", MC68HC11, 4000000) // ?
	MDRV_CPU_PROGRAM_MAP(hc11_pgm_map)
	MDRV_CPU_IO_MAP(hc11_io_map)
	MDRV_CPU_CONFIG(skeetsht_hc11_config)

	MDRV_CPU_ADD("tms", TMS34010, 48000000)
	MDRV_CPU_CONFIG(tms_config)
	MDRV_CPU_PROGRAM_MAP(tms_program_map)

	MDRV_MACHINE_RESET(skeetsht)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(48000000 / 8, 156*4, 0, 100*4, 328, 0, 300) // FIXME

	MDRV_VIDEO_START(skeetsht)
	MDRV_VIDEO_UPDATE(tms340x0)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 2000000) // ?
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( skeetsht )
	ROM_REGION( 0x40000, "68hc11", 0 )
	ROM_LOAD( "hc_11_v1.2.u34",  0x00000, 0x20000, CRC(b9801dea) SHA1(bc5bcd29b5880081c87b4014eb2c9c5077024db1) )
	ROM_LOAD( "sound.u35",       0x20000, 0x20000, CRC(0d9be853) SHA1(51eda4e0a99d50e09476704eb75310b5ee2690f4) )

	ROM_REGION16_LE( 0x200000, "tms", 0 )
	ROM_LOAD16_BYTE( "even_v1.2.u14", 0x000000, 0x40000, CRC(c7c9515e) SHA1(ce3e813c15085790d5335d9fc751b3cc5b617b20) )
	ROM_LOAD16_BYTE( "odd_v1.2.u13",  0x000001, 0x40000, CRC(ea4402fb) SHA1(b0b6b191a8b48bead660a385c638363943a6ffe2) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, skeetsht, 0, skeetsht, skeetsht, 0, ROT0, "Dynamo", "Skeet Shot", GAME_NOT_WORKING )
