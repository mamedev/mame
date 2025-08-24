// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************

 Atari Lynx
 PeT peter.trauner@utanet.at 2000,2001

 info found in bastian schick's bll
 and in cc65 for lynx

 TODO:
 - ComLynx emulation is missing/imperfect
 - Verify timings from real hardware
 - EEPROM Support in some homebrew cartridges?
 - Lynx II support, needs internal ROM dump

******************************************************************************/

#include "emu.h"
#include "lynx.h"

#include "cpu/m6502/g65sc02.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "lynx.lh"

void lynx_state::cpu_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share(m_dram);
	map(0xfc00, 0xfcff).view(m_suzy_view);
	m_suzy_view[0](0xfc00, 0xfcff).rw(FUNC(lynx_state::suzy_read), FUNC(lynx_state::suzy_write));
	map(0xfd00, 0xfdff).view(m_mikey_view);
	m_mikey_view[0](0xfd00, 0xfdff).rw(FUNC(lynx_state::mikey_read), FUNC(lynx_state::mikey_write));
	map(0xfe00, 0xffff).view(m_rom_view);
	m_rom_view[0](0xfe00, 0xfff7).rom().region("maincpu", 0x0000);
	map(0xfff8, 0xfff8).ram(); // Reserved for future hardware (RAM)
	map(0xfff9, 0xfff9).rw(FUNC(lynx_state::memory_config_r), FUNC(lynx_state::memory_config_w));
	map(0xfff8, 0xffff).view(m_vector_view);
	m_vector_view[0](0xfffa, 0xffff).rom().region("maincpu", 0x01fa);
}

static INPUT_PORTS_START( lynx )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Opt 2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Opt 1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("PAUSE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_3)
	//PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) CART0 Strobe
	//PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) CART1 Strobe
	// power on and power off buttons
INPUT_PORTS_END

void lynx_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
}

u32 lynx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

// callback for Mikey call of shift(3) which shall act on the timer_count_down
void lynx_state::sound_cb()
{
	timer_count_down(1);
}

void lynx_state::lynx(machine_config &config)
{
	/* basic machine hardware */
	G65SC02(config, m_maincpu, XTAL(16'000'000) / 4); /* vti core, integrated in vlsi, stz, but not bbr bbs */
	m_maincpu->set_addrmap(AS_PROGRAM, &lynx_state::cpu_map);
	config.set_maximum_quantum(attotime::from_hz(60));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(XTAL(16'000'000) / 16 / (158 + 1) / (104 + 1)); // default config from machine_reset(), actually variable
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(FUNC(lynx_state::screen_update));
	m_screen->set_size(160, 105); // 102 visible scanline + 3 blank scanline, horizontal unknown/unverified, variable?
	m_screen->set_visarea(0, 160-1, 0, 102-1);
	config.set_default_layout(layout_lynx);

	PALETTE(config, m_palette).set_entries(0x10);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	LYNX_SND(config, m_sound, XTAL(16'000'000));
	m_sound->set_timer_delegate(FUNC(lynx_state::sound_cb));
	m_sound->add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	QUICKLOAD(config, "quickload", "o").set_load_callback(FUNC(lynx_state::quickload_cb));

	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "lynx_cart", "lnx,lyx"));
	cartslot.set_must_be_loaded(true);
	cartslot.set_device_load(FUNC(lynx_state::cart_load));

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("lynx");
}

// Lynx II has Hayato replaces Mikey, Compatible but supports stereo
#if 0
void lynx_state::lynx2(machine_config &config)
{
	lynx(config);

	/* sound hardware */
	config.device_remove("mono");
	SPEAKER(config, "speaker", 2).front();
	LYNX2_SND(config.replace(), m_sound, XTAL(16'000'000));
	m_sound->set_timer_delegate(FUNC(lynx_state::sound_cb));
	m_sound->add_route(0, "speaker", 0.50, 0);
	m_sound->add_route(1, "speaker", 0.50, 1);
}
#endif

/* these 2 dumps are saved from an running machine,
   and therefor the rom byte at 0xfff9 is not readable!
   (memory configuration)
   these 2 dumps differ only in this byte!
*/

ROM_START(lynx)
	ROM_REGION(0x200,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "default",   "rom save" )
	ROMX_LOAD("lynx.bin",  0x00000, 0x200, BAD_DUMP CRC(e1ffecb6) SHA1(de60f2263851bbe10e5801ef8f6c357a4bc077e6), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "a", "alternate rom save" )
	ROMX_LOAD("lynxa.bin", 0x00000, 0x200, BAD_DUMP CRC(0d973c9d) SHA1(e4ed47fae31693e016b081c6bda48da5b70d7ccb), ROM_BIOS(1))
ROM_END

#if 0
ROM_START(lynx2)
	ROM_REGION(0x200,"maincpu", 0)
	ROM_LOAD("lynx2.bin", 0, 0x200, NO_DUMP)
ROM_END
#endif


QUICKLOAD_LOAD_MEMBER(lynx_state::quickload_cb)
{
	u8 header[10]; // 80 08 dw Start dw Len B S 9 3
	if (image.fread( header, sizeof(header)) != sizeof(header))
		return std::make_pair(image_error::UNSPECIFIED, std::string());

	/* Check the image */
	auto err = verify_cart((const char*)header, LYNX_QUICKLOAD);
	if (err.first)
		return err;

	uint16_t const start = header[3] | (header[2]<<8); //! big endian format in file format for little endian cpu
	uint16_t const length = (header[5] | (header[4]<<8)) - 10;

	std::vector<u8> data;
	data.resize(length);
	if (image.fread(&data[0], length) != length)
		return std::make_pair(image_error::INVALIDIMAGE, "Invalid length in file header");

	address_space &space = m_maincpu->space(AS_PROGRAM);
	for (int i = 0; i < length; i++)
		space.write_byte(start + i, data[i]);

	u8 *rom = memregion("maincpu")->base();
	rom[0x1fc] = start & 0xff;
	rom[0x1fd] = start >> 8;
	space.write_byte(0x1fc, start & 0xff);
	space.write_byte(0x1fd, start >> 8);

	m_maincpu->set_pc(start);

	return std::make_pair(std::error_condition(), std::string());
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME  FLAGS */
CONS( 1989, lynx, 0,      0,      lynx,    lynx,  lynx_state, empty_init, "Atari", "Lynx",   MACHINE_SUPPORTS_SAVE )
// CONS( 1991, lynx2,  lynx,  0,      lynx2,  lynx, lynx_state, empty_init, "Atari",  "Lynx II",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
