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

class skeetsht_state : public driver_device
{
public:
	skeetsht_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_tms_vram;
	UINT8 m_porta_latch;
	UINT8 m_ay_sel;
	UINT8 m_lastdataw;
	UINT16 m_lastdatar;
	device_t *m_ay;
	device_t *m_tms;
};


/*************************************
 *
 *  Initialisation
 *
 *************************************/

static MACHINE_RESET( skeetsht )
{
	skeetsht_state *state = machine.driver_data<skeetsht_state>();

	state->m_ay = machine.device("aysnd");
	state->m_tms = machine.device("tms");
}


/*************************************
 *
 *  Video Functions
 *
 *************************************/

static VIDEO_START ( skeetsht )
{
}

static void skeetsht_scanline_update(screen_device &screen, bitmap_rgb32 &bitmap, int scanline, const tms34010_display_params *params)
{
	skeetsht_state *state = screen.machine().driver_data<skeetsht_state>();
	const rgb_t *const pens = tlc34076_get_pens(screen.machine().device("tlc34076"));
	UINT16 *vram = &state->m_tms_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = &bitmap.pix32(scanline);
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

	return tlc34076_r(space->machine().device("tlc34076"), offset);
}

static WRITE16_HANDLER( ramdac_w )
{
	offset = (offset >> 12) & ~4;

	if (offset & 8)
		offset = (offset & ~8) | 4;

	tlc34076_w(space->machine().device("tlc34076"), offset, data);
}


/*************************************
 *
 *  CPU Communications
 *
 *************************************/

static void skeetsht_tms_irq(device_t *device, int state)
{
	cputag_set_input_line(device->machine(), "68hc11", MC68HC11_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


static WRITE8_HANDLER( tms_w )
{
	skeetsht_state *state = space->machine().driver_data<skeetsht_state>();

	if ((offset & 1) == 0)
		state->m_lastdataw = data;
	else
		tms34010_host_w(state->m_tms, offset >> 1, (state->m_lastdataw << 8) | data);
}

static READ8_HANDLER( tms_r )
{
	skeetsht_state *state = space->machine().driver_data<skeetsht_state>();

	if ((offset & 1) == 0)
		state->m_lastdatar = tms34010_host_r(state->m_tms, offset >> 1);

	return state->m_lastdatar >> ((offset & 1) ? 0 : 8);
}


/*************************************
 *
 *  I/O
 *
 *************************************/

static READ8_HANDLER( hc11_porta_r )
{
	skeetsht_state *state = space->machine().driver_data<skeetsht_state>();

	return state->m_porta_latch;
}

static WRITE8_HANDLER( hc11_porta_w )
{
	skeetsht_state *state = space->machine().driver_data<skeetsht_state>();

	if (!(data & 0x8) && (state->m_porta_latch & 8))
		state->m_ay_sel = state->m_porta_latch & 0x10;

	state->m_porta_latch = data;
}

static WRITE8_HANDLER( ay8910_w )
{
	skeetsht_state *state = space->machine().driver_data<skeetsht_state>();

	if (state->m_ay_sel)
		ay8910_data_w(state->m_ay, 0, data);
	else
		ay8910_address_w(state->m_ay, 0, data);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( hc11_pgm_map, AS_PROGRAM, 8, skeetsht_state )
	AM_RANGE(0x2800, 0x2807) AM_READWRITE_LEGACY(tms_r, tms_w)
	AM_RANGE(0x1800, 0x1800) AM_WRITE_LEGACY(ay8910_w)
	AM_RANGE(0xb600, 0xbdff) AM_RAM //internal EEPROM
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_REGION("68hc11", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hc11_io_map, AS_IO, 8, skeetsht_state )
	AM_RANGE(MC68HC11_IO_PORTA, MC68HC11_IO_PORTA) AM_READWRITE_LEGACY(hc11_porta_r, hc11_porta_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Video CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( tms_program_map, AS_PROGRAM, 16, skeetsht_state )
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE_LEGACY(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE(m_tms_vram)
	AM_RANGE(0x00440000, 0x004fffff) AM_READWRITE_LEGACY(ramdac_r, ramdac_w)
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
	0x01,
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
	NULL,						/* scanline updater (indexed16) */
	skeetsht_scanline_update,   /* scanline updater (rgb32) */
	skeetsht_tms_irq,           /* generate interrupt */
	NULL,                       /* write to shiftreg function */
	NULL                        /* read from shiftreg function */
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( skeetsht, skeetsht_state )

	MCFG_CPU_ADD("68hc11", MC68HC11, 4000000) // ?
	MCFG_CPU_PROGRAM_MAP(hc11_pgm_map)
	MCFG_CPU_IO_MAP(hc11_io_map)
	MCFG_CPU_CONFIG(skeetsht_hc11_config)

	MCFG_CPU_ADD("tms", TMS34010, 48000000)
	MCFG_CPU_CONFIG(tms_config)
	MCFG_CPU_PROGRAM_MAP(tms_program_map)

	MCFG_MACHINE_RESET(skeetsht)

	MCFG_TLC34076_ADD("tlc34076", TLC34076_6_BIT)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(48000000 / 8, 156*4, 0, 100*4, 328, 0, 300) // FIXME
	MCFG_SCREEN_UPDATE_STATIC(tms340x0_rgb32)

	MCFG_VIDEO_START(skeetsht)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000) // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


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
