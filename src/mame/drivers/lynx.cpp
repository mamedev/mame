// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT peter.trauner@utanet.at 2000,2001

 info found in bastian schick's bll
 and in cc65 for lynx

******************************************************************************/

#include "emu.h"
#include "includes/lynx.h"
#include "audio/lynx.h"

#include "cpu/m6502/m65sc02.h"
#include "softlist.h"
#include "speaker.h"

#include "lynx.lh"

void lynx_state::lynx_mem(address_map &map)
{
	map(0x0000, 0xfbff).ram().share("mem_0000");
	map(0xfc00, 0xfcff).m(m_bank_fc00, FUNC(address_map_bank_device::amap8));
	map(0xfd00, 0xfdff).m(m_bank_fd00, FUNC(address_map_bank_device::amap8));
	map(0xfe00, 0xfff7).bankr("bank_fe00").writeonly().share("mem_fe00");
	map(0xfff8, 0xfff8).ram();
	map(0xfff9, 0xfff9).rw(FUNC(lynx_state::lynx_memory_config_r), FUNC(lynx_state::lynx_memory_config_w));
	map(0xfffa, 0xffff).bankr("bank_fffa").writeonly().share("mem_fffa");
}

void lynx_state::lynx_fc00_mem(address_map &map)
{
	map(0x000, 0x0ff).rw(FUNC(lynx_state::suzy_read), FUNC(lynx_state::suzy_write));
	map(0x100, 0x1ff).ram().share("mem_fc00");
}

void lynx_state::lynx_fd00_mem(address_map &map)
{
	map(0x000, 0x0ff).rw(FUNC(lynx_state::mikey_read), FUNC(lynx_state::mikey_write));
	map(0x100, 0x1ff).ram().share("mem_fd00");
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
	// power on and power off buttons
INPUT_PORTS_END

void lynx_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
}

uint32_t lynx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

// callback for Mikey call of shift(3) which shall act on the lynx_timer_count_down
void lynx_state::sound_cb()
{
	lynx_timer_count_down(1);
}

MACHINE_CONFIG_START(lynx_state::lynx)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M65SC02, 4000000)        /* vti core, integrated in vlsi, stz, but not bbr bbs */
	MCFG_DEVICE_PROGRAM_MAP(lynx_mem)
	config.m_minimum_quantum = attotime::from_hz(60);

	ADDRESS_MAP_BANK(config, "bank_fc00").set_map(&lynx_state::lynx_fc00_mem).set_options(ENDIANNESS_LITTLE, 8, 9, 0x100);
	ADDRESS_MAP_BANK(config, "bank_fd00").set_map(&lynx_state::lynx_fd00_mem).set_options(ENDIANNESS_LITTLE, 8, 9, 0x100);

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(30)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(lynx_state, screen_update)
	MCFG_SCREEN_SIZE(160, 102)
	MCFG_SCREEN_VISIBLE_AREA(0, 160-1, 0, 102-1)
	config.set_default_layout(layout_lynx);

	MCFG_PALETTE_ADD("palette", 0x10)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	LYNX_SND(config, m_sound, 0);
	m_sound->set_timer_delegate(FUNC(lynx_state::sound_cb), this);
	m_sound->add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	MCFG_QUICKLOAD_ADD("quickload", lynx_state, lynx, "o");

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "lynx_cart")
	MCFG_GENERIC_EXTENSIONS("lnx,lyx")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_LOAD(lynx_state, lynx_cart)

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("lynx");
MACHINE_CONFIG_END

#if 0
void lynx_state::lynx2(machine_config &config)
{
	lynx(config);

	/* sound hardware */
	config.device_remove("mono");
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	config.device_remove("lynx");
	LYNX2_SND(config.replace(), m_sound, 0);
	m_sound->set_timer_delegate(FUNC(lynx_state::sound_cb), this);
	m_sound->add_route(0, "lspeaker", 0.50);
	m_sound->add_route(1, "rspeaker", 0.50);
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

	ROM_REGION(0x100,"gfx1", ROMREGION_ERASE00)
ROM_END

#if 0
ROM_START(lynx2)
	ROM_REGION(0x200,"maincpu", 0)
	ROM_LOAD("lynx2.bin", 0, 0x200, NO_DUMP)

	ROM_REGION(0x100,"gfx1", ROMREGION_ERASE00)
ROM_END
#endif


QUICKLOAD_LOAD_MEMBER( lynx_state, lynx )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	std::vector<uint8_t> data;
	uint8_t *rom = memregion("maincpu")->base();
	uint8_t header[10]; // 80 08 dw Start dw Len B S 9 3
	uint16_t start, length;
	int i;

	if (image.fread( header, sizeof(header)) != sizeof(header))
		return image_init_result::FAIL;

	/* Check the image */
	if (lynx_verify_cart((char*)header, LYNX_QUICKLOAD) != image_verify_result::PASS)
		return image_init_result::FAIL;

	start = header[3] | (header[2]<<8); //! big endian format in file format for little endian cpu
	length = header[5] | (header[4]<<8);
	length -= 10;

	data.resize(length);

	if (image.fread( &data[0], length) != length)
	{
		return image_init_result::FAIL;
	}

	for (i = 0; i < length; i++)
		space.write_byte(start + i, data[i]);

	rom[0x1fc] = start & 0xff;
	rom[0x1fd] = start >> 8;
	space.write_byte(0x1fc, start & 0xff);
	space.write_byte(0x1fd, start >> 8);

	m_maincpu->set_pc(start);

	return image_init_result::PASS;
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME  FLAGS */
CONS( 1989, lynx, 0,      0,      lynx,    lynx,  lynx_state, empty_init, "Atari", "Lynx",   MACHINE_SUPPORTS_SAVE )
// CONS( 1991, lynx2,  lynx,  0,      lynx2,  lynx, lynx_state, empty_init, "Atari",  "Lynx II",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
