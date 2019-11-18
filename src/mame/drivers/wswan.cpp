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

#include "emu.h"
#include "includes/wswan.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "wswan.lh"

void wswan_state::wswan_mem(address_map &map)
{
	map(0x00000, 0x03fff).rw(m_vdp, FUNC(wswan_video_device::vram_r), FUNC(wswan_video_device::vram_w));       // 16kb RAM / 4 colour tiles
	map(0x04000, 0x0ffff).noprw();       // nothing
	//map(0x10000, 0xeffff)    // cart range, setup at machine_start
	map(0xf0000, 0xfffff).r(FUNC(wswan_state::bios_r));
}

void wscolor_state::wscolor_mem(address_map &map)
{
	map(0x00000, 0x0ffff).rw("vdp", FUNC(wswan_video_device::vram_r), FUNC(wswan_video_device::vram_w));       // 16kb RAM / 4 colour tiles, 16 colour tiles + palettes
	//map(0x10000, 0xeffff)    // cart range, setup at machine_start
	map(0xf0000, 0xfffff).r(FUNC(wscolor_state::bios_r));
}

void wswan_state::wswan_io(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(wswan_state::port_r), FUNC(wswan_state::port_w));   // I/O ports
}

void wswan_state::wswan_snd(address_map &map)
{
	map(0x00000, 0x03fff).r(m_vdp, FUNC(wswan_video_device::vram_r));
}

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

static GFXDECODE_START( gfx_wswan )
GFXDECODE_END

/* WonderSwan can display 16 shades of grey */
void wswan_state::wswan_palette(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
	{
		uint8_t const shade = i * (256 / 16);
		palette.set_pen_color(15 - i, shade, shade, shade);
	}
}

void wscolor_state::wscolor_palette(palette_device &palette) const
{
	for (int i = 0; i < 4096; i++)
	{
		int const r = (i & 0x0f00) >> 8;
		int const g = (i & 0x00f0) >> 4;
		int const b = i & 0x000f;
		palette.set_pen_color(i, r << 4, g << 4, b << 4);
	}
}

static void wswan_cart(device_slot_interface &device)
{
	device.option_add_internal("ws_rom",     WS_ROM_STD);
	device.option_add_internal("ws_sram",    WS_ROM_SRAM);
	device.option_add_internal("ws_eeprom",  WS_ROM_EEPROM);
}

void wswan_state::wswan(machine_config &config)
{
	/* Basic machine hardware */
	V30MZ(config, m_maincpu, 3.072_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &wswan_state::wswan_mem);
	m_maincpu->set_addrmap(AS_IO, &wswan_state::wswan_io);

	WSWAN_VIDEO(config, m_vdp, 0);
	m_vdp->set_screen("screen");
	m_vdp->set_vdp_type(VDP_TYPE_WSWAN);
	m_vdp->set_irq_callback(FUNC(wswan_state::set_irq_line));
	m_vdp->set_dmasnd_callback(FUNC(wswan_state::dma_sound_cb));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
//  screen.set_refresh_rate(75);
//  screen.set_vblank_time(0);
	screen.set_screen_update("vdp", FUNC(wswan_video_device::screen_update));
//  screen.set_size(WSWAN_X_PIXELS, WSWAN_Y_PIXELS);
//  screen.set_visarea(0*8, WSWAN_X_PIXELS - 1, 0, WSWAN_Y_PIXELS - 1);
	screen.set_raw(3.072_MHz_XTAL, 256, 0, WSWAN_X_PIXELS, 159, 0, WSWAN_Y_PIXELS);
	screen.set_palette("palette");

	config.set_default_layout(layout_wswan);

	config.set_maximum_quantum(attotime::from_hz(60));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	GFXDECODE(config, "gfxdecode", "palette", gfx_wswan);
	PALETTE(config, "palette", FUNC(wswan_state::wswan_palette), 16);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	WSWAN_SND(config, m_sound, 3.072_MHz_XTAL);
	m_sound->set_addrmap(0, &wswan_state::wswan_snd);
	m_sound->add_route(0, "lspeaker", 0.50);
	m_sound->add_route(1, "rspeaker", 0.50);

	/* cartridge */
	WS_CART_SLOT(config, m_cart, 3.072_MHz_XTAL / 8, wswan_cart, nullptr);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("wswan");
	SOFTWARE_LIST(config, "wsc_list").set_compatible("wscolor");

	SOFTWARE_LIST(config, "pc2_list").set_compatible("pockchalv2");
}

void wscolor_state::wscolor(machine_config &config)
{
	wswan(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &wscolor_state::wscolor_mem);

	m_vdp->set_vdp_type(VDP_TYPE_WSC);

	auto &palette(*subdevice<palette_device>("palette"));
	palette.set_entries(4096);
	palette.set_init(FUNC(wscolor_state::wscolor_palette));

	/* software lists */
	config.device_remove("wsc_list");
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("wscolor");
	SOFTWARE_LIST(config, "ws_list").set_compatible("wswan");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wswan )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD( "ws_bios.bin", 0x0000, 0x0001, NO_DUMP )
ROM_END

ROM_START( wscolor )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD( "wsc_bios.bin", 0x0000, 0x0001, NO_DUMP )
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS          INIT        COMPANY   FULLNAME
CONS( 1999, wswan,   0,      0,      wswan,   wswan, wswan_state,   empty_init, "Bandai", "WonderSwan",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 2000, wscolor, wswan,  0,      wscolor, wswan, wscolor_state, empty_init, "Bandai", "WonderSwan Color", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
