// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II/12/16/6000 video/keyboard interface emulation

    Model II: 8000043 Rev A/B, 8709048 Rev C
    Model 12/16B/6000: 8000176 (Project 448)

    Port    Read                            Write
    FC      keyboard data, clears KBIRQ     CRTC address
    FD      CRTC data                       CRTC data
    FE      clears RTC interrupt
    FF      status                          control latch (bits 4-7)

    TODO:

    - Model 12/16B/6000 video wait jumper factory position unknown
      (E1-E2 + E5-E6 inferred).
    - Bulletin 12/16B:12 asynchronous MSP preset mod not modelled.
    - Blanked character cell during CPU video RAM access not modelled.

**********************************************************************/

#include "emu.h"
#include "vidkb.h"

#include <algorithm>


namespace {

constexpr u32 DOT_CLOCK = 12'480'000;

} // anonymous namespace


DEFINE_DEVICE_TYPE(TRS80M2_VIDKB, trs80m2_vidkb_device, "trs80m2_vidkb", "Tandy Model II video/keyboard interface")
DEFINE_DEVICE_TYPE(TRS80M12_VIDKB, trs80m12_vidkb_device, "trs80m12_vidkb", "Tandy Model 12 video/keyboard interface")


ROM_START( trs80m2_vidkb )
	ROM_REGION( 0x800, "u9", 0 )
	ROM_LOAD( "8043316.u9", 0x0000, 0x0800, CRC(04425b03) SHA1(32a29dc202b7fcf21838289cc3bffc51ef943dab) )
ROM_END

const tiny_rom_entry *trs80m2_vidkb_device_base::device_rom_region() const
{
	return ROM_NAME( trs80m2_vidkb );
}


static INPUT_PORTS_START( trs80m2_vidkb )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "RTC rate" )
	PORT_CONFSETTING( 0x00, "30 Hz (J1-J2)" )
	PORT_CONFSETTING( 0x01, "60 Hz (J1-J3)" )
	PORT_CONFNAME( 0x02, 0x02, "Board revision" )
	PORT_CONFSETTING( 0x00, "Rev A/B (8000043)" )
	PORT_CONFSETTING( 0x02, "Rev C (8709048)" )
INPUT_PORTS_END

static INPUT_PORTS_START( trs80m12_vidkb )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "RTC rate" )
	PORT_CONFSETTING( 0x00, "30 Hz (J1-J2)" )
	PORT_CONFSETTING( 0x01, "60 Hz (J1-J3)" )
	PORT_CONFNAME( 0x04, 0x00, "Video wait" )
	PORT_CONFSETTING( 0x00, "On request (E1-E2, E5-E6)" )
	PORT_CONFSETTING( 0x04, "During retrace (E2-E3, E4-E5)" )
INPUT_PORTS_END

ioport_constructor trs80m2_vidkb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( trs80m2_vidkb );
}

ioport_constructor trs80m12_vidkb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( trs80m12_vidkb );
}


trs80m2_vidkb_device_base::trs80m2_vidkb_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool has_mirror) :
	device_t(mconfig, type, tag, owner, clock),
	device_trs80m2_card_interface(mconfig, *this),
	m_config(*this, "CONFIG"),
	m_screen(*this, "screen"),
	m_crtc(*this, "u11"),
	m_palette(*this, "palette"),
	m_kb(*this, "kbd"),
	m_char_rom(*this, "u9"),
	m_video_ram(*this, "video_ram", 0x800, ENDIANNESS_LITTLE),
	m_has_mirror(has_mirror),
	m_installed(false),
	m_crtc_address(0),
	m_hdisp(0),
	m_de(0),
	m_key_data(0),
	m_key_bit(0),
	m_kbclk(1),
	m_kbdata(0),
	m_kbirq(1),
	m_blnkvid(0),
	m_80_40_char_en(0),
	m_enable_rtc_int(0),
	m_rtc_div(0),
	m_rtc_int(0)
{
}

trs80m2_vidkb_device::trs80m2_vidkb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_vidkb_device_base(mconfig, TRS80M2_VIDKB, tag, owner, clock, true)
{
}

trs80m12_vidkb_device::trs80m12_vidkb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_vidkb_device_base(mconfig, TRS80M12_VIDKB, tag, owner, clock, false)
{
}

void trs80m2_vidkb_device_base::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(12.48_MHz_XTAL, 800, 0, 640, 260, 0, 240);
	m_screen->set_screen_update(FUNC(trs80m2_vidkb_device_base::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	MC6845(config, m_crtc, 12.48_MHz_XTAL / 8);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(trs80m2_vidkb_device_base::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(trs80m2_vidkb_device_base::de_w));
	m_crtc->out_vsync_callback().set(FUNC(trs80m2_vidkb_device_base::vsync_w));

	TRS80M2_KEYBOARD_PORT(config, m_kb, trs80m2_keyboards, "trs80m2_kbd");
	m_kb->clock_wr_callback().set(FUNC(trs80m2_vidkb_device_base::kb_clock_w));
}

void trs80m2_vidkb_device_base::map(address_map &map)
{
	map(0x0, 0x0).r(FUNC(trs80m2_vidkb_device_base::keyboard_r)).w(FUNC(trs80m2_vidkb_device_base::crtc_address_w));
	map(0x1, 0x1).r(m_crtc, FUNC(mc6845_device::register_r)).w(FUNC(trs80m2_vidkb_device_base::crtc_register_w));
	map(0x2, 0x2).r(FUNC(trs80m2_vidkb_device_base::rtc_r));
	map(0x3, 0x3).rw(FUNC(trs80m2_vidkb_device_base::status_r), FUNC(trs80m2_vidkb_device_base::latch_w));
}

void trs80m2_vidkb_device_base::device_start()
{
	trs80m2_bus_device &bus = m_slot->bus();

	bus.io().install_device(0xfc, 0xff, *this, &trs80m2_vidkb_device_base::map);

	for (u32 i = 0; i < trs80m2_bus_device::PAGE_ENTRIES; i++)
	{
		if (trs80m2_bus_device::entry_vram(i))
		{
			bus.page_view(i).install_readwrite_handler(0xf800, 0xffff,
					emu::rw_delegate(*this, FUNC(trs80m2_vidkb_device_base::video_ram_r)),
					emu::rw_delegate(*this, FUNC(trs80m2_vidkb_device_base::video_ram_w)));
		}
	}

	save_item(NAME(m_crtc_address));
	save_item(NAME(m_hdisp));
	save_item(NAME(m_de_start));
	save_item(NAME(m_de));
	save_item(NAME(m_key_data));
	save_item(NAME(m_key_bit));
	save_item(NAME(m_kbclk));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_kbirq));
	save_item(NAME(m_blnkvid));
	save_item(NAME(m_80_40_char_en));
	save_item(NAME(m_enable_rtc_int));
	save_item(NAME(m_rtc_div));
	save_item(NAME(m_rtc_int));
}

void trs80m2_vidkb_device_base::device_reset()
{
	if (!m_installed)
	{
		m_installed = true;

		if (m_has_mirror && !BIT(m_config->read(), 1))
			m_slot->bus().io().install_device(0x7c, 0x7f, *this, &trs80m2_vidkb_device_base::map);
	}

	m_kbirq = 1;
	update_kbirq();

	latch_w(0);

	m_rtc_int = 0;
	update_nmi();
}

int trs80m2_vidkb_device_base::cclk_wait(attotime const &now)
{
	uint64_t const dots_per_char = DOT_CLOCK / m_crtc->clock();
	uint64_t const dot = now.as_ticks(DOT_CLOCK);
	uint64_t const release = (dot / dots_per_char + 1) * dots_per_char + 2 * (dots_per_char / 8);

	return std::max<int>(1, (attotime::from_ticks(release, DOT_CLOCK) - now).as_ticks(clock()) + 1);
}

int trs80m2_vidkb_device_base::blank_wait(attotime const &now)
{
	if (!m_de)
		return 1;

	attotime const end = m_de_start + attotime::from_ticks(m_hdisp, m_crtc->clock());

	if (end <= now)
		return 1;

	return (end - now).as_ticks(clock()) + 1;
}

int trs80m2_vidkb_device::access_wait(attotime const &now)
{
	return cclk_wait(now);
}

int trs80m12_vidkb_device::access_wait(attotime const &now)
{
	return BIT(m_config->read(), 2) ? blank_wait(now) : cclk_wait(now);
}

uint8_t trs80m2_vidkb_device_base::video_ram_r(offs_t offset)
{
	if (m_slot->bus().wait(access_wait(m_slot->bus().local_time()), this))
		return 0;

	return m_video_ram[offset];
}

void trs80m2_vidkb_device_base::video_ram_w(offs_t offset, uint8_t data)
{
	if (m_slot->bus().wait(access_wait(m_slot->bus().local_time()), this))
		return;

	m_video_ram[offset] = data;
}

uint8_t trs80m2_vidkb_device_base::keyboard_r()
{
	if (!machine().side_effects_disabled())
	{
		if (!m_kbirq)
		{
			m_kbirq = 1;
			update_kbirq();
		}

		m_key_bit = 0;
	}

	return m_key_data;
}

void trs80m2_vidkb_device_base::crtc_address_w(uint8_t data)
{
	m_crtc_address = data;
	m_crtc->address_w(data);
}

void trs80m2_vidkb_device_base::crtc_register_w(uint8_t data)
{
	if ((m_crtc_address & 0x1f) == 1)
		m_hdisp = data;

	m_crtc->register_w(data);
}

uint8_t trs80m2_vidkb_device_base::rtc_r()
{
	if (!machine().side_effects_disabled())
	{
		m_rtc_int = 0;
		update_nmi();
	}

	return 0;
}

uint8_t trs80m2_vidkb_device_base::status_r()
{
	/*

	    bit     signal              description

	    0-3                         not driven
	    4       80/40 CHAR EN       80/40 character mode
	    5       ENABLE RTC INT      RTC interrupt enable
	    6       BLNKVID             video blanked
	    7       KBIRQ               keyboard interrupt

	*/

	uint8_t data = 0;

	data |= m_80_40_char_en << 4;
	data |= m_enable_rtc_int << 5;
	data |= m_blnkvid << 6;
	data |= m_kbirq ? 0x00 : 0x80;

	return data;
}

void trs80m2_vidkb_device_base::latch_w(uint8_t data)
{
	/*

	    bit     signal              description

	    0-3                         memory page select (RAM boards)
	    4       80/40 CHAR EN       80/40 character mode
	    5       ENABLE RTC INT      RTC interrupt enable
	    6       BLNKVID             video display blank
	    7                           video RAM enable

	*/

	m_screen->update_partial(m_screen->vpos());

	m_80_40_char_en = BIT(data, 4);
	m_crtc->set_unscaled_clock(12.48_MHz_XTAL / (m_80_40_char_en ? 16 : 8));

	m_enable_rtc_int = BIT(data, 5);
	update_nmi();

	m_blnkvid = BIT(data, 6);
}

void trs80m2_vidkb_device_base::update_nmi()
{
	m_slot->nmi_w(m_rtc_int && m_enable_rtc_int);
}

void trs80m2_vidkb_device_base::update_kbirq()
{
	m_slot->kbirq_w(!m_kbirq);
	m_kb->busy_w(!m_kbirq);
}

void trs80m2_vidkb_device_base::de_w(int state)
{
	if (state && !m_de)
		m_de_start = machine().time();

	m_de = state;
}

void trs80m2_vidkb_device_base::vsync_w(int state)
{
	if (!state)
		return;

	bool rtc = true;

	if (!BIT(m_config->read(), 0))
	{
		m_rtc_div = !m_rtc_div;
		rtc = m_rtc_div;
	}

	if (rtc)
	{
		m_rtc_int = 1;
		update_nmi();
	}
}

void trs80m2_vidkb_device_base::kb_clock_w(int state)
{
	int const kbdata = m_kb->data_r();

	if (m_key_bit == 8)
	{
		if (!m_kbdata && kbdata)
		{
			m_kbirq = 0;
			update_kbirq();
		}
	}
	else if (m_kbclk && !state)
	{
		m_key_data >>= 1;
		m_key_data |= kbdata << 7;
		m_key_bit++;
	}

	m_kbdata = kbdata;
	m_kbclk = state;
}

MC6845_UPDATE_ROW( trs80m2_vidkb_device_base::crtc_update_row )
{
	pen_t const *const pen = m_palette->pens();

	uint32_t *dst = &bitmap.pix(vbp + y, hbp);

	for (int column = 0; column < x_count; column++)
	{
		uint8_t const code = m_video_ram[(ma + column) & 0x7ff];
		offs_t const address = ((code & 0x7f) << 4) | (ra & 0x0f);
		uint8_t data = m_char_rom[address];

		int const dcursor = (column == cursor_x);
		int const drevid = BIT(code, 7);

		for (int bit = 0; bit < 8; bit++)
		{
			int const dout = BIT(data, 7);
			int const color = (dcursor ^ drevid ^ dout) && de;

			*dst++ = pen[color];

			data <<= 1;
		}
	}
}

uint32_t trs80m2_vidkb_device_base::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	if (!m_blnkvid)
		m_crtc->screen_update(screen, bitmap, cliprect);

	return 0;
}
