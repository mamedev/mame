/***************************************************************************

    Dynamo Skeet Shot

***************************************************************************/

#include "driver.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "cpu/tms34010/tms34010.h"
#include "video/tlc34076.h"


/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT16 *tms_vram;


/*************************************
 *
 *  Initialisation
 *
 *************************************/

static MACHINE_RESET( skeetsht )
{
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

void skeetsht_to_shiftreg(const address_space *space, UINT32 address, UINT16 *shiftreg)
{
	memcpy(shiftreg, &tms_vram[TOWORD(address)], 512 * sizeof(UINT16));
}

void skeetsht_from_shiftreg(const address_space *space, UINT32 address, UINT16 *shiftreg)
{
	memcpy(&tms_vram[TOWORD(address)], shiftreg, 512 * sizeof(UINT16));
}

void skeetsht_scanline_update(const device_config *screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params)
{
	UINT16 *vram = &tms_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = BITMAP_ADDR32(bitmap, scanline, 0);
	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = screen->machine->pens[pixels & 0xff];
		dest[x + 1] = screen->machine->pens[pixels >> 8];
	}
}


/*************************************
 *
 *  CPU Communications
 *
 *************************************/

static void skeetsht_tms_irq(const device_config *device, int state)
{
	cputag_set_input_line(device->machine, "68hc11", MC68HC11_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


static WRITE8_HANDLER( tms_w )
{
	static UINT8 lastdata;

	if ((offset & 1) == 0)
		lastdata = data;
	else
		tms34010_host_w(cputag_get_cpu(space->machine, "tms"), offset >> 1, (lastdata << 8) | data);
}

static READ8_HANDLER( tms_r )
{
	static UINT16 data;

	if ((offset & 1) == 0)
		data = tms34010_host_r(cputag_get_cpu(space->machine, "tms"), offset >> 1);

	return data >> ((offset & 1) ? 0 : 8);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( hc11_pgm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x2800, 0x2807) AM_READWRITE(tms_r, tms_w)
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_REGION("68hc11", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hc11_io_map, ADDRESS_SPACE_IO, 8 )
//	AM_RANGE(MC68HC11_IO_PORTA,     MC68HC11_IO_PORTA    ) AM_NOP
//	AM_RANGE(MC68HC11_IO_PORTG,     MC68HC11_IO_PORTG    ) AM_READWRITE(hc11_comm_r, hc11_comm_w)
//	AM_RANGE(MC68HC11_IO_PORTH,     MC68HC11_IO_PORTH    ) AM_NOP
//	AM_RANGE(MC68HC11_IO_SPI2_DATA, MC68HC11_IO_SPI2_DATA) AM_READWRITE(hc11_data_r, hc11_data_w)
//	AM_RANGE(MC68HC11_IO_AD0,       MC68HC11_IO_AD7      ) AM_READ(hc11_analog_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Video CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( tms_program_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE(&tms_vram)
//	AM_RANGE(0x00450000, 0x0045001f) AM_READWRITE(tlc34076_msb_r, tlc34076_msb_w)
	AM_RANGE(0xffc00000, 0xffffffff) AM_ROM AM_REGION("tms", 0)
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
 *  TMS34010 configuration
 *
 *************************************/

static const tms34010_config tms_config =
{
	FALSE,                      /* halt on reset */
	"screen",                   /* the screen operated on */
	48000000 / 8,               /* pixel clock */
	1,                          /* pixels per clock */
	skeetsht_scanline_update,   /* scanline updater */
	skeetsht_tms_irq,           /* generate interrupt */
	skeetsht_to_shiftreg,       /* write to shiftreg function */
	skeetsht_from_shiftreg      /* read from shiftreg function */
};


/*************************************
 *
 *  68HC11A1 configuration
 *
 *************************************/

static const hc11_config skeetsht_hc11_config =
{
	0,
	0x100,	/* 256 bytes RAM */
	/* 512 bytes EEPROM */
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( skeetsht )
	MDRV_CPU_ADD("tms", TMS34010, 48000000)
	MDRV_CPU_CONFIG(tms_config)
	MDRV_CPU_PROGRAM_MAP(tms_program_map)

	MDRV_CPU_ADD("68hc11", MC68HC11, 4000000)
	MDRV_CPU_PROGRAM_MAP(hc11_pgm_map)
	MDRV_CPU_IO_MAP(hc11_io_map)
	MDRV_CPU_CONFIG(skeetsht_hc11_config)

	MDRV_MACHINE_RESET(skeetsht)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(40000000 / 4, 156*4, 0, 100*4, 328, 0, 300)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(skeetsht)
	MDRV_VIDEO_UPDATE(tms340x0)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( skeetsht )
	ROM_REGION16_LE( 0x200000, "tms", 0 )
	ROM_LOAD16_BYTE( "even_v1.2.u14", 0x000000, 0x40000, CRC(c7c9515e) SHA1(ce3e813c15085790d5335d9fc751b3cc5b617b20) )
	ROM_LOAD16_BYTE( "odd_v1.2.u13",  0x000001, 0x40000, CRC(ea4402fb) SHA1(b0b6b191a8b48bead660a385c638363943a6ffe2) )

	ROM_REGION( 0x20000, "68hc11", 0 )
	ROM_LOAD( "hc_11_v1.2.u34",  0x00000, 0x20000, CRC(b9801dea) SHA1(bc5bcd29b5880081c87b4014eb2c9c5077024db1) )

	ROM_REGION( 0x20000, "snd", 0 )
	ROM_LOAD( "sound.u35",  0x00000, 0x20000, CRC(0d9be853) SHA1(51eda4e0a99d50e09476704eb75310b5ee2690f4) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, skeetsht, 0, skeetsht, skeetsht, 0, ROT0, "Dynamo", "Skeet Shot", GAME_NOT_WORKING | GAME_NO_SOUND )
