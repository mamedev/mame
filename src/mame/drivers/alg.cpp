// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**************************************************************************************

    American Laser Game Hardware

    Amiga 500 + sony ldp1450 laserdisc palyer

    Games Supported:

        Mad Dog McCree [3 versions]
        Who Shot Johnny Rock? [2 versions]
        Mad Dog II: The Lost Gold [2 versions]
        Space Pirates
        Gallagher's Gallery
        Crime Patrol
        Crime Patrol 2: Drug Wars [2 versions]
        The Last Bounty Hunter
        Fast Draw Showdown
        Platoon
        Zorton Brothers (Los Justicieros)

**************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "render.h"
#include "includes/amiga.h"
#include "machine/ldstub.h"
#include "machine/nvram.h"
#include "machine/amigafdc.h"


class alg_state : public amiga_state
{
public:
	alg_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag),
		m_laserdisc(*this, "laserdisc"),
		m_gun1x(*this, "GUN1X"),
		m_gun1y(*this, "GUN1Y"),
		m_gun2x(*this, "GUN2X"),
		m_gun2y(*this, "GUN2Y"),
		m_triggers(*this, "TRIGGERS")
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(lightgun_pos_r);
	DECLARE_CUSTOM_INPUT_MEMBER(lightgun_trigger_r);
	DECLARE_CUSTOM_INPUT_MEMBER(lightgun_holster_r);

	DECLARE_DRIVER_INIT(aplatoon);
	DECLARE_DRIVER_INIT(palr3);
	DECLARE_DRIVER_INIT(palr1);
	DECLARE_DRIVER_INIT(palr6);
	DECLARE_DRIVER_INIT(ntsc);
	DECLARE_DRIVER_INIT(pal);

	DECLARE_VIDEO_START(alg);

protected:
	// amiga_state overrides
	virtual void potgo_w(UINT16 data) override;
	int get_lightgun_pos(int player, int *x, int *y);

private:
	required_device<sony_ldp1450_device> m_laserdisc;
	required_ioport m_gun1x;
	required_ioport m_gun1y;
	optional_ioport m_gun2x;
	optional_ioport m_gun2y;
	optional_ioport m_triggers;

	UINT16 m_input_select;
};





/*************************************
 *
 *  Lightgun reading
 *
 *************************************/

int alg_state::get_lightgun_pos(int player, int *x, int *y)
{
	const rectangle &visarea = m_screen->visible_area();

	int xpos = (player == 0) ? m_gun1x->read() : (m_gun2x ? m_gun2x->read() : 0xffffffff);
	int ypos = (player == 0) ? m_gun1y->read() : (m_gun2y ? m_gun2y->read() : 0xffffffff);

	if (xpos == -1 || ypos == -1)
		return FALSE;

	*x = visarea.min_x + xpos * visarea.width() / 255;
	*y = visarea.min_y + ypos * visarea.height() / 255;
	return TRUE;
}




/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START_MEMBER(alg_state,alg)
{
	/* standard video start */
	VIDEO_START_CALL_MEMBER(amiga);

	/* configure pen 4096 as transparent in the renderer and use it for the genlock color */
	m_palette->set_pen_color(4096, rgb_t(0,0,0,0));
	amiga_set_genlock_color(machine(), 4096);
}




/*************************************
 *
 *  I/O ports
 *
 *************************************/

void alg_state::potgo_w(UINT16 data)
{
	/* bit 15 controls whether pin 9 is input/output */
	/* bit 14 controls the value, which selects which player's controls to read */
	m_input_select = (data & 0x8000) ? ((data >> 14) & 1) : 0;
}


CUSTOM_INPUT_MEMBER(alg_state::lightgun_pos_r)
{
	int x = 0, y = 0;

	/* get the position based on the input select */
	get_lightgun_pos(m_input_select, &x, &y);
	return (y << 8) | (x >> 2);
}


CUSTOM_INPUT_MEMBER(alg_state::lightgun_trigger_r)
{
	/* read the trigger control based on the input select */
	return (m_triggers->read() >> m_input_select) & 1;
}


CUSTOM_INPUT_MEMBER(alg_state::lightgun_holster_r)
{
	/* read the holster control based on the input select */
	return (m_triggers->read() >> (2 + m_input_select)) & 1;
}




/*************************************
 *
 *  Memory map
 *
 *************************************/

static ADDRESS_MAP_START( overlay_512kb_map, AS_PROGRAM, 16, alg_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_MIRROR(0x180000) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( a500_mem, AS_PROGRAM, 16, alg_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_DEVICE("overlay", address_map_bank_device, amap16)
	AM_RANGE(0xa00000, 0xbfffff) AM_READWRITE(cia_r, cia_w)
	AM_RANGE(0xc00000, 0xd7ffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xd80000, 0xddffff) AM_NOP
	AM_RANGE(0xde0000, 0xdeffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xe00000, 0xe7ffff) AM_WRITENOP AM_READ(rom_mirror_r)
	AM_RANGE(0xe80000, 0xefffff) AM_NOP // autoconfig space (installed by devices)
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_map_r1, AS_PROGRAM, 16, alg_state )
	AM_IMPORT_FROM(a500_mem)
	AM_RANGE(0xf00000, 0xf1ffff) AM_ROM AM_REGION("user2", 0)           /* Custom ROM */
	AM_RANGE(0xf54000, 0xf55fff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_map_r2, AS_PROGRAM, 16, alg_state )
	AM_IMPORT_FROM(a500_mem)
	AM_RANGE(0xf00000, 0xf3ffff) AM_ROM AM_REGION("user2", 0)           /* Custom ROM */
	AM_RANGE(0xf7c000, 0xf7dfff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_map_picmatic, AS_PROGRAM, 16, alg_state )
	AM_IMPORT_FROM(a500_mem)
	AM_RANGE(0xf00000, 0xf1ffff) AM_ROM AM_REGION("user2", 0)           /* Custom ROM */
	AM_RANGE(0xf40000, 0xf41fff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( alg )
	PORT_START("joy_0_dat")   /* read by Amiga core */
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, alg_state,amiga_joystick_convert, (void *)0)
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("joy_1_dat")   /* read by Amiga core */
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, alg_state,amiga_joystick_convert, (void *)1)
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("potgo")     /* read by Amiga core */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xaaff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("HVPOS")     /* read by Amiga core */
	PORT_BIT( 0x1ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, alg_state,lightgun_pos_r, NULL)

	PORT_START("FIRE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("p1_joy")     /* referenced by JOY0DAT */
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_HIGH )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("p2_joy")     /* referenced by JOY1DAT */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GUN1X")     /* referenced by lightgun_pos_r */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUN1Y")     /* referenced by lightgun_pos_r */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( alg_2p )
	PORT_INCLUDE(alg)

	PORT_MODIFY("potgo")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, alg_state,lightgun_trigger_r, NULL)

	PORT_MODIFY("FIRE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, alg_state,lightgun_holster_r, NULL)

	PORT_START("GUN2X")     /* referenced by lightgun_pos_r */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUN2Y")     /* referenced by lightgun_pos_r */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("TRIGGERS")  /* referenced by lightgun_trigger_r and lightgun_holster_r */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( alg_r1, alg_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, amiga_state::CLK_7M_NTSC)
	MCFG_CPU_PROGRAM_MAP(main_map_r1)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(overlay_512kb_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_FRAGMENT_ADD(ntsc_video)

	MCFG_LASERDISC_LDP1450_ADD("laserdisc")
	MCFG_LASERDISC_SCREEN("screen")
	MCFG_LASERDISC_OVERLAY_DRIVER(512*2, 262, amiga_state, screen_update_amiga)
	MCFG_LASERDISC_OVERLAY_CLIP((129-8)*2, (449+8-1)*2, 44-8, 244+8-1)
	MCFG_LASERDISC_OVERLAY_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 4097)
	MCFG_PALETTE_INIT_OWNER(alg_state,amiga)

	MCFG_VIDEO_START_OVERRIDE(alg_state,alg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("amiga", AMIGA, amiga_state::CLK_C1_NTSC)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(3, "lspeaker", 0.25)

	MCFG_SOUND_MODIFY("laserdisc")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* cia */
	MCFG_DEVICE_ADD("cia_0", MOS8520, amiga_state::CLK_E_NTSC)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(amiga_state, cia_0_irq))
	MCFG_MOS6526_PA_INPUT_CALLBACK(IOPORT("FIRE"))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(amiga_state, cia_0_port_a_write))
	MCFG_DEVICE_ADD("cia_1", MOS8520, amiga_state::CLK_E_NTSC)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(amiga_state, cia_1_irq))

	/* fdc */
	MCFG_DEVICE_ADD("fdc", AMIGA_FDC, amiga_state::CLK_7M_NTSC)
	MCFG_AMIGA_FDC_INDEX_CALLBACK(DEVWRITELINE("cia_1", mos8520_device, flag_w))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( alg_r2, alg_r1 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(main_map_r2)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( picmatic, alg_r1 )
	/* adjust for PAL specs */
	MCFG_CPU_REPLACE("maincpu", M68000, amiga_state::CLK_7M_PAL)
	MCFG_CPU_PROGRAM_MAP(main_map_picmatic)

	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(pal_video)

	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_PAL)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_PAL)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_PAL)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_PAL)
MACHINE_CONFIG_END



/*************************************
 *
 *  BIOS definitions
 *
 *************************************/

#define ALG_BIOS \
	ROM_REGION16_BE(0x80000, "kickstart", 0) \
	ROM_SYSTEM_BIOS(0, "kick13",  "Kickstart 1.3 (34.5)") \
	ROMX_LOAD("315093-02.u2", 0x00000, 0x40000, CRC(c4f0f55f) SHA1(891e9a547772fe0c6c19b610baf8bc4ea7fcb785), ROM_GROUPWORD | ROM_BIOS(1)) \
	ROM_COPY("kickstart", 0x000000, 0x040000, 0x040000)



/*************************************
*
*  ROM definitions
*
*************************************/

/* BIOS */
ROM_START( alg_bios )
	ALG_BIOS

	ROM_REGION( 0x20000, "user2", ROMREGION_ERASEFF )
ROM_END


/* Rev. A board */
/* PAL R1 */
ROM_START( maddoga )
	ALG_BIOS

	ROM_REGION( 0x20000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "maddog_01.dat", 0x000000, 0x10000, CRC(04572557) SHA1(3dfe2ce94ced8701a3e73ed5869b6fbe1c8b3286) )
	ROM_LOAD16_BYTE( "maddog_02.dat", 0x000001, 0x10000, CRC(f64014ec) SHA1(d343a2cb5d8992153b8c916f39b11d3db736543d))

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog", 0, NO_DUMP )
ROM_END


/* PAL R3 */
ROM_START( wsjr )  /* 1.6 */
	ALG_BIOS

	ROM_REGION( 0x20000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "johnny_01.bin", 0x000000, 0x10000, CRC(edde1745) SHA1(573b79f8808fedaabf3b762350a915792d26c1bc) )
	ROM_LOAD16_BYTE( "johnny_02.bin", 0x000001, 0x10000, CRC(046569b3) SHA1(efe5a8b2be1c555695f2a91c88951d3545f1b915) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "wsjr", 0, NO_DUMP )
ROM_END

ROM_START( wsjr_15 )  /* 1.5 */
	ALG_BIOS

	ROM_REGION( 0x20000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wsjr151.bin", 0x000000, 0x10000, CRC(9beeb1d7) SHA1(3fe0265e5d36103d3d9557d75e5e3728e0b30da7) )
	ROM_LOAD16_BYTE( "wsjr152.bin", 0x000001, 0x10000, CRC(8ab626dd) SHA1(e45561f77fc279b71dc1dd2e15a0870cb5c1cd89) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "wsjr", 0, NO_DUMP )
ROM_END


//REV.B
ROM_START( maddog )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md_2.03_1.bin", 0x000000, 0x20000, CRC(6f5b8f2d) SHA1(bbf32bb27a998d53744411d75efdbdb730855809) )
	ROM_LOAD16_BYTE( "md_2.03_2.bin", 0x000001, 0x20000, CRC(a50d3c04) SHA1(4cf100fdb5b2f2236539fd0ec33b3db19c64a6b8) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog", 0, NO_DUMP )
ROM_END


ROM_START( maddog_202 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md_2.02_u1.bin", 0x000000, 0x20000, CRC(a49890d1) SHA1(148f78fb426f5b912e8c3836a149bfcb966da477) )
	ROM_LOAD16_BYTE( "md_2.02_u2.bin", 0x000001, 0x20000, CRC(f46e1242) SHA1(2960bc1800b22eea50036ae43fd3cb2ab7dcf8a4) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog", 0, NO_DUMP )
ROM_END

ROM_START( maddog2 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_01_v.2.04.bin", 0x000000, 0x20000, CRC(0e1227f4) SHA1(bfd9081bb7d2bcbb77357839f292ce6136e9b228) )
	ROM_LOAD16_BYTE( "md2_02_v.2.04.bin", 0x000001, 0x20000, CRC(361bd99c) SHA1(5de6ef38e334e19f509227de7880306ac984ec23) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog2", 0, NO_DUMP )
ROM_END

ROM_START( maddog2_202 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_01.bin", 0x000000, 0x20000, CRC(4092227f) SHA1(6e5393aa5e64b59887260f483c50960084de7bd1) )
	ROM_LOAD16_BYTE( "md2_02.bin", 0x000001, 0x20000, CRC(addffa51) SHA1(665e9d93ddfa6b2ea5d006b41bf7eac3294244cc) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog2", 0, NO_DUMP )
ROM_END

ROM_START( maddog2_110 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_1.10_u1.bin", 0x000000, 0x20000, CRC(61808612) SHA1(1a0a301e79585a81e6cf46737068970fb8a205fa) )
	ROM_LOAD16_BYTE( "md2_1.10_u2.bin", 0x000001, 0x20000, CRC(0e113b2c) SHA1(739d777f3cb92fbc730c6c1b664d874121d191b1) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog2", 0, NO_DUMP )
ROM_END

ROM_START( maddog2_100 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_1.0_1.bin", 0x000000, 0x20000, CRC(97272a1d) SHA1(109014647c491f019ffb21091c7d0b89e1755b75) )
	ROM_LOAD16_BYTE( "md2_1.0_2.bin", 0x000001, 0x20000, CRC(0ce8db97) SHA1(dd4c09db59bb8c6caba935b1b28babe28ba8516b) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog2", 0, NO_DUMP )
ROM_END


ROM_START( spacepir )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sp_02.dat", 0x000000, 0x20000, CRC(10d162a2) SHA1(26833d5be1057be8639c00a7be18be33404ea751) )
	ROM_LOAD16_BYTE( "sp_01.dat", 0x000001, 0x20000, CRC(c0975188) SHA1(fd7643dc972e7861249ab7e76199975984888ae4) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "spacepir", 0, NO_DUMP )
ROM_END


ROM_START( spacepir_14 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sp_14_u1.bin", 0x000000, 0x20000, CRC(30390ab0) SHA1(80fa14d881902258398225bdfd71ed5e9e2d6c91) )
	ROM_LOAD16_BYTE( "sp_14_u2.bin", 0x000001, 0x20000, CRC(4102988c) SHA1(969d4668be50990c7debf9ed4e8b8c6e422acdf5) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "spacepir", 0, NO_DUMP )
ROM_END

ROM_START( gallgall )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gg_1.dat", 0x000000, 0x20000, CRC(3793b211) SHA1(dccb1d9c5e2d6a4d249426ae6348e9fc9b72e665)  )
	ROM_LOAD16_BYTE( "gg_2.dat", 0x000001, 0x20000,  CRC(855c9d82) SHA1(96711aaa02f309cacd3e8d8efbe95cfc811aba96) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "gallgall", 0, NO_DUMP )
ROM_END

ROM_START( gallgall_21 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gg_21_rom1.bin", 0x000000, 0x20000, CRC(4109f39e) SHA1(42d06de42c56f21e4899b4c4252baabb51f24fda)  )
	ROM_LOAD16_BYTE( "gg_21_rom2.bin", 0x000001, 0x20000, CRC(70f887e5) SHA1(cd6cedc85bbe67674dfd140fed9018778f8cd8db) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "gallgall", 0, NO_DUMP )
ROM_END

ROM_START( crimepat )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp_151_u1.bin", 0x000000, 0x20000, CRC(f2270cee) SHA1(1e735373723c3cffb6dd04d5c30c08e757deae61) )
	ROM_LOAD16_BYTE( "cp_151_u2.bin", 0x000001, 0x20000, CRC(aefd6e09) SHA1(351e665e2e6368047a5cc80d0f9d876bca068b6a) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimepat", 0, NO_DUMP )
ROM_END

ROM_START( crimepat_14 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp02.dat", 0x000000, 0x20000, CRC(a39a8b50) SHA1(55ca317ef13c3a42f12d68c480e6cc2d4459f6a4) )
	ROM_LOAD16_BYTE( "cp01.dat", 0x000001, 0x20000, CRC(e41fd2e8) SHA1(1cd9875fb4133ba4e3616271975dc736b343f156) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimepat", 0, NO_DUMP )
ROM_END

ROM_START( crimepat_12 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp_1.20_u1.bin", 0x000000, 0x20000, CRC(814f5777) SHA1(341a1d7b64112af3e8243bdbfec72e7fa85aa903) )
	ROM_LOAD16_BYTE( "cp_1.20_u2.bin", 0x000001, 0x20000, CRC(475e847a) SHA1(82fd160835758cd51ea22f90b08921a40409f94d) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimepat", 0, NO_DUMP )
ROM_END

ROM_START( crimep2 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp2_1.3_1.bin", 0x000000, 0x20000, CRC(e653395d) SHA1(8f6c86d98a52b7d85ae285fd841167cd07979318) )
	ROM_LOAD16_BYTE( "cp2_1.3_2.bin", 0x000001, 0x20000, CRC(dbdaa79a) SHA1(998044909d5c93e3bd1baafefab818fdb7b3f55e) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimep2", 0, NO_DUMP )
ROM_END

ROM_START( crimep2_11 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp2_1.dat", 0x000000, 0x20000, CRC(47879042) SHA1(8bb6c541e4e8e4508da8d4b93600176a2e7a1f41) )
	ROM_LOAD16_BYTE( "cp2_2.dat", 0x000001, 0x20000, CRC(f4e5251e) SHA1(e0c91343a98193d487c40e7a85f542b2a7a88f03) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimep2", 0, NO_DUMP )
ROM_END


ROM_START( lastbh )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "lbh_101_u1.bin", 0x000000, 0x20000, CRC(f13b25d2) SHA1(e2f663c23b03592f482ef5e1df87b651937a500d) )
	ROM_LOAD16_BYTE( "lbh_101_u2.bin", 0x000001, 0x20000, CRC(b21c5c42) SHA1(9ac856cdf2c9538cc4ae55f079f337376d3361c0) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "lastbh", 0, NO_DUMP )
ROM_END

ROM_START( lastbh_006 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "bounty_01.bin", 0x000000, 0x20000, CRC(977566b2) SHA1(937e079e992ecb5930b17c1024c326e10962642b) )
	ROM_LOAD16_BYTE( "bounty_02.bin", 0x000001, 0x20000, CRC(2727ef1d) SHA1(f53421390b65c21a7666ff9d0f53ebf2a463d836) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "lastbh", 0, NO_DUMP )
ROM_END

ROM_START( fastdraw )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fd_131_u1.bin", 0x000000, 0x20000, CRC(b7c79ab3) SHA1(6eca1bc9590c22a004fb85901e5c7d41f5b14ee2) )
	ROM_LOAD16_BYTE( "fd_131_u2.bin", 0x000001, 0x20000, CRC(e1ed7982) SHA1(f7562c6e0ce6bf1a9885cc593e08c2509f82bbe1) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "fastdraw", 0, NO_DUMP )
ROM_END

ROM_START( fastdraw_130 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fast_01.bin", 0x000000, 0x20000, CRC(4c4eb71e) SHA1(3bd487c546b6c80770a5fc880dcb10395ca431a2) )
	ROM_LOAD16_BYTE( "fast_02.bin", 0x000001, 0x20000, CRC(0d76a2da) SHA1(d396371ae1b9b0b6e6bc6f1f85c4b97bfc5dc34d) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "fastdraw", 0, NO_DUMP )
ROM_END

ROM_START( aplatoon )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "platoonv4u1.bin", 0x000000, 0x20000, CRC(8b33263e) SHA1(a1df38236321af90b522e2a783984fdf02e4c597) )
	ROM_LOAD16_BYTE( "platoonv4u2.bin", 0x000001, 0x20000, CRC(09a133cf) SHA1(9b3ff63035be8576c88fb284a25c2da5db0d5160) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "platoon", 0, NO_DUMP )
ROM_END

ROM_START( zortonbr )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "zb_u2.bin", 0x000000, 0x10000, CRC(938b25cb) SHA1(d0114bbc588dcfce6a469013d0e35afb93e38af5) )
	ROM_LOAD16_BYTE( "zb_u3.bin", 0x000001, 0x10000, CRC(f59cfc4a) SHA1(9fadf7f1e23d6b4e828bf2b3de919d087c690a3f) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "zortonbr", 0, NO_DUMP )
ROM_END



/*************************************
 *
 *  Per-game decryption
 *
 *************************************/

DRIVER_INIT_MEMBER( alg_state, ntsc )
{
	m_agnus_id = AGNUS_NTSC;
	m_denise_id = DENISE;
}

DRIVER_INIT_MEMBER( alg_state, pal )
{
	m_agnus_id = AGNUS_PAL;
	m_denise_id = DENISE;
}

DRIVER_INIT_MEMBER(alg_state,palr1)
{
	DRIVER_INIT_CALL(ntsc);

	UINT32 length = memregion("user2")->bytes();
	UINT8 *rom = memregion("user2")->base();
	dynamic_buffer original(length);
	UINT32 srcaddr;

	memcpy(&original[0], rom, length);
	for (srcaddr = 0; srcaddr < length; srcaddr++)
	{
		UINT32 dstaddr = srcaddr;
		if (srcaddr & 0x2000) dstaddr ^= 0x1000;
		if (srcaddr & 0x8000) dstaddr ^= 0x4000;
		rom[dstaddr] = original[srcaddr];
	}
}

DRIVER_INIT_MEMBER(alg_state,palr3)
{
	DRIVER_INIT_CALL(ntsc);

	UINT32 length = memregion("user2")->bytes();
	UINT8 *rom = memregion("user2")->base();
	dynamic_buffer original(length);
	UINT32 srcaddr;

	memcpy(&original[0], rom, length);
	for (srcaddr = 0; srcaddr < length; srcaddr++)
	{
		UINT32 dstaddr = srcaddr;
		if (srcaddr & 0x2000) dstaddr ^= 0x1000;
		rom[dstaddr] = original[srcaddr];
	}
}

DRIVER_INIT_MEMBER(alg_state,palr6)
{
	DRIVER_INIT_CALL(ntsc);

	UINT32 length = memregion("user2")->bytes();
	UINT8 *rom = memregion("user2")->base();
	dynamic_buffer original(length);
	UINT32 srcaddr;

	memcpy(&original[0], rom, length);
	for (srcaddr = 0; srcaddr < length; srcaddr++)
	{
		UINT32 dstaddr = srcaddr;
		if (~srcaddr & 0x2000) dstaddr ^= 0x1000;
		if ( srcaddr & 0x8000) dstaddr ^= 0x4000;
		dstaddr ^= 0x20000;
		rom[dstaddr] = original[srcaddr];
	}
}

DRIVER_INIT_MEMBER(alg_state,aplatoon)
{
	DRIVER_INIT_CALL(ntsc);

	/* NOT DONE TODO FIGURE OUT THE RIGHT ORDER!!!! */
	UINT8 *rom = memregion("user2")->base();
	std::unique_ptr<UINT8[]> decrypted = std::make_unique<UINT8[]>(0x40000);
	int i;

	static const int shuffle[] =
	{
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
		32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
	};

	for (i = 0; i < 64; i++)
		memcpy(decrypted.get() + i * 0x1000, rom + shuffle[i] * 0x1000, 0x1000);
	memcpy(rom, decrypted.get(), 0x40000);
	logerror("decrypt done\n ");
}




/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* BIOS */
GAME( 199?, alg_bios,   0,         alg_r1,   alg, alg_state,    ntsc,     ROT0,  "American Laser Games", "American Laser Games BIOS", MACHINE_IS_BIOS_ROOT )

/* Rev. A board */
/* PAL R1 */
GAME( 1990, maddoga,     maddog,   alg_r1,   alg, alg_state,    palr1,    ROT0,  "American Laser Games", "Mad Dog McCree v1C board rev.A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

/* PAL R3 */
GAME( 1991, wsjr,        alg_bios, alg_r1,   alg, alg_state,    palr3,    ROT0,  "American Laser Games", "Who Shot Johnny Rock? v1.6", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, wsjr_15,     wsjr,     alg_r1,   alg, alg_state,    palr3,    ROT0,  "American Laser Games", "Who Shot Johnny Rock? v1.5", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

/* Rev. B board */
/* PAL R6 */

GAME( 1990, maddog,      alg_bios, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Mad Dog McCree v2.03 board rev.B", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1990, maddog_202,  maddog, alg_r2,   alg_2p, alg_state, palr6,      ROT0,  "American Laser Games", "Mad Dog McCree v2.02 board rev.B", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

	/* works ok but uses right player (2) controls only for trigger and holster */
GAME( 1992, maddog2,     alg_bios, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold v2.04", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, maddog2_202, maddog2, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold v2.02", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, maddog2_110, maddog2,  alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold v1.10", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, maddog2_100, maddog2,  alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold v1.00", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
	/* works ok but uses right player (2) controls only for trigger and holster */
GAME( 1992, spacepir,    alg_bios, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Space Pirates v2.2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, spacepir_14, spacepir, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Space Pirates v1.4", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, gallgall,    alg_bios, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Gallagher's Gallery v2.2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, gallgall_21, gallgall, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Gallagher's Gallery v2.1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
	/* all good, but no holster */
GAME( 1993, crimepat,    alg_bios, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Crime Patrol v1.51", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, crimepat_14, crimepat, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Crime Patrol v1.4", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, crimepat_12, crimepat, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Crime Patrol v1.2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1993, crimep2,     alg_bios, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Crime Patrol 2: Drug Wars v1.3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, crimep2_11,  crimep2,  alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "Crime Patrol 2: Drug Wars v1.1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, lastbh,      alg_bios, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "The Last Bounty Hunter v1.01", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, lastbh_006,  lastbh, alg_r2,   alg_2p, alg_state, palr6,    ROT0,  "American Laser Games", "The Last Bounty Hunter v0.06", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, fastdraw,    alg_bios, alg_r2,   alg_2p, alg_state, palr6,    ROT90, "American Laser Games", "Fast Draw Showdown v1.31", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, fastdraw_130,fastdraw, alg_r2,   alg_2p, alg_state, palr6,    ROT90, "American Laser Games", "Fast Draw Showdown v1.30", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
	/* works ok but uses right player (2) controls only for trigger and holster */

/* NOVA games on ALG hardware with own address scramble */
GAME( 199?, aplatoon, alg_bios, alg_r2,   alg, alg_state,    aplatoon, ROT0,  "Nova?", "Platoon V.3.1 US", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

/* Web Picmatic games PAL tv standard, own rom board */
GAME( 1993, zortonbr, alg_bios, picmatic, alg, alg_state,    pal,     ROT0,  "Web Picmatic", "Zorton Brothers (Los Justicieros)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
