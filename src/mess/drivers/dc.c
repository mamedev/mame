/*

    dc.c - Sega Dreamcast driver
    by R. Belmont

    SH-4 @ 200 MHz
    ARM7DI @ 2.8223 MHz (no T or M extensions)
    PowerVR 3D video
    AICA audio
    GD-ROM drive (modified ATAPI interface)

            NTSC/N  NTSC/I   PAL/N   PAL/I   VGA
        (x/240) (x/480) (x/240)  (x/480) (640x480)
    VTOTAL   262     524      312     624    524
    HTOTAL   857     857      863     863    857

    PCLKs = 26917135 (NTSC 480 @ 59.94), 26944080 (VGA 480 @ 60.0), 13458568 (NTSC 240 @ 59.94),
            25925600 (PAL 480 @ 50.00), 13462800 (PAL 240 @ 50.00)

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/sh4/sh4.h"
#include "cpu/arm7/arm7core.h"
#include "sound/aica.h"
#include "includes/dc.h"
#include "imagedev/chd_cd.h"
#include "machine/maple-dc.h"
#include "machine/dc-ctrl.h"
#include "machine/gdrom.h"

#define CPU_CLOCK (200000000)

// things from mess/machine/dc.c
void dreamcast_atapi_init(running_machine &machine);
void dreamcast_atapi_reset(running_machine &machine);
extern DECLARE_READ64_HANDLER( dc_mess_gdrom_r );
extern DECLARE_WRITE64_HANDLER( dc_mess_gdrom_w );
extern DECLARE_READ64_HANDLER( dc_mess_g1_ctrl_r );
extern DECLARE_WRITE64_HANDLER( dc_mess_g1_ctrl_w );

static READ64_HANDLER( dcus_idle_skip_r )
{
	if (space.device().safe_pc()==0xc0ba52a)
		space.device().execute().spin_until_time(attotime::from_usec(2500));
	//  device_spinuntil_int(&space.device());

	return space.machine().driver_data<dc_state>()->dc_ram[0x2303b0/8];
}

static READ64_HANDLER( dcjp_idle_skip_r )
{
	if (space.device().safe_pc()==0xc0bac62)
		space.device().execute().spin_until_time(attotime::from_usec(2500));
	//  device_spinuntil_int(&space.device());

	return space.machine().driver_data<dc_state>()->dc_ram[0x2302f8/8];
}

DRIVER_INIT_MEMBER(dc_state,dc)
{
	dreamcast_atapi_init(machine());
}

DRIVER_INIT_MEMBER(dc_state,dcus)
{
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xc2303b0, 0xc2303b7, FUNC(dcus_idle_skip_r));

	DRIVER_INIT_CALL(dc);
}

DRIVER_INIT_MEMBER(dc_state,dcjp)
{
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xc2302f8, 0xc2302ff, FUNC(dcjp_idle_skip_r));

	DRIVER_INIT_CALL(dc);
}

static UINT64 PDTRA, PCTRA;
static READ64_HANDLER( dc_pdtra_r )
{
	UINT64 out = PCTRA<<32;

	out |= PDTRA & ~3;

	// if both bits are inputs
	if (!(PCTRA & 0x5))
	{
		out |= 3;
	}

	// one's input one's output, always pull up both bits
	if (((PCTRA & 5) == 1) || ((PCTRA & 5) == 4))
	{
		if (PDTRA & 3)
		{
			out |= 3;
		}
	}

	return out;
}

static WRITE64_HANDLER( dc_pdtra_w )
{
	PCTRA = (data>>16) & 0xffff;
	PDTRA = (data & 0xffff);
}

static READ64_HANDLER( dc_arm_r )
{
	dc_state *state = space.machine().driver_data<dc_state>();

	return *((UINT64 *)state->dc_sound_ram.target()+offset);
}

static WRITE64_HANDLER( dc_arm_w )
{
	dc_state *state = space.machine().driver_data<dc_state>();

	COMBINE_DATA((UINT64 *)state->dc_sound_ram.target() + offset);
}


 // SB_LMMODE0
 static WRITE64_HANDLER( ta_texture_directpath0_w )
 {
	dc_state *state = space.machine().driver_data<dc_state>();

	int mode = state->pvrctrl_regs[SB_LMMODE0]&1;
	if (mode&1)
	{
		printf("ta_texture_directpath0_w 32-bit access!\n");
		COMBINE_DATA(&state->dc_framebuffer_ram[offset]);
	}
	else
	{
		COMBINE_DATA(&state->dc_texture_ram[offset]);
	}
 }

 // SB_LMMODE1
 static WRITE64_HANDLER( ta_texture_directpath1_w )
 {
	dc_state *state = space.machine().driver_data<dc_state>();

	int mode = state->pvrctrl_regs[SB_LMMODE1]&1;
	if (mode&1)
	{
		printf("ta_texture_directpath1_w 32-bit access!\n");
		COMBINE_DATA(&state->dc_framebuffer_ram[offset]);
	}
	else
	{
		COMBINE_DATA(&state->dc_texture_ram[offset]);
	}
 }

static ADDRESS_MAP_START( dc_map, AS_PROGRAM, 64, dc_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM	AM_WRITENOP				// BIOS
	AM_RANGE(0x00200000, 0x0021ffff) AM_ROM AM_REGION("maincpu", 0x200000)	// flash
	AM_RANGE(0x005f6800, 0x005f69ff) AM_READWRITE_LEGACY(dc_sysctrl_r, dc_sysctrl_w )
	AM_RANGE(0x005f6c00, 0x005f6cff) AM_DEVICE32( "maple_dc", maple_dc_device, amap, U64(0xffffffffffffffff) )
	AM_RANGE(0x005f7000, 0x005f70ff) AM_READWRITE_LEGACY(dc_mess_gdrom_r, dc_mess_gdrom_w )
	AM_RANGE(0x005f7400, 0x005f74ff) AM_READWRITE_LEGACY(dc_mess_g1_ctrl_r, dc_mess_g1_ctrl_w )
	AM_RANGE(0x005f7800, 0x005f78ff) AM_READWRITE_LEGACY(dc_g2_ctrl_r, dc_g2_ctrl_w )
	AM_RANGE(0x005f7c00, 0x005f7cff) AM_READWRITE_LEGACY(pvr_ctrl_r, pvr_ctrl_w )
	AM_RANGE(0x005f8000, 0x005f9fff) AM_READWRITE_LEGACY(pvr_ta_r, pvr_ta_w )
	AM_RANGE(0x00600000, 0x006007ff) AM_READWRITE_LEGACY(dc_modem_r, dc_modem_w )
	AM_RANGE(0x00700000, 0x00707fff) AM_DEVREADWRITE_LEGACY("aica", dc_aica_reg_r, dc_aica_reg_w )
	AM_RANGE(0x00710000, 0x0071000f) AM_READWRITE_LEGACY(dc_rtc_r, dc_rtc_w )
	AM_RANGE(0x00800000, 0x009fffff) AM_READWRITE_LEGACY(dc_arm_r, dc_arm_w )

	/* Area 1 */
	AM_RANGE(0x04000000, 0x04ffffff) AM_RAM	AM_SHARE("dc_texture_ram")      // texture memory 64 bit access
	AM_RANGE(0x05000000, 0x05ffffff) AM_RAM AM_SHARE("frameram") // apparently this actually accesses the same memory as the 64-bit texture memory access, but in a different format, keep it apart for now

	/* Area 3 */
	AM_RANGE(0x0c000000, 0x0cffffff) AM_RAM AM_SHARE("dc_ram")
	AM_RANGE(0x0d000000, 0x0dffffff) AM_RAM AM_SHARE("dc_ram")// extra ram on Naomi (mirror on DC)
	AM_RANGE(0x0e000000, 0x0effffff) AM_RAM AM_SHARE("dc_ram")// mirror
	AM_RANGE(0x0f000000, 0x0fffffff) AM_RAM AM_SHARE("dc_ram")// mirror

	/* Area 4 */
	AM_RANGE(0x10000000, 0x107fffff) AM_WRITE_LEGACY(ta_fifo_poly_w )
	AM_RANGE(0x10800000, 0x10ffffff) AM_WRITE_LEGACY(ta_fifo_yuv_w )
	AM_RANGE(0x11000000, 0x117fffff) AM_WRITE_LEGACY(ta_texture_directpath0_w ) AM_MIRROR(0x00800000)  // access to texture / fraembfufer memory (either 32-bit or 64-bit area depending on SB_LMMODE0 register - cannot be written directly, only through dma / store queue

	AM_RANGE(0x12000000, 0x127fffff) AM_WRITE_LEGACY(ta_fifo_poly_w )
	AM_RANGE(0x12800000, 0x12ffffff) AM_WRITE_LEGACY(ta_fifo_yuv_w )
	AM_RANGE(0x13000000, 0x137fffff) AM_WRITE_LEGACY(ta_texture_directpath1_w ) AM_MIRROR(0x00800000) // access to texture / fraembfufer memory (either 32-bit or 64-bit area depending on SB_LMMODE1 register - cannot be written directly, only through dma / store queue

	AM_RANGE(0x8c000000, 0x8cffffff) AM_RAM AM_SHARE("dc_ram")	// another RAM mirror

	AM_RANGE(0xa0000000, 0xa01fffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dc_port, AS_IO, 64, dc_state )
	AM_RANGE(0x00000000, 0x00000007) AM_READWRITE_LEGACY(dc_pdtra_r, dc_pdtra_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( dc_audio_map, AS_PROGRAM, 32, dc_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("dc_sound_ram")		/* shared with SH-4 */
	AM_RANGE(0x00800000, 0x00807fff) AM_DEVREADWRITE_LEGACY("aica", dc_arm_aica_r, dc_arm_aica_w)
ADDRESS_MAP_END

static MACHINE_RESET( dc_console )
{
	dc_state *state = machine.driver_data<dc_state>();

	device_t *aica = machine.device("aica");
	state->machine_reset();
	aica_set_ram_base(aica, state->dc_sound_ram, 2*1024*1024);
	dreamcast_atapi_reset(machine);
}

static void aica_irq(device_t *device, int irq)
{
	device->machine().device("soundcpu")->execute().set_input_line(ARM7_FIRQ_LINE, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const aica_interface dc_aica_interface =
{
	0,
	0,
	aica_irq
};

static const struct sh4_config sh4cpu_config = {  1,  0,  1,  0,  0,  0,  1,  1,  0, CPU_CLOCK };

static MACHINE_CONFIG_START( dc, dc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH4LE, CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(dc_map)
	MCFG_CPU_IO_MAP(dc_port)

	MCFG_CPU_ADD("soundcpu", ARM7, ((XTAL_33_8688MHz*2)/3)/8)	// AICA bus clock is 2/3rds * 33.8688.  ARM7 gets 1 bus cycle out of each 8.
	MCFG_CPU_PROGRAM_MAP(dc_audio_map)

	MCFG_MACHINE_RESET( dc_console )

	MCFG_MAPLE_DC_ADD( "maple_dc", "maincpu", dc_maple_irq )
	MCFG_DC_CONTROLLER_ADD("dcctrl0", "maple_dc", 0, "P1:0", "P1:1", "P1:A0", "P1:A1", "P1:A2", "P1:A3", "P1:A4", "P1:A5")
	MCFG_DC_CONTROLLER_ADD("dcctrl1", "maple_dc", 1, "P2:0", "P2:1", "P2:A0", "P2:A1", "P2:A2", "P2:A3", "P2:A4", "P2:A5")
	MCFG_DC_CONTROLLER_ADD("dcctrl2", "maple_dc", 2, "P3:0", "P3:1", "P3:A0", "P3:A1", "P3:A2", "P3:A3", "P3:A4", "P3:A5")
	MCFG_DC_CONTROLLER_ADD("dcctrl3", "maple_dc", 3, "P4:0", "P4:1", "P4:A0", "P4:A1", "P4:A2", "P4:A3", "P4:A4", "P4:A5")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(dc_state, screen_update_dc)

	MCFG_PALETTE_LENGTH(0x1000)


	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("aica", AICA, 0)
	MCFG_SOUND_CONFIG(dc_aica_interface)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 2.0)

	MCFG_DEVICE_ADD("cdrom", GDROM, 0)
MACHINE_CONFIG_END

ROM_START(dc)
	ROM_REGION(0x220000, "maincpu", 0)
        ROM_LOAD( "dc101d_us.bin", 0x000000, 0x200000, CRC(89f2b1a1) SHA1(8951d1bb219ab2ff8583033d2119c899cc81f18c) )	// BIOS
        ROM_LOAD( "dcus_ntsc.bin", 0x200000, 0x020000, CRC(c611b498) SHA1(94d44d7f9529ec1642ba3771ed3c5f756d5bc872) )	// Flash
ROM_END

ROM_START( dceu )
	ROM_REGION(0x220000, "maincpu", 0)
        ROM_LOAD( "dc101d_eu.bin", 0x000000, 0x200000, CRC(a2564fad) SHA1(edc5d3d70a93c935703d26119b37731fd317d2bf) )	// BIOS
        ROM_LOAD( "dceu_pal.bin", 0x200000, 0x020000, CRC(b7e5aeeb) SHA1(11e02433e13b793ec7ffe0ae2356750bb8a575b4) )	// Flash
ROM_END

ROM_START( dcjp )
	ROM_REGION(0x220000, "maincpu", 0)
        ROM_LOAD( "dc1004jp.bin", 0x000000, 0x200000, CRC(5454841f) SHA1(1ea132c0fbbf07ef76789eadc07908045c089bd6) )	// BIOS
        /* ROM_LOAD( "dcjp_ntsc.bad", 0x200000, 0x020000, BAD_DUMP CRC(307a7035) SHA1(1411423a9d071340ea52c56e19c1aafc4e1309ee) )      // Hacked Flash */
        ROM_LOAD( "dcjp_ntsc.bin", 0x200000, 0x020000, CRC(5F92BF76) SHA1(BE78B834F512AB2CF3D67B96E377C9F3093FF82A) )  // Flash
ROM_END

ROM_START( dcdev )
	ROM_REGION(0x220000, "maincpu", 0)
        ROM_LOAD( "hkt-0120.bin", 0x000000, 0x200000, CRC(2186E0E5) SHA1(6BD18FB83F8FDB56F1941E079580E5DD672A6DAD) )		// BIOS
        ROM_LOAD( "hkt-0120-flash.bin", 0x200000, 0x020000, CRC(7784C304) SHA1(31EF57F550D8CD13E40263CBC657253089E53034) )	// Flash
ROM_END

ROM_START( dcprt )
    ROM_REGION(0x220000, "maincpu", 0)
    ROM_LOAD( "katana-set5-v0.41-98-08-27.bin", 0x000000, 0x200000, CRC(485877bd) SHA1(dc1af1f1248ffa87d57bc5ef2ea41aac95ecfc5e) ) // BIOS
    ROM_LOAD( "dcjp_ntsc.bin", 0x200000, 0x020000, CRC(5F92BF76) SHA1(BE78B834F512AB2CF3D67B96E377C9F3093FF82A) )  // Flash
ROM_END

static INPUT_PORTS_START( dc )
	PORT_START("P1:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_START("P1:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)

	PORT_START("P2:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_START("P2:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)

	PORT_START("P3:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_START("P3:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(3)

	PORT_START("P4:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_START("P4:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4)

	PORT_START("MAMEDEBUG") \
	PORT_DIPNAME( 0x01, 0x00, "Bilinear Filtering" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END


/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT      COMPANY FULLNAME */
CONS( 1999, dc,     dcjp,   0,      dc,     dc, dc_state,     dcus,   "Sega", "Dreamcast (USA, NTSC)", GAME_NOT_WORKING )
CONS( 1998, dcjp,   0,      0,      dc,     dc, dc_state,     dcjp,   "Sega", "Dreamcast (Japan, NTSC)", GAME_NOT_WORKING )
CONS( 1999, dceu,   dcjp,   0,      dc,     dc, dc_state,     dcus,   "Sega", "Dreamcast (Europe, PAL)", GAME_NOT_WORKING )
CONS( 1998, dcdev,  dcjp,   0,      dc,     dc, dc_state,     dc,     "Sega", "HKT-0120 Sega Dreamcast Development Box", GAME_NOT_WORKING )
CONS( 1998, dcprt,  dcjp,   0,      dc,     dc, dc_state,     dcjp,   "Sega", "Katana Set 5 Prototype", GAME_NOT_WORKING )

