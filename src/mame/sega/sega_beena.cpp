// license:BSD-3-Clause
// copyright-holders:R. Belmont, QUFB
/******************************************************************************

    sega_beena.cpp

    Driver for the Sega Toys' Advanced Pico BEENA

    TODO:

        MIDI audio
        Peripherals (including the SD-Card adapter)
        Component list / PCB diagram
        Fix graphical glitches (e.g. 'Car Beena' scrolling background)
        Fix test mode for 'Car Beena' (fails on 'Test EEP')

    Hardware
    --------

    The PCB includes a custom SoC marked with 'Sega Toys 9H0-0008' but manufactured by Applause Technologies.

    There are 2 known PCB revisions, which only have minor differences unrelated to gameplay,
    yielding identical BIOS dumps:

        9B0-0007G / 702810006.03GA SW1529 (Rev. 1)
        9B0-0007F / 702810004.02A  SW1404 (Rev. 4)

    A headerless but fully functional JTAG port is present, following the ARM standard pinout:

         20  19
       -----------
        GND  0
          0  0
        GND  nSRST
        GND  TDO
        GND  RTCK
        GND  TCLK
        GND  TMS
        GND  TDI
        GND  nTRST
        3.3  0
       -----------
          2  1

    Both BIOS and Cartridge ROMs have 'edinburgh' in their headers, which is a codename that also appears
    marked on the SoC's development board as 'EDINBURGH_EVAL V1.0'. A non Sega Toys version of the SoC was sold,
    marked as 'Applause Technologies AP2010' with codename 'LANCELOT'. They appear to have identical features,
    with the only difference presumably being the contents of the internal ROM.

    Cartridge ROMs are also full of .ogg files containing the string 'Encoded with Speex speex-1.0.4'
    as well as .au files for sounds and .mid files for music.

    Cartridges pinout:

        Glob down, PCB cut corner at upper right.

        Top row of pins A25 on left to A1 on right.
        Bottom row of pins B25 on left to B1 on right.

        A1  /CE (tied high with resistor)
        A2  D11
        A3  D3
        A4  D10
        A5  D2
        A6  D9
        A7  D1
        A8  D8
        A9  D0
        A10 /OE
        A11 N/C
        A12 A0
        A13 A1
        A14 A2
        A15 A3
        A16 A4
        A17 A5
        A18 A6
        A19 A7
        A20 A17
        A21 A18
        A22 N/C
        A23 VCC
        A24 N/C
        A25 GND

        B1  CE (for Flashrom)
        B2  D4
        B3  D12
        B4  D5
        B5  D13
        B6  D6
        B7  D14
        B8  D7
        B9  D15
        B10 A16
        B11 A15
        B12 A14
        B13 A13
        B14 A12
        B15 A11
        B16 A10
        B17 A9
        B18 A8
        B19 A19
        B20 A20
        B21 WE (for Flashrom)
        B22 A21
        B23 VCC
        B24 N/C
        B25 GND

    Storyware
    ---------

    Toggling 'Pen Target' input switches between mapping pen coordinates to the tablet or the book.

    Test Mode
    ---------

    Most games contain a hidden test mode that can be activated by the same inputs:

        Pages 1, 3, and 5 covered, the others exposed;
        Left red button held down;

    If the machine configuration 'Test Mode Pages' is enabled, the driver forces this page setup,
    so that the player only needs to hold the left red button at reset to activate this mode.

    Toggling 'Memory Cache' allows the player to observe differences between test failure and success.

    For 'TV Ocha-Ken', hold down A B C, then power on the system, release all buttons, and press B 3 times.

    For 'Car Beena', hold down all 3 buttons, turn the handle left to full lock, then power on the system.

*******************************************************************************/

#include "emu.h"

#include "tvochken_card.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/arm7/ap2010cpu.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "sound/ap2010pcm.h"

#include "crsshair.h"
#include "emupal.h"
#include "render.h"
#include "softlist_dev.h"
#include "schedule.h"
#include "speaker.h"
#include "screen.h"

#include "beena.lh"
#include "tvochken.lh"

#define VERBOSE (0)
#include "logmacro.h"


namespace {

class sega_9h0_0008_state : public driver_device
{
public:
	sega_9h0_0008_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_workram(*this, "workram")
		, m_pcm(*this, "pcm")
		, m_screen_main(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_paletteram(*this, "paletteram")
		, m_tilemap_bg(*this, "tilemap_bg")
		, m_tilemap_fg(*this, "tilemap_fg")
		, m_tilemap_scroll_x(*this, "tilemap_scroll_x")
		, m_tilemap_scroll_y(*this, "tilemap_scroll_y")
		, m_tilemap_sprites(*this, "tilemap_sprites")
		, m_bitmap(*this, "bitmap")
		, m_io_sensor_regs(*this, "io_sensor_regs")
		, m_io_auxiliary_regs(*this, "io_auxiliary_regs")
		, m_io_cpu_config(*this, "CPU_CONFIG")
		, m_io_video_config(*this, "VIDEO_CONFIG")
	{ }

protected:
	static inline constexpr uint32_t ROM_MASK_BASE = 0x80000000;
	static inline constexpr uint32_t ROM_FLASH_BASE = 0xa0000000;

	void sega_9h0_0008(machine_config &config);

	virtual void device_post_load() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	virtual void install_game_rom();
	virtual void update_sensors(offs_t offset);

	void beena_arm7_map(address_map &map);

	void request_irq();
	void request_fiq();

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	virtual void update_crosshair(screen_device &screen);
	void irq_wait_speedup();
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t video_reg_r(offs_t offset);
	void video_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void pal_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t tiles_layers_r(offs_t offset);
	uint8_t tiles_sprites_r(offs_t offset);
	void tiles_layers_w(offs_t offset, uint8_t data);
	void tiles_sprites_w(offs_t offset, uint8_t data);
	int32_t scroll_x(int32_t x, uint16_t i);
	int32_t scroll_y(int32_t y, uint16_t i);
	void draw_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint16_t *tilemap, const uint8_t scroll_idx, const bool is_active, const bool is_overlay_rendered);
	void draw_layer_tiles(bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint16_t *tilemap, const uint8_t scroll_idx, const bool is_overlay_rendered);
	void draw_layer_scanlines(bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint16_t *tilemap, const uint8_t scroll_idx, const bool is_overlay_rendered);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, const bool is_overlay_rendered);
	void draw_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_blend(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int32_t rescale_alpha_step(uint8_t step);
	template <typename BitmapType, typename FunctionClass>
	void drawgfxzoom_with_pixel_op(gfx_element *gfx, BitmapType &dest, const rectangle &cliprect, uint32_t code, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, FunctionClass &&pixel_op);

	uint32_t io_sensors_r(offs_t offset);

	uint32_t io_memcache_r();
	virtual uint32_t io_expansion_r();
	void memcache_advance(uint32_t &status);
	void memcache_parse_data_bit(uint32_t &status);

	uint32_t rtc_r(offs_t offset);
	void rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void update_rtc();

	uint32_t midi_reg_r(offs_t offset);
	void midi_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	static constexpr int16_t signed10(uint32_t number)
	{
		return util::sext(number, 10);
	}
	static constexpr int32_t rescale(int32_t x, int32_t min_x, int32_t max_x, int32_t a, int32_t b)
	{
		// Rescaling (min-max normalization) from [min_x..max_x] to [a..b].
		return a + (((x - min_x) * (b - a)) / (max_x - min_x));
	}
	static constexpr uint32_t alpha_blend_rgb_levels(uint32_t dst, uint32_t src, uint8_t level_b, uint8_t level_g, uint8_t level_r)
	{
		// Similar to drawgfx::alpha_blend_r32(), but distinct levels are applied to each channel.
		return ((((src & 0x0000ff) * level_r + (dst & 0x0000ff) * int(256 - level_r)) >> 8)) |
				((((src & 0x00ff00) * level_g + (dst & 0x00ff00) * int(256 - level_g)) >> 8) & 0x00ff00) |
				((((src & 0xff0000) * level_b + (dst & 0xff0000) * int(256 - level_b)) >> 8) & 0xff0000);
	}

	required_device<ap2010cpu_device> m_maincpu;
	required_shared_ptr<uint32_t> m_workram;
	required_device<ap2010pcm_device> m_pcm;
	bool m_requested_fiq;
	uint32_t m_irq_wait_start_addr;

	required_device<screen_device> m_screen_main;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	std::unique_ptr<rgb_t[]> m_cache_palette;
	required_shared_ptr<uint32_t> m_paletteram;
	required_shared_ptr<uint32_t> m_tilemap_bg;
	required_shared_ptr<uint32_t> m_tilemap_fg;
	required_shared_ptr<uint32_t> m_tilemap_scroll_x;
	required_shared_ptr<uint32_t> m_tilemap_scroll_y;
	required_shared_ptr<uint32_t> m_tilemap_sprites;
	required_shared_ptr<uint32_t> m_bitmap;
	std::unique_ptr<uint8_t[]> m_tiles_layers;
	std::unique_ptr<uint8_t[]> m_tiles_sprites;
	std::unique_ptr<uint32_t[]> m_video_regs;
	bitmap_rgb32 m_cache_layer; // Used for x-scrolling scanlines, same size as bitplane
	uint8_t m_scale;
	uint16_t m_scanline;

	required_shared_ptr<uint32_t> m_io_sensor_regs;
	required_shared_ptr<uint32_t> m_io_auxiliary_regs;
	uint32_t m_pen_target;
	uint32_t m_effective_page;

	uint16_t m_rtc[0x100/2]{};

	std::unique_ptr<uint32_t[]> m_midi_regs;
	uint32_t m_midi_busy_count;

	required_ioport m_io_cpu_config;
	required_ioport m_io_video_config;

private:
	static inline constexpr uint16_t SCREEN_W = 704;
	static inline constexpr uint16_t SCREEN_H = 480;

	static inline constexpr uint32_t UNKNOWN_ADDR = 0xffffffff;

	static inline constexpr uint16_t MEMCACHE_FIFO_MAX_SIZE = 0x100;

	enum memcache_seq : uint32_t
	{
		BITS_EMPTY = 0,
		BITS_0     = 0x0000ff00,
		BITS_0_1   = 0x00ff0001, // Command 0xa0 store bit 0
								 // Command 0xa1 move to next bit?
		BITS_0_1_7 = 0xff000107, // Reset
		BITS_6     = 0x0000ff06, // Query (1 if ready for next events)
		BITS_6_7   = 0x00ff0607, // Query (0 if events parsed successfully)
								 // Command 0xa0 store bit 1
								 // Command 0xa1 load bit
		BITS_7     = 0x0000ff07,
		BITS_7_1   = 0x00ff0701, // Start command
		BITS_ERR   = 0xffffffff,
	};

	enum memcache_state : uint8_t
	{
		IDLE = 0,
		READ_COMMAND,
		READ_ADDRESS_BYTE,
		READ_ACK,
		READ_RAM_BYTE,
		WRITE_RAM_BYTE,
		CONSUME_UNTIL_IDLE,
	};

	uint8_t fifo_state_after_pop();
	uint8_t fifo_events_pop();
	void fifo_state_after_push(uint8_t state);
	void fifo_events_push(uint8_t event);

	uint8_t m_memcache[0x800]{};

	uint32_t m_memcache_seq;

	uint32_t m_memcache_addr;
	uint8_t m_memcache_data;
	uint8_t m_memcache_i;
	uint8_t m_memcache_state;

	uint8_t m_fifo_state_after_data[MEMCACHE_FIFO_MAX_SIZE]{};
	uint16_t m_fifo_state_after_size = 0;
	uint16_t m_fifo_state_after_head = 0;
	uint16_t m_fifo_state_after_tail = 0;

	uint8_t m_fifo_events_data[MEMCACHE_FIFO_MAX_SIZE]{};
	uint16_t m_fifo_events_size = 0;
	uint16_t m_fifo_events_head = 0;
	uint16_t m_fifo_events_tail = 0;
};

/*
    FIXME: All of these have guessed timing.

    Animation durations in hardware captures suggest that an IRQ is raised every VBLANK.
    Palette changes can occur for each scanline via FIQ callback set by games.

    These variables in BIOS RAM may be worth following:
    - 0x20003ee0: FIQ enabled;
    - 0x20003ee1: IRQ enabled;
    Along with pairs used by interrupt callbacks:
    - 0x20003ed1..0x20003ed4: status;
    - 0x20003ee2..0x20003ee5: 1 if busy?
*/
TIMER_DEVICE_CALLBACK_MEMBER(sega_9h0_0008_state::scanline)
{
	irq_wait_speedup();

	// Pixel clock approximation derived from palette lookup tables applied per scanline,
	// used for gradient backgrounds in "Partner In TV".
	//
	// FIXME:
	// - Rainbow arc in "Cars 2" before bridge section is not rendered;
	// - Foreground drop animation in "Cooking Beena" title screen is skipped;
	m_scanline = param / m_scale;
	m_video_regs[0x4/4] = 0x10000 * (20 + m_scanline);

	uint8_t irq_frequency = m_io_cpu_config->read() & 0xf;
	if ((m_scanline % (SCREEN_H / irq_frequency)) == 0) {
		uint8_t video_status = (m_video_regs[0] & 0x10) ^ 0x10;
		m_video_regs[0] = (m_video_regs[0] & 0xffffffef) | video_status;

		update_rtc();

		request_irq();

		// FIXME: Needed for .au data processing in "Kazoku Minna no Nouryoku Trainer"
		m_requested_fiq = true;
	}

	request_fiq();
}

void sega_9h0_0008_state::irq_wait_speedup()
{
	if ((m_io_cpu_config->read() & 0x10) == 0x10) {
		return;
	}

	/*
	    All games execute a busy wait until the next IRQ request is served.
	    This can lead to significant downgrade of emulation speed.

	    The busy wait subroutine is copied to a dynamic location in work RAM,
	    somewhere after 0xc00cc000, but before the stack pointer. r0 stores
	    an address to a variable that is updated by the game's IRQ callback
	    when video data has been processed:

	        e3 a0 30 01   mov     r3,#0x1
	        e5 c0 30 00   strb    r3,[r0,#0x0]
	        e5 d0 30 00   ldrb    r3,[r0,#0x0]
	        e3 53 00 00   cmp     r3,#0x0
	        1a ff ff fc   bne     LAB_c00ce8bc

	    Epilogue is the following for most games:

	        e5 9f 30 00   ldr     r3,[DAT_c00ce8d0] = 80000000h
	        e5 93 f0 08   ldr     pc=>LAB_c00fff80,[r3,#offset ->SP]

	    But slightly different in early games:

	        e1 2f ff 1e   bx      lr

	    Since this code has a predictable byte signature, we can search
	    in memory to find its exact start address, then consume enough cycles to
	    reduce the number of instructions executed until the next IRQ is asserted.
	*/
	if (m_irq_wait_start_addr == UNKNOWN_ADDR) {
		if (m_maincpu->pc() > 0xc00cc000 && m_maincpu->pc() < 0xc00fff80) {
			const uint32_t IRQ_WAIT_SIGNATURE[] = {
				0xe3a03001,
				0xe5c03000,
				0xe5d03000,
				0xe3530000,
				0x1afffffc
			};
			int8_t addr_delta = 8;
			uint32_t *shared32 = reinterpret_cast<uint32_t *>(m_workram.target());
			uint32_t candidate_start_addr = m_maincpu->pc() - addr_delta;
			uint32_t candidate_offset = (candidate_start_addr - 0xc00cc000) / 4;
			for (size_t i = 0; i < addr_delta; i++) {
				bool matched = true;
				for (size_t sig_i = 0; sig_i < 5; sig_i++) {
					if (IRQ_WAIT_SIGNATURE[sig_i] != shared32[candidate_offset + i + sig_i]) {
						matched = false;
						break;
					}
				}
				if (matched) {
					m_irq_wait_start_addr = candidate_start_addr + i;

					for (size_t sig_i = 0; sig_i < 5; sig_i++) {
						m_maincpu->add_hotspot(candidate_start_addr + i + sig_i * 4);
					}
				}
			}
		}
	}
}

static const gfx_layout sega_beena_8bpp_layout =
{
	16,16,
	0x800, // 0x800 * 0x10 * 0x10 = 0x80000 (tile data mapping size)
	8,
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	{ STEP16(0, 8*16) },
	8*16*16
};

void sega_9h0_0008_state::beena_arm7_map(address_map &map)
{
	// BIOS internal ROM
	map(0x00000000, 0x0001ffff).rom();
	// BIOS internal RAM
	map(0x20000000, 0x20003fff).ram();

	// FIXME: Need to confirm upper bounds / mirrored ranges on hardware, some return inconsistent reads
	// Video (registers, palette, sprite data...)
	map(0x40000000, 0x400000ff).mirror(0xff00).rw(FUNC(sega_9h0_0008_state::video_reg_r), FUNC(sega_9h0_0008_state::video_reg_w));
	map(0x40010000, 0x400103ff).ram().share("tilemap_sprites");
	map(0x40010400, 0x4001ffff).ram();
	map(0x40020000, 0x400201ff).w(FUNC(sega_9h0_0008_state::pal_w)).share("paletteram");
	map(0x40020200, 0x4002ffff).ram();
	map(0x40030000, 0x400302ff).ram().share("tilemap_scroll_y");
	map(0x40030300, 0x4003ffff).ram();

	// I/O (registers, pages, pads, pen...)
	map(0x50000000, 0x5000000f).mirror(0xfff0).ram(); // Unknown
	map(0x50010000, 0x5001003f).mirror(0xffc0).lrw32(
		NAME([this](offs_t offset) {
			LOG("audio_pcm_reg_r @ %08x: addr=%08x data=%08x\n", m_maincpu->pc(), offset * 4, m_pcm->reg_r(offset));

			return m_pcm->reg_r(offset);
		}),
		NAME([this](offs_t offset, uint32_t data, uint32_t mem_mask) {
			LOG("audio_pcm_reg_w @ %08x: addr=%08x data=%08x mask=%08x\n", m_maincpu->pc(), offset * 4, data, mem_mask);

			// FIXME: Games hang during .au data processing unless FIQ requests are served,
			// review this after playback is implemented
			if (offset == 0x10/4) {
				if (data != 0) {
					m_requested_fiq = true;
				}
			}

			m_pcm->reg_w(offset, data, mem_mask);
		}));
	map(0x50020000, 0x5002007f).mirror(0xff80).r(FUNC(sega_9h0_0008_state::io_sensors_r)).share("io_sensor_regs");
	map(0x50030000, 0x500300ff).mirror(0xff00).lrw32(
		NAME([this](offs_t offset) {
			LOG("m_io_auxiliary_regs_r @ %08x: addr=%08x\n", m_maincpu->pc(), offset * 4);

			if (offset == 0x0c/4) {
				return io_expansion_r();
			}
			return offset == 0xbc/4 ? io_memcache_r() : m_io_auxiliary_regs[offset];
		}),
		NAME([this](offs_t offset, uint32_t data, uint32_t mem_mask) {
			LOG("m_io_auxiliary_regs_w @ %08x: addr=%08x data=%08x mask=%08x\n", m_maincpu->pc(), offset * 4, data, mem_mask);

			COMBINE_DATA(&m_io_auxiliary_regs[offset]);
			if (offset == 0xc0/4) {
				fifo_events_push(data);
			}
		})).share("io_auxiliary_regs");

	// Realtime clock
	map(0x60000000, 0x600000ff).mirror(0xff00).rw(FUNC(sega_9h0_0008_state::rtc_r), FUNC(sega_9h0_0008_state::rtc_w));
	map(0x60010000, 0x6003ffff).ram(); // Unknown

	// MIDI
	map(0x70000000, 0x7000001f).mirror(0xffe0).rw(FUNC(sega_9h0_0008_state::midi_reg_r), FUNC(sega_9h0_0008_state::midi_reg_w));
	map(0x70010000, 0x7003ffff).ram(); // Unknown

	// Game Mask ROM
	map(ROM_MASK_BASE, ROM_MASK_BASE + 0x7fffff).mirror(0x800000).lr8(NAME([]() { return 0xff; }));

	// Game NOR Flash ROM
	// Some games have an unreferenced handler that jumps to 0xa0ffc000;
	map(ROM_FLASH_BASE, ROM_FLASH_BASE + 0x7fffff).mirror(0x800000).lr8(NAME([]() { return 0xff; }));

	// Direct bitmap with 2-byte BGR555 colors per pixel, same as tile palettes. Scaling also applied.
	// The bitmap size is checked by games to be `w * h <= 0x63000`, so we allocate `0x63000 * 2 = 0xc6000`.
	map(0xc0000000, 0xc00c5fff).ram().share("bitmap");
	// Tilemap data for 2 layers (background and foreground).
	map(0xc00c6000, 0xc00c68ff).ram().share("tilemap_scroll_x");
	// Work RAM for tilemap data
	map(0xc00c6900, 0xc00c7fff).ram();
	map(0xc00c8000, 0xc00c9fff).ram().share("tilemap_fg");
	map(0xc00ca000, 0xc00cbfff).ram().share("tilemap_bg");
	// Work RAM for general use + stack (usually starts at 0xc00fff80)
	map(0xc00cc000, 0xc00fffff).ram().share("workram");
	// Tile data for 2 layers (background and foreground) + sprites
	map(0xc0100000, 0xc017ffff).ram().rw(FUNC(sega_9h0_0008_state::tiles_layers_r), FUNC(sega_9h0_0008_state::tiles_layers_w));
	map(0xc0180000, 0xc01fffff).ram().rw(FUNC(sega_9h0_0008_state::tiles_sprites_r), FUNC(sega_9h0_0008_state::tiles_sprites_w));
}

void sega_9h0_0008_state::update_sensors(offs_t offset)
{
	// Power status
	// - bit 1 = 0: a "low battery" screen shows up;
	// - bit 2 = 0: game code goes into infinite loop, maybe power button pressed?
	if (offset == 0x38/4) {
		m_io_sensor_regs[offset] = 3;
	}
}

uint32_t sega_9h0_0008_state::io_sensors_r(offs_t offset)
{
	if (!machine().side_effects_disabled()) {
		update_sensors(offset);
	}

	return m_io_sensor_regs[offset];
}

uint32_t sega_9h0_0008_state::video_reg_r(offs_t offset)
{
	return m_video_regs[offset];
}

void sega_9h0_0008_state::video_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("video_reg_w @ %08x: addr=%08x data=%08x mask=%08x\n", m_maincpu->pc(), offset * 4, data, mem_mask);

	/*
	   Video control
	   - bit 1..4: rendering toggles
	     - 0x1: enable rendering
	     - 0x2: enable fg layer
	     - 0x4: enable bg layer
	   - bit 5..8: scroll data entries
	     - 0x10: 0x900 for x-axis scanlines
	     - 0x20:  0x90 for x-axis tiles
	     - 0x40:  0xb4 for y-axis tiles
	   - bit 9..12: video format
	     - 0x100: vertical frequency: 60Hz (NTSC) if 0, 50Hz (PAL) if 1
	     - 0x200: mode: 352x240 (scales 2x from top-left corner) if 0, 704x480 if 1
	   - bit 13..16: direct bitmap render priority
	     - 0x0000: before overlay sprites
	     - 0x1000: before all sprites
	     - 0x2000: before fg layer
	     - 0x3000: before bg and fg layers
	*/
	if (offset == 0x10/4) {
		m_scale = ((data & 0x200) == 0) ? 2 : 1;
	}

	COMBINE_DATA(&m_video_regs[offset]);
}

uint32_t sega_9h0_0008_state::midi_reg_r(offs_t offset)
{
	if (offset == 0 && m_midi_busy_count > 0) {
		if (!machine().side_effects_disabled()) {
			m_midi_busy_count--;
		}
		return 0x2;
	}

	return m_midi_regs[offset];
}

void sega_9h0_0008_state::midi_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("midi_reg_w @ %08x: addr=%08x data=%08x mask=%08x\n", m_maincpu->pc(), offset * 4, data, mem_mask);

	switch (offset) {
		case 0x4/4:
			if ((data & 5) == 5) {
				// TODO: MIDI all notes off
			}
			break;
		case 0x8/4:
			// TODO: MIDI buffered event
			// Games wait before sending additional notes,
			// checking if audio status is busy then ready.
			m_midi_busy_count = 1;
			break;
		case 0xc/4:
			// TODO: MIDI passthrough event
			break;
		case 0x18/4:
			// TODO: MIDI update_divisor
			break;
	}

	COMBINE_DATA(&m_midi_regs[offset]);
}

void sega_9h0_0008_state::install_game_rom()
{
	// Nothing (overriden by each system)
}

void sega_9h0_0008_state::machine_start()
{
	install_game_rom();

	save_item(NAME(m_requested_fiq));
	save_item(NAME(m_irq_wait_start_addr));

	save_item(NAME(m_fifo_state_after_data));
	save_item(NAME(m_fifo_state_after_size));
	save_item(NAME(m_fifo_state_after_head));
	save_item(NAME(m_fifo_state_after_tail));

	save_item(NAME(m_fifo_events_data));
	save_item(NAME(m_fifo_events_size));
	save_item(NAME(m_fifo_events_head));
	save_item(NAME(m_fifo_events_tail));

	m_video_regs = make_unique_clear<uint32_t[]>(0x100/4);
	save_pointer(NAME(m_video_regs), 0x100/4);
	save_item(NAME(m_scale));

	m_midi_regs = make_unique_clear<uint32_t[]>(0x20/4);
	save_pointer(NAME(m_midi_regs), 0x20/4);
	save_item(NAME(m_midi_busy_count));

	save_item(NAME(m_pen_target));
	save_item(NAME(m_effective_page));

	m_cache_layer.allocate(1024, 512);
}

void sega_9h0_0008_state::video_start()
{
	m_cache_palette = std::make_unique<rgb_t[]>(0x100*SCREEN_H);
	for (int i = 0; i < 0x100 * SCREEN_H; i++) {
		m_cache_palette[i] = 0;
	}
	save_pointer(NAME(m_cache_palette.get()), 0x100*SCREEN_H);

	m_tiles_layers = make_unique_clear<uint8_t[]>(0x80000);
	m_tiles_sprites = make_unique_clear<uint8_t[]>(0x80000);
	save_pointer(NAME(m_tiles_layers.get()), 0x80000);
	save_pointer(NAME(m_tiles_sprites.get()), 0x80000);

	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(
		m_palette, sega_beena_8bpp_layout, m_tiles_layers.get(), 0, 1, 0));
	m_gfxdecode->set_gfx(1, std::make_unique<gfx_element>(
		m_palette, sega_beena_8bpp_layout, m_tiles_sprites.get(), 0, 1, 0));
}

void sega_9h0_0008_state::machine_reset()
{
	m_requested_fiq = false;
	m_irq_wait_start_addr = UNKNOWN_ADDR;

	memset(m_video_regs.get(), 0, 0x100);
	m_video_regs[0x10/4] = 0x200; // Tiles are 1x scaled by default.
	m_scale = 1;
	m_scanline = 0;

	bool is_video_connected = m_io_video_config->read() == 0;
	if (!is_video_connected) {
		m_video_regs[0] |= 0x00000200;
	}

	memset(m_midi_regs.get(), 0, 0x20);
	m_midi_busy_count = 0;

	m_io_sensor_regs[0] = 7; // Pen is making contact with the target surface.
	m_io_sensor_regs[0x2c/4] = 0xffffffff; // Pages 1 to 4 are closed.
	m_io_sensor_regs[0x30/4] = 0x00ffffff; // Pages 5 to 6 are closed.
	m_pen_target = 0;
	m_effective_page = 0;

	m_memcache_addr = 0;
	m_memcache_data = 0;
	m_memcache_i = 0;
	m_memcache_seq = BITS_EMPTY;
	m_memcache_state = IDLE;

	m_fifo_state_after_size = 0;
	m_fifo_state_after_head = 0;
	m_fifo_state_after_tail = 0;

	update_rtc();
}

void sega_9h0_0008_state::device_post_load()
{
	m_gfxdecode->gfx(0)->mark_all_dirty();
	m_gfxdecode->gfx(1)->mark_all_dirty();
}

uint8_t sega_9h0_0008_state::fifo_state_after_pop()
{
	uint8_t state = m_fifo_state_after_data[m_fifo_state_after_head];
	m_fifo_state_after_head = (m_fifo_state_after_head + 1) & (MEMCACHE_FIFO_MAX_SIZE - 1);
	m_fifo_state_after_size--;
	return state;
}

uint8_t sega_9h0_0008_state::fifo_events_pop()
{
	uint8_t event = m_fifo_events_data[m_fifo_events_head];
	m_fifo_events_head = (m_fifo_events_head + 1) & (MEMCACHE_FIFO_MAX_SIZE - 1);
	m_fifo_events_size--;
	return event;
}

void sega_9h0_0008_state::fifo_state_after_push(uint8_t state)
{
	// trash old data
	if (m_fifo_state_after_size > MEMCACHE_FIFO_MAX_SIZE - 1) {
		fifo_state_after_pop();
	}

	m_fifo_state_after_data[m_fifo_state_after_tail] = state;
	m_fifo_state_after_tail = (m_fifo_state_after_tail + 1) & (MEMCACHE_FIFO_MAX_SIZE - 1);
	m_fifo_state_after_size++;
}

void sega_9h0_0008_state::fifo_events_push(uint8_t event)
{
	// trash old data
	if (m_fifo_events_size > MEMCACHE_FIFO_MAX_SIZE - 1) {
		fifo_events_pop();
	}

	m_fifo_events_data[m_fifo_events_tail] = event;
	m_fifo_events_tail = (m_fifo_events_tail + 1) & (MEMCACHE_FIFO_MAX_SIZE - 1);
	m_fifo_events_size++;
}

void sega_9h0_0008_state::memcache_advance(uint32_t &status)
{
	uint8_t event = fifo_events_pop();

	status = event == 6 ? 1 : 0;

	switch (m_memcache_seq) {
		case BITS_0:
			m_memcache_seq = event == 1 ? BITS_0_1 : BITS_ERR;
			break;
		case BITS_0_1:
			m_memcache_seq = event == 7 ? BITS_0_1_7 : (event == 6 ? BITS_6 : BITS_ERR);
			break;
		case BITS_6:
			m_memcache_seq = event == 7 ? BITS_6_7 : (event == 0 ? BITS_0 : BITS_ERR);
			break;
		case BITS_7:
			m_memcache_seq = event == 1 ? BITS_7_1 : BITS_ERR;
			break;
		case BITS_EMPTY:
			m_memcache_seq = event == 7 ? BITS_7 : (event == 6 ? BITS_6 : (event == 0 ? BITS_0 : BITS_ERR));
			break;
		default:
			m_memcache_seq = BITS_ERR;
	}

	LOG("memcache seq = %08x\n", m_memcache_seq);
}

void sega_9h0_0008_state::memcache_parse_data_bit(uint32_t &status)
{
	if (m_memcache_seq == BITS_0_1) {
		// After a command has been ack'd, we might reset instead of parse the command's data bits.
		// Due to both sequences starting with the same prefix, we have to bailout on the shorter
		// sequence if the lookahead matches the longer sequence.
		if (m_fifo_events_size && m_fifo_events_data[m_fifo_events_head] == 7) {
			LOG("memcache READ_COMMAND 0 1 skipped\n");
			return;
		}
		m_memcache_seq = BITS_EMPTY;
		m_memcache_data = (m_memcache_data << 1) | 0;
		m_memcache_i++;
	} else if (m_memcache_seq == BITS_6_7) {
		m_memcache_seq = BITS_EMPTY;
		m_memcache_data = (m_memcache_data << 1) | 1;
		m_memcache_i++;
	} else if (m_memcache_seq == BITS_0_1_7) {
		status = 1;
		m_memcache_seq = BITS_EMPTY;
		m_memcache_state = IDLE;
		m_fifo_state_after_size = 0;
		m_fifo_state_after_head = 0;
		m_fifo_state_after_tail = 0;
		LOG("memcache READ_COMMAND 0 1 7 -> IDLE\n");
	}
}

uint32_t sega_9h0_0008_state::io_expansion_r()
{
	return 0; // TODO
}

uint32_t sega_9h0_0008_state::io_memcache_r()
{
	/*
	    A non-memory mapped cache can be interfaced with data address 0x500300c0,
	    while the status after parsing this data (events) can be read at 0x500300bc.

	    The cache stores RAM values in range 0xc00c6400..0xc00c6fff.

	    During I/O init, this cache is cleared.

	    During Test Mode, some memory integrity checks are carried out,
	    consisting of reading RAM to the cache, then writing those cached bytes
	    at the same RAM address, which is then compared with the expected values.

	    The communication protocol consists of sending sequences of events,
	    contextual to the command that is currently being parsed (which in turn
	    is initiated with a sequence of events).

	    Simplified flow:

	        IDLE
	        READ_COMMAND (includes RAM address bits 8..12)
	            if 0xa0:
	                READ_ADDRESS_BYTE (RAM address bits 0..7)
	                READ_RAM_BYTE (RAM value bits 0..7)
	            if 0xa1:
	                WRITE_RAM_BYTE (assumed to be at previous read RAM address)

	    Some of these sequences can be followed by acknowledgements (READ_ACK), to
	    ensure sent events were parsed as expected, otherwise games bailout from the
	    session. Always ends with the same sequence (BITS_0_1_7).
	*/
	uint32_t status = 0;

	if (machine().side_effects_disabled()) {
		return status;
	}

	while (m_fifo_events_size) {
		memcache_advance(status);

		bool is_memcache_disabled = (m_io_cpu_config->read() & 0x40) == 0x40;
		if (is_memcache_disabled) {
			continue;
		}

		switch (m_memcache_state) {
			case IDLE:
				if (m_memcache_seq == BITS_6) {
					m_memcache_seq = BITS_EMPTY;
					m_memcache_state = CONSUME_UNTIL_IDLE;
					LOG("memcache IDLE stat\n");
				} else if (m_memcache_seq == BITS_7_1) {
					m_memcache_seq = BITS_EMPTY;
					m_memcache_state = READ_COMMAND;
					LOG("memcache IDLE -> READ_COMMAND\n");
				}
				break;
			case READ_COMMAND:
				memcache_parse_data_bit(status);
				if (m_memcache_i == 8) {
					LOG("memcache READ_COMMAND byte data = %08x\n", m_memcache_data);
					m_memcache_state = READ_ACK;
					m_memcache_addr = (((m_memcache_data & 0xe) >> 1) << 8) | (m_memcache_addr & 0xff);
					LOG("memcache READ_COMMAND addr & 0xf00 = %08x\n", m_memcache_addr);
					if ((m_memcache_data & 0xa1) == 0xa0) {
						fifo_state_after_push(READ_ADDRESS_BYTE);
						fifo_state_after_push(READ_ACK);
						fifo_state_after_push(READ_RAM_BYTE);
						LOG("memcache READ_COMMAND 0xa0 -> READ_ACK\n");
					} else if ((m_memcache_data & 0xa1) == 0xa1) {
						fifo_state_after_push(WRITE_RAM_BYTE);
						LOG("memcache READ_COMMAND 0xa1 -> READ_ACK\n");
					} else {
						m_memcache_state = CONSUME_UNTIL_IDLE;
						m_fifo_state_after_size = 0;
						m_fifo_state_after_head = 0;
						m_fifo_state_after_tail = 0;
						LOG("memcache READ_COMMAND -> CONSUME_UNTIL_IDLE (unknown command?)\n");
					}
					m_memcache_data = 0;
					m_memcache_i = 0;
				}
				break;
			case READ_ACK:
				if (m_memcache_seq == BITS_6_7) {
					m_memcache_seq = BITS_EMPTY;
					m_memcache_state = fifo_state_after_pop();
					LOG("memcache READ_ACK\n");
				}
				break;
			case READ_ADDRESS_BYTE:
				memcache_parse_data_bit(status);
				if (m_memcache_i == 8) {
					m_memcache_addr |= m_memcache_data;
					m_memcache_data = 0;
					m_memcache_i = 0;
					m_memcache_state = fifo_state_after_pop();
					LOG("memcache READ_ADDRESS_BYTE addr = %08x\n", m_memcache_addr);
				}
				break;
			case READ_RAM_BYTE:
				memcache_parse_data_bit(status);
				if (m_memcache_seq == BITS_7_1) {
					m_memcache_seq = BITS_EMPTY;
					m_memcache_state = READ_COMMAND;
					m_memcache_data = 0;
					m_memcache_i = 0;
					LOG("memcache READ_RAM_BYTE -> READ_COMMAND (interrupted?)\n");
				}
				if (m_memcache_i == 8) {
					LOG("memcache READ_RAM_BYTE [%08x]=>%02x\n", m_memcache_addr, m_memcache_data);
					LOG("memcache READ_RAM_BYTE effective addr = %08x\n", 0xc00c6400 + 0x100 + m_memcache_addr);
					m_memcache[m_memcache_addr] = m_memcache_data;
					m_memcache_data = 0;
					m_memcache_i = 0;

					// Later games (e.g. "Pocket Monsters Best Wishes") optimize reading the next RAM bytes
					// and just store one after the other, skipping READ_COMMAND + READ_ADDRESS_BYTE sequences.
					m_memcache_state = READ_ACK;
					m_fifo_state_after_size = 0;
					m_fifo_state_after_head = 0;
					m_fifo_state_after_tail = 0;
					fifo_state_after_push(READ_RAM_BYTE);
					m_memcache_addr++;
				}
				break;
			case WRITE_RAM_BYTE:
				if (m_memcache_i == 8) {
					if (m_memcache_seq == BITS_6_7) {
						m_memcache_seq = BITS_EMPTY;
						m_memcache_i = 0;
						LOG("memcache WRITE_RAM_BYTE ACK\n");
					}
				}
				if (m_memcache_seq == BITS_0_1) {
					if (m_fifo_events_size && m_fifo_events_data[m_fifo_events_head] == 7) {
						LOG("memcache WRITE_RAM_BYTE 0 1 skipped\n");
						continue;
					}
					m_memcache_seq = BITS_EMPTY;
					m_memcache_addr++;
					m_memcache_i = 0;
					LOG("memcache WRITE_RAM_BYTE next addr %08x\n", m_memcache_addr);
				} else if (m_memcache_seq == BITS_6_7) {
					status = (m_memcache[m_memcache_addr] >> (7 - m_memcache_i)) & 1;
					LOG("memcache WRITE_RAM_BYTE read addr [%08x]=>(%08x >> %02x) & 1 = %02x\n",
							m_memcache_addr,
							m_memcache[m_memcache_addr],
							m_memcache_i,
							status);
					m_memcache_seq = BITS_EMPTY;
					m_memcache_i++;
				} else if (m_memcache_seq == BITS_0_1_7) {
					status = 1;
					m_memcache_addr = 0;
					m_memcache_data = 0;
					m_memcache_i = 0;
					m_memcache_seq = BITS_EMPTY;
					m_memcache_state = IDLE;
					m_fifo_state_after_size = 0;
					m_fifo_state_after_head = 0;
					m_fifo_state_after_tail = 0;
					LOG("memcache WRITE_RAM_BYTE 0 1 7 -> IDLE\n");
				}
				break;
			case CONSUME_UNTIL_IDLE:
				if (m_memcache_seq == BITS_0_1_7) {
					status = 1;
					m_memcache_addr = 0;
					m_memcache_data = 0;
					m_memcache_i = 0;
					m_memcache_seq = BITS_EMPTY;
					m_memcache_state = IDLE;
					LOG("memcache CONSUME_UNTIL_IDLE 0 1 7 -> IDLE\n");
				}
				break;
		}

		if (m_memcache_seq == BITS_ERR) {
			// Caught unexpected events, so clear all state, and try to resync at next reset bits.
			m_memcache_addr = 0;
			m_memcache_data = 0;
			m_memcache_i = 0;
			m_memcache_seq = BITS_EMPTY;
			m_memcache_state = CONSUME_UNTIL_IDLE;
			m_fifo_state_after_size = 0;
			m_fifo_state_after_head = 0;
			m_fifo_state_after_tail = 0;
			LOG("memcache BITS_ERR -> CONSUME_UNTIL_IDLE\n");
		}
	}

	return status;
}

uint32_t sega_9h0_0008_state::rtc_r(offs_t offset)
{
	return m_rtc[offset];
}

void sega_9h0_0008_state::rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_rtc[offset]);
	LOG("rtc_w @ %08x [%02x]=%08x\n", m_maincpu->pc(), offset, data);
}

void sega_9h0_0008_state::pal_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t *shared16 = reinterpret_cast<uint16_t *>(m_paletteram.target());
	COMBINE_DATA(&shared16[BYTE_XOR_BE(offset)]);

	uint16_t pal_entry = shared16[BYTE_XOR_BE(offset)];
	uint8_t r, g, b;
	r = (pal_entry & 0x001f) >> 0;
	g = (pal_entry & 0x03e0) >> 5;
	b = (pal_entry & 0x7c00) >> 10;
	rgb_t color = rgb_t(pal5bit(r), pal5bit(g), pal5bit(b));
	m_cache_palette[m_scanline * 0x100 + offset] = color;
	m_palette->set_pen_color(offset, color);
	LOG("m_cache_palette[%d / m_scale * 0x100 + %04x] = %04x (%04x)\n", m_scanline, offset, color, pal_entry);

	m_requested_fiq = true;
}

uint8_t sega_9h0_0008_state::tiles_layers_r(offs_t offset)
{
	return m_tiles_layers[offset];
}

void sega_9h0_0008_state::tiles_layers_w(offs_t offset, uint8_t data)
{
	if (m_tiles_layers[offset] != data) {
		m_tiles_layers[offset] = data;
		m_gfxdecode->gfx(0)->mark_dirty(offset/(16*16));
	}
}

uint8_t sega_9h0_0008_state::tiles_sprites_r(offs_t offset)
{
	return m_tiles_sprites[offset];
}

void sega_9h0_0008_state::tiles_sprites_w(offs_t offset, uint8_t data)
{
	if (m_tiles_sprites[offset] != data) {
		m_tiles_sprites[offset] = data;
		m_gfxdecode->gfx(1)->mark_dirty(offset/(16*16));
	}
}

void sega_9h0_0008_state::draw_layer(
		bitmap_rgb32 &bitmap,
		const rectangle &cliprect,
		const uint16_t *tilemap,
		const uint8_t scroll_idx,
		const bool is_active,
		const bool is_overlay_rendered)
{
	if (!is_active) {
		return; // Disabled
	}

	if ((m_video_regs[0x10/4] & 0x30) == 0x30) {
		draw_layer_scanlines(bitmap, cliprect, tilemap, scroll_idx, is_overlay_rendered);
	} else {
		draw_layer_tiles(bitmap, cliprect, tilemap, scroll_idx, is_overlay_rendered);
	}
}

void sega_9h0_0008_state::draw_layer_tiles(
		bitmap_rgb32 &bitmap,
		const rectangle &cliprect,
		const uint16_t *tilemap,
		const uint8_t scroll_idx,
		const bool is_overlay_rendered)
{
	// Bitplane area =  1024x512 (64x32 tiles), matches tile data mapping entries (64 * 32 = 0x800).
	//  Visible area =   704x480 (44x30 tiles), offset by +10 tiles in x-axis and +2 tiles in y-axis.
	//  Tilemap area = 1024x1024 (64x64 tiles), all these off-screen tiles must be considered for scrolling.
	for (size_t offset = 0; offset < 0x2000/2; offset++) {
		int32_t y = ((offset / 64) - 2) * 16;
		int32_t x = ((offset % 64) - 10) * 16;

		uint16_t scroll_x_idx = scroll_idx;
		uint16_t scroll_y_idx = scroll_idx;
		uint8_t entry_size = 4; // 2 int16_t values, 1 for each tile layer
		if (y >= 0 && y < (0x90 / entry_size) * 16) {
			if ((m_video_regs[0x10/4] & 0x20) == 0x20) {
				scroll_x_idx += y/16 * 2;
			}
		}
		if (x >= 0 && x < (0xb4 / entry_size) * 16) {
			if ((m_video_regs[0x10/4] & 0x40) == 0x40) {
				scroll_y_idx += x/16 * 2;
			}
		}

		x = scroll_x(x, scroll_x_idx);
		y = scroll_y(y, scroll_y_idx);

		uint16_t tile = tilemap[BYTE_XOR_BE(offset)];
		uint16_t tile_transform = (tile & 0xf000) >> 12;
		uint16_t flip_x = (tile_transform & 1) != 0;
		uint16_t flip_y = (tile_transform & 2) != 0;
		bool is_overlay = (tile_transform & 4) != 0;
		if (is_overlay != is_overlay_rendered) {
			continue;
		}

		uint16_t tile_idx = tile & 0x7ff;
		bool is_sprite_tile_data_used = tile & 0x800;
		gfx_element *gfx = m_gfxdecode->gfx(is_sprite_tile_data_used ? 1 : 0);
		const pen_t *paldata = m_palette->pens() + gfx->colorbase() + gfx->granularity() * (7 % gfx->colors());
		const uint8_t scale = m_scale;
		drawgfxzoom_with_pixel_op(gfx, bitmap, cliprect,
				tile_idx,
				flip_x,
				flip_y,
				x * m_scale,
				y * m_scale,
				0x10000 * m_scale,
				0x10000 * m_scale,
				[scale, paldata] (int32_t y, const std::unique_ptr<rgb_t[]> &palcache, uint32_t &destp, const uint8_t &srcp)
				{
					if (srcp != 0) {
						rgb_t color = palcache[y / scale * 0x100 + srcp];
						if (color == 0) {
							color = paldata[srcp];
						}
						destp = color;
					}
				});
	}
}

/**
 * Similar to gfx_element::drawgfxzoom_core(), but uses a pixel_op that depends on the current y-position to render,
 * in order to apply dedicated palettes for each scanline. These palettes are cached and passed directly
 * as a parameter instead of being captured in the pixel_op lambda, to avoid degraded performance from large copies.
 */
template <typename BitmapType, typename FunctionClass>
inline void sega_9h0_0008_state::drawgfxzoom_with_pixel_op(
		gfx_element *gfx,
		BitmapType &dest,
		const rectangle &cliprect,
		uint32_t code,
		int flipx,
		int flipy,
		int32_t
		destx,
		int32_t desty,
		uint32_t scalex,
		uint32_t scaley,
		FunctionClass &&pixel_op)
{
	do {
		assert(dest.valid());
		assert(dest.cliprect().contains(cliprect));

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		// compute scaled size
		uint32_t dstwidth = (scalex * gfx->width() + 0x8000) >> 16;
		uint32_t dstheight = (scaley * gfx->height() + 0x8000) >> 16;
		if (dstwidth < 1 || dstheight < 1)
			break;

		// compute 16.16 source steps in dx and dy
		int32_t dx = (gfx->width() << 16) / dstwidth;
		int32_t dy = (gfx->height() << 16) / dstheight;

		// compute final pixel in X and exit if we are entirely clipped
		int32_t destendx = destx + dstwidth - 1;
		if (destx > cliprect.right() || destendx < cliprect.left())
			break;

		// apply left clip
		int32_t srcx = 0;
		if (destx < cliprect.left()) {
			srcx = (cliprect.left() - destx) * dx;
			destx = cliprect.left();
		}

		// apply right clip
		if (destendx > cliprect.right())
			destendx = cliprect.right();

		// compute final pixel in Y and exit if we are entirely clipped
		int32_t destendy = desty + dstheight - 1;
		if (desty > cliprect.bottom() || destendy < cliprect.top())
			break;

		// apply top clip
		int32_t srcy = 0;
		if (desty < cliprect.top()) {
			srcy = (cliprect.top() - desty) * dy;
			desty = cliprect.top();
		}

		// apply bottom clip
		if (destendy > cliprect.bottom())
			destendy = cliprect.bottom();

		// apply X flipping
		if (flipx) {
			srcx = (dstwidth - 1) * dx - srcx;
			dx = -dx;
		}

		// apply Y flipping
		if (flipy) {
			srcy = (dstheight - 1) * dy - srcy;
			dy = -dy;
		}

		// fetch the source data
		const uint8_t *srcdata = gfx->get_data(code);

		// compute how many blocks of 4 pixels we have
		uint32_t numblocks = (destendx + 1 - destx) / 4;
		uint32_t leftovers = (destendx + 1 - destx) - 4 * numblocks;

		// iterate over pixels in Y
		for (int32_t cury = desty; cury <= destendy; cury++) {
			auto *destptr = &dest.pix(cury, destx);
			const uint8_t *srcptr = srcdata + (srcy >> 16) * gfx->rowbytes();
			int32_t cursrcx = srcx;
			srcy += dy;

			// iterate over unrolled blocks of 4
			for (int32_t curx = 0; curx < numblocks; curx++) {
				pixel_op(cury, m_cache_palette, destptr[0], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				pixel_op(cury, m_cache_palette, destptr[1], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				pixel_op(cury, m_cache_palette, destptr[2], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				pixel_op(cury, m_cache_palette, destptr[3], srcptr[cursrcx >> 16]);
				cursrcx += dx;

				destptr += 4;
			}

			// iterate over leftover pixels
			for (int32_t curx = 0; curx < leftovers; curx++) {
				pixel_op(cury, m_cache_palette, destptr[0], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				destptr++;
			}
		}
	} while (0);
}

int32_t sega_9h0_0008_state::scroll_x(int32_t x, uint16_t i)
{
	uint16_t *scroll_data = reinterpret_cast<uint16_t *>(m_tilemap_scroll_x.target());
	x += -signed10(scroll_data[BYTE_XOR_BE(i)]);

	// Wrap-around at tilemap borders
	if (x > 0x2c0 + 0xa0) {
		x -= (0x2c0 + 0xa0 * 2);
	}
	if (x < -0xa0) {
		x += (0x2c0 + 0xa0 * 2);
	}

	return x;
}

int32_t sega_9h0_0008_state::scroll_y(int32_t y, uint16_t i)
{
	uint16_t *scroll_data = reinterpret_cast<uint16_t *>(m_tilemap_scroll_y.target());
	uint32_t scroll_entry = scroll_data[BYTE_XOR_BE(i)];
	if (((scroll_entry & 0xfc00) == 0xfc00) && (y <= 224)) {
		// Skip scroll on first 14 (7 if 2x scaled) tile rows.
		// Test case is title screen of "Cooking Beena", where both title and table
		// are rendered in the foreground layer, but only the table should be scrolled.
		return y;
	}

	y += signed10(scroll_entry);

	// Wrap-around at tilemap borders
	if (y > (0x400 - 0x20) - 0x10) {
		y -= 0x400;
	}
	if (y < -0x20) {
		y += 0x400;
	}

	return y;
}

void sega_9h0_0008_state::draw_layer_scanlines(
		bitmap_rgb32 &bitmap,
		const rectangle &cliprect,
		const uint16_t *tilemap,
		const uint8_t scroll_idx,
		const bool is_overlay_rendered)
{
	const rectangle cache_bitmap_bounds(0, m_cache_layer.width()-1, 0, m_cache_layer.height()-1);
	m_cache_layer.fill(0, cache_bitmap_bounds);

	// Apply y-scrolling + wrap-around on each tile row and draw to cached bitmap
	const int8_t tile_factor = 16;
	for (size_t offset = 0; offset < 0x2000/2; offset++) {
		int32_t y = ((offset / 64) - 2) * tile_factor;
		int32_t x = ((offset % 64) - 10) * tile_factor;

		uint16_t tile = tilemap[BYTE_XOR_BE(offset)];
		uint16_t tile_transform = (tile & 0xf000) >> 12;
		uint16_t flip_x = (tile_transform & 1) != 0;
		uint16_t flip_y = (tile_transform & 2) != 0;
		bool is_overlay = (tile_transform & 4) != 0;
		if (is_overlay != is_overlay_rendered) {
			continue;
		}

		uint16_t scroll_y_idx = scroll_idx;
		if (x >= 0 && x < (0xb4 / 4) * tile_factor) {
			if ((m_video_regs[0x10/4] & 0x40) == 0x40) {
				scroll_y_idx += x /tile_factor * 2;
			}
		}

		y = scroll_y(y, scroll_y_idx);

		// Undo the x-axis visible area adjustment, otherwise we clip out tile data
		// that is later used for x-scrolling.
		x += 10 * tile_factor;

		uint16_t tile_idx = tile & 0x7ff;
		bool is_sprite_tile_data_used = tile & 0x800;
		gfx_element *gfx = m_gfxdecode->gfx(is_sprite_tile_data_used ? 1 : 0);
		gfx->zoom_alpha(m_cache_layer, cache_bitmap_bounds,
				tile_idx,
				7,
				flip_x,
				flip_y,
				x,
				y,
				0x10000,
				0x10000,
				0,
				0xff);
	}

	// Apply x-scrolling + wrap-around on each cached scanline and draw to screen bitmap
	for (int8_t yi = 0; yi < 32 / m_scale; yi++) {
		for (int8_t xi = 0; xi < 64; xi++) {
			int32_t y = (yi - 0) * tile_factor;
			int32_t x = (xi - 0) * tile_factor;

			for (int8_t line_yi = 0; line_yi < tile_factor; line_yi++) {
				int32_t dst_y = y + line_yi;
				int32_t src_y = dst_y;

				uint16_t scroll_x_idx = scroll_idx;
				if (y >= 0 && y < (0x90 / 4) * tile_factor) {
					scroll_x_idx += (y + line_yi) * 2;
				}

				for (int8_t line_xi = 0; line_xi < tile_factor; line_xi++) {
					int32_t dst_x = x + line_xi;
					int32_t src_x = dst_x;

					dst_x = scroll_x(dst_x, scroll_x_idx);

					// Redo the x-axis visible area adjustment
					dst_x -= 10 * tile_factor;

					if (dst_x * m_scale < cliprect.min_x
						|| dst_x * m_scale > cliprect.max_x
						|| dst_y * m_scale < cliprect.min_y
						|| dst_y * m_scale > cliprect.max_y) {
						continue; // Skip beyond screen size
					}
					if (src_x < m_cache_layer.cliprect().min_x
						|| src_x > m_cache_layer.cliprect().max_x
						|| src_y < m_cache_layer.cliprect().min_y
						|| src_y > m_cache_layer.cliprect().max_y) {
						continue; // Skip beyond cache size
					}

					uint32_t *src = &m_cache_layer.pix(src_y, src_x);
					if (*src != 0) {
						for (size_t dyi = 0; dyi < m_scale; dyi++) {
							for (size_t dxi = 0; dxi < m_scale; dxi++) {
								bitmap.pix(dyi + dst_y * m_scale, dxi + dst_x * m_scale) = *src;
							}
						}
					}
				}
			}
		}
	}
}

void sega_9h0_0008_state::draw_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if ((m_video_regs[0x20/4] & 1) == 0) {
		return; // Disabled
	}

	uint16_t *bitmap_data = reinterpret_cast<uint16_t *>(m_bitmap.target());

	uint16_t bitmap_full_w   = ((m_video_regs[0x20/4] >> 8)  & 0xff) << 4;
	uint16_t bitmap_full_h   = ((m_video_regs[0x20/4] >> 16) & 0xff) << 4;
	uint16_t bitmap_offset_x = ((m_video_regs[0x24/4] >> 0)  & 0x3ff);
	uint16_t bitmap_offset_y = ((m_video_regs[0x24/4] >> 16) & 0x3ff);
	uint16_t bitmap_scroll_x = ((m_video_regs[0x28/4] >> 0)  & 0x3ff);
	uint16_t bitmap_scroll_y = ((m_video_regs[0x28/4] >> 16) & 0x3ff);
	uint16_t bitmap_clip_w   = ((m_video_regs[0x2c/4] >> 0)  & 0x3ff);
	uint16_t bitmap_clip_h   = ((m_video_regs[0x2c/4] >> 16) & 0x3ff);
	uint32_t i = bitmap_offset_y * bitmap_full_w + bitmap_offset_x;
	for (size_t y = 0; y < bitmap_full_h * m_scale; y += m_scale) {
		for (size_t x = 0; x < bitmap_full_w * m_scale; x += m_scale) {
			if (i >= 0xc6000/2) {
				return;
			}

			uint16_t color = bitmap_data[BYTE_XOR_BE(i)];
			i++;

			if (color == 0
					|| y >= bitmap_clip_h * m_scale
					|| x >= bitmap_clip_w * m_scale
					|| y + bitmap_scroll_y * m_scale > cliprect.max_y
					|| x + bitmap_scroll_x * m_scale > cliprect.max_x) {
				continue;
			}

			uint8_t r = pal5bit((color & 0x001f) >> 0);
			uint8_t g = pal5bit((color & 0x03e0) >> 5);
			uint8_t b = pal5bit((color & 0x7c00) >> 10);
			uint32_t rgb = (b) | (g << 8) | (r << 16);
			for (size_t yi = 0; yi < m_scale; yi++) {
				for (size_t xi = 0; xi < m_scale; xi++) {
					bitmap.pix(y + yi + bitmap_scroll_y * m_scale, x + xi + bitmap_scroll_x * m_scale) = rgb;
				}
			}
		}
	}
}

void sega_9h0_0008_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, const bool is_overlay_rendered)
{
	gfx_element *gfx = m_gfxdecode->gfx(1);

	uint16_t *tilemap_data = reinterpret_cast<uint16_t *>(m_tilemap_sprites.target());

	// For sprites to overlap as expected, they must be drawn in reverse order
	for (int16_t offset = 0x400/2 - 4; offset >= 0; offset -= 4) {
		uint16_t priority =  tilemap_data[BYTE_XOR_BE(offset + 1)] & 0xff;
		if ((priority > 0 && !is_overlay_rendered) || (priority == 0 && is_overlay_rendered)) {
			continue;
		}

		uint32_t tile_pos = (tilemap_data[BYTE_XOR_BE(offset)] << 16) | tilemap_data[BYTE_XOR_BE(offset + 1)];
		uint16_t y = (tile_pos >> 20) & 0x7ff;
		uint16_t x = (tile_pos >> 8)  & 0x7ff;

		uint16_t flip_y = ((tile_pos >> 20) & 0x800) != 0;
		uint16_t flip_x = ((tile_pos >> 8)  & 0x800) != 0;

		int16_t delta_factor = m_video_regs[0x38/4] & 0xff;
		int16_t delta_x = 0;
		int16_t delta_y = 0;
		if (delta_factor == 0x2c) {
			delta_x = 14 * 16;
			delta_y =  7 * 16;
		} else if (delta_factor == 0x20) {
			delta_x =  6 * 16;
			delta_y = -1 * 16;
		}

		uint16_t tile_idx = tilemap_data[BYTE_XOR_BE(offset + 2)] & 0x7ff;
		uint16_t tile_pattern_len = (tilemap_data[BYTE_XOR_BE(offset + 2)] & 0xf000) >> 12;

		if (tile_idx == 0 && x == 0 && y == 0)
			continue;

		uint8_t pattern_x_len = ((tile_pattern_len % 4) + 1);
		uint8_t pattern_y_len = ((tile_pattern_len / 4) + 1);
		uint8_t pattern_i = 0;

		std::vector<uint8_t> pattern_x_v(pattern_x_len);
		std::iota(pattern_x_v.begin(), pattern_x_v.end(), 0);
		if (flip_x) {
			std::reverse(pattern_x_v.begin(), pattern_x_v.end());
		}
		std::vector<uint8_t> pattern_y_v(pattern_y_len);
		std::iota(pattern_y_v.begin(), pattern_y_v.end(), 0);
		if (flip_y) {
			std::reverse(pattern_y_v.begin(), pattern_y_v.end());
		}
		for (size_t pattern_y : pattern_y_v) {
			for (size_t pattern_x : pattern_x_v) {
				// FIXME: Can be transparent (e.g. pen cursor fade-in seen in "Kazoku Minna no Nouryoku Trainer")
				gfx->zoom_alpha(bitmap, cliprect,
						tile_idx + pattern_i++,
						7,
						flip_x,
						flip_y,
						(x + pattern_x * 16) * m_scale - (0x2c0 / 2) + delta_x,
						(y + pattern_y * 16) * m_scale - (0x1e0 / 2) + delta_y,
						0x10000 * m_scale,
						0x10000 * m_scale,
						0,
						0xff);
			}
		}
	}
}

void sega_9h0_0008_state::update_crosshair(screen_device &screen)
{
	// Nothing (only the console uses a crosshair)
}

uint32_t sega_9h0_0008_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	update_crosshair(screen);

	bitmap.fill(m_palette->pen_color(0), cliprect);

	if ((m_video_regs[0x10/4] & 1) == 0) {
		screen_blend(bitmap, cliprect);

		return 0; // Disabled
	}

	uint16_t *shared16_tilemap_bg = reinterpret_cast<uint16_t *>(m_tilemap_bg.target());
	uint16_t *shared16_tilemap_fg = reinterpret_cast<uint16_t *>(m_tilemap_fg.target());

	if (((m_video_regs[0x10/4] & 0x3000) == 0x3000)) {
		draw_bitmap(bitmap, cliprect);
	}

	draw_layer(bitmap, cliprect, shared16_tilemap_bg, 1, (m_video_regs[0x10/4] & 4) != 0, false);

	if (((m_video_regs[0x10/4] & 0x3000) == 0x2000)) {
		draw_bitmap(bitmap, cliprect);
	}

	draw_layer(bitmap, cliprect, shared16_tilemap_fg, 0, (m_video_regs[0x10/4] & 2) != 0, false);

	bool is_bitmap_after_sprites = (m_video_regs[0x20/4] & 0x10) != 0;
	if (((m_video_regs[0x10/4] & 0x3000) == 0x1000) && !is_bitmap_after_sprites) {
		draw_bitmap(bitmap, cliprect);
	}

	draw_sprites(bitmap, cliprect, false);

	draw_layer(bitmap, cliprect, shared16_tilemap_bg, 1, (m_video_regs[0x10/4] & 4) != 0, true);

	if (((m_video_regs[0x10/4] & 0x3000) == 0) && !is_bitmap_after_sprites) {
		draw_bitmap(bitmap, cliprect);
	}

	// TV bottom frame in front of running dog in "Partner in TV"
	draw_layer(bitmap, cliprect, shared16_tilemap_fg, 0, (m_video_regs[0x10/4] & 2) != 0, true);

	if (((m_video_regs[0x10/4] & 0x3000) == 0x1000) && is_bitmap_after_sprites) {
		// Minimap after lamp posts in "Cars 2" bridge section
		draw_bitmap(bitmap, cliprect);
	}

	draw_sprites(bitmap, cliprect, true);

	if (((m_video_regs[0x10/4] & 0x3000) == 0) && is_bitmap_after_sprites) {
		draw_bitmap(bitmap, cliprect);
	}

	screen_blend(bitmap, cliprect);

	// Invalidate on VBLANK: the latest entries are kept by m_palette.
	for (int i = 0; i < SCREEN_H; i++)
		for (int offset = 0; offset < 0x100; offset++)
			m_cache_palette[i * 0x100 + offset] = 0;

	return 0;
}

void sega_9h0_0008_state::screen_blend(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t alpha_r_step = (m_video_regs[0x30/4] >>  0) & 0x3f;
	uint8_t alpha_g_step = (m_video_regs[0x30/4] >>  8) & 0x3f;
	uint8_t alpha_b_step = (m_video_regs[0x30/4] >> 16) & 0x3f;

	uint8_t alpha_r = rescale_alpha_step(alpha_r_step);
	uint8_t alpha_g = rescale_alpha_step(alpha_g_step);
	uint8_t alpha_b = rescale_alpha_step(alpha_b_step);

	uint8_t blend_r = alpha_r_step < 0x20 ? 0xff : 0;
	uint8_t blend_g = alpha_g_step < 0x20 ? 0xff : 0;
	uint8_t blend_b = alpha_b_step < 0x20 ? 0xff : 0;
	uint32_t blend_rgb = blend_b | (blend_g << 8) | (blend_r << 16);

	for (size_t y = cliprect.min_y; y <= cliprect.max_y; y++) {
		for (size_t x = cliprect.min_x; x <= cliprect.max_x; x++) {
			uint32_t *dst = &bitmap.pix(y, x);
			*dst = alpha_blend_rgb_levels(blend_rgb, *dst, alpha_b, alpha_r, alpha_g);
		}
	}
}

/**
 * Similar to sega_9h0_0008_state::rescale(),
 * but the step can either lighten (e.g. video debug flag `0xc00c7cc7 = 1`
 * in "Soreike! Anpanman Hajimete Kaketa yo! Oboeta yo! Hiragana Katakana"
 * or darken (e.g. screen fadeout).
 */
int32_t sega_9h0_0008_state::rescale_alpha_step(uint8_t step)
{
	return step < 0x20 ? rescale(step, 0, 0x1f, 0xff, 0) : rescale(step, 0x20, 0x3f, 0, 0xff);
}

void sega_9h0_0008_state::request_irq()
{
	m_maincpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
	m_maincpu->set_input_line(ARM7_IRQ_LINE, CLEAR_LINE);
}

void sega_9h0_0008_state::request_fiq()
{
	if (m_requested_fiq) {
		m_maincpu->set_input_line(ARM7_FIRQ_LINE, ASSERT_LINE);
		m_maincpu->set_input_line(ARM7_FIRQ_LINE, CLEAR_LINE);

		m_requested_fiq = false;
	}
}

void sega_9h0_0008_state::update_rtc()
{
	system_time systime;
	machine().current_datetime(systime);
	m_rtc[0] = systime.local_time.second;
	m_rtc[1] = systime.local_time.minute;
	m_rtc[2] = systime.local_time.hour;
	m_rtc[3] = systime.local_time.mday;
	m_rtc[4] = systime.local_time.weekday;
	m_rtc[5] = systime.local_time.month;
	m_rtc[6] = systime.local_time.year - 2000;
}

void sega_9h0_0008_state::sega_9h0_0008(machine_config &config)
{
	AP2010CPU(config, m_maincpu, 81'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sega_9h0_0008_state::beena_arm7_map);

	SCREEN(config, m_screen_main, SCREEN_TYPE_RASTER);
	m_screen_main->set_refresh_hz(60);
	m_screen_main->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen_main->set_size(SCREEN_W, SCREEN_H);
	m_screen_main->set_visarea(0, SCREEN_W-1, 0, SCREEN_H-1);
	m_screen_main->set_screen_update(FUNC(sega_9h0_0008_state::screen_update));

	TIMER(config, "scantimer").configure_scanline(FUNC(sega_9h0_0008_state::scanline), m_screen_main, 0, 1);

	PALETTE(config, m_palette).set_entries(0x100);

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	SPEAKER(config, "speaker").front_center();
	AP2010PCM(config, m_pcm); // Unknown clock
	m_pcm->add_route(ALL_OUTPUTS, "speaker", 1.0);
}


class sega_9h0_0008_cart_state : public sega_9h0_0008_state
{
public:
	sega_9h0_0008_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_9h0_0008_state(mconfig, type, tag)
		, m_cart(*this, "cartslot")
	{ }

protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
};

DEVICE_IMAGE_LOAD_MEMBER(sega_9h0_0008_cart_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM32_WIDTH, ENDIANNESS_BIG);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	if (!image.loaded_through_softlist()) {
		// loadflags weren't parsed, start by manually applying `endianness="big"`,
		// taking into consideration the host's endianness.
		uint32_t *rom32 = reinterpret_cast<uint32_t *>(m_cart->get_rom_base());
		for (size_t i = 0; i < size / 4; i++) {
			rom32[i] = big_endianize_int32(rom32[i]);
		}
		// Afterwards apply `load16_word_swap`, regardless of host's endianness,
		// since this reflects how ROM data was stored, not the CPU's architecture.
		uint16_t *rom16 = reinterpret_cast<uint16_t *>(m_cart->get_rom_base());
		for (size_t i = 0; i < size / 2; i++) {
			rom16[i] = swapendian_int16(rom16[i]);
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}


class sega_beena_state : public sega_9h0_0008_cart_state
{
public:
	sega_beena_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_9h0_0008_cart_state(mconfig, type, tag)
		, m_io_page_config(*this, "PAGE_CONFIG")
		, m_io_page(*this, "PAGE")
		, m_io_pad_left(*this, "PAD_LEFT")
		, m_io_pad_right(*this, "PAD_RIGHT")
		, m_io_pen_left(*this, "PEN_LEFT")
		, m_io_pen_right(*this, "PEN_RIGHT")
		, m_io_pen_x(*this, "PENX")
		, m_io_pen_y(*this, "PENY")
	{ }

	void sega_beena(machine_config &config);

	virtual DECLARE_CROSSHAIR_MAPPER_MEMBER(pen_y_mapper);

private:
	virtual void install_game_rom() override;
	virtual void update_crosshair(screen_device &screen) override;
	virtual void update_sensors(offs_t offset) override;

	required_ioport m_io_page_config;
	required_ioport m_io_page;
	required_ioport m_io_pad_left;
	required_ioport m_io_pad_right;
	required_ioport m_io_pen_left;
	required_ioport m_io_pen_right;
	required_ioport m_io_pen_x;
	required_ioport m_io_pen_y;
};

void sega_beena_state::sega_beena(machine_config &config)
{
	sega_9h0_0008(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "sega_beena_cart");
	m_cart->set_endian(ENDIANNESS_BIG);
	m_cart->set_width(GENERIC_ROM32_WIDTH);
	m_cart->set_device_load(FUNC(sega_beena_state::cart_load));
	m_cart->set_must_be_loaded(false);

	SOFTWARE_LIST(config, "cart_list").set_original("sega_beena_cart");

	config.set_default_layout(layout_beena);
}

void sega_beena_state::install_game_rom()
{
	if (m_cart->exists()) {
		std::string region_tag;
		memory_region *cart_rom = m_cart->memregion("cart:rom");
		uint32_t *rom32 = reinterpret_cast<uint32_t *>(cart_rom->base());

		/*
		    ROM header contains a field for the base address which we can use to map
		    the corresponding address range. On hardware, this mapping is done by the BIOS when
		    writing to the following registers:

		        0x60020004=0x1ff and 0x60020010=0xa5: ROM_MASK_BASE mapped when the cart has a mask ROM;
		        0x60020004=0x3fc and 0x60020010=0x90: ROM_FLASH_BASE mapped when the cart has a flash ROM;

		    Signatures on each address map are then checked to choose which one will run game code.

		    It's possible to force some invalid mappings which result in the console hanging during reads.
		    For example, setting 0x60020004=0x1ff and 0x60020010=0x90 will map both address ranges
		    when the cart has a mask ROM.
		*/
		uint32_t base_address = rom32[0xdc/4];
		if (base_address != ROM_MASK_BASE && base_address != ROM_FLASH_BASE) {
			osd_printf_warning("Unknown base address '%08x' parsed from ROM, defaulting to '%08x'...\n", base_address, ROM_MASK_BASE);
			base_address = ROM_MASK_BASE;
		}
		m_maincpu->space(AS_PROGRAM).install_rom(base_address, base_address + 0x7fffff, 0x800000, cart_rom->base());
	}
}

void sega_beena_state::update_crosshair(screen_device &screen)
{
	// Show crosshair on single screen with storyware pen target
	machine().crosshair().get_crosshair(0).set_screen(m_pen_target ? CROSSHAIR_SCREEN_NONE : &screen);
}

void sega_beena_state::update_sensors(offs_t offset)
{
	sega_9h0_0008_state::update_sensors(offset);

	uint16_t io_page = 0;
	int16_t io_pen_x = 0;
	int16_t io_pen_y = 0;
	int16_t io_pen_x_min = m_io_pen_x->field(0xff)->minval();
	int16_t io_pen_x_max = m_io_pen_x->field(0xff)->maxval();
	int16_t io_pen_y_min = m_io_pen_y->field(0xff)->minval();
	int16_t io_pen_y_max = m_io_pen_y->field(0xff)->maxval();
	int16_t pen_y_min = io_pen_y_min;
	int16_t pen_y_max = io_pen_y_max;
	uint32_t pen_pressed = 0;
	uint32_t pen_data = 0;

	bool is_tablet = true;
	bool is_test_mode = m_io_page_config->read() == 0;

	switch (offset) {
		// Pen position + button press state.
		//
		// A minimum of 3 readings must be parsed, otherwise effective pen position is not computed.
		// Later games seem to only require 2 readings.
		//
		// Each reading encodes bits `pyyy yyyy yyyx xxxx xxxx`, where `p` is 1 on pen button pressed.
		//
		// Offsets of readings for each pen are given by 3 bits set at 0x50020000:
		// - e.g. for 1st pen, 0x50020000=0x05 will parse 0x50020008, and 0x50020010, ignoring 0x5002000c;
		// - e.g. for 2nd pen, 0x50020000=0x70 will parse 0x50020018, 0x5002001c, and 0x50020020;
		//
		// TODO: Check how 2 connected pens are reported. Regardless of where a single pen is connected
		// (ports marked on PCB as CN12 or CN13), the first set of offsets is updated.
		//
		// 1st pen's 1st reading.
		case 0x8/4:
		// 1st pen's 2nd reading.
		case 0xc/4:
		// 1st pen's 3rd reading.
		case 0x10/4:
			io_pen_x = m_io_pen_x->read();
			io_pen_y = m_io_pen_y->read();

			m_pen_target = BIT(m_io_pen_left->read(), 1);

			// The y-position is used to distinguish between pen targets:
			// - Tablet:    0x338..0x248
			// - Storyware: 0x3b8..0x342
			//
			// The storyware's coordinates don't reach maximum screen positions (games don't compute
			// effective pen positions if such values are forced), possibly to account for page tab gaps.
			// Applying the same displacements to rescaled positions also results in more accurate mappings.
			//
			// FIXME: Confirm displacements with hardware tests.
			is_tablet = m_pen_target == 0;
			if (is_tablet) {
				// screen x-position [0..0x2c0]
				io_pen_x = rescale(io_pen_x, io_pen_x_min, io_pen_x_max, 0x1b8, 0x58);
				// screen y-position [0..0x1e0]
				io_pen_y = rescale(io_pen_y, pen_y_min, pen_y_max, 0x338, 0x248);
			}
			else {
				// screen x-position [8..0x2bc], off-by-4 upper bound?
				io_pen_x = rescale(io_pen_x, io_pen_x_min+8, io_pen_x_max-8, 0x1b6, 0x109);
				// screen y-position [0..0x1d8]
				io_pen_y = rescale(io_pen_y, pen_y_min+8, pen_y_max, 0x3b8, 0x342);
			}
			pen_pressed = (m_io_pen_left->read() & 1) ? 0x80000 : 0;
			pen_data = (io_pen_y << 9) | io_pen_x | pen_pressed;
			LOG("Pen on %s(%d): x=%04x,y=%04x => %08x",
				is_tablet ? "tablet" : "storyware",
				m_pen_target,
				io_pen_x,
				io_pen_y,
				pen_data);

			m_io_sensor_regs[offset] = pen_data;
			break;
		// Pages 1 to 4, one for each byte value.
		// We'll just threshold them to 0x00 (open) and 0xff (closed).
		case 0x2c/4:
			io_page = m_io_page->read();
			if (is_test_mode) {
				m_io_sensor_regs[offset] = 0xff00ff00;
				m_effective_page = 6;
			} else if ((m_effective_page ^ io_page) != 0) {
				m_effective_page = io_page;
				m_io_sensor_regs[offset] = (0xffffffffULL >> (m_effective_page * 8));
				LOG("Selected page: %d\n", m_effective_page);
			}

			break;
		// Pages 5 to 6, encodes bytes `XX AA BB YY`
		// - XX: unused? always 0x00 on hardware;
		// - YY: needs to be parsed as open for effective page computation (TODO: is this a sensor?);
		// - AA: page 5;
		// - BB: page 6;
		case 0x30/4:
			if (is_test_mode) {
				m_io_sensor_regs[offset] = 0xffff00ff;
			} else {
				switch (m_effective_page) {
					case 5:
						m_io_sensor_regs[offset] = 0x0000ff00;
						break;
					case 6:
						m_io_sensor_regs[offset] = 0x00000000;
						break;
					default:
						m_io_sensor_regs[offset] = 0x00ffff00;
				}
			}

			break;
		// Left and right pads, encodes bits `XXXX XXAB CDEF GHIJ`
		// - X: unused?
		// - ABCDE: red + directional buttons of left pad;
		// - FGHIJ: red + directional buttons of right pad;
		case 0x34/4:
			m_io_sensor_regs[offset] = ((m_io_pad_left->read() << 5) | m_io_pad_right->read()) & 0x3ff;
			break;
	}
}

CROSSHAIR_MAPPER_MEMBER(sega_beena_state::pen_y_mapper)
{
	// TODO: Either remove or adapt for Storyware layout
	return linear_value;
}


class tvochken_state : public sega_9h0_0008_state
{
public:
	tvochken_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_9h0_0008_state(mconfig, type, tag)
		, m_card(*this, "card")
		, m_io_buttons(*this, "BUTTONS")
	{ }

	void tvochken(machine_config &config);

	virtual uint32_t io_expansion_r() override;

	void scan_card(int state);

private:
	enum card_state : uint8_t
	{
		IDLE = 0,
		START_WRITE_DATA,
		WRITE_DATA,
	};

	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void install_game_rom() override;

	required_device<tvochken_card_device> m_card;

	required_ioport m_io_buttons;
	uint8_t m_card_previous_input;

	uint16_t m_card_data;
	uint8_t m_card_data_i;
	uint8_t m_card_state;
	uint8_t m_card_hold_i;
	uint8_t m_card_status;
};

void tvochken_state::tvochken(machine_config &config)
{
	sega_9h0_0008(config);

	TVOCHKEN_CARD(config, m_card);

	SOFTWARE_LIST(config, "card_list").set_original("tvochken");

	config.set_default_layout(layout_tvochken);
}

void tvochken_state::machine_start()
{
	sega_9h0_0008_state::machine_start();

	save_item(NAME(m_card_data));
	save_item(NAME(m_card_data_i));
	save_item(NAME(m_card_hold_i));
	save_item(NAME(m_card_state));
	save_item(NAME(m_card_status));
}

void tvochken_state::machine_reset()
{
	sega_9h0_0008_state::machine_reset();

	m_card_previous_input = 0;

	m_card_data = 0;
	m_card_data_i = 0;
	m_card_hold_i = 0;
	m_card_state = IDLE;
	m_card_status = 0;
}

/**
 * Combines all inputs:
 * - Pressed buttons
 *   - bit 1: A;
 *   - bit 2: B;
 *   - bit 3: C;
 * - Scanned card barcode
 *   - bit 5: current data bit to read from 16-bit value;
 *   - bit 6: status for advancing data bit position;
 *
 * Parsing protocol at function 0xa0001240 doesn't seem to use any
 * control commands to start parsing either input. It expects each parsed barcode
 * to take several reads before advancing to the next data bit. We handle this
 * by holding the value for a few reads, which also covers reads unrelated
 * to barcodes.
 */
uint32_t tvochken_state::io_expansion_r()
{
	if (machine().side_effects_disabled() || (m_io_auxiliary_regs[0x8/4] & 0xff) != 0) {
		return 0x98 | m_io_buttons->read();
	}

	/**
	 * Each scanned barcode is compared against these values taken from
	 * an in-memory table at 0xc00d0f9c. Valid barcodes always have the
	 * last bit set.
	 *
	 * 0x900a, 0xa05a, 0xb0aa, 0x90ca, 0x910a,
	 * 0x914a, 0x918a, 0x91ca, 0x920a, 0xa25a,
	 * 0x928a, 0x92ca, 0xa312, 0x934a, 0x938a,
	 * 0x93ca, 0xa41a, 0x944a, 0x948a, 0xb4da,
	 * 0xb512, 0xa55a, 0x958a, 0x95ca, 0x960a,
	 * 0x964a, 0xb69a, 0x96ca, 0x970a, 0x974a,
	 * 0x978a, 0x97ca, 0x980a, 0x984a, 0xa892,
	 * 0xa8da, 0xa91a, 0xa952, 0x998a, 0xb9da,
	 * 0xaa1a, 0x9a4a, 0x9a8a, 0x9aca, 0xab12,
	 * 0xab52, 0xbba2, 0xabd2, 0x9c0a, 0x9c4a
	*/

	if (m_card_state == START_WRITE_DATA) {
		m_card_hold_i--;
		if (m_card_hold_i == 0) {
			m_card_hold_i = 10;
			m_card_data_i = 0;
			m_card_status = 0;
			m_card_state = WRITE_DATA;
		}

		return 0x98 | (1 << 5) | m_io_buttons->read();
	}

	if (m_card_state == WRITE_DATA) {
		uint8_t data_bit = (m_card_data >> (15 - m_card_data_i)) & 1;
		LOG("write card: bit %d -> %d (sync %d)\n", m_card_data_i, data_bit, m_card_status);

		m_card_hold_i--;
		if (m_card_hold_i == 0) {
			m_card_hold_i = 10;
			m_card_status ^= 1;
			m_card_data_i++;
			if (m_card_data_i == 16) {
				m_card_data_i = 0;
				m_card_status = 0;
				m_card_state = IDLE;
			}
		}

		return 0x98 | (m_card_status << 6) | (data_bit << 5) | m_io_buttons->read();
	}

	return 0x98 | m_io_buttons->read();
}

void tvochken_state::scan_card(int state)
{
	if (m_card->exists() && state && (m_card_state == IDLE)) {
		m_card_data = m_card->barcode();
		m_card_hold_i = 10;
		m_card_state = START_WRITE_DATA;
		LOG("scanning card: %04x\n", m_card_data);
	}
}

void tvochken_state::install_game_rom()
{
	memory_region *rom = memregion("flash_rom");
	m_maincpu->space(AS_PROGRAM).install_rom(ROM_FLASH_BASE, ROM_FLASH_BASE + 0x7fffff, 0x800000, rom->base());
}


class carbeena_state : public sega_9h0_0008_cart_state
{
public:
	carbeena_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_9h0_0008_cart_state(mconfig, type, tag)
		, m_io_buttons(*this, "BUTTONS")
	{ }

	void carbeena(machine_config &config);

	virtual uint32_t io_expansion_r() override;

private:
	virtual void install_game_rom() override;

	required_ioport m_io_buttons;
};

void carbeena_state::carbeena(machine_config &config)
{
	sega_9h0_0008(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "carbeena_cart");
	m_cart->set_endian(ENDIANNESS_BIG);
	m_cart->set_width(GENERIC_ROM32_WIDTH);
	m_cart->set_device_load(FUNC(carbeena_state::cart_load));
	m_cart->set_must_be_loaded(false);

	SOFTWARE_LIST(config, "cart_list").set_original("carbeena_cart");
}

uint32_t carbeena_state::io_expansion_r()
{
	return m_io_buttons->read() & 0x3f;
}

void carbeena_state::install_game_rom()
{
	// TODO: These mappings are controlled by program code via writes to 0x60020004 and 0x60020010.
	// Likely only one of them should be mapped at a time (also suggested by test mode, which only
	// outputs 1 size out of 2 installed ROMs).
	if (m_cart->exists()) {
		std::string region_tag;
		memory_region *cart_rom = m_cart->memregion("cart:rom");
		m_maincpu->space(AS_PROGRAM).install_rom(ROM_FLASH_BASE, ROM_FLASH_BASE + 0x7fffff, 0x800000, cart_rom->base());
	}

	// Despite being a flash ROM, it gets mapped on the base address usually assigned to mask ROMs.
	memory_region *rom = memregion("mainboard_rom");
	m_maincpu->space(AS_PROGRAM).install_rom(ROM_MASK_BASE, ROM_MASK_BASE + 0x7fffff, 0x800000, rom->base());
}

static INPUT_PORTS_START( sega_9h0_0008 )
	PORT_START("VIDEO_CONFIG")
	PORT_CONFNAME( 0x80, 0x00, "Video Output" )
	PORT_CONFSETTING( 0x00, DEF_STR( On ) )
	PORT_CONFSETTING( 0x80, DEF_STR( Off ) )

	PORT_START("CPU_CONFIG")
	PORT_CONFNAME( 0x0f, 0x01, "IRQ Frequency" )
	PORT_CONFSETTING( 0x01, "1 (Once every VBLANK)" )
	PORT_CONFSETTING( 0x02, "2" )
	PORT_CONFSETTING( 0x03, "3" )
	PORT_CONFSETTING( 0x04, "4" )
	PORT_CONFSETTING( 0x05, "5" )
	PORT_CONFNAME( 0x10, 0x10, "IRQ Wait Speedup" )
	PORT_CONFSETTING( 0x00, DEF_STR( On ) )
	PORT_CONFSETTING( 0x10, DEF_STR( Off ) )
	PORT_CONFNAME( 0x60, 0x20, "Memory Cache" )
	PORT_CONFSETTING( 0x20, DEF_STR( On ) )
	PORT_CONFSETTING( 0x40, DEF_STR( Off ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sega_beena )
	PORT_INCLUDE( sega_9h0_0008 )

	PORT_START("PAGE_CONFIG")
	PORT_CONFNAME( 0x80, 0x80, "Test Mode Pages" )
	PORT_CONFSETTING( 0x00, DEF_STR( On ) )
	PORT_CONFSETTING( 0x80, DEF_STR( Off ) )

	PORT_START("PAD_LEFT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Left Red Button")

	PORT_START("PAD_RIGHT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Right Red Button")

	PORT_START("PEN_LEFT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Left Pen Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_TOGGLE PORT_PLAYER(1) PORT_NAME("Left Pen Target")

	PORT_START("PEN_RIGHT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Right Pen Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_TOGGLE PORT_PLAYER(1) PORT_NAME("Right Pen Target")

	PORT_START("PAGE")
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(7) PORT_WRAPS PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CODE(JOYCODE_X) PORT_CODE_DEC(KEYCODE_HOME) PORT_CODE_INC(KEYCODE_END) PORT_FULL_TURN_COUNT(7) PORT_NAME("Selected Page")

	PORT_START("PENX")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("Pen X")

	PORT_START("PENY")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("Pen Y") PORT_CROSSHAIR_MAPPER_MEMBER(DEVICE_SELF, sega_beena_state, pen_y_mapper)
INPUT_PORTS_END

static INPUT_PORTS_START( tvochken )
	PORT_INCLUDE( sega_9h0_0008 )

	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("A")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("B")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("C")

	PORT_START("CARDS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Scan Card") PORT_WRITE_LINE_MEMBER(tvochken_state, scan_card)
INPUT_PORTS_END

static INPUT_PORTS_START( carbeena )
	PORT_INCLUDE( sega_9h0_0008 )

	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_NAME("Handle Left")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_NAME("Handle Limit") // TODO: Likely set when turning the handle to full lock
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Handle Right")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Blue Button") // At the right of the center button, despite being mapped to a lower bit
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Center Button")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Yellow Button")
INPUT_PORTS_END

#define ROM_9H0_0008 \
	/* SoC internal BIOS dumped with a JTAG debugger. */ \
	ROM_REGION32_BE( 0x20000, "maincpu", 0 ) \
	ROM_LOAD( "9h0-0008.bios.ic1", 0x00000000, 0x20000, CRC(5471aaf8) SHA1(33d756b6c64afb0fa377b3f85ab74505e9ae2f9c) ) \
	/* SoC MIDI synthesizer parameters and PCM data. */ \
	/* Control ROM doesn't appear to be memory-mapped, so this data will only */ \
	/* be usable when the synthesizer gets reverse engineered. */ \
	ROM_REGION32_BE( 0x8000, "midipcm", 0 ) \
	ROM_LOAD( "9h0-0008.midipcm.ic1", 0x00000000, 0x8000, CRC(ed336d29) SHA1(4af7b7cf7fc4fe8b7a514cde91f930a582742509) )

ROM_START( beena )
	ROM_9H0_0008
ROM_END

ROM_START( tvochken )
	ROM_9H0_0008

	ROM_REGION32_BE( 0x400000, "flash_rom", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "m5m29gt320vp-80.u3", 0x00000000, 0x400000, CRC(75c1fbc1) SHA1(b07adcabaadb8b684335f52dd953f4696585c819) )
ROM_END

ROM_START( carbeena )
	ROM_9H0_0008

	ROM_REGION32_BE( 0x800000, "mainboard_rom", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "en29lv640b.ic9", 0x00000000, 0x800000, CRC(ce6649df) SHA1(e09568b93c9ab0901c6eb32d5e0408e484d4c564) )
ROM_END

} // anonymous namespace

//    year, name,     parent, compat, machine,    input,      class,            init,       company,                                fullname,              flags
CONS( 2005, beena,    0,      0,      sega_beena, sega_beena, sega_beena_state, empty_init, "Sega Toys",                            "Advanced Pico BEENA", MACHINE_IMPERFECT_GRAPHICS|MACHINE_IMPERFECT_TIMING|MACHINE_IMPERFECT_SOUND )
CONS( 2005, tvochken, 0,      0,      tvochken,   tvochken,   tvochken_state,   empty_init, "Sega Toys",                            "TV Ocha-Ken",         MACHINE_IMPERFECT_GRAPHICS|MACHINE_IMPERFECT_TIMING|MACHINE_IMPERFECT_SOUND )
// Release date 2009-12: http://products.alpine.co.jp/om/owner/product?P1=1632
CONS( 2009, carbeena, 0,      0,      carbeena,   carbeena,   carbeena_state,   empty_init, "Sega Toys / Alpine Electronics, Inc.", "Car Beena",           MACHINE_IMPERFECT_GRAPHICS|MACHINE_IMPERFECT_TIMING|MACHINE_IMPERFECT_SOUND )
