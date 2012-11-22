/***************************************************************************

    Tandy 2000

    Skeleton driver.

****************************************************************************/

/*

    TODO:

    - CRT9007
    - CRT9212 Double Row Buffer
    - CRT9021B Attribute Generator
    - 80186
    - keyboard ROM
    - hires graphics board
    - floppy 720K DSQD
    - DMA
    - WD1010
    - hard disk
    - mouse

*/

#include "includes/tandy2k.h"
#include "formats/mfi_dsk.h"

enum
{
	LPINEN = 0,
	KBDINEN,
	PORTINEN
};

// Read/Write Handlers

void tandy2k_state::dma_request(int line, int state)
{

}

void tandy2k_state::speaker_update()
{
	int level = !(m_spkrdata & m_outspkr);

	speaker_level_w(m_speaker, level);
}

READ8_MEMBER( tandy2k_state::videoram_r )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	offs_t addr = (m_vram_base << 15) | (offset << 1);
	UINT16 data = program.read_word(addr);

	// character
	m_drb0->write(space, 0, data & 0xff);

	// attributes
	m_drb1->write(space, 0, data >> 8);

	return data & 0xff;
}

READ8_MEMBER( tandy2k_state::enable_r )
{
	/*

        bit     signal      description

        0                   RS-232 ring indicator
        1                   RS-232 carrier detect
        2
        3
        4
        5
        6
        7       _ACLOW

    */

	return 0x80;
}

WRITE8_MEMBER( tandy2k_state::enable_w )
{
	/*

        bit     signal      description

        0       KBEN        keyboard enable
        1       EXTCLK      external baud rate clock
        2       SPKRGATE    enable periodic m_speaker output
        3       SPKRDATA    direct output to m_speaker
        4       RFRQGATE    enable refresh and baud rate clocks
        5       FDCRESET*   reset 8272
        6       TMRIN0      enable 80186 timer 0
        7       TMRIN1      enable 80186 timer 1

    */

	// keyboard enable
	m_kb->power_w(BIT(data, 0));

	// external baud rate clock
	m_extclk = BIT(data, 1);

	// m_speaker gate
	pit8253_gate0_w(m_pit, BIT(data, 2));

	// m_speaker data
	m_spkrdata = BIT(data, 3);
	speaker_update();

	// refresh and baud rate clocks
	pit8253_gate1_w(m_pit, BIT(data, 4));
	pit8253_gate2_w(m_pit, BIT(data, 4));

	// FDC reset
	if(BIT(data, 5))
		m_fdc->reset();

	// timer 0 enable
	m_maincpu->set_input_line(INPUT_LINE_TMRIN0, BIT(data, 6));

	// timer 1 enable
	m_maincpu->set_input_line(INPUT_LINE_TMRIN1, BIT(data, 7));
}

WRITE8_MEMBER( tandy2k_state::dma_mux_w )
{
	/*

        bit     description

        0       DMA channel 0 enable
        1       DMA channel 1 enable
        2       DMA channel 2 enable
        3       DMA channel 3 enable
        4       DMA channel 0 select
        5       DMA channel 1 select
        6       DMA channel 2 select
        7       DMA channel 3 select

    */

	m_dma_mux = data;

	// check for DMA error
	int drq0 = 0;
	int drq1 = 0;

	for (int ch = 0; ch < 4; ch++)
	{
		if (BIT(data, ch)) { if (BIT(data, ch + 4)) drq1++; else drq0++; }
	}

	int dme = (drq0 > 2) || (drq1 > 2);

	pic8259_ir6_w(m_pic1, dme);
}

READ8_MEMBER( tandy2k_state::kbint_clr_r )
{
	if (m_pb_sel == KBDINEN)
	{
		m_kb->busy_w(1);
		pic8259_ir0_w(m_pic1, CLEAR_LINE);
	}

	return 0xff;
}

READ16_MEMBER( tandy2k_state::vpac_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return m_vpac->read(space, offset);
	}
	else
	{
		return 0xff00;
	}
}

WRITE16_MEMBER( tandy2k_state::vpac_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_vpac->write(space, offset, data & 0xff);
	}
	else
	{
		addr_ctrl_w(space, offset, data >> 8);
	}
}

READ8_MEMBER( tandy2k_state::fldtc_r )
{
	m_fdc->tc_w(true);
	m_fdc->tc_w(false);

	return 0;
}

WRITE8_MEMBER( tandy2k_state::fldtc_w )
{
	m_fdc->tc_w(true);
	m_fdc->tc_w(false);
}

WRITE8_MEMBER( tandy2k_state::addr_ctrl_w )
{
	/*

        bit     signal      description

        8       A15         A15 of video access
        9       A16         A16 of video access
        10      A17         A17 of video access
        11      A18         A18 of video access
        12      A19         A19 of video access
        13      CLKSPD      clock speed (0 = 22.4 MHz, 1 = 28 MHz)
        14      CLKCNT      dots/char (0 = 10 [800x400], 1 = 8 [640x400])
        15      VIDOUTS     selects the video source for display on monochrome monitor

    */

	// video access
	m_vram_base = data & 0x1f;

	// dots per char
	int character_width = BIT(data, 6) ? 8 : 10;

	if (m_clkcnt != BIT(data, 6))
	{
		m_vpac->set_hpixels_per_column(character_width);
		m_clkcnt = BIT(data, 6);
	}

	// video clock speed
	if (m_clkspd != BIT(data, 5))
	{
		float pixel_clock = BIT(data, 5) ? XTAL_16MHz*28/16 : XTAL_16MHz*28/20;
		float character_clock = pixel_clock / character_width;

		m_vpac->set_unscaled_clock(pixel_clock);
		m_vac->set_unscaled_clock(character_clock);
		m_clkspd = BIT(data, 5);
	}

	// video source select
	m_vidouts = BIT(data, 7);

	logerror("Address Control %02x\n", data);
}

// Memory Maps

static ADDRESS_MAP_START( tandy2k_mem, AS_PROGRAM, 16, tandy2k_state )
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(0x00000, 0xdffff) AM_RAM
	AM_RANGE(0xe0000, 0xf7fff) AM_RAM AM_SHARE("hires_ram")
	AM_RANGE(0xf8000, 0xfbfff) AM_RAM AM_SHARE("char_ram")
	AM_RANGE(0xfc000, 0xfdfff) AM_MIRROR(0x2000) AM_ROM AM_REGION(I80186_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tandy2k_io, AS_IO, 16, tandy2k_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x00001) AM_READWRITE8(enable_r, enable_w, 0x00ff)
	AM_RANGE(0x00002, 0x00003) AM_WRITE8(dma_mux_w, 0x00ff)
	AM_RANGE(0x00004, 0x00005) AM_READWRITE8(fldtc_r, fldtc_w, 0x00ff)
	AM_RANGE(0x00010, 0x00013) AM_DEVREADWRITE8(I8251A_TAG, i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0x00030, 0x00033) AM_DEVICE8(I8272A_TAG, i8272a_device, map, 0x00ff)
	AM_RANGE(0x00040, 0x00047) AM_DEVREADWRITE8_LEGACY(I8253_TAG, pit8253_r, pit8253_w, 0x00ff)
	AM_RANGE(0x00052, 0x00053) AM_READ8(kbint_clr_r, 0x00ff)
	AM_RANGE(0x00050, 0x00057) AM_DEVREADWRITE8(I8255A_TAG, i8255_device, read, write, 0x00ff)
	AM_RANGE(0x00060, 0x00063) AM_DEVREADWRITE8_LEGACY(I8259A_0_TAG, pic8259_r, pic8259_w, 0x00ff)
	AM_RANGE(0x00070, 0x00073) AM_DEVREADWRITE8_LEGACY(I8259A_1_TAG, pic8259_r, pic8259_w, 0x00ff)
	AM_RANGE(0x00080, 0x00081) AM_DEVREADWRITE8(I8272A_TAG, i8272a_device, mdma_r, mdma_w, 0x00ff)
//  AM_RANGE(0x00100, 0x0017f) AM_DEVREADWRITE8(CRT9007_TAG, crt9007_device, read, write, 0x00ff) AM_WRITE8(addr_ctrl_w, 0xff00)
	AM_RANGE(0x00100, 0x0017f) AM_READWRITE(vpac_r, vpac_w)
//  AM_RANGE(0x00180, 0x00180) AM_READ8(hires_status_r, 0x00ff)
//  AM_RANGE(0x00180, 0x001bf) AM_WRITE(hires_palette_w)
//  AM_RANGE(0x001a0, 0x001a0) AM_READ8(hires_plane_w, 0x00ff)
//  AM_RANGE(0x0ff00, 0x0ffff) AM_READWRITE(i186_internal_port_r, i186_internal_port_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tandy2k_hd_io, AS_IO, 16, tandy2k_state )
	AM_IMPORT_FROM(tandy2k_io)
//  AM_RANGE(0x000e0, 0x000ff) AM_WRITE8(hdc_dack_w, 0x00ff)
//  AM_RANGE(0x0026c, 0x0026d) AM_DEVREADWRITE8_LEGACY(WD1010_TAG, hdc_reset_r, hdc_reset_w, 0x00ff)
//  AM_RANGE(0x0026e, 0x0027f) AM_DEVREADWRITE8_LEGACY(WD1010_TAG, wd1010_r, wd1010_w, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vpac_mem, AS_0, 8, tandy2k_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(videoram_r)
ADDRESS_MAP_END

// Input Ports

static INPUT_PORTS_START( tandy2k )
	// defined in machine/tandy2kb.c
INPUT_PORTS_END

// Video

UINT32 tandy2k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_vidouts)
	{
		m_vac->screen_update(screen, bitmap, cliprect);
	}

	return 0;
}
/*
static CRT9007_DRAW_SCANLINE( tandy2k_crt9007_display_pixels )
{
    tandy2k_state *state = device->machine().driver_data<tandy2k_state>();
    address_space &program = state->m_maincpu->space(AS_PROGRAM);

    for (int sx = 0; sx < x_count; sx++)
    {
        UINT32 videoram_addr = ((state->m_vram_base << 15) | (va << 1)) + sx;
        UINT8 videoram_data = program.read_word(videoram_addr);
        UINT16 charram_addr = (videoram_data << 4) | sl;
        UINT8 charram_data = state->m_char_ram[charram_addr] & 0xff;

        for (int bit = 0; bit < 10; bit++)
        {
            if (BIT(charram_data, 7))
            {
                bitmap.pix16(y, x + (sx * 10) + bit) = 1;
            }

            charram_data <<= 1;
        }
    }
}
*/

WRITE_LINE_MEMBER( tandy2k_state::vpac_vlt_w )
{
	m_drb0->clrcnt_w(state);
	m_drb1->clrcnt_w(state);
}

WRITE_LINE_MEMBER( tandy2k_state::vpac_drb_w )
{
	m_drb0->tog_w(state);
	m_drb1->tog_w(state);
}

static CRT9007_INTERFACE( vpac_intf )
{
	SCREEN_TAG,
	10,
	DEVCB_DEVICE_LINE(I8259A_1_TAG, pic8259_ir1_w),
	DEVCB_NULL,	// DMAR     80186 HOLD
	DEVCB_DEVICE_LINE_MEMBER(CRT9021B_TAG, crt9021_device, vsync_w), // VS
	DEVCB_NULL,	// HS
	DEVCB_DRIVER_LINE_MEMBER(tandy2k_state, vpac_vlt_w), // VLT
	DEVCB_DEVICE_LINE_MEMBER(CRT9021B_TAG, crt9021_device, cursor_w), // CURS
	DEVCB_DRIVER_LINE_MEMBER(tandy2k_state, vpac_drb_w), // DRB
	DEVCB_DEVICE_LINE_MEMBER(CRT9021B_TAG, crt9021_device, retbl_w), // CBLANK
	DEVCB_DEVICE_LINE_MEMBER(CRT9021B_TAG, crt9021_device, slg_w), // SLG
	DEVCB_DEVICE_LINE_MEMBER(CRT9021B_TAG, crt9021_device, sld_w) // SLD
};

static CRT9212_INTERFACE( drb0_intf )
{
	DEVCB_NULL, // ROF
	DEVCB_NULL, // WOF
	DEVCB_DEVICE_LINE_MEMBER(CRT9007_TAG, crt9007_device, vlt_r), // REN
	DEVCB_DEVICE_LINE_MEMBER(CRT9007_TAG, crt9007_device, wben_r), // WEN
	DEVCB_LINE_VCC // WEN2
};

static CRT9212_INTERFACE( drb1_intf )
{
	DEVCB_NULL, // ROF
	DEVCB_NULL, // WOF
	DEVCB_DEVICE_LINE_MEMBER(CRT9007_TAG, crt9007_device, vlt_r), // REN
	DEVCB_DEVICE_LINE_MEMBER(CRT9007_TAG, crt9007_device, wben_r), // WEN
	DEVCB_LINE_VCC // WEN2
};

static CRT9021_INTERFACE( vac_intf )
{
	SCREEN_TAG,
	DEVCB_DEVICE_MEMBER(CRT9212_0_TAG, crt9212_device, read), // data
	DEVCB_DEVICE_MEMBER(CRT9212_1_TAG, crt9212_device, read), // attributes
	DEVCB_LINE_VCC // ATTEN
};

// Intel 8251A Interface

WRITE_LINE_MEMBER( tandy2k_state::rxrdy_w )
{
	m_rxrdy = state;
	pic8259_ir2_w(m_pic0, m_rxrdy | m_txrdy);
}

WRITE_LINE_MEMBER( tandy2k_state::txrdy_w )
{
	m_txrdy = state;
	pic8259_ir2_w(m_pic0, m_rxrdy | m_txrdy);
}

static const i8251_interface usart_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(tandy2k_state, rxrdy_w),
	DEVCB_DRIVER_LINE_MEMBER(tandy2k_state, txrdy_w),
	DEVCB_NULL,
	DEVCB_NULL
};

// Intel 8253 Interface

WRITE_LINE_MEMBER( tandy2k_state::outspkr_w )
{
	m_outspkr = state;
	speaker_update();
}

WRITE_LINE_MEMBER( tandy2k_state::intbrclk_w )
{
	if (!m_extclk && state)
	{
		m_uart->transmit_clock();
		m_uart->receive_clock();
	}
}

WRITE_LINE_MEMBER( tandy2k_state::rfrqpulse_w )
{
}

static const struct pit8253_config pit_intf =
{
	{
		{
			XTAL_16MHz/16,
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(tandy2k_state, outspkr_w)
		}, {
			XTAL_16MHz/8,
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(tandy2k_state, intbrclk_w)
		}, {
			XTAL_16MHz/8,
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(tandy2k_state, rfrqpulse_w)
		}
	}
};

// Intel 8255A Interface

READ8_MEMBER( tandy2k_state::ppi_pb_r )
{
	/*

        bit     signal          description

        0       LPRIN0          auxiliary input 0
        1       LPRIN1          auxiliary input 1
        2       LPRIN2          auxiliary input 2
        3       _LPRACK         acknowledge
        4       _LPRFLT         fault
        5       _LPRSEL         select
        6       LPRPAEM         paper empty
        7       LPRBSY          busy

    */

	UINT8 data = 0;

	switch (m_pb_sel)
	{
	case LPINEN:
		// printer acknowledge
		data |= m_centronics->ack_r() << 3;

		// printer fault
		data |= m_centronics->fault_r() << 4;

		// printer select
		data |= m_centronics->vcc_r() << 5;

		// paper empty
		data |= m_centronics->pe_r() << 6;

		// printer busy
		data |= m_centronics->busy_r() << 7;
		break;

	case KBDINEN:
		// keyboard data
		data = m_kbdin;
		break;

	case PORTINEN:
		// PCB revision
		data = 0x03;
		break;
	}

	return data;
}

WRITE8_MEMBER( tandy2k_state::ppi_pc_w )
{
	/*

        bit     signal          description

        0                       port A direction
        1                       port B input select bit 0
        2                       port B input select bit 1
        3       LPRINT13        interrupt
        4       STROBE IN
        5       INBUFFULL
        6       _LPRACK
        7       _LPRDATSTB

    */

	// input select
	m_pb_sel = (data >> 1) & 0x03;

	// interrupt
	pic8259_ir3_w(m_pic1, BIT(data, 3));

	// printer strobe
	m_centronics->strobe_w(BIT(data, 7));
}

static I8255A_INTERFACE( ppi_intf )
{
	DEVCB_NULL,													// Port A read
	DEVCB_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, write),	// Port A write
	DEVCB_DRIVER_MEMBER(tandy2k_state, ppi_pb_r),				// Port B write
	DEVCB_NULL,													// Port B write
	DEVCB_NULL,													// Port C read
	DEVCB_DRIVER_MEMBER(tandy2k_state, ppi_pc_w)				// Port C write
};

// Intel 8259 Interfaces

/*

    IR0     MEMINT00
    IR1     TMOINT01
    IR2     SERINT02
    IR3     BUSINT03
    IR4     FLDINT04
    IR5     BUSINT05
    IR6     HDCINT06
    IR7     BUSINT07

*/

static const struct pic8259_interface pic0_intf =
{
	DEVCB_CPU_INPUT_LINE(I80186_TAG, INPUT_LINE_INT0),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};

/*

    IR0     KBDINT10
    IR1     VIDINT11
    IR2     RATINT12
    IR3     LPRINT13
    IR4     MCPINT14
    IR5     MEMINT15
    IR6     DMEINT16
    IR7     BUSINT17

*/

static const struct pic8259_interface pic1_intf =
{
	DEVCB_CPU_INPUT_LINE(I80186_TAG, INPUT_LINE_INT1),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};

// Intel 8272 Interface

void tandy2k_state::fdc_irq(bool state)
{
	pic8259_ir4_w(m_pic0, state);
}

void tandy2k_state::fdc_drq(bool state)
{
	dma_request(0, state);
}

static SLOT_INTERFACE_START( tandy2k_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

// Centronics Interface

static const centronics_interface centronics_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(I8255A_TAG, i8255_device, pc6_w),	// ACK output
	DEVCB_NULL,										// BUSY output
	DEVCB_NULL										// NOT BUSY output
};

// Keyboard

WRITE_LINE_MEMBER( tandy2k_state::kbdclk_w )
{
	if (!m_kbdclk && state)
	{
		m_kbdin >>= 1;
		m_kbdin |= m_kb->data_r() << 7;
	}

	m_kbdclk = state;
}

WRITE_LINE_MEMBER( tandy2k_state::kbddat_w )
{
	if (!m_kbddat && state)
	{
		m_kb->busy_w(m_kbdclk);
		pic8259_ir0_w(m_pic1, !m_kbdclk);
	}

	m_kbddat = state;
}

static TANDY2K_KEYBOARD_INTERFACE( kb_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(tandy2k_state, kbdclk_w),
	DEVCB_DRIVER_LINE_MEMBER(tandy2k_state, kbddat_w)
};

// Machine Initialization

void tandy2k_state::machine_start()
{
	// memory banking
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();
	int ram_size = m_ram->size();

	program.install_ram(0x00000, ram_size - 1, ram);

	m_fdc->setup_intrq_cb(i8272a_device::line_cb(FUNC(tandy2k_state::fdc_irq), this));
	m_fdc->setup_drq_cb(i8272a_device::line_cb(FUNC(tandy2k_state::fdc_drq), this));

	// patch out i186 relocation register check
	UINT8 *rom = memregion(I80186_TAG)->base();
	rom[0x1f16] = 0x90;
	rom[0x1f17] = 0x90;

	// register for state saving
	save_item(NAME(m_dma_mux));
	save_item(NAME(m_kbdclk));
	save_item(NAME(m_kbddat));
	save_item(NAME(m_kbdin));
	save_item(NAME(m_extclk));
	save_item(NAME(m_rxrdy));
	save_item(NAME(m_txrdy));
	save_item(NAME(m_pb_sel));
	save_item(NAME(m_vidouts));
	save_item(NAME(m_clkspd));
	save_item(NAME(m_clkcnt));
	save_item(NAME(m_outspkr));
	save_item(NAME(m_spkrdata));
}

// Machine Driver

static MACHINE_CONFIG_START( tandy2k, tandy2k_state )
    // basic machine hardware
	MCFG_CPU_ADD(I80186_TAG, I80186, XTAL_16MHz)
    MCFG_CPU_PROGRAM_MAP(tandy2k_mem)
    MCFG_CPU_IO_MAP(tandy2k_io)

    // video hardware
    MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
    MCFG_SCREEN_REFRESH_RATE(50)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_UPDATE_DRIVER(tandy2k_state, screen_update)
    MCFG_SCREEN_SIZE(640, 480)
    MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MCFG_CRT9007_ADD(CRT9007_TAG, XTAL_16MHz*28/16, vpac_intf, vpac_mem)
	MCFG_CRT9212_ADD(CRT9212_0_TAG, drb0_intf)
	MCFG_CRT9212_ADD(CRT9212_1_TAG, drb1_intf)
	MCFG_CRT9021_ADD(CRT9021B_TAG, XTAL_16MHz*28/16/8, vac_intf)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_I8255A_ADD(I8255A_TAG, ppi_intf)
	MCFG_I8251_ADD(I8251A_TAG, usart_intf)
	MCFG_PIT8253_ADD(I8253_TAG, pit_intf)
	MCFG_PIC8259_ADD(I8259A_0_TAG, pic0_intf)
	MCFG_PIC8259_ADD(I8259A_1_TAG, pic1_intf)
	MCFG_I8272A_ADD(I8272A_TAG, true)
	MCFG_FLOPPY_DRIVE_ADD(I8272A_TAG ":0", tandy2k_floppies, "525qd", 0, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(I8272A_TAG ":1", tandy2k_floppies, "525qd", 0, floppy_image_device::default_floppy_formats)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, standard_centronics)
	MCFG_TANDY2K_KEYBOARD_ADD(kb_intf)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "tandy2k")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("256K,384K,512K,640K,768K,896K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tandy2k_hd, tandy2k )
    // basic machine hardware
	MCFG_CPU_MODIFY(I80186_TAG)
    MCFG_CPU_IO_MAP(tandy2k_hd_io)

	// Tandon TM502 hard disk
	MCFG_HARDDISK_ADD("harddisk0")
	//MCFG_WD1010_ADD(WD1010_TAG, wd1010_intf)
	//MCFG_WD1100_11_ADD(WD1100_11_TAG, wd1100_11_intf)
MACHINE_CONFIG_END

// ROMs

ROM_START( tandy2k )
	ROM_REGION( 0x2000, I80186_TAG, 0 )
	ROM_LOAD16_BYTE( "484a00.u48", 0x0000, 0x1000, CRC(a5ee3e90) SHA1(4b1f404a4337c67065dd272d62ff88dcdee5e34b) )
	ROM_LOAD16_BYTE( "474600.u47", 0x0001, 0x1000, CRC(345701c5) SHA1(a775cbfa110b7a88f32834aaa2a9b868cbeed25b) )

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "82s153.u62", 0x000, 0x100, NO_DUMP ) // interrupt/DMA
	ROM_LOAD( "82s153.u68", 0x000, 0x100, NO_DUMP ) // video
	ROM_LOAD( "82s153.u95", 0x000, 0x100, NO_DUMP ) // memory timing
	ROM_LOAD( "pal10l8.u82", 0x000, 0x100, NO_DUMP ) // video
	ROM_LOAD( "pal16l8a.u102", 0x000, 0x100, NO_DUMP ) // bus interface
	ROM_LOAD( "pal16l8a.u103", 0x000, 0x100, NO_DUMP ) // bus interface
	ROM_LOAD( "pal20l8.u103", 0x000, 0x100, NO_DUMP ) // bus interface, alternate
	ROM_LOAD( "pal16r6a.u16", 0x000, 0x100, NO_DUMP ) // HDC
ROM_END

#define rom_tandy2khd rom_tandy2k

// System Drivers

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY                 FULLNAME        FLAGS
COMP( 1983, tandy2k,	0,			0,		tandy2k,	tandy2k, driver_device,	0,		"Tandy Radio Shack",	"Tandy 2000",	GAME_NOT_WORKING)
COMP( 1983, tandy2khd,	tandy2k,	0,		tandy2k_hd,	tandy2k, driver_device,	0,		"Tandy Radio Shack",	"Tandy 2000HD",	GAME_NOT_WORKING)
