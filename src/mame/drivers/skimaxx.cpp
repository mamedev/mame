// license:BSD-3-Clause
// copyright-holders:Philip Bennett, Luca Elia
/*****************************************************************************************************

    Skimaxx

    driver by Phil Bennett and Luca Elia

    Top Board:

        68EC030-40
        (MSM6295 + Crystal) * 4

    Bottom Board:

        68EC030-40
        TMS34010-50

    Video hardware:

    - Blitter that supports rotation and zoom (background and sprites)
    - TMS34010 (text, score)

    Notes:

    - It looks like a bistream for a Xilinx FPGA is stored in ROM.
      The sub CPU bit-bangs 30864 bits worth of data to 0x40000003 twice.

    - There seems to be a bug in the game code drawing routines, causing objects to have some pixels
      chopped off. This is most noticeable for distant thin objects (falgs, poles of the start banner).
      See maincpu routine at 387B8, specifically the jump at 3880E throws 4 horizontal pixels away.

    - The player often disappears (when she's too slow?). Is this normal ?

    - Are button 2 and analog input "Y" used ?

*****************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "sound/okim6295.h"


class skimaxx_state : public driver_device
{
public:
	skimaxx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_tms(*this, "tms"),
		m_blitter_regs(*this, "blitter_regs"),
		m_fpga_ctrl(*this, "fpga_ctrl"),
		m_fg_buffer(*this, "fg_buffer") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<tms34010_device> m_tms;

	required_shared_ptr<UINT32> m_blitter_regs;
	required_shared_ptr<UINT32> m_fpga_ctrl;
	required_shared_ptr<UINT16> m_fg_buffer;

	UINT32 *m_bg_buffer;
	UINT32 *m_bg_buffer_front;
	UINT32 *m_bg_buffer_back;
	UINT16 *m_blitter_gfx;
	UINT32 m_blitter_gfx_len;
	UINT32 m_blitter_src_x;
	UINT32 m_blitter_src_dx;
	UINT32 m_blitter_src_y;
	UINT32 m_blitter_src_dy;

	DECLARE_WRITE32_MEMBER(skimaxx_blitter_w);
	DECLARE_READ32_MEMBER(skimaxx_blitter_r);
	DECLARE_WRITE32_MEMBER(skimaxx_fpga_ctrl_w);
	DECLARE_READ32_MEMBER(unk_r);
	DECLARE_READ32_MEMBER(skimaxx_unk1_r);
	DECLARE_WRITE32_MEMBER(skimaxx_unk1_w);
	DECLARE_WRITE32_MEMBER(skimaxx_sub_ctrl_w);
	DECLARE_READ32_MEMBER(skimaxx_analog_r);
	DECLARE_WRITE_LINE_MEMBER(tms_irq);

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	virtual void machine_reset() override;
	virtual void video_start() override;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

// Set up blit parameters
WRITE32_MEMBER(skimaxx_state::skimaxx_blitter_w)
{
	UINT32 newdata = COMBINE_DATA( &m_blitter_regs[offset] );

	switch (offset)
	{
		case 0:
			m_blitter_src_dy = (newdata & 0x7fff) - (newdata & 0x8000);
			break;

		case 1:
			m_blitter_src_x = (newdata & 0x1ff) << 8;
			m_blitter_src_y = (newdata >> 9) << 8;
			break;

		case 2:
			if (ACCESSING_BITS_16_31)
				m_blitter_src_dx = (newdata >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_blitter_src_dx = newdata & 0xffff;

			m_blitter_src_dx = (m_blitter_src_dx & 0x7fff) - (m_blitter_src_dx & 0x8000);
			break;

		case 3:
			// Usually 0. Set to 10 for water/sand near the horizon
			break;
	}
}

// A read by the 68030 from this area blits one pixel to the back buffer (at the same offset)
READ32_MEMBER(skimaxx_state::skimaxx_blitter_r)
{
	UINT32 penaddr = ((m_blitter_src_x >> 8) & 0x1ff) + ((m_blitter_src_y >> 8) << 9);
	UINT16 *src = m_blitter_gfx + (penaddr % m_blitter_gfx_len);
	UINT32 *dst = m_bg_buffer_back + offset;

	UINT16 pen = (*src) & 0x7fff;

	if (pen)
	{
		if (ACCESSING_BITS_16_31)
			*dst = (*dst & 0x0000ffff) | (pen << 16);
		else
			*dst = (*dst & 0xffff0000) | pen;
	}

	m_blitter_src_x = (m_blitter_src_x & 0x10000) | ((m_blitter_src_x + m_blitter_src_dx) & 0xffff);
	m_blitter_src_y = (m_blitter_src_y & 0xffff0000) | ((m_blitter_src_y + m_blitter_src_dy) & 0xffff);

	return 0;
}


void skimaxx_state::video_start()
{
	m_blitter_gfx = (UINT16 *) memregion( "blitter" )->base();
	m_blitter_gfx_len = memregion( "blitter" )->bytes() / 2;

	m_bg_buffer = auto_alloc_array(machine(), UINT32, 0x400 * 0x100 * sizeof(UINT16) / sizeof(UINT32) * 2); // 2 buffers
	m_bg_buffer_back  = m_bg_buffer + 0x400 * 0x100 * sizeof(UINT16) / sizeof(UINT32) * 0;
	m_bg_buffer_front = m_bg_buffer + 0x400 * 0x100 * sizeof(UINT16) / sizeof(UINT32) * 1;
	membank("bank1")->configure_entry(0, m_bg_buffer_back);
	membank("bank1")->configure_entry(1, m_bg_buffer_front);
}

/*************************************
 *
 *  VRAM shift register callbacks
 *
 *************************************/

// TODO: Might not be used
TMS340X0_TO_SHIFTREG_CB_MEMBER(skimaxx_state::to_shiftreg)
{
	memcpy(shiftreg, &m_fg_buffer[TOWORD(address)], 512 * sizeof(UINT16));
}

TMS340X0_FROM_SHIFTREG_CB_MEMBER(skimaxx_state::from_shiftreg)
{
	memcpy(&m_fg_buffer[TOWORD(address)], shiftreg, 512 * sizeof(UINT16));
}


/*************************************
 *
 *  Main video refresh
 *
 *************************************/

TMS340X0_SCANLINE_IND16_CB_MEMBER(skimaxx_state::scanline_update)
{
	// TODO: This isn't correct. I just hacked it together quickly so I could see something!

	if (params->rowaddr >= 0x220)
	{
		UINT32 rowaddr = (params->rowaddr - 0x220);
		UINT16 *fg = &m_fg_buffer[rowaddr << 8];
		UINT32 *bg = &m_bg_buffer_front[rowaddr/2 * 1024/2];
		UINT16 *dest = &bitmap.pix16(scanline);
		//int coladdr = params->coladdr;
		int x;
		//coladdr = 0;

		dest += params->heblnk;
		for (x = params->heblnk; x < params->hsblnk; x+=2)
		{
			UINT16 tmspix;
			tmspix = *fg & 0x7fff;
			if (tmspix)
			{
				*dest++ = tmspix;
				*dest++ = tmspix;
			}
			else
			{
				UINT32 data = *bg & 0x7fff7fff;
				*dest++ = data >> 16;
				*dest++ = data;
			}
			fg++;
			bg++;
		}
	}
}



/*************************************
 *
 *  Handlers
 *
 *************************************/

/*
  bit 7: 0->1 at the start, and before bit banging to 40000003 (Xilinx FPGA configuration write port?)
  bit 6: double buffering
  bit 5: 0/1 less oftenly
  bit 4: 0->1 clocks bit banging
  bit 3-1: 1?
  bit 0: bit banging data
*/

WRITE32_MEMBER(skimaxx_state::skimaxx_fpga_ctrl_w)
{
	UINT32 newdata = COMBINE_DATA( m_fpga_ctrl );

	if (ACCESSING_BITS_0_7)
	{
		// double buffering
		UINT8 bank_bg_buffer = (newdata & 0x40) ? 1 : 0;

		m_bg_buffer_back  = m_bg_buffer + 0x400 * 0x100 * sizeof(UINT16) / sizeof(UINT32) * bank_bg_buffer;
		m_bg_buffer_front = m_bg_buffer + 0x400 * 0x100 * sizeof(UINT16) / sizeof(UINT32) * (1 - bank_bg_buffer);

		membank("bank1")->set_entry(bank_bg_buffer);
	}
}

// 0x2000004c: bit 7, bit 0
READ32_MEMBER(skimaxx_state::unk_r)
{
	return (*m_fpga_ctrl & 0x20) ? 0x80 : 0x00;
}

// 0x20000023
READ32_MEMBER(skimaxx_state::skimaxx_unk1_r)
{
	return 0x80;
}

WRITE32_MEMBER(skimaxx_state::skimaxx_unk1_w)
{
}

WRITE32_MEMBER(skimaxx_state::skimaxx_sub_ctrl_w)
{
	// 7e/7f at the start. 3f/7f, related to reads from 1018xxxx
	if (ACCESSING_BITS_0_7)
	{
		m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
		m_subcpu->set_input_line(INPUT_LINE_HALT,  (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
	}
}

/*
    00      left max again

    01      right max
    ..
    10      center
    ..
    1f      left max
*/
READ32_MEMBER(skimaxx_state::skimaxx_analog_r)
{
	return BITSWAP8(ioport(offset ? "Y" : "X")->read(), 0,1,2,3,4,5,6,7);
}

/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

static ADDRESS_MAP_START( 68030_1_map, AS_PROGRAM, 32, skimaxx_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM
	AM_RANGE(0x10000000, 0x10000003) AM_WRITE(skimaxx_sub_ctrl_w )
	AM_RANGE(0x10100000, 0x1010000f) AM_DEVREADWRITE16("tms", tms34010_device, host_r, host_w, 0x0000ffff)
//  AM_RANGE(0x10180000, 0x10187fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x10180000, 0x1018ffff) AM_RAM AM_SHARE("share1")  // above 10188000 accessed at level end (game bug?)
	AM_RANGE(0x20000000, 0x20000003) AM_READNOP // watchdog_r?

	AM_RANGE(0x20000010, 0x20000013) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff) // left
	AM_RANGE(0x20000014, 0x20000017) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff) // left
	AM_RANGE(0x20000018, 0x2000001b) AM_DEVREADWRITE8("oki3", okim6295_device, read, write, 0x00ff) // right
	AM_RANGE(0x2000001c, 0x2000001f) AM_DEVREADWRITE8("oki4", okim6295_device, read, write, 0x00ff) // right

	AM_RANGE(0x20000020, 0x20000023) AM_READ(skimaxx_unk1_r )   // units linking?
	AM_RANGE(0x20000024, 0x20000027) AM_WRITE(skimaxx_unk1_w )  // ""

	AM_RANGE(0x20000040, 0x20000043) AM_RAM // write
	AM_RANGE(0x20000044, 0x20000047) AM_READ_PORT( "DSW" )
	AM_RANGE(0x20000048, 0x2000004b) AM_READ_PORT( "COIN" )
	AM_RANGE(0x2000004c, 0x2000004f) AM_READ(unk_r) // bit 7, bit 0

	AM_RANGE(0x20000050, 0x20000057) AM_READ(skimaxx_analog_r ) AM_WRITENOP // read (0-1f), write motor?

	AM_RANGE(0xfffc0000, 0xffffffff) AM_RAM AM_MIRROR(0x00020000)
ADDRESS_MAP_END


/*************************************
 *
 *  Sub CPU memory map (Blitter control)
 *
 *************************************/

static ADDRESS_MAP_START( 68030_2_map, AS_PROGRAM, 32, skimaxx_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_ROM

	AM_RANGE(0x20000000, 0x2007ffff) AM_READ(skimaxx_blitter_r )    // do blit
	AM_RANGE(0x30000000, 0x3000000f) AM_WRITE(skimaxx_blitter_w ) AM_SHARE("blitter_regs")

	AM_RANGE(0x40000000, 0x40000003) AM_WRITE(skimaxx_fpga_ctrl_w ) AM_SHARE("fpga_ctrl")

	AM_RANGE(0x50000000, 0x5007ffff) AM_RAMBANK("bank1")    // background ram allocated here at video_start (skimaxx_bg_buffer_back/front)
//  AM_RANGE(0xfffc0000, 0xfffc7fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xfffc0000, 0xfffcffff) AM_RAM AM_SHARE("share1")
//  AM_RANGE(0xfffe0000, 0xffffffff) AM_RAM // I think this is banked with the shared RAM? (see CPU sync routines)
	AM_RANGE(0xfffe0010, 0xfffeffff) AM_RAM             // HACK
	AM_RANGE(0xfffe0000, 0xfffeffff) AM_RAM AM_SHARE("share1")  // HACK
	AM_RANGE(0xffff0000, 0xffffffff) AM_RAM
ADDRESS_MAP_END


/*************************************
 *
 *  Video CPU memory map (Text layer)
 *
 *************************************/

static ADDRESS_MAP_START( tms_program_map, AS_PROGRAM, 16, skimaxx_state )
	AM_RANGE(0x00000000, 0x000100ff) AM_RAM
	AM_RANGE(0x00008000, 0x0003ffff) AM_RAM
	AM_RANGE(0x00050000, 0x0005ffff) AM_RAM
	AM_RANGE(0x00220000, 0x003fffff) AM_RAM AM_SHARE("fg_buffer")
	AM_RANGE(0x02000000, 0x0200000f) AM_RAM
	AM_RANGE(0x02100000, 0x0210000f) AM_RAM
	AM_RANGE(0x04000000, 0x047fffff) AM_ROM AM_REGION("tmsgfx", 0)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("tms", tms34010_device, io_register_r, io_register_w)
	AM_RANGE(0xff800000, 0xffffffff) AM_ROM AM_REGION("tms", 0)
ADDRESS_MAP_END


/*************************************
 *
 *  Graphics decoding (for debug)
 *
 *************************************/

#if 0

static const UINT32 texlayout_xoffset[512] =
{
	STEP512(0,16)
};

static const UINT32 texlayout_yoffset[128] =
{
	STEP128(0,512*16)
};

static const gfx_layout texlayout =
{
	512, 128,
	RGN_FRAC(1,1),
	5,
	{ 9,10,11,12,13 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	512*128*16,
	texlayout_xoffset,
	texlayout_yoffset
};

static GFXDECODE_START( skimaxx )
	GFXDECODE_ENTRY( "blitter", 0, texlayout, 0, 1 )
GFXDECODE_END

#endif


/*************************************
 *
 *  Input definitions
 *
 *************************************/

static INPUT_PORTS_START( skimaxx )
	PORT_START("DSW")   // 20000044.l & 0xff
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x05, "12" )
	PORT_DIPSETTING(    0x04, "17" )
	PORT_DIPSETTING(    0x03, "25" )
	PORT_DIPSETTING(    0x02, "33" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x07, "100" )
	PORT_DIPSETTING(    0x06, "200" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, "0" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "8" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "12" )
	PORT_DIPSETTING(    0x50, "15" )
	PORT_DIPSETTING(    0x60, "20" )
	PORT_DIPSETTING(    0x70, "30" )
	PORT_DIPNAME( 0x80, 0x80, "Time For Super Course" )
	PORT_DIPSETTING(    0x80, "50 s" )  // 3000
	PORT_DIPSETTING(    0x00, "60 s" )  // 3600

	PORT_START("COIN")  // 20000048.l & 0xff
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )    // ?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1   )    // coin
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2   )    // coin
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1  )    // start
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )    // change course / letter

	PORT_START("X")
	PORT_BIT( 0x1f, 0x10, IPT_AD_STICK_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_MINMAX(0x00,0x1e) PORT_INVERT

	PORT_START("Y")
	PORT_BIT( 0x1f, 0x10, IPT_AD_STICK_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_MINMAX(0x00,0x1e) PORT_INVERT
//  PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END


/*************************************
 *
 *  TMS34010 configuration
 *
 *************************************/

WRITE_LINE_MEMBER(skimaxx_state::tms_irq)
{
	// TODO
}

/*************************************
 *
 *  Initialisation
 *
 *************************************/

void skimaxx_state::machine_reset()
{
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( skimaxx, skimaxx_state )
	MCFG_CPU_ADD("maincpu", M68EC030, XTAL_40MHz)
	MCFG_CPU_PROGRAM_MAP(68030_1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", skimaxx_state,  irq3_line_hold)    // 1,3,7 are identical, rest is RTE

	MCFG_CPU_ADD("subcpu", M68EC030, XTAL_40MHz)
	MCFG_CPU_PROGRAM_MAP(68030_2_map)


	/* video hardware */
	MCFG_CPU_ADD("tms", TMS34010, XTAL_50MHz)
	MCFG_CPU_PROGRAM_MAP(tms_program_map)
	MCFG_TMS340X0_HALT_ON_RESET(FALSE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(50000000/8) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(2) /* pixels per clock */
	MCFG_TMS340X0_SCANLINE_IND16_CB(skimaxx_state, scanline_update)     /* scanline updater (indexed16) */
	MCFG_TMS340X0_OUTPUT_INT_CB(WRITELINE(skimaxx_state, tms_irq))
	MCFG_TMS340X0_TO_SHIFTREG_CB(skimaxx_state, to_shiftreg)  /* write to shiftreg function */
	MCFG_TMS340X0_FROM_SHIFTREG_CB(skimaxx_state, from_shiftreg) /* read from shiftreg function */

	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_RAW_PARAMS(40000000/4, 156*4, 0, 100*4, 328, 0, 300) // TODO - Wrong but TMS overrides it anyway
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(0x400, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x280-1, 0, 0xf0-1)
	MCFG_SCREEN_UPDATE_DEVICE("tms", tms34010_device, tms340x0_ind16)
	MCFG_SCREEN_PALETTE("palette")

//  MCFG_GFXDECODE_ADD("gfxdecode", "palette", skimaxx )

	MCFG_PALETTE_ADD_RRRRRGGGGGBBBBB("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki1", XTAL_4MHz, OKIM6295_PIN7_LOW)     // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki2", XTAL_4MHz/2, OKIM6295_PIN7_HIGH)  // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki3", XTAL_4MHz, OKIM6295_PIN7_LOW)     // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki4", XTAL_4MHz/2, OKIM6295_PIN7_HIGH)  // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( skimaxx )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "main2_0.ve2", 0x000000, 0x80000, CRC(0e139138) SHA1(b28b0f5dba7810712cee99a9a9748ca2dabddf15) )
	ROM_LOAD32_BYTE( "main2_0.ve4", 0x000001, 0x80000, CRC(4e0502cf) SHA1(451efd82656dd32e696f572d848b2717453f05c8) )
	ROM_LOAD32_BYTE( "main2_0.ve3", 0x000002, 0x80000, CRC(53887cfd) SHA1(79d9547e0e9c4c41913279f977bf25cdbe800356) )
	ROM_LOAD32_BYTE( "main2_0.ve5", 0x000003, 0x80000, CRC(6317f54d) SHA1(057141acd2a8a804d8cc0487d5cacc286c344866) )

	ROM_REGION( 0x400000, "subcpu", 0 )
	ROM_LOAD32_BYTE( "ve1v-2_0", 0x000003, 0x100000, CRC(871e16d2) SHA1(9318ebcff4032fffcbe53c8fb805b14578eaebac) )
	ROM_LOAD32_BYTE( "ve2v-2_0", 0x000002, 0x100000, CRC(17b01a96) SHA1(79ef3bd6a22abf774095741c29deeb6af53d1585) )
	ROM_LOAD32_BYTE( "ve3v-2_0", 0x000001, 0x100000, CRC(c1491795) SHA1(3a527ea4bf8cfdab072153b1cd66b0d090ef4d31) )
	ROM_LOAD32_BYTE( "ve4v-2_0", 0x000000, 0x100000, CRC(950773bb) SHA1(68e4a5780701488a934104eb87d10c36d736f049) )

	ROM_REGION16_LE( 0x100000, "tms", 0 )
	ROM_LOAD16_BYTE( "vc8v-2_0", 0x000000, 0x80000, CRC(a6e9ef81) SHA1(59a0fb149e17d3773adb980428a0b107647bd4fa) )
	ROM_LOAD16_BYTE( "vc9v-2_0", 0x000001, 0x80000, CRC(b1e8ba65) SHA1(b91cf93ecd9b6067664780ab2c1b69f632c7ae05) )

	ROM_REGION( 0x100000, "tmsgfx", 0 )
	ROM_LOAD16_BYTE( "vc10v2_0", 0x000000, 0x80000, CRC(433651cd) SHA1(9b9801703d16adbbca2b03e1714490fb166d48a0) )
	ROM_LOAD16_BYTE( "vc11v2_0", 0x000001, 0x80000, CRC(a906fc72) SHA1(c61ad560f203c7f507cc7b7dc8834f529a6501a7) )

	ROM_REGION16_LE( 0x600000, "blitter", 0 )
	ROM_LOAD16_BYTE( "ve5v-2_0", 0x000000, 0x100000, CRC(b95544a3) SHA1(1ad49da54d36543229e241887c3dd8411ef5c89a) )
	ROM_LOAD16_BYTE( "ve6v-2_0", 0x000001, 0x100000, CRC(d98bf89a) SHA1(f7fd66487f7696b53ee90a4cf45a810ec791df59) )
	ROM_LOAD16_BYTE( "ve10v2_0", 0x200000, 0x100000, CRC(c2a16a62) SHA1(3663c19a6517b0a01fb454523b995be3cdf2e4b3) )
	ROM_LOAD16_BYTE( "ve9v-2_0", 0x200001, 0x100000, CRC(587b364d) SHA1(b53412bdcc804727990d959e5b2399a7e5e7fbf3) )
	ROM_LOAD16_BYTE( "ve7v-2_0", 0x400000, 0x100000, CRC(a927716b) SHA1(0189d7b08b65b6a8a8d4c3e2affe93612db38155) )
	ROM_LOAD16_BYTE( "ve8v-2_0", 0x400001, 0x100000, CRC(ad8ed4a4) SHA1(5d1339b3d6ce59fea062273fa2da35988ba94a80) )

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "main2_0.u3", 0x000000, 0x80000, CRC(24d8c6ad) SHA1(06f51a4c380c91c930d646826246f62c4e1f9cda) )

	ROM_REGION( 0x80000, "oki2", 0 )
	ROM_LOAD( "main2_0.u2", 0x000000, 0x80000, CRC(c84b3c46) SHA1(b956358518495aa822a5b699cbad1abac212dd09) )   // identical to main2_0.u4

	ROM_REGION( 0x80000, "oki3", 0 )
	ROM_LOAD( "main2_0.u5", 0x000000, 0x80000, CRC(e2ba07ad) SHA1(cf82753975f7b6756cca4e10b5372e00135440bf) )

	ROM_REGION( 0x80000, "oki4", 0 )
	ROM_LOAD( "main2_0.u4", 0x000000, 0x80000, CRC(c84b3c46) SHA1(b956358518495aa822a5b699cbad1abac212dd09) )   // identical to main2_0.u2
ROM_END


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1996, skimaxx, 0, skimaxx, skimaxx, driver_device, 0, ROT0, "Kyle Hodgetts / ICE", "Skimaxx", MACHINE_IMPERFECT_GRAPHICS )
