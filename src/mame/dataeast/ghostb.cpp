// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Stephane Humbert
/***************************************************************************

Gondomania                  (c) 1987 Data East USA
Makyou Senshi               (c) 1987 Data East Corporation
Garyo Retsuden              (c) 1987 Data East Corporation
The Real Ghostbusters (2p)  (c) 1987 Data East USA
The Real Ghostbusters (3p)  (c) 1987 Data East USA
Meikyuu Hunter G            (c) 1987 Data East Corporation

Meikyuu Hunter G was formerly known as Mazehunter. It's a Japan-only modified
version of Ghostbusters, due to licensing restrictions.

Emulation by Bryan McPhail, mish@tendril.co.uk

TODO:
- strangely coloured butterfly on Garyo Retsuden water levels!
- gondo 2nd coin doesn't work, probably due to hacked MCU ROM
- ghostb coinage dipswitch
- how does meikyuhbl circumvent the MCU? It won't boot in MAME if MCU is removed
- weird NMI issue in ghostb: Before starting stage 2, it waits for vblank, then
  enables NMI by writing to ghostb_bank_w, then stores the written value in RAM.
  It fails to initialize stage 2 properly if NMI happens right after enabling
  the NMI flip-flop (when it hasn't yet stored a copy the bank register value
  in RAM), so it appears that it expects 1 more opcode, see:

  88D5: LDA    $3803
  88D8: ANDA   #$08  ; vblank flag
  88DA: BNE    $88D5

  88DC: LDA    #$B7
  88DE: STA    $3840 ; ghostb_bank_w
  88E1: STA    <$68  ; expects NMI after this opcode

  Where does this delay come from? Is it a MAME 6809 timing bug with NMI edge
  detection? Or a brief TTL delay and it works by luck? Either way, it looks
  like a bug by Data East. Normally you'd store the local variable first,
  then write to the register.

***************************************************************************/

#include "emu.h"

#include "decbac06.h"
#include "deckarn.h"
#include "deco222.h"
#include "decrmc3.h"

#include "cpu/m6502/r65c02.h"
#include "cpu/m6809/hd6309.h"
#include "cpu/mcs51/i8051.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"

#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "multibyte.h"


namespace {

class garyoret_state : public driver_device
{
public:
	garyoret_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_spritegen(*this, "spritegen"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_nmigate(*this, "nmigate"),
		m_soundirq(*this, "soundirq"),
		m_soundlatch(*this, "soundlatch"),
		m_mainbank(*this, "mainbank"),
		m_videoram(*this, "videoram"),
		m_bg_ram(*this, "bg_ram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram16(*this, "spriteram16", 0x800, ENDIANNESS_BIG)
	{ }

	void garyoret(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void buffer_spriteram_w(int state);

	void bg_ram_w(offs_t offset, u8 data);
	u8 bg_ram_r(offs_t offset);
	void videoram_w(offs_t offset, u8 data);

	u8 i8751_hi_r();
	u8 i8751_lo_r();
	virtual void i8751_hi_w(u8 data);
	void i8751_lo_w(u8 data);

	u8 i8751_port0_r();
	void i8751_port0_w(u8 data);
	u8 i8751_port1_r();
	void i8751_port1_w(u8 data);
	void mcu_to_main_w(u8 data);

	void gondo_scroll_w(offs_t offset, u8 data);
	void gondo_bank_w(u8 data);
	void ghostb_bank_w(u8 data);
	void sound_w(u8 data);

	TIMER_CALLBACK_MEMBER(audiocpu_nmi_clear);
	TIMER_CALLBACK_MEMBER(mcu_irq_clear);
	TIMER_CALLBACK_MEMBER(nmigate_set) { m_nmigate->in_w<0>(param); }

	TILE_GET_INFO_MEMBER(get_gondo_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_gondo_tile_info);

	DECLARE_VIDEO_START(garyoret);

	u32 screen_update_garyoret(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void set_screen_raw_params(machine_config &config) ATTR_COLD;

	void garyoret_map(address_map &map) ATTR_COLD;
	void gondo_sound_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8751_device> m_mcu;
	required_device<deco_karnovsprites_device> m_spritegen;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<deco_rmc3_device> m_palette;
	required_device<input_merger_device> m_nmigate;
	required_device<input_merger_device> m_soundirq;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory regions
	required_memory_bank m_mainbank;

	// memory pointers
	required_shared_ptr<u8> m_videoram;
	optional_shared_ptr<u8> m_bg_ram;

	required_shared_ptr<u8> m_spriteram;
	memory_share_creator<u16> m_spriteram16;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fix_tilemap = nullptr;
	u8 m_scroll[4]{};
	u8 m_game_uses_priority = 0;

	// misc
	u8 m_bank_mask = 0;

	// MCU communication
	u8 m_i8751_p2 = 0;
	u8 m_i8751_port0 = 0;
	u8 m_i8751_port1 = 0;
	u16 m_i8751_return = 0;
	u16 m_i8751_value = 0;

	u8 m_bank_data = 0xff;
	bool m_secclr = false;
	bool m_buffer_strobe = false;

	emu_timer *m_nmi_timer = nullptr;
	emu_timer *m_6502_timer = nullptr;
	emu_timer *m_i8751_timer = nullptr;
};

// with rotary joystick
class gondo_state : public garyoret_state
{
public:
	gondo_state(const machine_config &mconfig, device_type type, const char *tag) :
		garyoret_state(mconfig, type, tag),
		m_analog_io(*this, "AN%u", 0U),
		m_in_io(*this, "IN%u", 0U)
	{ }

	void gondo(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	template <unsigned Which> u8 player_io_r(offs_t offset);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void colpri_cb(u32 &colour, u32 &pri_mask);

	void main_map(address_map &map) ATTR_COLD;

	required_ioport_array<2> m_analog_io;
	required_ioport_array<4> m_in_io;
};

// with PROM palette, BAC06 tilemap
class ghostb_state : public garyoret_state
{
public:
	ghostb_state(const machine_config &mconfig, device_type type, const char *tag) :
		garyoret_state(mconfig, type, tag),
		m_tilegen(*this, "tilegen")
	{ }

	void ghostb(machine_config &config) ATTR_COLD;
	void meikyuh(machine_config &config) ATTR_COLD;

	void init_meikyuhbl() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	virtual void i8751_hi_w(u8 data) override;

	TILE_GET_INFO_MEMBER(get_fix_tile_info);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// devices
	required_device<deco_bac06_device> m_tilegen;
};

/******************************************************************************/

// Only used by gondo, garyoret, ghostb, meikyuh
void garyoret_state::buffer_spriteram_w(int state)
{
	// rising edge
	if (!m_buffer_strobe && state)
	{
		// copy to a 16-bit region for the sprite chip
		for (int i = 0; i < 0x800/2 ; i++)
			m_spriteram16[i] = get_u16be(&m_spriteram[i * 2]);
	}

	m_buffer_strobe = bool(state);
}

void garyoret_state::bg_ram_w(offs_t offset, u8 data)
{
	m_bg_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

u8 garyoret_state::bg_ram_r(offs_t offset)
{
	return m_bg_ram[offset];
}


void garyoret_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset / 2);
}

void garyoret_state::gondo_scroll_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0x0:
		m_scroll[1] = data; // X LSB
		break;
	case 0x8:
		m_scroll[3] = data; // Y LSB
		break;
	case 0x10:
		m_scroll[0] = BIT(data, 0); // Bit 0: X MSB
		m_scroll[2] = BIT(data, 1); // Bit 1: Y MSB
		buffer_spriteram_w(BIT(data, 2));
		break;
	}
}

/******************************************************************************/

u32 ghostb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilegen->draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram16.target(), 0x400);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILE_GET_INFO_MEMBER(ghostb_state::get_fix_tile_info)
{
	const u32 offs = tile_index << 1;
	const u16 tile = get_u16be(&m_videoram[offs]);
	const u8 color = (tile & 0xc00) >> 10;

	tileinfo.set(0, tile & 0x3ff, color, 0);
}

void ghostb_state::video_start()
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ghostb_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fix_tilemap->set_transparent_pen(0);
}

/******************************************************************************/

void gondo_state::colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0; // above foreground, background
	if (colour & 8)
		pri_mask |= GFX_PMASK_2; // behind foreground, above background
}

u32 gondo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	m_bg_tilemap->set_scrollx(0, get_u16be(&m_scroll[0]));
	m_bg_tilemap->set_scrolly(0, get_u16be(&m_scroll[2]));

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 2);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram16.target(), 0x400);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

u32 garyoret_state::screen_update_garyoret(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, get_u16be(&m_scroll[0]));
	m_bg_tilemap->set_scrolly(0, get_u16be(&m_scroll[2]));

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram16.target(), 0x400);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILE_GET_INFO_MEMBER(garyoret_state::get_gondo_fix_tile_info)
{
	const u32 offs = tile_index * 2;
	const u16 tile = get_u16be(&m_videoram[offs]);
	const u8 color = (tile & 0x7000) >> 12;

	tileinfo.set(0, tile & 0xfff, color, 0);
}

TILE_GET_INFO_MEMBER(garyoret_state::get_gondo_tile_info)
{
	const u32 offs = tile_index * 2;
	const u16 tile = get_u16be(&m_bg_ram[offs]);
	const u8 color = tile >> 12;

	if (color & 8 && m_game_uses_priority)
		tileinfo.category = 1;
	else
		tileinfo.category = 0;

	tileinfo.set(1, tile & 0xfff, color, 0);
}

void gondo_state::video_start()
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gondo_state::get_gondo_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gondo_state::get_gondo_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transmask(0, 0x00ff, 0xff00); // Bottom 8 pens
	m_game_uses_priority = 0;
}

VIDEO_START_MEMBER(garyoret_state,garyoret)
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(garyoret_state::get_gondo_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(garyoret_state::get_gondo_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_game_uses_priority = 1;
}


/******************************************************************************/

u8 garyoret_state::i8751_hi_r()
{
	return m_i8751_return >> 8; // MSB
}

u8 garyoret_state::i8751_lo_r()
{
	return m_i8751_return & 0xff; // LSB
}


/******************************************************************************/

template <unsigned Which>
u8 gondo_state::player_io_r(offs_t offset)
{
	const int val = 1 << m_analog_io[Which]->read();

	switch (offset)
	{
		case 0: // Rotary low byte
			return ~(val & 0xff);
		case 1: // Joystick = bottom 4 bits, rotary = top 4
			return ((~val >> 4) & 0xf0) | (m_in_io[Which]->read() & 0xf);
	}
	return 0xff;
}


/***************************************************
*
* Hook-up for games that have a proper MCU dump.
*
***************************************************/

TIMER_CALLBACK_MEMBER(garyoret_state::mcu_irq_clear)
{
	// Gondomania schematics show a clocked LS194 shift register (3A) is used to
	// automatically clear the IRQ request. The MCU does not clear it itself.
	m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(garyoret_state::audiocpu_nmi_clear)
{
	// Gondomania schematics show a LS194 for the sound IRQ, sharing the 6502 clock
	// S1=H, S0=L, LSI=H, and QA is the only output connected (to NMI)
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void garyoret_state::i8751_lo_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff00) | data;
}

void ghostb_state::i8751_hi_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff) | (data << 8);

	// MCU interrupt is still level-triggered, but no ack?
	m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
	m_i8751_timer->adjust(attotime::from_ticks(64, 12_MHz_XTAL / 8)); // 64 clocks not confirmed
}

void garyoret_state::i8751_hi_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff) | (data << 8);

	// MCU interrupt is edge-triggered
	m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
	m_i8751_timer->adjust(attotime::from_ticks(4, 12_MHz_XTAL / 8));
}


/******************************************************************************/

void garyoret_state::gondo_bank_w(u8 data)
{
	/* Bit 0: SECCLR - acknowledge interrupt from I8751
	   Bit 1: NMI enable/disable
	   Bit 2: Not connected according to schematics
	   Bit 3: Screen flip
	   Bits 4-7: Bank switch
	*/
	m_mainbank->set_entry((data >> 4) & m_bank_mask);

	m_secclr = BIT(data, 0);
	if (!m_secclr)
		m_maincpu->set_input_line(HD6309_IRQ_LINE, CLEAR_LINE);

	// it relies on 1 more opcode after enabling NMI (see TODO notes)
	if (BIT(m_bank_data ^ data, 1))
		m_nmi_timer->adjust(m_maincpu->minimum_quantum_time(), BIT(data, 1));

	flip_screen_set(BIT(data, 3));
	m_bank_data = data;
}

void garyoret_state::ghostb_bank_w(u8 data)
{
	gondo_bank_w(data);

	// Bit 2: Sprite DMA (see gondo_scroll_w for gondo/garyoret)
	buffer_spriteram_w(BIT(data, 2));
}

void garyoret_state::sound_w(u8 data)
{
	m_soundlatch->write(data);
	m_audiocpu->set_input_line(m6502_device::NMI_LINE, ASSERT_LINE);
	m_6502_timer->adjust(m_audiocpu->cycles_to_attotime(4));
}


/******************************************************************************/

void ghostb_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x17ff).ram();
	map(0x1800, 0x1fff).ram().w(FUNC(ghostb_state::videoram_w)).share(m_videoram);
	map(0x2000, 0x27ff).rw(m_tilegen, FUNC(deco_bac06_device::vram8_r<false>), FUNC(deco_bac06_device::vram8_w<false>));
	map(0x2800, 0x2bff).ram(); // colscroll? mirror?
	map(0x2c00, 0x2fff).rw(m_tilegen, FUNC(deco_bac06_device::rowscroll8_r<false>), FUNC(deco_bac06_device::rowscroll8_w<false>));
	map(0x3000, 0x37ff).ram().share(m_spriteram);
	map(0x3800, 0x3800).portr("IN0");
	map(0x3800, 0x3800).w(FUNC(ghostb_state::sound_w));
	map(0x3801, 0x3801).portr("IN1");
	map(0x3802, 0x3802).portr("IN2");
	map(0x3803, 0x3803).portr("DSW0");
	map(0x3820, 0x3820).portr("DSW1");
	map(0x3820, 0x3827).w(m_tilegen, FUNC(deco_bac06_device::ctrlreg8_w));
	map(0x3830, 0x383f).rw(m_tilegen, FUNC(deco_bac06_device::scrollreg8_r<false>), FUNC(deco_bac06_device::scrollreg8_w<false>));
	map(0x3840, 0x3840).r(FUNC(ghostb_state::i8751_hi_r));
	map(0x3840, 0x3840).w(FUNC(ghostb_state::ghostb_bank_w));
	map(0x3860, 0x3860).r(FUNC(ghostb_state::i8751_lo_r));
	map(0x3860, 0x3860).w(FUNC(ghostb_state::i8751_hi_w));
	map(0x3861, 0x3861).w(FUNC(ghostb_state::i8751_lo_w));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

void gondo_state::main_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();
	map(0x1800, 0x1fff).ram().w(FUNC(gondo_state::videoram_w)).share(m_videoram);
	map(0x2000, 0x27ff).rw(FUNC(gondo_state::bg_ram_r), FUNC(gondo_state::bg_ram_w)).share(m_bg_ram);
	map(0x2800, 0x2bff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x2c00, 0x2fff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x3000, 0x37ff).ram().share(m_spriteram);
	map(0x3800, 0x3800).portr("DSW0");
	map(0x3801, 0x3801).portr("DSW1");
	map(0x380a, 0x380b).r(FUNC(gondo_state::player_io_r<0>));
	map(0x380c, 0x380d).r(FUNC(gondo_state::player_io_r<1>));
	map(0x380e, 0x380e).portr("IN3");
	map(0x380f, 0x380f).portr("IN2");
	map(0x3810, 0x3810).w(FUNC(gondo_state::sound_w));
	map(0x3818, 0x382f).w(FUNC(gondo_state::gondo_scroll_w));
	map(0x3830, 0x3830).w(FUNC(gondo_state::gondo_bank_w));
	map(0x3838, 0x3838).r(FUNC(gondo_state::i8751_hi_r));
	map(0x3839, 0x3839).r(FUNC(gondo_state::i8751_lo_r));
	map(0x383a, 0x383a).w(FUNC(gondo_state::i8751_hi_w));
	map(0x383b, 0x383b).w(FUNC(gondo_state::i8751_lo_w));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

void garyoret_state::garyoret_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();
	map(0x1800, 0x1fff).ram().w(FUNC(garyoret_state::videoram_w)).share(m_videoram);
	map(0x2000, 0x27ff).rw(FUNC(garyoret_state::bg_ram_r), FUNC(garyoret_state::bg_ram_w)).share(m_bg_ram);
	map(0x2800, 0x2bff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x2c00, 0x2fff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x3000, 0x37ff).ram().share(m_spriteram);
	map(0x3800, 0x3800).portr("DSW0");
	map(0x3801, 0x3801).portr("DSW1");
	map(0x3808, 0x3808).nopr(); // ?
	map(0x380a, 0x380a).portr("IN1");
	map(0x380b, 0x380b).portr("IN0");
	map(0x3810, 0x3810).w(FUNC(garyoret_state::sound_w));
	map(0x3818, 0x382f).w(FUNC(garyoret_state::gondo_scroll_w));
	map(0x3830, 0x3830).w(FUNC(garyoret_state::gondo_bank_w));
	map(0x3838, 0x3838).w(FUNC(garyoret_state::i8751_hi_w));
	map(0x3839, 0x3839).w(FUNC(garyoret_state::i8751_lo_w));
	map(0x383a, 0x383a).r(FUNC(garyoret_state::i8751_hi_r));
	map(0x383b, 0x383b).r(FUNC(garyoret_state::i8751_lo_r));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}


/******************************************************************************/

// Used for Maze Hunter, The Real Ghostbusters
void ghostb_state::sound_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x2000, 0x2001).w("ym1", FUNC(ym2203_device::write));
	map(0x4000, 0x4001).w("ym2", FUNC(ym3812_device::write));
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}

// Used by Gondomania, Garyo Retsuden
void garyoret_state::gondo_sound_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x2000, 0x2001).w("ym1", FUNC(ym2203_device::write));
	map(0x4000, 0x4001).w("ym2", FUNC(ym3526_device::write));
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}


/******************************************************************************/

/*
    Gondomania schematics show the following:

    Port P0 - attached to 2 * LS374 at location 4C & 1C
    Port P1 - attached to 2 * LS374 at location 3C & 2C
    Port P2.2 -> SECIRQ (IRQ to main CPU)
    Port P2.3 -> 'COUNT' (Enable coin counter - also wired directly to coinage) [not emulated]
    Port P2.4-7 -> Enable latches 4C, 1C, 3C, 2C
    Port P3.4-7 -> Directly attached to coinage connector (3 coins & service)

*/

u8 garyoret_state::i8751_port0_r()
{
	return m_i8751_port0;
}

void garyoret_state::i8751_port0_w(u8 data)
{
	m_i8751_port0 = data;
}

u8 garyoret_state::i8751_port1_r()
{
	return m_i8751_port1;
}

void garyoret_state::i8751_port1_w(u8 data)
{
	m_i8751_port1 = data;
}

void garyoret_state::mcu_to_main_w(u8 data)
{
	const u8 fall = ~data & m_i8751_p2;
	const u8 rise = data & ~m_i8751_p2;
	m_i8751_p2 = data;

	// P24-P27: controls latches for main CPU communication
	if (BIT(fall, 4))
		m_i8751_port0 = m_i8751_value >> 8;
	if (BIT(fall, 5))
		m_i8751_port1 = m_i8751_value & 0xff;
	if (BIT(fall, 6))
		m_i8751_return = (m_i8751_return & 0xff) | (u16(m_i8751_port0) << 8);
	if (BIT(fall, 7))
		m_i8751_return = (m_i8751_return & 0xff00) | m_i8751_port1;

	// P22: IRQ to main CPU
	if (BIT(rise, 2) && m_secclr)
		m_maincpu->set_input_line(HD6309_IRQ_LINE, ASSERT_LINE);

	// P20,P21: N/C
}


/******************************************************************************/

// verified from HD6309 code
static INPUT_PORTS_START( gondo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	// Top 4 bits are rotary controller

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	// Top 4 bits are rotary controller

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN") // hooked up on the i8751
	// Low 4 bits not connected on schematics
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // produces sound but gives 0 credits - coinage not initialised in the MCU

	PORT_START("AN0") // player 1 12-way rotary control
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("AN1") // player 2 12-way rotary control
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2") // table at 0x01b8 in MCU (4 bytes : coins in 4 MSbits and credits in 4 LSbits)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:3,4") // table at 0x01bc in MCU (4 bytes : coins in 4 MSbits and credits in 4 LSbits)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:5")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Swap buttons" )                  PORT_DIPLOCATION("SW1:8") // code at 0x8a2b in 'gondo', 0x88c5 in 'makyosen' - undocumented in the manual
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)") // gives 99 lives
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END


// verified from HD6309 code
static INPUT_PORTS_START( garyoret )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // bomb
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // bomb
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("COIN") // hooked up on the i8751
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // produces sound but gives 0 credits - coinage not initialised in the MCU

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:5")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:8") // not tested - no cocktail when simultaneous players anyway

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:5")
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPNAME( 0x40, 0x40, "Leave Off" )                     PORT_DIPLOCATION("SW2:7") // game doesn't boot when this is On - code at 0x807f and test at 0x819e
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END


// verified from HD6309 code
static INPUT_PORTS_START( ghostb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // "FIRE"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // beam / upgradable shot when out of energy
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // "FIRE"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // beam / upgradable shot when out of energy
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	// Low 4 bits not connected on schematics
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // produce sound but gives 0 credits - "ANDA" instruction at 0x8a5a

	PORT_START("DSW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) // Tested on real hardware
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	// 1-2 should be coinage.
//  PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2")
//  PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
//  PORT_DIPLOCATION("SW1:3") // Manual says 'Must Be Off'. Note: Turning on 3+4+5+8 does nothing on real hardware.
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:4") // Manual says 'Must Be Off'. See note
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:5") // Manual says 'Must Be Off'. See note
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:8") // not tested - no cocktail when simultaneous players anyway. Manual says 'Must Be Off'. See note

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1,2") // lives are added when STARTn is pressed
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "Invulnerability (Cheat)")        // gives 1 life and energy does not decrease
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, "Max Scene Time" )                PORT_DIPLOCATION("SW2:5,6") // 1:00 is added when STARTn is pressed until max scene time is reached
	PORT_DIPSETTING(    0x00, "4:00" )
	PORT_DIPSETTING(    0x10, "4:30" )
	PORT_DIPSETTING(    0x30, "5:00" )
	PORT_DIPSETTING(    0x20, "6:00" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Energy Bonus" )                  PORT_DIPLOCATION("SW2:8") // energy is set to value each new life
	PORT_DIPSETTING(    0x80, DEF_STR( None ) )                                           // 0x0100
	PORT_DIPSETTING(    0x00, "+25%" )                                                    // 0x0140
INPUT_PORTS_END

// verified from HD6309 code
static INPUT_PORTS_START( ghostb2a )
	PORT_INCLUDE(ghostb)

	// BUTTON1 : upgradable shot - BUTTON2 : beam (provided you have energy)

	PORT_MODIFY("COIN")
	// Low 4 bits not connected on schematics
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // produce sound but gives 0 lives - "ANDA" instruction at 0x8a20

	PORT_MODIFY("DSW0")
	// NO start buttons - to start a game, press any button from any player
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// 1 & 2 should be coinage
//  PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2")
//  PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )

	PORT_MODIFY("DSW1")
	// lives are added when COINn is pressed
	// 1:00 is added when COINn is pressed until max scene time is reached
	PORT_DIPNAME( 0x80, 0x80, "Energy Bonus" )                  PORT_DIPLOCATION("SW2:8") // energy is added when COINn is pressed
	PORT_DIPSETTING(    0x80, DEF_STR( None ) )                                           // 0x0040
	PORT_DIPSETTING(    0x00, "+50%" )                                                    // 0x0060
INPUT_PORTS_END

// verified from HD6309 code
static INPUT_PORTS_START( ghostb3 )
	PORT_INCLUDE(ghostb2a)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) // upgradable shot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) // beam (provided you have energy)

	PORT_MODIFY("COIN")
	// Low 4 bits not connected on schematics
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )
INPUT_PORTS_END

// verified from HD6309 code
static INPUT_PORTS_START( meikyuh )
	PORT_INCLUDE(ghostb)

	PORT_MODIFY("COIN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	// BUTTON1 : upgradable shot - BUTTON2 : circular fire (provided you have energy) - BUTTON1 + BUTTON2 : beam (provided you have energy)

	PORT_MODIFY("DSW1")
	// lives are added when STARTn is pressed - 1 extra life is awarded on 2nd credit and after for the same player who gets then 2, 4 or 6 additional lives
	// max time scene is always 6:00 at start - 0:30 is subed every 8 levels - 1:00 is added when STARTn is pressed until max scene time is reached
	PORT_DIPNAME( 0x10, 0x10, "Energy Bonus" )                  PORT_DIPLOCATION("SW2:5") // energy is added when STARTn is pressed
	PORT_DIPSETTING(    0x10, DEF_STR( None ) )                                           // 0x0020
	PORT_DIPSETTING(    0x00, "+50%" )                                                    // 0x0030
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	3,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every sprite takes 8 consecutive bytes
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),0 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
		0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};

// X flipped on Ghostbusters tiles
static const gfx_layout ghostb_tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(2,4),0,RGN_FRAC(3,4),RGN_FRAC(1,4) },
	{ 7,6,5,4,3,2,1,0,
		7+(16*8), 6+(16*8), 5+(16*8), 4+(16*8), 3+(16*8), 2+(16*8), 1+(16*8), 0+(16*8) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};

static GFXDECODE_START( gfx_ghostb )
	GFXDECODE_ENTRY( "char",   0, charlayout,          0,  4 )
	GFXDECODE_ENTRY( "tiles",  0, ghostb_tilelayout, 512, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_gondo )
	GFXDECODE_ENTRY( "char",    0, charlayout,   0, 16 ) // Chars
	GFXDECODE_ENTRY( "tiles",   0, tilelayout, 768, 16 ) // Tiles
GFXDECODE_END

static GFXDECODE_START( gfx_gondo_spr )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 256, 32 ) // Sprites
GFXDECODE_END

static GFXDECODE_START( gfx_shackled )
	GFXDECODE_ENTRY( "char",    0, charlayout,   0,  4 )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout, 768, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_shackled_spr )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 256, 16 )
GFXDECODE_END


/******************************************************************************/

void garyoret_state::machine_start()
{
	u8 *ROM = memregion("mainbank")->base();
	const u8 max_bank = memregion("mainbank")->bytes() / 0x4000;
	m_mainbank->configure_entries(0, max_bank, &ROM[0], 0x4000);
	m_bank_mask = (max_bank - 1) & 0xf;

	m_i8751_p2 = 0xff;

	m_nmi_timer = timer_alloc(FUNC(garyoret_state::nmigate_set), this);
	m_6502_timer = timer_alloc(FUNC(garyoret_state::audiocpu_nmi_clear), this);
	m_i8751_timer = timer_alloc(FUNC(garyoret_state::mcu_irq_clear), this);

	save_item(NAME(m_bank_data));
	save_item(NAME(m_secclr));
	save_item(NAME(m_buffer_strobe));
	save_item(NAME(m_scroll));
	save_item(NAME(m_i8751_p2));
	save_item(NAME(m_i8751_port0));
	save_item(NAME(m_i8751_port1));
	save_item(NAME(m_i8751_return));
	save_item(NAME(m_i8751_value));
}

void garyoret_state::machine_reset()
{
	m_scroll[0] = m_scroll[1] = m_scroll[2] = m_scroll[3] = 0;

	m_i8751_port0 = m_i8751_port1 = 0;
	m_i8751_return = m_i8751_value = 0;

	// reset clears LS273 latch, which disables NMI
	ghostb_bank_w(0);
}


void garyoret_state::set_screen_raw_params(machine_config &config)
{
	// DECO video CRTC, matches PCB measurements
	m_screen->set_raw(12_MHz_XTAL / 2, 384, 0, 256, 272, 8, 248);
}

void gondo_state::gondo(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 3'000'000); // HD63C09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &gondo_state::main_map);

	R65C02(config, m_audiocpu, 1'500'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &gondo_state::gondo_sound_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(gondo_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(gondo_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(gondo_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(gondo_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(gondo_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	// video hardware
	DECO_KARNOVSPRITES(config, m_spritegen, m_palette, gfx_gondo_spr);
	m_spritegen->set_colpri_callback(FUNC(gondo_state::colpri_cb));

	SCREEN(config, m_screen);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(gondo_state::screen_update));
	m_screen->screen_vblank().set(m_nmigate, FUNC(input_merger_device::in_w<1>));
	m_screen->screen_vblank().append_inputline(m_mcu, MCS51_INT0_LINE);
	m_screen->set_palette(m_palette);

	INPUT_MERGER_ALL_HIGH(config, m_nmigate).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gondo);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 1'500'000));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3526_device &ym2(YM3526(config, "ym2", 3'000'000));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void garyoret_state::garyoret(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 3'000'000); // HD63C09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &garyoret_state::garyoret_map);

	R65C02(config, m_audiocpu, 1'500'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &garyoret_state::gondo_sound_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(garyoret_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(garyoret_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(garyoret_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(garyoret_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(garyoret_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	// video hardware
	DECO_KARNOVSPRITES(config, m_spritegen, m_palette, gfx_gondo_spr);

	SCREEN(config, m_screen);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(garyoret_state::screen_update_garyoret));
	m_screen->screen_vblank().set(m_nmigate, FUNC(input_merger_device::in_w<1>));
	m_screen->screen_vblank().append_inputline(m_mcu, MCS51_INT0_LINE);
	m_screen->set_palette(m_palette);

	INPUT_MERGER_ALL_HIGH(config, m_nmigate).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gondo);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	MCFG_VIDEO_START_OVERRIDE(garyoret_state,garyoret)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 1'500'000));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3526_device &ym2(YM3526(config, "ym2", 3'000'000));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void ghostb_state::ghostb(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 12_MHz_XTAL / 4); // HD63C09EP, clock verified
	m_maincpu->set_addrmap(AS_PROGRAM, &ghostb_state::main_map);

	DECO_222(config, m_audiocpu, 12_MHz_XTAL / 8); // also seen with stock M6502, clock verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &ghostb_state::sound_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL); // 8.0MHz OSC next to MCU - clock verified
	m_mcu->port_in_cb<0>().set(FUNC(ghostb_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(ghostb_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(ghostb_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(ghostb_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(ghostb_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	// video hardware
	DECO_BAC06(config, m_tilegen);
	m_tilegen->set_gfx_region_wide(1, 1, 0);
	m_tilegen->set_gfxdecode_tag(m_gfxdecode);

	DECO_KARNOVSPRITES(config, m_spritegen, m_palette, gfx_shackled_spr);

	SCREEN(config, m_screen);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(ghostb_state::screen_update));
	m_screen->screen_vblank().set(m_nmigate, FUNC(input_merger_device::in_w<1>));
	m_screen->screen_vblank().append_inputline(m_mcu, MCS51_INT0_LINE);
	m_screen->set_palette(m_palette);

	INPUT_MERGER_ALL_HIGH(config, m_nmigate).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ghostb);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting
	m_palette->set_prom_region("proms");
	m_palette->set_init(m_palette, FUNC(deco_rmc3_device::palette_init_proms));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8)); // clock verified
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3812_device &ym2(YM3812(config, "ym2", 12_MHz_XTAL / 4)); // clock verified
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void ghostb_state::meikyuh(machine_config &config)
{
	ghostb(config);

	R65C02(config.replace(), m_audiocpu, 1'500'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &ghostb_state::sound_map);
}


/******************************************************************************/

ROM_START( gondo )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dt00-e.f3", 0x00000, 0x08000, CRC(912a7eee) SHA1(15af89babf166dadaa77640e1973d7ebb4c078db) ) // Verified only DT00-E & DT03-E have the "-E" extention

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dt01.f5",   0x00000, 0x10000, CRC(c39bb877) SHA1(9beb59ba19f38417c5d4d36e8f3c41f2b017d2d6) )
	ROM_LOAD( "dt02.f6",   0x10000, 0x10000, CRC(925307a4) SHA1(1e8b8eb21df1a11b14c981b343b34c6cc3676517) ) // same label as the US version, but content identical to Japanese version
	ROM_LOAD( "dt03-e.f7", 0x20000, 0x10000, CRC(ee7475eb) SHA1(8c68198ea1c3e89c9c2c4ba0e5d3f47afb8eecd4) )
	ROM_FILL(              0x30000, 0x10000, 0xff)

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dt05-e.h5", 0x0000, 0x8000, CRC(ec08aa29) SHA1(ce83974ae095d9518d1ebf9f7e712f0cbc2c1b42) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dt-e.b1", 0x0000, 0x1000, BAD_DUMP CRC(0d0532ec) SHA1(30894f69ff24c1be4b684e07729bbb3e0f353086) ) // hand-crafted from the US version

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dt14-e.b18", 0x00000, 0x08000, CRC(00cbe9c8) SHA1(de7b640de8fd54ee79194945c96d5768d09f483b) ) // identical to Japanese version

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "dt19.f13",   0x00000, 0x10000, CRC(da2abe4b) SHA1(d53e4769671f3fd437edcff7e7ea05156bbcb45d) ) // All sprite data matches the Japanese set
	ROM_LOAD( "dt20-e.f15", 0x10000, 0x08000, CRC(0eef7f56) SHA1(05c23aa6a598478cd4822634cff96055c585e9d2) ) // Verified only DT17-E, DT18-E, DT20-E & DT22-E have the "-E" extention
	ROM_LOAD( "dt16.f9",    0x18000, 0x10000, CRC(e9955d8f) SHA1(aeef5e18f9d36c1bab3000e95205ce1b18cfbf0b) ) // DT15, DT16, DT19 & DT21 do NOT have the "-E" extention
	ROM_LOAD( "dt18-e.f12", 0x28000, 0x08000, CRC(2b2d1468) SHA1(a144ac1b367e1fec876156230e9ab1c99416962e) )
	ROM_LOAD( "dt15.f8",    0x30000, 0x10000, CRC(a54b2eb6) SHA1(25cb61f67135672154f1ad8e0c49ec04655e91de) )
	ROM_LOAD( "dt17-e.f11", 0x40000, 0x08000, CRC(75ae349a) SHA1(15755a28925d5ed37fab4bd988716fcc5d20c290) )
	ROM_LOAD( "dt21.f16",   0x48000, 0x10000, CRC(1c5f682d) SHA1(4b7022cce930a9e9a0087c91e8344269fe7ed889) )
	ROM_LOAD( "dt22-e.f18", 0x58000, 0x08000, CRC(c8ffb148) SHA1(ae1a8b3cd1f5e423dc1a3c7d05f9fe7c689432e3) )

	ROM_REGION( 0x60000, "tiles", 0 )
	ROM_LOAD( "dt08.h10", 0x00000, 0x08000, CRC(aec483f5) SHA1(1d6de823ab0eeb9c89e9c227428ff278663627f3) ) // Tiles data is the same for all 3 regions
	ROM_CONTINUE(         0x10000, 0x08000 )
	ROM_LOAD( "dt09.h12", 0x08000, 0x08000, CRC(446f0ce0) SHA1(072b88d6de5aa0ed6b1d60c266bcf170dea927d5) )
	ROM_LOAD( "dt06.h7",  0x18000, 0x08000, CRC(3fe1527f) SHA1(b8df4bef2b1a879b65214025fc3b5998ef5c8886) )
	ROM_CONTINUE(         0x28000, 0x08000 )
	ROM_LOAD( "dt07.h9",  0x20000, 0x08000, CRC(61f9bce5) SHA1(ef8a5f5e4c66a143304bcab50ca87579f1507864) )
	ROM_LOAD( "dt12.h16", 0x30000, 0x08000, CRC(1a72ca8d) SHA1(f412758452cb3417e85c355ccb8794fde7edf1cc) )
	ROM_CONTINUE(         0x40000, 0x08000 )
	ROM_LOAD( "dt13.h18", 0x38000, 0x08000, CRC(ccb81aec) SHA1(56e524ed4373b7bd1074a0d22ff75ede379f1696) )
	ROM_LOAD( "dt10.h13", 0x48000, 0x08000, CRC(cfcfc9ed) SHA1(57f43d638cf864d68420f0203740be7bda9da5ca) )
	ROM_CONTINUE(         0x58000, 0x08000 )
	ROM_LOAD( "dt11.h15", 0x50000, 0x08000, CRC(53e9cf17) SHA1(8cbb45154a60f42f1b1e7299b12d2e92fc194df8) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ds-23.b10", 0x0000, 0x0400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for Priority (Not yet used)
ROM_END

ROM_START( gondou )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dt00.f3", 0x00000, 0x08000, CRC(a8cf9118) SHA1(865744c9866957d686a31608d356e279fe58934e) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dt01.f5", 0x00000, 0x10000, CRC(c39bb877) SHA1(9beb59ba19f38417c5d4d36e8f3c41f2b017d2d6) )
	ROM_LOAD( "dt02.f6", 0x10000, 0x10000, CRC(bb5e674b) SHA1(8057dc7464a8b6987536f248d607957923b223cf) )
	ROM_LOAD( "dt03.f7", 0x20000, 0x10000, CRC(99c32b13) SHA1(3d79f48e7d198cb2e519d592a89eda505044bce5) )
	ROM_FILL(            0x30000, 0x10000, 0xff)

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dt05.h5", 0x0000, 0x8000, CRC(ec08aa29) SHA1(ce83974ae095d9518d1ebf9f7e712f0cbc2c1b42) ) // == dt05-e.h5

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dt-a.b1", 0x0000, 0x1000, CRC(03abceeb) SHA1(a16b779d7cea1c1437f85fa6b6e08894a46a5674) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dt14.b18", 0x00000, 0x08000, CRC(4bef16e1) SHA1(b8157a7a1b8f36cea1fd353267a4e03d920cb4aa) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "dt19.f13", 0x00000, 0x10000, CRC(da2abe4b) SHA1(d53e4769671f3fd437edcff7e7ea05156bbcb45d) )
	ROM_LOAD( "dt20.f15", 0x10000, 0x08000, CRC(42d01002) SHA1(5a289ffdc83c05f21908a5d0b6247da5b51c1ddd) ) // Unique data to the US set
	ROM_LOAD( "dt16.f9",  0x18000, 0x10000, CRC(e9955d8f) SHA1(aeef5e18f9d36c1bab3000e95205ce1b18cfbf0b) )
	ROM_LOAD( "dt18.f12", 0x28000, 0x08000, CRC(c0c5df1c) SHA1(5b0f71f590434cdd0545ce098666798927727469) ) // Unique data to the US set
	ROM_LOAD( "dt15.f8",  0x30000, 0x10000, CRC(a54b2eb6) SHA1(25cb61f67135672154f1ad8e0c49ec04655e91de) )
	ROM_LOAD( "dt17.f11", 0x40000, 0x08000, CRC(3bbcff0d) SHA1(a8f7aa56ff49ed6b29240c3504d6c9945944953b) ) // Unique data to the US set
	ROM_LOAD( "dt21.f16", 0x48000, 0x10000, CRC(1c5f682d) SHA1(4b7022cce930a9e9a0087c91e8344269fe7ed889) )
	ROM_LOAD( "dt22.f18", 0x58000, 0x08000, CRC(c1876a5f) SHA1(66122ce765723765e20036bd4d461a210c8b94d3) ) // Unique data to the US set

	ROM_REGION( 0x60000, "tiles", 0 )
	ROM_LOAD( "dt08.h10", 0x00000, 0x08000, CRC(aec483f5) SHA1(1d6de823ab0eeb9c89e9c227428ff278663627f3) ) // Tiles data is the same for all 3 regions
	ROM_CONTINUE(         0x10000, 0x08000 )
	ROM_LOAD( "dt09.h12", 0x08000, 0x08000, CRC(446f0ce0) SHA1(072b88d6de5aa0ed6b1d60c266bcf170dea927d5) )
	ROM_LOAD( "dt06.h7",  0x18000, 0x08000, CRC(3fe1527f) SHA1(b8df4bef2b1a879b65214025fc3b5998ef5c8886) )
	ROM_CONTINUE(         0x28000, 0x08000 )
	ROM_LOAD( "dt07.h9",  0x20000, 0x08000, CRC(61f9bce5) SHA1(ef8a5f5e4c66a143304bcab50ca87579f1507864) )
	ROM_LOAD( "dt12.h16", 0x30000, 0x08000, CRC(1a72ca8d) SHA1(f412758452cb3417e85c355ccb8794fde7edf1cc) )
	ROM_CONTINUE(         0x40000, 0x08000 )
	ROM_LOAD( "dt13.h18", 0x38000, 0x08000, CRC(ccb81aec) SHA1(56e524ed4373b7bd1074a0d22ff75ede379f1696) )
	ROM_LOAD( "dt10.h13", 0x48000, 0x08000, CRC(cfcfc9ed) SHA1(57f43d638cf864d68420f0203740be7bda9da5ca) )
	ROM_CONTINUE(         0x58000, 0x08000 )
	ROM_LOAD( "dt11.h15", 0x50000, 0x08000, CRC(53e9cf17) SHA1(8cbb45154a60f42f1b1e7299b12d2e92fc194df8) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ds-23.b10", 0x0000, 0x0400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for Priority (Not yet used)
ROM_END

ROM_START( makyosen )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ds00.f3", 0x00000, 0x08000, CRC(33bb16fe) SHA1(5d3873b66e0d08b35d56a8b508c774b27368a100) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "ds01.f5", 0x00000, 0x10000, CRC(c39bb877) SHA1(9beb59ba19f38417c5d4d36e8f3c41f2b017d2d6) )
	ROM_LOAD( "ds02.f6", 0x10000, 0x10000, CRC(925307a4) SHA1(1e8b8eb21df1a11b14c981b343b34c6cc3676517) )
	ROM_LOAD( "ds03.f7", 0x20000, 0x10000, CRC(9c0fcbf6) SHA1(bfe42b5277fea111840a9f59b2cb8dfe44444029) )
	ROM_FILL(            0x30000, 0x10000, 0xff)

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ds05.h5",  0x0000, 0x8000, CRC(e6e28ca9) SHA1(3b1f8219331db1910bfb428f8964f8fc1063df6f) ) // == dt05-e.h5

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "ds.b1",  0x0000, 0x1000, CRC(08f36e35) SHA1(e8913da71704a89fad41d5bfba45682119166681) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "ds14.b18", 0x00000, 0x08000, CRC(00cbe9c8) SHA1(de7b640de8fd54ee79194945c96d5768d09f483b) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "ds19.f13", 0x00000, 0x10000, CRC(da2abe4b) SHA1(d53e4769671f3fd437edcff7e7ea05156bbcb45d) ) // == dt19.f13
	ROM_LOAD( "ds20.f15", 0x10000, 0x08000, CRC(0eef7f56) SHA1(05c23aa6a598478cd4822634cff96055c585e9d2) ) // == dt20-e.f15
	ROM_LOAD( "ds16.f9",  0x18000, 0x10000, CRC(e9955d8f) SHA1(aeef5e18f9d36c1bab3000e95205ce1b18cfbf0b) ) // == dt16.f9
	ROM_LOAD( "ds18.f12", 0x28000, 0x08000, CRC(2b2d1468) SHA1(a144ac1b367e1fec876156230e9ab1c99416962e) ) // == dt18-e.f12
	ROM_LOAD( "ds15.f8",  0x30000, 0x10000, CRC(a54b2eb6) SHA1(25cb61f67135672154f1ad8e0c49ec04655e91de) ) // == dt15.f8
	ROM_LOAD( "ds17.f11", 0x40000, 0x08000, CRC(75ae349a) SHA1(15755a28925d5ed37fab4bd988716fcc5d20c290) ) // == dt17-e.f11
	ROM_LOAD( "ds21.f16", 0x48000, 0x10000, CRC(1c5f682d) SHA1(4b7022cce930a9e9a0087c91e8344269fe7ed889) ) // == dt21.f16
	ROM_LOAD( "ds22.f18", 0x58000, 0x08000, CRC(c8ffb148) SHA1(ae1a8b3cd1f5e423dc1a3c7d05f9fe7c689432e3) ) // == dt22-e.f18

	ROM_REGION( 0x60000, "tiles", 0 )
	ROM_LOAD( "ds08.h10", 0x00000, 0x08000, CRC(aec483f5) SHA1(1d6de823ab0eeb9c89e9c227428ff278663627f3) ) // Tiles data is the same for all 3 regions
	ROM_CONTINUE(         0x10000, 0x08000 )
	ROM_LOAD( "ds09.h12", 0x08000, 0x08000, CRC(446f0ce0) SHA1(072b88d6de5aa0ed6b1d60c266bcf170dea927d5) )
	ROM_LOAD( "ds06.h7",  0x18000, 0x08000, CRC(3fe1527f) SHA1(b8df4bef2b1a879b65214025fc3b5998ef5c8886) )
	ROM_CONTINUE(         0x28000, 0x08000 )
	ROM_LOAD( "ds07.h9",  0x20000, 0x08000, CRC(61f9bce5) SHA1(ef8a5f5e4c66a143304bcab50ca87579f1507864) )
	ROM_LOAD( "ds12.h16", 0x30000, 0x08000, CRC(1a72ca8d) SHA1(f412758452cb3417e85c355ccb8794fde7edf1cc) )
	ROM_CONTINUE(         0x40000, 0x08000 )
	ROM_LOAD( "ds13.h18", 0x38000, 0x08000, CRC(ccb81aec) SHA1(56e524ed4373b7bd1074a0d22ff75ede379f1696) )
	ROM_LOAD( "ds10.h13", 0x48000, 0x08000, CRC(cfcfc9ed) SHA1(57f43d638cf864d68420f0203740be7bda9da5ca) )
	ROM_CONTINUE(         0x58000, 0x08000 )
	ROM_LOAD( "ds11.h15", 0x50000, 0x08000, CRC(53e9cf17) SHA1(8cbb45154a60f42f1b1e7299b12d2e92fc194df8) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ds-23.b10", 0x0000, 0x0400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for Priority (Not yet used)

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16r4nc.u10", 0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r4nc.g11", 0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r4nc.s1",  0x0400, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( garyoret )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dv00", 0x00000, 0x08000, CRC(cceaaf05) SHA1(b8f54638feab77d023e01ced947e1269f0d19fd8) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dv01", 0x00000, 0x10000, CRC(c33fc18a) SHA1(0d9594b0e6c39aea5b9f15f6aa364b31604f1066) )
	ROM_LOAD( "dv02", 0x10000, 0x10000, CRC(f9e26ce7) SHA1(8589594ebc7ae16942739382273a222dfa30b3b7) )
	ROM_LOAD( "dv03", 0x20000, 0x10000, CRC(55d8d699) SHA1(da1519cd54d27cc406420ce0845e43f7228cfd4e) )
	ROM_LOAD( "dv04", 0x30000, 0x10000, CRC(ed3d00ee) SHA1(6daa2ee509235ad03d3012a00382820279add620) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dv05", 0x00000, 0x08000, CRC(c97c347f) SHA1(a1b22733dc15d524af97db3e608a82503a49b182) )

	ROM_REGION( 0x1000, "mcu", 0 ) // ID8751H (fake) MCU based on 'gondo' one
	ROM_LOAD( "dv__.mcu", 0x0000, 0x1000, BAD_DUMP CRC(37cacec6) SHA1(d81fe36939ccac784a83a69dfc6898b22a4515ec) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dv14", 0x00000, 0x08000, CRC(fb2bc581) SHA1(d597976c5ae586166c49051cc3de8cf0c2e5a5e1) ) // Characters

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "dv22", 0x00000, 0x10000, CRC(cef0367e) SHA1(8beb3a6b91ec0a6ec052243c8f626a581d349f65) )
	ROM_LOAD( "dv21", 0x10000, 0x08000, CRC(90042fb7) SHA1(f19bbf158c92030e8fddb5087b5b69b71956baf8) )
	ROM_LOAD( "dv20", 0x18000, 0x10000, CRC(451a2d8c) SHA1(f4eea444b797d394edeb514ddc1c494fd7ccc2f2) )
	ROM_LOAD( "dv19", 0x28000, 0x08000, CRC(14e1475b) SHA1(f0aec5b7b4be0da06a73ed382e7e851654e47e47) )
	ROM_LOAD( "dv18", 0x30000, 0x10000, CRC(7043bead) SHA1(5d1be8b9cd56ae43d60406b05258d20de980096d) )
	ROM_LOAD( "dv17", 0x40000, 0x08000, CRC(28f449d7) SHA1(cf1bc690b67910c42ad09531ab1d88461d00b944) )
	ROM_LOAD( "dv16", 0x48000, 0x10000, CRC(37e4971e) SHA1(9442c315b7cdbcc046d9e6838cb793c1857174ed) )
	ROM_LOAD( "dv15", 0x58000, 0x08000, CRC(ca41b6ac) SHA1(d7a9ef6c89741c1e8e17a668a9d39ea089f5c73f) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "dv08", 0x00000, 0x08000, CRC(89c13e15) SHA1(6507e60de5cd78a5b46090e4825a44c2a23631d7) )
	ROM_CONTINUE(     0x10000, 0x08000 )
	ROM_LOAD( "dv09", 0x08000, 0x08000, CRC(6a345a23) SHA1(b86f81b9fe25acd833ca3e2cff6cfa853c02280a) )
	ROM_CONTINUE(     0x18000, 0x08000 )

	ROM_LOAD( "dv06", 0x20000, 0x08000, CRC(1eb52a20) SHA1(46670ed973f794be9c2c7e6bf5d97db51211e9a9) )
	ROM_CONTINUE(     0x30000, 0x08000 )
	ROM_LOAD( "dv07", 0x28000, 0x08000, CRC(e7346ef8) SHA1(8083a7a182e8ed904daf2f692115d01b3d0830eb) )
	ROM_CONTINUE(     0x38000, 0x08000 )

	ROM_LOAD( "dv12", 0x40000, 0x08000, CRC(46ba5af4) SHA1(a1c13e7e3c85060202120b64e3cee32c1f733270) )
	ROM_CONTINUE(     0x50000, 0x08000 )
	ROM_LOAD( "dv13", 0x48000, 0x08000, CRC(a7af6dfd) SHA1(fa41bdafb64c79bd9769903fd37d4d5172b66a52) )
	ROM_CONTINUE(     0x58000, 0x08000 )

	ROM_LOAD( "dv10", 0x60000, 0x08000, CRC(68b6d75c) SHA1(ac719ef6b30ac9e63fab13cb359e6114493f274d) )
	ROM_CONTINUE(     0x70000, 0x08000 )
	ROM_LOAD( "dv11", 0x68000, 0x08000, CRC(b5948aee) SHA1(587afbfeda985bede9e4b5f57dad6763f4294962) )
	ROM_CONTINUE(     0x78000, 0x08000 )
ROM_END

ROM_START( ghostb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dz01-22.1d", 0x00000, 0x08000, CRC(fc65fdf2) SHA1(b6ffe2043d5dbff061a9806631646428eed95dd3) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dz02.3d",    0x00000, 0x10000, CRC(8e117541) SHA1(7dfa6eabb29f39a615f3e5123bddcc7197ab82d0) )
	ROM_LOAD( "dz03.4d",    0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) )
	ROM_LOAD( "dz04-21.6d", 0x20000, 0x10000, CRC(7d46582f) SHA1(22e70675d76e2a93a732370fa42cc4b79381f4b0) )
	ROM_LOAD( "dz05-21.7d", 0x30000, 0x10000, CRC(23e1c758) SHA1(c6432682e1d4429d0cfa8de6a05ca0152611b5b1) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dz06.5f", 0x0000, 0x8000, CRC(798f56df) SHA1(aee33cd0c102015114e17f6cb98945e7cc806f55) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dz-1.1b", 0x0000, 0x1000, CRC(9f5f3cb5) SHA1(5ef2b8a5411dde28277d9364db014763019ecf15) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dz00.16b", 0x00000, 0x08000, CRC(992b4f31) SHA1(a9f255286193ccc261a9b6983aabf3c76ebe5ce5) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dz15.14f", 0x00000, 0x10000, CRC(a01a5fd9) SHA1(c15e11fbc0ede9e4a232abe37e6d221d5789ce8e) )
	ROM_LOAD( "dz16.15f", 0x10000, 0x10000, CRC(5a9a344a) SHA1(f4e8c2bae023ce996e99383873eba23ab6f972a8) )
	ROM_LOAD( "dz12.9f",  0x20000, 0x10000, CRC(817fae99) SHA1(4179501aedbdf5bb0824bf1c13e033685e57a207) )
	ROM_LOAD( "dz14.12f", 0x30000, 0x10000, CRC(0abbf76d) SHA1(fefb0cb7b866452b890bcf8c47b1ed95df35095e) )
	ROM_LOAD( "dz11.8f",  0x40000, 0x10000, CRC(a5e19c24) SHA1(a4aae81a116577ee3cdd9e1a46cae413ae252b76) )
	ROM_LOAD( "dz13.1f",  0x50000, 0x10000, CRC(3e7c0405) SHA1(2cdcb9a902acecec0729a906b7edb44baf130d32) )
	ROM_LOAD( "dz17.17f", 0x60000, 0x10000, CRC(40361b8b) SHA1(6ee59129e236ead3d9b828fb9726311b7a4f2ff6) )
	ROM_LOAD( "dz18.18f", 0x70000, 0x10000, CRC(8d219489) SHA1(0490ad84085d1a60ece1b8ab45f0c551d2ac219d) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "dz07.12f", 0x00000, 0x10000, CRC(e7455167) SHA1(a4582ced57862563ef626a25ced4072bc2c95750) )
	ROM_LOAD( "dz08.14f", 0x10000, 0x10000, CRC(32f9ddfe) SHA1(2b8c228b0ca938ab7495d53e1a39275a8b872828) )
	ROM_LOAD( "dz09.15f", 0x20000, 0x10000, CRC(bb6efc02) SHA1(ec501cd4a624d9c36a545dd100bc4f2f8b1e5cc0) )
	ROM_LOAD( "dz10.17f", 0x30000, 0x10000, CRC(6ef9963b) SHA1(f12a2e2b0451a118234b2995185bb14d4998d430) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, CRC(47e1f83b) SHA1(f073eea1f33ed7a4862e4efd143debf1e0ee64b4) )
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, CRC(d8fe2d99) SHA1(56f8fcf2f871c7d52d4299a5b9988401ada4319d) )
ROM_END

ROM_START( ghostb2a )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dz01.1d", 0x00000, 0x08000, CRC(7c5bb4b1) SHA1(75865102c9bfbf9bd341b8ea54f49904c727f474) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dz02.3d", 0x00000, 0x10000, CRC(8e117541) SHA1(7dfa6eabb29f39a615f3e5123bddcc7197ab82d0) )
	ROM_LOAD( "dz03.4d", 0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) )
	ROM_LOAD( "dz04.6d", 0x20000, 0x10000, CRC(d09bad99) SHA1(bde8e4316cedf1d292f0aed8627b0dc6794c6e07) )
	ROM_LOAD( "dz05.7d", 0x30000, 0x10000, CRC(0315f691) SHA1(3bfad18b9f230e64c608a22af20c3b00dca3e6da) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dz06.5f", 0x0000, 0x8000, CRC(798f56df) SHA1(aee33cd0c102015114e17f6cb98945e7cc806f55) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dz-1.1b", 0x0000, 0x1000, CRC(9f5f3cb5) SHA1(5ef2b8a5411dde28277d9364db014763019ecf15) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dz00.16b", 0x00000, 0x08000, CRC(992b4f31) SHA1(a9f255286193ccc261a9b6983aabf3c76ebe5ce5) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dz15.14f", 0x00000, 0x10000, CRC(a01a5fd9) SHA1(c15e11fbc0ede9e4a232abe37e6d221d5789ce8e) )
	ROM_LOAD( "dz16.15f", 0x10000, 0x10000, CRC(5a9a344a) SHA1(f4e8c2bae023ce996e99383873eba23ab6f972a8) )
	ROM_LOAD( "dz12.9f",  0x20000, 0x10000, CRC(817fae99) SHA1(4179501aedbdf5bb0824bf1c13e033685e57a207) )
	ROM_LOAD( "dz14.12f", 0x30000, 0x10000, CRC(0abbf76d) SHA1(fefb0cb7b866452b890bcf8c47b1ed95df35095e) )
	ROM_LOAD( "dz11.8f",  0x40000, 0x10000, CRC(a5e19c24) SHA1(a4aae81a116577ee3cdd9e1a46cae413ae252b76) )
	ROM_LOAD( "dz13.1f",  0x50000, 0x10000, CRC(3e7c0405) SHA1(2cdcb9a902acecec0729a906b7edb44baf130d32) )
	ROM_LOAD( "dz17.17f", 0x60000, 0x10000, CRC(40361b8b) SHA1(6ee59129e236ead3d9b828fb9726311b7a4f2ff6) )
	ROM_LOAD( "dz18.18f", 0x70000, 0x10000, CRC(8d219489) SHA1(0490ad84085d1a60ece1b8ab45f0c551d2ac219d) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "dz07.12f", 0x00000, 0x10000, CRC(e7455167) SHA1(a4582ced57862563ef626a25ced4072bc2c95750) )
	ROM_LOAD( "dz08.14f", 0x10000, 0x10000, CRC(32f9ddfe) SHA1(2b8c228b0ca938ab7495d53e1a39275a8b872828) )
	ROM_LOAD( "dz09.15f", 0x20000, 0x10000, CRC(bb6efc02) SHA1(ec501cd4a624d9c36a545dd100bc4f2f8b1e5cc0) )
	ROM_LOAD( "dz10.17f", 0x30000, 0x10000, CRC(6ef9963b) SHA1(f12a2e2b0451a118234b2995185bb14d4998d430) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, CRC(47e1f83b) SHA1(f073eea1f33ed7a4862e4efd143debf1e0ee64b4) )
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, CRC(d8fe2d99) SHA1(56f8fcf2f871c7d52d4299a5b9988401ada4319d) )
ROM_END

ROM_START( ghostb3 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dz01-3b.1d", 0x00000, 0x08000, CRC(c8cc862a) SHA1(e736107beb11a12cdf413655c6874df28d5a9c70) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dz02.3d",    0x00000, 0x10000, CRC(8e117541) SHA1(7dfa6eabb29f39a615f3e5123bddcc7197ab82d0) )
	ROM_LOAD( "dz03.4d",    0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) )
	ROM_LOAD( "dz04-1.6d",  0x20000, 0x10000, CRC(3c3eb09f) SHA1(ae4975992698fa97c68a857a25b470a05539160a) )
	ROM_LOAD( "dz05-1.7d",  0x30000, 0x10000, CRC(b4971d33) SHA1(25e052c4b414c7bd7b6e3ae9c211873902afb5f7) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dz06.5f", 0x0000, 0x8000, CRC(798f56df) SHA1(aee33cd0c102015114e17f6cb98945e7cc806f55) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dz-1.1b", 0x0000, 0x1000, CRC(9f5f3cb5) SHA1(5ef2b8a5411dde28277d9364db014763019ecf15) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dz00.16b", 0x00000, 0x08000, CRC(992b4f31) SHA1(a9f255286193ccc261a9b6983aabf3c76ebe5ce5) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dz15.14f", 0x00000, 0x10000, CRC(a01a5fd9) SHA1(c15e11fbc0ede9e4a232abe37e6d221d5789ce8e) )
	ROM_LOAD( "dz16.15f", 0x10000, 0x10000, CRC(5a9a344a) SHA1(f4e8c2bae023ce996e99383873eba23ab6f972a8) )
	ROM_LOAD( "dz12.9f",  0x20000, 0x10000, CRC(817fae99) SHA1(4179501aedbdf5bb0824bf1c13e033685e57a207) )
	ROM_LOAD( "dz14.12f", 0x30000, 0x10000, CRC(0abbf76d) SHA1(fefb0cb7b866452b890bcf8c47b1ed95df35095e) )
	ROM_LOAD( "dz11.8f",  0x40000, 0x10000, CRC(a5e19c24) SHA1(a4aae81a116577ee3cdd9e1a46cae413ae252b76) )
	ROM_LOAD( "dz13.1f",  0x50000, 0x10000, CRC(3e7c0405) SHA1(2cdcb9a902acecec0729a906b7edb44baf130d32) )
	ROM_LOAD( "dz17.17f", 0x60000, 0x10000, CRC(40361b8b) SHA1(6ee59129e236ead3d9b828fb9726311b7a4f2ff6) )
	ROM_LOAD( "dz18.18f", 0x70000, 0x10000, CRC(8d219489) SHA1(0490ad84085d1a60ece1b8ab45f0c551d2ac219d) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "dz07.12f", 0x00000, 0x10000, CRC(e7455167) SHA1(a4582ced57862563ef626a25ced4072bc2c95750) )
	ROM_LOAD( "dz08.14f", 0x10000, 0x10000, CRC(32f9ddfe) SHA1(2b8c228b0ca938ab7495d53e1a39275a8b872828) )
	ROM_LOAD( "dz09.15f", 0x20000, 0x10000, CRC(bb6efc02) SHA1(ec501cd4a624d9c36a545dd100bc4f2f8b1e5cc0) )
	ROM_LOAD( "dz10.17f", 0x30000, 0x10000, CRC(6ef9963b) SHA1(f12a2e2b0451a118234b2995185bb14d4998d430) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, CRC(47e1f83b) SHA1(f073eea1f33ed7a4862e4efd143debf1e0ee64b4) )
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, CRC(d8fe2d99) SHA1(56f8fcf2f871c7d52d4299a5b9988401ada4319d) )
ROM_END

// DZ-1 is the verified correct MCU code for the ghostb3a set below, both DZ01-2 & DZ04- ROMs have been verified correct from multiple sources
ROM_START( ghostb3a )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dz01-2.1d", 0x00000, 0x08000, CRC(1b16890e) SHA1(eebd253d616b6286937b72cfb64612877f383932) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dz02-.3d",  0x00000, 0x10000, CRC(8e117541) SHA1(7dfa6eabb29f39a615f3e5123bddcc7197ab82d0) ) // == dz02.3d (ghostb3)
	ROM_LOAD( "dz03-.4d",  0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) ) // == dz03.4d (ghostb3)
	ROM_LOAD( "dz04-.6d",  0x20000, 0x10000, CRC(490b4525) SHA1(3066b76f8fe99c8f9f1cdf943209883a199a4184) )
	ROM_LOAD( "dz05-.7d",  0x30000, 0x10000, CRC(b4971d33) SHA1(25e052c4b414c7bd7b6e3ae9c211873902afb5f7) ) // == dz05-1.7d (ghostb3)

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dz06.5f", 0x0000, 0x8000, CRC(798f56df) SHA1(aee33cd0c102015114e17f6cb98945e7cc806f55) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dz-1.1b", 0x0000, 0x1000, CRC(9f5f3cb5) SHA1(5ef2b8a5411dde28277d9364db014763019ecf15) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dz00.16b", 0x00000, 0x08000, CRC(992b4f31) SHA1(a9f255286193ccc261a9b6983aabf3c76ebe5ce5) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dz15.14f", 0x00000, 0x10000, CRC(a01a5fd9) SHA1(c15e11fbc0ede9e4a232abe37e6d221d5789ce8e) )
	ROM_LOAD( "dz16.15f", 0x10000, 0x10000, CRC(5a9a344a) SHA1(f4e8c2bae023ce996e99383873eba23ab6f972a8) )
	ROM_LOAD( "dz12.9f",  0x20000, 0x10000, CRC(817fae99) SHA1(4179501aedbdf5bb0824bf1c13e033685e57a207) )
	ROM_LOAD( "dz14.12f", 0x30000, 0x10000, CRC(0abbf76d) SHA1(fefb0cb7b866452b890bcf8c47b1ed95df35095e) )
	ROM_LOAD( "dz11.8f",  0x40000, 0x10000, CRC(a5e19c24) SHA1(a4aae81a116577ee3cdd9e1a46cae413ae252b76) )
	ROM_LOAD( "dz13.1f",  0x50000, 0x10000, CRC(3e7c0405) SHA1(2cdcb9a902acecec0729a906b7edb44baf130d32) )
	ROM_LOAD( "dz17.17f", 0x60000, 0x10000, CRC(40361b8b) SHA1(6ee59129e236ead3d9b828fb9726311b7a4f2ff6) )
	ROM_LOAD( "dz18.18f", 0x70000, 0x10000, CRC(8d219489) SHA1(0490ad84085d1a60ece1b8ab45f0c551d2ac219d) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "dz07.12f", 0x00000, 0x10000, CRC(e7455167) SHA1(a4582ced57862563ef626a25ced4072bc2c95750) )
	ROM_LOAD( "dz08.14f", 0x10000, 0x10000, CRC(32f9ddfe) SHA1(2b8c228b0ca938ab7495d53e1a39275a8b872828) )
	ROM_LOAD( "dz09.15f", 0x20000, 0x10000, CRC(bb6efc02) SHA1(ec501cd4a624d9c36a545dd100bc4f2f8b1e5cc0) )
	ROM_LOAD( "dz10.17f", 0x30000, 0x10000, CRC(6ef9963b) SHA1(f12a2e2b0451a118234b2995185bb14d4998d430) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, CRC(47e1f83b) SHA1(f073eea1f33ed7a4862e4efd143debf1e0ee64b4) )
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, CRC(d8fe2d99) SHA1(56f8fcf2f871c7d52d4299a5b9988401ada4319d) )
ROM_END

/*

Meikyuu Hunter G (Data East, 1987)
Hardware info by Guru

PCB Layout
----------

DE-0273-1
|-------------------------------------------------------------|
|  2018           DW09                       DW00             |
|  2018                                                       |
|                 DW08                                      |-|
|   |---------|                                    6264     | |
|   |         |   DW07                                      | |
|   |L7A0072  |                                             | |
|   |DATA EAST|   DW06                                      | |
|   |BAC 06   |                                             | |
|J  |---------|                                             | |
|A                             DW19                         |-|
|M                                                            |
|M   DSW1      DSW2        DW18                               |
|A                                                          |-|
|                6116    |---|                              | |
|                        | H |                              | |
|                DW05    | D | DW04                         | |
|                        | 6 |                       2018   | |
|   65C02        YM3812  | 3 | DW03                         | |
|                        | C |                              | |
|   YM2203      YM3014   | 0 | DW02                         |-|
|                YM3014  | 9 |                                |
|        VOL  UPC324     |---| DW01-5           i8751H  8MHz  |
|-------------------------------------------------------------|
Notes:
      2018         - 2K x8 SRAM (DIP24)
      6116         - 2K x8 SRAM (DIP24)
      6264         - 8K x8 SRAM (DIP28)
      6502 CPU clock - 1.500MHz
      6309 CPU clock - 3.000MHz
      YM2203 clock   - 1.500MHz
      8751 clock     - 8.000MHz (contents secured)
      YM3812 clock   - 3.000MHz
      VSync       - 58Hz
      HSync       - 15.68kHz
      ROMs -
            DW00/DW01/DW05      - 27C256
            DW02/DW03/DW04      \
            DW06/DW07/DW08/DW09 / 27C512
            DW18 - Fujitsu MB7132, compatible with Philips 82S181
            DW19 - Fujitsu MB7122, compatible with Philips 82S137


DE-0259-1
|-------------------------------------------------------------|
|                                                             |
|                       2018                                  |
|   2018                                                    |-|
|                       2018                                | |
|                                         2018              | |
|   2018                                   2018             | |
|                                                           | |
|                       DW10                    |-------|   | |
|                                               |       |   | |
|                       DW11  2018              | DRL40 |   |-|
|                                               |       |     |
|                       DW12                    |-------|     |
|                                                           |-|
|                       DW13  2018                          | |
|     VSC30                                     |-------|   | |
|                       DW14                    |       |   | |
|                                               | DRL40 |   | |
|                       DW15  2018              |       |   | |
| HMC20                                         |-------|   | |
|                       DW16                                |-|
|                                                             |
|12MHz                 DW17  2018                             |
|-------------------------------------------------------------|
Notes:
      2018 - 2K x8 SRAM (DIP24)
      All ROMs 27512
      DECO Custom ICs -
                        DECO VSC30 M60348 6102 (DIP40)
                        DECO HMC20 M60232 722001 (DIP28)
                        DATA EAST DRL40 TC17G042AF 8053 8702A (x2, QFP144)
*/

ROM_START( meikyuh )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dw01-5.1d", 0x00000, 0x08000, CRC(87610c39) SHA1(47b83e7decd18f117d00a9f55c42da93b337c04a) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dw02.3d",   0x00000, 0x10000, CRC(40c9b0b8) SHA1(81deb25e00eb4d4c5133ea42cda279c318ee771c) )
	ROM_LOAD( "dw03.4d",   0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) )
	ROM_LOAD( "dw04.6d",   0x20000, 0x10000, CRC(235c0c36) SHA1(f0635f8348459cb8a56eb6184f1bc31c3a82de6a) )
	ROM_FILL(              0x30000, 0x10000, 0xff)

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dw05.5f", 0x0000, 0x8000, CRC(c28c4d82) SHA1(ad88506bcbc9763e39d6e6bb25ef2bd6aa929f30) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dw.1b", 0x0000, 0x1000, CRC(28e9ced9) SHA1(a3d6dfa1e44fa93c0f30fa0a88b6dd3d6e5c4dda) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dw00.16b", 0x00000, 0x8000, CRC(3d25f15c) SHA1(590518460d069bc235b5efebec81731d7a2375de) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dw14.14f", 0x00000, 0x10000, CRC(9b0dbfa9) SHA1(c9db6e70b217a34fbc2bf17da3f5ec6f0130514a) )
	ROM_LOAD( "dw15.15f", 0x10000, 0x10000, CRC(95683fda) SHA1(aa91ad1cd685790e29e16d64bd75a5b4367cf87b) )
	ROM_LOAD( "dw11.9f",  0x20000, 0x10000, CRC(1b1fcca7) SHA1(17e510c1b3efa0f6da49461c286b89295db6b9a6) )
	ROM_LOAD( "dw13.12f", 0x30000, 0x10000, CRC(e7413056) SHA1(62048a9648cbb6b651e3409f77cee268822fd2e1) )
	ROM_LOAD( "dw10.8f",  0x40000, 0x10000, CRC(57667546) SHA1(e7756997ea04204e62404ce8069f8cdb33cb4565) )
	ROM_LOAD( "dw12.1f",  0x50000, 0x10000, CRC(4c548db8) SHA1(988411ab41884c926ca971e7b58f406f85be3b54) )
	ROM_LOAD( "dw16.17f", 0x60000, 0x10000, CRC(e5bcf927) SHA1(b96bd4c124c9745fae1c1f35bdbbdec9f97ab4a5) )
	ROM_LOAD( "dw17.18f", 0x70000, 0x10000, CRC(9e10f723) SHA1(159c5e3d821a10b64cd6d538d19063d0f5b057c0) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "dw06.12f", 0x00000, 0x10000, CRC(b65e029d) SHA1(f8791d57f688f16e0f076361603510e7133f4e36) )
	ROM_LOAD( "dw07.14f", 0x10000, 0x10000, CRC(668d995d) SHA1(dc6221de6103168c8e19f2c6eb159b8989ca2208) )
	ROM_LOAD( "dw08.15f", 0x20000, 0x10000, CRC(bb2cf4a0) SHA1(78806adb6a9ad9fc0707ead567a3220eb2bdb32f) )
	ROM_LOAD( "dw09.17f", 0x30000, 0x10000, CRC(6a528d13) SHA1(f1ef592f1efea637abde26bb8e3d02d552582a43) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dw18.9d",  0x0000, 0x0400, CRC(75f1945f) SHA1(6fa436ae21851ec30847d57c31bdd2fd695e08af)  )
	ROM_LOAD( "dw19.10d", 0x0400, 0x0400, CRC(cc16f3fa) SHA1(4562106ff752f5fc5ae00ff098141e5d74fe4700)  )
ROM_END

/*

Meikyuu Hunter G (Japan, bootleg)

the code is very different, this is a bootleg board. It lacks original labels
and the IC positions are different on the sprite ROMs

this version lacks the linescroll effects when starting the game / demoplay, but the demoplay seems
more complete, whereas in the original the players appear to get stuck before reaching the boss.
Probably bootlegged from a different revision.

CPU
---

CPUs PCB (AT0789A):
1x MC68B09EP (main)
1x 8751H (missing, the socket is empty!)
1x UM6502 (sound)
1x YM2203 (sound)
1x YM3526 (sound)
2x Y3414B (sound)
1x CA324E (sound)
1x oscillator 8.0000MHz

ROMs PCB (AT0789B):
1x oscillator 12.000MHz


ROMs
----

CPUs PCB (AT0789A):
3x P27256
2x TMM24512
5x M27512ZB
3x N82S137N

ROMs PCB (AT0789B):
8x M27512ZB
3x PAL16R4ANC (not dumped)
Note    CPUs PCB (AT0789A):
1x 28x2 JAMMA edge connector
1x trimmer (volume)
2x 8 switches dip
2x 50 pins flat cable connector to ROMs PCB

ROMs PCB (AT0789B):
2x 50 pins flat cable connector to CPUs PCB

------------------------------------
There is a small piggyback attached under CPUs PCB full of 74Sxx

ALL MEMORIES ARE MASK ROMS!

*/

ROM_START( meikyuhbl )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "27256.1d", 0x00000, 0x08000, CRC(d5b5e8a2) SHA1(0155d1d0ddbd764b960148c3c9ef34223e101659) ) // dw-01-5.1d matched 6.552124%

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "24512.3d", 0x00000, 0x10000, CRC(40c9b0b8) SHA1(81deb25e00eb4d4c5133ea42cda279c318ee771c) )
	ROM_LOAD( "24512.4d", 0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) )
	ROM_LOAD( "27512.6d", 0x20000, 0x10000, CRC(8ca6055d) SHA1(37dc5d3b158dc5d7c9677fc4f82e10804181619f) ) // dw-04.6d matched 99.995422% (verified on 2 different PCBs, so almost certainly good)
	ROM_FILL(             0x30000, 0x10000, 0xff)

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "27256.5f", 0x0000, 0x8000, CRC(c28c4d82) SHA1(ad88506bcbc9763e39d6e6bb25ef2bd6aa929f30) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller - should be unpopulated
	ROM_LOAD( "dw.1b", 0x0000, 0x1000, CRC(28e9ced9) SHA1(a3d6dfa1e44fa93c0f30fa0a88b6dd3d6e5c4dda) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "27256.16b", 0x00000, 0x8000, CRC(3d25f15c) SHA1(590518460d069bc235b5efebec81731d7a2375de) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "92.6m", 0x00000, 0x10000, CRC(9b0dbfa9) SHA1(c9db6e70b217a34fbc2bf17da3f5ec6f0130514a) )
	ROM_LOAD( "93.6o", 0x10000, 0x10000, CRC(95683fda) SHA1(aa91ad1cd685790e29e16d64bd75a5b4367cf87b) )
	ROM_LOAD( "89.6i", 0x20000, 0x10000, CRC(1b1fcca7) SHA1(17e510c1b3efa0f6da49461c286b89295db6b9a6) )
	ROM_LOAD( "91.6l", 0x30000, 0x10000, CRC(e7413056) SHA1(62048a9648cbb6b651e3409f77cee268822fd2e1) )
	ROM_LOAD( "88.6h", 0x40000, 0x10000, CRC(57667546) SHA1(e7756997ea04204e62404ce8069f8cdb33cb4565) )
	ROM_LOAD( "90.6k", 0x50000, 0x10000, CRC(4c548db8) SHA1(988411ab41884c926ca971e7b58f406f85be3b54) )
	ROM_LOAD( "94.6p", 0x60000, 0x10000, CRC(e5bcf927) SHA1(b96bd4c124c9745fae1c1f35bdbbdec9f97ab4a5) )
	ROM_LOAD( "95.6r", 0x70000, 0x10000, CRC(9e10f723) SHA1(159c5e3d821a10b64cd6d538d19063d0f5b057c0) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "27512.12f", 0x00000, 0x10000, CRC(b65e029d) SHA1(f8791d57f688f16e0f076361603510e7133f4e36) )
	ROM_LOAD( "27512.14f", 0x10000, 0x10000, CRC(668d995d) SHA1(dc6221de6103168c8e19f2c6eb159b8989ca2208) )
	ROM_LOAD( "27512.15f", 0x20000, 0x10000, CRC(bb2cf4a0) SHA1(78806adb6a9ad9fc0707ead567a3220eb2bdb32f) )
	ROM_LOAD( "27512.17f", 0x30000, 0x10000, CRC(6a528d13) SHA1(f1ef592f1efea637abde26bb8e3d02d552582a43) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD_NIB_LOW(  "82s137.12d", 0x0000, 0x0400, CRC(bf922733) SHA1(c2566b2ad3d7520aa57a1e8027d4832631bd9a72) )
	ROM_LOAD_NIB_HIGH( "82s137.13d", 0x0000, 0x0400, CRC(4ccc328e) SHA1(7d527f5265b65ac070c41e89b39c38c1ba42b544) )
	ROM_LOAD(          "82s137.10d", 0x0400, 0x0400, CRC(cc16f3fa) SHA1(4562106ff752f5fc5ae00ff098141e5d74fe4700) )

	ROM_REGION( 0x600, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "pal16r4anc.16",  0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r4anc.158", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r4anc.165", 0x400, 0x104, NO_DUMP )
ROM_END


void ghostb_state::init_meikyuhbl()
{
	// this bootleg has the high nibble of the first 0x400 bytes with reversed bits.
	// Address it here instead of hacking the DECO RM-C3 device.
	u8 *proms = memregion("proms")->base();

	for (int i = 0; i < 0x400; i++)
		proms[i] = bitswap<8>(proms[i], 4, 5, 6, 7, 3, 2, 1, 0);

	m_palette->update();
}


/******************************************************************************/

} // anonymous namespace

GAME( 1987, gondo,      0,        gondo,     gondo,     gondo_state,    empty_init,     ROT270, "Data East Corporation", "Gondomania (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, gondou,     gondo,    gondo,     gondo,     gondo_state,    empty_init,     ROT270, "Data East USA",         "Gondomania (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, makyosen,   gondo,    gondo,     gondo,     gondo_state,    empty_init,     ROT270, "Data East Corporation", "Makyou Senshi (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, garyoret,   0,        garyoret,  garyoret,  garyoret_state, empty_init,     ROT0,   "Data East Corporation", "Garyo Retsuden (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ghostb,     0,        ghostb,    ghostb,    ghostb_state,   empty_init,     ROT0,   "Data East USA",         "The Real Ghostbusters (US 2 Players, revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ghostb2a,   ghostb,   ghostb,    ghostb2a,  ghostb_state,   empty_init,     ROT0,   "Data East USA",         "The Real Ghostbusters (US 2 Players)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ghostb3,    ghostb,   ghostb,    ghostb3,   ghostb_state,   empty_init,     ROT0,   "Data East USA",         "The Real Ghostbusters (US 3 Players, revision 3B?)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ghostb3a,   ghostb,   ghostb,    ghostb3,   ghostb_state,   empty_init,     ROT0,   "Data East USA",         "The Real Ghostbusters (US 3 Players, revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, meikyuh,    0,        meikyuh,   meikyuh,   ghostb_state,   empty_init,     ROT0,   "Data East Corporation", "Meikyuu Hunter G (Japan)", MACHINE_SUPPORTS_SAVE ) // modified Ghostbusters
GAME( 1987, meikyuhbl,  meikyuh,  meikyuh,   meikyuh,   ghostb_state,   init_meikyuhbl, ROT0,   "bootleg",               "Meikyuu Hunter G (Japan, bootleg)", MACHINE_SUPPORTS_SAVE )
