/******************************************************************************


    Philips CD-I-based games
    ------------------------

    Preliminary MAME driver by Harmony
    Help provided by CD-i Fan


*******************************************************************************

STATUS:

Quizard does not work for unknown reasons.

TODO:

- Proper handling of the 68070's internal devices (UART,DMA,Timers etc.)

- Full emulation of the CDIC, SLAVE and/or MCD212 customs

*******************************************************************************/

#define CLOCK_A XTAL_30MHz
#define CLOCK_B XTAL_19_6608MHz

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/cdi.h"
#include "machine/timekpr.h"
#include "sound/cdda.h"
#include "cdrom.h"
#include "cdi.lh"


#if ENABLE_VERBOSE_LOG
INLINE void verboselog(running_machine *machine, int n_level, const char *s_fmt, ...)
{
    if( VERBOSE_LEVEL >= n_level )
    {
        va_list v;
        char buf[ 32768 ];
        va_start( v, s_fmt );
        vsprintf( buf, s_fmt, v );
        va_end( v );
        logerror( "%08x: %s", cpu_get_pc(machine->device("maincpu")), buf );
    }
}
#else
#define verboselog(x,y,z,...)
#endif

/*************************
*      Memory maps       *
*************************/

static ADDRESS_MAP_START( cdimono1_mem, ADDRESS_SPACE_PROGRAM, 16 )
    AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_BASE_MEMBER(cdi_state,planea)
    AM_RANGE(0x00200000, 0x0027ffff) AM_RAM AM_BASE_MEMBER(cdi_state,planeb)
#if ENABLE_UART_PRINTING
    AM_RANGE(0x00301400, 0x00301403) AM_READ(uart_loopback_enable)
#endif
	AM_RANGE(0x00300000, 0x00303bff) AM_DEVREADWRITE("cdic", cdic_ram_r, cdic_ram_w)
    //AM_RANGE(0x00300000, 0x00303bff) AM_RAM AM_BASE_MEMBER(cdi_state,cdic_regs.ram)
	AM_RANGE(0x00303c00, 0x00303fff) AM_DEVREADWRITE("cdic", cdic_r, cdic_w)
	AM_RANGE(0x00310000, 0x00317fff) AM_DEVREADWRITE("slave", slave_r, slave_w)
    //AM_RANGE(0x00318000, 0x0031ffff) AM_NOP
    AM_RANGE(0x00320000, 0x00323fff) AM_DEVREADWRITE8("mk48t08", timekeeper_r, timekeeper_w, 0xff00)    /* nvram (only low bytes used) */
    AM_RANGE(0x00400000, 0x0047ffff) AM_ROM AM_REGION("maincpu", 0)
    AM_RANGE(0x004fffe0, 0x004fffff) AM_READWRITE(mcd212_r, mcd212_w)
    //AM_RANGE(0x00500000, 0x0057ffff) AM_RAM
    AM_RANGE(0x00500000, 0x00ffffff) AM_NOP
    //AM_RANGE(0x00e00000, 0x00efffff) AM_RAM // DVC
    AM_RANGE(0x80000000, 0x8000807f) AM_READWRITE(scc68070_periphs_r, scc68070_periphs_w)
ADDRESS_MAP_END

/*************************
*      Input ports       *
*************************/

static INPUT_CHANGED( mcu_input )
{
    cdi_state *state = field->port->machine->driver_data<cdi_state>();
    scc68070_regs_t *scc68070 = &state->scc68070_regs;
	bool send = false;

	switch((FPTR)param)
	{
		case 0x39:
			if(input_port_read(field->port->machine, "INPUT1") & 0x01) send = true;
			break;
		case 0x37:
			if(input_port_read(field->port->machine, "INPUT1") & 0x02) send = true;
			break;
		case 0x31:
			if(input_port_read(field->port->machine, "INPUT1") & 0x04) send = true;
			break;
		case 0x32:
			if(input_port_read(field->port->machine, "INPUT1") & 0x08) send = true;
			break;
		case 0x33:
			if(input_port_read(field->port->machine, "INPUT1") & 0x10) send = true;
			break;

		case 0x30:
			if(input_port_read(field->port->machine, "INPUT2") & 0x01) send = true;
			break;
		case 0x38:
			if(input_port_read(field->port->machine, "INPUT2") & 0x02) send = true;
			break;
		case 0x34:
			if(input_port_read(field->port->machine, "INPUT2") & 0x04) send = true;
			break;
		case 0x35:
			if(input_port_read(field->port->machine, "INPUT2") & 0x08) send = true;
			break;
		case 0x36:
			if(input_port_read(field->port->machine, "INPUT2") & 0x10) send = true;
			break;
	}

	if(send)
	{
		UINT8 data = (UINT8)((FPTR)param & 0x000000ff);
		scc68070_quizard_rx(field->port->machine, scc68070, data);
	}
}

static INPUT_PORTS_START( cdi )
    PORT_START("MOUSEX")
    PORT_BIT(0x3ff, 0x000, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(0) PORT_CHANGED(cdislave_device::mouse_update, 0)

    PORT_START("MOUSEY")
    PORT_BIT(0x3ff, 0x000, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(0) PORT_CHANGED(cdislave_device::mouse_update, 0)

    PORT_START("MOUSEBTN")
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 1") PORT_CHANGED(cdislave_device::mouse_update, 0)
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 2") PORT_CHANGED(cdislave_device::mouse_update, 0)
    PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

    PORT_START("INPUT1")
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Coin 1") PORT_CHANGED(mcu_input, (void*)0x39)
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start 1") PORT_CHANGED(mcu_input, (void*)0x37)
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Player 1 A") PORT_CHANGED(mcu_input, (void*)0x31)
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Player 1 B") PORT_CHANGED(mcu_input, (void*)0x32)
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Player 1 C") PORT_CHANGED(mcu_input, (void*)0x33)
    PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

    PORT_START("INPUT2")
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service") PORT_CHANGED(mcu_input, (void*)0x30)
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Start 2") PORT_CHANGED(mcu_input, (void*)0x38)
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Player 2 A") PORT_CHANGED(mcu_input, (void*)0x34)
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Player 2 B") PORT_CHANGED(mcu_input, (void*)0x35)
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Player 2 C") PORT_CHANGED(mcu_input, (void*)0x36)
    PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

    PORT_START("DEBUG")
    PORT_CONFNAME( 0x01, 0x00, "Plane A Disable")
    PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
    PORT_CONFSETTING(    0x01, DEF_STR( On ) )
    PORT_CONFNAME( 0x02, 0x00, "Plane B Disable")
    PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
    PORT_CONFSETTING(    0x02, DEF_STR( On ) )
    PORT_CONFNAME( 0x04, 0x00, "Force Backdrop Color")
    PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
    PORT_CONFSETTING(    0x04, DEF_STR( On ) )
    PORT_CONFNAME( 0xf0, 0x00, "Backdrop Color")
    PORT_CONFSETTING(    0x00, "Black" )
    PORT_CONFSETTING(    0x10, "Half-Bright Blue" )
    PORT_CONFSETTING(    0x20, "Half-Bright Green" )
    PORT_CONFSETTING(    0x30, "Half-Bright Cyan" )
    PORT_CONFSETTING(    0x40, "Half-Bright Red" )
    PORT_CONFSETTING(    0x50, "Half-Bright Magenta" )
    PORT_CONFSETTING(    0x60, "Half-Bright Yellow" )
    PORT_CONFSETTING(    0x70, "Half-Bright White" )
    PORT_CONFSETTING(    0x80, "Black (Alternate)" )
    PORT_CONFSETTING(    0x90, "Blue" )
    PORT_CONFSETTING(    0xa0, "Green" )
    PORT_CONFSETTING(    0xb0, "Cyan" )
    PORT_CONFSETTING(    0xc0, "Red" )
    PORT_CONFSETTING(    0xd0, "Magenta" )
    PORT_CONFSETTING(    0xe0, "Yellow" )
    PORT_CONFSETTING(    0xf0, "White" )
INPUT_PORTS_END

static MACHINE_START( cdi )
{
    cdi_state *state = machine->driver_data<cdi_state>();

    scc68070_register_globals(machine, &state->scc68070_regs);
}

static MACHINE_RESET( cdi )
{
    cdi_state *state = machine->driver_data<cdi_state>();
    UINT16 *src   = (UINT16*)machine->region("maincpu")->base();
    UINT16 *dst   = state->planea;
    //device_t *cdrom_dev = machine->device("cdrom");
    memcpy(dst, src, 0x8);

    scc68070_init(machine, &state->scc68070_regs);

    machine->device("maincpu")->reset();

    state->dmadac[0] = machine->device<dmadac_sound_device>("dac1");
    state->dmadac[1] = machine->device<dmadac_sound_device>("dac2");
}

static MACHINE_RESET( quizrd12 )
{
	MACHINE_RESET_CALL( cdi );

    scc68070_set_quizard_mcu_value(machine, 0x021f);
    scc68070_set_quizard_mcu_ack(machine, 0x5a);
}

static MACHINE_RESET( quizrd17 )
{
	MACHINE_RESET_CALL( cdi );

    scc68070_set_quizard_mcu_value(machine, 0x021f);
    scc68070_set_quizard_mcu_ack(machine, 0x5a);
}

static MACHINE_RESET( quizrd22 )
{
	MACHINE_RESET_CALL( cdi );

	// 0x2b1: Italian
	// 0x001: French
	// 0x188: German

	scc68070_set_quizard_mcu_value(machine, 0x188);
    scc68070_set_quizard_mcu_ack(machine, 0x59);
}

static MACHINE_RESET( quizrd32 )
{
	MACHINE_RESET_CALL( cdi );

	scc68070_set_quizard_mcu_value(machine, 0x00ae);
    scc68070_set_quizard_mcu_ack(machine, 0x58);
}

static MACHINE_RESET( quizrr41 )
{
	MACHINE_RESET_CALL( cdi );

	//scc68070_set_quizard_mcu_value(machine, 0x0139);
	scc68070_set_quizard_mcu_value(machine, 0x011f);
	scc68070_set_quizard_mcu_ack(machine, 0x57);
}

static MACHINE_RESET( quizrr42 )
{
	MACHINE_RESET_CALL( cdi );

	scc68070_set_quizard_mcu_value(machine, 0x011f);
	scc68070_set_quizard_mcu_ack(machine, 0x57);
}

/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( cdi, cdi_state )

    MCFG_CPU_ADD("maincpu", SCC68070, CLOCK_A/2)
    MCFG_CPU_PROGRAM_MAP(cdimono1_mem)
    MCFG_CPU_VBLANK_INT("screen", scc68070_mcu_frame)

    MCFG_SCREEN_ADD("screen", RASTER)
    MCFG_SCREEN_REFRESH_RATE(60)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
    MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
    MCFG_SCREEN_SIZE(384, 302)
    MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 22, 302-1) //dynamic resolution,TODO

    MCFG_SCREEN_ADD("lcd", RASTER)
    MCFG_SCREEN_REFRESH_RATE(60)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
    MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
    MCFG_SCREEN_SIZE(192, 22)
    MCFG_SCREEN_VISIBLE_AREA(0, 192-1, 0, 22-1)

    MCFG_PALETTE_LENGTH(0x100)

    MCFG_DEFAULT_LAYOUT(layout_cdi)

    MCFG_VIDEO_START(cdimono1)
    MCFG_VIDEO_UPDATE(cdimono1)

    MCFG_MACHINE_START(cdi)

    MCFG_CDICDIC_ADD( "cdic" )
    MCFG_CDISLAVE_ADD( "slave" )

    /* sound hardware */
    MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

    MCFG_SOUND_ADD( "dac1", DMADAC, 0 )
    MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 1.0 )

    MCFG_SOUND_ADD( "dac2", DMADAC, 0 )
    MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 1.0 )

    MCFG_SOUND_ADD( "cdda", CDDA, 0 )
    MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 1.0 )
    MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 1.0 )

    MCFG_MK48T08_ADD( "mk48t08" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizrd12, cdi )
	MCFG_MACHINE_RESET( quizrd12 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizrd17, cdi )
	MCFG_MACHINE_RESET( quizrd17 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizrd22, cdi )
	MCFG_MACHINE_RESET( quizrd22 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizrd32, cdi )
	MCFG_MACHINE_RESET( quizrd32 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizrr41, cdi )
	MCFG_MACHINE_RESET( quizrr41 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizrr42, cdi )
	MCFG_MACHINE_RESET( quizrr42 )
MACHINE_CONFIG_END

/*************************
*        Rom Load        *
*************************/

ROM_START( cdi )
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping
ROM_END

ROM_START( quizard )
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrd32", 0, SHA1(31e9fa2169aa44d799c37170b238134ab738e1a1) )
ROM_END

ROM_START( quizrd22 )
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrd22", 0, SHA1(03c8fdcf27ead6e221691111e8c679b551099543) )
ROM_END

ROM_START( quizrd17 )
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrd17", 0, SHA1(4bd698f076505b4e17be978481bce027eb47123b) )
ROM_END

ROM_START( quizrd12 ) /* CD-ROM printed 01/95 */
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrd12", 0, SHA1(6e41683b96b74e903040842aeb18437ad7813c82) )
ROM_END

ROM_START( quizrr42 ) /* CD-ROM printed 09/98 */
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrr42", 0, SHA1(a5d5c8950b4650b8753f9119dc7f1ccaa2aa5442) )
ROM_END

ROM_START( quizrr41 )
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrr41", 0, SHA1(2c0484c6545aac8e00b318328c6edce6f5dde43d) )
ROM_END

/*************************
*      Game driver(s)    *
*************************/

// BIOS
GAME( 1991, cdi,      0,        cdi,           cdi,      0, ROT0,     "Philips", "CD-i (Mono-I) BIOS", GAME_IS_BIOS_ROOT )

// Working
GAME( 1995, quizrd12, cdi,      quizrd12,      cdi,      0, ROT0,     "TAB Austria",  "Quizard 1.2", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION )
GAME( 1995, quizrd17, cdi,      quizrd17,      cdi,      0, ROT0,     "TAB Austria",  "Quizard 1.7", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION )
GAME( 1995, quizrd22, cdi,      quizrd22,      cdi,      0, ROT0,     "TAB Austria",  "Quizard 2.2", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION )

// Partially working
GAME( 1996, quizard,  cdi,      quizrd32,      cdi,      0, ROT0,     "TAB Austria",  "Quizard 3.2", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION )
GAME( 1998, quizrr41, cdi,      quizrr41,      cdi,      0, ROT0,     "TAB Austria",  "Quizard Rainbow 4.1", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION )
GAME( 1998, quizrr42, cdi,      quizrr42,      cdi,      0, ROT0,     "TAB Austria",  "Quizard Rainbow 4.2", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION )
