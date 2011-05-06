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
#define CPU_CLOCK (200000000)

#ifdef MESS
UINT16 actel_id;
int jvsboard_type;
#else
extern UINT16 actel_id;
extern int jvsboard_type;
#endif

// things from mess/machine/dc.c
void dreamcast_atapi_init(running_machine &machine);
void dreamcast_atapi_reset(running_machine &machine);
extern READ64_HANDLER( dc_mess_gdrom_r );
extern WRITE64_HANDLER( dc_mess_gdrom_w );
extern READ64_HANDLER( dc_mess_g1_ctrl_r );
extern WRITE64_HANDLER( dc_mess_g1_ctrl_w );

static UINT32 *dc_sound_ram;
static UINT64 *dc_ram;

static READ64_HANDLER( dcus_idle_skip_r )
{
	if (cpu_get_pc(&space->device())==0xc0ba52a)
		device_spin_until_time(&space->device(), attotime::from_usec(2500));
	//  device_spinuntil_int(&space->device());

	return dc_ram[0x2303b0/8];
}

static READ64_HANDLER( dcjp_idle_skip_r )
{
	if (cpu_get_pc(&space->device())==0xc0bac62)
		device_spin_until_time(&space->device(), attotime::from_usec(2500));
	//  device_spinuntil_int(&space->device());

	return dc_ram[0x2302f8/8];
}

static DRIVER_INIT(dc)
{
	dreamcast_atapi_init(machine);
}

static DRIVER_INIT(dcus)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xc2303b0, 0xc2303b7, FUNC(dcus_idle_skip_r));

	DRIVER_INIT_CALL(dc);
}

static DRIVER_INIT(dcjp)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xc2302f8, 0xc2302ff, FUNC(dcjp_idle_skip_r));

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
	return *((UINT64 *)dc_sound_ram+offset);
}

static WRITE64_HANDLER( dc_arm_w )
{
	COMBINE_DATA((UINT64 *)dc_sound_ram + offset);
}

 // SB_LMMODE0
 static WRITE64_HANDLER( ta_texture_directpath0_w )
 {
	int mode = pvrctrl_regs[SB_LMMODE0]&1;
	if (mode&1)
	{
		printf("ta_texture_directpath0_w 32-bit access!\n");
		COMBINE_DATA(&dc_framebuffer_ram[offset]);
	}
	else
	{
		COMBINE_DATA(&dc_texture_ram[offset]);
	}
 }

 // SB_LMMODE1
 static WRITE64_HANDLER( ta_texture_directpath1_w )
 {
	int mode = pvrctrl_regs[SB_LMMODE1]&1;
	if (mode&1)
	{
		printf("ta_texture_directpath0_w 32-bit access!\n");
		COMBINE_DATA(&dc_framebuffer_ram[offset]);
	}
	else
	{
		COMBINE_DATA(&dc_texture_ram[offset]);
	}
 }

static ADDRESS_MAP_START( dc_map, AS_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM	AM_WRITENOP				// BIOS
	AM_RANGE(0x00200000, 0x0021ffff) AM_ROM AM_REGION("maincpu", 0x200000)	// flash
	AM_RANGE(0x005f6800, 0x005f69ff) AM_READWRITE( dc_sysctrl_r, dc_sysctrl_w )
	AM_RANGE(0x005f6c00, 0x005f6cff) AM_DEVREADWRITE32_MODERN( "maple_dc", maple_dc_device, sb_mdstar_r, sb_mdstar_w, U64(0xffffffff00000000) )
	AM_RANGE(0x005f7000, 0x005f70ff) AM_READWRITE( dc_mess_gdrom_r, dc_mess_gdrom_w )
	AM_RANGE(0x005f7400, 0x005f74ff) AM_READWRITE( dc_mess_g1_ctrl_r, dc_mess_g1_ctrl_w )
	AM_RANGE(0x005f7800, 0x005f78ff) AM_READWRITE( dc_g2_ctrl_r, dc_g2_ctrl_w )
	AM_RANGE(0x005f7c00, 0x005f7cff) AM_READWRITE( pvr_ctrl_r, pvr_ctrl_w )
	AM_RANGE(0x005f8000, 0x005f9fff) AM_READWRITE( pvr_ta_r, pvr_ta_w )
	AM_RANGE(0x00600000, 0x006007ff) AM_READWRITE( dc_modem_r, dc_modem_w )
	AM_RANGE(0x00700000, 0x00707fff) AM_DEVREADWRITE("aica", dc_aica_reg_r, dc_aica_reg_w )
	AM_RANGE(0x00710000, 0x0071000f) AM_READWRITE( dc_rtc_r, dc_rtc_w )
	AM_RANGE(0x00800000, 0x009fffff) AM_READWRITE( dc_arm_r, dc_arm_w )

	/* Area 1 */
	AM_RANGE(0x04000000, 0x04ffffff) AM_RAM	AM_BASE( &dc_texture_ram )      // texture memory 64 bit access
	AM_RANGE(0x05000000, 0x05ffffff) AM_RAM AM_BASE( &dc_framebuffer_ram ) // apparently this actually accesses the same memory as the 64-bit texture memory access, but in a different format, keep it apart for now

	/* Area 3 */
	AM_RANGE(0x0c000000, 0x0cffffff) AM_RAM AM_SHARE("share4") AM_BASE(&dc_ram)
	AM_RANGE(0x0d000000, 0x0dffffff) AM_RAM AM_SHARE("share4")// extra ram on Naomi (mirror on DC)
	AM_RANGE(0x0e000000, 0x0effffff) AM_RAM AM_SHARE("share4")// mirror
	AM_RANGE(0x0f000000, 0x0fffffff) AM_RAM AM_SHARE("share4")// mirror

	/* Area 4 */
	AM_RANGE(0x10000000, 0x107fffff) AM_WRITE( ta_fifo_poly_w )
	AM_RANGE(0x10800000, 0x10ffffff) AM_WRITE( ta_fifo_yuv_w )
	AM_RANGE(0x11000000, 0x117fffff) AM_WRITE( ta_texture_directpath0_w ) AM_MIRROR(0x00800000)  // access to texture / fraembfufer memory (either 32-bit or 64-bit area depending on SB_LMMODE0 register - cannot be written directly, only through dma / store queue

	AM_RANGE(0x12000000, 0x127fffff) AM_WRITE( ta_fifo_poly_w )
	AM_RANGE(0x12800000, 0x12ffffff) AM_WRITE( ta_fifo_yuv_w )
	AM_RANGE(0x13000000, 0x137fffff) AM_WRITE( ta_texture_directpath1_w ) AM_MIRROR(0x00800000) // access to texture / fraembfufer memory (either 32-bit or 64-bit area depending on SB_LMMODE1 register - cannot be written directly, only through dma / store queue

	AM_RANGE(0x8c000000, 0x8cffffff) AM_RAM AM_SHARE("share4")	// another RAM mirror

	AM_RANGE(0xa0000000, 0xa01fffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dc_port, AS_IO, 64 )
	AM_RANGE(0x00000000, 0x00000007) AM_READWRITE( dc_pdtra_r, dc_pdtra_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( dc_audio_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_BASE(&dc_sound_ram)		/* shared with SH-4 */
	AM_RANGE(0x00800000, 0x00807fff) AM_DEVREADWRITE("aica", dc_arm_aica_r, dc_arm_aica_w)
ADDRESS_MAP_END

static MACHINE_RESET( dc_console )
{
	device_t *aica = machine.device("aica");
	MACHINE_RESET_CALL(dc);
	aica_set_ram_base(aica, dc_sound_ram, 2*1024*1024);
	dreamcast_atapi_reset(machine);
}

static void aica_irq(device_t *device, int irq)
{
	cputag_set_input_line(device->machine(), "soundcpu", ARM7_FIRQ_LINE, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const aica_interface dc_aica_interface =
{
	0,
	0,
	aica_irq
};

static const struct sh4_config sh4cpu_config = {  1,  0,  1,  0,  0,  0,  1,  1,  0, CPU_CLOCK };

static MACHINE_CONFIG_START( dc, driver_device )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH4, CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(dc_map)
	MCFG_CPU_IO_MAP(dc_port)

	MCFG_CPU_ADD("soundcpu", ARM7, ((XTAL_33_8688MHz*2)/3)/8)	// AICA bus clock is 2/3rds * 33.8688.  ARM7 gets 1 bus cycle out of each 8.
	MCFG_CPU_PROGRAM_MAP(dc_audio_map)

	MCFG_MACHINE_START( dc )
	MCFG_MACHINE_RESET( dc_console )

	MCFG_MAPLE_DC_ADD( "maple_dc", "maincpu" )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE(dc)

	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_VIDEO_START(dc)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("aica", AICA, 0)
	MCFG_SOUND_CONFIG(dc_aica_interface)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 2.0)

	MCFG_CDROM_ADD( "cdrom" )
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

static INPUT_PORTS_START( dc )
	PORT_START("P1L")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_START("P1H")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)

	PORT_START("P2L")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_START("P2H")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)

	PORT_START("P3L")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_START("P3H")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(3)

	PORT_START("P4L")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_START("P4H")
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
CONS( 1999, dc,     dcjp,   0,      dc,     dc,     dcus,   "Sega", "Dreamcast (USA, NTSC)", GAME_NOT_WORKING )
CONS( 1998, dcjp,   0,      0,      dc,     dc,     dcjp,   "Sega", "Dreamcast (Japan, NTSC)", GAME_NOT_WORKING )
CONS( 1999, dceu,   dcjp,   0,      dc,     dc,     dcus,   "Sega", "Dreamcast (Europe, PAL)", GAME_NOT_WORKING )
CONS( 1998, dcdev,  dcjp,   0,      dc,     dc,     dc,     "Sega", "HKT-0120 Sega Dreamcast Development Box", GAME_NOT_WORKING )
