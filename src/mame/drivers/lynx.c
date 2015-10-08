// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT peter.trauner@utanet.at 2000,2001

 info found in bastian schick's bll
 and in cc65 for lynx

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m65sc02.h"
#include "audio/lynx.h"
#include "includes/lynx.h"

#include "lynx.lh"

static ADDRESS_MAP_START( lynx_mem , AS_PROGRAM, 8, lynx_state )
	AM_RANGE(0x0000, 0xfbff) AM_RAM AM_SHARE("mem_0000")
	AM_RANGE(0xfc00, 0xfcff) AM_RAM AM_SHARE("mem_fc00")
	AM_RANGE(0xfd00, 0xfdff) AM_RAM AM_SHARE("mem_fd00")
	AM_RANGE(0xfe00, 0xfff7) AM_READ_BANK("bank3") AM_WRITEONLY AM_SHARE("mem_fe00")
	AM_RANGE(0xfff8, 0xfff8) AM_RAM
	AM_RANGE(0xfff9, 0xfff9) AM_READWRITE(lynx_memory_config_r, lynx_memory_config_w)
	AM_RANGE(0xfffa, 0xffff) AM_READ_BANK("bank4") AM_WRITEONLY AM_SHARE("mem_fffa")
ADDRESS_MAP_END

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

PALETTE_INIT_MEMBER(lynx_state, lynx)
{
	int i;

	for (i=0; i< 0x1000; i++)
	{
		palette.set_pen_color(i,
			((i >> 0) & 0x0f) * 0x11,
			((i >> 4) & 0x0f) * 0x11,
			((i >> 8) & 0x0f) * 0x11);
	}
}

void lynx_state::video_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);
}

UINT32 lynx_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

// callback for Mikey call of shift(3) which shall act on the lynx_timer_count_down
void lynx_state::sound_cb()
{
	lynx_timer_count_down(1);
}

static MACHINE_CONFIG_START( lynx, lynx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65SC02, 4000000)        /* vti core, integrated in vlsi, stz, but not bbr bbs */
	MCFG_CPU_PROGRAM_MAP(lynx_mem)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(LCD_FRAMES_PER_SECOND)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(lynx_state, screen_update)
	MCFG_SCREEN_SIZE(160, 102)
	MCFG_SCREEN_VISIBLE_AREA(0, 160-1, 0, 102-1)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_DEFAULT_LAYOUT(layout_lynx)

	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_INIT_OWNER(lynx_state, lynx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", LYNX_SND, 0)
	MCFG_LYNX_SND_SET_TIMER(lynx_state, sound_cb)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_QUICKLOAD_ADD("quickload", lynx_state, lynx, "o", 0)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "lynx_cart")
	MCFG_GENERIC_EXTENSIONS("lnx,lyx")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_LOAD(lynx_state, lynx_cart)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","lynx")
MACHINE_CONFIG_END

#if 0
static MACHINE_CONFIG_DERIVED( lynx2, lynx )

	/* sound hardware */
	MCFG_DEVICE_REMOVE("mono")
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_DEVICE_REMOVE("lynx")
	MCFG_SOUND_ADD("custom", LYNX2_SND, 0)
	MCFG_LYNX_SND_SET_TIMER(lynx_state, sound_cb)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
MACHINE_CONFIG_END
#endif

/* these 2 dumps are saved from an running machine,
   and therefor the rom byte at 0xfff9 is not readable!
   (memory configuration)
   these 2 dumps differ only in this byte!
*/

ROM_START(lynx)
	ROM_REGION(0x200,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "default",   "rom save" )
	ROMX_LOAD( "lynx.bin",  0x00000, 0x200, BAD_DUMP CRC(e1ffecb6) SHA1(de60f2263851bbe10e5801ef8f6c357a4bc077e6), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "a", "alternate rom save" )
	ROMX_LOAD( "lynxa.bin", 0x00000, 0x200, BAD_DUMP CRC(0d973c9d) SHA1(e4ed47fae31693e016b081c6bda48da5b70d7ccb), ROM_BIOS(2))

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
	dynamic_buffer data;
	UINT8 *rom = memregion("maincpu")->base();
	UINT8 header[10]; // 80 08 dw Start dw Len B S 9 3
	UINT16 start, length;
	int i;

	if (image.fread( header, sizeof(header)) != sizeof(header))
		return IMAGE_INIT_FAIL;

	/* Check the image */
	if (lynx_verify_cart((char*)header, LYNX_QUICKLOAD) == IMAGE_VERIFY_FAIL)
		return IMAGE_INIT_FAIL;

	start = header[3] | (header[2]<<8); //! big endian format in file format for little endian cpu
	length = header[5] | (header[4]<<8);
	length -= 10;

	data.resize(length);

	if (image.fread( &data[0], length) != length)
	{
		return IMAGE_INIT_FAIL;
	}

	for (i = 0; i < length; i++)
		space.write_byte(start + i, data[i]);

	rom[0x1fc] = start & 0xff;
	rom[0x1fd] = start >> 8;
	space.write_byte(0x1fc, start & 0xff);
	space.write_byte(0x1fd, start >> 8);

	m_maincpu->set_pc(start);

	return IMAGE_INIT_PASS;
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY   FULLNAME      FLAGS */
CONS( 1989, lynx,   0,      0,      lynx,   lynx, driver_device,   0,       "Atari",  "Lynx", MACHINE_SUPPORTS_SAVE )
// CONS( 1991, lynx2,  lynx,   0,      lynx2,  lynx, driver_device,   0,       "Atari",  "Lynx II",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
