// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega Picture Magic (codename JANUS) */
// http://segaretro.org/Sega_Picture_Magic

// this uses a Sega 32X PCB (not in a 32X case) attached to a stripped down 68k based board rather than a full Genesis / Megadrive
// it is possible the internal SH2 BIOS roms differ


#include "emu.h"
#include "cpu/m68000/m68000.h"

#include "mega32x.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class segapm_state : public driver_device
{
public:
	segapm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_32x(*this,"sega32x")
		, m_screen(*this, "screen")
		, m_scan_timer(*this, "scan_timer")
	{ }

	void segapm(machine_config &config);

protected:
	static inline constexpr XTAL MASTER_CLOCK_NTSC = 53.693175_MHz_XTAL;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<sega_32x_device> m_32x;
	required_device<screen_device> m_screen;
	required_device<timer_device> m_scan_timer;

	bitmap_rgb32 m_bitmap;
	std::unique_ptr<u32[]> m_render_line;

	void segapm_map(address_map &map) ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer_cb);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


void segapm_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
}

uint32_t segapm_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}



void segapm_state::segapm_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();

	// A15100, cfr. shared/mega32x.cpp

	// $c0002x smartmedia i/f? (noisy)
	// $c00028 passed along to $a15128 (in SH-2 comms area)
	map(0xc00028, 0xc00029).lr16(NAME([] (offs_t offset) { return 0; }));

	// 512 KB of RAM (HM514260CJ7) rather than the usual 64 KB
	map(0xe00000, 0xe7ffff).mirror(0x180000).ram();

}

static INPUT_PORTS_START( segapm )
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(segapm_state::scanline_timer_cb)
{
	int scanline = param;

	if (scanline >= 0 && scanline < 224)
	{
		int x;
		m_32x->render_videobuffer_to_screenbuffer_helper(scanline);

		for (x = 0; x < 320; x++)
		{
			m_32x->render_videobuffer_to_screenbuffer(x, 0, m_render_line[x]);
			m_bitmap.pix(scanline, x) = m_render_line[x];
		}
	}

	// TODO: pushing at 0 doesn't work somehow ...
	if (scanline == 226)
		m_32x->screen_eof(0);

	bool int6_vbl = scanline == 224;

	// only 68k irq valid, pushes stuff to a non-existant VDP, left-over?
//	if (int6_vbl)
//		m_maincpu->set_input_line(6, HOLD_LINE);

	m_32x->interrupt_cb(scanline, int6_vbl);

	scanline ++;
	scanline %= m_screen->height();

	m_scan_timer->adjust(m_screen->time_until_pos(scanline), scanline);

}

void segapm_state::machine_start()
{
	m_render_line = std::make_unique<u32[]>(1280);
}

void segapm_state::machine_reset()
{
	m_32x->pause_cpu();

	m_scan_timer->adjust(m_screen->time_until_pos(m_screen->vpos() + 1), m_screen->vpos() + 1);
}

void segapm_state::segapm(machine_config &config)
{
	M68000(config, m_maincpu, 8000000); // ??
	m_maincpu->set_addrmap(AS_PROGRAM, &segapm_state::segapm_map);

	TIMER(config, m_scan_timer).configure_generic(FUNC(segapm_state::scanline_timer_cb));

	SEGA_32X_NTSC(config, m_32x, (MASTER_CLOCK_NTSC * 3) / 7, m_maincpu, m_scan_timer);
	m_32x->set_screen("screen");
	m_32x->add_route(0, "speaker", 1.00, 0);
	m_32x->add_route(1, "speaker", 1.00, 1);
	m_32x->set_32x_pal(false);
	m_32x->set_framerate(60);
	m_32x->set_total_scanlines(262);


	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK_NTSC / 8, 427, 0, 320, 262, 0, 224);
	m_screen->set_screen_update(FUNC(segapm_state::screen_update));

	SPEAKER(config, "speaker", 2).front();
}

ROM_START( segapm )
	// TODO: this is a cart, needs to be softlisted
	ROM_REGION16_BE( 0x400000, "gamecart", ROMREGION_ERASE00 ) /* 68000 Code */
	ROM_LOAD( "picture magic boot cart.bin", 0x00000, 0x80000, CRC(c9ab4e60) SHA1(9c4d4ab3e59c8acde86049a1ba3787aa03b549a3) ) // internal header is GOUSEI HENSYUU

	ROM_REGION32_BE( 0x400000, "gamecart_sh2", ROMREGION_ERASE00 ) /* Copy for the SH2 */
	ROM_COPY( "gamecart", 0, 0, 0x400000 )

	// baddump: picking up from 32X BIOS, unconfirmed
	ROM_REGION32_BE( 0x400000, "master", 0 )
	ROM_LOAD( "32x_m_bios.bin", 0x000000,  0x000800, BAD_DUMP CRC(dd9c46b8) SHA1(1e5b0b2441a4979b6966d942b20cc76c413b8c5e) )

	ROM_REGION32_BE( 0x400000, "slave", 0 )
	ROM_LOAD( "32x_s_bios.bin", 0x000000,  0x000400, BAD_DUMP CRC(bfda1fe5) SHA1(4103668c1bbd66c5e24558e73d4f3f92061a109a) )

	ROM_REGION16_BE( 0x400000, "32x_68k_bios", 0 )
	ROM_LOAD( "32x_g_bios.bin", 0x000000,  0x000100, BAD_DUMP CRC(5c12eae8) SHA1(dbebd76a448447cb6e524ac3cb0fd19fc065d944) )

	ROM_REGION16_BE( 0x400000, "maincpu", 0 )
	ROM_COPY( "gamecart",     0, 0, 0x400000 )
	ROM_COPY( "32x_68k_bios", 0, 0, 0x100)
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME         FLAGS
CONS( 1996, segapm, 0,      0,      segapm,  segapm, segapm_state, empty_init, "Sega",  "Picture Magic", MACHINE_NOT_WORKING )
