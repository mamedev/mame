// license:BSD-3-Clause
// copyright-holders:David Haywood, QUFB
/************************************************************************

    Tomy Prin-C driver for the following models:

    - Prin-C (ジヨイホビーコンピューター プリンシー): Built-in keyboard and unused(?) Mini-DIN-4 mouse port;
    - Prin-C Tablet (マルチホビーコンピューター プリンシータブレット): Built-in keyboard with touch-pad;

    TODO:

    - Flash ROM support (used by Memory Cassette)
    - Printer (graphics to print are previously transferred to work area);
    - Audio;

    Hardware
    --------

    [princnt]

    - U1,U2,U7,U9,U11 (Hex Inverter): Motorola 74HC04
    - U3 (ROM): Toshiba TC538000AF (1M x 8-bit)
    - U4 (SRAM): Hyundai HY62256A / Winbond W24257S-70LL (32K x 8-bit)
    - U5 (DRAM): IBM 014400J1F / Hitachi HM514400C (1M x 4-bit)
    - U6 (MCU): Fujitsu MB90611A (FPT-100P-M06 package, F2MC-16L ISA)
    - U8 (VDP): FInput(?) FIT-V01
    - U12,U13 (Stepper Motor): Shindengen MTD2003F
    - X1 (Crystal): D143K6

    There are 2 known revisions given by the mask ROM's serial: PRIN-C #1 and #3. #1 has
    an additional Motorola MC74HC107 (Dual J-K Flip-Flop) with bodge wires soldered to
    connector CN1 and motor U12, which is absent in #3.

    Although the mouse port is connected to MCU pins P90 and P91, it is only read once
    during system init, and stored in memory at 0xE07BD5. This variable is then
    accessed once during printing, and doesn't seem to influence its operation.

    Memory Cassette can be emulated by loading a cart using a 0x40000-sized zero-filled
    binary with the following header:

    00000000: 0000 0000 5241 4d20 5241 4d20 4441 5441  ....RAM RAM DATA

    Flash ROM access:

    [:maincpu] ':maincpu' (FCA6DE): unmapped program memory write to 805555 = F0 & FF
    [:maincpu] ':maincpu' (FCA6EA): unmapped program memory write to 805555 = AA & FF
    [:maincpu] ':maincpu' (FCA6F6): unmapped program memory write to 802AAA = 55 & FF
    [:maincpu] ':maincpu' (FCA702): unmapped program memory write to 805555 = 80 & FF
    [:maincpu] ':maincpu' (FCA70E): unmapped program memory write to 805555 = AA & FF
    [:maincpu] ':maincpu' (FCA71A): unmapped program memory write to 802AAA = 55 & FF
    [:maincpu] ':maincpu' (FCA726): unmapped program memory write to 805555 = 10 & FF

    [princ]

    - U2 (DRAM): NEC 424400-70 (1M x 4-bit)
    - U4 (Flash ROM): Fujitsu 29F800TA-90 (1M x 8/512K x 16-bit)
    - U5 (SRAM): Winbond W24257S-70LL (32K x 8-bit)
    - U6 (MCU): Fujitsu MB90611A (FPT-100P-M06 package, F2MC-16L ISA)
    - U7 (VDP): Tomy TEE-X01
    - U12,U13 (PWM Motor): Allegro MicroSystems A3964SLB
    - X1 (Crystal): D143C8

    Init trace:

    [:maincpu] ':maincpu' (FFC431): unmapped program memory write to 0000A1 = 00 & FF  CKSCR
    [:maincpu] ':maincpu' (FFC437): unmapped program memory write to 000048 = 04 & FF  CS control 0  (Enable out, region is 1 MByte @ F00000)
    [:maincpu] ':maincpu' (FFC43D): unmapped program memory write to 000049 = 04 & FF  CS control 1  (Enable out, region is 1 MByte @ E00000)
    [:maincpu] ':maincpu' (FFC443): unmapped program memory write to 00004A = 07 & FF  CS control 2  (Enable out, region is 128 byte @ 68FF80)
    [:maincpu] ':maincpu' (FFC449): unmapped program memory write to 00004B = 00 & FF  CS control 3  (No out, region is reserved)
    [:maincpu] ':maincpu' (FFC44F): unmapped program memory write to 0000A5 = D3 & FF  ARSR (3 cycle wait state from addrs 002000 to 7FFFFF, 3 waits from C0 to FF, 1 cycle wait on addresses > 800000)
    [:maincpu] ':maincpu' (FFC455): unmapped program memory write to 0000A6 = 00 & FF  HACR ()
    [:maincpu] ':maincpu' (FFC45B): unmapped program memory write to 0000A7 = 7F & FF  ECSR
    [:maincpu] ':maincpu' (FFC461): unmapped program memory write to 000011 = 00 & FF  Port 1 DDR
    [:maincpu] ':maincpu' (FFC467): unmapped program memory write to 000012 = FF & FF       2
    [:maincpu] ':maincpu' (FFC46D): unmapped program memory write to 000013 = FF & FF       3
    [:maincpu] ':maincpu' (FFC473): unmapped program memory write to 000014 = FF & FF       4
    [:maincpu] ':maincpu' (FFC479): unmapped program memory write to 000015 = 01 & FF       5
    [:maincpu] ':maincpu' (FFC47F): unmapped program memory write to 000016 = 1F & FF  Analog input enable
    [:maincpu] ':maincpu' (FFC485): unmapped program memory write to 000016 = E0 & FF       7
    [:maincpu] ':maincpu' (FFC48B): unmapped program memory write to 000017 = 30 & FF       8
    [:maincpu] ':maincpu' (FFC491): unmapped program memory write to 000018 = 0C & FF       9
    [:maincpu] ':maincpu' (FFC497): unmapped program memory write to 00001A = FF & FF       A
    [:maincpu] ':maincpu' (FFC189): unmapped program memory write to 00000A = 00 & FF  port A
    [:maincpu] ':maincpu' (FFC257): unmapped program memory write to 00000A = 80 & FF  port A
    [:maincpu] ':maincpu' (FE2C08): unmapped program memory write to 0000A9 = 96 & FF  TBTC - IRQ enabled, 16.384 ms timebase
    [:maincpu] ':maincpu' (FE2C11): unmapped program memory write to 0000BB = 06 & FF  ICR11 - level 6 interrupt, no intelligent I/O
    [:maincpu] ':maincpu' (FE2959): unmapped program memory write to 000017 = 30 & FF  port 7 DDR
    [:maincpu] ':maincpu' (FE2963): unmapped program memory write to 0000A9 = 96 & FF  TBTC
    [:maincpu] ':maincpu' (FE296C): unmapped program memory write to 0000BB = 06 & FF  ICR11
    [:maincpu] ':maincpu' (FE29CC): unmapped program memory write to 000007 = 00 & FF  port 7 out
    [:maincpu] ':maincpu' (FE2A69): unmapped program memory write to 0000A9 = 96 & FF  TBTC
    [:maincpu] ':maincpu' (FE2A72): unmapped program memory write to 0000BB = 06 & FF  ICR11
    [:maincpu] ':maincpu' (FC2AD5): unmapped program memory write to 000018 = 0C & FF  port 8 DDR
    [:maincpu] ':maincpu' (FC2ADE): unmapped program memory write to 000039 = 0C & FF  TMCSR0 (clock = phase 16 MHz / 2^1, trigger input, rising edge)
    [:maincpu] ':maincpu' (FC2AE8): unmapped program memory write to 000038 = F0 & FF  TMCSR0 (toggle output, H Level at start, no count or interrupt enable)
    [:maincpu] ':maincpu' (FC2AF1): unmapped program memory write to 00003D = 0C & FF  TMCSR1
    [:maincpu] ':maincpu' (FC2AFB): unmapped program memory write to 00003C = F0 & FF  TMCSR1
    [:maincpu] ':maincpu' (FE2B89): unmapped program memory write to 000007 = 00 & FF  port 7 out
    [:maincpu] ':maincpu' (FCE68D): unmapped program memory write to 000007 = 10 & FF  port 7 out
    [:maincpu] ':maincpu' (FCE6BA): unmapped program memory write to 000034 = 73 & FF  PRL0 (PPG0 reload)
    [:maincpu] ':maincpu' (FCE6D3): unmapped program memory write to 000036 = 0D & FF  PRL1 (PPG1 reload)
    [:maincpu] ':maincpu' (FCE6DE): unmapped program memory write to 000030 = 85 & FF  PPG0C0 (PPG0 control)  (PPG Enabled, 16 MHz / 16, no interrupts)

    Hidden Features
    ---------------

    [princnt] On reset, press one of the following key combinations:

    - "T" + "M": Developer credits are shown before the logo screen;
    - "Print" + "1".."7": Print 1 out of 7 pre-defined patterns;
    - "Menu" + "0": Label "デモ" (Demo) is overlayed on several screens;

************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/f2mc16/mb9061x.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#define LOG_VDP (1U << 1)
#define LOG_REG (1U << 2)

//#define VERBOSE (LOG_VDP | LOG_REG)
#include "logmacro.h"

// Renders each dedicated area/layer, toggled by pressing keys "ASDFG"
//#define VIDEO_DEBUG 1

namespace {

class tomy_princ_state : public driver_device
{
public:
	tomy_princ_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_scantimer(*this, "scantimer")
		, m_io_keys(*this, "KEY%u", 1U)
	{
#ifdef VIDEO_DEBUG
		SCREEN_W = BITPLANE_W;
		SCREEN_H = LAYER_H;
#else
		SCREEN_W = 360;
		SCREEN_H = 240;
#endif
		SRC_POS = 0x20;
		DST_POS = 0x24;
		SHAPE_SIZE = 0x28;
		SHAPE_DIRECTION = 0x2c;
		SHAPE_MODE = 0x2d;
		SHAPE_PAL = 0x30;
		RENDER_CMD = 0x31;
		RENDER2_CMD = 0x31;
		LAYER_MODE = 0xc2;
		LAYER2_MODE = 0xc2;
		LAYER_OFFSET_Y = 0xc3;
		LAYER_OFFSET_X = 0xc4;
		LAYER_CLIP_H = 0xc5;
		LAYER_CLIP_W = 0xc6;
		SCREEN_OFFSET = 0xc9;
		PAL_DATA = 0xd1;
		SPRITE1_DST_POS_Y = 0xd2;
		SPRITE1_DST_POS_X = 0xd3;
		SPRITE1_SELECT = 0xd4;
		SPRITE2_DST_POS_Y = 0xd5;
		SPRITE2_DST_POS_X = 0xd6;
		SPRITE2_SELECT = 0xd7;
	}

	void tomy_princ(machine_config &config);

protected:
	static inline constexpr u16 BITPLANE_W = 0x200;
	static inline constexpr u16 BITPLANE_H = 0x800;
	static inline constexpr u16 LAYER_H = 0x400;
	static inline constexpr u16 LAYER_WORKAREA_H = 0x3c0;
	static inline constexpr u16 LAYER_SPRITES_H = 0x40;
	static inline constexpr u16 SPRITE_SIZE = 0x20;

	// Visible area screen dimensions
	u16 SCREEN_W;
	u16 SCREEN_H;

	// VDP register indexes
	u16 SRC_POS;
	u16 DST_POS;
	u16 SHAPE_SIZE;
	u16 SHAPE_DIRECTION;
	u16 SHAPE_MODE;
	u16 SHAPE_PAL;
	u16 RENDER_CMD;
	u16 RENDER2_CMD;
	u16 LAYER_MODE;
	u16 LAYER2_MODE;
	u16 LAYER_OFFSET_Y;
	u16 LAYER_OFFSET_X;
	u16 LAYER_CLIP_H;
	u16 LAYER_CLIP_W;
	u16 SCREEN_OFFSET;
	u16 PAL_DATA;
	u16 SPRITE1_DST_POS_Y;
	u16 SPRITE1_DST_POS_X;
	u16 SPRITE1_SELECT;
	u16 SPRITE2_DST_POS_Y;
	u16 SPRITE2_DST_POS_X;
	u16 SPRITE2_SELECT;

	virtual u8 vdp_stat_r();

	u8 pdr8_r();
	u8 pdra_r();
	u8 key_r();
	virtual u8 key_to_pdr_bit(u8 port, u8 bit);

	enum render_cmd_type : u8
	{
		BUFFER_WRITE = 0x10,
		PUT_SHAPE_RECT = 0x20,
		BUFFER_READ = 0x30,
		PUT_FROM_BUFFER = 0x40,
		PUT_SHAPE_LINE = 0x50,
		BUFFER_FIND_BOUND = 0x60,
		BUFFER_READ_PIXEL = 0x70,
		PUT_SHAPE_PIXEL = 0x80,
	};

	required_device<mb90611_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<screen_device> m_screen;

	/*
	    FIT-V01 is a single bitplane VDP, with graphics stored as 4bpp
	    indexed to 16 palette colors, where the first entry is used as
	    transparency. It has limited support for drawing lines and
	    rendering sprites (seems like up to 2 are visible).

	    This bitplane has width=0x200 and height=0x800, separated by
	    height value into 3 distinct ranges:

	    - Visible area:     0..0x400;
	    - Work area:    0x400..0x7c0;
	    - Sprites:      0x7c0..0x800;

	    Visible area is dynamically clipped to 360x240 and can also be xy-offset by
	    writing to specific VDP internal registers. These writes are done by
	    first setting the register offset at address 68FF41, then writing data to
	    address 68FF42, which implicitly increments the register offset.

	    TEE-X01 is functionally very close to FIT-V01, but with different register
	    offsets and usage patterns (e.g. allows reading directly from internal buffer, and
	    clearing the bitplane in a single operation spanning the full range, instead of
	    3 operations for each discrete area).
	*/
	struct vdp_state {
		u8 regs[0x100];
		u8 reg;
		u32 reg_data_i;
		u16 screen_offset_x;
		u16 screen_offset_y;
		u16 offset_x;
		u16 offset_y;
		u16 src_pos_x;
		u16 src_pos_y;
		u16 dst_pos_x;
		u16 dst_pos_y;
		u16 size_x;
		u16 size_y;
		u8 shape_direction;
		u8 shape_blend;
		u8 shape_transform;
		u8 shape_pal_i;
		u16 sprite_dst_pos_x;
		u16 sprite_dst_pos_y;
		u16 sprite_i;
		u16 sprite_transform;
		u16 sprite2_dst_pos_x;
		u16 sprite2_dst_pos_y;
		u16 sprite2_i;
		u16 sprite2_transform;
		u16 sprite_size_x;
		u16 sprite_size_y;
		u32 pending_pixels;
		u8 layer_mode;
		u8 pal_r;
		u8 pal_g;
		u8 pal_b;
		u32 pal_i;
		u16 bound_x;
	} m_vdp;

	bool m_is_bound_found;
	bool m_is_overlay_enabled;

private:
	enum layer_target : u8
	{
		DEFAULT_TARGET,
		OVERLAY_TARGET,
		SPRITES_TARGET,
		SPRITES_PRIORITY_TARGET,
	};

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void video_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(scan_interrupt);

	DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	void princ_map(address_map &map);

	void princ_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_update_layer(u8 *layer, u16 layer_offset, u16 layer_w, u16 layer_h, screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void clear_screen();
	void clear_overlay();
	void vdp_update();
	void vdp_sprite_update();
	void vdp_reg_w(u8 data);
	u8 vdp_reg_data_r();
	void vdp_reg_data_w(u8 data);
	void vdp_pal_data_w(u8 data);
	u8 vdp_gfx_data_r();
	void vdp_gfx_data_w(u8 data);
	void draw_pixel(u8 color, u32 i, u8 layer_target, bool is_vertical, bool is_xor);
	u8 read_pixel(u32 i);
	void draw_pixel_at(u8 *layer, u16 layer_w, u16 layer_h, u16 pos_x, u16 pos_y, u8 color, u32 i, bool is_overlay, bool is_vertical, bool is_xor);
	u8 read_pixel_at(u8 *layer, u16 layer_w, u16 layer_h, u16 pos_x, u16 pos_y, u32 i);
	void draw_sprite(u32 dst_pos_x, u32 dst_pos_y, u32 i, u32 transform, bool is_priority);

	required_device<palette_device> m_palette;
	required_device<timer_device> m_scantimer;
	required_ioport_array<9> m_io_keys;

	std::unique_ptr<u8[]> m_layer;
	std::unique_ptr<u8[]> m_layer_overlay_sprites;
	std::unique_ptr<u8[]> m_layer_overlay_sprites_priority;

	u8 m_ext_reg0C;
	bool m_is_ext_reg00_read;
};

TIMER_DEVICE_CALLBACK_MEMBER(tomy_princ_state::scan_interrupt)
{
	for (size_t i = 0; i < 128; i++)
	{
		m_maincpu->tin0_w(ASSERT_LINE);
		m_maincpu->tin0_w(CLEAR_LINE);
	}

	m_maincpu->tin1_w(ASSERT_LINE);
	m_maincpu->tin1_w(CLEAR_LINE);
}

void tomy_princ_state::machine_start()
{
	if (m_cart->exists())
	{
		memory_region *const cart_rom = m_cart->memregion("rom");
		m_maincpu->space(AS_PROGRAM).install_rom(0x800000, 0x87ffff, cart_rom->base());
	}

	save_item(NAME(m_ext_reg0C));
	save_item(NAME(m_is_ext_reg00_read));
}

void tomy_princ_state::machine_reset()
{
	m_ext_reg0C = 0;
	m_is_ext_reg00_read = false;
}

void tomy_princ_state::video_start()
{
	m_layer = std::make_unique<u8[]>(BITPLANE_W * BITPLANE_H);
	m_layer_overlay_sprites = std::make_unique<u8[]>(BITPLANE_W * LAYER_H);
	m_layer_overlay_sprites_priority = std::make_unique<u8[]>(BITPLANE_W * LAYER_H);
	save_pointer(NAME(m_layer), BITPLANE_W * BITPLANE_H);
	save_pointer(NAME(m_layer_overlay_sprites), BITPLANE_W * LAYER_H);
	save_pointer(NAME(m_layer_overlay_sprites_priority), BITPLANE_W * LAYER_H);

	save_item(NAME(m_is_bound_found));
	save_item(NAME(m_is_overlay_enabled));

	save_item(STRUCT_MEMBER(m_vdp, regs));
	save_item(STRUCT_MEMBER(m_vdp, reg));
	save_item(STRUCT_MEMBER(m_vdp, reg_data_i));
	save_item(STRUCT_MEMBER(m_vdp, screen_offset_x));
	save_item(STRUCT_MEMBER(m_vdp, screen_offset_y));
	save_item(STRUCT_MEMBER(m_vdp, offset_x));
	save_item(STRUCT_MEMBER(m_vdp, offset_y));
	save_item(STRUCT_MEMBER(m_vdp, src_pos_x));
	save_item(STRUCT_MEMBER(m_vdp, src_pos_y));
	save_item(STRUCT_MEMBER(m_vdp, dst_pos_x));
	save_item(STRUCT_MEMBER(m_vdp, dst_pos_y));
	save_item(STRUCT_MEMBER(m_vdp, size_x));
	save_item(STRUCT_MEMBER(m_vdp, size_y));
	save_item(STRUCT_MEMBER(m_vdp, shape_direction));
	save_item(STRUCT_MEMBER(m_vdp, shape_blend));
	save_item(STRUCT_MEMBER(m_vdp, shape_transform));
	save_item(STRUCT_MEMBER(m_vdp, shape_pal_i));
	save_item(STRUCT_MEMBER(m_vdp, sprite_dst_pos_x));
	save_item(STRUCT_MEMBER(m_vdp, sprite_dst_pos_y));
	save_item(STRUCT_MEMBER(m_vdp, sprite_i));
	save_item(STRUCT_MEMBER(m_vdp, sprite_transform));
	save_item(STRUCT_MEMBER(m_vdp, sprite2_dst_pos_x));
	save_item(STRUCT_MEMBER(m_vdp, sprite2_dst_pos_y));
	save_item(STRUCT_MEMBER(m_vdp, sprite2_i));
	save_item(STRUCT_MEMBER(m_vdp, sprite2_transform));
	save_item(STRUCT_MEMBER(m_vdp, sprite_size_x));
	save_item(STRUCT_MEMBER(m_vdp, sprite_size_y));
	save_item(STRUCT_MEMBER(m_vdp, pending_pixels));
	save_item(STRUCT_MEMBER(m_vdp, layer_mode));
	save_item(STRUCT_MEMBER(m_vdp, pal_r));
	save_item(STRUCT_MEMBER(m_vdp, pal_g));
	save_item(STRUCT_MEMBER(m_vdp, pal_b));
	save_item(STRUCT_MEMBER(m_vdp, pal_i));
	save_item(STRUCT_MEMBER(m_vdp, bound_x));
}

void tomy_princ_state::video_reset()
{
	memset(&m_vdp, 0, sizeof(m_vdp));
	m_vdp.sprite_size_x = SPRITE_SIZE;
	m_vdp.sprite_size_y = SPRITE_SIZE;

	clear_screen();
}

void tomy_princ_state::clear_screen()
{
	std::fill_n(m_layer.get(), BITPLANE_W * BITPLANE_H, 0);

	clear_overlay();
}

void tomy_princ_state::clear_overlay()
{
	std::fill_n(m_layer_overlay_sprites.get(), BITPLANE_W * LAYER_H, 0);
	std::fill_n(m_layer_overlay_sprites_priority.get(), BITPLANE_W * LAYER_H, 0);
}

u8 tomy_princ_state::pdr8_r()
{
	if (m_is_ext_reg00_read)
	{
		m_is_ext_reg00_read = false;
		return 0x20;
	}

	return 0x00;
}

u8 tomy_princ_state::pdra_r()
{
	return m_ext_reg0C == 0x0c ? 0 : 0x10;
}

void tomy_princ_state::vdp_reg_w(u8 data)
{
	LOGMASKED(LOG_VDP, "%s: reg=%02x\n", machine().describe_context(), data);

	m_vdp.reg = data;
	m_vdp.reg_data_i = 0;
	m_vdp.pal_i = 0;
}

void tomy_princ_state::vdp_update()
{
	m_vdp.src_pos_x = (m_vdp.regs[SRC_POS + 1] << 8) | m_vdp.regs[SRC_POS];
	m_vdp.src_pos_y = (m_vdp.regs[SRC_POS + 3] << 8) | m_vdp.regs[SRC_POS + 2];
	m_vdp.dst_pos_x = (m_vdp.regs[DST_POS + 1] << 8) | m_vdp.regs[DST_POS];
	m_vdp.dst_pos_y = (m_vdp.regs[DST_POS + 3] << 8) | m_vdp.regs[DST_POS + 2];

	m_vdp.size_x = ((m_vdp.regs[SHAPE_SIZE + 1] << 8) | m_vdp.regs[SHAPE_SIZE]) + 1;
	m_vdp.size_y = ((m_vdp.regs[SHAPE_SIZE + 3] << 8) | m_vdp.regs[SHAPE_SIZE + 2]) + 1;

	m_vdp.shape_direction = m_vdp.regs[SHAPE_DIRECTION];
	m_vdp.shape_blend = m_vdp.regs[SHAPE_MODE] & 0x0f;
	m_vdp.shape_transform = (m_vdp.regs[SHAPE_MODE] & 0xf0) >> 4;
	m_vdp.shape_pal_i = (m_vdp.regs[SHAPE_PAL] & 0xf0) >> 4;

	m_vdp.offset_y = (m_vdp.regs[SCREEN_OFFSET + 1] << 8) | m_vdp.regs[SCREEN_OFFSET];
	m_vdp.offset_x = (m_vdp.regs[SCREEN_OFFSET + 3] << 8) | m_vdp.regs[SCREEN_OFFSET + 2];
	if (m_vdp.regs[LAYER_OFFSET_X] != 0 && m_vdp.regs[LAYER_OFFSET_Y] != 0)
	{
		m_vdp.screen_offset_y = m_vdp.regs[LAYER_OFFSET_Y] * 2;
		m_vdp.screen_offset_x = m_vdp.regs[LAYER_OFFSET_X] * 2;
	}
	else
	{
		m_vdp.screen_offset_y = m_vdp.offset_y;
		m_vdp.screen_offset_x = m_vdp.offset_x;
	}
}

void tomy_princ_state::vdp_sprite_update()
{
	m_vdp.sprite_dst_pos_y = m_vdp.regs[SPRITE1_DST_POS_Y];
	m_vdp.sprite_dst_pos_x = m_vdp.regs[SPRITE1_DST_POS_X];
	m_vdp.sprite_i = m_vdp.regs[SPRITE1_SELECT] & 0xf;
	m_vdp.sprite_transform = (m_vdp.regs[SPRITE1_SELECT] >> 4) & 0xf;

	m_vdp.sprite2_dst_pos_y = m_vdp.regs[SPRITE2_DST_POS_Y];
	m_vdp.sprite2_dst_pos_x = m_vdp.regs[SPRITE2_DST_POS_X];
	m_vdp.sprite2_i = m_vdp.regs[SPRITE2_SELECT] & 0xf;
	m_vdp.sprite2_transform = (m_vdp.regs[SPRITE2_SELECT] >> 4) & 0xf;
}

u8 tomy_princ_state::vdp_reg_data_r()
{
	const u8 data = m_is_bound_found ? (m_vdp.bound_x >> (8 * m_vdp.reg_data_i++)) : 0;

	LOGMASKED(LOG_VDP, "%s: reg=%02x i=%02x r reg_data=%02x\n", machine().describe_context(), m_vdp.reg, m_vdp.reg_data_i, data);

	return data;
}

void tomy_princ_state::vdp_reg_data_w(u8 data)
{
	LOGMASKED(LOG_VDP, "%s: reg=%02x i=%02x reg_data=%02x\n", machine().describe_context(), m_vdp.reg, m_vdp.reg_data_i, data);

	m_vdp.regs[m_vdp.reg] = data;

	if (m_vdp.reg == RENDER_CMD || m_vdp.reg == RENDER2_CMD)
	{
		vdp_update();
		LOGMASKED(LOG_VDP, "%s:    src x=%04x y=%04x\n", machine().describe_context(), m_vdp.src_pos_x, m_vdp.src_pos_y);
		LOGMASKED(LOG_VDP, "%s:    dst x=%04x y=%04x\n", machine().describe_context(), m_vdp.dst_pos_x, m_vdp.dst_pos_y);
		LOGMASKED(LOG_VDP, "%s:   size x=%04x y=%04x\n", machine().describe_context(), m_vdp.size_x, m_vdp.size_y);
		LOGMASKED(LOG_VDP, "%s:  shape d=%02x b=%02x p=%02x\n", machine().describe_context(), m_vdp.shape_direction, m_vdp.shape_blend, m_vdp.shape_pal_i);

		switch (data)
		{
			case BUFFER_WRITE:
			case BUFFER_READ:
				m_vdp.pending_pixels = m_vdp.size_x * m_vdp.size_y;
				break;

			case BUFFER_READ_PIXEL:
				m_vdp.pending_pixels = 1;
				break;

			case PUT_SHAPE_RECT:
				for (size_t i = 0; i < m_vdp.size_x * m_vdp.size_y; i++)
				{
					draw_pixel(m_vdp.shape_pal_i, i, OVERLAY_TARGET, false, false);
				}
				break;

			case PUT_FROM_BUFFER:
				for (size_t i = 0; i < m_vdp.size_x * m_vdp.size_y; i++)
				{
					draw_pixel(read_pixel(i), i, OVERLAY_TARGET, false, false);
				}
				break;

			case PUT_SHAPE_LINE:
			{
				m_vdp.size_y = 1;
				u32 offset = 0;
				if (!BIT(m_vdp.shape_direction, 0))
				{
					// horizontal direction
					for (size_t i = 0; i < m_vdp.size_x; i++)
					{
						draw_pixel(m_vdp.shape_pal_i, offset, OVERLAY_TARGET, false, m_vdp.shape_blend == 0x6);
						if (!BIT(m_vdp.shape_direction, 3))
							offset++;
						else
							m_vdp.dst_pos_x--;
					}
				}
				else
				{
					// vertical direction
					for (size_t i = 0; i < m_vdp.size_x; i++)
					{
						draw_pixel(m_vdp.shape_pal_i, offset, OVERLAY_TARGET, true, m_vdp.shape_blend == 0x6);
						if (!BIT(m_vdp.shape_direction, 3))
							offset += m_vdp.size_x;
						else
							m_vdp.dst_pos_y--;
					}
				}
				break;
			}

			case BUFFER_FIND_BOUND:
				// Used by Prin-C Tablet's flood fill brush. Given the brush sprite's x-position,
				// left and right bounds for the same palette index are queried, followed by a
				// PUT_SHAPE_LINE command with size spanning these bounds.
				m_is_bound_found = false;
				m_vdp.bound_x = 0;
				m_vdp.size_x = BITPLANE_W - m_vdp.src_pos_x;
				m_vdp.size_y = 1;
				if (BIT(m_vdp.shape_direction, 1))
				{
					u16 i = 0;
					const u8 target_pixel = m_vdp.shape_pal_i;
					while ((m_vdp.src_pos_x + i >= 0) && (m_vdp.src_pos_x + i < BITPLANE_W))
					{
						const u8 candidate_pixel = read_pixel(i);
						if (candidate_pixel != target_pixel)
						{
							m_vdp.bound_x = m_vdp.src_pos_x + i;
							m_is_bound_found = true;
							LOGMASKED(LOG_VDP, "%s: reg=%02x bound_x=%04x (had=%02x got=%02x)\n", machine().describe_context(), m_vdp.reg, m_vdp.bound_x, target_pixel, candidate_pixel);
							break;
						}

						if (!BIT(m_vdp.shape_direction, 2))
							i++;
						else
							m_vdp.src_pos_x--;
					}
				}
				else
				{
					// Tests if a pixel was already visited / rendered, comparing the
					// candidate palette index against the index of the previously unfilled pixel.
					// If returned bound value is not zero, the query is retried. We need to return
					// the next x-position, so that program code doesn't keep retrying the same one.
					const u8 target_pixel = m_vdp.shape_pal_i & 0xf;
					const u8 candidate_pixel = read_pixel(0) & 0xf;
					LOGMASKED(LOG_VDP, "%s: reg=%02x d=0 (had=%02x got=%02x)\n", machine().describe_context(), m_vdp.reg, target_pixel, candidate_pixel);
					if (candidate_pixel != target_pixel)
					{
						m_vdp.bound_x = m_vdp.src_pos_x + 1;
						m_is_bound_found = true;
						LOGMASKED(LOG_VDP, "%s: reg=%02x bound_x=%04x (had=%02x got=%02x)\n", machine().describe_context(), m_vdp.reg, m_vdp.bound_x, target_pixel, candidate_pixel);
					}
				}
				break;

			case PUT_SHAPE_PIXEL:
				m_vdp.size_x = 1;
				m_vdp.size_y = 1;
				draw_pixel(m_vdp.shape_pal_i, 0, OVERLAY_TARGET, false, m_vdp.shape_blend == 0x6);
				break;

			default:
				logerror("%s: Unhandled cmd=%02x\n", machine().describe_context(), data);
		}
	}
	else if (m_vdp.reg == LAYER_MODE || m_vdp.reg == LAYER2_MODE)
	{
		m_vdp.layer_mode = data;
		if (BIT(m_vdp.layer_mode, 1))
		{
			vdp_update();
			vdp_sprite_update();
			LOGMASKED(LOG_VDP, "%s: sprite size x=%04x y=%04x\n", machine().describe_context(), m_vdp.sprite_size_x, m_vdp.sprite_size_y);
			LOGMASKED(LOG_VDP, "%s: sprite1 dst x=%04x y=%04x\n", machine().describe_context(), m_vdp.sprite_dst_pos_x, m_vdp.sprite_dst_pos_y);
			LOGMASKED(LOG_VDP, "%s: sprite1 sel i=%02x t=%02x\n", machine().describe_context(), m_vdp.sprite_i, m_vdp.sprite_transform);
			LOGMASKED(LOG_VDP, "%s: sprite2 dst x=%04x y=%04x\n", machine().describe_context(), m_vdp.sprite2_dst_pos_x, m_vdp.sprite2_dst_pos_y);
			LOGMASKED(LOG_VDP, "%s: sprite2 sel i=%02x t=%02x\n", machine().describe_context(), m_vdp.sprite2_i, m_vdp.sprite2_transform);

			clear_overlay();

			bool is_sprite_priority_set = m_vdp.offset_y & 0x8000;
			draw_sprite(m_vdp.sprite_dst_pos_x, m_vdp.sprite_dst_pos_y, m_vdp.sprite_i, m_vdp.sprite_transform, is_sprite_priority_set);
			draw_sprite(m_vdp.sprite2_dst_pos_x, m_vdp.sprite2_dst_pos_y, m_vdp.sprite2_i, m_vdp.sprite2_transform, !is_sprite_priority_set);

			m_is_overlay_enabled = true;
		}
		else
		{
			LOGMASKED(LOG_VDP, "%s: reg=%02x overlay disabled, m=%02x\n", machine().describe_context(), m_vdp.reg, m_vdp.layer_mode);

			m_is_overlay_enabled = false;
		}
	}

	m_vdp.reg++;
}

void tomy_princ_state::draw_sprite(u32 dst_pos_x, u32 dst_pos_y, u32 i, u32 transform, bool is_priority)
{
	if (BIT(transform, 2)) {
		return;
	}

	m_vdp.dst_pos_x = (BIT(transform, 1) ? 0x100 : 0) + dst_pos_x + (m_vdp.offset_x & 0x0fff);
	m_vdp.dst_pos_y = dst_pos_y + (m_vdp.offset_y & 0x0fff);
	m_vdp.src_pos_x = 0x20 * i;
	m_vdp.src_pos_y = (BIT(transform, 0) ? 0x20 : 0) + LAYER_H + LAYER_WORKAREA_H;
	m_vdp.size_x = m_vdp.sprite_size_x;
	m_vdp.size_y = m_vdp.sprite_size_y;
	LOGMASKED(LOG_VDP, "%s: sprite draw sx=%04x sy=%04x dx=%04x dy=%04x\n", machine().describe_context(), m_vdp.src_pos_x, m_vdp.src_pos_y, m_vdp.dst_pos_x, m_vdp.dst_pos_y);

	for (size_t i = 0; i < m_vdp.size_x * m_vdp.size_y; i++)
	{
		draw_pixel(read_pixel(i), i, is_priority ? SPRITES_PRIORITY_TARGET : SPRITES_TARGET, false, false);
	}
}

void tomy_princ_state::vdp_pal_data_w(u8 data)
{
	LOGMASKED(LOG_VDP, "%s: reg=%02x i=%04x reg_data=%02x\n", machine().describe_context(), m_vdp.reg, m_vdp.reg_data_i, data);

	if (m_vdp.reg == PAL_DATA + 1)
	{
		switch (m_vdp.reg_data_i)
		{
			case 0:
				m_vdp.pal_r = data * 0xff / 0x3f;
				break;
			case 1:
				m_vdp.pal_g = data * 0xff / 0x3f;
				break;
			case 2:
				m_vdp.pal_b = data * 0xff / 0x3f;
				m_palette->set_pen_color(m_vdp.pal_i, m_vdp.pal_r, m_vdp.pal_g, m_vdp.pal_b);
				LOGMASKED(LOG_VDP, "%s: reg=%02x i=%04x pal=%02x%02x%02x\n",
						machine().describe_context(),
						m_vdp.reg,
						m_vdp.pal_i,
						m_vdp.pal_r,
						m_vdp.pal_g,
						m_vdp.pal_b);
				m_vdp.pal_i++;
				break;
		}
	}

	m_vdp.reg_data_i++;
	m_vdp.reg_data_i %= 3;
}

u8 tomy_princ_state::vdp_gfx_data_r()
{
	u8 data = 0;
	if (m_vdp.pending_pixels != 0)
	{
		data = (read_pixel(m_vdp.reg_data_i) & 0xf) << 4;
		m_vdp.reg_data_i++;
		m_vdp.pending_pixels--;
	}
	if (m_vdp.pending_pixels != 0)
	{
		data |= read_pixel(m_vdp.reg_data_i) & 0xf;
		m_vdp.reg_data_i++;
		m_vdp.pending_pixels--;
	}

	LOGMASKED(LOG_VDP, "%s: reg=%02x i=%04x k=%04x r gfx_data=%02x\n", machine().describe_context(), m_vdp.reg, m_vdp.reg_data_i, m_vdp.pending_pixels, data);

	return data;
}

void tomy_princ_state::vdp_gfx_data_w(u8 data)
{
	LOGMASKED(LOG_VDP, "%s: reg=%02x i=%04x k=%04x w gfx_data=%02x\n", machine().describe_context(), m_vdp.reg, m_vdp.reg_data_i, m_vdp.pending_pixels, data);

	if (m_vdp.pending_pixels != 0)
	{
		draw_pixel((data & 0xf0) >> 4, m_vdp.reg_data_i, DEFAULT_TARGET, false, false);
		m_vdp.reg_data_i++;
		m_vdp.pending_pixels--;
	}
	if (m_vdp.pending_pixels != 0)
	{
		draw_pixel(data & 0x0f, m_vdp.reg_data_i, DEFAULT_TARGET, false, false);
		m_vdp.reg_data_i++;
		m_vdp.pending_pixels--;
	}
}

u8 tomy_princ_state::vdp_stat_r()
{
	return m_screen->vblank() ? 0x11 : 0x10;
}

void tomy_princ_state::draw_pixel(u8 color, u32 i, u8 layer_target, bool is_vertical, bool is_xor)
{
	const bool is_overlay = (layer_target == OVERLAY_TARGET);

	if (m_vdp.dst_pos_y < 0x800)
	{
		draw_pixel_at(layer_target == SPRITES_TARGET
						? m_layer_overlay_sprites.get()
						: layer_target == SPRITES_PRIORITY_TARGET
							? m_layer_overlay_sprites_priority.get()
							: m_layer.get(),
						BITPLANE_W,
						BITPLANE_H,
						m_vdp.dst_pos_x,
						m_vdp.dst_pos_y,
						color,
						i,
						is_overlay,
						is_vertical,
						is_xor);
	}
	else
	{
		logerror("%s: Unhandled w dst y=%04x\n", machine().describe_context(), m_vdp.dst_pos_y);
	}
}

void tomy_princ_state::draw_pixel_at(u8 *layer, u16 layer_w, u16 layer_h, u16 pos_x, u16 pos_y, u8 color, u32 i, bool is_overlay, bool is_vertical, bool is_xor)
{
	if (m_vdp.size_x == 0)
	{
		logerror("%s: Unhandled w size_x=0\n", machine().describe_context());
		return;
	}
	if (is_vertical)
	{
		if (pos_x + m_vdp.size_y > BITPLANE_H)
		{
			logerror("%s: OOB w x+y=%04x+%04x\n", machine().describe_context(), m_vdp.dst_pos_x, m_vdp.size_y);
			return;
		}
	}
	else
	{
		if (pos_x + m_vdp.size_x > BITPLANE_W)
		{
			logerror("%s: OOB w x=%04x+%04x\n", machine().describe_context(), m_vdp.dst_pos_x, m_vdp.size_x);
			return;
		}
		if (pos_y + m_vdp.size_y > BITPLANE_H)
		{
			logerror("%s: OOB w y=%04x+%04x\n", machine().describe_context(), m_vdp.dst_pos_y, m_vdp.size_y);
			return;
		}
	}

	const u32 base_offset = layer_w * pos_y + pos_x;
	const u32 gfx_offset = base_offset + layer_w * (i / m_vdp.size_x) + (i % m_vdp.size_x);
	if (gfx_offset > layer_w * layer_h)
	{
		logerror("%s: OOB w o=%08x x=%04x y=%04x\n", machine().describe_context(), gfx_offset, m_vdp.dst_pos_x, m_vdp.dst_pos_y);
		return;
	}
	if (is_overlay || color != 0)
	{
		if (is_xor)
		{
			layer[gfx_offset] ^= color;
			layer[gfx_offset] &= 0xf;
		}
		else
		{
			layer[gfx_offset] = color;
		}
	}
}

u8 tomy_princ_state::read_pixel(u32 i)
{
	if (m_vdp.src_pos_y < 0x800)
	{
		return read_pixel_at(m_layer.get(), BITPLANE_W, BITPLANE_H, m_vdp.src_pos_x, m_vdp.src_pos_y, i);
	}
	else
	{
		logerror("%s: Unhandled r src y=%04x\n", machine().describe_context(), m_vdp.src_pos_y);
	}

	return 0;
}

u8 tomy_princ_state::read_pixel_at(u8 *layer, u16 layer_w, u16 layer_h, u16 pos_x, u16 pos_y, u32 i)
{
	if (m_vdp.size_x == 0)
	{
		logerror("%s: Unhandled r size_x=0\n", machine().describe_context());
		return 0;
	}
	if (pos_x + m_vdp.size_x > BITPLANE_W)
	{
		logerror("%s: OOB r x=%04x+%04x\n", machine().describe_context(), m_vdp.dst_pos_x, m_vdp.size_x);
		return 0;
	}
	if (pos_y + m_vdp.size_y > BITPLANE_H)
	{
		logerror("%s: OOB r y=%04x+%04x\n", machine().describe_context(), m_vdp.dst_pos_y, m_vdp.size_y);
		return 0;
	}

	const u32 base_offset = layer_w * pos_y + pos_x;
	const u32 gfx_offset = base_offset + layer_w * (i / m_vdp.size_x) + (i % m_vdp.size_x);
	if (gfx_offset > layer_w * layer_h)
	{
		logerror("%s: OOB r o=%08x\n", machine().describe_context(), gfx_offset);
		return 0;
	}

	return layer[gfx_offset];
}

void tomy_princ_state::princ_palette(palette_device &palette) const
{
	// These entries are dynamically set at program start, assuming black as default.
	for (size_t i = 0; i < 16; i++)
	{
		palette.set_pen_color(i, 0, 0, 0);
	}
}

u32 tomy_princ_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

#ifdef VIDEO_DEBUG
	if (machine().input().code_pressed(KEYCODE_A))
	{
		screen_update_layer(m_layer.get(), LAYER_H, BITPLANE_W, LAYER_WORKAREA_H, screen, bitmap, cliprect);
		return 0;
	}
	else if (machine().input().code_pressed(KEYCODE_S))
	{
		screen_update_layer(m_layer.get(), 0, BITPLANE_W, LAYER_H, screen, bitmap, cliprect);
		return 0;
	}
	else if (machine().input().code_pressed(KEYCODE_D))
	{
		screen_update_layer(m_layer.get(), LAYER_H + LAYER_WORKAREA_H, BITPLANE_W, LAYER_SPRITES_H, screen, bitmap, cliprect);
		return 0;
	}
	else if (machine().input().code_pressed(KEYCODE_F))
	{
		screen_update_layer(m_layer_overlay_sprites.get(), 0, BITPLANE_W, LAYER_H, screen, bitmap, cliprect);
		return 0;
	}
	else if (machine().input().code_pressed(KEYCODE_G))
	{
		screen_update_layer(m_layer_overlay_sprites_priority.get(), 0, BITPLANE_W, LAYER_H, screen, bitmap, cliprect);
		return 0;
	}
#endif

	screen_update_layer(m_layer.get(), 0, BITPLANE_W, LAYER_H, screen, bitmap, cliprect);

	if (m_is_overlay_enabled)
	{
		screen_update_layer(m_layer_overlay_sprites.get(), 0, BITPLANE_W, LAYER_H, screen, bitmap, cliprect);
		screen_update_layer(m_layer_overlay_sprites_priority.get(), 0, BITPLANE_W, LAYER_H, screen, bitmap, cliprect);
	}

	return 0;
}

void tomy_princ_state::screen_update_layer(u8 *layer, u16 layer_offset, u16 layer_w, u16 layer_h, screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
#ifdef VIDEO_DEBUG
	u32 offset = layer_w * layer_offset;
	for (size_t y = 0; y < layer_h; y++)
	{
		for (size_t x = 0; x < layer_w; x++)
		{
			u8 color = layer[offset];
			if (color != 0)
			{
				bitmap.pix(y, x) = m_palette->pen_color(color);
			}
			offset++;
		}
	}
#else
	for (size_t y = 0; y < SCREEN_H; y++)
	{
		u32 offset_y = ((m_vdp.screen_offset_y & 0x0fff) + y) * layer_w;
		for (size_t x = 0; x < SCREEN_W; x++)
		{
			u32 offset_x = m_vdp.screen_offset_x + x;
			u8 color = layer[offset_y + offset_x];
			if (color != 0)
			{
				bitmap.pix(y, x) = m_palette->pen_color(color);
			}
		}
	}
#endif
}

DEVICE_IMAGE_LOAD_MEMBER(tomy_princ_state::cart_load)
{
	u64 length;
	memory_region *cart_rom = nullptr;
	if (m_cart->loaded_through_softlist())
	{
		cart_rom = m_cart->memregion("rom");
		if (!cart_rom)
		{
			return std::make_pair(image_error::BADSOFTWARE, "Software list item has no 'rom' data area");
		}
		length = cart_rom->bytes();
	}
	else
	{
		length = m_cart->length();
	}

	if (!length)
	{
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridges must not be empty");
	}
	if (length & 1)
	{
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be a multiple of 2 bytes)");
	}

	if (!m_cart->loaded_through_softlist())
	{
		cart_rom = machine().memory().region_alloc(m_cart->subtag("rom"), length, 2, ENDIANNESS_LITTLE);
		if (!cart_rom)
		{
			return std::make_pair(std::errc::not_enough_memory, std::string());
		}

		u16 *const base = reinterpret_cast<u16 *>(cart_rom->base());
		if (m_cart->fread(base, length) != length)
		{
			return std::make_pair(std::errc::io_error, "Error reading cartridge file");
		}

		if (ENDIANNESS_NATIVE != ENDIANNESS_LITTLE)
		{
			for (u64 i = 0; (length / 2) > i; ++i)
				base[i] = swapendian_int16(base[i]);
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}

u8 tomy_princ_state::key_r()
{
	u8 pdr_bits = 0;
	for (u8 port = 0; port < 9; port++)
	{
		u32 port_bit = m_io_keys[port]->read();
		for (u8 bit = 0; bit < 8; bit++)
		{
			if (BIT(port_bit, bit) != 0)
			{
				pdr_bits |= key_to_pdr_bit(port, bit);
			}
		}
	}

	return pdr_bits ^ 0xff;
}

u8 tomy_princ_state::key_to_pdr_bit(u8 port, u8 bit)
{
	const u8 pdr6 = m_maincpu->read_port(mb9061x_device::PDR6);
	const u8 pdr8 = m_maincpu->read_port(mb9061x_device::PDR8);

	LOGMASKED(LOG_REG, "%s: pdr6=%02x pdr8=%02x\n", machine().describe_context(), pdr6 ^ 0xff, pdr8 ^ 0xff);

	u8 target_port = 0;
	for (size_t i = 0; i < 7; i++)
	{
		if (BIT(pdr6 ^ 0xff, i))
		{
			target_port = i;
			if (port == target_port)
			{
				LOGMASKED(LOG_REG, "%s: matched port=%02x bit=%02x\n", machine().describe_context(), port, bit);
				return 1 << bit;
			}
		}
	}
	for (size_t i = 5; i < 7; i++)
	{
		if (BIT(pdr8 ^ 0xff, i))
		{
			target_port = 7 + (i - 5);
			if (port == target_port)
			{
				LOGMASKED(LOG_REG, "%s: matched port=%02x bit=%02x\n", machine().describe_context(), port, bit);
				return 1 << bit;
			}
		}
	}

	return 0;
}

void tomy_princ_state::princ_map(address_map &map)
{
	map(0x68ff00, 0x68ff00).lw8([this] (u8 data) { LOGMASKED(LOG_REG, "%s: 68FF00=%02X\n", machine().describe_context(), data); m_is_ext_reg00_read = true; }, "ext_reg00");
	map(0x68ff0c, 0x68ff0c).lw8([this] (u8 data) { LOGMASKED(LOG_REG, "%s: 68FF0C=%02X\n", machine().describe_context(), data); m_ext_reg0C = data; }, "ext_reg0C");
	map(0x68ff40, 0x68ff40).rw(FUNC(tomy_princ_state::vdp_reg_data_r), FUNC(tomy_princ_state::vdp_reg_data_w));
	map(0x68ff41, 0x68ff41).w(FUNC(tomy_princ_state::vdp_reg_w));
	map(0x68ff42, 0x68ff42).w(FUNC(tomy_princ_state::vdp_pal_data_w));
	map(0x68ff43, 0x68ff43).rw(FUNC(tomy_princ_state::vdp_gfx_data_r), FUNC(tomy_princ_state::vdp_gfx_data_w));
	map(0x68ff44, 0x68ff44).r(FUNC(tomy_princ_state::vdp_stat_r));
	map(0x800000, 0x87ffff).lr8(NAME([]() { return 0xff; }));
	map(0xe00000, 0xe07fff).ram();  // stacks are placed here
	map(0xf00000, 0xffffff).rom().region("maincpu", 0x00000);
}

// FIXME: Identify unknown keys
static INPUT_PORTS_START( tomy_princ )
	PORT_START( "KEY1" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ho") PORT_CODE(KEYCODE_EQUALS) // ほ 82D9h
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Memory") PORT_CODE(KEYCODE_F8) // メモリー FF08h

	PORT_START( "KEY2" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Menu") PORT_CODE(KEYCODE_F7) // メニュー FF07h
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("He") PORT_CODE(KEYCODE_BACKSPACE) // へ 82D6h
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Print") PORT_CODE(KEYCODE_F9) // 印刷 FF09h

	PORT_START( "KEY3" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("FF17h") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Symbol") PORT_CODE(KEYCODE_F11) // 絵記号 FF0Bh

	PORT_START( "KEY4" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Font Color") PORT_CODE(KEYCODE_F10) // 字体色 FF0Ah
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("FF16h") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Complete") PORT_CODE(KEYCODE_F12) //  文完了 FF0Ch

	PORT_START( "KEY5" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Re") PORT_CODE(KEYCODE_BACKSLASH) // れ 82EAh
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("!") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Romaji") PORT_CODE(KEYCODE_HOME) // ローマ字 FF0Eh

	PORT_START( "KEY6" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_INSERT) // ー字消す FF0Dh
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Zoom") PORT_CODE(KEYCODE_PGUP) // ズーム FF0Fh

	PORT_START( "KEY7" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) // FF13h
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ne") PORT_CODE(KEYCODE_RCONTROL) // ね 82CBh
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("NO") PORT_CODE(KEYCODE_RSHIFT) // FF10h
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) // FF12h

	PORT_START( "KEY8" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) // FF14h
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ru") PORT_CODE(KEYCODE_QUOTE) // る 82E9h
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) // FF15h

	PORT_START( "KEY9" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Extra Kanji") PORT_CODE(KEYCODE_F6) // 別漢字 FF06h
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("No conversion") PORT_CODE(KEYCODE_F5) // 無変換 FF05h
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A/a Numeral") PORT_CODE(KEYCODE_F3) // A/a 数 FF03h
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_F1) // シフト FF01h
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Hira/Kata") PORT_CODE(KEYCODE_F2) // ひら/カタ FF02h
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kanji conversion") PORT_CODE(KEYCODE_F4) // 漢字変換 FF04h
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) // スペース 8140h
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("OK") PORT_CODE(KEYCODE_ENTER) // FF11h
INPUT_PORTS_END

void tomy_princ_state::tomy_princ(machine_config &config)
{
	MB90611A(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tomy_princ_state::princ_map);
	m_maincpu->read_pdr1().set(FUNC(tomy_princ_state::key_r));
	m_maincpu->read_pdr8().set(FUNC(tomy_princ_state::pdr8_r));
	m_maincpu->read_pdra().set(FUNC(tomy_princ_state::pdra_r));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(FUNC(tomy_princ_state::screen_update));
	m_screen->set_size(SCREEN_W, SCREEN_H);
	m_screen->set_visarea_full();

	PALETTE(config, m_palette, FUNC(tomy_princ_state::princ_palette), 16);

	TIMER(config, m_scantimer, 0);
	m_scantimer->configure_scanline(FUNC(tomy_princ_state::scan_interrupt), "screen", 0, 1);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "princ_cart");
	m_cart->set_endian(ENDIANNESS_LITTLE);
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(tomy_princ_state::cart_load));
	m_cart->set_must_be_loaded(false);

	SOFTWARE_LIST(config, "cart_list").set_original("princ");
}

class tomy_princ_tablet_state : public tomy_princ_state
{
public:
	tomy_princ_tablet_state(const machine_config &mconfig, device_type type, const char *tag)
		: tomy_princ_state(mconfig, type, tag)
		, m_io_pen_x(*this, "PENX")
		, m_io_pen_y(*this, "PENY")
		, m_config(*this, "CONFIG")
	{
#ifdef VIDEO_DEBUG
		SCREEN_W = BITPLANE_W;
		SCREEN_H = LAYER_H;
#else
		SCREEN_W = 324; // 340 - 0x10
		SCREEN_H = 206; // 222 - 0x10
#endif
		SRC_POS = 0x60;
		DST_POS = 0x64;
		SHAPE_SIZE = 0x68;
		SHAPE_DIRECTION = 0x6c;
		SHAPE_MODE = 0x6d;
		SHAPE_PAL = 0x70;
		RENDER_CMD = 0x71;
		RENDER2_CMD = 0xf1;
		LAYER_MODE = 0x42;
		LAYER2_MODE = 0xc2;
		LAYER_OFFSET_Y = 0x43;
		LAYER_OFFSET_X = 0x44;
		LAYER_CLIP_H = 0x45;
		LAYER_CLIP_W = 0x46;
		SCREEN_OFFSET = 0x49;
		PAL_DATA = 0x51;
		SPRITE1_DST_POS_Y = 0x52;
		SPRITE1_DST_POS_X = 0x53;
		SPRITE1_SELECT = 0x54;
		SPRITE2_DST_POS_Y = 0x55;
		SPRITE2_DST_POS_X = 0x56;
		SPRITE2_SELECT = 0x57;
	}

	void tomy_princ_tablet(machine_config &config);

protected:
	virtual u8 vdp_stat_r() override;
	virtual u8 key_to_pdr_bit(u8 port, u8 bit) override;

private:
	u16 adcr_r();

	static constexpr s32 rescale(s32 x, s32 min_x, s32 max_x, s32 a, s32 b)
	{
		// Rescaling (min-max normalization) from [min_x..max_x] to [a..b].
		return a + (((x - min_x) * (b - a)) / (max_x - min_x));
	}

	required_ioport m_io_pen_x;
	required_ioport m_io_pen_y;
	required_ioport m_config;
};

void tomy_princ_tablet_state::tomy_princ_tablet(machine_config &config)
{
	tomy_princ_state::tomy_princ(config);

	m_maincpu->read_adcr().set(FUNC(tomy_princ_tablet_state::adcr_r));
}

u8 tomy_princ_tablet_state::vdp_stat_r()
{
	u8 stat = m_screen->vblank() ? 0x11 : 0x10;

	if (m_is_bound_found)
	{
		stat |= 0x20;
	}

	const bool is_buffer_render_cmd = m_vdp.regs[RENDER_CMD] == BUFFER_READ
			|| m_vdp.regs[RENDER2_CMD] == BUFFER_READ
			|| m_vdp.regs[RENDER_CMD] == BUFFER_WRITE
			|| m_vdp.regs[RENDER2_CMD] == BUFFER_WRITE;
	if (is_buffer_render_cmd)
	{
		return m_vdp.pending_pixels > 0 ? stat | 0x8 : stat & 1;
	}

	return stat;
}

u8 tomy_princ_tablet_state::key_to_pdr_bit(u8 port, u8 bit)
{
	const u8 pdr6 = m_maincpu->read_port(mb9061x_device::PDR6);
	const u8 pdr9 = m_maincpu->read_port(mb9061x_device::PDR9);

	LOGMASKED(LOG_REG, "%s: pdr6=%02x pdr9=%02x\n", machine().describe_context(), pdr6 ^ 0xff, pdr9 ^ 0xff);

	u8 target_port = 0;
	for (size_t i = 0; i < 5; i++)
	{
		if (BIT(pdr6 ^ 0xff, i))
		{
			target_port = i;
			if (port == target_port)
			{
				LOGMASKED(LOG_REG, "%s: matched port=%02x bit=%02x\n", machine().describe_context(), port, bit);
				return 1 << bit;
			}
		}
	}
	if (pdr9 != 0)
	{
		for (size_t i = 2; i < 5; i++)
		{
			if (BIT(pdr9 ^ 0xff, i))
			{
				target_port = 5 + (i - 2);
				if (port == target_port)
				{
					LOGMASKED(LOG_REG, "%s: matched port=%02x bit=%02x\n", machine().describe_context(), port, bit);
					return 1 << bit;
				}
			}
		}
	}

	return 0;
}

u16 tomy_princ_tablet_state::adcr_r()
{
	u8 pdra = m_maincpu->read_port(mb9061x_device::PDRA);
	u16 adcs = m_maincpu->read_reg_adcs();

	// Is pen enabled?
	if (BIT(m_config->read(), 0))
	{
		if (BIT(pdra, 3) && (adcs & 0b101101) == 0b101101) // PDRA = u8 & 0x80 | 0x48; ADCS = 5 | (5 << 3);
		{
			// Pen y-position
			const s16 io_pen_y_min = m_io_pen_y->field(0xff)->minval();
			const s16 io_pen_y_max = m_io_pen_y->field(0xff)->maxval();
			const s16 io_pen_y = m_io_pen_y->read();
			const u16 pen_y = rescale(io_pen_y, io_pen_y_min, io_pen_y_max, 0x078, 0x3b0);
			LOGMASKED(LOG_REG, "%s: adcr_r pen y=%04x, PDRA=%02x, ADCS=%04x\n", machine().describe_context(), pen_y, pdra, adcs);
			return pen_y;
		}
		else if (BIT(pdra, 4) && (adcs & 0b110110) == 0b110110) // PDRA = u8 & 0x80 | 0x30; ADCS = 6 | (6 << 3);
		{
			// Pen x-position
			const s16 io_pen_x_min = m_io_pen_x->field(0xff)->minval();
			const s16 io_pen_x_max = m_io_pen_x->field(0xff)->maxval();
			const s16 io_pen_x = m_io_pen_x->read();
			const u16 pen_x = rescale(io_pen_x, io_pen_x_min, io_pen_x_max, 0x068, 0x3c0);
			LOGMASKED(LOG_REG, "%s: adcr_r pen x=%04x, PDRA=%02x, ADCS=%04x\n", machine().describe_context(), pen_x, pdra, adcs);
			return pen_x;
		}
		else if (BIT(pdra, 5)) // PDRA = u8 & 0x80 | 0x20;
		{
			// Program code checks for value >= 600 before reading pen position,
			// likely some time-based counter of pen touching tablet. We just send max value.
			const u16 pen_stat = 0x3ff;
			LOGMASKED(LOG_REG, "%s: adcr_r pen stat=%04x, PDRA=%02x, ADCS=%04x\n", machine().describe_context(), pen_stat, pdra, adcs);
			return pen_stat;
		}

		LOGMASKED(LOG_REG, "%s: adcr_r unknown state, PDRA=%02x, ADCS=%04x\n", machine().describe_context(), pdra, adcs);
	}

	return 0;
}

// FIXME: Identify unknown keys
static INPUT_PORTS_START( tomy_princ_tablet )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "Controller" )
	PORT_CONFSETTING( 0x00, DEF_STR( None ) )
	PORT_CONFSETTING( 0x01, "Pen" )

	PORT_START( "PENX" )
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("PEN X")

	PORT_START( "PENY" )
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("PEN Y")

	PORT_START( "KEY1" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_F1) // シフト FF01h
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ho") PORT_CODE(KEYCODE_EQUALS) // ほ 82D9h

	PORT_START( "KEY2" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Hira/Kata") PORT_CODE(KEYCODE_F2) // ひら/カタ FF02h
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("FF07h") PORT_CODE(KEYCODE_F7) // F378h

	PORT_START( "KEY3" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A/a Numeral") PORT_CODE(KEYCODE_F3) // A/a 数 FF03h
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_SLASH)

	PORT_START( "KEY4" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kanji conversion") PORT_CODE(KEYCODE_F4) // 漢字変換 FF04h
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ru") PORT_CODE(KEYCODE_QUOTE) // る 82E9h
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)

	PORT_START( "KEY5" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ne") PORT_CODE(KEYCODE_RCONTROL) // ね 82CBh
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("No conversion") PORT_CODE(KEYCODE_F5) // 無変換 FF05h
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("FF0Bh") PORT_CODE(KEYCODE_F11)

	PORT_START( "KEY6" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Re") PORT_CODE(KEYCODE_BACKSLASH) // れ 82EAh
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) // スペース 8140h
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("!") PORT_CODE(KEYCODE_COLON)

	PORT_START( "KEY7" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("FF0Ah") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Extra Kanji") PORT_CODE(KEYCODE_F6) // 別漢字 FF06h
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("FF08h") PORT_CODE(KEYCODE_F8) // F379h

	PORT_START( "KEY8" )
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("FF09h") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ok (T)") PORT_CODE(KEYCODE_ENTER) // FF0Ch
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("He") PORT_CODE(KEYCODE_BACKSPACE) // へ 82D6h

	PORT_START( "KEY9" )
	PORT_BIT(0xff,IP_ACTIVE_HIGH,IPT_UNUSED)
INPUT_PORTS_END

// PCB markings: Prin-C Tablet E100-T201-11
ROM_START( princ )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("29f800t.u4", 0x00000, 0x100000, CRC(30b6b864) SHA1(7ada3af85dd8dd3f95ca8965ad8e642c26445293))
ROM_END

// PCB markings: Prin-C E100-T001-11
ROM_START( princnt )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("tc538000.u3", 0x00000, 0x100000, CRC(e4e2bfe9) SHA1(b3a7727544918b9030c362694ddf9a2fc3bca8b4))
ROM_END
ROM_START( princnt3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("tc538000.3.u3", 0x00000, 0x100000, CRC(eed0a0f9) SHA1(c15b97c2c29b354136cef046f72a6443b415e673))
ROM_END

} // anonymous namespace

// Back case and ICs dated 1996
COMP( 1996?, princnt,   princ,   0, tomy_princ,        tomy_princ,        tomy_princ_state,        empty_init, "Tomy", "Prin-C (Rev 1)", MACHINE_NO_SOUND | MACHINE_NODEVICE_PRINTER )
COMP( 1996?, princnt3,  princ,   0, tomy_princ,        tomy_princ,        tomy_princ_state,        empty_init, "Tomy", "Prin-C (Rev 3)", MACHINE_NO_SOUND | MACHINE_NODEVICE_PRINTER )

// ICs dated 1997; promo from 1998: https://web.archive.org/web/19990302035001/http://www.tomy.co.jp/toyking/osirase/PrinC/
COMP( 1997?, princ,     0,       0, tomy_princ_tablet, tomy_princ_tablet, tomy_princ_tablet_state, empty_init, "Tomy", "Prin-C Tablet",  MACHINE_NO_SOUND | MACHINE_NODEVICE_PRINTER )
