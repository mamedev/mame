// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    Xexex  (c) 1991 Konami - GX067


Xexex
Konami 1991

PCB Layout
----------
GX067 PWB352898B
|--------------------------------------------------------|
|MB3722   067B07.1E                         067B14.1N    |
| 054544  067B06.3E  84256                  067B13.2N    |
|        067JAA05.4E                   |------| |------| |
|CN5               |------|            |053246A |053247A |
| YM2151   8464    |054539|            |      | |      | |
| 054744           |      |            |      | |      | |
|          Z80E    |      |            |      | |      | |
| 051550           |------|            |------| |------| |
|J                                                       |
|A 054573  |------| 2018                 5168(X10)       |
|M 054573  |054338| 2018                                 |
|M 054573  |      | 2018     |------|  |------| |------| |
|A 054574  |      |          |053251|  |054157| |054156| |
|          |------|          |      |  |      | |      | |
|  053252         067B04.13F |------|  |      | |      | |
|           067B03.13D      2018       |      | |      | |
|    054743  84256   84256  2018       |------| |------| |
| 18.432MHz      067JAA02.16F |------|                   |
| 32MHz     067JAA01.16D      |053250|        067B12.17N |
|            |------------|   |      |                   |
|TEST_SW     |   68000    |   |------|        067B11.19N |
|005273(X6)  |            |                              |
|  ER5911.19B|------------|                   067B10.20N |
|                                                        |
|                  067B08.22F                 067B09.22N |
|--------------------------------------------------------|
Notes:
      68000  - Clock 16.000MHz [32/2]
      Z80E   - Clock 8.000MHz [32/4]
      YM2151 - Clock 4.000MHz [32/8]
      2018   - Motorola MCM2018 2kx8 SRAM (DIP24)
      84256  - Fujitsu MB84256 32kx8 SRAM (DIP28)
      5168   - Sharp LH5168 8kx8 SRAM (DIP28)
      ER5911 - EEPROM (128 bytes)
      CN5    - 4 pin connector for stereo sound output
      067*   - EPROM/mask ROM
      MB3722 - Power AMP IC

      Custom Chips
      ------------
      053250  - Road generator
      053251  - Priority encoder
      053252  - Timing/Interrupt controller. Clock input 32MHz
      054157  \
      054156  / Tilemap generators
      053246A \
      053247A / Sprite generators
      054539  - 8-Channel ADPCM sound generator. Clock input 18.432MHz. Clock outputs 18.432/4 & 18.432/8
      054573  - Video DAC (one for each R,G,B video signal)
      054574  - Possibly RGB mixer/DAC/filter? (connected to 054573)
      054338  - Color mixer for special effects/alpha blending etc (connected to 054573 & 054574 and 2018 RAM)
      051550  - EMI filter for credit/coin counter
      005273  - Resistor array for player 3 & player 4 controls (PL3/4 connectors not populated)
      054544  - Audio DAC/filter
      054744  - PAL16L8
      054743  - PAL20L10

      Sync Measurements
      -----------------
      HSync - 15.3670kHz
      VSync - 54.0657Hz


****************************************************************************

Change Log
----------

(ATXXXX03)

Hooked up missing memory handler, emulated object DMA, revised IRQ,
rewrote the K053250(LVC) effect generator, ported tilemaps to use the
K056832 emulation(the K054157 is a complete subset of the K056832),
corrected a few K054539 PCM chip misbehaviors, etc.


The following bugs appear to be fixed:

General:

- game doesn't slow down like the arcade
    IRQ 5 is the "OBJDMA end interrupt" and shouldn't be triggered
    if DMA didn't complete within the frame.

    * game speed may not be 100% correct but close to that on the
    Gamest video especially in stage 6. Xexex is 384x256 which suggests
    an 8Mhz horizontal dotclock and DMA delay can range up to 32.0us(clear)
    + 256.0us(transfer). Increase XE_DMADELAY if emulation runs faster
    than the original or use cheat to overclock CPU 0 if you prefer faster
    gameplay.

- sprite lag, dithering, flicking (DMA)
- line effects go out of sync (K053250 also does DMA)
- inconsistent reverb (maths bug)
- lasers don't change color (IRQ masking)
- xexex057gre_1 (delayed sfx, missing speech, Xexexj only: random 1-up note)
- xexex057gre_2 (reversed stereo)
- xexex065gre (coin up problems, IRQ order)

- L1: xexex067gre (tilemap boundary), misaligned bosses (swapXY)
- L2: xexex061gre (K054157 offset)
- L4: half the foreground missing (LVC no-wraparound)
- L5: poly-face boss missing (coordinate masking)
- L6: sticky galaxies (LVC scroll bug)
- L7: misaligned ship patches (swapXY)


Unresolved Issues:

- random 1-up notes still pop up in the world version (filtered temporarily)
- mono/stereo softdip has no effect (xexex057gre_3, external mixing?)
- K053250 shows a one-frame glitch at stage 1 boss (DMA timing?)
- stage 3 intro missing alpha effect (known K054338 deficiency)
- the stage 4 boss(tentacles) sometimes appears darker (palette update timing?)
- the furthest layer in stage 5 shakes when scrolling up or down (needs verification)
- Elaine's end-game graphics has wrong masking effect (known non-zoomed pdrawgfx issue)
- intro screen at wrong alpha effect; it has K054157 flag-per-tile for alpha blending?
- Tilemap scroll is wrong at cocktail mode
- Music fadeout isn't working
reference(xexexj) : https://www.youtube.com/watch?v=TegjBEvvGxI

***************************************************************************/

#include "emu.h"

#include "k053246_k053247_k055673.h"
#include "k053250.h"
#include "k053251.h"
#include "k054156_k054157_k056832.h"
#include "k054338.h"
#include "konamipt.h"
#include "konami_helper.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/k053252.h"
#include "machine/k054321.h"
#include "machine/timer.h"
#include "sound/flt_vol.h"
#include "sound/k054539.h"
#include "sound/ymopm.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define XE_DEBUG      0
#define XE_SKIPIDLE   1
#define XE_DMADELAY   attotime::from_usec(256)

class xexex_state : public driver_device
{
public:
	xexex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_workram(*this, "workram")
		, m_spriteram(*this, "spriteram")
		, m_z80bank(*this, "z80bank")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_k054539(*this, "k054539")
		, m_filter_l(*this, "filter%u_l", 1)
		, m_filter_r(*this, "filter%u_r", 1)
		, m_k056832(*this, "k056832")
		, m_k053246(*this, "k053246")
		, m_k053250(*this, "k053250")
		, m_k053251(*this, "k053251")
		, m_k053252(*this, "k053252")
		, m_k054338(*this, "k054338")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_k054321(*this, "k054321")
	{
	}

	void xexex(machine_config &config);

	void init_xexex();

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_workram;
	required_shared_ptr<uint16_t> m_spriteram;

	/* memory regions */
	required_memory_bank m_z80bank;

	/* video-related */
	int        m_layer_colorbase[4]{};
	int        m_sprite_colorbase = 0;
	int        m_layerpri[4]{};
	int        m_cur_alpha = 0;

	/* misc */
	uint16_t     m_cur_control2 = 0;
	int32_t      m_strip_0x1a = 0;
	int        m_suspension_active = 0;
	int        m_resume_trigger = 0;
	emu_timer  *m_dmadelay_timer = nullptr;
	int        m_frame = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054539_device> m_k054539;
	required_device_array<filter_volume_device, 2> m_filter_l;
	required_device_array<filter_volume_device, 2> m_filter_r;
	required_device<k056832_device> m_k056832;
	required_device<k053247_device> m_k053246;
	required_device<k053250_device> m_k053250;
	required_device<k053251_device> m_k053251;
	required_device<k053252_device> m_k053252;
	required_device<k054338_device> m_k054338;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<k054321_device> m_k054321;

	uint16_t spriteram_mirror_r(offs_t offset);
	void spriteram_mirror_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t xexex_waitskip_r();
	uint16_t control2_r();
	void control2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_irq_w(uint16_t data);
	void sound_bankswitch_w(uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_xexex(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(dmaend_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(xexex_interrupt);
	void xexex_postload();
	void xexex_objdma(int limiter);
	void parse_control2();
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
	K054539_CB_MEMBER(ym_set_mixing);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


K053246_CB_MEMBER(xexex_state::sprite_callback)
{
	// Xexex doesn't seem to use bit8 and 9 as effect selectors so this should be safe.
	// (pdrawgfx() still needs change to fix Elaine's end-game graphics)
	int pri = (*color & 0x3e0) >> 4;

	if (pri <= m_layerpri[3])
		*priority_mask = 0;
	else if (pri > m_layerpri[3] && pri <= m_layerpri[2])
		*priority_mask = 0xff00;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority_mask = 0xff00 | 0xf0f0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc;
	else
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc | 0xaaaa;

	*color = m_sprite_colorbase | (*color & 0x001f);
}

K056832_CB_MEMBER(xexex_state::tile_callback)
{
	/*
	    Color format
	    xxxx ---- Color
	    ---- -x-- Alpha blend enable
	    ---- --x- Used, Unknown
	    Everything else : unknown
	*/
	*priority = *color & 1; // alpha flag
	*color = m_layer_colorbase[layer] | (*color >> 2 & 0x0f);
}

void xexex_state::video_start()
{
	assert(m_screen->format() == BITMAP_FORMAT_RGB32);

	m_cur_alpha = 0;

	// Xexex has relative plane offsets of -2,2,4,6 vs. -2,0,2,3 in MW and GX.
	m_k056832->set_layer_offs(0, -2, 16);
	m_k056832->set_layer_offs(1,  2, 16);
	m_k056832->set_layer_offs(2,  4, 16);
	m_k056832->set_layer_offs(3,  6, 16);
}

uint32_t xexex_state::screen_update_xexex(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const int K053251_CI[4] = { k053251_device::CI1, k053251_device::CI2, k053251_device::CI3, k053251_device::CI4 };
	int layer[4];
	int bg_colorbase, new_colorbase, plane, alpha;

	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI0);
	bg_colorbase = m_k053251->get_palette_index(k053251_device::CI1);
	m_layer_colorbase[0] = 0x70;

	for (plane = 1; plane < 4; plane++)
	{
		new_colorbase = m_k053251->get_palette_index(K053251_CI[plane]);
		if (m_layer_colorbase[plane] != new_colorbase)
		{
			m_layer_colorbase[plane] = new_colorbase;
			m_k056832->mark_plane_dirty( plane);
		}
	}

	layer[0] = 1;
	m_layerpri[0] = m_k053251->get_priority(k053251_device::CI2);
	layer[1] = 2;
	m_layerpri[1] = m_k053251->get_priority(k053251_device::CI3);
	layer[2] = 3;
	m_layerpri[2] = m_k053251->get_priority(k053251_device::CI4);
	layer[3] = -1;
	m_layerpri[3] = m_k053251->get_priority(k053251_device::CI1);

	konami_sortlayers4(layer, m_layerpri);

	m_k054338->update_all_shadows(0, *m_palette);
	m_k054338->fill_solid_bg(bitmap, cliprect);

	screen.priority().fill(0, cliprect);

	for (plane = 0; plane < 4; plane++)
	{
		if (layer[plane] < 0)
		{
			m_k053250->draw(bitmap, cliprect, bg_colorbase, 0, screen.priority(), 1 << plane);
		}
		else if (!m_cur_alpha || layer[plane] != 1)
		{
			m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[plane], TILEMAP_DRAW_ALL_CATEGORIES, 1 << plane);
		}
		else
		{
			m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[plane], TILEMAP_DRAW_CATEGORY(0), 1 << plane);
		}
	}

	m_k053246->k053247_sprites_draw( bitmap, cliprect);

	if (m_cur_alpha)
	{
		alpha = m_k054338->set_alpha_level(1);

		if (alpha > 0)
		{
			m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_ALPHA(alpha) | TILEMAP_DRAW_CATEGORY(1), 0);
		}
	}

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_ALL_CATEGORIES, 0);
	return 0;
}

#if 0 // (for reference; do not remove)

/* the interface with the 053247 is weird. The chip can address only 0x1000 bytes */
/* of RAM, but they put 0x8000 there. The CPU can access them all. Address lines */
/* A1, A5 and A6 don't go to the 053247. */
uint16_t xexex_state::k053247_scattered_word_r(offs_t offset, uint16_t mem_mask)
{
	if (offset & 0x0031)
		return m_spriteram[offset];
	else
	{
		offset = ((offset & 0x000e) >> 1) | ((offset & 0x3fc0) >> 3);
		return k053247_word_r(m_k053246, offset, mem_mask);
	}
}

void xexex_state::k053247_scattered_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset & 0x0031)
		COMBINE_DATA(m_spriteram + offset);
	else
	{
		offset = ((offset & 0x000e) >> 1) | ((offset & 0x3fc0) >> 3);
		k053247_word_w(m_k053246, offset, data, mem_mask);
	}
}

#endif


void xexex_state::xexex_objdma( int limiter )
{
	int counter, num_inactive;
	uint16_t *src, *dst;

	counter = m_frame;
	m_frame = m_screen->frame_number();
	if (limiter && counter == m_frame)
		return; // make sure we only do DMA transfer once per frame

	m_k053246->k053247_get_ram( &dst);
	counter = m_k053246->k053247_get_dy();
	src = m_spriteram;
	num_inactive = counter = 256;

	do
	{
		if (*src & 0x8000)
		{
			dst[0] = src[0x0];  dst[1] = src[0x2];
			dst[2] = src[0x4];  dst[3] = src[0x6];
			dst[4] = src[0x8];  dst[5] = src[0xa];
			dst[6] = src[0xc];  dst[7] = src[0xe];
			dst += 8;
			num_inactive--;
		}
		src += 0x40;
	}
	while (--counter);

	if (num_inactive) do { *dst = 0; dst += 8; } while (--num_inactive);
}

uint16_t xexex_state::spriteram_mirror_r(offs_t offset)
{
	return m_spriteram[offset];
}

void xexex_state::spriteram_mirror_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_spriteram + offset);
}

uint16_t xexex_state::xexex_waitskip_r()
{
	if (m_maincpu->pc() == 0x1158)
	{
		m_maincpu->spin_until_trigger(m_resume_trigger);
		m_suspension_active = 1;
	}

	return m_workram[0x14/2];
}


void xexex_state::parse_control2(  )
{
	/* bit 0  is data */
	/* bit 1  is cs (active low) */
	/* bit 2  is clock (active high) */
	/* bit 5  is enable irq 6 */
	/* bit 6  is enable irq 5 */
	/* bit 11 is watchdog */
	ioport("EEPROMOUT")->write(m_cur_control2, 0xff);

	/* bit 8 = enable sprite ROM reading */
	m_k053246->k053246_set_objcha_line( (m_cur_control2 & 0x0100) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 9 = disable alpha channel on K054157 plane 0 (under investigation) */
	m_cur_alpha = !(m_cur_control2 & 0x200);
}

uint16_t xexex_state::control2_r()
{
	return m_cur_control2;
}

void xexex_state::control2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_cur_control2);
	parse_control2();
}

void xexex_state::sound_irq_w(uint16_t data)
{
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

void xexex_state::sound_bankswitch_w(uint8_t data)
{
	m_z80bank->set_entry(data & 0x07);
}

K054539_CB_MEMBER(xexex_state::ym_set_mixing)
{
	for (int out = 0; out < 2; out++)
	{
		m_filter_l[out]->set_gain((71.0 * left) / 55.0);
		m_filter_r[out]->set_gain((71.0 * right) / 55.0);
	}
}

TIMER_CALLBACK_MEMBER(xexex_state::dmaend_callback)
{
	if (m_cur_control2 & 0x0040)
	{
		// foul-proof (CPU0 could be deactivated while we wait)
		if (m_suspension_active)
		{
			m_suspension_active = 0;
			machine().scheduler().trigger(m_resume_trigger);
		}

		// IRQ 5 is the "object DMA end interrupt" and shouldn't be triggered
		// if object data isn't ready for DMA within the frame.
		m_maincpu->set_input_line(5, HOLD_LINE);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(xexex_state::xexex_interrupt)
{
	int scanline = param;

	if (m_suspension_active)
	{
		m_suspension_active = 0;
		machine().scheduler().trigger(m_resume_trigger);
	}

	if(scanline == 0)
	{
		// IRQ 6 is for test mode only
			if (m_cur_control2 & 0x0020)
				m_maincpu->set_input_line(6, HOLD_LINE);
	}

	/* TODO: vblank is at 256! (enable CCU then have fun in fixing offsetted layers) */
	if(scanline == 128)
	{
		if (m_k053246->k053246_is_irq_enabled())
		{
			// OBJDMA starts at the beginning of V-blank
			xexex_objdma(0);

			// schedule DMA end interrupt
			m_dmadelay_timer->adjust(XE_DMADELAY);
		}

		// IRQ 4 is the V-blank interrupt. It controls color, sound and
		// vital game logics that shouldn't be interfered by frame-drop.
		if (m_cur_control2 & 0x0800)
			m_maincpu->set_input_line(4, HOLD_LINE);
	}
}


void xexex_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x08ffff).ram().share("workram");         // work RAM

#if XE_SKIPIDLE
	map(0x080014, 0x080015).r(FUNC(xexex_state::xexex_waitskip_r));              // helps sound CPU by giving back control as early as possible
#endif

	map(0x090000, 0x097fff).ram().share("spriteram");           // K053247 sprite RAM
	map(0x098000, 0x09ffff).rw(FUNC(xexex_state::spriteram_mirror_r), FUNC(xexex_state::spriteram_mirror_w));   // K053247 sprite RAM mirror read
	map(0x0c0000, 0x0c003f).w(m_k056832, FUNC(k056832_device::word_w));              // VACSET (K054157)
	map(0x0c2000, 0x0c2007).w(m_k053246, FUNC(k053247_device::k053246_w));              // OBJSET1
	map(0x0c4000, 0x0c4001).r(m_k053246, FUNC(k053247_device::k053246_r));               // Passthrough to sprite roms
	map(0x0c6000, 0x0c7fff).rw(m_k053250, FUNC(k053250_device::ram_r), FUNC(k053250_device::ram_w));    // K053250 "road" RAM
	map(0x0c8000, 0x0c800f).rw(m_k053250, FUNC(k053250_device::reg_r), FUNC(k053250_device::reg_w));
	map(0x0ca000, 0x0ca01f).w(m_k054338, FUNC(k054338_device::word_w));              // CLTC
	map(0x0cc000, 0x0cc01f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);               // priority encoder
//  map(0x0d0000, 0x0d001f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write)).umask16(0x00ff);                // CCU
	map(0x0d4000, 0x0d4001).w(FUNC(xexex_state::sound_irq_w));
	map(0x0d6000, 0x0d601f).m(m_k054321, FUNC(k054321_device::main_map)).umask16(0x00ff);
	map(0x0d8000, 0x0d8007).w(m_k056832, FUNC(k056832_device::b_word_w));                // VSCCS regs
	map(0x0da000, 0x0da001).portr("P1");
	map(0x0da002, 0x0da003).portr("P2");
	map(0x0dc000, 0x0dc001).portr("SYSTEM");
	map(0x0dc002, 0x0dc003).portr("EEPROM");
	map(0x0de000, 0x0de001).rw(FUNC(xexex_state::control2_r), FUNC(xexex_state::control2_w));
	map(0x100000, 0x17ffff).rom();
	map(0x180000, 0x181fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));
	map(0x182000, 0x183fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));
	map(0x190000, 0x191fff).r(m_k056832, FUNC(k056832_device::rom_word_r));       // Passthrough to tile roms
	map(0x1a0000, 0x1a1fff).r(m_k053250, FUNC(k053250_device::rom_r));
	map(0x1b0000, 0x1b1fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

#if XE_DEBUG
	map(0x0c0000, 0x0c003f).r(m_k056832, FUNC(k056832_device::word_r));
	map(0x0c2000, 0x0c2007).r(m_k053246, FUNC(k053247_device::k053246_read_register));
	map(0x0ca000, 0x0ca01f).r(m_k054338, FUNC(k054338_device::register_r));
	map(0x0cc000, 0x0cc01f).r(m_k053251, FUNC(k053251_device::read)).umask16(0x00ff);
	map(0x0d8000, 0x0d8007).r(m_k056832, FUNC(k056832_device::b_word_r));
#endif

}


void xexex_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("z80bank");
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe22f).rw(m_k054539, FUNC(k054539_device::read), FUNC(k054539_device::write));
	map(0xec00, 0xec01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xf000, 0xf003).m(m_k054321, FUNC(k054321_device::sound_map));
	map(0xf800, 0xf800).w(FUNC(xexex_state::sound_bankswitch_w));
}


static INPUT_PORTS_START( xexex )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI16_LSB(1, IPT_UNKNOWN, IPT_START1 )

	PORT_START("P2")
	KONAMI16_LSB(2, IPT_UNKNOWN, IPT_START2 )

	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)
INPUT_PORTS_END



void xexex_state::xexex_postload()
{
	parse_control2();
}

void xexex_state::machine_start()
{
	m_z80bank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);
	m_z80bank->set_entry(0);

	save_item(NAME(m_cur_alpha));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));

	save_item(NAME(m_suspension_active));
	save_item(NAME(m_frame));

	save_item(NAME(m_cur_control2));
	machine().save().register_postload(save_prepost_delegate(FUNC(xexex_state::xexex_postload), this));

	m_dmadelay_timer = timer_alloc(FUNC(xexex_state::dmaend_callback), this);
}

void xexex_state::machine_reset()
{
	int i;

	for (i = 0; i < 4; i++)
	{
		m_layerpri[i] = 0;
		m_layer_colorbase[i] = 0;
	}

	m_sprite_colorbase = 0;

	m_cur_control2 = 0;
	m_suspension_active = 0;
	m_resume_trigger = 1000;
	m_frame = -1;
}

void xexex_state::xexex(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2); // 16MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &xexex_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(xexex_state::xexex_interrupt), "screen", 0, 1);

	Z80(config, m_audiocpu, XTAL(32'000'000)/4); // Z80E 8Mhz
	m_audiocpu->set_addrmap(AS_PROGRAM, &xexex_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(1920));

	EEPROM_ER5911_8BIT(config, "eeprom");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
//  m_screen->set_refresh_hz(XTAL(32'000'000)/4/512/288);
	m_screen->set_raw(XTAL(32'000'000)/4, 384+33+40+55, 0, 383, 256+12+6+14, 0, 255); // 8Mhz horizontal dotclock
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(40, 40+384-1, 0, 0+256-1);
	m_screen->set_screen_update(FUNC(xexex_state::screen_update_xexex));

	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 2048);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(xexex_state::tile_callback));
	m_k056832->set_config(K056832_BPP_4, 1, 0);
	m_k056832->set_palette(m_palette);

	K053246(config, m_k053246, 0);
	m_k053246->set_sprite_callback(FUNC(xexex_state::sprite_callback));
	m_k053246->set_config(NORMAL_PLANE_ORDER, -48, 32);
	m_k053246->set_palette(m_palette);

	K053250(config, m_k053250, 0, m_palette, m_screen, -5, -16);

	K053251(config, m_k053251, 0);

	K053252(config, m_k053252, XTAL(32'000'000)/4);

	K054338(config, m_k054338, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	K054321(config, m_k054321, "lspeaker", "rspeaker");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(32'000'000)/8)); // 4MHz
	ymsnd.add_route(0, "filter1_l", 0.2);
	ymsnd.add_route(0, "filter1_r", 0.2);
	ymsnd.add_route(1, "filter2_l", 0.2);
	ymsnd.add_route(1, "filter2_r", 0.2);

	K054539(config, m_k054539, XTAL(18'432'000));
	m_k054539->set_analog_callback(FUNC(xexex_state::ym_set_mixing));
	m_k054539->add_route(0, "lspeaker", 0.4);
	m_k054539->add_route(0, "rspeaker", 0.4);
	m_k054539->add_route(1, "lspeaker", 0.4);
	m_k054539->add_route(1, "rspeaker", 0.4);

	FILTER_VOLUME(config, "filter1_l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "filter1_r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "filter2_l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "filter2_r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}


ROM_START( xexex ) /* Europe, Version AA */
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "067eaa01.16d", 0x000000, 0x040000, CRC(3ebcb066) SHA1(83a20433d9fdcc8b8d7133991f9a8164dddb61f3) )
	ROM_LOAD16_BYTE( "067eaa02.16f", 0x000001, 0x040000, CRC(36ea7a48) SHA1(34f8046d7ecf5ea66c59c5bc0d7627942c28fd3b) )
	ROM_LOAD16_BYTE( "067b03.13d",   0x100000, 0x040000, CRC(97833086) SHA1(a564f7b1b52c774d78a59f4418c7ecccaf94ad41) )
	ROM_LOAD16_BYTE( "067b04.13f",   0x100001, 0x040000, CRC(26ec5dc8) SHA1(9da62683bfa8f16607cbea2d59a1446ec8588c5b) )

	ROM_REGION( 0x020000, "audiocpu", 0 )
	ROM_LOAD( "067eaa05.4e", 0x000000, 0x020000, CRC(0e33d6ec) SHA1(4dd68cb78c779e2d035e43fec35a7672ed1c259b) )

	ROM_REGION( 0x200000, "k056832", 0 )
	ROM_LOAD32_WORD( "067b14.1n",   0x000000, 0x100000, CRC(02a44bfa) SHA1(ad95df4dbf8842820ef20f54407870afb6d0e4a3) )
	ROM_LOAD32_WORD( "067b13.2n",   0x000002, 0x100000, CRC(633c8eb5) SHA1(a11f78003a1dffe2d8814d368155059719263082) )

	ROM_REGION( 0x400000, "k053246", 0 )
	ROM_LOAD64_WORD( "067b12.17n",  0x000000, 0x100000, CRC(08d611b0) SHA1(9cac60131e0411f173acd8ef3f206e5e58a7e5d2) )
	ROM_LOAD64_WORD( "067b11.19n",  0x000002, 0x100000, CRC(a26f7507) SHA1(6bf717cb9fcad59a2eafda967f14120b9ebbc8c5) )
	ROM_LOAD64_WORD( "067b10.20n",  0x000004, 0x100000, CRC(ee31db8d) SHA1(c41874fb8b401ea9cdd327ee6239b5925418cf7b) )
	ROM_LOAD64_WORD( "067b09.22n",  0x000006, 0x100000, CRC(88f072ef) SHA1(7ecc04dbcc29b715117e970cc96e11137a21b83a) )

	ROM_REGION( 0x080000, "k053250", 0 )
	ROM_LOAD( "067b08.22f",  0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, "k054539", 0 )
	ROM_LOAD( "067b06.3e",   0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067b07.1e",   0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.19b",  0x0000, 0x0080, CRC(155624cc) SHA1(457f921e3a5d053c53e4f1a44941eb0a1f22e1b2) )
ROM_END

ROM_START( orius ) /* USA, Version AA */
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "067uaa01.16d", 0x000000, 0x040000, CRC(f1263d3e) SHA1(c8a10b90e754ec7a72a23ac85b888f071ca40bb1) )
	ROM_LOAD16_BYTE( "067uaa02.16f", 0x000001, 0x040000, CRC(77709f64) SHA1(c26f09c9723facb89ab8aae2a036be4e6892d4bf) )
	ROM_LOAD16_BYTE( "067b03.13d",   0x100000, 0x040000, CRC(97833086) SHA1(a564f7b1b52c774d78a59f4418c7ecccaf94ad41) )
	ROM_LOAD16_BYTE( "067b04.13f",   0x100001, 0x040000, CRC(26ec5dc8) SHA1(9da62683bfa8f16607cbea2d59a1446ec8588c5b) )

	ROM_REGION( 0x020000, "audiocpu", 0 )
	ROM_LOAD( "067uaa05.4e", 0x000000, 0x020000, CRC(0e33d6ec) SHA1(4dd68cb78c779e2d035e43fec35a7672ed1c259b) )

	ROM_REGION( 0x200000, "k056832", 0 )
	ROM_LOAD32_WORD( "067b14.1n",   0x000000, 0x100000, CRC(02a44bfa) SHA1(ad95df4dbf8842820ef20f54407870afb6d0e4a3) )
	ROM_LOAD32_WORD( "067b13.2n",   0x000002, 0x100000, CRC(633c8eb5) SHA1(a11f78003a1dffe2d8814d368155059719263082) )

	ROM_REGION( 0x400000, "k053246", 0 )
	ROM_LOAD64_WORD( "067b12.17n",  0x000000, 0x100000, CRC(08d611b0) SHA1(9cac60131e0411f173acd8ef3f206e5e58a7e5d2) )
	ROM_LOAD64_WORD( "067b11.19n",  0x000002, 0x100000, CRC(a26f7507) SHA1(6bf717cb9fcad59a2eafda967f14120b9ebbc8c5) )
	ROM_LOAD64_WORD( "067b10.20n",  0x000004, 0x100000, CRC(ee31db8d) SHA1(c41874fb8b401ea9cdd327ee6239b5925418cf7b) )
	ROM_LOAD64_WORD( "067b09.22n",  0x000006, 0x100000, CRC(88f072ef) SHA1(7ecc04dbcc29b715117e970cc96e11137a21b83a) )

	ROM_REGION( 0x080000, "k053250", 0 )
	ROM_LOAD( "067b08.22f",  0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, "k054539", 0 )
	ROM_LOAD( "067b06.3e",   0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067b07.1e",   0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.19b",  0x0000, 0x0080, CRC(547ee4e4) SHA1(089601fcfa513f129d6e2587594b932d4a8fde18) ) //sldh
ROM_END

ROM_START( xexexa ) /* Asia, Version AA */
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "067aaa01.16d", 0x000000, 0x040000, CRC(cf557144) SHA1(4ce587580d953b88864652dd66485d49ca719ec5) )
	ROM_LOAD16_BYTE( "067aaa02.16f", 0x000001, 0x040000, CRC(b7b98d52) SHA1(ca2343bf37f779699b6782772e559ea5662c1742) )
	ROM_LOAD16_BYTE( "067b03.13d",   0x100000, 0x040000, CRC(97833086) SHA1(a564f7b1b52c774d78a59f4418c7ecccaf94ad41) )
	ROM_LOAD16_BYTE( "067b04.13f",   0x100001, 0x040000, CRC(26ec5dc8) SHA1(9da62683bfa8f16607cbea2d59a1446ec8588c5b) )

	ROM_REGION( 0x020000, "audiocpu", 0 )
	ROM_LOAD( "067eaa05.4e", 0x000000, 0x020000, CRC(0e33d6ec) SHA1(4dd68cb78c779e2d035e43fec35a7672ed1c259b) )

	ROM_REGION( 0x200000, "k056832", 0 )
	ROM_LOAD32_WORD( "067b14.1n",   0x000000, 0x100000, CRC(02a44bfa) SHA1(ad95df4dbf8842820ef20f54407870afb6d0e4a3) )
	ROM_LOAD32_WORD( "067b13.2n",   0x000002, 0x100000, CRC(633c8eb5) SHA1(a11f78003a1dffe2d8814d368155059719263082) )

	ROM_REGION( 0x400000, "k053246", 0 )
	ROM_LOAD64_WORD( "067b12.17n",  0x000000, 0x100000, CRC(08d611b0) SHA1(9cac60131e0411f173acd8ef3f206e5e58a7e5d2) )
	ROM_LOAD64_WORD( "067b11.19n",  0x000002, 0x100000, CRC(a26f7507) SHA1(6bf717cb9fcad59a2eafda967f14120b9ebbc8c5) )
	ROM_LOAD64_WORD( "067b10.20n",  0x000004, 0x100000, CRC(ee31db8d) SHA1(c41874fb8b401ea9cdd327ee6239b5925418cf7b) )
	ROM_LOAD64_WORD( "067b09.22n",  0x000006, 0x100000, CRC(88f072ef) SHA1(7ecc04dbcc29b715117e970cc96e11137a21b83a) )

	ROM_REGION( 0x080000, "k053250", 0 )
	ROM_LOAD( "067b08.22f",  0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, "k054539", 0 )
	ROM_LOAD( "067b06.3e",   0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067b07.1e",   0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.19b",  0x0000, 0x0080, CRC(051c14c6) SHA1(23addbaa2ce323c06551b343ca45dea4fd2b9eee) ) // sldh
ROM_END

ROM_START( xexexj ) /* Japan, Version AA */
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "067jaa01.16d", 0x000000, 0x040000, CRC(06e99784) SHA1(d53fe3724608992a6938c36aa2719dc545d6b89e) )
	ROM_LOAD16_BYTE( "067jaa02.16f", 0x000001, 0x040000, CRC(30ae5bc4) SHA1(60491e31eef64a9206d1372afa32d83c6c0968b3) )
	ROM_LOAD16_BYTE( "067b03.13d",   0x100000, 0x040000, CRC(97833086) SHA1(a564f7b1b52c774d78a59f4418c7ecccaf94ad41) )
	ROM_LOAD16_BYTE( "067b04.13f",   0x100001, 0x040000, CRC(26ec5dc8) SHA1(9da62683bfa8f16607cbea2d59a1446ec8588c5b) )

	ROM_REGION( 0x020000, "audiocpu", 0 )
	ROM_LOAD( "067jaa05.4e", 0x000000, 0x020000, CRC(2f4dd0a8) SHA1(bfa76c9c968f1beba648a2911510e3d666a8fe3a) )

	ROM_REGION( 0x200000, "k056832", 0 )
	ROM_LOAD32_WORD( "067b14.1n",   0x000000, 0x100000, CRC(02a44bfa) SHA1(ad95df4dbf8842820ef20f54407870afb6d0e4a3) )
	ROM_LOAD32_WORD( "067b13.2n",   0x000002, 0x100000, CRC(633c8eb5) SHA1(a11f78003a1dffe2d8814d368155059719263082) )

	ROM_REGION( 0x400000, "k053246", 0 )
	ROM_LOAD64_WORD( "067b12.17n",  0x000000, 0x100000, CRC(08d611b0) SHA1(9cac60131e0411f173acd8ef3f206e5e58a7e5d2) )
	ROM_LOAD64_WORD( "067b11.19n",  0x000002, 0x100000, CRC(a26f7507) SHA1(6bf717cb9fcad59a2eafda967f14120b9ebbc8c5) )
	ROM_LOAD64_WORD( "067b10.20n",  0x000004, 0x100000, CRC(ee31db8d) SHA1(c41874fb8b401ea9cdd327ee6239b5925418cf7b) )
	ROM_LOAD64_WORD( "067b09.22n",  0x000006, 0x100000, CRC(88f072ef) SHA1(7ecc04dbcc29b715117e970cc96e11137a21b83a) )

	ROM_REGION( 0x080000, "k053250", 0 )
	ROM_LOAD( "067b08.22f",  0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, "k054539", 0 )
	ROM_LOAD( "067b06.3e",   0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067b07.1e",   0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.19b",  0x0000, 0x0080, CRC(79a79c7b) SHA1(02eb235226949af0147d6d0fd2bd3d7a68083ae6) ) // sldh
ROM_END


void xexex_state::init_xexex()
{
	m_strip_0x1a = 0;

	if (!strcmp(machine().system().name, "xexex"))
	{
		// Invulnerability
//      *(uint16_t *)(memregion("maincpu")->base() + 0x648d4) = 0x4a79;
//      *(uint16_t *)(memregion("maincpu")->base() + 0x00008) = 0x5500;
		m_strip_0x1a = 1;
	}
}

} // anonymous namespace

GAME( 1991, xexex,  0,     xexex, xexex, xexex_state, init_xexex, ROT0, "Konami", "Xexex (ver EAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, orius,  xexex, xexex, xexex, xexex_state, init_xexex, ROT0, "Konami", "Orius (ver UAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, xexexa, xexex, xexex, xexex, xexex_state, init_xexex, ROT0, "Konami", "Xexex (ver AAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, xexexj, xexex, xexex, xexex, xexex_state, init_xexex, ROT0, "Konami", "Xexex (ver JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
