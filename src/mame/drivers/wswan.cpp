// license:BSD-3-Clause
// copyright-holders:Anthony Kruize,Wilbert Pol
/***************************************************************************

  wswan.c

  Driver file to handle emulation of the Bandai WonderSwan
  By:

  Anthony Kruize
  Wilbert Pol

  Based on the WStech documentation by Judge and Dox.

  Known issues/TODOs:
  - Get the V30MZ core into MAME, still need to remove some nec specific
    instructions and fix the flags handling of the div/mul instructions.
  - Add support for noise sound
  - Add support for voice sound
  - Add support for enveloped sound
  - Perform video DMA at proper timing.
  - Add sound DMA.
  - Setup some reasonable values in the internal EEPROM area?
  - Add (real/proper) RTC support.
  - Look into timing issues like in Puzzle Bobble. VBlank interrupt lasts very long
    which causes sprites to be disabled until about 10%-40% of drawing the screen.
    The real unit seems to display things fine, need a real unit + real cart to
    verify.
  - Is background color setting really ok?
  - Get a dump of the internal BIOSes.
  - Swan Crystal can handle up to 512Mbit ROMs??????


***************************************************************************/

#include "includes/wswan.h"
#include "softlist.h"
#include "wswan.lh"

static ADDRESS_MAP_START (wswan_mem, AS_PROGRAM, 8, wswan_state)
	AM_RANGE(0x00000, 0x03fff) AM_DEVREADWRITE("vdp", wswan_video_device, vram_r, vram_w)       // 16kb RAM / 4 colour tiles
	AM_RANGE(0x04000, 0x0ffff) AM_NOP       // nothing
	//AM_RANGE(0x10000, 0xeffff)    // cart range, setup at machine_start
	AM_RANGE(0xf0000, 0xfffff) AM_READ(bios_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START (wscolor_mem, AS_PROGRAM, 8, wswan_state)
	AM_RANGE(0x00000, 0x0ffff) AM_DEVREADWRITE("vdp", wswan_video_device, vram_r, vram_w)       // 16kb RAM / 4 colour tiles, 16 colour tiles + palettes
	//AM_RANGE(0x10000, 0xeffff)    // cart range, setup at machine_start
	AM_RANGE(0xf0000, 0xfffff) AM_READ(bios_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START (wswan_io, AS_IO, 8, wswan_state)
	AM_RANGE(0x00, 0xff) AM_READWRITE(port_r, port_w)   // I/O ports
ADDRESS_MAP_END

static INPUT_PORTS_START( wswan )
	PORT_START("CURSX")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("X1 - Up")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("X3 - Down")
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("X4 - Left")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("X2 - Right")

	PORT_START("BUTTONS")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Button A")
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Button B")

	PORT_START("CURSY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y1 - Up") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y3 - Down") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y4 - Left") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y2 - Right") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

static GFXDECODE_START( wswan )
GFXDECODE_END

/* WonderSwan can display 16 shades of grey */
PALETTE_INIT_MEMBER(wswan_state, wswan)
{
	for (int i = 0; i < 16; i++)
	{
		UINT8 shade = i * (256 / 16);
		palette.set_pen_color(15 - i, shade, shade, shade);
	}
}

PALETTE_INIT_MEMBER(wswan_state, wscolor)
{
	for (int i = 0; i < 4096; i++)
	{
		int r = (i & 0x0f00) >> 8;
		int g = (i & 0x00f0) >> 4;
		int b = i & 0x000f;
		palette.set_pen_color(i, r << 4, g << 4, b << 4);
	}
}

static SLOT_INTERFACE_START(wswan_cart)
	SLOT_INTERFACE_INTERNAL("ws_rom",     WS_ROM_STD)
	SLOT_INTERFACE_INTERNAL("ws_sram",    WS_ROM_SRAM)
	SLOT_INTERFACE_INTERNAL("ws_eeprom",  WS_ROM_EEPROM)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( wswan, wswan_state )
	/* Basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30MZ, 3072000)
	MCFG_CPU_PROGRAM_MAP(wswan_mem)
	MCFG_CPU_IO_MAP(wswan_io)

	MCFG_DEVICE_ADD("vdp", WSWAN_VIDEO, 0)
	MCFG_WSWAN_VIDEO_TYPE(VDP_TYPE_WSWAN)
	MCFG_WSWAN_VIDEO_IRQ_CB(wswan_state, set_irq_line)
	MCFG_WSWAN_VIDEO_DMASND_CB(wswan_state, dma_sound_cb)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(75)
	MCFG_SCREEN_VBLANK_TIME(0)
	MCFG_SCREEN_UPDATE_DEVICE("vdp", wswan_video_device, screen_update)
	MCFG_SCREEN_SIZE(WSWAN_X_PIXELS, WSWAN_Y_PIXELS)
	MCFG_SCREEN_VISIBLE_AREA(0*8, WSWAN_X_PIXELS - 1, 0, WSWAN_Y_PIXELS - 1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_wswan)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", wswan)
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(wswan_state, wswan)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("custom", WSWAN_SND, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* cartridge */
	MCFG_WSWAN_CARTRIDGE_ADD("cartslot", wswan_cart, nullptr)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","wswan")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("wsc_list","wscolor")

	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("pc1_list","pockchalv1")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("pc2_list","pockchalv2")

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( wscolor, wswan )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(wscolor_mem)
	MCFG_MACHINE_START_OVERRIDE(wswan_state, wscolor)

	MCFG_DEVICE_MODIFY("vdp")
	MCFG_WSWAN_VIDEO_TYPE(VDP_TYPE_WSC)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(4096)
	MCFG_PALETTE_INIT_OWNER(wswan_state, wscolor)

	/* software lists */
	MCFG_DEVICE_REMOVE("cart_list")
	MCFG_DEVICE_REMOVE("wsc_list")
	MCFG_SOFTWARE_LIST_ADD("cart_list","wscolor")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("ws_list","wswan")


MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wswan )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD_OPTIONAL( "ws_bios.bin", 0x0000, 0x0001, NO_DUMP )
ROM_END

ROM_START( wscolor )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD_OPTIONAL( "wsc_bios.bin", 0x0000, 0x0001, NO_DUMP )
ROM_END

/*     YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  INIT COMPANY   FULLNAME*/
CONS( 1999, wswan,   0,      0,      wswan,   wswan, driver_device, 0,    "Bandai", "WonderSwan",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 2000, wscolor, wswan,  0,      wscolor, wswan, driver_device, 0,    "Bandai", "WonderSwan Color", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
