// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, ywy, 12Me21
#ifndef MAME_TAITO_TAITO_F3_H
#define MAME_TAITO_TAITO_F3_H

#pragma once

#include "taito_en.h"

#include "machine/eepromser.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#include <bitset>


class taito_f3_state : public driver_device
{
public:
	taito_f3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_eeprom(*this, "eeprom"),
		m_spriteram(*this, "spriteram", 0x10000, ENDIANNESS_BIG),
		m_pf_ram(*this, "pf_ram", 0xc000, ENDIANNESS_BIG),
		m_textram(*this, "textram", 0x2000, ENDIANNESS_BIG),
		m_charram(*this, "charram", 0x2000, ENDIANNESS_BIG),
		m_line_ram(*this, "line_ram", 0x10000, ENDIANNESS_BIG),
		m_pivot_ram(*this, "pivot_ram", 0x10000, ENDIANNESS_BIG),
		m_input(*this, "IN.%u", 0),
		m_dial(*this, "DIAL.%u", 0),
		m_eepromin(*this, "EEPROMIN"),
		m_eepromout(*this, "EEPROMOUT"),
		m_taito_en(*this, "taito_en"),
		m_oki(*this, "oki"),
		m_paletteram32(*this, "paletteram"),
		m_okibank(*this, "okibank")
	{ }

	void f3(machine_config &config);
	void f3_224a(machine_config &config);
	void bubsympb(machine_config &config);
	void f3_224b(machine_config &config);
	void f3_224c(machine_config &config);

	void init_commandw();
	void init_pbobble2();
	void init_puchicar();
	void init_intcup94();
	void init_landmakr();
	void init_twinqix();
	void init_elvactr();
	void init_arabianm();
	void init_bubsympb();
	void init_ktiger2();
	void init_lightbr();
	void init_gekirido();
	void init_arkretrn();
	void init_kirameki();
	void init_qtheater();
	void init_popnpop();
	void init_spcinvdj();
	void init_pbobbl2p();
	void init_landmkrp();
	void init_bubblem();
	void init_ridingf();
	void init_gseeker();
	void init_bubsymph();
	void init_hthero95();
	void init_gunlock();
	void init_pbobble4();
	void init_dariusg();
	void init_recalh();
	void init_kaiserkn();
	void init_spcinv95();
	void init_trstaroj();
	void init_ringrage();
	void init_cupfinal();
	void init_quizhuhu();
	void init_pbobble3();
	void init_cleopatr();
	void init_scfinals();
	void init_pbobbl2x();

	template <int Num> ioport_value f3_analog_r();
	template <int Num> ioport_value f3_coin_r();
	ioport_value eeprom_read();

protected:
	using fixed8 = s32;

	// should be 30.47618_MHz_XTAL / 2
	static inline constexpr XTAL F3_MAIN_CLK = 16_MHz_XTAL;

	static inline constexpr int H_TOTAL = 432;
	static inline constexpr int H_VIS   = 320;
	static inline constexpr int H_START = 46;
	static inline constexpr int V_TOTAL = 262;
	static inline constexpr int V_VIS   = 232;
	static inline constexpr int V_START = 24;

	static inline constexpr int NUM_PLAYFIELDS = 4;
	static inline constexpr int NUM_TILEMAPS = 5;
	static inline constexpr int NUM_SPRITEGROUPS = 4; // high 2 bits of color
	static inline constexpr int NUM_CLIPPLANES = 4;

	struct F3config;
	/* This it the best way to allow game specific kludges until the system is fully understood */
	enum {
		/* Early F3 class games, these are not cartridge games and system features may be different */
		RINGRAGE=0, /* D21 */
		ARABIANM,   /* D29 */
		RIDINGF,    /* D34 */
		GSEEKER,    /* D40 */
		TRSTAR,     /* D53 */
		GUNLOCK,    /* D66 */
		TWINQIX,
		UNDRFIRE,   /* D67 - Heavily modified F3 hardware (different memory map) */
		SCFINALS,
		LIGHTBR,    /* D69 */

		/* D77 - F3 motherboard proms, all following games are 'F3 package system' */
		/* D78 I CUP */
		KAISERKN,   /* D84 */
		DARIUSG,    /* D87 */
		BUBSYMPH,   /* D90 */
		SPCINVDX,   /* D93 */
		HTHERO95,   /* D94 */
		QTHEATER,   /* D95 */
		EACTION2,   /* E02 */
		SPCINV95,   /* E06 */
		QUIZHUHU,   /* E08 */
		PBOBBLE2,   /* E10 */
		GEKIRIDO,   /* E11 */
		KTIGER2,    /* E15 */
		BUBBLEM,    /* E21 */
		CLEOPATR,   /* E28 */
		PBOBBLE3,   /* E29 */
		ARKRETRN,   /* E36 */
		KIRAMEKI,   /* E44 */
		PUCHICAR,   /* E46 */
		PBOBBLE4,   /* E49 */
		POPNPOP,    /* E51 */
		LANDMAKR,   /* E61 */
		RECALH,     /* prototype */
		COMMANDW,   /* prototype */
		TMDRILL
	};

	static const F3config f3_config_table[];

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void device_post_load(void) override;

	TIMER_CALLBACK_MEMBER(trigger_int3);

	required_device<cpu_device> m_maincpu;
	optional_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<eeprom_serial_base_device> m_eeprom;

	memory_share_creator<u16> m_spriteram;
	memory_share_creator<u16> m_pf_ram;
	memory_share_creator<u16> m_textram;
	memory_share_creator<u16> m_charram;
	memory_share_creator<u16> m_line_ram;
	memory_share_creator<u16> m_pivot_ram;
	u16 m_control_0[8]{};
	u16 m_control_1[8]{};

	optional_ioport_array<6> m_input;
	optional_ioport_array<2> m_dial;
	optional_ioport m_eepromin;
	optional_ioport m_eepromout;

	emu_timer *m_interrupt3_timer;
	u32 m_coin_word[2];
	std::unique_ptr<u8[]> m_decoded_gfx4;
	std::unique_ptr<u8[]> m_decoded_gfx5;

	struct tempsprite {
		int code; // 17 bits
		u8 color;
		bool flip_x, flip_y;
		fixed8 x, y;
		fixed8 scale_x, scale_y;
		u8 pri;
	};

	struct clip_plane_inf {
		s16 l;
		s16 r;

		clip_plane_inf() { l = 0; r = 0; }
		clip_plane_inf(s16 left, s16 right)
		{
			l = left;
			r = right;
		}
		clip_plane_inf& set_upper(s8 left, s8 right)
		{
			l = (l & 0xff) | left<<8;
			r = (r & 0xff) | right<<8;
			return *this;
		}
		clip_plane_inf& set_lower(u8 left, u8 right)
		{
			l = (l & 0x100) | left;
			r = (r & 0x100) | right;
			return *this;
		}
	};

	struct draw_source {
		draw_source() { }
		draw_source(bitmap_ind16 *bitmap)
		{
			src = bitmap;
			flags = nullptr;
		}
		draw_source(tilemap_t *tilemap)
		{
			if (!tilemap)
				return;
			src = &tilemap->pixmap();
			flags = &tilemap->flagsmap();
		};
		bitmap_ind16 *src{nullptr};
		bitmap_ind8  *flags{nullptr};
	};

	struct mix_pix { // per-pixel information for the blending circuit
		u16 src_pal[H_TOTAL];
		u16 dst_pal[H_TOTAL];
		u8  src_blend[H_TOTAL];
		u8  dst_blend[H_TOTAL];
	};

	struct f3_line_inf;

	struct mixable {// layer compositing information
		draw_source bitmap;
		bool x_sample_enable{false};
		u16 mix_value{0};
		u8 prio{0};
		u8 blend_mode;

		u8 index{0};

		void set_mix(u16 v) { mix_value = v; prio = v & 0xf; blend_mode = BIT(mix_value, 14, 2); };
		void set_prio(u8 p) { mix_value = (mix_value & 0xfff0) | p; prio = p; };
		void set_blend(u8 b) { mix_value = (mix_value & 0x3fff) | (b << 14); blend_mode = b; };
		auto clip_inv() const { return std::bitset<4>(mix_value >> 4); }
		auto clip_enable() const { return std::bitset<4>(mix_value >> 8); }
		bool clip_inv_mode() const { return mix_value & 0x1000; }
		bool layer_enable() const;
		bool blend_a() const { return mix_value & 0x4000; }
		bool blend_b() const { return mix_value & 0x8000; }

		bool operator<(const mixable &rhs) const noexcept { return this->prio < rhs.prio; }
		bool operator>(const mixable &rhs) const noexcept { return this->prio > rhs.prio; }

		u16 palette_adjust(u16 pal) const { return pal; }
		int y_index(int y) const;
		int x_index(int x) const;
		bool blend_select(const u8 *line_flags, int x) const { return false; }
		bool inactive_group(u16 color) const { return false; }

		static const char *debug_name() { return "MX"; }
	};

	struct sprite_inf : mixable {
		// alpha mode in 6000
		// mosaic enable in 6400
		// line enable, clip settings in 7400
		// priority in 7600
		bool blend_select_v{false}; // 7400 0xf000
		u8 (*sprite_pri_usage)[256]{};

		bool blend_select(const u8 *line_flags, int x) const { return blend_select_v; }
		bool layer_enable() const;
		bool inactive_group(u16 color) const { return BIT(color, 10, 2) != index; }

		static const char *debug_name() { return "SP"; };
	};

	struct pivot_inf : mixable {
		u8 pivot_control{0};     // 6000
		bool blend_select_v{false};
		// mosaic enable in 6400
		u16 pivot_enable{0}; // 7000 - what is in this word ?
		// mix info from 7200

		u16 reg_sx{0};
		u16 reg_sy{0};

		bool blend_select(const u8 *line_flags, int x) const { return blend_select_v; }
		bool use_pix() const { return pivot_control & 0xa0; }

		int y_index(int y) const;
		int x_index(int x) const;
		static const char *debug_name() { return "PV"; }
	};

	struct playfield_inf : mixable {
		u16 colscroll{0};            // 4000
		bool alt_tilemap{false};     // 4000
		// mosaic enable in 6400
		fixed8 x_scale{0x80};        // 8000
		fixed8 y_scale{0};           // 8000
		u16 pal_add{0};              // 9000
		fixed8 rowscroll{0};         // a000

		fixed8 reg_sx{0};
		fixed8 reg_sy{0};
		fixed8 reg_fx_y{0};
		fixed8 reg_fx_x{0};

		u16 width_mask{0};

		u16 palette_adjust(u16 pal) const;
		int y_index(int y) const;
		int x_index(int x) const;
		bool blend_select(const u8 *line_flags, int x) const { return BIT(line_flags[x], 0); }
		static const char *debug_name() { return "PF"; }
	};

	struct pri_mode {
		u8 src_prio[H_TOTAL]{};
		u8 dst_prio[H_TOTAL]{};
		u8 src_blendmode[H_TOTAL]{};
		u8 dst_blendmode[H_TOTAL]{};
	};

	struct f3_line_inf {
		int y{0};
		int screen_y{0};
		pri_mode pri_alp{};
		// 5000/4000
		clip_plane_inf clip[NUM_CLIPPLANES];
		// 6000 - pivot_control, sprite alpha
		u16 maybe_sync_reg{0};
		bool no_opaque_dest{false};
		// 6200
		u8 blend[4]{}; // less 0 - 8 more
		// 6400
		u8 x_sample{16 - 0}; // mosaic effect
		u8 fx_6400{0}; // unemulated other effects (palette interpretation + unused bits)
		// 6600
		u16 bg_palette{0}; // always palette 0 in existing games
		// 7200
		pivot_inf pivot;
		sprite_inf sp[NUM_SPRITEGROUPS];
		playfield_inf pf[NUM_PLAYFIELDS];
	};

	int m_game = 0;
	tilemap_t *m_tilemap[8] = {nullptr};
	tilemap_t *m_pixel_layer = nullptr;
	tilemap_t *m_vram_layer = nullptr;
	bool m_flipscreen = false;
	bool m_extend = false;
	u8 m_sprite_extra_planes = 0;
	u8 m_sprite_pen_mask = 0;
	bool m_sprite_trails = false;
	u16 *m_pf_data[8]{};
	int m_sprite_lag = 0;
	u8 m_textram_row_usage[64]{};
	u8 m_sprite_pri_row_usage[256]{};
	u8 m_tilemap_row_usage[32][8]{};
	bitmap_ind8 m_pri_alp_bitmap;
	bitmap_ind16 m_sprite_framebuffer{};
	u16 m_width_mask = 0;
	u8 m_twidth_mask = 0;
	u8 m_twidth_mask_bit = 0;
	std::unique_ptr<tempsprite[]> m_spritelist;
	const tempsprite *m_sprite_end = nullptr;
	bool m_sprite_bank = 0;
	//f3_line_inf m_line_data{};
	const F3config *m_game_config = nullptr;

	u16 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pf_ram_r(offs_t offset);
	void pf_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 textram_r(offs_t offset);
	void textram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 charram_r(offs_t offset);
	void charram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 lineram_r(offs_t offset);
	void lineram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pivot_r(offs_t offset);
	void pivot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void control_0_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void control_1_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info_text);
	TILE_GET_INFO_MEMBER(get_tile_info_pixel);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void bubsympb_map(address_map &map) ATTR_COLD;
	void f3_map(address_map &map) ATTR_COLD;

	void tile_decode();

	void create_tilemaps(bool extend);

	inline void f3_drawgfx(const tempsprite &sprite, const rectangle &cliprect);
	void get_sprite_info();
	void draw_sprites(const rectangle &cliprect);
	void get_pf_scroll(int pf_num, fixed8 &reg_sx, fixed8 &reg_sy);
	void read_line_ram(f3_line_inf &line, int y);
	void render_line(pen_t *dst, const mix_pix &z);
	void scanline_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	template<typename Mix>
	std::vector<clip_plane_inf> calc_clip(const clip_plane_inf (&clip)[NUM_CLIPPLANES], const Mix &layer);
	inline bool used(const pivot_inf &layer, int y) const;
	inline bool used(const sprite_inf &layer, int y) const;
	inline bool used(const playfield_inf &layer, int y) const;
	template<typename Mix>
	bool mix_line(const Mix &gfx, mix_pix &z, pri_mode &pri, const f3_line_inf &line, const clip_plane_inf &range);

private:
	optional_device<taito_en_device> m_taito_en;
	optional_device<okim6295_device> m_oki;

	optional_shared_ptr<u32> m_paletteram32;
	optional_memory_bank m_okibank;

	void bubsympb_oki_w(u8 data);
	u32 f3_control_r(offs_t offset);
	void f3_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void f3_timer_control_w(offs_t offset, u16 data);
	void sound_reset_0_w(u32 data);
	void sound_reset_1_w(u32 data);
	void sound_bankswitch_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void palette_24bit_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	INTERRUPT_GEN_MEMBER(interrupt2);

	void bubsympb_oki_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TAITO_TAITO_F3_H
