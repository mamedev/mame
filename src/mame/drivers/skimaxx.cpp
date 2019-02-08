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
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class skimaxx_state : public driver_device
{
public:
	skimaxx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_tms(*this, "tms"),
		m_blitter_regs(*this, "blitter_regs"),
		m_fpga_ctrl(*this, "fpga_ctrl"),
		m_fg_buffer(*this, "fg_buffer")
	{ }

	void skimaxx(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<tms34010_device> m_tms;

	required_shared_ptr<uint32_t> m_blitter_regs;
	required_shared_ptr<uint32_t> m_fpga_ctrl;
	required_shared_ptr<uint16_t> m_fg_buffer;

	std::unique_ptr<uint32_t[]> m_bg_buffer;
	uint32_t *m_bg_buffer_front;
	uint32_t *m_bg_buffer_back;
	uint16_t *m_blitter_gfx;
	uint32_t m_blitter_gfx_len;
	uint32_t m_blitter_src_x;
	uint32_t m_blitter_src_dx;
	uint32_t m_blitter_src_y;
	uint32_t m_blitter_src_dy;

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
	void m68030_1_map(address_map &map);
	void m68030_2_map(address_map &map);
	void tms_program_map(address_map &map);
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

// Set up blit parameters
WRITE32_MEMBER(skimaxx_state::skimaxx_blitter_w)
{
	uint32_t newdata = COMBINE_DATA( &m_blitter_regs[offset] );

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
	uint32_t penaddr = ((m_blitter_src_x >> 8) & 0x1ff) + ((m_blitter_src_y >> 8) << 9);
	uint16_t *src = m_blitter_gfx + (penaddr % m_blitter_gfx_len);
	uint32_t *dst = m_bg_buffer_back + offset;

	uint16_t pen = (*src) & 0x7fff;

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
	m_blitter_gfx = (uint16_t *) memregion( "blitter" )->base();
	m_blitter_gfx_len = memregion( "blitter" )->bytes() / 2;

	m_bg_buffer = std::make_unique<uint32_t[]>(0x400 * 0x100 * sizeof(uint16_t) / sizeof(uint32_t) * 2); // 2 buffers
	m_bg_buffer_back  = m_bg_buffer.get() + 0x400 * 0x100 * sizeof(uint16_t) / sizeof(uint32_t) * 0;
	m_bg_buffer_front = m_bg_buffer.get() + 0x400 * 0x100 * sizeof(uint16_t) / sizeof(uint32_t) * 1;
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
	memcpy(shiftreg, &m_fg_buffer[address >> 4], 512 * sizeof(uint16_t));
}

TMS340X0_FROM_SHIFTREG_CB_MEMBER(skimaxx_state::from_shiftreg)
{
	memcpy(&m_fg_buffer[address >> 4], shiftreg, 512 * sizeof(uint16_t));
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
		uint32_t rowaddr = (params->rowaddr - 0x220);
		uint16_t *fg = &m_fg_buffer[rowaddr << 8];
		uint32_t *bg = &m_bg_buffer_front[rowaddr/2 * 1024/2];
		uint16_t *dest = &bitmap.pix16(scanline);
		//int coladdr = params->coladdr;
		int x;
		//coladdr = 0;

		dest += params->heblnk;
		for (x = params->heblnk; x < params->hsblnk; x+=2)
		{
			uint16_t tmspix;
			tmspix = *fg & 0x7fff;
			if (tmspix)
			{
				*dest++ = tmspix;
				*dest++ = tmspix;
			}
			else
			{
				uint32_t data = *bg & 0x7fff7fff;
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
	uint32_t newdata = COMBINE_DATA( m_fpga_ctrl );

	if (ACCESSING_BITS_0_7)
	{
		// double buffering
		uint8_t bank_bg_buffer = (newdata & 0x40) ? 1 : 0;

		m_bg_buffer_back  = m_bg_buffer.get() + 0x400 * 0x100 * sizeof(uint16_t) / sizeof(uint32_t) * bank_bg_buffer;
		m_bg_buffer_front = m_bg_buffer.get() + 0x400 * 0x100 * sizeof(uint16_t) / sizeof(uint32_t) * (1 - bank_bg_buffer);

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
	return bitswap<8>(ioport(offset ? "Y" : "X")->read(), 0,1,2,3,4,5,6,7);
}

/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

void skimaxx_state::m68030_1_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom();
	map(0x10000000, 0x10000003).w(FUNC(skimaxx_state::skimaxx_sub_ctrl_w));
	map(0x10100000, 0x1010000f).rw(m_tms, FUNC(tms34010_device::host_r), FUNC(tms34010_device::host_w)).umask32(0x0000ffff);
//  AM_RANGE(0x10180000, 0x10187fff) AM_RAM AM_SHARE("share1")
	map(0x10180000, 0x1018ffff).ram().share("share1");  // above 10188000 accessed at level end (game bug?)
	map(0x20000000, 0x20000003).nopr(); // watchdog_r?

	map(0x20000013, 0x20000013).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // left
	map(0x20000017, 0x20000017).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // left
	map(0x2000001b, 0x2000001b).rw("oki3", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // right
	map(0x2000001f, 0x2000001f).rw("oki4", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // right

	map(0x20000020, 0x20000023).r(FUNC(skimaxx_state::skimaxx_unk1_r));   // units linking?
	map(0x20000024, 0x20000027).w(FUNC(skimaxx_state::skimaxx_unk1_w));  // ""

	map(0x20000040, 0x20000043).ram(); // write
	map(0x20000044, 0x20000047).portr("DSW");
	map(0x20000048, 0x2000004b).portr("COIN");
	map(0x2000004c, 0x2000004f).r(FUNC(skimaxx_state::unk_r)); // bit 7, bit 0

	map(0x20000050, 0x20000057).r(FUNC(skimaxx_state::skimaxx_analog_r)).nopw(); // read (0-1f), write motor?

	map(0xfffc0000, 0xfffdffff).ram().mirror(0x00020000);
}


/*************************************
 *
 *  Sub CPU memory map (Blitter control)
 *
 *************************************/

void skimaxx_state::m68030_2_map(address_map &map)
{
	map(0x00000000, 0x003fffff).rom();

	map(0x20000000, 0x2007ffff).r(FUNC(skimaxx_state::skimaxx_blitter_r));    // do blit
	map(0x30000000, 0x3000000f).w(FUNC(skimaxx_state::skimaxx_blitter_w)).share("blitter_regs");

	map(0x40000000, 0x40000003).w(FUNC(skimaxx_state::skimaxx_fpga_ctrl_w)).share("fpga_ctrl");

	map(0x50000000, 0x5007ffff).bankrw("bank1");    // background ram allocated here at video_start (skimaxx_bg_buffer_back/front)
//  AM_RANGE(0xfffc0000, 0xfffc7fff) AM_RAM AM_SHARE("share1")
	map(0xfffc0000, 0xfffcffff).ram().share("share1");
//  AM_RANGE(0xfffe0000, 0xffffffff) AM_RAM // I think this is banked with the shared RAM? (see CPU sync routines)
	map(0xfffe0000, 0xfffeffff).ram().share("share1");  // HACK
	map(0xfffe0010, 0xfffeffff).ram();             // HACK
	map(0xffff0000, 0xffffffff).ram();
}


/*************************************
 *
 *  Video CPU memory map (Text layer)
 *
 *************************************/

void skimaxx_state::tms_program_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).ram();
	map(0x00050000, 0x0005ffff).ram();
	map(0x00220000, 0x003fffff).ram().share("fg_buffer");
	map(0x02000000, 0x0200000f).ram();
	map(0x02100000, 0x0210000f).ram();
	map(0x04000000, 0x047fffff).rom().region("tmsgfx", 0);
	map(0xff800000, 0xffffffff).rom().region("tms", 0);
}


/*************************************
 *
 *  Graphics decoding (for debug)
 *
 *************************************/

#if 0

static const uint32_t texlayout_xoffset[512] =
{
	STEP512(0,16)
};

static const uint32_t texlayout_yoffset[128] =
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

static GFXDECODE_START( gfx_skimaxx )
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

MACHINE_CONFIG_START(skimaxx_state::skimaxx)
	MCFG_DEVICE_ADD("maincpu", M68EC030, XTAL(40'000'000))
	MCFG_DEVICE_PROGRAM_MAP(m68030_1_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", skimaxx_state,  irq3_line_hold)    // 1,3,7 are identical, rest is RTE

	MCFG_DEVICE_ADD("subcpu", M68EC030, XTAL(40'000'000))
	MCFG_DEVICE_PROGRAM_MAP(m68030_2_map)


	/* video hardware */
	TMS34010(config, m_tms, XTAL(50'000'000));
	m_tms->set_addrmap(AS_PROGRAM, &skimaxx_state::tms_program_map);
	m_tms->set_halt_on_reset(false);
	m_tms->set_pixel_clock(50000000/8);
	m_tms->set_pixels_per_clock(2);
	m_tms->set_scanline_ind16_callback(FUNC(skimaxx_state::scanline_update));
	m_tms->output_int().set(FUNC(skimaxx_state::tms_irq));
	m_tms->set_shiftreg_in_callback(FUNC(skimaxx_state::to_shiftreg));
	m_tms->set_shiftreg_out_callback(FUNC(skimaxx_state::from_shiftreg));

	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_RAW_PARAMS(40000000/4, 156*4, 0, 100*4, 328, 0, 300) // TODO - Wrong but TMS overrides it anyway
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(0x400, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x280-1, 0, 0xf0-1)
	MCFG_SCREEN_UPDATE_DEVICE("tms", tms34010_device, tms340x0_ind16)
	MCFG_SCREEN_PALETTE("palette")

//  GFXDECODE(config, "gfxdecode", "palette", gfx_skimaxx);

	PALETTE(config, "palette", palette_device::RGB_555);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("oki1", OKIM6295, XTAL(4'000'000), okim6295_device::PIN7_LOW)     // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_DEVICE_ADD("oki2", OKIM6295, XTAL(4'000'000)/2, okim6295_device::PIN7_HIGH)  // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_DEVICE_ADD("oki3", OKIM6295, XTAL(4'000'000), okim6295_device::PIN7_LOW)     // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("oki4", OKIM6295, XTAL(4'000'000)/2, okim6295_device::PIN7_HIGH)  // ?
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

GAME( 1996, skimaxx, 0, skimaxx, skimaxx, skimaxx_state, empty_init, ROT0, "Kyle Hodgetts / ICE", "Skimaxx", MACHINE_IMPERFECT_GRAPHICS )
