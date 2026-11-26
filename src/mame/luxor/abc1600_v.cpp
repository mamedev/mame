// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Mover emulation

**********************************************************************/

#include "emu.h"
#include "abc1600.lh"
#include "abc1600_v.h"
#include "render.h"

#include <algorithm>
#include <bit>

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define SCREEN_TAG "screen"


// video RAM
#define VIDEORAM_SIZE       0x40000
#define VIDEORAM16_MASK     0x3ffff
#define VIDEORAM8_MASK      0x7fffe


// flag register
#define L_P         BIT(m_flag, 0)
#define BLANK       BIT(m_flag, 1)
#define PIX_POL     BIT(m_flag, 2)
#define FRAME_POL   BIT(m_flag, 3)
#define HOLD_FY     BIT(m_flag, 4)
#define HOLD_FX     BIT(m_flag, 5)
#define COMP_MOVE   BIT(m_flag, 6)
#define REPLACE     BIT(m_flag, 7)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC1600_MOVER, abc1600_mover_device, "abc1600mover", "ABC 1600 Mover")


void abc1600_mover_device::vram_map(address_map &map)
{
	map(0x00000, 0x7ffff).rw(FUNC(abc1600_mover_device::video_ram_r), FUNC(abc1600_mover_device::video_ram_w));
}

void abc1600_mover_device::crtc_map(address_map &map)
{
	map(0x00, 0x00).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x01, 0x01).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

void abc1600_mover_device::iowr0_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(abc1600_mover_device::iord0_r));
	map(0x00, 0x00).w(FUNC(abc1600_mover_device::ldsx_hb_w));
	map(0x01, 0x01).w(FUNC(abc1600_mover_device::ldsx_lb_w));
	map(0x02, 0x02).w(FUNC(abc1600_mover_device::ldsy_hb_w));
	map(0x03, 0x03).w(FUNC(abc1600_mover_device::ldsy_lb_w));
	map(0x04, 0x04).w(FUNC(abc1600_mover_device::ldtx_hb_w));
	map(0x05, 0x05).w(FUNC(abc1600_mover_device::ldtx_lb_w));
	map(0x06, 0x06).w(FUNC(abc1600_mover_device::ldty_hb_w));
	map(0x07, 0x07).w(FUNC(abc1600_mover_device::ldty_lb_w));
}

void abc1600_mover_device::iowr1_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xf8).w(FUNC(abc1600_mover_device::ldfx_hb_w));
	map(0x01, 0x01).mirror(0xf8).w(FUNC(abc1600_mover_device::ldfx_lb_w));
	map(0x02, 0x02).mirror(0xf8).w(FUNC(abc1600_mover_device::ldfy_hb_w));
	map(0x03, 0x03).mirror(0xf8).w(FUNC(abc1600_mover_device::ldfy_lb_w));
	map(0x05, 0x05).mirror(0x08).select(0xf0).w(FUNC(abc1600_mover_device::wrml_w));
	map(0x07, 0x07).mirror(0x08).select(0xf0).w(FUNC(abc1600_mover_device::wrdl_w));
}

void abc1600_mover_device::iowr2_map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(abc1600_mover_device::wrmask_strobe_hb_w));
	map(0x01, 0x01).w(FUNC(abc1600_mover_device::wrmask_strobe_lb_w));
	map(0x02, 0x02).w(FUNC(abc1600_mover_device::enable_clocks_w));
	map(0x03, 0x03).w(FUNC(abc1600_mover_device::flag_strobe_w));
	map(0x04, 0x04).w(FUNC(abc1600_mover_device::endisp_w));
}


void abc1600_mover_device::mover_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
}


//-------------------------------------------------
//  ROM( abc1600_mover )
//-------------------------------------------------

ROM_START( abc1600_mover )
	ROM_REGION16_BE( 0x2000, "wrmsk", 0 )
	ROM_LOAD16_BYTE( "wrmskl 6490362-01.1g", 0x0001, 0x1000, CRC(bc737538) SHA1(80e2c3757eb7f713018808d6e41ebef612425028) )
	ROM_LOAD16_BYTE( "wrmskh 6490363-01.1j", 0x0000, 0x1000, CRC(6b7c9f0b) SHA1(7155a993adcf08a5a8a2f22becf9fd66fda698be) )

	ROM_REGION( 0x200, "shinf", 0 )
	ROM_LOAD( "shinf 6490361-01.1f", 0x000, 0x200, CRC(20260f8f) SHA1(29bf49c64e7cc7592e88cde2768ac57c7ce5e085) )

	ROM_REGION16_BE( 0x40, "drmsk", 0 )
	ROM_LOAD16_BYTE( "drmskl 6490359-01.1k", 0x01, 0x20, CRC(6e71087c) SHA1(0acf67700d6227f4b315cf8fb0fb31c0e7fb9496) )
	ROM_LOAD16_BYTE( "drmskh 6490358-01.1l", 0x00, 0x20, CRC(a4a9a9dc) SHA1(d8575c0335d6021cbb5f7bcd298b41c35294a80a) )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "drmsk 6490360-01.1m", 0x000, 0x104, CRC(5f7143c1) SHA1(1129917845f8e505998b15288f02bf907487e4ac) ) // PAL16L8 mover word mixer @ 1m,1n,1t,2t
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *abc1600_mover_device::device_rom_region() const
{
	return ROM_NAME( abc1600_mover );
}


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

inline uint16_t abc1600_mover_device::get_crtca(uint16_t ma, uint8_t ra, uint8_t column)
{
	/*

	    bit         description

	    CRTCA0      0
	    CRTCA1      0
	    CRTCA2      CC1/MA1
	    CRTCA3      CC2/MA2
	    CRTCA4      CC3/MA3
	    CRTCA5      CC4/MA4
	    CRTCA6      RA0
	    CRTCA7      RA1
	    CRTCA8      RA2
	    CRTCA9      RA3
	    CRTCA10     CR0/MA8
	    CRTCA11     CR1/MA9
	    CRTCA12     CR2/MA10
	    CRTCA13     CR3/MA11
	    CRTCA14     CR4/MA12
	    CRTCA15     CR5/MA13

	*/

	uint8_t cc = (ma & 0xff) + column;
	uint8_t cr = ma >> 8;

	return (cr << 10) | ((ra & 0x0f) << 6) | ((cc << 1) & 0x3c);
}

MC6845_UPDATE_ROW(abc1600_mover_device::crtc_update_row)
{
	if (((vbp + y) > cliprect.max_y) || ((vbp + y) < m_render_line))
		return;

	const pen_t *pen = m_palette->pens();
	const int width = std::min(((x_count + 1) / 2) * 64, cliprect.max_x - hbp + 1);

	if (width <= 0)
		return;

	uint32_t *dest = &bitmap.pix(vbp + y, hbp);

	if (!de || BLANK)
	{
		std::fill_n(dest, width, pen[0]);
		return;
	}

	const uint16_t pol = PIX_POL ? 0xffff : 0;
	int x = 0;
	s64 now = 0;
	s64 row_tick = 0;

	if (m_move_active)
	{
		now = now_tick();
		row_tick = now + (s64(vbp + y) - m_screen->vpos()) * m_screen->width() + (hbp - m_screen->hpos());
	}

	for (int column = 0; x < width; column += 2)
	{
		uint16_t dma = get_crtca(ma, ra, column);

		if (m_move_active)
			mover_catch_up(std::clamp<s64>(row_tick + x, 0, now), true);

		// data is read out of video RAM in nibble mode by strobing CAS 4 times
		for (int cas = 0; cas < 4 && x < width; cas++)
		{
			uint16_t data = read_videoram(dma + cas) ^ pol;
			const int bits = std::min(16, width - x);

			for (int bit = 0; bit < bits; bit++, data <<= 1)
				dest[x++] = pen[BIT(data, 15)];
		}
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED( abc1600_mover_device::crtc_update )
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc1600_mover_device::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_abc1600);

	screen_device &screen(SCREEN(config, SCREEN_TAG).set_color(rgb_t::green()));
	screen.set_screen_update(FUNC(abc1600_mover_device::screen_update));
	screen.set_raw(XTAL(64'000'000), HTOTAL, 0, HDISP, VTOTAL, 0, VDISP);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	SY6845E(config, m_crtc, XTAL(64'000'000)/32);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(32);
	m_crtc->set_update_row_callback(FUNC(abc1600_mover_device::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(abc1600_mover_device::crtc_update));

	TIMER(config, "amm").configure_generic(FUNC(abc1600_mover_device::amm));
}


//-------------------------------------------------
//  INPUT_PORTS( abc1600_mover )
//-------------------------------------------------

static INPUT_PORTS_START( abc1600_mover )
	PORT_START("SCREENPOS")
	PORT_CONFNAME( 0x02, 0x00, "Monitor Orientation" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(abc1600_mover_device::screenpos_changed), 0)
	PORT_CONFSETTING(    0x00, "Portrait" )
	PORT_CONFSETTING(    0x02, "Landscape" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor abc1600_mover_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc1600_mover );
}

INPUT_CHANGED_MEMBER( abc1600_mover_device::screenpos_changed )
{
	select_screenpos_view();
}

void abc1600_mover_device::select_screenpos_view()
{
	machine().render().first_target()->set_view(BIT(m_screenpos->read(), 1));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc1600_mover_device - constructor
//-------------------------------------------------

abc1600_mover_device::abc1600_mover_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABC1600_MOVER, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space_config("vram", ENDIANNESS_BIG, 16, 18, -1, address_map_constructor(FUNC(abc1600_mover_device::mover_map), this)),
	m_write_amm(*this),
	m_screen(*this, SCREEN_TAG),
	m_crtc(*this, "sy6845e"),
	m_palette(*this, "palette"),
	m_timer_amm(*this, "amm"),
	m_wrmsk_rom(*this, "wrmsk"),
	m_shinf_rom(*this, "shinf"),
	m_drmsk_rom(*this, "drmsk"),
	m_screenpos(*this, "SCREENPOS")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc1600_mover_device::device_start()
{
	// state saving
	save_item(NAME(m_endisp));
	save_item(NAME(m_clocks_disabled));
	save_item(NAME(m_gmdi));
	save_item(NAME(m_wrm));
	save_item(NAME(m_ms));
	save_item(NAME(m_ds));
	save_item(NAME(m_flag));
	save_item(NAME(m_xsize));
	save_item(NAME(m_ysize));
	save_item(NAME(m_udx));
	save_item(NAME(m_udy));
	save_item(NAME(m_xfrom));
	save_item(NAME(m_xto));
	save_item(NAME(m_yto));
	save_item(NAME(m_ty));
	save_item(NAME(m_mfa));
	save_item(NAME(m_mta));
	save_item(NAME(m_sh));
	save_item(NAME(m_mdor));
	save_item(NAME(m_hold_1w_cyk));
	save_item(NAME(m_wrms0));
	save_item(NAME(m_wrms1));
	save_item(NAME(m_rmc));
	save_item(NAME(m_cmc));
	save_item(NAME(m_amm));
	save_item(NAME(m_ammclk_phase));
	save_item(NAME(m_ldfy_pending));
	save_item(NAME(m_move_active));
	save_item(NAME(m_line_start));
	save_item(NAME(m_amm_start));
	save_item(NAME(m_amm_end));
	save_item(NAME(m_busy_end));
	save_item(NAME(m_step_tick));

	update_seq_edges();

	m_bitmap.allocate(HTOTAL, VTOTAL);
	m_render_timer = timer_alloc(FUNC(abc1600_mover_device::render_frame), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc1600_mover_device::device_reset()
{
	select_screenpos_view();

	// disable display
	m_clocks_disabled = 1;
	m_endisp = 0;

	// busy flag
	m_timer_amm->adjust(attotime::never);
	m_ldfy_pending = false;
	m_move_active = false;
	m_amm_start = m_amm_end = m_busy_end = 0;
	m_amm = 0;
	m_write_amm(!m_amm);

	m_render_frame = m_screen->frame_number();
	m_render_line = 0;
	m_render_timer->adjust(m_screen->time_until_pos(m_screen->visible_area().bottom()));
}


//-------------------------------------------------
//  device_post_load - device-specific post load
//-------------------------------------------------

void abc1600_mover_device::device_post_load()
{
	update_seq_edges();

	m_render_frame = m_screen->frame_number();
	m_render_line = 0;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector abc1600_mover_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


//-------------------------------------------------
//  read_videoram -
//-------------------------------------------------

inline uint16_t abc1600_mover_device::read_videoram(offs_t offset)
{
	return space().read_word((offset & VIDEORAM16_MASK) << 1);
}


//-------------------------------------------------
//  write_videoram -
//-------------------------------------------------

inline void abc1600_mover_device::write_videoram(offs_t offset, uint16_t data, uint16_t mask)
{
	uint16_t old_data = read_videoram(offset);

	space().write_word((offset & VIDEORAM16_MASK) << 1, (data & mask) | (old_data & (mask ^ 0xffff)));
}


//-------------------------------------------------
//  video_ram_r -
//-------------------------------------------------

uint8_t abc1600_mover_device::video_ram_r(offs_t offset)
{
	if (m_move_active && !machine().side_effects_disabled())
		mover_catch_up(~u64(0), false);

	offs_t addr = (offset & VIDEORAM8_MASK) >> 1;
	uint8_t data = 0;

	if (offset & 0x01)
	{
		data = read_videoram(addr) & 0xff;
	}
	else
	{
		data = read_videoram(addr) >> 8;
	}

	return data;
}


//-------------------------------------------------
//  video_ram_w -
//-------------------------------------------------

void abc1600_mover_device::video_ram_w(offs_t offset, uint8_t data)
{
	if (m_move_active)
		mover_catch_up(~u64(0), false);

	offs_t addr = (offset & VIDEORAM8_MASK) >> 1;

	if (offset & 0x01)
	{
		if (REPLACE)
		{
			// WRPORT_LB
			m_wrm = (m_wrm & 0xff00) | data;
			LOG("WRM LB %02x -> %04x\n", data, m_wrm);
		}
		else
		{
			// DATAPORT_LB
			m_gmdi = (m_gmdi & 0xff00) | data;
			LOG("GMDI LB %02x -> %04x\n", data, m_gmdi);
		}

		write_videoram(addr, m_gmdi, m_wrm & 0x00ff);

		LOG("Video RAM write LB to %05x : %04x\n", addr, read_videoram(addr));
	}
	else
	{
		if (REPLACE)
		{
			// WRPORT_HB
			m_wrm = (data << 8) | (m_wrm & 0xff);
			LOG("WRM HB %02x -> %04x\n", data, m_wrm);
		}
		else
		{
			// DATAPORT_HB
			m_gmdi = (data << 8) | (m_gmdi & 0xff);
			LOG("GMDI HB %02x -> %04x\n", data, m_gmdi);
		}

		write_videoram(addr, m_gmdi, m_wrm & 0xff00);

		LOG("Video RAM write HB to %05x : %04x\n", addr, read_videoram(addr));
	}
}


//-------------------------------------------------
//  iord0_r -
//-------------------------------------------------

uint8_t abc1600_mover_device::iord0_r()
{
	/*

	    bit     description

	    0       0
	    1       SCREENPOS
	    2
	    3
	    4
	    5
	    6       VSYNC
	    7       BUSY

	*/

	uint8_t data = 0;

	// monitor orientation
	data |= m_screenpos->read();

	// vertical sync
	data |= m_crtc->vsync_r() << 6;

	// busy
	u64 const now = now_tick();
	if (now >= m_amm_start && now < m_busy_end)
		data |= 0x80;

	return data;
}


//-------------------------------------------------
//  ldsx_hb_w -
//-------------------------------------------------

void abc1600_mover_device::ldsx_hb_w(uint8_t data)
{
	/*

	    bit     description

	    0       XSIZE8
	    1       XSIZE9
	    2       U/D* Y
	    3       U/D* X
	    4
	    5
	    6
	    7

	*/

	mover_sync();

	LOG("%s LDSX HB: %02x\n", machine().describe_context(), data);

	m_xsize = ((data & 0x03) << 8) | (m_xsize & 0xff);
	m_udy = BIT(data, 2);
	m_udx = BIT(data, 3);
}


//-------------------------------------------------
//  ldsx_lb_w -
//-------------------------------------------------

void abc1600_mover_device::ldsx_lb_w(uint8_t data)
{
	/*

	    bit     description

	    0       XSIZE0
	    1       XSIZE1
	    2       XSIZE2
	    3       XSIZE3
	    4       XSIZE4
	    5       XSIZE5
	    6       XSIZE6
	    7       XSIZE7

	*/

	mover_sync();

	LOG("%s LDSX LB: %02x\n", machine().describe_context(), data);

	m_xsize = (m_xsize & 0x300) | data;
}


//-------------------------------------------------
//  ldsy_hb_w -
//-------------------------------------------------

void abc1600_mover_device::ldsy_hb_w(uint8_t data)
{
	/*

	    bit     description

	    0       YSIZE8
	    1       YSIZE9
	    2       YSIZE10
	    3       YSIZE11
	    4
	    5
	    6
	    7

	*/

	mover_sync();

	LOG("%s LDSY HB: %02x\n", machine().describe_context(), data);

	m_ysize = ((data & 0x0f) << 8) | (m_ysize & 0xff);
}


//-------------------------------------------------
//  ldsy_lb_w -
//-------------------------------------------------

void abc1600_mover_device::ldsy_lb_w(uint8_t data)
{
	/*

	    bit     description

	    0       YSIZE0
	    1       YSIZE1
	    2       YSIZE2
	    3       YSIZE3
	    4       YSIZE4
	    5       YSIZE5
	    6       YSIZE6
	    7       YSIZE7

	*/

	mover_sync();

	LOG("%s LDSY LB: %02x\n", machine().describe_context(), data);

	m_ysize = (m_ysize & 0xf00) | data;
}


//-------------------------------------------------
//  ldtx_hb_w -
//-------------------------------------------------

void abc1600_mover_device::ldtx_hb_w(uint8_t data)
{
	/*

	    bit     description

	    0       XTO8, MTA4
	    1       XTO9, MTA5
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	mover_sync();

	LOG("%s LDTX HB: %02x\n", machine().describe_context(), data);

	m_xto = ((data & 0x03) << 8) | (m_xto & 0xff);
	m_mta = (m_mta & 0x3ffcf) | ((data & 0x03) << 4);
}


//-------------------------------------------------
//  ldtx_lb_w -
//-------------------------------------------------

void abc1600_mover_device::ldtx_lb_w(uint8_t data)
{
	/*

	    bit     description

	    0       XTO0
	    1       XTO1
	    2       XTO2
	    3       XTO3
	    4       XTO4, MTA0
	    5       XTO5, MTA1
	    6       XTO6, MTA2
	    7       XTO7, MTA3

	*/

	mover_sync();

	LOG("%s LDTX LB: %02x\n", machine().describe_context(), data);

	m_xto = (m_xto & 0x300) | data;
	m_mta = (m_mta & 0x3fff0) | (data >> 4);
}


//-------------------------------------------------
//  ldty_hb_w -
//-------------------------------------------------

void abc1600_mover_device::ldty_hb_w(uint8_t data)
{
	/*

	    bit     description

	    0       YTO8, MTA14
	    1       YTO9, MTA15
	    2       YTO10, MTA16
	    3       YTO11, MTA17
	    4
	    5
	    6
	    7

	*/

	mover_sync();

	LOG("%s LDTY HB: %02x\n", machine().describe_context(), data);

	m_ty = ((data & 0x0f) << 8) | (m_ty & 0xff);
	m_mta = ((data & 0x0f) << 14) | (m_mta & 0x3fff);
}


//-------------------------------------------------
//  ldty_lb_w -
//-------------------------------------------------

void abc1600_mover_device::ldty_lb_w(uint8_t data)
{
	/*

	    bit     description

	    0       YTO0, MTA6
	    1       YTO1, MTA7
	    2       YTO2, MTA8
	    3       YTO3, MTA9
	    4       YTO4, MTA10
	    5       YTO5, MTA11
	    6       YTO6, MTA12
	    7       YTO7, MTA13

	*/

	mover_sync();

	LOG("%s LDTY LB: %02x\n", machine().describe_context(), data);

	m_ty = (m_ty & 0xf00) | data;
	m_mta = (m_mta & 0x3c03f) | (data << 6);
}


//-------------------------------------------------
//  ldfx_hb_w -
//-------------------------------------------------

void abc1600_mover_device::ldfx_hb_w(uint8_t data)
{
	/*

	    bit     description

	    0       XFROM8, MFA4
	    1       XFROM9, MFA5
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	mover_sync();

	LOG("%s LDFX HB: %02x\n", machine().describe_context(), data);

	m_xfrom = ((data & 0x03) << 8) | (m_xfrom & 0xff);
	m_mfa = (m_mfa & 0x3ffcf) | ((data & 0x03) << 4);
}


//-------------------------------------------------
//  ldfx_lb_w -
//-------------------------------------------------

void abc1600_mover_device::ldfx_lb_w(uint8_t data)
{
	/*

	    bit     description

	    0       XFROM0
	    1       XFROM1
	    2       XFROM2
	    3       XFROM3
	    4       XFROM4, MFA0
	    5       XFROM5, MFA1
	    6       XFROM6, MFA2
	    7       XFROM7, MFA3

	*/

	mover_sync();

	LOG("%s LDFX LB: %02x\n", machine().describe_context(), data);

	m_xfrom = (m_xfrom & 0x300) | data;
	m_mfa = (m_mfa & 0x3fff0) | (data >> 4);
}


//-------------------------------------------------
//  ldfy_hb_w -
//-------------------------------------------------

void abc1600_mover_device::ldfy_hb_w(uint8_t data)
{
	/*

	    bit     description

	    0       MFA14
	    1       MFA15
	    2       MFA16
	    3       MFA17
	    4
	    5
	    6
	    7

	*/

	mover_sync();

	LOG("%s LDFY HB: %02x\n", machine().describe_context(), data);

	m_mfa = ((data & 0x0f) << 14) | (m_mfa & 0x3fff);
}


//-------------------------------------------------
//  ldfy_lb_w -
//-------------------------------------------------

void abc1600_mover_device::ldfy_lb_w(uint8_t data)
{
	/*

	    bit     description

	    0       MFA6
	    1       MFA7
	    2       MFA8
	    3       MFA9
	    4       MFA10
	    5       MFA11
	    6       MFA12
	    7       MFA13

	*/

	mover_sync();

	LOG("%s LDFY LB: %02x\n", machine().describe_context(), data);

	m_mfa = (m_mfa & 0x3c03f) | (data << 6);

	u64 const now = now_tick();

	if (m_move_active || (now < m_amm_end))
		m_ldfy_pending = true;
	else
		start_move(now);
}


//-------------------------------------------------
//  wrml_w -
//-------------------------------------------------

void abc1600_mover_device::wrml_w(offs_t offset, uint8_t data)
{
	/*

	    bit     description

	    0       MOVE CYK CLK
	    1       DISP CYC SEL / DISP CYK PRE FETCH (+1 PIXCLK)
	    2       DATA CLK
	    3       _DISP MEM WE
	    4       _CAS HB
	    5       DTACK CLK / BLANK TEST (+2 PIXCLK)
	    6       DISPREC CLK
	    7       _RAS HB

	*/

	LOG("MS %u : %02x\n", (offset >> 4) & 0x0f, data);

	if (m_clocks_disabled)
	{
		m_ms[(offset >> 4) & 0x0f] = data;
	}
}


//-------------------------------------------------
//  wrdl_w -
//-------------------------------------------------

void abc1600_mover_device::wrdl_w(offs_t offset, uint8_t data)
{
	/*

	    bit     description

	    0       MOVE CYK CLK
	    1       DISP CYC SEL / DISP CYK PRE FETCH (+1 PIXCLK)
	    2       DATA CLK
	    3       _DISP MEM WE
	    4       _CAS HB
	    5       DTACK CLK / BLANK TEST (+2 PIXCLK)
	    6       DISPREC CLK
	    7       _RAS HB

	*/

	LOG("WS %u : %02x\n", (offset >> 4) & 0x0f, data);

	if (m_clocks_disabled)
	{
		m_ds[(offset >> 4) & 0x0f] = data;
	}
}


//-------------------------------------------------
//  wrmask_strobe_hb_w -
//-------------------------------------------------

void abc1600_mover_device::wrmask_strobe_hb_w(uint8_t data)
{
	if (REPLACE)
	{
		// DATAPORT_HB
		m_gmdi = (data << 8) | (m_gmdi & 0xff);
		LOG("GMDI HB %04x\n", m_gmdi);
	}
	else
	{
		// WRPORT_HB
		m_wrm = (data << 8) | (m_wrm & 0xff);
		LOG("WRM HB %04x\n", m_gmdi);
	}
}


//-------------------------------------------------
//  wrmask_strobe_lb_w -
//-------------------------------------------------

void abc1600_mover_device::wrmask_strobe_lb_w(uint8_t data)
{
	if (REPLACE)
	{
		// DATAPORT_LB
		m_gmdi = (m_gmdi & 0xff00) | data;
		LOG("GMDI LB %04x\n", m_gmdi);
	}
	else
	{
		// WRPORT_LB
		m_wrm = (m_wrm & 0xff00) | data;
		LOG("WRM LB %04x\n", m_gmdi);
	}
}


//-------------------------------------------------
//  enable_clocks_w -
//-------------------------------------------------

void abc1600_mover_device::enable_clocks_w(uint8_t data)
{
	LOG("ENABLE CLOCKS\n");

	if (!m_clocks_disabled)
		return;

	m_clocks_disabled = 0;
	update_seq_edges();

	u64 const now = now_tick();
	m_ammclk_phase = edge_parity(now);

	if (m_ldfy_pending)
		start_move(now);
}


//-------------------------------------------------
//  flag_strobe_w -
//-------------------------------------------------

void abc1600_mover_device::flag_strobe_w(uint8_t data)
{
	/*

	    bit     description

	    0       L/_P FLAG
	    1       BLANK FLAG
	    2       PIX POL
	    3       FRAME POL
	    4       HOLD FY
	    5       HOLD FX
	    6       COMP MOVE FLAG
	    7       REPLACE/SET & RESET

	*/

	mover_sync();

	m_flag = data;
	LOG("FLAG %02x\n", m_flag);
}


//-------------------------------------------------
//  endisp_w -
//-------------------------------------------------

void abc1600_mover_device::endisp_w(uint8_t data)
{
	m_endisp = 1;
	LOG("ENDISP\n");
}



//**************************************************************************
//  MOVER
//**************************************************************************

//-------------------------------------------------
//  clock_mfa_x -
//-------------------------------------------------

inline void abc1600_mover_device::clock_mfa_x()
{
	uint16_t mfa_y = m_mfa >> 6;
	uint8_t mfa_x = m_mfa & 0x3f;

	if (!HOLD_FX)
	{
		mfa_x += m_udx ? 1 : -1;
		mfa_x &= 0x3f;
	}

	m_mfa = (mfa_y << 6) | mfa_x;
}


//-------------------------------------------------
//  clock_mfa_y -
//-------------------------------------------------

inline void abc1600_mover_device::clock_mfa_y()
{
	uint16_t mfa_y = m_mfa >> 6;
	uint8_t mfa_x = m_mfa & 0x3f;

	if (!HOLD_FY)
	{
		mfa_y += m_udy ? 1 : -1;
		mfa_y &= 0xfff;
	}

	m_mfa = (mfa_y << 6) | mfa_x;
}


//-------------------------------------------------
//  clock_mta_x -
//-------------------------------------------------

inline void abc1600_mover_device::clock_mta_x()
{
	uint16_t mta_y = m_mta >> 6;
	uint8_t mta_x = m_mta & 0x3f;

	mta_x += m_udx ? 1 : -1;
	mta_x &= 0x3f;

	m_mta = (mta_y << 6) | mta_x;
}


//-------------------------------------------------
//  clock_mta_y -
//-------------------------------------------------

inline void abc1600_mover_device::clock_mta_y()
{
	uint16_t mta_y = m_mta >> 6;
	uint8_t mta_x = m_mta & 0x3f;

	mta_y += m_udy ? 1 : -1;
	mta_y &= 0xfff;

	m_mta = (mta_y << 6) | mta_x;
}


//-------------------------------------------------
//  load_mfa_x -
//-------------------------------------------------

inline void abc1600_mover_device::load_mfa_x()
{
	uint16_t mfa_y = m_mfa >> 6;
	uint8_t mfa_x = m_xfrom >> 4;

	m_mfa = (mfa_y << 6) | mfa_x;
}


//-------------------------------------------------
//  load_mta_x -
//-------------------------------------------------

inline void abc1600_mover_device::load_mta_x()
{
	uint16_t mta_y = m_mta >> 6;
	uint8_t mta_x = m_xto >> 4;

	m_mta = (mta_y << 6) | mta_x;
}


//-------------------------------------------------
//  load_xy_reg -
//-------------------------------------------------

inline void abc1600_mover_device::load_xy_reg()
{
	if (L_P) return;

	uint16_t sum = m_xto + m_xsize + 1;

	m_xto = sum & 0x3ff;
	m_mta = (m_ty << 6) | (sum >> 4);
}


//-------------------------------------------------
//  compare_mta_x -
//-------------------------------------------------

inline void abc1600_mover_device::compare_mta_x()
{
	uint8_t mta_x_end = ((m_xto + m_xsize) >> 4) & 0x3f;
	uint8_t mta_x = m_mta & 0x3f;

	if (mta_x == mta_x_end)
	{
		m_cmc = 0;
	}

	m_wrms1 = m_cmc;
}


//-------------------------------------------------
//  compare_mta_y -
//-------------------------------------------------

inline void abc1600_mover_device::compare_mta_y()
{
	int mta_y_end = (m_yto + m_ysize) & 0xfff;
	uint16_t mta_y = m_mta >> 6;

	if (mta_y == mta_y_end)
	{
		m_rmc = 0;
	}
}


//-------------------------------------------------
//  get_shinf -
//-------------------------------------------------

inline void abc1600_mover_device::get_shinf()
{
	/*

	    bit     description

	    A0      XFROM0
	    A1      XFROM1
	    A2      XFROM2
	    A3      XFROM3
	    A4      XTO0
	    A5      XTO1
	    A6      XTO2
	    A7      XTO3
	    A8      U/D* X

	*/

	uint16_t shinf_addr = (m_udx << 8) | ((m_xto & 0x0f) << 4) | (m_xfrom & 0x0f);
	uint8_t shinf = m_shinf_rom[shinf_addr];

	m_sh = shinf & 0x0f;
	m_hold_1w_cyk = BIT(shinf, 5);
}


//-------------------------------------------------
//  get_drmsk -
//-------------------------------------------------

inline uint16_t abc1600_mover_device::get_drmsk()
{
	/*

	    bit     description

	    A0      SH0
	    A1      SH1
	    A2      SH2
	    A3      SH3
	    A4      U/D* X

	*/

	uint16_t drmsk_addr = (m_udx << 4) | (m_sh & 0x0f);
	return m_drmsk_rom[drmsk_addr];
}


//-------------------------------------------------
//  get_wrmsk - get mover write mask
//-------------------------------------------------

inline uint16_t abc1600_mover_device::get_wrmsk()
{
	/*

	    bit     description

	    A0      XTO0
	    A1      XTO1
	    A2      XTO2
	    A3      XTO3
	    A4      XSIZE0
	    A5      XSIZE1
	    A6      XSIZE2
	    A7      XSIZE3
	    A8      U/D* X
	    A9      ANDED MASKS
	    A10     WRMS0
	    A11     WRMS1

	*/

	uint16_t wrmsk_addr = (m_wrms1 << 11) | (m_wrms0 << 10) | ((!m_wrms1 && !m_wrms0) << 9) | (m_udx << 8) | ((m_xsize & 0x0f) << 4) | (m_xto & 0x0f);
	return m_wrmsk_rom[wrmsk_addr] ^ 0xffff;
}


//-------------------------------------------------
//  barrel_shift -
//-------------------------------------------------

inline uint16_t abc1600_mover_device::barrel_shift(uint16_t gmdr)
{
	uint16_t rot = gmdr;

	for (int sh = 0; sh < m_sh; sh++)
	{
		int msb = BIT(rot, 15);
		rot <<= 1;
		rot |= msb;
	}

	return rot;
}


//-------------------------------------------------
//  word_mixer -
//-------------------------------------------------

inline uint16_t abc1600_mover_device::word_mixer(uint16_t rot)
{
	uint16_t drmsk = get_drmsk();
	uint16_t gmdi = (rot & drmsk) | (m_mdor & (drmsk ^ 0xffff));

	if (COMP_MOVE)
	{
		gmdi ^= 0xffff;
	}

	m_mdor = rot;

	return gmdi;
}


//-------------------------------------------------
//  tick_time - delay until a 64 MHz tick
//-------------------------------------------------

attotime abc1600_mover_device::tick_time(u64 tick) const
{
	attotime const when = attotime::from_ticks(tick, clock());
	attotime const now = machine().time();

	return (when > now) ? (when - now) : attotime::zero;
}


//-------------------------------------------------
//  window_ds - display sequence table selected
//-------------------------------------------------

bool abc1600_mover_device::window_ds(u64 window) const
{
	u64 const pos = (window * 32) % (HTOTAL * VTOTAL);
	u64 const x = pos % HTOTAL;

	return (pos / HTOTAL < VDISP) && (x < HDISP) && !BIT(x, 5);
}


//-------------------------------------------------
//  seq_bit - sequencer output level
//-------------------------------------------------

bool abc1600_mover_device::seq_bit(u64 slot, int bit) const
{
	return !BIT(window_ds(slot >> 4) ? m_ds[~slot & 0x0f] : m_ms[~slot & 0x0f], bit);
}


//-------------------------------------------------
//  update_seq_edges - MOVE CYK CLK edge tables
//-------------------------------------------------

void abc1600_mover_device::update_seq_edges()
{
	for (int prev_ds = 0; prev_ds < 2; prev_ds++)
	{
		for (int ds = 0; ds < 2; ds++)
		{
			uint8_t const *const prev = prev_ds ? m_ds : m_ms;
			uint8_t const *const cur = ds ? m_ds : m_ms;
			u16 mask = 0;

			for (int slot = 0; slot < 16; slot++)
			{
				bool const level = !BIT(cur[~slot & 0x0f], 0);
				bool const last = slot ? !BIT(cur[~(slot - 1) & 0x0f], 0) : !BIT(prev[0], 0);

				if (level && !last)
					mask |= 1 << slot;
			}

			m_cyk_mask[(prev_ds << 1) | ds] = mask;
		}
	}

	m_frame_edges = 0;
	m_line_edges[0] = m_line_edges[1] = 0;

	if (!m_cyk_mask[0] && !m_cyk_mask[1] && !m_cyk_mask[2] && !m_cyk_mask[3])
		return;

	m_line_edges[1] = count_edges(0, HTOTAL);
	m_line_edges[0] = count_edges(VDISP * HTOTAL, (VDISP + 1) * HTOTAL);
	m_frame_edges = VDISP * m_line_edges[1] + (VTOTAL - VDISP) * m_line_edges[0];
}


//-------------------------------------------------
//  next_edge - first MOVE CYK CLK edge after tick
//-------------------------------------------------

u64 abc1600_mover_device::next_edge(u64 tick) const
{
	u64 window = tick >> 5;

	if (window)
		window--;

	bool prev_ds = window ? window_ds(window - 1) : false;

	for (u64 end = window + (HTOTAL * VTOTAL / 32) + 2; window < end; window++)
	{
		bool const ds = window_ds(window);
		u16 mask = m_cyk_mask[(prev_ds ? 2 : 0) | (ds ? 1 : 0)];

		while (mask)
		{
			int const slot = std::countr_zero(mask);
			u64 const edge = (window << 5) + (slot << 1) + 2;

			if (edge > tick)
				return edge;

			mask &= mask - 1;
		}

		prev_ds = ds;
	}

	return ~u64(0);
}


//-------------------------------------------------
//  count_edges - MOVE CYK CLK edges in (from, to]
//-------------------------------------------------

u64 abc1600_mover_device::count_edges(u64 from, u64 to) const
{
	u64 count = 0;

	for (u64 edge = next_edge(from); edge <= to; edge = next_edge(edge))
		count++;

	return count;
}


//-------------------------------------------------
//  advance_edges - tick of the nth edge after tick
//-------------------------------------------------

u64 abc1600_mover_device::advance_edges(u64 tick, u64 count) const
{
	if (!m_frame_edges)
		return ~u64(0);

	u64 const line_end = (tick / HTOTAL + 1) * HTOTAL;
	u64 const partial = count_edges(tick, line_end);

	if (partial < count)
	{
		count -= partial;
		tick = line_end;

		for (;;)
		{
			u64 const line_edges = m_line_edges[(tick / HTOTAL) % VTOTAL < VDISP];

			if (line_edges >= count)
				break;

			count -= line_edges;
			tick += HTOTAL;
		}
	}

	while (count--)
		tick = next_edge(tick);

	return tick;
}


//-------------------------------------------------
//  edge_parity - MOVE CYK CLK edges up to tick
//-------------------------------------------------

bool abc1600_mover_device::edge_parity(u64 tick) const
{
	u64 const frame = HTOTAL * VTOTAL;
	u64 const pos = tick % frame;
	u64 const line = pos / HTOTAL;
	u64 count = (tick / frame) * m_frame_edges;

	count += std::min<u64>(line, VDISP) * m_line_edges[1];
	count += ((line > VDISP) ? (line - VDISP) : 0) * m_line_edges[0];
	count += count_edges(tick - (pos % HTOTAL), tick);

	return BIT(count, 0);
}


//-------------------------------------------------
//  cpu_pack_tick - CPU video RAM acknowledge time
//-------------------------------------------------

u64 abc1600_mover_device::cpu_pack_tick(u64 das, bool write) const
{
	int edges = write ? 1 : 2;
	u64 pack = das;

	for (u64 slot = das >> 1; slot < (das >> 1) + 64; slot++)
	{
		u64 const edge = (slot << 1) + 2;

		if (edge > das && seq_bit(slot, 5) && !seq_bit(slot - 1, 5) && !--edges)
		{
			pack = edge;
			break;
		}
	}

	if (pack >= m_amm_start && pack < m_amm_end)
		pack = m_amm_end;

	return pack;
}


//-------------------------------------------------
//  amm - active mover mask timer
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER( abc1600_mover_device::amm )
{
	if (param)
	{
		m_amm = 1;
		m_write_amm(!m_amm);
		m_timer_amm->adjust(tick_time(m_amm_end));
	}
	else
	{
		mover_catch_up(~u64(0), false);
		load_xy_reg();

		m_amm = 0;
		m_write_amm(!m_amm);

		if (m_ldfy_pending)
			start_move(now_tick());
	}

	LOG("%s AMM %u\n", machine().time().as_string(), m_amm);
}


//-------------------------------------------------
//  start_move - LDFY LB sets the AMM request
//-------------------------------------------------

void abc1600_mover_device::start_move(u64 tick)
{
	if (m_clocks_disabled || !m_frame_edges)
	{
		m_ldfy_pending = true;
		return;
	}

	m_ldfy_pending = false;

	LOG("XFROM %u XSIZE %u YSIZE %u XTO %u YTO %u MFA %05x MTA %05x U/D*X %u U/D*Y %u\n", m_xfrom, m_xsize, m_ysize, m_xto, m_yto, m_mfa, m_mta, m_udx, m_udy);

	get_shinf();

	m_rmc = 1;
	m_yto = m_mta >> 6;
	m_line_start = true;
	m_move_active = true;

	uint8_t const x0 = (m_xto >> 4) & 0x3f;
	uint8_t const x1 = ((m_xto + m_xsize) >> 4) & 0x3f;
	uint16_t const y0 = m_yto;
	uint16_t const y1 = (m_yto + m_ysize) & 0xfff;
	u64 const words = ((m_udx ? (x1 - x0) : (x0 - x1)) & 0x3f) + 1;
	u64 const lines = ((m_udy ? (y1 - y0) : (y0 - y1)) & 0xfff) + 1;
	u64 const periods = lines * (words + m_hold_1w_cyk);

	// AMM rises on the first AMMCLK rising edge, every word takes one AMMCLK period
	u64 start = next_edge(tick);

	if (edge_parity(start) == m_ammclk_phase)
		start = next_edge(start);

	m_amm_start = start;
	m_step_tick = next_edge(start);
	m_amm_end = advance_edges(start, periods * 2);
	m_busy_end = advance_edges(m_amm_end, 2);

	m_timer_amm->adjust(tick_time(m_amm_start), 1);
}


//-------------------------------------------------
//  mover_step - one AMMCLK period
//-------------------------------------------------

void abc1600_mover_device::mover_step()
{
	if (m_line_start)
	{
		m_line_start = false;

		compare_mta_y();

		load_mfa_x();
		load_mta_x();
		m_cmc = 1;
		m_wrms0 = 0;

		if (m_hold_1w_cyk)
		{
			// read one word in advance
			uint16_t gmdr = read_videoram(m_mfa);
			uint16_t rot = barrel_shift(gmdr);
			word_mixer(rot);

			clock_mfa_x();
			return;
		}
	}

	compare_mta_x();

	uint16_t gmdr = read_videoram(m_mfa);
	uint16_t rot = barrel_shift(gmdr);
	uint16_t gmdi = word_mixer(rot);
	uint16_t mask = get_wrmsk();

	write_videoram(m_mta, gmdi, mask);

	clock_mfa_x();
	clock_mta_x();

	m_wrms0 = 1;

	if (!m_cmc)
	{
		clock_mfa_y();
		clock_mta_y();

		if (m_rmc)
			m_line_start = true;
		else
			m_move_active = false;
	}
}


//-------------------------------------------------
//  mover_catch_up - run the mover up to tick
//-------------------------------------------------

void abc1600_mover_device::mover_catch_up(u64 tick, bool display)
{
	if (!m_move_active || (m_step_tick > tick))
		return;

	if (!display)
		render_rows(m_screen->vpos());

	while (m_move_active && (m_step_tick <= tick))
	{
		mover_step();

		m_step_tick = next_edge(next_edge(m_step_tick));
	}
}

//-------------------------------------------------
//  render_rows - draw the frame up to a scanline
//-------------------------------------------------

void abc1600_mover_device::render_rows(int end)
{
	if (m_screen->frame_number() != m_render_frame)
	{
		m_render_frame = m_screen->frame_number();
		m_render_line = 0;
	}

	rectangle const clip = m_screen->visible_area() & m_bitmap.cliprect();
	end = std::min(end, clip.bottom() + 1);

	if (end <= m_render_line)
		return;

	rectangle const rows(clip.left(), clip.right(), m_render_line, end - 1);
	rectangle const frame(clip.left(), clip.right(), 0, end - 1);

	if (m_endisp)
	{
		m_bitmap.fill(m_palette->pen(FRAME_POL), rows);
		m_crtc->screen_update(*m_screen, m_bitmap, frame);
	}
	else
	{
		m_bitmap.fill(rgb_t::black(), rows);
	}

	m_render_line = end;
}


//-------------------------------------------------
//  render_frame - draw the rest of the frame
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(abc1600_mover_device::render_frame)
{
	int const bottom = m_screen->visible_area().bottom();

	render_rows(bottom + 1);

	m_render_timer->adjust(m_screen->time_until_pos(bottom));
}


uint32_t abc1600_mover_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}
