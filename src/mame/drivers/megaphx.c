/* Megaphoenix
*/


/*

  Chips of note

  Main board:

  TS68000CP8
  TMS34010FNL-40
  TMP82C55AP-2
 
  Bt478KPJ35  Palette / RAMDAC

  Actel A1010A-PL68C  (custom blitter maybe?)


 Sub / Sound board:

  ST Z8430AB1

  custom INDER badged chip 40 pin?  (probably just a z80 - it's in the sound section)
	MODELO: MEGA PHOENIX
	KIT NO. 1.034
	FECHA FABRICACION 08.10.91
	LA MANIPULCION DE LA ETIQUETA O DE LA PLACA ANULA SU SARANTIA
	(this sticker is also present on the other PCB)


*/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "video/ramdac.h"

class megaphx_state : public driver_device
{
public:
	megaphx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16> m_mainram;
	required_shared_ptr<UINT16> m_vram;


	DECLARE_DRIVER_INIT(megaphx);
	DECLARE_MACHINE_RESET(megaphx);



	DECLARE_READ16_MEMBER(megaphx_60004_r);
	DECLARE_READ16_MEMBER(megaphx_50002_r);

	DECLARE_READ16_MEMBER(tms_host_r);
	DECLARE_WRITE16_MEMBER(tms_host_w);

};

READ16_MEMBER(megaphx_state::megaphx_60004_r)
{
	return rand();
}

READ16_MEMBER(megaphx_state::megaphx_50002_r)
{
	return rand();
}

READ16_MEMBER(megaphx_state::tms_host_r)
{
	return tms34010_host_r(machine().device("tms"), offset);
}


WRITE16_MEMBER(megaphx_state::tms_host_w)
{
	tms34010_host_w(machine().device("tms"), offset, data);
}



static ADDRESS_MAP_START( megaphx_68k_map, AS_PROGRAM, 16, megaphx_state )
//	AM_RANGE(0x000000, 0x0000ff) AM_ROM
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_SHARE("mainram")

	AM_RANGE(0x040000, 0x040007) AM_READWRITE(tms_host_r, tms_host_w)

	AM_RANGE(0x050002, 0x050003) AM_READ(megaphx_50002_r)

	AM_RANGE(0x060004, 0x060005) AM_READ(megaphx_60004_r)
	AM_RANGE(0x060006, 0x060007) AM_WRITENOP

	
	AM_RANGE(0x800000, 0x8fffff) AM_ROM

ADDRESS_MAP_END


static ADDRESS_MAP_START( megaphx_tms_map, AS_PROGRAM, 16, megaphx_state )

	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("vram") // vram?
	AM_RANGE(0x00100000, 0x002fffff) AM_RAM  // vram?
	AM_RANGE(0x00300000, 0x003fffff) AM_RAM
//	AM_RANGE(0x04000000, 0x040000ff) AM_WRITENOP

	AM_RANGE(0x04000000, 0x0400000f) AM_DEVWRITE8("ramdac",ramdac_device,index_w,0x00ff)
	AM_RANGE(0x04000010, 0x0400001f) AM_DEVREADWRITE8("ramdac",ramdac_device,pal_r,pal_w,0x00ff)
	AM_RANGE(0x04000030, 0x0400003f) AM_DEVWRITE8("ramdac",ramdac_device,index_r_w,0x00ff)

	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE_LEGACY(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xffc00000, 0xffffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, megaphx_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x401f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io, AS_IO, 8, megaphx_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x07) AM_RAM
	AM_RANGE(0x31, 0x31) AM_READNOP
ADDRESS_MAP_END


static void megaphx_scanline(screen_device &screen, bitmap_rgb32 &bitmap, int scanline, const tms34010_display_params *params)
{
	megaphx_state *state = screen.machine().driver_data<megaphx_state>();

	UINT16 *vram = &state->m_vram[(params->rowaddr << 8) & 0x1ff00];
	UINT32 *dest = &bitmap.pix32(scanline);

	const pen_t *paldata = screen.machine().pens;

	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = paldata[pixels & 0xff];
		dest[x + 1] = paldata[pixels >> 8];
	}

}

static void megaphx_to_shiftreg(address_space &space, UINT32 address, UINT16 *shiftreg)
{
}

static void megaphx_from_shiftreg(address_space &space, UINT32 address, UINT16 *shiftreg)
{
}

MACHINE_RESET_MEMBER(megaphx_state,megaphx)
{
}

static void m68k_gen_int(device_t *device, int state)
{
	megaphx_state *drvstate = device->machine().driver_data<megaphx_state>();
	if (state) drvstate->m_maincpu->set_input_line(4, ASSERT_LINE);
	else drvstate->m_maincpu->set_input_line(4, CLEAR_LINE);
//	printf("interrupt %d\n", state);
}


static const tms34010_config tms_config_megaphx =
{
	TRUE,                          /* halt on reset */
	"screen",                       /* the screen operated on */
	XTAL_40MHz/12,                   /* pixel clock */
	2,                              /* pixels per clock */
	NULL,                           /* scanline callback (indexed16) */
	megaphx_scanline,              /* scanline callback (rgb32) */
	m68k_gen_int,                   /* generate interrupt */
	megaphx_to_shiftreg,           /* write to shiftreg function */
	megaphx_from_shiftreg          /* read from shiftreg function */
};



static INPUT_PORTS_START( megaphx )
INPUT_PORTS_END

static ADDRESS_MAP_START( ramdac_map, AS_0, 8, megaphx_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb888_w)
ADDRESS_MAP_END

static RAMDAC_INTERFACE( ramdac_intf )
{
	1
};


static MACHINE_CONFIG_START( megaphx, megaphx_state )

	MCFG_CPU_ADD("maincpu", M68000, 8000000) // ??  can't read xtal due to reflections, CPU is an 8Mhz part
	MCFG_CPU_PROGRAM_MAP(megaphx_68k_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", megaphx_state,  irq6_line_hold)


	MCFG_CPU_ADD("tms", TMS34010, XTAL_40MHz)
	MCFG_CPU_CONFIG(tms_config_megaphx)
	MCFG_CPU_PROGRAM_MAP(megaphx_tms_map)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) // unk freq
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io)


	MCFG_MACHINE_RESET_OVERRIDE(megaphx_state,megaphx)

//	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_40MHz/12, 424, 0, 320, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DEVICE("tms", tms34010_device, tms340x0_rgb32)

	MCFG_PALETTE_LENGTH(256)

	MCFG_RAMDAC_ADD("ramdac", ramdac_intf, ramdac_map)

MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(megaphx_state,megaphx)
{
	UINT16 *src = (UINT16*)memregion( "maincpu" )->base();
	// copy vector table?
	memcpy(m_mainram, src, 0x100);
}


ROM_START( megaphx )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mph6.u32", 0x000001, 0x20000, CRC(b99703d4) SHA1(393b6869e71d4c61060e66e0e9e36a1e6ca345d1) )
	ROM_LOAD16_BYTE( "mph7.u21", 0x000000, 0x20000, CRC(f11e7449) SHA1(1017142d10011d68e49d3ccdb1ac4e815c03b17a) )

	ROM_LOAD16_BYTE( "mph0.u38", 0x800001, 0x20000, CRC(b63dd20f) SHA1(c8ce5985a6ba49428d66a49d9d623ccdfce422c2) )
	ROM_LOAD16_BYTE( "mph1.u27", 0x800000, 0x20000, CRC(4dcbf44b) SHA1(a8fa49ecd033f1aeb323e0032ddcf5f8f9463ac0) )
	ROM_LOAD16_BYTE( "mph2.u37", 0x840001, 0x20000, CRC(a0f69c27) SHA1(d0c5c241d94a1f03f51e7e517e2f9dec6abcf75a) )
	ROM_LOAD16_BYTE( "mph3.u26", 0x840000, 0x20000, CRC(4db84cc5) SHA1(dd74acd4b32c7e7553554ac0f9ba13503358e869) )
	ROM_LOAD16_BYTE( "mph4.u36", 0x880001, 0x20000, CRC(c8e0725e) SHA1(b3af315b9a94a692e81e0dbfd4035036c2af4f50) )
	ROM_LOAD16_BYTE( "mph5.u25", 0x880000, 0x20000, CRC(c95ccb69) SHA1(9d14cbfafd943f6ff461a7f373170a35e36eb695) )

	ROM_REGION( 0x200000, "user2", 0 )
	ROM_LOAD( "sonido_mph1.u39", 0x000000, 0x20000, CRC(f5e65557) SHA1(5ae759c2bcef96fbda42f088c02b6dec208030f3) )
	ROM_LOAD( "sonido_mph2.u38", 0x000000, 0x20000, CRC(7444d0f9) SHA1(9739b48993bccea5530533b67808d13d6155ffe3) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD( "sonido_mph0.u35", 0x000000, 0x2000,  CRC(abc1b140) SHA1(8384a162d85cf9ea870d22f44b1ca64001c6a083) )

	ROM_REGION( 0x100000, "pals", 0 ) // jedutil won't convert these? are they bad?
	ROM_LOAD( "p31_u31_palce16v8h-25.jed", 0x000, 0xbd4, CRC(05ef04b7) SHA1(330dd81a832b6675fb0473868c26fe9bec2da854) )
	ROM_LOAD( "p40_u29_palce16v8h-25.jed", 0x000, 0xbd4, CRC(44b7e51c) SHA1(b8b34f3b319d664ec3ad72ed87d9f65701f183a5) )
ROM_END

GAME( 1991, megaphx,  0,        megaphx, megaphx, megaphx_state, megaphx, ROT0, "Dinamic / Inder", "MegaPhoenix", GAME_NO_SOUND | GAME_NOT_WORKING )
