// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Tandy 2000

****************************************************************************/

/*

    TODO:

    - floppy
        - HDL is also connected to WP/TS input where TS is used to detect motor status
        - 3 second motor off delay timer
    - video (video RAM is at memory top - 0x1400, i.e. 0x1ec00)
    - keyboard ROM
    - hires graphics board
    - WD1010
    - hard disk
    - mouse

*/

#include "includes/tandy2k.h"
#include "machine/pckeybrd.h"

#define LOG 1

// Read/Write Handlers

void tandy2k_state::update_drq()
{
	int drq0 = CLEAR_LINE;
	int drq1 = CLEAR_LINE;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_dma_mux, 0 + i))
		{
			if (BIT(m_dma_mux, 4 + i))
				drq1 |= m_busdmarq[i];
			else
				drq0 |= m_busdmarq[i];
		}
	}

	m_maincpu->drq0_w(drq0);
	m_maincpu->drq1_w(drq1);
}

void tandy2k_state::dma_request(int line, int state)
{
	m_busdmarq[line] = state;

	update_drq();
}

void tandy2k_state::speaker_update()
{
	int level = !(m_spkrdata & m_outspkr);

	m_speaker->level_w(level);
}

READ8_MEMBER( tandy2k_state::char_ram_r )
{
	return m_char_ram[offset];
}

WRITE8_MEMBER( tandy2k_state::char_ram_w )
{
	m_char_ram[offset] = data;
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

	UINT8 data = 0x80;

	data |= m_rs232->ri_r();
	data |= m_rs232->dcd_r() << 1;

	return data;
}

WRITE8_MEMBER( tandy2k_state::enable_w )
{
	/*

	    bit     signal      description

	    0       KBEN        keyboard enable
	    1       EXTCLK      external baud rate clock
	    2       SPKRGATE    enable periodic speaker output
	    3       SPKRDATA    direct output to speaker
	    4       RFRQGATE    enable refresh and baud rate clocks
	    5       FDCRESET*   reset 8272
	    6       TMRIN0      enable 80186 timer 0
	    7       TMRIN1      enable 80186 timer 1

	*/

	if (LOG) logerror("ENABLE %02x\n", data);

	// keyboard enable
	m_kb->power_w(BIT(data, 0));
	machine().device<pc_keyboard_device>("pc_keyboard")->enable(BIT(data, 0));

	// external baud rate clock
	m_extclk = BIT(data, 1);

	// speaker gate
	m_pit->write_gate0(BIT(data, 2));

	// speaker data
	m_spkrdata = BIT(data, 3);
	speaker_update();

	// refresh and baud rate clocks
	m_pit->write_gate1(BIT(data, 4));
	m_pit->write_gate2(BIT(data, 4));

	// FDC reset
	if (!BIT(data, 5))
	{
		m_fdc->soft_reset();
	}

	// timer 0 enable
	m_maincpu->tmrin0_w(BIT(data, 6));

	// timer 1 enable
	m_maincpu->tmrin1_w(BIT(data, 7));
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

	if (LOG) logerror("DMA MUX %02x\n", data);

	m_dma_mux = data;

	// check for DMA error
	int drq0 = 0;
	int drq1 = 0;

	for (int ch = 0; ch < 4; ch++)
	{
		if (BIT(data, ch)) { if (BIT(data, ch + 4)) drq1++; else drq0++; }
	}

	int dme = (drq0 > 2) || (drq1 > 2);

	m_pic1->ir6_w(dme);

	update_drq();
}

READ8_MEMBER( tandy2k_state::kbint_clr_r )
{
	if (m_pb_sel == KBDINEN)
	{
		m_kb->busy_w(1);
		m_pic1->ir0_w(CLEAR_LINE);

		return machine().device<pc_keyboard_device>("pc_keyboard")->read(space, 0);
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
	if (LOG) logerror("FLDTC\n");

	fldtc_w(space, 0, 0);

	return 0;
}

WRITE8_MEMBER( tandy2k_state::fldtc_w )
{
	m_fdc->tc_w(1);
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
	    13      CLKSP0      clock speed (0 = 22.4 MHz, 1 = 28 MHz)
	    14      CLKCNT      dots/char (0 = 10 [800x400], 1 = 8 [640x400])
	    15      VIDOUTS     selects the video source for display on monochrome monitor

	*/

	if (LOG) logerror("Address Control %02x\n", data);

	// video access
	m_vram_base = data & 0x1f;

	// video clock speed
	int clkspd = BIT(data, 5);
	int clkcnt = BIT(data, 6);

	if (m_clkspd != clkspd || m_clkcnt != clkcnt)
	{
		float busdotclk = XTAL_16MHz*28 / (clkspd ? 16 : 20);
		float vidcclk = busdotclk / (clkcnt ? 8 : 10);

		m_vpac->set_character_width(clkcnt ? 8 : 10);
		m_vpac->set_unscaled_clock(vidcclk);

		m_vac->set_unscaled_clock(busdotclk);

		m_timer_vidldsh->adjust(attotime::from_hz(vidcclk), 0, attotime::from_hz(vidcclk));

		m_clkspd = clkspd;
		m_clkcnt = clkcnt;
	}

	// video source select
	m_vidouts = BIT(data, 7);
}

// Memory Maps

static ADDRESS_MAP_START( tandy2k_mem, AS_PROGRAM, 16, tandy2k_state )
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(0x00000, 0xdffff) AM_RAM
	AM_RANGE(0xe0000, 0xf7fff) AM_RAM AM_SHARE("hires_ram")
	AM_RANGE(0xf8000, 0xfbfff) AM_READWRITE8(char_ram_r, char_ram_w, 0x00ff)
	AM_RANGE(0xfc000, 0xfdfff) AM_MIRROR(0x2000) AM_ROM AM_REGION(I80186_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tandy2k_io, AS_IO, 16, tandy2k_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x00001) AM_MIRROR(0x8) AM_READWRITE8(enable_r, enable_w, 0x00ff)
	AM_RANGE(0x00002, 0x00003) AM_MIRROR(0x8) AM_WRITE8(dma_mux_w, 0x00ff)
	AM_RANGE(0x00004, 0x00005) AM_MIRROR(0x8) AM_READWRITE8(fldtc_r, fldtc_w, 0x00ff)
	AM_RANGE(0x00010, 0x00013) AM_MIRROR(0xc) AM_DEVREADWRITE8(I8251A_TAG, i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0x00030, 0x00033) AM_MIRROR(0xc) AM_DEVICE8(I8272A_TAG, i8272a_device, map, 0x00ff)
	AM_RANGE(0x00040, 0x00047) AM_MIRROR(0x8) AM_DEVREADWRITE8(I8253_TAG, pit8253_device, read, write, 0x00ff)
	AM_RANGE(0x00052, 0x00053) AM_MIRROR(0x8) AM_READ8(kbint_clr_r, 0x00ff)
	AM_RANGE(0x00050, 0x00057) AM_MIRROR(0x8) AM_DEVREADWRITE8(I8255A_TAG, i8255_device, read, write, 0x00ff)
	AM_RANGE(0x00060, 0x00063) AM_MIRROR(0xc) AM_DEVREADWRITE8(I8259A_0_TAG, pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x00070, 0x00073) AM_MIRROR(0xc) AM_DEVREADWRITE8(I8259A_1_TAG, pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x00080, 0x00081) AM_MIRROR(0xe) AM_DEVREADWRITE8(I8272A_TAG, i8272a_device, mdma_r, mdma_w, 0x00ff)
//  AM_RANGE(0x00100, 0x0017f) AM_DEVREADWRITE8(CRT9007_TAG, crt9007_t, read, write, 0x00ff) AM_WRITE8(addr_ctrl_w, 0xff00)
	AM_RANGE(0x00100, 0x0017f) AM_READWRITE(vpac_r, vpac_w)
//  AM_RANGE(0x00180, 0x00180) AM_READ8(hires_status_r, 0x00ff)
//  AM_RANGE(0x00180, 0x001bf) AM_WRITE(hires_palette_w)
//  AM_RANGE(0x001a0, 0x001a0) AM_READ8(hires_plane_w, 0x00ff)
//  AM_RANGE(0x0ff00, 0x0ffff) AM_READWRITE(i186_internal_port_r, i186_internal_port_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tandy2k_hd_io, AS_IO, 16, tandy2k_state )
	AM_IMPORT_FROM(tandy2k_io)
//  AM_RANGE(0x000e0, 0x000ff) AM_WRITE8(hdc_dack_w, 0x00ff)
//  AM_RANGE(0x0026c, 0x0026d) AM_DEVREADWRITE8(WD1010_TAG, wd1010_device, hdc_reset_r, hdc_reset_w, 0x00ff)
//  AM_RANGE(0x0026e, 0x0027f) AM_DEVREADWRITE8(WD1010_TAG, wd1010_device, wd1010_r, wd1010_w, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vpac_mem, AS_0, 8, tandy2k_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(videoram_r)
ADDRESS_MAP_END

// Input Ports

static INPUT_PORTS_START( tandy2k )
	// defined in machine/tandy2kb.c
	PORT_INCLUDE(pc_keyboard)
INPUT_PORTS_END

// Video

UINT32 tandy2k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();
	address_space &program = m_maincpu->space(AS_PROGRAM);

	for (int y = 0; y < 400; y++)
	{
		UINT8 cgra = y % 16;

		for (int sx = 0; sx < 80; sx++)
		{
			offs_t addr = m_ram->size() - 0x1400 + (((y / 16) * 80) + sx) * 2;
			UINT8 vidla = program.read_word(addr);
			UINT8 data = m_char_ram[(vidla << 4) | cgra];

			for (int x = 0; x < 8; x++)
			{
				int color = BIT(data, 7);
				bitmap.pix32(y, (sx * 8) + x) = pen[color];
				data <<= 1;
			}
		}
	}

	return 0;
}

WRITE_LINE_MEMBER( tandy2k_state::vpac_vlt_w )
{
	m_drb0->ren_w(state);
	m_drb0->clrcnt_w(state);

	m_drb1->ren_w(state);
	m_drb1->clrcnt_w(state);
}

WRITE_LINE_MEMBER( tandy2k_state::vpac_drb_w )
{
	m_drb0->tog_w(state);
	m_drb1->tog_w(state);
}

WRITE_LINE_MEMBER( tandy2k_state::vpac_wben_w )
{
	m_drb0->wen1_w(state);
	m_drb1->wen1_w(state);
}

WRITE_LINE_MEMBER( tandy2k_state::vpac_cblank_w )
{
	m_cblank = state;
}

WRITE_LINE_MEMBER( tandy2k_state::vpac_slg_w )
{
	m_slg = state;

	m_vac->slg_w(state);
}

WRITE_LINE_MEMBER( tandy2k_state::vpac_sld_w )
{
	m_sld = state;

	m_vac->sld_w(state);
}

WRITE8_MEMBER( tandy2k_state::vidla_w )
{
	m_vidla = data;
}

WRITE8_MEMBER( tandy2k_state::drb_attr_w )
{
	/*

	    bit     description

	    0       BLC -> DBLC (delayed 2 CCLKs)
	    1       BKC -> DBKC (delayed 2 CCLKs)
	    2       CHABL
	    3       MS0
	    4       MS1
	    5       BLINK
	    6       INT
	    7       REVID

	*/

	m_blc = BIT(data, 0);
	m_bkc = BIT(data, 1);
	m_vac->chabl_w(BIT(data, 2));
	m_vac->ms0_w(BIT(data, 3));
	m_vac->ms1_w(BIT(data, 4));
	m_vac->blink_w(BIT(data, 5));
	m_vac->intin_w(BIT(data, 6));
	m_vac->revid_w(BIT(data, 7));
}

CRT9021_DRAW_CHARACTER_MEMBER( tandy2k_state::vac_draw_character )
{
	const pen_t *pen = m_palette->pens();

	for (int i = 0; i < 8; i++)
	{
		int color = BIT(video, 7 - i);

		bitmap.pix32(y, x++) = pen[color];
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( tandy2k_state::vidldsh_tick )
{
	m_drb0->rclk_w(0);
	m_drb0->wclk_w(0);
	m_drb1->rclk_w(0);
	m_drb1->wclk_w(0);
	m_vac->ld_sh_w(0);

	// 1 busdotclk later
	m_vac->blc_w(BIT(m_dblc, 0));
	m_dblc >>= 1;
	m_dblc |= m_blc << 2;

	m_vac->bkc_w(BIT(m_dbkc, 0));
	m_dbkc >>= 1;
	m_dbkc |= m_bkc << 2;

	m_vac->retbl_w(BIT(m_dblank, 0));
	m_dblank >>= 1;
	m_dblank |= m_cblank << 2;

	if (!m_slg)
	{
		m_cgra >>= 1;
		m_cgra |= m_sld << 3;
	}

	UINT8 vidd = m_char_ram[(m_vidla << 4) | m_cgra];
	m_vac->write(vidd);

	m_drb0->rclk_w(1);
	m_drb0->wclk_w(1);
	m_drb1->rclk_w(1);
	m_drb1->wclk_w(1);
	m_vac->ld_sh_w(1);
}

// Intel 8251A Interface

WRITE_LINE_MEMBER( tandy2k_state::rxrdy_w )
{
	m_rxrdy = state;
	m_pic0->ir2_w(m_rxrdy || m_txrdy);
}

WRITE_LINE_MEMBER( tandy2k_state::txrdy_w )
{
	m_txrdy = state;
	m_pic0->ir2_w(m_rxrdy || m_txrdy);
}

// Intel 8253 Interface

WRITE_LINE_MEMBER( tandy2k_state::outspkr_w )
{
	m_outspkr = state;
	speaker_update();
}

WRITE_LINE_MEMBER( tandy2k_state::intbrclk_w )
{
	if (!m_extclk)
	{
		m_uart->write_txc(state);
		m_uart->write_rxc(state);
	}
}

WRITE_LINE_MEMBER( tandy2k_state::rfrqpulse_w )
{
	// memory refresh counter up
}

// Intel 8255A Interface

WRITE_LINE_MEMBER( tandy2k_state::write_centronics_ack )
{
	m_centronics_ack = state;
	m_i8255a->pc6_w(state);
}

WRITE_LINE_MEMBER( tandy2k_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( tandy2k_state::write_centronics_perror )
{
	m_centronics_perror = state;
}

WRITE_LINE_MEMBER( tandy2k_state::write_centronics_select )
{
	m_centronics_select = state;
}

WRITE_LINE_MEMBER( tandy2k_state::write_centronics_fault )
{
	m_centronics_fault = state;
}

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
		data |= m_centronics_ack << 3;

		// printer fault
		data |= m_centronics_fault << 4;

		// printer select
		data |= m_centronics_select << 5;

		// paper empty
		data |= m_centronics_perror << 6;

		// printer busy
		data |= m_centronics_busy << 7;
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
	m_pic1->ir3_w(BIT(data, 3));

	// printer strobe
	m_centronics->write_strobe(BIT(data, 7));
}


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

// Intel 8272 Interface

WRITE_LINE_MEMBER( tandy2k_state::fdc_drq_w )
{
	dma_request(0, state);
}

WRITE_LINE_MEMBER( tandy2k_state::fdc_hdl_w )
{
	m_floppy0->mon_w(!state);
	m_floppy1->mon_w(!state);
}

FLOPPY_FORMATS_MEMBER( tandy2k_state::floppy_formats )
	FLOPPY_TANDY_2000_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( tandy2k_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

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
		m_pic1->ir0_w(!m_kbdclk);
	}

	m_kbddat = state;
}

READ8_MEMBER( tandy2k_state::irq_callback )
{
	return (offset ? m_pic1 : m_pic0)->acknowledge();
}

// Machine Initialization

void tandy2k_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();
	int ram_size = m_ram->size();

	program.install_ram(0x00000, ram_size - 1, ram);

	m_char_ram.allocate(0x1000);

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

void tandy2k_state::device_reset_after_children()
{
	machine().device<pc_keyboard_device>("pc_keyboard")->enable(0);
}

// Machine Driver

static MACHINE_CONFIG_START( tandy2k, tandy2k_state )
	// basic machine hardware
	MCFG_CPU_ADD(I80186_TAG, I80186, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(tandy2k_mem)
	MCFG_CPU_IO_MAP(tandy2k_io)
	MCFG_80186_IRQ_SLAVE_ACK(DEVREAD8(DEVICE_SELF, tandy2k_state, irq_callback))

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	//MCFG_SCREEN_UPDATE_DEVICE(CRT9021B_TAG, crt9021_t, screen_update)
	MCFG_SCREEN_UPDATE_DRIVER(tandy2k_state, screen_update)

	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_DEVICE_ADD(CRT9007_TAG, CRT9007, XTAL_16MHz*28/20/8)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, vpac_mem)
	MCFG_CRT9007_CHARACTER_WIDTH(8)
	MCFG_CRT9007_INT_CALLBACK(DEVWRITELINE(I8259A_1_TAG, pic8259_device, ir1_w))
	MCFG_CRT9007_VS_CALLBACK(DEVWRITELINE(CRT9021B_TAG, crt9021_t, vsync_w))
	MCFG_CRT9007_VLT_CALLBACK(WRITELINE(tandy2k_state, vpac_vlt_w))
	MCFG_CRT9007_CURS_CALLBACK(DEVWRITELINE(CRT9021B_TAG, crt9021_t, cursor_w))
	MCFG_CRT9007_DRB_CALLBACK(WRITELINE(tandy2k_state, vpac_drb_w))
	MCFG_CRT9007_WBEN_CALLBACK(WRITELINE(tandy2k_state, vpac_wben_w))
	MCFG_CRT9007_CBLANK_CALLBACK(WRITELINE(tandy2k_state, vpac_cblank_w))
	MCFG_CRT9007_SLG_CALLBACK(WRITELINE(tandy2k_state, vpac_slg_w))
	MCFG_CRT9007_SLD_CALLBACK(WRITELINE(tandy2k_state, vpac_sld_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)

	MCFG_DEVICE_ADD(CRT9212_0_TAG, CRT9212, 0)
	MCFG_CRT9212_WEN2_VCC()
	MCFG_CRT9212_DOUT_CALLBACK(WRITE8(tandy2k_state, vidla_w))

	MCFG_DEVICE_ADD(CRT9212_1_TAG, CRT9212, 0)
	MCFG_CRT9212_WEN2_VCC()
	MCFG_CRT9212_DOUT_CALLBACK(WRITE8(tandy2k_state, drb_attr_w))

	MCFG_DEVICE_ADD(CRT9021B_TAG, CRT9021, XTAL_16MHz*28/20)
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)

	MCFG_TIMER_DRIVER_ADD("vidldsh", tandy2k_state, vidldsh_tick)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_DEVICE_ADD(I8255A_TAG, I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_I8255_IN_PORTB_CB(READ8(tandy2k_state, ppi_pb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(tandy2k_state, ppi_pc_w))

	MCFG_DEVICE_ADD(I8251A_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(tandy2k_state, rxrdy_w))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE(tandy2k_state, txrdy_w))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251A_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251A_TAG, i8251_device, write_dsr))
	// TODO pin 15 external transmit clock
	// TODO pin 17 external receiver clock

	MCFG_DEVICE_ADD(I8253_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_16MHz/16)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(tandy2k_state, outspkr_w))
	MCFG_PIT8253_CLK1(XTAL_16MHz/8)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(tandy2k_state, intbrclk_w))
	//MCFG_PIT8253_CLK2(XTAL_16MHz/8)
	//MCFG_PIT8253_OUT2_HANDLER(WRITELINE(tandy2k_state, rfrqpulse_w))

	MCFG_PIC8259_ADD(I8259A_0_TAG, DEVWRITELINE(I80186_TAG, i80186_cpu_device, int0_w), VCC, NULL)

	MCFG_PIC8259_ADD(I8259A_1_TAG, DEVWRITELINE(I80186_TAG, i80186_cpu_device, int1_w), VCC, NULL)

	MCFG_I8272A_ADD(I8272A_TAG, true)
	downcast<i8272a_device *>(device)->set_select_lines_connected(true);
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE(I8259A_0_TAG, pic8259_device, ir4_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(tandy2k_state, fdc_drq_w))
	MCFG_UPD765_HDL_CALLBACK(WRITELINE(tandy2k_state, fdc_hdl_w))
	MCFG_FLOPPY_DRIVE_ADD(I8272A_TAG ":0", tandy2k_floppies, "525qd", tandy2k_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(I8272A_TAG ":1", tandy2k_floppies, "525qd", tandy2k_state::floppy_formats)

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(tandy2k_state, write_centronics_ack))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(tandy2k_state, write_centronics_busy))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(tandy2k_state, write_centronics_perror))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(tandy2k_state, write_centronics_select))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(tandy2k_state, write_centronics_fault))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_DEVICE_ADD(TANDY2K_KEYBOARD_TAG, TANDY2K_KEYBOARD, 0)
	MCFG_TANDY2000_KEYBOARD_CLOCK_CALLBACK(WRITELINE(tandy2k_state, kbdclk_w))
	MCFG_TANDY2000_KEYBOARD_DATA_CALLBACK(WRITELINE(tandy2k_state, kbddat_w))

	// temporary until the tandy keyboard has a rom dump
	MCFG_PC_KEYB_ADD("pc_keyboard", DEVWRITELINE(I8259A_1_TAG, pic8259_device, ir0_w))


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
COMP( 1983, tandy2k,    0,          0,      tandy2k,    tandy2k, driver_device, 0,      "Tandy Radio Shack",    "Tandy 2000",   MACHINE_NOT_WORKING )
COMP( 1983, tandy2khd,  tandy2k,    0,      tandy2k_hd, tandy2k, driver_device, 0,      "Tandy Radio Shack",    "Tandy 2000HD", MACHINE_NOT_WORKING )
