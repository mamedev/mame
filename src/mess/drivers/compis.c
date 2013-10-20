/******************************************************************************

    drivers/compis.c
    machine driver

    Per Ola Ingvarsson
    Tomas Karlsson

    Hardware:
        - Intel 80186 CPU 8MHz, integrated DMA(8237?), PIC(8259?), PIT(8253?)
                - Intel 80130 OSP Operating system processor (PIC 8259, PIT 8254)
        - Intel 8274 MPSC Multi-protocol serial communications controller (NEC 7201)
        - Intel 8255 PPI Programmable peripheral interface
        - Intel 8253 PIT Programmable interval timer
        - Intel 8251 USART Universal synchronous asynchronous receiver transmitter
        - National 58174 Real-time clock (compatible with 58274)
    Peripheral:
        - Intel 82720 GDC Graphic display processor (NEC uPD 7220)
        - Intel 8272 FDC Floppy disk controller (Intel iSBX-218A)
        - Western Digital WD1002-05 Winchester controller

    Memory map:

    00000-3FFFF RAM LMCS (Low Memory Chip Select)
    40000-4FFFF RAM MMCS 0 (Midrange Memory Chip Select)
    50000-5FFFF RAM MMCS 1 (Midrange Memory Chip Select)
    60000-6FFFF RAM MMCS 2 (Midrange Memory Chip Select)
    70000-7FFFF RAM MMCS 3 (Midrange Memory Chip Select)
    80000-EFFFF NOP
    F0000-FFFFF ROM UMCS (Upper Memory Chip Select)

18/08/2011 -[Robbbert]
- Modernised
- Removed F4 display, as the gfx is in different places per bios.
- Changed to monochrome, it usually had a greenscreen monitor, although some
  were amber.
- Still doesn't work.
- Added a nasty hack to get a display on compis2 (wait 20 seconds)


 ******************************************************************************/

#include "includes/compis.h"
#include "formats/mfi_dsk.h"

UINT32 compis_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(RGB_BLACK, cliprect);

	m_crtc->screen_update(screen, bitmap, cliprect);

	return 0;
}

static UPD7220_DISPLAY_PIXELS( hgdc_display_pixels )
{
	compis_state *state = device->machine().driver_data<compis_state>();
	UINT8 i,gfx = state->m_video_ram[address];

	for(i=0; i<8; i++)
		bitmap.pix32(y, x + i) = RGB_MONOCHROME_GREEN_HIGHLIGHT[BIT((gfx >> i), 0)];
}

static UPD7220_INTERFACE( hgdc_intf )
{
	hgdc_display_pixels,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/* TODO: why it writes to ROM region? */
WRITE8_MEMBER( compis_state::vram_w )
{
	m_video_ram[offset+0x20000] = data;
}

static ADDRESS_MAP_START( compis_mem , AS_PROGRAM, 16, compis_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM
	AM_RANGE(0x40000, 0x4ffff) AM_RAM
	AM_RANGE(0x50000, 0x5ffff) AM_RAM
	AM_RANGE(0x60000, 0x6ffff) AM_RAM
	AM_RANGE(0x70000, 0x7ffff) AM_RAM
	AM_RANGE(0xe8000, 0xeffff) AM_ROM AM_REGION("maincpu",0)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("maincpu",0) AM_WRITE8(vram_w, 0xffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( compis_io, AS_IO, 16, compis_state )
	AM_RANGE(0x0000, 0x0007) /* PCS0 */ AM_MIRROR(0x78) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xff00)
	AM_RANGE(0x0080, 0x0087) /* PCS1 */ AM_MIRROR(0x78) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0x00ff)
	AM_RANGE(0x0100, 0x011f) /* PCS2 */ AM_MIRROR(0x60) AM_DEVREADWRITE8("mm58274c", mm58274c_device, read, write, 0x00ff)
  //AM_RANGE(0x0180, 0x0181) /* PCS3 */ AM_MIRROR(0x7e)
  //AM_RANGE(0x0200, 0x0201) /* PCS4 */ AM_MIRROR(0x7e)
	AM_RANGE(0x0280, 0x0283) /* PCS5 */ AM_MIRROR(0x70) AM_DEVREADWRITE8("pic8259_master", pic8259_device, read, write, 0x00ff) /* 80150/80130 */
	AM_RANGE(0x0288, 0x028f) /* PCS5 */ AM_MIRROR(0x70) AM_DEVREADWRITE8("pit8254", pit8254_device, read, write, 0x00ff) /* 80150/80130 */
    AM_RANGE(0x0300, 0x0301) /* PCS6:0 */ AM_MIRROR(0xe) AM_WRITE8(tape_mon_w, 0x00ff)
  //AM_RANGE(0x0300, 0x0301) /* PCS6:1 0xff00 */ AM_MIRROR(0xe) // DMA-ACK graphics
  //AM_RANGE(0x0310, 0x0311) /* PCS6:2 0x00ff */ AM_MIRROR(0xe) // 8274 INTERRUPT ACKNOWLEDGE
	AM_RANGE(0x0310, 0x0311) /* PCS6:3 */ AM_MIRROR(0xc) AM_DEVREADWRITE8("uart", i8251_device, data_r, data_w, 0xff00)
	AM_RANGE(0x0312, 0x0313) /* PCS6:3 */ AM_MIRROR(0xc) AM_DEVREADWRITE8("uart", i8251_device, status_r, control_w, 0xff00)
  //AM_RANGE(0x0320, 0x0321) /* PCS6:4 0x00ff */ AM_MIRROR(0xe) // 8274
  //AM_RANGE(0x0320, 0x0321) /* PCS6:5 0xff00 */ AM_MIRROR(0xe) // DMA-TERMINATE J8 (iSBX0)
	AM_RANGE(0x0330, 0x0333) /* PCS6:6 */ AM_DEVREADWRITE8("upd7220", upd7220_device, read, write, 0x00ff)
  //AM_RANGE(0x0330, 0x0331) /* PCS6:7 0xff00 */ AM_MIRROR(0xe) // DMA-TERMINATE J9 (iSBX1)
	AM_RANGE(0x0340, 0x0343) /* PCS6:8 */ AM_DEVICE8("i8272a", i8272a_device, map, 0x00ff) // 8272 CS0 (8/16-bit) J8 (iSBX0)
	AM_RANGE(0x034e, 0x034f) /* PCS6:8 */ AM_READWRITE8(fdc_mon_r, fdc_mon_w, 0x00ff) // 8272 CS0 (8/16-bit) J8 (iSBX0)
  //AM_RANGE(0x0340, 0x0341) /* PCS6:9 0xff00 */ AM_MIRROR(0xe) // CS1 (16-bit) J8 (iSBX0)
  //AM_RANGE(0x0350, 0x0351) /* PCS6:10 0x00ff */ AM_MIRROR(0xe) // CS1 (8-bit) J8 (iSBX0)
	AM_RANGE(0x0350, 0x0351) /* PCS6:11 */ AM_MIRROR(0xe) AM_DEVREADWRITE8("i8272a", i8272a_device, mdma_r, mdma_w, 0xff00) // DMA-ACK J8 (iSBX0)
  //AM_RANGE(0x0360, 0x0361) /* PCS6:13 0x00ff */ AM_MIRROR(0xe) // CS0 (8/16-bit) J9 (iSBX1)
  //AM_RANGE(0x0360, 0x0361) /* PCS6:13 0xff00 */ AM_MIRROR(0xe) // CS1 (16-bit) J9 (iSBX1)
  //AM_RANGE(0x0370, 0x0371) /* PCS6:14 0x00ff */ AM_MIRROR(0xe) // CS1 (8-bit) J9 (iSBX1)
  //AM_RANGE(0x0370, 0x0371) /* PCS6:15 0xff00 */ AM_MIRROR(0xe) // DMA-ACK J9 (iSBX1)
ADDRESS_MAP_END

static INPUT_PORTS_START (compis)
	PORT_START("DSW0")
	PORT_DIPNAME( 0x18, 0x00, "S8 Test mode")
	PORT_DIPSETTING( 0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x08, "Remote" )
	PORT_DIPSETTING( 0x10, "Stand alone" )
	PORT_DIPSETTING( 0x18, "Reserved" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "iSBX-218A DMA")
	PORT_DIPSETTING( 0x01, "Enabled" )
	PORT_DIPSETTING( 0x00, "Disabled" )
INPUT_PORTS_END


static const mm58274c_interface compis_mm58274c_interface =
{
	0,  /*  mode 24*/
	1   /*  first day of week */
};


//-------------------------------------------------
//  cassette_interface compis_cassette_interface
//-------------------------------------------------

static const cassette_interface compis_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};

const floppy_format_type compis_floppy_formats[] = {
	FLOPPY_D88_FORMAT,
	FLOPPY_DFI_FORMAT,
	FLOPPY_IMD_FORMAT,
	FLOPPY_IPF_FORMAT,
	FLOPPY_MFI_FORMAT,
	FLOPPY_MFM_FORMAT,
	FLOPPY_TD0_FORMAT,
	FLOPPY_CPIS_FORMAT,
	NULL
};
static SLOT_INTERFACE_START( compis_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

static ADDRESS_MAP_START( upd7220_map, AS_0, 8, compis_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3ffff)
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END

static MACHINE_CONFIG_START( compis, compis_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80186, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(compis_mem)
	MCFG_CPU_IO_MAP(compis_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", compis_state,  compis_vblank_int)
	MCFG_80186_IRQ_SLAVE_ACK(DEVREAD8(DEVICE_SELF, compis_state, compis_irq_callback))
	MCFG_80186_TMROUT0_HANDLER(DEVWRITELINE(DEVICE_SELF, compis_state, tmr0_w))

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("upd7220", upd7220_device, screen_update)

	/* Devices */
	MCFG_PIT8253_ADD( "pit8253", compis_pit8253_config )
	MCFG_PIT8254_ADD( "pit8254", compis_pit8254_config )
	MCFG_PIC8259_ADD( "pic8259_master", DEVWRITELINE("maincpu", i80186_cpu_device, int0_w), VCC, NULL )
	MCFG_I8255_ADD( "ppi8255", compis_ppi_interface )
	MCFG_UPD7220_ADD("upd7220", XTAL_4_433619MHz/2, hgdc_intf, upd7220_map) //unknown clock
	MCFG_CENTRONICS_PRINTER_ADD("centronics", standard_centronics)
	MCFG_I8251_ADD("uart", compis_usart_interface)
	MCFG_MM58274C_ADD("mm58274c", compis_mm58274c_interface)
	MCFG_I8272A_ADD("i8272a", true)
	MCFG_FLOPPY_DRIVE_ADD("i8272a:0", compis_floppies, "525qd", compis_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("i8272a:1", compis_floppies, "525qd", compis_floppy_formats)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, compis_cassette_interface)
	MCFG_COMPIS_KEYBOARD_ADD(NULL)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_list", "compis")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( compis2, compis_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80186, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(compis_mem)
	MCFG_CPU_IO_MAP(compis_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", compis_state,  compis_vblank_int)
	MCFG_80186_IRQ_SLAVE_ACK(DEVREAD8(DEVICE_SELF, compis_state, compis_irq_callback))
	MCFG_80186_TMROUT0_HANDLER(DEVWRITELINE(DEVICE_SELF, compis_state, tmr0_w))

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_UPDATE_DRIVER(compis_state, screen_update)

	/* Devices */
	MCFG_PIT8253_ADD( "pit8253", compis_pit8253_config )
	MCFG_PIT8254_ADD( "pit8254", compis_pit8254_config )
	MCFG_PIC8259_ADD( "pic8259_master", DEVWRITELINE("maincpu", i80186_cpu_device, int0_w), VCC, NULL )
	MCFG_I8255_ADD( "ppi8255", compis_ppi_interface )
	MCFG_UPD7220_ADD("upd7220", XTAL_4_433619MHz/2, hgdc_intf, upd7220_map) //unknown clock
	MCFG_CENTRONICS_PRINTER_ADD("centronics", standard_centronics)
	MCFG_I8251_ADD("uart", compis_usart_interface)
	MCFG_MM58274C_ADD("mm58274c", compis_mm58274c_interface)
	MCFG_I8272A_ADD("i8272a", true)
	MCFG_FLOPPY_DRIVE_ADD("i8272a:0", compis_floppies, "525qd", compis_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("i8272a:1", compis_floppies, "525qd", compis_floppy_formats)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, compis_cassette_interface)
	MCFG_COMPIS_KEYBOARD_ADD(NULL)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_list", "compis")
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( compis )
	ROM_REGION16_LE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa883003.u40", 0x0000, 0x4000, CRC(195ef6bf) SHA1(eaf8ae897e1a4b62d3038ff23777ce8741b766ef) )
	ROM_LOAD16_BYTE( "sa883003.u36", 0x0001, 0x4000, CRC(7c918f56) SHA1(8ba33d206351c52f44f1aa76cc4d7f292dcef761) )
	ROM_LOAD16_BYTE( "sa883003.u39", 0x8000, 0x4000, CRC(3cca66db) SHA1(cac36c9caa2f5bb42d7a6d5b84f419318628935f) )
	ROM_LOAD16_BYTE( "sa883003.u35", 0x8001, 0x4000, CRC(43c38e76) SHA1(f32e43604107def2c2259898926d090f2ed62104) )
ROM_END

ROM_START( compis2 )
	ROM_REGION16_LE( 0x10000, "maincpu", 0 )
	ROM_DEFAULT_BIOS( "v303" )

	ROM_SYSTEM_BIOS( 0, "v302", "Compis II v3.02 (1986-09-09)" )
	ROMX_LOAD( "comp302.u39", 0x0000, 0x8000, CRC(16a7651e) SHA1(4cbd4ba6c6c915c04dfc913ec49f87c1dd7344e3), ROM_BIOS(1) | ROM_SKIP(1) )
	ROMX_LOAD( "comp302.u35", 0x0001, 0x8000, CRC(ae546bef) SHA1(572e45030de552bb1949a7facbc885b8bf033fc6), ROM_BIOS(1) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 1, "v303", "Compis II v3.03 (1987-03-09)" )
	ROMX_LOAD( "rysa094.u39", 0x0000, 0x8000, CRC(e7302bff) SHA1(44ea20ef4008849af036c1a945bc4f27431048fb), ROM_BIOS(2) | ROM_SKIP(1) )
	ROMX_LOAD( "rysa094.u35", 0x0001, 0x8000, CRC(b0694026) SHA1(eb6b2e3cb0f42fd5ffdf44f70e652ecb9714ce30), ROM_BIOS(2) | ROM_SKIP(1) )
ROM_END

/*   YEAR   NAME        PARENT  COMPAT MACHINE  INPUT   INIT     COMPANY     FULLNAME */
COMP(1985,  compis,     0,      0,     compis,  compis, driver_device, 0, "Telenova", "Compis" , GAME_NOT_WORKING )
COMP(1986,  compis2,    compis, 0,     compis2, compis, driver_device, 0, "Telenova", "Compis II" , GAME_NOT_WORKING )
