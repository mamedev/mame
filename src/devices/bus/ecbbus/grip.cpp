// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Conitec Datensysteme GRIP graphics card emulation

**********************************************************************/

#include "grip.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define SCREEN_TAG          "screen"
#define Z80_TAG             "grip_z1"
#define MC6845_TAG          "z30"
#define HD6345_TAG          "z30"
#define I8255A_TAG          "z6"
#define Z80STI_TAG          "z9"
#define CENTRONICS_TAG      "centronics"


#define VIDEORAM_SIZE       0x10000



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ECB_GRIP21 = &device_creator<grip_device>;



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( grip21 )
//-------------------------------------------------

ROM_START( grip21 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_LOAD( "grip21.z2", 0x0000, 0x4000, CRC(7f6a37dd) SHA1(2e89f0b0c378257ff7e41c50d57d90865c6e214b) )
ROM_END


#if 0
//-------------------------------------------------
//  ROM( grip25 )
//-------------------------------------------------

ROM_START( grip25 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_LOAD( "grip25.z2", 0x0000, 0x4000, CRC(49ebb284) SHA1(0a7eaaf89da6db2750f820146c8f480b7157c6c7) )
ROM_END


//-------------------------------------------------
//  ROM( grip26 )
//-------------------------------------------------

ROM_START( grip26 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_LOAD( "grip26.z2", 0x0000, 0x4000, CRC(a1c424f0) SHA1(83942bc75b9475f044f936b8d9d7540551d87db9) )
ROM_END


//-------------------------------------------------
//  ROM( grip31 )
//-------------------------------------------------

ROM_START( grip31 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_LOAD( "grip31.z2", 0x0000, 0x4000, CRC(e0e4e8ab) SHA1(73d3d14c9b06fed0c187fb0fffe5ec035d8dd256) )
ROM_END


//-------------------------------------------------
//  ROM( grip562 )
//-------------------------------------------------

ROM_START( grip562 )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_LOAD( "grip562.z2", 0x0000, 0x8000, CRC(74be0455) SHA1(1c423ecca6363345a8690ddc45dbafdf277490d3) )
ROM_END


//-------------------------------------------------
//  ROM( grips115 )
//-------------------------------------------------

ROM_START( grips115 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_LOAD( "grips115.z2", 0x0000, 0x4000, CRC(505706ef) SHA1(05fb032fb1a504c534c30c352ba4bd47623503d0) )
ROM_END
#endif


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *grip_device::device_rom_region() const
{
	return ROM_NAME( grip21 );
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( grip_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( grip_mem, AS_PROGRAM, 8, grip_device )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_RAMBANK("videoram")
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( grip_io )
//-------------------------------------------------

static ADDRESS_MAP_START( grip_io, AS_IO, 8, grip_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(cxstb_r, cxstb_w)
//  AM_RANGE(0x10, 0x10) AM_WRITE(ccon_w)
	AM_RANGE(0x11, 0x11) AM_WRITE(vol0_w)
//  AM_RANGE(0x12, 0x12) AM_WRITE(rts_w)
	AM_RANGE(0x13, 0x13) AM_WRITE(page_w)
//  AM_RANGE(0x14, 0x14) AM_WRITE(cc1_w)
//  AM_RANGE(0x15, 0x15) AM_WRITE(cc2_w)
	AM_RANGE(0x16, 0x16) AM_WRITE(flash_w)
	AM_RANGE(0x17, 0x17) AM_WRITE(vol1_w)
	AM_RANGE(0x20, 0x2f) AM_DEVREADWRITE(Z80STI_TAG, z80sti_device, read, write)
	AM_RANGE(0x30, 0x30) AM_READWRITE(lrs_r, lrs_w)
	AM_RANGE(0x40, 0x40) AM_READ(stat_r)
	AM_RANGE(0x50, 0x50) AM_DEVWRITE(MC6845_TAG, mc6845_device, address_w)
	AM_RANGE(0x52, 0x52) AM_DEVWRITE(MC6845_TAG, mc6845_device, register_w)
	AM_RANGE(0x53, 0x53) AM_DEVREAD(MC6845_TAG, mc6845_device, register_r)
	AM_RANGE(0x60, 0x60) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE(0x70, 0x73) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)
//  AM_RANGE(0x80, 0x80) AM_WRITE(bl2out_w)
//  AM_RANGE(0x90, 0x90) AM_WRITE(gr2out_w)
//  AM_RANGE(0xa0, 0xa0) AM_WRITE(rd2out_w)
//  AM_RANGE(0xb0, 0xb0) AM_WRITE(clrg2_w)
//  AM_RANGE(0xc0, 0xc0) AM_WRITE(bluout_w)
//  AM_RANGE(0xd0, 0xd0) AM_WRITE(grnout_w)
//  AM_RANGE(0xe0, 0xe0) AM_WRITE(redout_w)
//  AM_RANGE(0xf0, 0xf0) AM_WRITE(clrg1_w)
ADDRESS_MAP_END

/*
//-------------------------------------------------
//  ADDRESS_MAP( grip5_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( grip5_mem, AS_PROGRAM, 8, grip5_state )
    AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("eprom")
    AM_RANGE(0x4000, 0x5fff) AM_RAM
    AM_RANGE(0x8000, 0xffff) AM_RAMBANK("videoram")
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( grip5_io )
//-------------------------------------------------

static ADDRESS_MAP_START( grip5_io, AS_IO, 8, grip5_device )
    ADDRESS_MAP_GLOBAL_MASK(0xff)
    AM_RANGE(0x00, 0x00) AM_READWRITE(cxstb_r, cxstb_w)
    AM_RANGE(0x10, 0x10) AM_WRITE(eprom_w)
    AM_RANGE(0x11, 0x11) AM_WRITE(vol0_w)
//  AM_RANGE(0x12, 0x12) AM_WRITE(rts_w)
    AM_RANGE(0x13, 0x13) AM_WRITE(page_w)
//  AM_RANGE(0x14, 0x14) AM_WRITE(str_w)
//  AM_RANGE(0x15, 0x15) AM_WRITE(intl_w)
    AM_RANGE(0x16, 0x16) AM_WRITE(dpage_w)
    AM_RANGE(0x17, 0x17) AM_WRITE(vol1_w)
    AM_RANGE(0x20, 0x2f) AM_DEVREADWRITE(Z80STI_TAG, z80sti_device, read, write)
    AM_RANGE(0x30, 0x30) AM_READWRITE(lrs_r, lrs_w)
    AM_RANGE(0x40, 0x40) AM_READ(stat_r)
    AM_RANGE(0x50, 0x50) AM_DEVWRITE(HD6345_TAG, hd6345_device, address_w)
    AM_RANGE(0x52, 0x52) AM_DEVWRITE(HD6345_TAG, hd6345_device, register_w)
    AM_RANGE(0x53, 0x53) AM_DEVREAD(HD6345_TAG, hd6345_device, register_r)
    AM_RANGE(0x60, 0x60) AM_DEVWRITE("cent_data_out", output_latch_device, write)
    AM_RANGE(0x70, 0x73) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)

//  AM_RANGE(0x80, 0x80) AM_WRITE(xrflgs_w)
//  AM_RANGE(0xc0, 0xc0) AM_WRITE(xrclrg_w)
//  AM_RANGE(0xe0, 0xe0) AM_WRITE(xrclu0_w)
//  AM_RANGE(0xe1, 0xe1) AM_WRITE(xrclu1_w)
//  AM_RANGE(0xe2, 0xe2) AM_WRITE(xrclu2_w)

//  AM_RANGE(0x80, 0x80) AM_WRITE(bl2out_w)
//  AM_RANGE(0x90, 0x90) AM_WRITE(gr2out_w)
//  AM_RANGE(0xa0, 0xa0) AM_WRITE(rd2out_w)
//  AM_RANGE(0xb0, 0xb0) AM_WRITE(clrg2_w)
//  AM_RANGE(0xc0, 0xc0) AM_WRITE(bluout_w)
//  AM_RANGE(0xd0, 0xd0) AM_WRITE(grnout_w)
//  AM_RANGE(0xe0, 0xe0) AM_WRITE(redout_w)
//  AM_RANGE(0xf0, 0xf0) AM_WRITE(clrg1_w)
ADDRESS_MAP_END
*/



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( grip_device::crtc_update_row )
{
	for (int column = 0; column < x_count; column++)
	{
		UINT16 address = (m_page << 12) | (((ma + column) & 0xfff) << 3) | (ra & 0x07);
		UINT8 data = m_video_ram[address];

		for (int bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;
			int color = (m_flash ? 0 : BIT(data, bit)) && de;

			bitmap.pix32(vbp + y, hbp + x) = m_palette->pen(color);
		}
	}
}
/*
MC6845_UPDATE_ROW( grip_device::grip5_update_row )
{
    const rgb_t *palette = m_palette->palette()->entry_list_raw();
    int column, bit;

    for (column = 0; column < x_count; column++)
    {
        UINT16 address = (m_dpage << 12) | (((ma + column) & 0xfff) << 3) | (ra & 0x07);
        UINT8 data = m_video_ram[address];

        for (bit = 0; bit < 8; bit++)
        {
            int x = (column * 8) + bit;
            int color = m_flash ? 0 : BIT(data, bit);

            bitmap.pix32(y, x) = palette[color];
        }
    }
}

MC6845_ON_UPDATE_ADDR_CHANGED( grip_device::grip5_addr_changed )
{
}
*/

static const INT16 speaker_levels[] = { -32768, 0, 32767, 0 };

//-------------------------------------------------
//  I8255A interface
//-------------------------------------------------

READ8_MEMBER( grip_device::ppi_pa_r )
{
	/*

	    bit     description

	    PA0     ECB bus D0
	    PA1     ECB bus D1
	    PA2     ECB bus D2
	    PA3     ECB bus D3
	    PA4     ECB bus D4
	    PA5     ECB bus D5
	    PA6     ECB bus D6
	    PA7     ECB bus D7

	*/

	return m_ppi_pa;
}

WRITE8_MEMBER( grip_device::ppi_pa_w )
{
	/*

	    bit     description

	    PA0     ECB bus D0
	    PA1     ECB bus D1
	    PA2     ECB bus D2
	    PA3     ECB bus D3
	    PA4     ECB bus D4
	    PA5     ECB bus D5
	    PA6     ECB bus D6
	    PA7     ECB bus D7

	*/

	m_ppi_pa = data;
}

READ8_MEMBER( grip_device::ppi_pb_r )
{
	/*

	    bit     description

	    PB0     Keyboard input
	    PB1     Keyboard input
	    PB2     Keyboard input
	    PB3     Keyboard input
	    PB4     Keyboard input
	    PB5     Keyboard input
	    PB6     Keyboard input
	    PB7     Keyboard input

	*/

	return m_keydata;
}

WRITE8_MEMBER( grip_device::ppi_pc_w )
{
	/*

	    bit     signal      description

	    PC0     INTRB       interrupt B output (keyboard)
	    PC1     KBF         input buffer B full output (keyboard)
	    PC2     _KBSTB      strobe B input (keyboard)
	    PC3     INTRA       interrupt A output (PROF-80)
	    PC4     _STBA       strobe A input (PROF-80)
	    PC5     IBFA        input buffer A full output (PROF-80)
	    PC6     _ACKA       acknowledge A input (PROF-80)
	    PC7     _OBFA       output buffer full output (PROF-80)

	*/

	// keyboard interrupt
	m_ib = BIT(data, 0);
	m_sti->i4_w(m_ib);

	// keyboard buffer full
	m_kbf = BIT(data, 1);

	// PROF-80 interrupt
	m_ia = BIT(data, 3);
	m_sti->i7_w(m_ia);

	// PROF-80 handshaking
	m_ppi_pc = (!BIT(data, 7) << 7) | (!BIT(data, 5) << 6) | (m_ppi->pa_r() & 0x3f);
}

//-------------------------------------------------
//  Z80STI_INTERFACE( sti_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER(grip_device::write_centronics_busy)
{
	m_centronics_busy = state;
}

READ8_MEMBER( grip_device::sti_gpio_r )
{
	/*

	    bit     signal      description

	    I0      _CTS        RS-232 clear to send input
	    I1      DE          MC6845 display enable input
	    I2      CURSOR      MC6845 cursor input
	    I3      BUSY        Centronics busy input
	    I4      IB          PPI8255 PC0 input
	    I5      _SKBD       Serial keyboard input
	    I6      EXIN        External interrupt input
	    I7      IA          PPI8255 PC3 input

	*/

	UINT8 data = 0x20;

	// display enable
	data |= m_crtc->de_r() << 1;

	// cursor
	data |= m_crtc->cursor_r() << 2;

	// centronics busy
	data |= m_centronics_busy << 3;

	// keyboard interrupt
	data |= m_ib << 4;

	// PROF-80 interrupt
	data |= m_ia << 7;

	return data;
}

WRITE_LINE_MEMBER( grip_device::speaker_w )
{
	int level = state && ((m_vol1 << 1) | m_vol0);

	m_speaker->level_w(level);
}

//-------------------------------------------------
//  z80_daisy_config grip_daisy_chain
//-------------------------------------------------

static const z80_daisy_config grip_daisy_chain[] =
{
	{ Z80STI_TAG },
	{ nullptr }
};


//-------------------------------------------------
//  ASCII_KEYBOARD_INTERFACE( kb_intf )
//-------------------------------------------------

WRITE8_MEMBER( grip_device::kb_w )
{
	if (!m_kbf)
	{
		m_keydata = data;

		// trigger GRIP 8255 port C bit 2 (_STBB)
		m_ppi->pc2_w(0);
		m_ppi->pc2_w(1);
	}
}

//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( grip )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( grip )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_16MHz/4)
	MCFG_CPU_CONFIG(grip_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(grip_mem)
	MCFG_CPU_IO_MAP(grip_io)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_TAG, XTAL_16MHz/4)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(grip_device, crtc_update_row)
	MCFG_MC6845_OUT_DE_CB(DEVWRITELINE(Z80STI_TAG, z80sti_device, i1_w))
	MCFG_MC6845_OUT_CUR_CB(DEVWRITELINE(Z80STI_TAG, z80sti_device, i1_w))

//  MCFG_MC6845_ADD(HD6345_TAG, HD6345, SCREEN_TAG, XTAL_16MHz/4)

	MCFG_DEVICE_ADD(I8255A_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(grip_device, ppi_pa_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(grip_device, ppi_pa_w))
	MCFG_I8255_IN_PORTB_CB(READ8(grip_device, ppi_pb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(grip_device, ppi_pc_w))

	MCFG_DEVICE_ADD(Z80STI_TAG, Z80STI, XTAL_16MHz/4)
	MCFG_Z80STI_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80STI_IN_GPIO_CB(READ8(grip_device, sti_gpio_r))
	MCFG_Z80STI_OUT_TBO_CB(WRITELINE(grip_device, speaker_w))
	MCFG_Z80STI_OUT_TCO_CB(DEVWRITELINE(Z80STI_TAG, z80sti_device, tc_w))
	MCFG_Z80STI_OUT_TDO_CB(DEVWRITELINE(Z80STI_TAG, z80sti_device, tc_w))

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(grip_device, write_centronics_busy))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(grip_device, write_centronics_fault))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(grip_device, kb_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor grip_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( grip );
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( grip )
//-------------------------------------------------

static INPUT_PORTS_START( grip )
	PORT_START("J1")
	PORT_CONFNAME( 0x01, 0x00, "J1 EPROM Type")
	PORT_CONFSETTING( 0x00, "2732" )
	PORT_CONFSETTING( 0x01, "2764/27128" )

	PORT_START("J2")
	PORT_CONFNAME( 0x03, 0x00, "J2 Centronics Mode")
	PORT_CONFSETTING( 0x00, "Mode 1" )
	PORT_CONFSETTING( 0x01, "Mode 2" )
	PORT_CONFSETTING( 0x02, "Mode 3" )

	PORT_START("J3A")
	PORT_CONFNAME( 0x07, 0x00, "J3 Host")
	PORT_CONFSETTING( 0x00, "ECB Bus" )
	PORT_CONFSETTING( 0x01, "V24 9600 Baud" )
	PORT_CONFSETTING( 0x02, "V24 4800 Baud" )
	PORT_CONFSETTING( 0x03, "V24 1200 Baud" )
	PORT_CONFSETTING( 0x04, "Keyboard" )

	PORT_START("J3B")
	PORT_CONFNAME( 0x07, 0x00, "J3 Keyboard")
	PORT_CONFSETTING( 0x00, "Parallel" )
	PORT_CONFSETTING( 0x01, "Serial (1200 Baud, 8 Bits)" )
	PORT_CONFSETTING( 0x02, "Serial (1200 Baud, 7 Bits)" )
	PORT_CONFSETTING( 0x03, "Serial (600 Baud, 8 Bits)" )
	PORT_CONFSETTING( 0x04, "Serial (600 Baud, 7 Bits)" )

	PORT_START("J4")
	PORT_CONFNAME( 0x01, 0x00, "J4 COLOR")
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x01, DEF_STR( Yes ) )

	PORT_START("J5")
	PORT_CONFNAME( 0x01, 0x01, "J5 Power On Reset")
	PORT_CONFSETTING( 0x00, "External" )
	PORT_CONFSETTING( 0x01, "Internal" )

	PORT_START("J6")
	PORT_CONFNAME( 0x03, 0x00, "J6 Serial Clock")
	PORT_CONFSETTING( 0x00, "TC/16, TD/16, TD" )
	PORT_CONFSETTING( 0x01, "TD/16, TD/16, TD" )
	PORT_CONFSETTING( 0x02, "TC/16, BAUD/16, input" )
	PORT_CONFSETTING( 0x03, "BAUD/16, BAUD/16, input" )

	PORT_START("J7")
	PORT_CONFNAME( 0xff, 0xc0, "J7 ECB Bus Address")
	PORT_CONFSETTING( 0xc0, "C0/C1" )
	PORT_CONFSETTING( 0xa0, "A0/A1" )

	PORT_START("J8")
	PORT_CONFNAME( 0x01, 0x00, "J8 Video RAM")
	PORT_CONFSETTING( 0x00, "32 KB" )
	PORT_CONFSETTING( 0x01, "64 KB" )

	PORT_START("J9")
	PORT_CONFNAME( 0x01, 0x01, "J9 CPU Clock")
	PORT_CONFSETTING( 0x00, "2 MHz" )
	PORT_CONFSETTING( 0x01, "4 MHz" )

	PORT_START("J10")
	PORT_CONFNAME( 0x01, 0x01, "J10 Pixel Clock")
	PORT_CONFSETTING( 0x00, "External" )
	PORT_CONFSETTING( 0x01, "Internal" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor grip_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( grip );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  grip_device - constructor
//-------------------------------------------------

grip_device::grip_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ECB_GRIP21, "GRIP-2.1", tag, owner, clock, "grip", __FILE__),
	device_ecbbus_card_interface(mconfig, *this),
	m_ppi(*this, I8255A_TAG),
	m_sti(*this, Z80STI_TAG),
	m_crtc(*this, MC6845_TAG),
	m_centronics(*this, CENTRONICS_TAG),
	m_palette(*this, "palette"),
	m_speaker(*this, "speaker"),
	m_video_ram(*this, "video_ram"),
	m_j3a(*this, "J3A"),
	m_j3b(*this, "J3B"),
	m_j7(*this, "J7"), m_centronics_busy(0), m_centronics_fault(0), m_vol0(0), m_vol1(0), m_ia(0), m_ib(0), m_keydata(0), m_kbf(0), m_lps(0), m_page(0), m_flash(0), m_base(0), m_ppi_pa(0), m_ppi_pc(0), m_kb_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void grip_device::device_start()
{
	// allocate video RAM
	m_video_ram.allocate(VIDEORAM_SIZE);

	// setup GRIP memory banking
	membank("videoram")->configure_entries(0, 2, m_video_ram, 0x8000);
	membank("videoram")->set_entry(0);

	// allocate keyboard scan timer
	m_kb_timer = timer_alloc();
	m_kb_timer->adjust(attotime::zero, 0, attotime::from_hz(2500));

	// register for state saving
	save_item(NAME(m_vol0));
	save_item(NAME(m_vol1));
	save_item(NAME(m_keydata));
	save_item(NAME(m_kbf));
	save_item(NAME(m_lps));
	save_item(NAME(m_page));
	save_item(NAME(m_flash));
}

/*
void grip5_state::machine_start()
{
    grip_device::machine_start();

    // setup ROM banking
    membank("eprom")->configure_entries(0, 2, memregion(Z80_TAG)->base(), 0x4000);
    *this.root_device().membank("eprom")->set_entry(0);

    // register for state saving
    save_item(NAME(m_dpage));
}
*/

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void grip_device::device_reset()
{
	m_base = m_j7->read();
	m_page = 0;
	m_lps = 0;
}


//-------------------------------------------------
//  vol0_w - volume 0
//-------------------------------------------------

WRITE8_MEMBER( grip_device::vol0_w )
{
	m_vol0 = BIT(data, 7);
}


//-------------------------------------------------
//  vol1_w - volume 1
//-------------------------------------------------

WRITE8_MEMBER( grip_device::vol1_w )
{
	m_vol1 = BIT(data, 7);
}


//-------------------------------------------------
//  flash_w -
//-------------------------------------------------

WRITE8_MEMBER( grip_device::flash_w )
{
	m_flash = BIT(data, 7);
}


//-------------------------------------------------
//  page_w - video page select
//-------------------------------------------------

WRITE8_MEMBER( grip_device::page_w )
{
	m_page = BIT(data, 7);

	membank("videoram")->set_entry(m_page);
}


//-------------------------------------------------
//  stat_r -
//-------------------------------------------------

WRITE_LINE_MEMBER(grip_device::write_centronics_fault)
{
	m_centronics_fault = state;
}

READ8_MEMBER( grip_device::stat_r )
{
	/*

	    bit     signal      description

	    0       LPA0
	    1       LPA1
	    2       LPA2
	    3       SENSE
	    4       JS0
	    5       JS1
	    6       _ERROR
	    7       LPSTB       light pen strobe

	*/

	UINT8 data = 0;
	int js0 = 0, js1 = 0;

	// JS0
	switch (m_j3a->read())
	{
	case 0: js0 = 0; break;
	case 1: js0 = 1; break;
	case 2: js0 = m_vol0; break;
	case 3: js0 = m_vol1; break;
	case 4: js0 = m_page; break;
	}

	data |= js0 << 4;

	// JS1
	switch (m_j3b->read())
	{
	case 0: js1 = 0; break;
	case 1: js1 = 1; break;
	case 2: js1 = m_vol0; break;
	case 3: js1 = m_vol1; break;
	case 4: js1 = m_page; break;
	}

	data |= js1 << 5;

	// centronics fault
	data |= m_centronics_fault << 6;

	// light pen strobe
	data |= m_lps << 7;

	return data;
}


//-------------------------------------------------
//  lrs_r -
//-------------------------------------------------

READ8_MEMBER( grip_device::lrs_r )
{
	m_lps = 0;

	return 0;
}


//-------------------------------------------------
//  lrs_w -
//-------------------------------------------------

WRITE8_MEMBER( grip_device::lrs_w )
{
	m_lps = 0;
}


//-------------------------------------------------
//  cxstb_r - centronics strobe
//-------------------------------------------------

READ8_MEMBER( grip_device::cxstb_r )
{
	m_centronics->write_strobe(0);
	m_centronics->write_strobe(1);

	return 0;
}


//-------------------------------------------------
//  cxstb_w - centronics strobe
//-------------------------------------------------

WRITE8_MEMBER( grip_device::cxstb_w )
{
	m_centronics->write_strobe(0);
	m_centronics->write_strobe(1);
}

/*
//-------------------------------------------------
//  eprom_w - EPROM bank select
//-------------------------------------------------

WRITE8_MEMBER( grip5_state::eprom_w )
{
    membank("eprom")->set_entry(BIT(data, 0));
}


//-------------------------------------------------
//  dpage_w - display page select
//-------------------------------------------------

WRITE8_MEMBER( grip5_state::dpage_w )
{
    m_dpage = BIT(data, 7);
}
*/


//-------------------------------------------------
//  ecbbus_io_r - I/O read
//-------------------------------------------------

UINT8 grip_device::ecbbus_io_r(offs_t offset)
{
	UINT8 data = 0;

	if ((offset & 0xfe) == m_base)
	{
		if (BIT(offset, 0))
		{
			data = m_ppi_pa;

			// acknowledge PPI port A
			m_ppi->pc6_w(0);
			m_ppi->pc6_w(1);
		}
		else
		{
			data = m_ppi_pc;
		}
	}

	return data;
}


//-------------------------------------------------
//  ecbbus_io_w - I/O write
//-------------------------------------------------

void grip_device::ecbbus_io_w(offs_t offset, UINT8 data)
{
	if ((offset & 0xfe) == m_base)
	{
		if (BIT(offset, 0))
		{
			m_ppi_pa = data;

			// strobe PPI port A
			m_ppi->pc4_w(0);
			m_ppi->pc4_w(1);
		}
	}
}
