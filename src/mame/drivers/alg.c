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
#include "machine/6526cia.h"
#include "machine/nvram.h"
#include "machine/amigafdc.h"


class alg_state : public amiga_state
{
public:
	alg_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag),
		  m_laserdisc(*this, "laserdisc") { }


	required_device<sony_ldp1450_device> m_laserdisc;
	emu_timer *m_serial_timer;
	UINT8 m_serial_timer_active;
	UINT16 m_input_select;
	DECLARE_CUSTOM_INPUT_MEMBER(lightgun_pos_r);
	DECLARE_CUSTOM_INPUT_MEMBER(lightgun_trigger_r);
	DECLARE_CUSTOM_INPUT_MEMBER(lightgun_holster_r);
};

static TIMER_CALLBACK( response_timer );



/*************************************
 *
 *  Lightgun reading
 *
 *************************************/

static int get_lightgun_pos(screen_device &screen, int player, int *x, int *y)
{
	const rectangle &visarea = screen.visible_area();

	int xpos = input_port_read_safe(screen.machine(), (player == 0) ? "GUN1X" : "GUN2X", 0xffffffff);
	int ypos = input_port_read_safe(screen.machine(), (player == 0) ? "GUN1Y" : "GUN2Y", 0xffffffff);

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

static VIDEO_START( alg )
{
	/* standard video start */
	VIDEO_START_CALL(amiga);

	/* configure pen 4096 as transparent in the renderer and use it for the genlock color */
	palette_set_color(machine, 4096, MAKE_ARGB(0,0,0,0));
	amiga_set_genlock_color(machine, 4096);
}



/*************************************
 *
 *  Machine start/reset
 *
 *************************************/

static MACHINE_START( alg )
{
	alg_state *state = machine.driver_data<alg_state>();

	state->m_serial_timer = machine.scheduler().timer_alloc(FUNC(response_timer));
	state->m_serial_timer_active = FALSE;
}


static MACHINE_RESET( alg )
{
	MACHINE_RESET_CALL(amiga);
}



/*************************************
 *
 *  Laserdisc communication
 *
 *************************************/

static TIMER_CALLBACK( response_timer )
{
	alg_state *state = machine.driver_data<alg_state>();

	/* if we still have data to send, do it now */
	if (state->m_laserdisc->data_available_r() == ASSERT_LINE)
	{
		UINT8 data = state->m_laserdisc->data_r();
		if (data != 0x0a)
			mame_printf_debug("Sending serial data = %02X\n", data);
		amiga_serial_in_w(machine, data);
	}

	/* if there's more to come, set another timer */
	if (state->m_laserdisc->data_available_r() == ASSERT_LINE)
		state->m_serial_timer->adjust(amiga_get_serial_char_period(machine));
	else
		state->m_serial_timer_active = FALSE;
}


static void vsync_callback(running_machine &machine)
{
	alg_state *state = machine.driver_data<alg_state>();

	/* if we have data available, set a timer to read it */
	if (!state->m_serial_timer_active && state->m_laserdisc->data_available_r() == ASSERT_LINE)
	{
		state->m_serial_timer->adjust(amiga_get_serial_char_period(machine));
		state->m_serial_timer_active = TRUE;
	}
}


static void serial_w(running_machine &machine, UINT16 data)
{
	alg_state *state = machine.driver_data<alg_state>();

	/* write to the laserdisc player */
	state->m_laserdisc->data_w(data & 0xff);

	/* if we have data available, set a timer to read it */
	if (!state->m_serial_timer_active && state->m_laserdisc->data_available_r() == ASSERT_LINE)
	{
		state->m_serial_timer->adjust(amiga_get_serial_char_period(machine));
		state->m_serial_timer_active = TRUE;
	}
}



/*************************************
 *
 *  I/O ports
 *
 *************************************/

static void alg_potgo_w(running_machine &machine, UINT16 data)
{
	alg_state *state = machine.driver_data<alg_state>();

	/* bit 15 controls whether pin 9 is input/output */
	/* bit 14 controls the value, which selects which player's controls to read */
	state->m_input_select = (data & 0x8000) ? ((data >> 14) & 1) : 0;
}


CUSTOM_INPUT_MEMBER(alg_state::lightgun_pos_r)
{
	int x = 0, y = 0;

	/* get the position based on the input select */
	get_lightgun_pos(*machine().primary_screen, m_input_select, &x, &y);
	return (y << 8) | (x >> 2);
}


CUSTOM_INPUT_MEMBER(alg_state::lightgun_trigger_r)
{

	/* read the trigger control based on the input select */
	return (input_port_read(machine(), "TRIGGERS") >> m_input_select) & 1;
}


CUSTOM_INPUT_MEMBER(alg_state::lightgun_holster_r)
{

	/* read the holster control based on the input select */
	return (input_port_read(machine(), "TRIGGERS") >> (2 + m_input_select)) & 1;
}



/*************************************
 *
 *  CIA port accesses
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( alg_cia_0_porta_w )
{
	address_space *space = device->machine().device("maincpu")->memory().space(AS_PROGRAM);

	/* switch banks as appropriate */
	device->machine().root_device().membank("bank1")->set_entry(data & 1);

	/* swap the write handlers between ROM and bank 1 based on the bit */
	if ((data & 1) == 0)
		/* overlay disabled, map RAM on 0x000000 */
		space->install_write_bank(0x000000, 0x07ffff, "bank1");

	else
		/* overlay enabled, map Amiga system ROM on 0x000000 */
		space->unmap_write(0x000000, 0x07ffff);
}


static READ8_DEVICE_HANDLER( alg_cia_0_porta_r )
{
	return input_port_read(device->machine(), "FIRE") | 0x3f;
}


static READ8_DEVICE_HANDLER( alg_cia_0_portb_r )
{
	logerror("%s:alg_cia_0_portb_r\n", device->machine().describe_context());
	return 0xff;
}


static WRITE8_DEVICE_HANDLER( alg_cia_0_portb_w )
{
	/* parallel port */
	logerror("%s:alg_cia_0_portb_w(%02x)\n", device->machine().describe_context(), data);
}


static READ8_DEVICE_HANDLER( alg_cia_1_porta_r )
{
	logerror("%s:alg_cia_1_porta_r\n", device->machine().describe_context());
	return 0xff;
}


static WRITE8_DEVICE_HANDLER( alg_cia_1_porta_w )
{
	logerror("%s:alg_cia_1_porta_w(%02x)\n", device->machine().describe_context(), data);
}



/*************************************
 *
 *  Memory map
 *
 *************************************/

static ADDRESS_MAP_START( main_map_r1, AS_PROGRAM, 16, alg_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_RAMBANK("bank1") AM_SHARE("chip_ram")
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE_LEGACY(amiga_cia_r, amiga_cia_w)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE_LEGACY(amiga_custom_r, amiga_custom_w) AM_SHARE("custom_regs")
	AM_RANGE(0xe80000, 0xe8ffff) AM_READWRITE_LEGACY(amiga_autoconfig_r, amiga_autoconfig_w)
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("user1", 0)			/* System ROM */

	AM_RANGE(0xf00000, 0xf1ffff) AM_ROM AM_REGION("user2", 0)			/* Custom ROM */
	AM_RANGE(0xf54000, 0xf55fff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( main_map_r2, AS_PROGRAM, 16, alg_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_RAMBANK("bank1") AM_SHARE("chip_ram")
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE_LEGACY(amiga_cia_r, amiga_cia_w)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE_LEGACY(amiga_custom_r, amiga_custom_w) AM_SHARE("custom_regs")
	AM_RANGE(0xe80000, 0xe8ffff) AM_READWRITE_LEGACY(amiga_autoconfig_r, amiga_autoconfig_w)
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("user1", 0)			/* System ROM */

	AM_RANGE(0xf00000, 0xf3ffff) AM_ROM AM_REGION("user2", 0)			/* Custom ROM */
	AM_RANGE(0xf7c000, 0xf7dfff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( main_map_picmatic, AS_PROGRAM, 16, alg_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_RAMBANK("bank1") AM_SHARE("chip_ram")
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE_LEGACY(amiga_cia_r, amiga_cia_w)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE_LEGACY(amiga_custom_r, amiga_custom_w) AM_SHARE("custom_regs")
	AM_RANGE(0xe80000, 0xe8ffff) AM_READWRITE_LEGACY(amiga_autoconfig_r, amiga_autoconfig_w)
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("user1", 0)			/* System ROM */

	AM_RANGE(0xf00000, 0xf1ffff) AM_ROM AM_REGION("user2", 0)			/* Custom ROM */
	AM_RANGE(0xf40000, 0xf41fff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( alg )
	PORT_START("JOY0DAT")	/* read by Amiga core */
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, alg_state,amiga_joystick_convert, "P1JOY")
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("JOY1DAT")	/* read by Amiga core */
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, alg_state,amiga_joystick_convert, "P2JOY")
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("POTGO")		/* read by Amiga core */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xaaff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("HVPOS")		/* read by Amiga core */
	PORT_BIT( 0x1ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, alg_state,lightgun_pos_r, NULL)

	PORT_START("FIRE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1JOY")		/* referenced by JOY0DAT */
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_HIGH )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("P2JOY")		/* referenced by JOY1DAT */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GUN1X")		/* referenced by lightgun_pos_r */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUN1Y")		/* referenced by lightgun_pos_r */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( alg_2p )
	PORT_INCLUDE(alg)

	PORT_MODIFY("POTGO")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, alg_state,lightgun_trigger_r, NULL)

	PORT_MODIFY("FIRE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, alg_state,lightgun_holster_r, NULL)

	PORT_START("GUN2X")		/* referenced by lightgun_pos_r */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUN2Y")		/* referenced by lightgun_pos_r */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("TRIGGERS")	/* referenced by lightgun_trigger_r and lightgun_holster_r */
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

static const mos6526_interface cia_0_intf =
{
	0,												/* tod_clock */
	DEVCB_LINE(amiga_cia_0_irq),								/* irq_func */
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(alg_cia_0_porta_r),
	DEVCB_HANDLER(alg_cia_0_porta_w),	/* port A */
	DEVCB_HANDLER(alg_cia_0_portb_r),
	DEVCB_HANDLER(alg_cia_0_portb_w)	/* port B */
};

static const mos6526_interface cia_1_intf =
{
	0,												/* tod_clock */
	DEVCB_LINE(amiga_cia_1_irq),								/* irq_func */
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(alg_cia_1_porta_r),
	DEVCB_HANDLER(alg_cia_1_porta_w),	/* port A */
	DEVCB_NULL,
	DEVCB_NULL								/* port B */
};

static MACHINE_CONFIG_START( alg_r1, alg_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, AMIGA_68000_NTSC_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map_r1)

	MCFG_MACHINE_START(alg)
	MCFG_MACHINE_RESET(alg)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_LASERDISC_LDP1450_ADD("laserdisc")
	MCFG_LASERDISC_OVERLAY_STATIC(512*2, 262, amiga)
	MCFG_LASERDISC_OVERLAY_CLIP((129-8)*2, (449+8-1)*2, 44-8, 244+8-1)

	/* video hardware */
	MCFG_LASERDISC_SCREEN_ADD_NTSC("screen", "laserdisc")
	MCFG_SCREEN_REFRESH_RATE(59.997)
	MCFG_SCREEN_SIZE(512*2, 262)
	MCFG_SCREEN_VISIBLE_AREA((129-8)*2, (449+8-1)*2, 44-8, 244+8-1)

	MCFG_PALETTE_LENGTH(4097)
	MCFG_PALETTE_INIT(amiga)

	MCFG_VIDEO_START(alg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("amiga", AMIGA, 3579545)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(3, "lspeaker", 0.25)

	MCFG_SOUND_MODIFY("laserdisc")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* cia */
	MCFG_MOS8520_ADD("cia_0", AMIGA_68000_NTSC_CLOCK / 10, cia_0_intf)
	MCFG_MOS8520_ADD("cia_1", AMIGA_68000_NTSC_CLOCK / 10, cia_1_intf)

	/* fdc */
	MCFG_AMIGA_FDC_ADD("fdc", AMIGA_68000_NTSC_CLOCK)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( alg_r2, alg_r1 )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(main_map_r2)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( picmatic, alg_r1 )

	/* adjust for PAL specs */
	MCFG_CPU_REPLACE("maincpu", M68000, AMIGA_68000_PAL_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map_picmatic)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(512*2, 312)
	MCFG_SCREEN_VISIBLE_AREA((129-8)*2, (449+8-1)*2, 44-8, 300+8-1)
MACHINE_CONFIG_END



/*************************************
 *
 *  BIOS definitions
 *
 *************************************/

#define ROM_LOAD16_WORD_BIOS(bios,name,offset,length,hash)     ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1))

#define ALG_BIOS \
	ROM_REGION16_BE( 0x80000, "user1", 0 ) \
	ROM_SYSTEM_BIOS( 0, "Kick1.3", "Kickstart 1.3") \
	ROM_LOAD16_WORD_BIOS(0, "kick13.rom", 0x000000, 0x40000, CRC(c4f0f55f) SHA1(891e9a547772fe0c6c19b610baf8bc4ea7fcb785)) \
	ROM_COPY( "user1", 0x000000, 0x040000, 0x040000 )



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
ROM_END

ROM_START( wsjr15 )  /* 1.5 */
	ALG_BIOS

	ROM_REGION( 0x20000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wsjr151.bin", 0x000000, 0x10000, CRC(9beeb1d7) SHA1(3fe0265e5d36103d3d9557d75e5e3728e0b30da7) )
	ROM_LOAD16_BYTE( "wsjr152.bin", 0x000001, 0x10000, CRC(8ab626dd) SHA1(e45561f77fc279b71dc1dd2e15a0870cb5c1cd89) )
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


ROM_START( maddog2 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_01_v.2.04.bin", 0x000000, 0x20000, CRC(0e1227f4) SHA1(bfd9081bb7d2bcbb77357839f292ce6136e9b228) )
	ROM_LOAD16_BYTE( "md2_02_v.2.04.bin", 0x000001, 0x20000, CRC(361bd99c) SHA1(5de6ef38e334e19f509227de7880306ac984ec23) )
ROM_END

ROM_START( maddog22 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_01.bin", 0x000000, 0x20000, CRC(4092227f) SHA1(6e5393aa5e64b59887260f483c50960084de7bd1) )
	ROM_LOAD16_BYTE( "md2_02.bin", 0x000001, 0x20000, CRC(addffa51) SHA1(665e9d93ddfa6b2ea5d006b41bf7eac3294244cc) )
ROM_END

ROM_START( maddog21 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_1.0_1.bin", 0x000000, 0x20000, CRC(97272a1d) SHA1(109014647c491f019ffb21091c7d0b89e1755b75) )
	ROM_LOAD16_BYTE( "md2_1.0_2.bin", 0x000001, 0x20000, CRC(0ce8db97) SHA1(dd4c09db59bb8c6caba935b1b28babe28ba8516b) )
ROM_END


ROM_START( spacepir )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sp_02.dat", 0x000000, 0x20000, CRC(10d162a2) SHA1(26833d5be1057be8639c00a7be18be33404ea751) )
	ROM_LOAD16_BYTE( "sp_01.dat", 0x000001, 0x20000, CRC(c0975188) SHA1(fd7643dc972e7861249ab7e76199975984888ae4) )
ROM_END


ROM_START( gallgall )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gg_1.dat", 0x000000, 0x20000, CRC(3793b211) SHA1(dccb1d9c5e2d6a4d249426ae6348e9fc9b72e665)  )
	ROM_LOAD16_BYTE( "gg_2.dat", 0x000001, 0x20000,  CRC(855c9d82) SHA1(96711aaa02f309cacd3e8d8efbe95cfc811aba96) )
ROM_END


ROM_START( crimepat )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp02.dat", 0x000000, 0x20000, CRC(a39a8b50) SHA1(55ca317ef13c3a42f12d68c480e6cc2d4459f6a4) )
	ROM_LOAD16_BYTE( "cp01.dat", 0x000001, 0x20000, CRC(e41fd2e8) SHA1(1cd9875fb4133ba4e3616271975dc736b343f156) )
ROM_END


ROM_START( crimep2 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp2_1.3_1.bin", 0x000000, 0x20000, CRC(e653395d) SHA1(8f6c86d98a52b7d85ae285fd841167cd07979318) )
	ROM_LOAD16_BYTE( "cp2_1.3_2.bin", 0x000001, 0x20000, CRC(dbdaa79a) SHA1(998044909d5c93e3bd1baafefab818fdb7b3f55e) )
ROM_END

ROM_START( crimep211 )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp2_1.dat", 0x000000, 0x20000, CRC(47879042) SHA1(8bb6c541e4e8e4508da8d4b93600176a2e7a1f41) )
	ROM_LOAD16_BYTE( "cp2_2.dat", 0x000001, 0x20000, CRC(f4e5251e) SHA1(e0c91343a98193d487c40e7a85f542b2a7a88f03) )
ROM_END


ROM_START( lastbh )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "bounty_01.bin", 0x000000, 0x20000, CRC(977566b2) SHA1(937e079e992ecb5930b17c1024c326e10962642b) )
	ROM_LOAD16_BYTE( "bounty_02.bin", 0x000001, 0x20000, CRC(2727ef1d) SHA1(f53421390b65c21a7666ff9d0f53ebf2a463d836) )
ROM_END


ROM_START( fastdraw )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fast_01.bin", 0x000000, 0x20000, CRC(4c4eb71e) SHA1(3bd487c546b6c80770a5fc880dcb10395ca431a2) )
	ROM_LOAD16_BYTE( "fast_02.bin", 0x000001, 0x20000, CRC(0d76a2da) SHA1(d396371ae1b9b0b6e6bc6f1f85c4b97bfc5dc34d) )
ROM_END


ROM_START( aplatoon )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "platoonv4u1.bin", 0x000000, 0x20000, CRC(8b33263e) SHA1(a1df38236321af90b522e2a783984fdf02e4c597) )
	ROM_LOAD16_BYTE( "platoonv4u2.bin", 0x000001, 0x20000, CRC(09a133cf) SHA1(9b3ff63035be8576c88fb284a25c2da5db0d5160) )
ROM_END


ROM_START( zortonbr )
	ALG_BIOS

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "zb_u2.bin", 0x000000, 0x10000, CRC(938b25cb) SHA1(d0114bbc588dcfce6a469013d0e35afb93e38af5) )
	ROM_LOAD16_BYTE( "zb_u3.bin", 0x000001, 0x10000, CRC(f59cfc4a) SHA1(9fadf7f1e23d6b4e828bf2b3de919d087c690a3f) )
ROM_END



/*************************************
 *
 *  Generic driver init
 *
 *************************************/

static void alg_init(running_machine &machine)
{
	alg_state *state = machine.driver_data<alg_state>();
	static const amiga_machine_interface alg_intf =
	{
		ANGUS_CHIP_RAM_MASK,
		NULL, NULL, alg_potgo_w,
		serial_w,

		vsync_callback,
		NULL,
		NULL,
		0
	};
	amiga_machine_config(machine, &alg_intf);

	/* set up memory */
	state->membank("bank1")->configure_entry(0, state->m_chip_ram);
	state->membank("bank1")->configure_entry(1, machine.region("user1")->base());
}



/*************************************
 *
 *  Per-game decryption
 *
 *************************************/

static DRIVER_INIT( palr1 )
{
	UINT32 length = machine.region("user2")->bytes();
	UINT8 *rom = machine.region("user2")->base();
	UINT8 *original = auto_alloc_array(machine, UINT8, length);
	UINT32 srcaddr;

	memcpy(original, rom, length);
	for (srcaddr = 0; srcaddr < length; srcaddr++)
	{
		UINT32 dstaddr = srcaddr;
		if (srcaddr & 0x2000) dstaddr ^= 0x1000;
		if (srcaddr & 0x8000) dstaddr ^= 0x4000;
		rom[dstaddr] = original[srcaddr];
	}
	auto_free(machine, original);

	alg_init(machine);
}

static DRIVER_INIT( palr3 )
{
	UINT32 length = machine.region("user2")->bytes();
	UINT8 *rom = machine.region("user2")->base();
	UINT8 *original = auto_alloc_array(machine, UINT8, length);
	UINT32 srcaddr;

	memcpy(original, rom, length);
	for (srcaddr = 0; srcaddr < length; srcaddr++)
	{
		UINT32 dstaddr = srcaddr;
		if (srcaddr & 0x2000) dstaddr ^= 0x1000;
		rom[dstaddr] = original[srcaddr];
	}
	auto_free(machine, original);

	alg_init(machine);
}

static DRIVER_INIT( palr6 )
{
	UINT32 length = machine.region("user2")->bytes();
	UINT8 *rom = machine.region("user2")->base();
	UINT8 *original = auto_alloc_array(machine, UINT8, length);
	UINT32 srcaddr;

	memcpy(original, rom, length);
	for (srcaddr = 0; srcaddr < length; srcaddr++)
	{
		UINT32 dstaddr = srcaddr;
		if (~srcaddr & 0x2000) dstaddr ^= 0x1000;
		if ( srcaddr & 0x8000) dstaddr ^= 0x4000;
		dstaddr ^= 0x20000;
		rom[dstaddr] = original[srcaddr];
	}
	auto_free(machine, original);

	alg_init(machine);
}

static DRIVER_INIT( aplatoon )
{
	/* NOT DONE TODO FIGURE OUT THE RIGHT ORDER!!!! */
	UINT8 *rom = machine.region("user2")->base();
	UINT8 *decrypted = auto_alloc_array(machine, UINT8, 0x40000);
	int i;

	static const int shuffle[] =
	{
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
		32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
	};

	for (i = 0; i < 64; i++)
		memcpy(decrypted + i * 0x1000, rom + shuffle[i] * 0x1000, 0x1000);
	memcpy(rom, decrypted, 0x40000);
	logerror("decrypt done\n ");
	alg_init(machine);
}

static DRIVER_INIT( none )
{
	alg_init(machine);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* BIOS */
GAME( 199?, alg_bios, 0, alg_r1,   alg,    none,     ROT0,  "American Laser Games", "American Laser Games BIOS", GAME_IS_BIOS_ROOT )

/* Rev. A board */
/* PAL R1 */
GAME( 1990, maddoga,  maddog, alg_r1,   alg,    palr1,    ROT0,  "American Laser Games", "Mad Dog McCree v1C board rev.A", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )

/* PAL R3 */
GAME( 1991, wsjr,     alg_bios, alg_r1,   alg,    palr3,    ROT0,  "American Laser Games", "Who Shot Johnny Rock? v1.6", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1991, wsjr15,   wsjr, alg_r1,   alg,    palr3,    ROT0,  "American Laser Games", "Who Shot Johnny Rock? v1.5", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )

/* Rev. B board */
/* PAL R6 */
GAME( 1990, maddog,   alg_bios, alg_r2,   alg_2p, palr6,    ROT0,  "American Laser Games", "Mad Dog McCree v2.03 board rev.B", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
  /* works ok but uses right player (2) controls only for trigger and holster */
GAME( 1992, maddog2,  alg_bios, alg_r2,   alg_2p, palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold v2.04", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1992, maddog22, alg_bios, alg_r2,   alg_2p, palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold v2.02", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1992, maddog21, maddog2,  alg_r2,   alg_2p, palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold v1.0", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
  /* works ok but uses right player (2) controls only for trigger and holster */
GAME( 1992, spacepir, alg_bios, alg_r2,   alg_2p, palr6,    ROT0,  "American Laser Games", "Space Pirates v2.2", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1992, gallgall, alg_bios, alg_r2,   alg_2p, palr6,    ROT0,  "American Laser Games", "Gallagher's Gallery v2.2", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
  /* all good, but no holster */
GAME( 1993, crimepat, alg_bios, alg_r2,   alg_2p, palr6,    ROT0,  "American Laser Games", "Crime Patrol v1.4", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1993, crimep2,  alg_bios, alg_r2,   alg_2p, palr6,    ROT0,  "American Laser Games", "Crime Patrol 2: Drug Wars v1.3", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1993, crimep211,crimep2,  alg_r2,   alg_2p, palr6,    ROT0,  "American Laser Games", "Crime Patrol 2: Drug Wars v1.1", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1994, lastbh,   alg_bios, alg_r2,   alg_2p, palr6,    ROT0,  "American Laser Games", "The Last Bounty Hunter v0.06", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1995, fastdraw, alg_bios, alg_r2,   alg_2p, palr6,    ROT90, "American Laser Games", "Fast Draw Showdown v1.3", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
  /* works ok but uses right player (2) controls only for trigger and holster */

/* NOVA games on ALG hardware with own address scramble */
GAME( 199?, aplatoon, alg_bios, alg_r2,   alg,    aplatoon, ROT0,  "Nova?", "Platoon V.?.? US", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )

/* Web Picmatic games PAL tv standard, own rom board */
GAME( 1993, zortonbr, alg_bios, picmatic, alg,    none,     ROT0,  "Web Picmatic", "Zorton Brothers (Los Justicieros)", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
