// license:BSD-3-Clause
// copyright-holders:Nathan Woods,R. Belmont
/***************************************************************************

    drivers/apple2gs.c
    Apple IIgs
    Driver by Nathan Woods and R. Belmont

    TODO:
    - Fix spurious interrupt problem (caused by ADB HLE; switching to emulation of the M50740 + GLU should solve this)
    - Fix 5.25" disks
    - Optimize video code
    - More RAM configurations

    NOTES:

    Video timing and the h/vcount registers:
                                  VCounts
    HCounts go like this:                     0xfa (start of frame, still in vblank)
    0 0x40 0x41 0x58 (first visible pixel)        0x7f
                 ____________________________________     0x100 (first visible scan line)
                |                                    |
                |                                    |
                |                                    |
                |                                    |
                |                                    |
    HBL region  |                                    |
                |                                    |
                |                                    |
                |                                    |
                |                                    |
                |                                    |   0x1c0 (first line of Vblank, c019 and heartbeat trigger here, only true VBL if in A2 classic modes)
                |                                    |
                 ____________________________________    0x1c8 (actual start of vblank in IIgs modes)

                                 0x1ff (end of frame, in vblank)

    There are 64 HCounts total, and 704 pixels total, so HCounts do not map to the pixel clock.
    VCounts do map directly to scanlines however, and count 262 of them.

=================================================================

***************************************************************************/


#include "emu.h"
#include "cpu/g65816/g65816.h"
#include "includes/apple2.h"
#include "imagedev/flopdrv.h"
#include "formats/ap2_dsk.h"
#include "formats/ap_dsk35.h"
#include "includes/apple2gs.h"
#include "machine/sonydriv.h"
#include "machine/appldriv.h"
#include "sound/es5503.h"
#include "machine/applefdc.h"
#include "machine/8530scc.h"
#include "sound/speaker.h"
#include "machine/ram.h"

#include "bus/a2bus/a2bus.h"
#include "bus/a2bus/a2lang.h"
#include "bus/a2bus/a2diskii.h"
#include "bus/a2bus/a2mockingboard.h"
#include "bus/a2bus/a2cffa.h"
#include "bus/a2bus/a2memexp.h"
#include "bus/a2bus/a2scsi.h"
#include "bus/a2bus/a2thunderclock.h"
#include "bus/a2bus/a2ssc.h"
#include "bus/a2bus/a2softcard.h"
#include "bus/a2bus/a2sam.h"
#include "bus/a2bus/a2alfam2.h"
#include "bus/a2bus/a2echoii.h"
#include "bus/a2bus/a2arcadebd.h"
#include "bus/a2bus/a2midi.h"
#include "bus/a2bus/a2vulcan.h"
#include "bus/a2bus/a2zipdrive.h"
//#include "bus/a2bus/a2udrive.h"
#include "bus/a2bus/a2hsscsi.h"

#include "softlist.h"

static const gfx_layout apple2gs_text_layout =
{
		14,8,       /* 14*8 characters */
	512,        /* 256 characters */
	1,          /* 1 bits per pixel */
	{ 0 },      /* no bitplanes; 1 bit per pixel */
	{ 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1 },   /* x offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8         /* every char takes 8 bytes */
};

static const gfx_layout apple2gs_dbltext_layout =
{
	7,8,        /* 7*8 characters */
	512,        /* 256 characters */
	1,          /* 1 bits per pixel */
	{ 0 },      /* no bitplanes; 1 bit per pixel */
	{ 7, 6, 5, 4, 3, 2, 1 },    /* x offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8         /* every char takes 8 bytes */
};

static GFXDECODE_START( apple2gs )
	GFXDECODE_ENTRY( "gfx1", 0x0000, apple2gs_text_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, apple2gs_dbltext_layout, 0, 2 )
GFXDECODE_END

static const unsigned char apple2gs_palette[] =
{
	0x0, 0x0, 0x0,  /* Black         $0              $0000 */
	0xD, 0x0, 0x3,  /* Deep Red      $1              $0D03 */
	0x0, 0x0, 0x9,  /* Dark Blue     $2              $0009 */
	0xD, 0x2, 0xD,  /* Purple        $3              $0D2D */
	0x0, 0x7, 0x2,  /* Dark Green    $4              $0072 */
	0x5, 0x5, 0x5,  /* Dark Gray     $5              $0555 */
	0x2, 0x2, 0xF,  /* Medium Blue   $6              $022F */
	0x6, 0xA, 0xF,  /* Light Blue    $7              $06AF */
	0x8, 0x5, 0x0,  /* Brown         $8              $0850 */
	0xF, 0x6, 0x0,  /* Orange        $9              $0F60 */
	0xA, 0xA, 0xA,  /* Light Gray    $A              $0AAA */
	0xF, 0x9, 0x8,  /* Pink          $B              $0F98 */
	0x1, 0xD, 0x0,  /* Light Green   $C              $01D0 */
	0xF, 0xF, 0x0,  /* Yellow        $D              $0FF0 */
	0x4, 0xF, 0x9,  /* Aquamarine    $E              $04F9 */
	0xF, 0xF, 0xF   /* White         $F              $0FFF */
};

static INPUT_PORTS_START( apple2gs )
	PORT_INCLUDE( apple2ep )

	PORT_START("adb_mouse_x")
	PORT_BIT( 0x7f, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 1")

	PORT_START("adb_mouse_y")
	PORT_BIT( 0x7f, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 0")

INPUT_PORTS_END



/* Initialize the palette */
PALETTE_INIT_MEMBER(apple2gs_state,apple2gs)
{
	int i;

//  PALETTE_INIT_NAME(apple2)(palette);

	for (i = 0; i < 16; i++)
	{
		palette.set_pen_color(i,
			apple2gs_palette[(3*i)]*17,
			apple2gs_palette[(3*i)+1]*17,
			apple2gs_palette[(3*i)+2]*17);

		m_a2_palette[i] = rgb_t(apple2gs_palette[(3*i)]*17, apple2gs_palette[(3*i)+1]*17, apple2gs_palette[(3*i)+2]*17);
	}
}

READ8_MEMBER(apple2gs_state::apple2gs_adc_read)
{
	return 0x80;
}

static const floppy_interface apple2gs_floppy35_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple35_iigs),
	"floppy_3_5"
};

static const floppy_interface apple2gs_floppy525_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple2),
	"floppy_5_25"
};


static ADDRESS_MAP_START( apple2gs_map, AS_PROGRAM, 8, apple2gs_state )
	/* nothing in the address map - everything is added dynamically */
ADDRESS_MAP_END

// ADB microcontroller emulation
//
// Huge thanks to Neil Parker's writeup on the ADB microcontroller!
// http://www.llx.com/~nparker/a2/adb.html

#if RUN_ADB_MICRO
READ8_MEMBER(apple2gs_state::adbmicro_p0_in)
{
	return m_glu_bus;
}

READ8_MEMBER(apple2gs_state::adbmicro_p1_in)
{
	if (m_is_rom3)
	{
		return 0x06;    // indicate ROM 3
	}

	return 0xff;
}

READ8_MEMBER(apple2gs_state::adbmicro_p2_in)
{
	UINT8 rv = 0;

	rv |= 0x40;     // no reset
	rv |= (m_adb_line) ? 0x00 : 0x80;

	return rv;
}

READ8_MEMBER(apple2gs_state::adbmicro_p3_in)
{
	return 0x7;     // don't press IIE capslock/ctrl/shift
}

WRITE8_MEMBER(apple2gs_state::adbmicro_p0_out)
{
	m_glu_bus = data;
}

WRITE8_MEMBER(apple2gs_state::adbmicro_p1_out)
{
}

WRITE8_MEMBER(apple2gs_state::adbmicro_p2_out)
{
	if (!(data & 0x10))
	{
			if (m_adbmicro->are_port_bits_output(0, 0xff))
			{
					keyglu_mcu_write(data & 7, m_glu_bus);
			}
			else    // read GLU
			{
					m_glu_bus = keyglu_mcu_read(data & 7);
			}
	}
}

WRITE8_MEMBER(apple2gs_state::adbmicro_p3_out)
{
	if (((data & 0x08) == 0x08) != m_adb_line)
	{
		m_adb_dtime = (int)(machine().time().as_ticks(XTAL_3_579545MHz*2) - m_last_adb_time);
//      printf("ADB change to %d (dtime %d)\n", (data>>3) & 1, m_adb_dtime);
		m_last_adb_time = machine().time().as_ticks(XTAL_3_579545MHz*2);
		m_adb_line = (data & 0x8) ? true : false;
	}
}
#endif

WRITE_LINE_MEMBER(apple2gs_state::a2bus_irq_w)
{
	if (state)
	{
		apple2gs_add_irq(IRQ_SLOT);
	}
	else
	{
		apple2gs_remove_irq(IRQ_SLOT);
	}
}

WRITE_LINE_MEMBER(apple2gs_state::a2bus_nmi_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

WRITE_LINE_MEMBER(apple2gs_state::a2bus_inh_w)
{
	m_inh_slot = -1;
	apple2_update_memory();
}

static SLOT_INTERFACE_START(apple2_cards)
	SLOT_INTERFACE("diskii", A2BUS_DISKII)  /* Disk II Controller Card */
	SLOT_INTERFACE("mockingboard", A2BUS_MOCKINGBOARD)  /* Sweet Micro Systems Mockingboard */
	SLOT_INTERFACE("phasor", A2BUS_PHASOR)  /* Applied Engineering Phasor */
	SLOT_INTERFACE("cffa2", A2BUS_CFFA2)  /* CFFA2000 Compact Flash for Apple II (www.dreher.net), 65C02/65816 firmware */
	SLOT_INTERFACE("cffa202", A2BUS_CFFA2_6502)  /* CFFA2000 Compact Flash for Apple II (www.dreher.net), 6502 firmware */
	SLOT_INTERFACE("memexp", A2BUS_MEMEXP)  /* Apple II Memory Expansion Card */
	SLOT_INTERFACE("ramfactor", A2BUS_RAMFACTOR)    /* Applied Engineering RamFactor */
	SLOT_INTERFACE("thclock", A2BUS_THUNDERCLOCK)    /* ThunderWare ThunderClock Plus */
	SLOT_INTERFACE("ssc", A2BUS_SSC)    /* Apple Super Serial Card */
	SLOT_INTERFACE("sam", A2BUS_SAM)    /* SAM Software Automated Mouth (8-bit DAC + speaker) */
	SLOT_INTERFACE("alfam2", A2BUS_ALFAM2)    /* ALF Apple Music II */
	SLOT_INTERFACE("echoii", A2BUS_ECHOII)    /* Street Electronics Echo II */
	SLOT_INTERFACE("arcbd", A2BUS_ARCADEBOARD)    /* Third Millenium Engineering Arcade Board */
	SLOT_INTERFACE("midi", A2BUS_MIDI)  /* Generic 6840+6850 MIDI board */
	SLOT_INTERFACE("vulcan", A2BUS_VULCAN)  /* AE Vulcan IDE card */
	SLOT_INTERFACE("vulcangold", A2BUS_VULCANGOLD)  /* AE Vulcan Gold IDE card */
	SLOT_INTERFACE("zipdrive", A2BUS_ZIPDRIVE)  /* ZIP Technologies IDE card */
	SLOT_INTERFACE("echoiiplus", A2BUS_ECHOPLUS)    /* Street Electronics Echo Plus (Echo II + Mockingboard clone) */
//  SLOT_INTERFACE("mdturbo", A2BUS_UDRIVE_TURBO)  /* ///SHH Systeme MicroDrive Turbo IDE card */

//    SLOT_INTERFACE("softcard", A2BUS_SOFTCARD)  /* Microsoft SoftCard */  // appears not to be IIgs compatible?
	SLOT_INTERFACE("scsi", A2BUS_SCSI)  /* Apple II SCSI Card */
	SLOT_INTERFACE("hsscsi", A2BUS_HSSCSI)  /* Apple II High-Speed SCSI Card */
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( apple2gs, apple2gs_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", G65816, APPLE2GS_14M/5)
	MCFG_CPU_PROGRAM_MAP(apple2gs_map)
	#if RUN_ADB_MICRO
	MCFG_CPU_ADD(ADBMICRO_TAG, M50741, XTAL_3_579545MHz)
	MCFG_M5074X_PORT0_READ_CALLBACK(READ8(apple2gs_state, adbmicro_p0_in))
	MCFG_M5074X_PORT0_WRITE_CALLBACK(WRITE8(apple2gs_state, adbmicro_p0_out))
	MCFG_M5074X_PORT1_READ_CALLBACK(READ8(apple2gs_state, adbmicro_p1_in))
	MCFG_M5074X_PORT1_WRITE_CALLBACK(WRITE8(apple2gs_state, adbmicro_p1_out))
	MCFG_M5074X_PORT2_READ_CALLBACK(READ8(apple2gs_state, adbmicro_p2_in))
	MCFG_M5074X_PORT2_WRITE_CALLBACK(WRITE8(apple2gs_state, adbmicro_p2_out))
	MCFG_M5074X_PORT3_READ_CALLBACK(READ8(apple2gs_state, adbmicro_p3_in))
	MCFG_M5074X_PORT3_WRITE_CALLBACK(WRITE8(apple2gs_state, adbmicro_p3_out))
	#endif
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", apple2gs_state, apple2_interrupt, "screen", 0, 1)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(704, 262)  // 640+32+32 for the borders
	MCFG_SCREEN_VISIBLE_AREA(0,703,0,230)
	MCFG_SCREEN_UPDATE_DRIVER(apple2gs_state, screen_update_apple2gs)

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(apple2gs_state, apple2gs)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", apple2gs )

	MCFG_MACHINE_START_OVERRIDE(apple2gs_state, apple2gs )
	MCFG_MACHINE_RESET_OVERRIDE(apple2gs_state, apple2gs )

	MCFG_VIDEO_START_OVERRIDE(apple2gs_state, apple2gs )

	/* keyboard controller */
	MCFG_DEVICE_ADD("ay3600", AY3600, 0)
	MCFG_AY3600_MATRIX_X0(IOPORT("X0"))
	MCFG_AY3600_MATRIX_X1(IOPORT("X1"))
	MCFG_AY3600_MATRIX_X2(IOPORT("X2"))
	MCFG_AY3600_MATRIX_X3(IOPORT("X3"))
	MCFG_AY3600_MATRIX_X4(IOPORT("X4"))
	MCFG_AY3600_MATRIX_X5(IOPORT("X5"))
	MCFG_AY3600_MATRIX_X6(IOPORT("X6"))
	MCFG_AY3600_MATRIX_X7(IOPORT("X7"))
	MCFG_AY3600_MATRIX_X8(IOPORT("X8"))
	MCFG_AY3600_SHIFT_CB(READLINE(apple2_state, ay3600_shift_r))
	MCFG_AY3600_CONTROL_CB(READLINE(apple2_state, ay3600_control_r))
	MCFG_AY3600_DATA_READY_CB(WRITELINE(apple2_state, ay3600_iie_data_ready_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("a2speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_ES5503_ADD("es5503", APPLE2GS_7M)
	MCFG_ES5503_OUTPUT_CHANNELS(2)
	MCFG_ES5503_IRQ_FUNC(WRITELINE(apple2gs_state, apple2gs_doc_irq))
	MCFG_ES5503_ADC_FUNC(READ8(apple2gs_state, apple2gs_adc_read))

	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* slot devices */
	MCFG_DEVICE_ADD("a2bus", A2BUS, 0)
	MCFG_A2BUS_CPU("maincpu")
	MCFG_A2BUS_OUT_IRQ_CB(WRITELINE(apple2gs_state, a2bus_irq_w))
	MCFG_A2BUS_OUT_NMI_CB(WRITELINE(apple2gs_state, a2bus_nmi_w))
	MCFG_A2BUS_OUT_INH_CB(WRITELINE(apple2gs_state, a2bus_inh_w))
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl0", A2BUS_LANG, NULL)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl1", apple2_cards, nullptr)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl2", apple2_cards, nullptr)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl3", apple2_cards, nullptr)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl4", apple2_cards, nullptr)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl5", apple2_cards, nullptr)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl6", apple2_cards, nullptr)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl7", apple2_cards, nullptr)

	MCFG_IWM_ADD("fdc", apple2_fdc_interface)

	/* SCC */
	MCFG_DEVICE_ADD("scc", SCC8530, APPLE2GS_14M/2)

	MCFG_LEGACY_FLOPPY_APPLE_2_DRIVES_ADD(apple2gs_floppy525_floppy_interface,15,16)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADDITIONAL_ADD(apple2gs_floppy35_floppy_interface)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")      // 1M on board + 1M in the expansion slot was common for ROM 03
	MCFG_RAM_EXTRA_OPTIONS("1M,3M,4M,5M,6M,7M,8M")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	MCFG_SOFTWARE_LIST_ADD("flop35_list","apple2gs")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("flop525_list", "apple2")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2gsr1, apple2gs )
	MCFG_MACHINE_START_OVERRIDE(apple2gs_state, apple2gsr1 )

	#if RUN_ADB_MICRO
	MCFG_CPU_REPLACE(ADBMICRO_TAG, M50740, XTAL_3_579545MHz)
	MCFG_M5074X_PORT0_READ_CALLBACK(READ8(apple2gs_state, adbmicro_p0_in))
	MCFG_M5074X_PORT0_WRITE_CALLBACK(WRITE8(apple2gs_state, adbmicro_p0_out))
	MCFG_M5074X_PORT1_READ_CALLBACK(READ8(apple2gs_state, adbmicro_p1_in))
	MCFG_M5074X_PORT1_WRITE_CALLBACK(WRITE8(apple2gs_state, adbmicro_p1_out))
	MCFG_M5074X_PORT2_READ_CALLBACK(READ8(apple2gs_state, adbmicro_p2_in))
	MCFG_M5074X_PORT2_WRITE_CALLBACK(WRITE8(apple2gs_state, adbmicro_p2_out))
	MCFG_M5074X_PORT3_READ_CALLBACK(READ8(apple2gs_state, adbmicro_p3_in))
	MCFG_M5074X_PORT3_WRITE_CALLBACK(WRITE8(apple2gs_state, adbmicro_p3_out))
	#endif

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1280K")  // 256K on board + 1M in the expansion slot was common for ROM 01
	MCFG_RAM_EXTRA_OPTIONS("256K,512K,768K,1M,2M,3M,4M,5M,6M,7M,8M")
	MCFG_RAM_DEFAULT_VALUE(0x00)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(apple2gs)
	// M50740/50741 ADB MCU inside the IIgs system unit
	ROM_REGION(0x1000,M5074X_INTERNAL_ROM(ADBMICRO_TAG),0)
	ROM_LOAD( "341s0632-2.bin", 0x000000, 0x001000, CRC(e1c11fb0) SHA1(141d18c36a617ab9dce668445440d34354be0672) )

	// i8048 microcontroller inside the IIgs ADB Standard Keyboard
	ROM_REGION(0x400, "kmcu", 0)
	// from early-production ROM 00 Woz Limited Edition IIgs.  keyboard "Part Number 658-4081  825-1301-A"
	// ROM is marked "NEC Japan  8626XD 341-0232A  543" so 26th week of 1986
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	// from later non-Woz ROM 01.  keyboard "Model A9M0330"
	// ROM is marked "NEC Japan 8806HD  8048HC610  341-0124-A  (c) APPLE 87" so 6th week of 1988
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89)) /* need label/part number */

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("341-0737", 0x0000, 0x20000, CRC(8d410067) SHA1(c0f4704233ead14cb8e1e8a68fbd7063c56afd27)) /* Needs verification; 341-0737: IIgs ROM03 FC-FD */
	ROM_LOAD("341-0748", 0x20000, 0x20000, CRC(d4c50550) SHA1(2784cdd7ac7094b3e494409db3e72b4e6d2d9e81)) /* Needs verification; 341-0748: IIgs ROM03 FE-FF */

	ROM_REGION(0x20000, "es5503", ROMREGION_ERASE00)

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr3p)
	ROM_REGION(0x1000,M5074X_INTERNAL_ROM(ADBMICRO_TAG),0)
	ROM_LOAD( "341s0632-2.bin", 0x000000, 0x001000, CRC(e1c11fb0) SHA1(141d18c36a617ab9dce668445440d34354be0672) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89)) /* need label/part number */

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("341-0728", 0x0000, 0x20000, NO_DUMP) /* 341-0728: IIgs ROM03 prototype FC-FD */
	ROM_LOAD("341-0729", 0x20000, 0x20000, NO_DUMP) /* 341-0729: IIgs ROM03 prototype FE-FF */

	ROM_REGION(0x20000, "es5503", ROMREGION_ERASE00)

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr3lp)
	ROM_REGION(0x1000,M5074X_INTERNAL_ROM(ADBMICRO_TAG),0)
	ROM_LOAD( "341s0632-2.bin", 0x000000, 0x001000, CRC(e1c11fb0) SHA1(141d18c36a617ab9dce668445440d34354be0672) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89)) /* need label/part number */

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("341-0737", 0x0000, 0x20000, CRC(8d410067) SHA1(c0f4704233ead14cb8e1e8a68fbd7063c56afd27)) /* 341-0737: IIgs ROM03 FC-FD */
	ROM_LOAD("341-0749", 0x20000, 0x20000, NO_DUMP) /* 341-0749: unknown ?post? ROM03 IIgs prototype? FE-FF */

	ROM_REGION(0x20000, "es5503", ROMREGION_ERASE00)

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr1)
	ROM_REGION(0xc00,M5074X_INTERNAL_ROM(ADBMICRO_TAG),0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89)) /* need label/part number */

	ROM_REGION(0x20000,"maincpu",0)
	ROM_LOAD("342-0077-b", 0x0000, 0x20000, CRC(42f124b0) SHA1(e4fc7560b69d062cb2da5b1ffbe11cd1ca03cc37)) /* 342-0077-B: IIgs ROM01 */

	ROM_REGION(0x20000, "es5503", ROMREGION_ERASE00)

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr0)
	ROM_REGION(0xc00,M5074X_INTERNAL_ROM(ADBMICRO_TAG),0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x20000,"maincpu",0)
	/* Should these roms really be split like this? according to the unofficial apple rom list, IIgs ROM00 was on one rom labeled 342-0077-A */
	ROM_LOAD("rom0a.bin", 0x0000,  0x8000, CRC(9cc78238) SHA1(0ea82e10720a01b68722ab7d9f66efec672a44d3))
	ROM_LOAD("rom0b.bin", 0x8000,  0x8000, CRC(8baf2a79) SHA1(91beeb11827932fe10475252d8036a63a2edbb1c))
	ROM_LOAD("rom0c.bin", 0x10000, 0x8000, CRC(94c32caa) SHA1(4806d50d676b06f5213b181693fc1585956b98bb))
	ROM_LOAD("rom0d.bin", 0x18000, 0x8000, CRC(200a15b8) SHA1(0c2890bb169ead63369738bbd5f33b869f24c42a))

	ROM_REGION(0x20000, "es5503", ROMREGION_ERASE00)

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr0p)  // 6/19/1986 Cortland prototype
	ROM_REGION(0xc00,M5074X_INTERNAL_ROM(ADBMICRO_TAG),0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x20000,"maincpu",0)
	ROM_LOAD( "rombf.bin",    0x000000, 0x020000, CRC(ab04fedf) SHA1(977589a17553956d583a21020080a39dd396df5c) )

	ROM_REGION(0x20000, "es5503", ROMREGION_ERASE00)

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr0p2)  // 3/10/1986 Cortland prototype, boots as "Apple //'ing - Alpha 2.0"
	ROM_REGION(0xc00,M5074X_INTERNAL_ROM(ADBMICRO_TAG),0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x20000,"maincpu",0)
	ROM_LOAD( "apple iigs alpha rom 2.0 19860310.bin", 0x000000, 0x020000, CRC(a47d275f) SHA1(c5836adcfc8be69c7351b84afa94c814e8d92b81) )

	ROM_REGION(0x20000, "es5503", ROMREGION_ERASE00)

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT       INIT      COMPANY            FULLNAME */
COMP( 1989, apple2gs, 0,        apple2, apple2gs,   apple2gs, driver_device, 0, "Apple Computer", "Apple IIgs (ROM03)", MACHINE_SUPPORTS_SAVE )
COMP( 198?, apple2gsr3p, apple2gs, 0,   apple2gs,   apple2gs, driver_device, 0, "Apple Computer", "Apple IIgs (ROM03 prototype)", MACHINE_NOT_WORKING )
COMP( 1989, apple2gsr3lp, apple2gs, 0,  apple2gs,   apple2gs, driver_device, 0, "Apple Computer", "Apple IIgs (ROM03 late prototype?)", MACHINE_NOT_WORKING )
COMP( 1987, apple2gsr1, apple2gs, 0,    apple2gsr1, apple2gs, driver_device, 0, "Apple Computer", "Apple IIgs (ROM01)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0, apple2gs, 0,    apple2gsr1, apple2gs, driver_device, 0, "Apple Computer", "Apple IIgs (ROM00)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0p,apple2gs, 0,    apple2gsr1, apple2gs, driver_device, 0, "Apple Computer", "Apple IIgs (ROM00 prototype 6/19/1986)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0p2,apple2gs,0,    apple2gsr1, apple2gs, driver_device, 0, "Apple Computer", "Apple IIgs (ROM00 prototype 3/10/1986)", MACHINE_SUPPORTS_SAVE )
