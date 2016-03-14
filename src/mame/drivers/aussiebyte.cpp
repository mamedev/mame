// license:BSD-3-Clause
// copyright-holders:Robbbert
/*************************************************************************************************

    The Aussie Byte II Single-Board Computer, created by SME Systems, Melbourne, Australia.
    Also known as the Knight 2000 Microcomputer.

    Status:
    Boots up from floppy.
    Output to serial terminal and to 6545 are working. Serial keyboard works.

    Developed in conjunction with members of the MSPP. Written in July, 2015.

    ToDo:
    - CRT8002 attributes controller
    - Graphics
    - Hard drive controllers and drives
    - Test Centronics printer
    - PIO connections
    - RTC not working

    Note of MAME restrictions:
    - Votrax doesn't sound anything like the real thing
    - WD1001/WD1002 device is not emulated
    - CRT8002 device is not emulated

**************************************************************************************************/

/***********************************************************

    Includes

************************************************************/
#include "includes/aussiebyte.h"


/***********************************************************

    Address Maps

************************************************************/

static ADDRESS_MAP_START( aussiebyte_map, AS_PROGRAM, 8, aussiebyte_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("bank1")
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank2")
	AM_RANGE(0xc000, 0xffff) AM_RAM AM_REGION("mram", 0x0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( aussiebyte_io, AS_IO, 8, aussiebyte_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("sio1", z80sio0_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("pio1", z80pio_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_NOP // winchester interface
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("fdc", wd2797_t, read, write)
	AM_RANGE(0x14, 0x14) AM_DEVREADWRITE("dma", z80dma_device, read, write)
	AM_RANGE(0x15, 0x15) AM_WRITE(port15_w) // boot rom disable
	AM_RANGE(0x16, 0x16) AM_WRITE(port16_w) // fdd select
	AM_RANGE(0x17, 0x17) AM_WRITE(port17_w) // DMA mux
	AM_RANGE(0x18, 0x18) AM_WRITE(port18_w) // fdc select
	AM_RANGE(0x19, 0x19) AM_READ(port19_r) // info port
	AM_RANGE(0x1a, 0x1a) AM_WRITE(port1a_w) // membank
	AM_RANGE(0x1b, 0x1b) AM_WRITE(port1b_w) // winchester control
	AM_RANGE(0x1c, 0x1f) AM_WRITE(port1c_w) // gpebh select
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("pio2", z80pio_device, read, write)
	AM_RANGE(0x24, 0x27) AM_DEVREADWRITE("sio2", z80sio0_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x28, 0x28) AM_READ(port28_r) AM_DEVWRITE("votrax", votrax_sc01_device, write)
	AM_RANGE(0x2c, 0x2c) AM_DEVWRITE("votrax", votrax_sc01_device, inflection_w)
	AM_RANGE(0x30, 0x30) AM_WRITE(address_w)
	AM_RANGE(0x31, 0x31) AM_DEVREAD("crtc", mc6845_device, status_r)
	AM_RANGE(0x32, 0x32) AM_WRITE(register_w)
	AM_RANGE(0x33, 0x33) AM_READ(port33_r)
	AM_RANGE(0x34, 0x34) AM_WRITE(port34_w) // video control
	AM_RANGE(0x35, 0x35) AM_WRITE(port35_w) // data to vram and aram
	AM_RANGE(0x36, 0x36) AM_READ(port36_r) // data from vram and aram
	AM_RANGE(0x37, 0x37) AM_READ(port37_r) // read dispen flag
	AM_RANGE(0x40, 0x4f) AM_DEVREADWRITE("rtc", msm5832_device, data_r, data_w)
ADDRESS_MAP_END

/***********************************************************

    Keyboard

************************************************************/
static INPUT_PORTS_START( aussiebyte )
INPUT_PORTS_END

/***********************************************************

    I/O Ports

************************************************************/
WRITE8_MEMBER( aussiebyte_state::port15_w )
{
	membank("bankr0")->set_entry(m_port15); // point at ram
	m_port15 = true;
}

/* FDD select
0 Drive Select bit O
1 Drive Select bit 1
2 Drive Select bit 2
3 Drive Select bit 3
  - These bits connect to a 74LS145 binary to BCD converter.
  - Drives 0 to 3 are 5.25 inch, 4 to 7 are 8 inch, 9 and 0 are not used.
  - Currently we only support drive 0.
4 Side Select to Disk Drives.
5 Disable 5.25 inch floppy spindle motors.
6 Unused.
7 Enable write precompensation on WD2797 controller. */
WRITE8_MEMBER( aussiebyte_state::port16_w )
{
	floppy_image_device *m_floppy = nullptr;
	if ((data & 15) == 0)
		m_floppy = m_floppy0->get_device();
	else
	if ((data & 15) == 1)
		m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(data, 5));
		m_floppy->ss_w(BIT(data, 4));
	}
}

/* DMA select
0 - FDC
1 - SIO Ch A
2 - SIO Ch B
3 - Winchester bus
4 - SIO Ch C
5 - SIO Ch D
6 - Ext ready 1
7 - Ext ready 2 */
WRITE8_MEMBER( aussiebyte_state::port17_w )
{
	m_port17 = data & 7;
	m_dma->rdy_w(BIT(m_port17_rdy, data));
}

/* FDC params
2 EXC: WD2797 clock frequency. H = 5.25"; L = 8"
3 WIEN: WD2797 Double density select. */
WRITE8_MEMBER( aussiebyte_state::port18_w )
{
	m_fdc->set_unscaled_clock(BIT(data, 2) ? 1e6 : 2e6);
	m_fdc->dden_w(BIT(data, 3));
}

READ8_MEMBER( aussiebyte_state::port19_r )
{
	return m_port19;
}

// Memory banking
WRITE8_MEMBER( aussiebyte_state::port1a_w )
{
	data &= 7;
	switch (data)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			m_port1a = data*3+1;
			if (m_port15)
				membank("bankr0")->set_entry(data*3+1);
			membank("bankw0")->set_entry(data*3+1);
			membank("bank1")->set_entry(data*3+2);
			membank("bank2")->set_entry(data*3+3);
			break;
		case 5:
			m_port1a = 1;
			if (m_port15)
				membank("bankr0")->set_entry(1);
			membank("bankw0")->set_entry(1);
			membank("bank1")->set_entry(2);
			membank("bank2")->set_entry(13);
			break;
		case 6:
			m_port1a = 14;
			if (m_port15)
				membank("bankr0")->set_entry(14);
			membank("bankw0")->set_entry(14);
			membank("bank1")->set_entry(15);
			//membank("bank2")->set_entry(0); // open bus
			break;
		case 7:
			m_port1a = 1;
			if (m_port15)
				membank("bankr0")->set_entry(1);
			membank("bankw0")->set_entry(1);
			membank("bank1")->set_entry(4);
			membank("bank2")->set_entry(13);
			break;
	}
}

// Winchester control
WRITE8_MEMBER( aussiebyte_state::port1b_w )
{
}

// GPEHB control
WRITE8_MEMBER( aussiebyte_state::port1c_w )
{
}

WRITE8_MEMBER( aussiebyte_state::port20_w )
{
	m_speaker->level_w(BIT(data, 7));
}

READ8_MEMBER( aussiebyte_state::port28_r )
{
	return m_port28;
}

/***********************************************************

    DMA

************************************************************/
READ8_MEMBER( aussiebyte_state::memory_read_byte )
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER( aussiebyte_state::memory_write_byte )
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

READ8_MEMBER( aussiebyte_state::io_read_byte )
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER( aussiebyte_state::io_write_byte )
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	prog_space.write_byte(offset, data);
}

WRITE_LINE_MEMBER( aussiebyte_state::busreq_w )
{
// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_dma->bai_w(state); // tell dma that bus has been granted
}

/***********************************************************

    DMA selector

************************************************************/
WRITE_LINE_MEMBER( aussiebyte_state::sio1_rdya_w )
{
	m_port17_rdy = (m_port17_rdy & 0xfd) | (UINT8)(state << 1);
	if (m_port17 == 1)
		m_dma->rdy_w(state);
}

WRITE_LINE_MEMBER( aussiebyte_state::sio1_rdyb_w )
{
	m_port17_rdy = (m_port17_rdy & 0xfb) | (UINT8)(state << 2);
	if (m_port17 == 2)
		m_dma->rdy_w(state);
}

WRITE_LINE_MEMBER( aussiebyte_state::sio2_rdya_w )
{
	m_port17_rdy = (m_port17_rdy & 0xef) | (UINT8)(state << 4);
	if (m_port17 == 4)
		m_dma->rdy_w(state);
}

WRITE_LINE_MEMBER( aussiebyte_state::sio2_rdyb_w )
{
	m_port17_rdy = (m_port17_rdy & 0xdf) | (UINT8)(state << 5);
	if (m_port17 == 5)
		m_dma->rdy_w(state);
}


/***********************************************************

    Video

************************************************************/

/* F4 Character Displayer */
static const gfx_layout crt8002_charlayout =
{
	8, 12,                   /* 7 x 11 characters */
	128,                  /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( crt8002 )
	GFXDECODE_ENTRY( "chargen", 0x0000, crt8002_charlayout, 0, 1 )
GFXDECODE_END

/***************************************************************

    Daisy Chain

****************************************************************/

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "dma" },
	{ "pio2" },
	{ "sio1" },
	{ "sio2" },
	{ "pio1" },
	{ "ctc" },
	{ nullptr }
};


/***********************************************************

    CTC

************************************************************/

// baud rate generator. All inputs are 1.2288MHz.
TIMER_DEVICE_CALLBACK_MEMBER( aussiebyte_state::ctc_tick )
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);
	m_ctc->trg1(1);
	m_ctc->trg1(0);
	m_ctc->trg2(1);
	m_ctc->trg2(0);
}

WRITE_LINE_MEMBER( aussiebyte_state::ctc_z0_w )
{
	m_sio1->rxca_w(state);
	m_sio1->txca_w(state);
}

WRITE_LINE_MEMBER( aussiebyte_state::ctc_z1_w )
{
	m_sio1->rxtxcb_w(state);
	m_sio2->rxca_w(state);
	m_sio2->txca_w(state);
}

WRITE_LINE_MEMBER( aussiebyte_state::ctc_z2_w )
{
	m_sio2->rxtxcb_w(state);
	m_ctc->trg3(1);
	m_ctc->trg3(0);
}

/***********************************************************

    Centronics ack

************************************************************/
WRITE_LINE_MEMBER( aussiebyte_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

/***********************************************************

    Speech ack

************************************************************/
WRITE_LINE_MEMBER( aussiebyte_state::votrax_w )
{
	m_port28 = state;
}


/***********************************************************

    Floppy Disk

************************************************************/

WRITE_LINE_MEMBER( aussiebyte_state::fdc_intrq_w )
{
	UINT8 data = (m_port19 & 0xbf) | (state ? 0x40 : 0);
	m_port19 = data;
}

WRITE_LINE_MEMBER( aussiebyte_state::fdc_drq_w )
{
	UINT8 data = (m_port19 & 0x7f) | (state ? 0x80 : 0);
	m_port19 = data;
	state ^= 1; // inverter on pin38 of fdc
	m_port17_rdy = (m_port17_rdy & 0xfe) | (UINT8)state;
	if (m_port17 == 0)
		m_dma->rdy_w(state);
}

static SLOT_INTERFACE_START( aussiebyte_floppies )
	SLOT_INTERFACE( "drive0", FLOPPY_525_QD )
	SLOT_INTERFACE( "drive1", FLOPPY_525_QD )
SLOT_INTERFACE_END

/***********************************************************

    Machine Driver

************************************************************/
MACHINE_RESET_MEMBER( aussiebyte_state, aussiebyte )
{
	m_port15 = false;
	m_port17 = 0;
	m_port17_rdy = 0;
	m_port1a = 1;
	m_alpha_address = 0;
	m_graph_address = 0;
	m_p_chargen = memregion("chargen")->base();
	m_p_videoram = memregion("vram")->base();
	m_p_attribram = memregion("aram")->base();
	membank("bankr0")->set_entry(16); // point at rom
	membank("bankw0")->set_entry(1); // always write to ram
	membank("bank1")->set_entry(2);
	membank("bank2")->set_entry(3);
	m_maincpu->reset();
}

static MACHINE_CONFIG_START( aussiebyte, aussiebyte_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(aussiebyte_map)
	MCFG_CPU_IO_MAP(aussiebyte_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)

	MCFG_MACHINE_RESET_OVERRIDE(aussiebyte_state, aussiebyte )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", sy6545_1_device, screen_update)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", crt8002)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_DEVICE_ADD("votrax", VOTRAX_SC01, 720000) /* 720kHz? needs verify */
	MCFG_VOTRAX_SC01_REQUEST_CB(WRITELINE(aussiebyte_state, votrax_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_DATA_INPUT_BUFFER("cent_data_in")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(aussiebyte_state, write_centronics_busy))
	MCFG_DEVICE_ADD("cent_data_in", INPUT_BUFFER, 0)
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_16MHz / 4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(aussiebyte_state, ctc_z0_w))    // SIO1 Ch A
	MCFG_Z80CTC_ZC1_CB(WRITELINE(aussiebyte_state, ctc_z1_w))    // SIO1 Ch B, SIO2 Ch A
	MCFG_Z80CTC_ZC2_CB(WRITELINE(aussiebyte_state, ctc_z2_w))    // SIO2 Ch B, CTC Ch 3

	MCFG_DEVICE_ADD("dma", Z80DMA, XTAL_16MHz / 4)
	MCFG_Z80DMA_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80DMA_OUT_BUSREQ_CB(WRITELINE(aussiebyte_state, busreq_w))
	// BAO, not used
	MCFG_Z80DMA_IN_MREQ_CB(READ8(aussiebyte_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(aussiebyte_state, memory_write_byte))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(aussiebyte_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(aussiebyte_state, io_write_byte))

	MCFG_DEVICE_ADD("pio1", Z80PIO, XTAL_16MHz / 4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_Z80PIO_IN_PB_CB(DEVREAD8("cent_data_in", input_buffer_device, read))
	MCFG_Z80PIO_OUT_ARDY_CB(DEVWRITELINE("centronics", centronics_device, write_strobe)) MCFG_DEVCB_INVERT

	MCFG_DEVICE_ADD("pio2", Z80PIO, XTAL_16MHz / 4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(aussiebyte_state, port20_w))

	MCFG_Z80SIO0_ADD("sio1", XTAL_16MHz / 4, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80DART_OUT_WRDYA_CB(WRITELINE(aussiebyte_state, sio1_rdya_w))
	MCFG_Z80DART_OUT_WRDYB_CB(WRITELINE(aussiebyte_state, sio1_rdyb_w))

	MCFG_Z80SIO0_ADD("sio2", XTAL_16MHz / 4, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80DART_OUT_WRDYA_CB(WRITELINE(aussiebyte_state, sio2_rdya_w))
	MCFG_Z80DART_OUT_WRDYB_CB(WRITELINE(aussiebyte_state, sio2_rdyb_w))
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio2", z80sio0_device, rxa_w))

	MCFG_WD2797_ADD("fdc", XTAL_16MHz / 16)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(aussiebyte_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(aussiebyte_state, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", aussiebyte_floppies, "drive0", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", aussiebyte_floppies, "drive1", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	/* devices */
	MCFG_MC6845_ADD("crtc", SY6545_1, "screen", XTAL_16MHz / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(aussiebyte_state, crtc_update_row)
	MCFG_MC6845_ADDR_CHANGED_CB(aussiebyte_state, crtc_update_addr)

	MCFG_MSM5832_ADD("rtc", XTAL_32_768kHz)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc_tick", aussiebyte_state, ctc_tick, attotime::from_hz(XTAL_4_9152MHz / 4))
MACHINE_CONFIG_END


DRIVER_INIT_MEMBER( aussiebyte_state, aussiebyte )
{
	// Main ram is divided into 16k blocks (0-15). The boot rom is block number 16.
	// For convenience, bank 0 is permanently assigned to C000-FFFF
	UINT8 *main = memregion("roms")->base();
	UINT8 *ram = memregion("mram")->base();

	membank("bankr0")->configure_entries(0, 16, &ram[0x0000], 0x4000);
	membank("bankw0")->configure_entries(0, 16, &ram[0x0000], 0x4000);
	membank("bank1")->configure_entries(0, 16, &ram[0x0000], 0x4000);
	membank("bank2")->configure_entries(0, 16, &ram[0x0000], 0x4000);
	membank("bankr0")->configure_entry(16, &main[0x0000]);
}


/***********************************************************

    Game driver

************************************************************/


ROM_START(aussieby)
	ROM_REGION(0x4000, "roms", 0) // Size of bank 16
	ROM_LOAD( "knight_boot_0000.u27", 0x0000, 0x1000, CRC(1f200437) SHA1(80d1d208088b325c16a6824e2da605fb2b00c2ce) )

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD( "8002.bin", 0x0000, 0x0800, CRC(fdd6eb13) SHA1(a094d416e66bdab916e72238112a6265a75ca690) )

	ROM_REGION(0x40000, "mram", ROMREGION_ERASE00) // main ram, 256k dynamic
	ROM_REGION(0x10000, "vram", ROMREGION_ERASEFF) // video ram, 64k dynamic
	ROM_REGION(0x00800, "aram", ROMREGION_ERASEFF) // attribute ram, 2k static
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE     INPUT        CLASS             INIT         COMPANY         FULLNAME          FLAGS */
COMP( 1984, aussieby,     0,        0,  aussiebyte, aussiebyte,  aussiebyte_state, aussiebyte, "SME Systems",  "Aussie Byte II" , MACHINE_IMPERFECT_GRAPHICS )
