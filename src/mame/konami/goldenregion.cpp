// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    goldenregion.cpp: Konami 68000 based slots/pachislots
    Skeleton by R. Belmont

    Golden Region (GS761)
    CPU: 68000-16
    Sound: YMZ280B

    Konami ICs:
    K058143 + K056832 = tilemaps
    K053246 + K055673(x2) = sprites
    K055555 = priority blender
    K056766 = color DAC
    K053252(x2) = timing/interrupt controller

    IRQs 4 & 5 are valid, the rest are not

    TODO
    * How is the sprite ROM readback banked?  Passing the POST would be
      important to figuring out more of what's going on with this.
    * This uses a high-res mode similar to tasman.cpp and the GX tilemaps
      simply can't handle it.
    * Additional chip hookups, correct rendering, controls

***************************************************************************/

#include "emu.h"

#include "k053246_k053247_k055673.h"
#include "k054156_k054157_k056832.h"
#include "k055555.h"
#include "konami_helper.h"

#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/ymz280b.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class gs761_state : public driver_device
{
public:
	gs761_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_k056832(*this, "k056832"),
		m_k055673(*this, "k055673"),
		m_k055555(*this, "k055555"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ymz(*this, "ymz"),
		m_vbl_scanline(240)
	{ }

	void gs761(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual void tilemap_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int i);

	required_device<cpu_device> m_maincpu;
	required_device<k056832_device> m_k056832;
	required_device<k055673_device> m_k055673;
	required_device<k055555_device> m_k055555;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ymz280b_device> m_ymz;

	int m_vbl_scanline;
	u8 m_control;
	u16 m_control2;

	void control_w(u8 data);
	u16 control2_r();
	void control2_w(u16 data);

	u16 K056832_rom_r(offs_t offset);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void fill_backcolor(bitmap_rgb32 &bitmap, const rectangle &cliprect, int pen_idx, int mode);

	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void gs761_map(address_map &map) ATTR_COLD;

	static constexpr int NUM_LAYERS = 4;
};

void gs761_state::video_start()
{
	m_k056832->set_layer_offs(0, -4, -8);  // title on title screen
	m_k056832->set_layer_offs(1, 12, 8);    // konami logo on title screen
	m_k056832->set_layer_offs(2, 6, -8);
	m_k056832->set_layer_offs(3, 6, -8);
	m_vbl_scanline = 240;
	m_control = m_control2 = 0;
}

void gs761_state::control_w(u8 data)
{
	if ((!(data & 4)) && (m_control & 4))
	{
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
	}

	if ((!(data & 2)) && (m_control & 2))
	{
		m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
	}

	m_control = data;
}

u16 gs761_state::control2_r()
{
	return m_control2;
}

void gs761_state::control2_w(u16 data)
{
	// bit 13 = 56832 VROM readback chip select
	// bit 12 = 56832 VROM readback offset
	m_control2 = data;
}

u16 gs761_state::K056832_rom_r(offs_t offset)
{
	uint16_t addr = offset<<1;
	if (m_control2 & 0x1000)
	{
		addr += 2;
	}

	return m_k056832->piratesh_rom_r(addr);
}

TIMER_DEVICE_CALLBACK_MEMBER(gs761_state::scanline)
{
	int scanline = param;

	if ((scanline == m_vbl_scanline) && (m_control & 0x4))
	{
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);

	}

	if ((scanline == m_vbl_scanline+15) && (m_control & 0x2))
	{
		m_maincpu->set_input_line(M68K_IRQ_5, ASSERT_LINE);
	}
}

K056832_CB_MEMBER(gs761_state::tile_callback)
{
}

K053246_CB_MEMBER(gs761_state::sprite_callback)
{
}

// modified from version in mame/video/k054338.cpp
void gs761_state::fill_backcolor(bitmap_rgb32 &bitmap, const rectangle &cliprect, int pen_idx, int mode)
{
	if ((mode & 0x02) == 0) // solid fill
	{
		bitmap.fill(pen_idx, cliprect);
	}
	else
	{
		uint32_t *dst_ptr = &bitmap.pix(cliprect.min_y);
		int const dst_pitch = bitmap.rowpixels();
		pen_t const *const paldata = m_palette->pens();

		if ((mode & 0x01) == 0) // vertical gradient fill
		{
			pen_idx += cliprect.min_y;
			for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				for(int x = cliprect.min_x; x <= cliprect.max_x; x++)
				{
					dst_ptr[x] = paldata[pen_idx];
				}

				pen_idx++;
				dst_ptr += dst_pitch;
			}
		}
		else    // horizontal gradient fill
		{
			pen_idx += cliprect.min_x;
			dst_ptr += cliprect.min_x;
			for (int y = cliprect.min_y; y<= cliprect.max_y; y++)
			{
				for(int x = cliprect.min_x; x <= cliprect.max_x; x++)
				{
					dst_ptr[x] = paldata[pen_idx];
				}
				dst_ptr += dst_pitch;
			}
		}
	}
}

void gs761_state::tilemap_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int i)
{
	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer, 0, 1 << i);
}

uint32_t gs761_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const int order[4] = { 3, 2, 1, 0 };
	//int enables = m_k055555->K055555_read_register(K55_INPUT_ENABLES);

	screen.priority().fill(0, cliprect);

	fill_backcolor(bitmap, cliprect, (m_k055555->K055555_read_register(0) << 9), m_k055555->K055555_read_register(1));

	for (int i = 0; i < NUM_LAYERS; i++)
	{
		int layer = order[i];

		if (1) //enables & (K55_INP_VRAM_A << layer))
		{
			tilemap_draw(screen, bitmap, cliprect, layer, 1 << i);
		}
	}

	return 0;
}

void gs761_state::gs761_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x080000, 0x08ffff).ram();                          // work RAM
	map(0x090000, 0x093fff).ram().share("nvram");           // backup RAM
	map(0x100000, 0x107fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x183fff).rw(m_k055673, FUNC(k055673_device::k053247_word_r), FUNC(k055673_device::k053247_word_w));  // sprite VRAM
	map(0x280000, 0x28ffff).r(FUNC(gs761_state::K056832_rom_r));                                                        // VRAM ROM
	map(0x300000, 0x300007).w(m_k055673, FUNC(k055673_device::k053246_w));
	map(0x300060, 0x30006f).r(m_k055673, FUNC(k055673_device::k055673_gr_rom_word_r));
	map(0x308000, 0x30803f).rw(m_k056832, FUNC(k056832_device::word_r), FUNC(k056832_device::word_w));
	map(0x318000, 0x31805f).w(m_k055555, FUNC(k055555_device::K055555_word_w));
	map(0x320010, 0x32001f).w(m_k055673, FUNC(k055673_device::k055673_reg_word_w));
	map(0x380000, 0x380001).w(FUNC(gs761_state::control_w));
	map(0x398000, 0x398001).rw(FUNC(gs761_state::control2_r), FUNC(gs761_state::control2_w));
	// TODO: 56832 VRAM is actually 400000-41ffff, but that doesn't show anything so we hack this
	map(0x400000, 0x407fff).ram();
	map(0x408000, 0x41ffff).rw(m_k056832, FUNC(k056832_device::unpaged_ram_word_r), FUNC(k056832_device::unpaged_ram_word_w));
}

static INPUT_PORTS_START( gs761 )
INPUT_PORTS_END

void gs761_state::machine_start()
{
}

void gs761_state::machine_reset()
{
}

void gs761_state::gs761(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(33'868'800)/2);    // 33.8688 MHz crystal verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &gs761_state::gs761_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(gs761_state::scanline), "screen", 0, 1);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59.62);  /* verified on pcb */
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0, 511-1, 0, 255-1);
	m_screen->set_screen_update(FUNC(gs761_state::screen_update));

	PALETTE(config, "palette").set_format(palette_device::xBGR_888, 8192).enable_shadows();

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(gs761_state::tile_callback));
	m_k056832->set_config(K056832_BPP_4dj, 1, 0);
	m_k056832->set_palette(m_palette);

	K055673(config, m_k055673, 0);
	m_k055673->set_sprite_callback(FUNC(gs761_state::sprite_callback));
	m_k055673->set_config(K055673_LAYOUT_PS, -48 + 1, -23);
	m_k055673->set_palette(m_palette);

	K055555(config, m_k055555, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YMZ280B(config, m_ymz, XTAL(33'868'800)/2); // 33.8688 MHz xtal verified on PCB
	m_ymz->add_route(0, "lspeaker", 0.75);
	m_ymz->add_route(1, "rspeaker", 0.75);
}

ROM_START( glregion )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* main program */
	ROM_LOAD16_WORD_SWAP("761.b01.34r", 0x000000, 0x080000, CRC(55c18c13) SHA1(12a6167f5a3a03869d20c496478273652cef03c4))

	ROM_REGION( 0x100000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD("761.a05.25ac", 0x000000, 0x080000, CRC(f57ff5b7) SHA1(b6009f79bc34b3fff1a3b13648d5e0dc26c7ca1c))
	ROM_LOAD("761.a04.25ab", 0x080000, 0x080000, CRC(7f921ec5) SHA1(d72a10c5671b0900d4f70cd64555c486f426bd16))

	ROM_REGION(0x200000, "k055673", 0)  /* sprites */
	ROM_LOAD32_WORD("761.a02.34v", 0x000000, 0x080000, CRC(f23d16e2) SHA1(16c4766ba0b225d132900877ba63ba908c540e68))
	ROM_LOAD32_WORD("761.a03.34w", 0x000002, 0x080000, CRC(86d7cc81) SHA1(b693f57b6cb1ae6f0fc9a670b07f18159f4f6d71))

	ROM_REGION( 0x100000, "ymz", 0 )    /* samples */
	ROM_LOAD("761.a06.4n", 0x000000, 0x080000, CRC(aeac1f65) SHA1(8d572ef2b31a916aa8a5037c61891d3944a08f53))
	ROM_LOAD("761.a07.4p", 0x080000, 0x080000, CRC(eb5b77f4) SHA1(6009796e6589f9abb7127d233e3ab97743e2ffe7))
ROM_END

} // Anonymous namespace

GAME( 1998, glregion,  0, gs761,  gs761,  gs761_state, empty_init, ROT0, "Konami", "Golden Region", MACHINE_NOT_WORKING )
