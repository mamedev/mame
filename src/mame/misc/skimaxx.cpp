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
      chopped off. This is most noticeable for distant thin objects (flags, poles of the start banner).
      See maincpu routine at 387B8, specifically the jump at 3880E throws 4 horizontal pixels away.

    - The player often disappears (when she's too slow?). Is this normal ?

    - Are button 2 and analog input "Y" used ?

*****************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68030.h"
#include "cpu/tms34010/tms34010.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

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
		m_fg_buffer(*this, "fg_buffer"),
		m_blitter_gfx(*this, "blitter"),
		m_bgrambank(*this, "bgrambank"),
		m_an_io(*this, {"X", "Y"})
	{ }

	void skimaxx(machine_config &config);

protected:
	virtual void device_post_load() override;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<tms34010_device> m_tms;

	required_shared_ptr<u32> m_blitter_regs;
	required_shared_ptr<u32> m_fpga_ctrl;
	required_shared_ptr<u16> m_fg_buffer;
	required_region_ptr<u16> m_blitter_gfx;

	required_memory_bank m_bgrambank;
	required_ioport_array<2> m_an_io;

	std::unique_ptr<u32[]> m_bg_buffer;
	u32 *m_bg_buffer_front;
	u32 *m_bg_buffer_back;
	u32 m_blitter_src_x;
	u32 m_blitter_src_dx;
	u32 m_blitter_src_y;
	u32 m_blitter_src_dy;

	void blitter_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 blitter_r(offs_t offset, u32 mem_mask = ~0);
	void fpga_ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 unk_r();
	u32 unk1_r();
	void unk1_w(u32 data);
	void sub_ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 analog_r(offs_t offset);
	void tms_irq(int state);

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	void m68030_1_map(address_map &map) ATTR_COLD;
	void m68030_2_map(address_map &map) ATTR_COLD;
	void tms_program_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

// Set up blit parameters
void skimaxx_state::blitter_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const newdata = COMBINE_DATA( &m_blitter_regs[offset] );

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
u32 skimaxx_state::blitter_r(offs_t offset, u32 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		u32 const penaddr = ((m_blitter_src_x >> 8) & 0x1ff) + ((m_blitter_src_y >> 8) << 9);
		const u16 *src = m_blitter_gfx + (penaddr % m_blitter_gfx.length());
		u32 *dst = m_bg_buffer_back + offset;

		u16 const pen = (*src) & 0x7fff;

		if (pen)
		{
			if (ACCESSING_BITS_16_31)
				*dst = (*dst & 0x0000ffff) | (pen << 16);
			else
				*dst = (*dst & 0xffff0000) | pen;
		}

		m_blitter_src_x = (m_blitter_src_x & 0x10000) | ((m_blitter_src_x + m_blitter_src_dx) & 0xffff);
		m_blitter_src_y = (m_blitter_src_y & 0xffff0000) | ((m_blitter_src_y + m_blitter_src_dy) & 0xffff);
	}

	return 0;
}


void skimaxx_state::video_start()
{
	m_bg_buffer = std::make_unique<u32[]>(0x400 * 0x100 * sizeof(u16) / sizeof(u32) * 2); // 2 buffers
	m_bg_buffer_back  = m_bg_buffer.get() + 0x400 * 0x100 * sizeof(u16) / sizeof(u32) * 0;
	m_bg_buffer_front = m_bg_buffer.get() + 0x400 * 0x100 * sizeof(u16) / sizeof(u32) * 1;
	m_bgrambank->configure_entry(0, m_bg_buffer_back);
	m_bgrambank->configure_entry(1, m_bg_buffer_front);

	save_item(NAME(m_blitter_src_x));
	save_item(NAME(m_blitter_src_y));
	save_item(NAME(m_blitter_src_dx));
	save_item(NAME(m_blitter_src_dy));
}

void skimaxx_state::device_post_load()
{
	// double buffering
	u8 const bank_bg_buffer = (*m_fpga_ctrl & 0x40) ? 1 : 0;

	m_bg_buffer_back  = m_bg_buffer.get() + 0x400 * 0x100 * sizeof(u16) / sizeof(u32) * bank_bg_buffer;
	m_bg_buffer_front = m_bg_buffer.get() + 0x400 * 0x100 * sizeof(u16) / sizeof(u32) * (1 - bank_bg_buffer);

	m_bgrambank->set_entry(bank_bg_buffer);
}

/*************************************
 *
 *  VRAM shift register callbacks
 *
 *************************************/

// TODO: Might not be used
TMS340X0_TO_SHIFTREG_CB_MEMBER(skimaxx_state::to_shiftreg)
{
	memcpy(shiftreg, &m_fg_buffer[address >> 4], 512 * sizeof(u16));
}

TMS340X0_FROM_SHIFTREG_CB_MEMBER(skimaxx_state::from_shiftreg)
{
	memcpy(&m_fg_buffer[address >> 4], shiftreg, 512 * sizeof(u16));
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
		u32 const rowaddr = (params->rowaddr - 0x220);
		u16 const *fg = &m_fg_buffer[rowaddr << 8];
		u32 const *bg = &m_bg_buffer_front[rowaddr/2 * 1024/2];
		u16 *dest = &bitmap.pix(scanline);
		//int coladdr = params->coladdr;
		//coladdr = 0;

		dest += params->heblnk;
		for (int x = params->heblnk; x < params->hsblnk; x+=2)
		{
			u16 const tmspix = *fg & 0x7fff;
			if (tmspix)
			{
				*dest++ = tmspix;
				*dest++ = tmspix;
			}
			else
			{
				u32 const data = *bg & 0x7fff7fff;
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

void skimaxx_state::fpga_ctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const newdata = COMBINE_DATA( m_fpga_ctrl );

	if (ACCESSING_BITS_0_7)
	{
		// double buffering
		u8 const bank_bg_buffer = (newdata & 0x40) ? 1 : 0;

		m_bg_buffer_back  = m_bg_buffer.get() + 0x400 * 0x100 * sizeof(u16) / sizeof(u32) * bank_bg_buffer;
		m_bg_buffer_front = m_bg_buffer.get() + 0x400 * 0x100 * sizeof(u16) / sizeof(u32) * (1 - bank_bg_buffer);

		m_bgrambank->set_entry(bank_bg_buffer);
	}
}

// 0x2000004c: bit 7, bit 0
u32 skimaxx_state::unk_r()
{
	return (*m_fpga_ctrl & 0x20) ? 0x80 : 0x00;
}

// 0x20000023
u32 skimaxx_state::unk1_r()
{
	return 0x80;
}

void skimaxx_state::unk1_w(u32 data)
{
}

void skimaxx_state::sub_ctrl_w(offs_t offset, u32 data, u32 mem_mask)
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
u32 skimaxx_state::analog_r(offs_t offset)
{
	return bitswap<8>(m_an_io[offset]->read(), 0,1,2,3,4,5,6,7);
}

/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

void skimaxx_state::m68030_1_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom();
	map(0x10000000, 0x10000003).w(FUNC(skimaxx_state::sub_ctrl_w));
	map(0x10100000, 0x1010000f).rw(m_tms, FUNC(tms34010_device::host_r), FUNC(tms34010_device::host_w)).umask32(0x0000ffff);
//  map(0x10180000, 0x10187fff).ram().share("share1");
	map(0x10180000, 0x1018ffff).ram().share("share1");  // above 10188000 accessed at level end (game bug?)
	map(0x20000000, 0x20000003).nopr(); // watchdog_r?

	map(0x20000013, 0x20000013).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // left
	map(0x20000017, 0x20000017).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // left
	map(0x2000001b, 0x2000001b).rw("oki3", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // right
	map(0x2000001f, 0x2000001f).rw("oki4", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // right

	map(0x20000020, 0x20000023).r(FUNC(skimaxx_state::unk1_r));   // units linking?
	map(0x20000024, 0x20000027).w(FUNC(skimaxx_state::unk1_w));  // ""

	map(0x20000040, 0x20000043).ram(); // write
	map(0x20000044, 0x20000047).portr("DSW");
	map(0x20000048, 0x2000004b).portr("COIN");
	map(0x2000004c, 0x2000004f).r(FUNC(skimaxx_state::unk_r)); // bit 7, bit 0

	map(0x20000050, 0x20000057).r(FUNC(skimaxx_state::analog_r)).nopw(); // read (0-1f), write motor?

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

	map(0x20000000, 0x2007ffff).r(FUNC(skimaxx_state::blitter_r));    // do blit
	map(0x30000000, 0x3000000f).w(FUNC(skimaxx_state::blitter_w)).share(m_blitter_regs);

	map(0x40000000, 0x40000003).w(FUNC(skimaxx_state::fpga_ctrl_w)).share(m_fpga_ctrl);

	map(0x50000000, 0x5007ffff).bankrw(m_bgrambank);    // background ram allocated here at video_start (skimaxx_bg_buffer_back/front)
//  map(0xfffc0000, 0xfffc7fff).ram().share("share1");
	map(0xfffc0000, 0xfffcffff).ram().share("share1");
//  map(0xfffe0000, 0xffffffff).ram(); // I think this is banked with the shared RAM? (see CPU sync routines)
	map(0xfffe0000, 0xfffeffff).ram().share("share1");  // HACK
	//  map(0xfffe0010, 0xfffeffff).ram();             // HACK
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
	map(0x00220000, 0x003fffff).ram().share(m_fg_buffer);
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

static const u32 texlayout_xoffset[512] =
{
	STEP512(0,16)
};

static const u32 texlayout_yoffset[128] =
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

void skimaxx_state::tms_irq(int state)
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

void skimaxx_state::skimaxx(machine_config &config)
{
	M68EC030(config, m_maincpu, XTAL(40'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &skimaxx_state::m68030_1_map);
	m_maincpu->set_vblank_int("screen", FUNC(skimaxx_state::irq3_line_hold));    // 1,3,7 are identical, rest is RTE

	M68EC030(config, m_subcpu, XTAL(40'000'000));
	m_subcpu->set_addrmap(AS_PROGRAM, &skimaxx_state::m68030_2_map);

	// video hardware
	TMS34010(config, m_tms, XTAL(50'000'000));
	m_tms->set_addrmap(AS_PROGRAM, &skimaxx_state::tms_program_map);
	m_tms->set_halt_on_reset(false);
	m_tms->set_pixel_clock(50000000/8);
	m_tms->set_pixels_per_clock(2);
	m_tms->set_scanline_ind16_callback(FUNC(skimaxx_state::scanline_update));
	m_tms->output_int().set(FUNC(skimaxx_state::tms_irq));
	m_tms->set_shiftreg_in_callback(FUNC(skimaxx_state::to_shiftreg));
	m_tms->set_shiftreg_out_callback(FUNC(skimaxx_state::from_shiftreg));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_raw(40000000/4, 156*4, 0, 100*4, 328, 0, 300); // TODO - Wrong but TMS overrides it anyway
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(0x400, 0x100);
	screen.set_visarea(0, 0x280-1, 0, 0xf0-1);
	screen.set_screen_update("tms", FUNC(tms34010_device::tms340x0_ind16));
	screen.set_palette("palette");

//  GFXDECODE(config, "gfxdecode", "palette", gfx_skimaxx);

	PALETTE(config, "palette", palette_device::RGB_555);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM6295(config, "oki1", XTAL(4'000'000), okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "lspeaker", 1.0);     // ?

	OKIM6295(config, "oki2", XTAL(4'000'000)/2, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "lspeaker", 1.0);  // ?

	OKIM6295(config, "oki3", XTAL(4'000'000), okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "rspeaker", 1.0);     // ?

	OKIM6295(config, "oki4", XTAL(4'000'000)/2, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "rspeaker", 1.0);  // ?
}


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

	ROM_REGION16_LE( 0x100000, "tmsgfx", 0 )
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

} // anonymous namespace


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1996, skimaxx, 0, skimaxx, skimaxx, skimaxx_state, empty_init, ROT0, "Kyle Hodgetts / ICE", "Skimaxx", MACHINE_IMPERFECT_GRAPHICS )
