// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/*****************************************************************************

    Casio Loopy (c) 1995 Casio

==============================================================================

Casio Loopy PCB Layout
----------------------

JCM631-MA1M C
|---------------------------------------------------------|
|    CB    CC              CD         CE       CF      CG |
|--|                                                      |
   |                                        BA10339F      |
|--| 15218  |--|     CXA1645M                           CH|
|           |  |                A1603C                    |
|    15218  |  |                                          |
|           |  |                                          |
|BIOS.LSI352|  |                                          |
|           |  |                      21MHz               |
| |--------||  |   |------|                 SW1           |
| |NEC     ||  |   |SH7021|      |----------|             |
| |CDT109  ||CA|   |      |      |          |             |
| |        ||  |   |------|      |CASIO     |             |
| |--------||  |                 |RH-7500   |             |
|           |  |                 |5C315     |          |--|
| |-------| |  |                 |          |          |
| |CASIO  | |  |                 |----------|          |--|
| |RH-7501| |  |  HM514260                                |
| |5C350  | |  |                               HM62256    |
| |-------| |  |                                          |
| 6379      |--|    SW301                      HM62256    /
|--------|                        HM538123               /
         |                                              /
         |                                             /
         |--------------------------------------------/

Notes:
      Connectors
      ----------
      CA - Cartridge connector
      CB - Power Input connector
      CC - Composite Video and Audio Out connector
      CD - Printer Cassette Motor connector
      CE - Printer Data connector
      CF - Printer Head connector
      CG - Paper Sensor connector
      CH - Joystick connector
      Connectors on the back of the main unit include RCA audio (left/right), RCA composite video,
      24V DC power input and contrast knob.
      On top of the main unit, there is a reset button, on/off slide switch, a big eject button, a
      button to cut off stickers after they're printed, a button to open the hatch where the sticker
      cassette is located and a red LED for power.

      IC's
      ----
      LSI352      - Hitachi HN62434 512k x8 (4MBit) mask ROM (SOP40)
      CDT-109     - NEC CDT109 (QFP120). This is the sound chip.
      RH-7500     - Casio RH-7500 5C315 (QFP208). This is the graphics generator chip.
      RH-7501     - Casio RH-7501 5C350 (QFP64). This is a support chip for the sound chip.
      SH7021      - Hitachi HD6437021TE20 SuperH RISC Engine SH-1 CPU with 32k internal mask ROM (TQFP100)
      CXA1645M    - Sony CXA1645M RGB Encoder (RGB -> Composite Video) (SOIC24)
      A1603C      - NEC uPA1603C Compound Field Effect Power Transistor Array (DIP16)
      HM514260    - Hitachi HM514260 256k x 16 DRAM (SOJ40)
      HM538123    - Hitachi HM538123 128k x8 multi-port Video DRAM with 256-word x8 serial access memory (SOJ40)
      HM62256     - Hitachi HM62256 32k x8 SRAM (SOP28)
      BA10339F    - Rohm BA10339F Quad Comparitor (SOIC14)
      6379        - NEC uPD6379 2-channel 16-bit D/A convertor for digital audio signal demodulation (SOIC8)
      15218       - Rohm BA15218 Dual High Slew Rate, Low Noise Operational Amplifier (SOIC8)

      Other
      -----
      SW1        - Reset Switch
      SW301      - ON/OFF Slide Switch


Inside the carts
----------------

Carts 401 - 404:
PCB 'JCM632-AN1M C'
1x 16M mask ROM (SOP44)
1x 8k x8 SRAM (SOP28)
1x 3V coin battery (CR2032)

Cart 501:
PCB 'Z544-1 A240427-1'
1x 16M mask ROM (SOP44)
1x 8k x8 SRAM (SOP28)
1x OKI MSM6653A Voice Synthesis IC with 544Kbits internal maskROM (SOP24)
1x Rohm BA15218 High Slew Rate, Low Noise, Dual Operational Amplifier (SOIC8)
1x 74HC273 logic chip
1x 3V coin battery (CR2032)

Cart 502:
PCB 'Z545-1 A240570-1'
1x 16M mask ROM (SOP44)
1x 32k x8 SRAM (SOP28)
1x 74HC00 logic chip
1x 3V coin battery (CR2032)



 TODO:
- Fix 8-bit readback mode for color blended pixels (this seems broken in HW)
- Clean up and condense video layer priority logic
- Factor out joypad and mouse into slot devices
- Sound
- ADPCM sound for wanwanam
- Printer

 Issues:
- vswordp: Green pixels show up in place of the demo images

******************************************************************************/

#include "emu.h"

#include "cpu/sh/sh7021.h"
#include "bus/casloopy/rom.h"
#include "bus/casloopy/slot.h"
#include "machine/timer.h"

#include "emuopts.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

/*************************************
 *
 *  Definitions
 *
 *************************************/

class casloopy_state : public driver_device
{
public:
	casloopy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_io0(*this, "CONTROLLER_0")
		, m_io1(*this, "CONTROLLER_1")
		, m_io2(*this, "CONTROLLER_2")
		, m_mouse(*this, "MOUSE")
		, m_mouse_x(*this, "MOUSE_X")
		, m_mouse_y(*this, "MOUSE_Y")
		, m_config(*this, "CONFIG")
	{ }

	void casloopy(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	static constexpr XTAL SH1_CLOCK = XTAL(16'000'000);
	static constexpr XTAL VIDEO_CLOCK = XTAL(21'477'272);
	static constexpr int H_TOTAL = 341;
	static constexpr int V_TOTAL = 263;
	static constexpr int H_ACTIVE = 256;
	static constexpr int V_ACTIVE_0 = 224;
	static constexpr int V_ACTIVE_1 = 240;

	required_device<sh7021_device> m_maincpu;
	required_device<casloopy_cart_slot_device> m_cart;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_ioport m_io0;
	required_ioport m_io1;
	required_ioport m_io2;
	required_ioport m_mouse;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;
	required_ioport m_config;

	std::unique_ptr<u16[]> m_sprite_ram;
	std::unique_ptr<u16[]> m_palette_ram;
	std::unique_ptr<u16[]> m_vram;
	std::unique_ptr<u8[]> m_bitmap_vram;

	std::unique_ptr<u16[]> m_bmp_line[4];
	std::unique_ptr<u16[]> m_tile_line[2];
	std::unique_ptr<u16[]> m_spr_line[2];

	emu_timer * m_hblank_timer = nullptr;

	u8 m_bmp_latch[4];
	u16 m_readback_buffer[256];
	u16 m_readback_latch;
	s16 m_d_mouse_x;
	s16 m_d_mouse_y;

	u16 m_system_control;
	u16 m_bmp_vram_x[4];
	u16 m_bmp_vram_y[4];
	u16 m_bmp_screen_x[4];
	u16 m_bmp_screen_y[4];
	u16 m_bmp_size_x[4];
	u16 m_bmp_size_y[4];
	u16 m_bmp_color;
	u16 m_bmp_control;
	u16 m_bmp_span[4];
	u16 m_screen_width;
	u16 m_screen_height;
	u16 m_int_control;
	u16 m_tilemap_color[2];
	u16 m_tilemap_control;
	u16 m_tilemap_page;
	u16 m_tilemap_scroll_x[2];
	u16 m_tilemap_scroll_y[2];
	u16 m_layer_control_0;
	u16 m_layer_enable;
	u16 m_layer_control_2;
	u16 m_sprite_control;
	u16 m_sprite_color[2];
	u16 m_color[2];
	u16 m_readback_control;
	u16 m_clear_mask;
	u16 m_clear_color;
	u16 m_5d020;
	u16 m_5d030;
	u16 m_5d040;
	u16 m_5d042;
	u16 m_5d044;
	u16 m_seal_sw;

	// Byte offsets
	static constexpr offs_t s_gfxdata_offsets[] =
	{
		0x4000,
		0x2000,
		0x2000,
		0x1000,
		0x2000,
		0x1000,
		0x1000,
		0x800
	};

	enum Layer : u8
	{
		B0,
		B1,
		B2,
		B3,
		X0,
		X1,
		Y0,
		Y1,
		S0,
		S1,
		NG,
	};

	static const Layer s_m4_pri_2[256][16][10];
	static const Layer s_m4_pri_4[256][16][10];
	static const Layer s_m4_pri_6[256][16][10];
	static const Layer s_m5_pri_6[256][16][10];
	static const Layer s_m0_pri_6[256][16][10];

	int bitmap_bpp() const;
	offs_t tilemap_offset(int which) const;
	offs_t tiledata_offset() const;
	offs_t spritedata_offset() const;
	u8 readback_mode() const;
	Layer priority(int mode1, int mode2, int mode3, int mode4, int i) const;
	u16 background_color() const;
	u16 border_color() const;

	TIMER_CALLBACK_MEMBER(hblank_start);
	void update_scanline_buffers(int y);
	void draw_bitmap_line(int which, int y);
	void draw_tilemap_line(int which, int y);
	void draw_sprite_line(int y);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void casloopy_map(address_map &map) ATTR_COLD;

	u8 bitmap_r(offs_t offset);
	void bitmap_w(offs_t offset, u8 data);
	u16 vram_r(offs_t offset);
	void vram_w(offs_t offset, u16 data);
	u16 sprite_ram_r(offs_t offset);
	void sprite_ram_w(offs_t offset, u16 data);
	u16 palette_r(offs_t offset);
	u16 readback_r(offs_t offset);
	void palette_w(offs_t offset, u16 data);
	u16 system_control_r();
	void system_control_w(u16 data);
	u16 hpos_r();
	void hpos_w(u16 data);
	u16 vpos_r();
	void vpos_w(u16 data);
	u16 reg_58006_r();
	void reg_58006_w(u16 data);
	void reg_58008_w(u16 data);
	u16 reg_5d000_r();
	void reg_5d000_w(u16 data);
	u16 controller_0_r();
	u16 controller_1_r();
	u16 controller_2_r();
	u16 reg_5d020_r();
	void reg_5d020_w(u16 data);
	u16 reg_5d030_r();
	void reg_5d030_w(u16 data);
	u16 reg_5d040_r();
	void reg_5d040_w(u16 data);
	u16 reg_5d042_r();
	void reg_5d042_w(u16 data);
	u16 reg_5d044_r();
	void reg_5d044_w(u16 data);
	u16 reg_5d054_r();
	void reg_5d054_w(u16 data);
	u16 reg_5e000_r();
	void reg_5e000_w(u16 data);
	void reg_60000_w(u16 data);
	void reg_a0000_w(u16 data);
	u16 default_mouse_state();
	u16 mouse1_r();
	u16 mouse2_r();
	void fast_clear_w(offs_t offset, u16 data);
	void sound_control_w(u16 data);
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

void casloopy_state::video_start()
{
	m_palette_ram = make_unique_clear<u16[]>(0x1000);
	m_vram = make_unique_clear<u16[]>(0x8000);
	m_bitmap_vram = make_unique_clear<u8[]>(0x20000);
	m_sprite_ram = make_unique_clear<u16[]>(0x100);

	m_bmp_line[0] = std::make_unique<u16[]>(256);
	m_bmp_line[1] = std::make_unique<u16[]>(256);
	m_bmp_line[2] = std::make_unique<u16[]>(256);
	m_bmp_line[3] = std::make_unique<u16[]>(256);
	m_spr_line[0] = std::make_unique<u16[]>(256);
	m_spr_line[1] = std::make_unique<u16[]>(256);
	m_tile_line[0] = std::make_unique<u16[]>(256);
	m_tile_line[1] = std::make_unique<u16[]>(256);

	m_hblank_timer = timer_alloc(FUNC(casloopy_state::hblank_start), this);
	m_hblank_timer->adjust(m_screen->time_until_pos(0, 256));

	save_pointer(NAME(m_palette_ram), 0x1000);
	save_pointer(NAME(m_vram), 0x8000);
	save_pointer(NAME(m_bitmap_vram), 0x20000);
	save_pointer(NAME(m_sprite_ram), 0x100);
}

int casloopy_state::bitmap_bpp() const
{
	static constexpr int bpp_table[] = { 8, 8, 4, 4, 4, 8, 8, 8 };
	return bpp_table[m_bmp_control];
}

offs_t casloopy_state::tilemap_offset(int which) const
{
	const u32 size = BIT(m_tilemap_control, 0, 3);

	if ((which == 0) || (size & 1))
		return 0;

	return s_gfxdata_offsets[size] / 2;
}

offs_t casloopy_state::tiledata_offset() const
{
	if (!BIT(m_tilemap_control, 3))
	{
		// 4bpp
		u32 v = m_tilemap_page & 0x7f;
		const offs_t page = v * 0x200;
		return (s_gfxdata_offsets[BIT(m_tilemap_control, 0, 3)] + page) / 2;
	}
	else
	{
		// 8bpp
		return s_gfxdata_offsets[BIT(m_tilemap_control, 0, 3)] / 2;
	}
}

offs_t casloopy_state::spritedata_offset() const
{
	return s_gfxdata_offsets[BIT(m_tilemap_control, 0, 3)] / 2;
}

u8 casloopy_state::readback_mode() const
{
	switch (BIT(m_readback_control, 8, 2))
	{
		case 0:
		case 1:
			return 1;
		case 2:
		case 3:
		default:
			return 0;
	}
}

#include "casloopy_tbl.ipp"

casloopy_state::Layer casloopy_state::priority(int mode1, int mode2, int mode3, int mode4, int i) const
{
	static constexpr u8 s_layers_enabled[8][16] =
	{
		{ 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 },
		{ 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 },
		{ 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1 },
		{ 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0 },
		{ 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	};

	Layer result = NG;

	switch (mode2)
	{
		case 0:
		case 1:
		case 8:
		case 9:
		case 0xb:
			result = NG;
			break;

		case 2:
		case 0xa:
			result = s_m4_pri_2[mode3][mode4][i];
			break;

		case 4:
		case 5:
		case 7:
		case 0xc:
		case 0xd:
		case 0xf:
			result = s_m4_pri_4[mode3][mode4][i];
			break;

		case 6:
		case 0xe:
			switch (mode1)
			{
				case 0:
				case 1:
				case 3:
					result = s_m0_pri_6[mode3][mode4][i];
					break;

				case 2:
					result = s_m4_pri_4[mode3][mode4][i];
					break;

				case 4:
					result = s_m4_pri_6[mode3][mode4][i];
					break;

				case 5:
					result = s_m5_pri_6[mode3][mode4][i];
					break;

				default:
					result = NG;
					break;

			}
			break;
	}

	return s_layers_enabled[mode1][mode2] ? result : NG;
}

u16 casloopy_state::background_color() const
{
	const int bmode = BIT(m_layer_control_0, 0, 3);

	if (bmode == 6 || bmode == 7)
		return 0;

	const int mode = BIT(m_layer_control_2, 4, 4);

	switch (bmode)
	{
		case 0:
		case 1:
		{
			switch (mode)
			{
				case 2:
				case 3:
					return m_color[0];

				case 4:
				case 5:
				case 6:
					return m_color[1];

				case 7:
					return m_color[0];

				case 12:
				case 13:
				case 14:
				case 15:
					return m_color[1];

				default:
					return 0;
			}
		}
		case 2:
		{
			switch (mode)
			{
				case 4:
				case 5:
				case 6:
				case 7:
					return m_color[1];

				case 12:
				case 13:
				case 14:
				case 15:
					return m_color[1];

				default:
					return 0;
			}
		}
		case 3:
		{
			switch (mode)
			{
				case 2:
				case 3:
					return m_color[0];

				case 4:
				case 5:
				case 6:
				case 7:
					return m_color[1];

				case 10:
				case 11:
					return m_color[0];

				case 12:
				case 13:
				case 14:
				case 15:
					return m_color[1];

				default:
					return 0;
			}
		}
		case 4:
		{
			switch (mode)
			{
				case 3:
					return m_color[0];

				case 4:
				case 5:
				case 6:
					return m_color[1];

				case 7:
					return m_color[0];

				case 11:
					return m_color[0];

				case 12:
				case 13:
				case 14:
					return m_color[1];

				case 15:
					return m_color[0];

				default:
					return 0;
			}
		}
		case 5:
		{
			switch (mode)
			{
				case 2:
				case 3:
					return m_color[0];

				case 6:
				case 7:
					return m_color[0];

				case 10:
				case 11:
					return m_color[0];

				case 14:
				case 15:
					return m_color[0];

				default:
					return 0;
			}
		}
	}
	return 0;
}

#ifdef UNUSED_FUNCTION
u16 casloopy_state::border_color() const
{
	const int bmode = BIT(m_layer_control_0, 0, 3);

	if (bmode == 6 || bmode == 7)
		return 0;

	const int mode = BIT(m_layer_control_2, 4, 4);

	switch (mode)
	{
		case 4:
		case 5:
		case 6:
		case 7:
			return m_color[1];

		case 12:
		case 13:
		case 14:
		case 15:
			return m_color[1];

		default:
			return 0;
	}
}
#endif

void add_colors(u16 ca, u16 cb, int &r, int &g, int &b)
{
	int ar = (ca >> 10) & 0x1f;
	int ag = (ca >> 5) & 0x1f;
	int ab = (ca >> 0) & 0x1f;

	int br = (cb >> 10) & 0x1f;
	int bg = (cb >> 5) & 0x1f;
	int bb = (cb >> 0) & 0x1f;

	r = ar + br;
	g = ag + bg;
	b = ab + bb;

	// Clamping is done later
}

void sub_colors(u16 ca, u16 cb, int &r, int &g, int &b)
{
	int ar = (ca >> 10) & 0x1f;
	int ag = (ca >> 5) & 0x1f;
	int ab = (ca >> 0) & 0x1f;

	int br = (cb >> 10) & 0x1f;
	int bg = (cb >> 5) & 0x1f;
	int bb = (cb >> 0) & 0x1f;

	r = ar - br;
	g = ag - bg;
	b = ab - bb;

	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
}

void casloopy_state::draw_bitmap_line(int which, int y)
{
	const int sy_start = m_bmp_screen_y[which];
	const int y_size = m_bmp_size_y[which];
	int y_offs;

	if (sy_start & 0x100)
	{
		y_offs = (0x200 - sy_start) + y;
	}
	else
	{
		y_offs = y - sy_start;

		if (y < sy_start)
			return;
	}

	if (y_offs > y_size)
		return;

	const bool is_8bpp = bitmap_bpp() == 8;
	const int sx_start = m_bmp_screen_x[which];
	const int x_size = BIT(m_bmp_size_x[which], 0, 8);
	const int x_min = BIT(m_bmp_size_x[which], 8, 8);
	const int bx_start = m_bmp_vram_x[which] & (is_8bpp ? 0xff : 0x1ff);
	const int by_start = m_bmp_vram_y[which];
	const int pal_offs = !is_8bpp ? ((m_bmp_color >> (4 * (3 - which))) & 0xf) * 0x10 : 0;
	const bool x_wrap = m_bmp_control == 3;
	const bool y_wrap = (m_bmp_control == 0 || m_bmp_control == 2 || m_bmp_control == 5);
	const int x_mask = x_wrap ? 0xff : 0x1ff;
	const int y_mask = y_wrap ? 0xff : 0x1ff;
	const int x_offs = x_wrap ? bx_start & 0x100 : 0;
	const u8 * bmp_ram = y_wrap ? &m_bitmap_vram[(by_start & 0x100) * 256] : &m_bitmap_vram[0];
	const int by = by_start + y_offs;
	const int x_pixels = x_size + 1;

	int x = sx_start;
	int bx = bx_start;

	for (int xp = 0; xp < x_pixels; ++xp)
	{
		if (xp >= x_min && x < 256)
		{
			u8 pix;

			if (is_8bpp)
			{
				pix = bmp_ram[(by & y_mask) * 256 + (bx & 0xff)];
			}
			else
			{
				pix = bmp_ram[(by & y_mask) * 256 + (x_offs / 2) + (bx & x_mask) / 2];
				pix = bx & 1 ? pix & 0xf : (pix >> 4) & 0xf;
			}

			if (pix != 0)
				m_bmp_line[which][x] = pal_offs | pix;
		}
		bx++;
		x = (x + 1) & 0x1ff;
	}
}

void casloopy_state::draw_tilemap_line(int which, int y)
{
	const bool is_8bpp = BIT(m_tilemap_control, 3);
	const int size = BIT(m_tilemap_control, 1, 2);
	const int ts_shift = (m_tilemap_control >> (4 + 2 * (1 ^ which))) & 3;
	const int rows = size & 1 ? 32 : 64;
	const int cols = size & 2 ? 32 : 64;
	const int col_mask = cols - 1;
	const int row_mask = rows - 1;
	const int color_base = m_tilemap_color[which];
	const int vram_offs = tilemap_offset(which) / 2;
	const int num_tiles = 1 << ts_shift;
	const int tile_size = 8 << ts_shift;
	const int scroll_x = m_tilemap_scroll_x[which];
	const int scroll_y = m_tilemap_scroll_y[which];
	const offs_t td_offs = tiledata_offset();

	int cs = scroll_x / tile_size;
	int rs = (y + scroll_y) / tile_size;
	int x_offs2 = (scroll_x % tile_size) / 8;
	int x_offs = scroll_x & 7;
	int y_offs = (y + scroll_y) % tile_size;
	int sy = y_offs & 7;
	int ys = 8 * ((y_offs / 8) % tile_size);
	int c = cs;
	int x = 0;

	while (x < 256)
	{
		const u32 tile_index = (rs & row_mask) * cols + (c & col_mask);
		const u16 data = m_vram[vram_offs + tile_index];
		const u16 code = data & 0x7ff;
		const int pri = BIT(data, 11);
		const int fx = BIT(data, 14);
		const int fy = BIT(data, 15);
		const int fx_mask = (fx * (num_tiles - 1));
		const int fy_mask = (fy * ((num_tiles - 1) << 3));

		u32 color = 0;

		if (!is_8bpp)
		{
			u32 color_idx = BIT(data, 12, 2);
			color = (color_base >> (4 * color_idx)) & 0xf;
		}

		int ctmp = ((code & ~7) + (ys ^ fy_mask));

		for (int tx = x_offs2; tx < num_tiles; ++tx)
		{
			const int idx = ctmp | ((code + (tx ^ fx_mask)) & 7);

			if (is_8bpp)
			{
				const offs_t fy_offs = fy ? 28 : 0;
				const offs_t base_offs = td_offs + 32 * idx + fy_offs;
				const u16 * pix_data = &m_vram[(base_offs ^ (sy * 4)) & 0x7fff];
				for (int sx = x_offs; sx < 8; ++sx, ++x)
				{
					int v = (fx ? 8 : 0) ^ ((sx & 1) ? 0 : 8);
					u16 word = pix_data[(fx ? 3 : 0) ^ (sx >> 1)];
					u8 pix = (word >> v) & 0xff;

					if (x < 256 && pix)
						m_tile_line[which][x] = (pri ? 0x100 : 0) | pix;
				}
			}
			else
			{
				const offs_t fy_offs = fy ? 14 : 0;
				const offs_t base_offs = td_offs + 16 * idx + fy_offs;
				const u16 * pix_data = &m_vram[(base_offs ^ (sy * 2)) & 0x7fff];
				for (int sx = x_offs; sx < 8; ++sx, ++x)
				{
					int v = (fx ? 0 : 12) ^ ((sx & 3) << 2);
					u16 word = pix_data[(fx ? 1 : 0) ^ (sx >> 2)];
					u8 pix = (word >> v) & 0xf;

					if (x < 256 && pix)
						m_tile_line[which][x] = (pri ? 0x100 : 0) | (color << 4) | pix;
				}
			}
			x_offs = 0;
		}
		x_offs2 = 0;
		c++;
	}
}

void casloopy_state::draw_sprite_line(int y)
{
	const int split = m_sprite_control & 0x7f;
	const int invert_group = BIT(m_sprite_control, 7);
	const bool is_8bpp = BIT(m_sprite_control, 14);
	const int code0 = (m_sprite_control & 0x3800) >> 3;
	const int code1 = m_sprite_control & 0x0700;
	const u16 * vram = m_vram.get() + spritedata_offset();

	int tiles_left = 32;

	for (int i = 0; i < 128; ++i)
	{
		const u16 word0 = m_sprite_ram[i * 2 + 0];
		const u16 word1 = m_sprite_ram[i * 2 + 1];
		const int size = (word1 >> 10) & 0x3;
		int y_start = word0 & 0xff;

		if (BIT(word1, 9))
			y_start -= 256;

		static constexpr int x_sizes[] = { 1, 2, 2, 4 };
		static constexpr int y_sizes[] = { 1, 2, 4, 4 };

		const int y_offs = y - y_start;

		if (y < y_start || y_offs >= (8 * y_sizes[size]))
			continue;

		const int idx = invert_group ^ (i < split);
		const int code = (word0 >> 8) | (idx == 0 ? code0 : code1);
		const u32 color_idx = (word1 >> 12) & 0x3;
		const u32 color = (m_sprite_color[idx] >> (4 * color_idx)) & 0xf;
		const u32 color_offs = color << 4;
		const int fx = BIT(word1, 14);
		const int fy = BIT(word1, 15);
		const int x_tiles = x_sizes[size];
		const int y_tiles = y_sizes[size];
		const int sy = y_offs & 7;
		const int ys = 8 * ((y_offs / 8) % (y_tiles * 8));
		const int fx_mask = (fx * (x_tiles - 1));
		const int fy_mask = (fy * ((y_tiles - 1) << 3));

		u16 * line_buffer = &m_spr_line[idx][0];
		int x = word1 & 0x1ff;
		int ctmp = ((code & ~7) + (ys ^ fy_mask));

		for (int tx = 0; tx < x_tiles; ++tx)
		{
			const int idx = ctmp | ((code + (tx ^ fx_mask)) & 7);

			if (is_8bpp)
			{
				const offs_t fy_offs = fy ? 28 : 0;
				const offs_t base_offs = 32 * idx + fy_offs;
				const u16 * pix_data = &vram[(base_offs ^ (sy * 4)) & 0x7fff];

				for (int sx = 0; sx < 8; ++sx)
				{
					if (x < 256 && line_buffer[x] == 0)
					{
						int v = (fx ? 8 : 0) ^ ((sx & 1) ? 0 : 8);
						u16 word = pix_data[(fx ? 3 : 0) ^ (sx >> 1)];
						u8 pix = (word >> v) & 0xff;

						if (pix)
							line_buffer[x] = pix;
					}
					x = (x + 1) & 0x1ff;
				}
			}
			else
			{
				const offs_t fy_offs = fy ? 14 : 0;
				const offs_t base_offs = 16 * idx + fy_offs;
				const u16 * pix_data = &vram[(base_offs ^ (sy * 2)) & 0x7fff];

				for (int sx = 0; sx < 8; ++sx)
				{
					if (x < 256 && line_buffer[x] == 0)
					{
						int v = (fx ? 0 : 12) ^ ((sx & 3) << 2);
						u16 word = pix_data[(fx ? 1 : 0) ^ (sx >> 2)];
						u8 pix = (word >> v) & 0xf;

						if (pix)
							line_buffer[x] = color_offs | pix;
					}
					x = (x + 1) & 0x1ff;
				}
			}
			if (--tiles_left == 0)
				return;
		}
	}
}

void casloopy_state::update_scanline_buffers(int y)
{
	std::fill_n(m_bmp_line[0].get(), 256, 0);
	std::fill_n(m_bmp_line[1].get(), 256, 0);
	std::fill_n(m_bmp_line[2].get(), 256, 0);
	std::fill_n(m_bmp_line[3].get(), 256, 0);
	std::fill_n(m_tile_line[0].get(), 256, 0);
	std::fill_n(m_tile_line[1].get(), 256, 0);
	std::fill_n(m_spr_line[0].get(), 256, 0);
	std::fill_n(m_spr_line[1].get(), 256, 0);

	if (BIT(m_layer_enable, 0))
		draw_tilemap_line(0, y);

	if (BIT(m_layer_enable, 1))
		draw_tilemap_line(1, y);

	if (BIT(m_layer_enable, 2))
		draw_bitmap_line(0, y);

	if (BIT(m_layer_enable, 3))
		draw_bitmap_line(1, y);

	if (BIT(m_layer_enable, 4))
		draw_bitmap_line(2, y);

	if (BIT(m_layer_enable, 5))
		draw_bitmap_line(3, y);

	if (BIT(m_layer_enable, 6) || BIT(m_layer_enable, 7))
		draw_sprite_line(y);
}

u32 casloopy_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t * const palette = m_palette->palette()->entry_list_raw();
	const int wy = BIT(m_readback_control, 0, 8);
	const int bmode = BIT(m_layer_control_0, 0, 3);
	const int mode = BIT(m_layer_enable, 8, 8);
	const int mode2 = BIT(m_layer_control_2, 4, 4);
	const int pri_select = BIT(m_layer_control_2, 0, 4);
	const int rback_mode = readback_mode();
	const u16 bg = background_color();
	const bool bmp_8bpp = bitmap_bpp() == 8;
	const int bits = m_layer_enable;
	const int b10mode = BIT(bits, 8, 2);
	const int b32mode = BIT(bits, 10, 2);
	const int s0mode = BIT(bits, 12, 2);
	const int s1mode = BIT(bits, 14, 2);

	Layer pri_table[10];

	for (int i = 0; i < 10; ++i)
		pri_table[i] = priority(bmode, mode2, mode, pri_select, i);

	// Combine the layers
	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		const bool rback_en = (y == wy) && m_readback_latch;

		if (rback_en)
			m_readback_latch = 0;

		update_scanline_buffers(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
		{
			u16 pixels[11] = {};

			const u16 pix_x = m_tile_line[0][x];
			const u16 pix_y = m_tile_line[1][x];

			pixels[pix_x & 0x100 ? X1 : X0] = pix_x & 0xff;
			pixels[pix_y & 0x100 ? Y1 : Y0] = pix_y & 0xff;
			pixels[B0] = m_bmp_line[0][x];
			pixels[B1] = m_bmp_line[1][x];
			pixels[B2] = m_bmp_line[2][x];
			pixels[B3] = m_bmp_line[3][x];
			pixels[S0] = m_spr_line[0][x];
			pixels[S1] = m_spr_line[1][x];

			u8 l1 = NG;
			u8 l2 = NG;

			// Handle bitmap layer latching
			if (bmp_8bpp)
			{
				for (int i = 0; i < 4; ++i)
				{
					if (m_bmp_span[i] & 0x100)
					{
						if (pixels[B0 + i] < (m_bmp_span[i] & 0xff))
						{
							m_bmp_latch[i] = pixels[B0 + i];
						}
						else if (pixels[B0 + i] == 0xff)
						{
							pixels[B0 + i] = m_bmp_latch[i];
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < 4; ++i)
				{
					if (m_bmp_span[i] & 0x100)
					{
						if ((pixels[B0 + i] & 0xf) < (m_bmp_span[i] & 0xf))
						{
							m_bmp_latch[i] = pixels[B0 + i] & 0xf;
						}
						else if ((pixels[B0 + i] & 0xf) == 0xf)
						{
							if (m_bmp_latch[i] == 0)
							{
								pixels[B0 + i] = 0;
							}
							else
							{
								pixels[B0 + i] &= 0xf0;
								pixels[B0 + i] |= m_bmp_latch[i];
							}
						}
					}
				}
			}

			// Get two visible pixels
			for (int i = 0; i < 10; ++i)
			{
				u8 layer = pri_table[i];

				if (pixels[layer] != 0)
				{
					if (l1 == NG)
					{
						l1 = layer;
					}
					else if (l2 == NG)
					{
						if (l1 == B0 && layer == B1)
							continue;
						if (l1 == B2 && layer == B3)
							continue;

						l2 = layer;
						break;
					}
				}
			}

			// Replacement doesn't apply?
			u8 r8 = l1 != NG ? pixels[l1] : 0;

			int r, g, b;

			const int lmode = BIT(m_layer_control_2, 4, 4);

			bool half_bright = bmode == 1;

			int mode1 = 0;
			int mode2 = 0;

			switch (l1)
			{
				case B0:
				case B1:
					mode1 = b10mode;

					if (mode1 == 3 && (bmode == 1 || bmode == 3))
						half_bright = false;

					break;

				case B2:
				case B3:
					mode1 = b32mode;

					if (mode1 == 3 && (bmode == 1 || bmode == 3))
						half_bright = false;

					break;

				case S0:
					mode1 = s0mode;
					break;

				case S1:
					mode1 = s1mode;
					break;

				case X0:
				case Y0:
					mode1 = 2;
					break;

				case X1:
				case Y1:
					mode1 = 1;
					break;
			}

			if (bmode == 0 || bmode == 1 || bmode == 3)
			{
				switch (l2)
				{
					case B0:
					case B1:
						mode2 = b10mode;
						break;

					case B2:
					case B3:
						mode2 = b32mode;
						break;

					case S0:
						mode2 = s0mode;
						break;

					case S1:
						mode2 = s1mode;
						break;

					case X0:
					case Y0:
						mode2 = 2;
						break;

					case X1:
					case Y1:
						mode2 = 1;
						break;
				}

				switch (lmode)
				{
					case 6:
					{
						// Blend
						if (mode1 != 3
							&& mode1 != mode2
							&& l2 != NG)
						{
							u16 color1 = pixels[l1];
							u16 color2 = pixels[l2];

							color1 = palette[color1].as_rgb15();
							color2 = palette[color2].as_rgb15();

							add_colors(color1, color2, r, g, b);
						}
						// Add a constant
						else
						{
							u16 color1 = palette[pixels[l1]].as_rgb15();
							u16 color2 = 0;

							switch (l1)
							{
								case B0:
								case B1:
								case B2:
								case B3:
								{
									if (mode1 == 0)
									{
										throw false;
									}
									else if (mode1 == 1)
									{
										color2 = m_color[1];
									}
									else if (mode1 == 2)
									{
										color2 = m_color[0];
									}
									else if (mode1 == 3)
									{
										color2 = 0;
									}
									break;
								}
								case S0:
								case S1:
								{
									if (mode1 == 0)
									{
										throw false;
									}
									else if (mode1 == 1)
									{
										color2 = m_color[1];
									}
									else if (mode1 == 2)
									{
										color2 = m_color[0];
									}
									else if (mode1 == 3)
									{
										color2 = color1;
									}
									break;
								}
								case X0:
								case Y0:
									color2 = m_color[0];
									break;
								case X1:
								case Y1:
									color2 = m_color[1];
									break;
								default:
									color1 = m_color[0];
									color2 = m_color[1];
									break;
							}

							add_colors(color1, color2, r, g, b);
						}
						break;
					}

					case 7:
					{
						// Add a constant
						u16 color1 = palette[pixels[l1]].as_rgb15();
						u16 color2 = 0;

						switch (l1)
						{
							case B0:
							case B1:
							case B2:
							case B3:
							case S0:
							case S1:
							{
								if (mode1 == 0 || mode1 == 1)
								{
									throw false;
								}
								else if (mode1 == 2 || mode == 3)
								{
									color2 = m_color[0];
								}
								break;
							}
							case X0:
							case Y0:
								color2 = m_color[0];
								break;
							default:
								color1 = m_color[0];
								color2 = m_color[1];
								break;
						}

						add_colors(color1, color2, r, g, b);
						break;
					}

					case 0xe:
					{
						bool skip = mode1 == 3;

						// Blend
						if (!skip
							&& mode1 != mode2
							&& l2 != NG)
						{
							u16 color1 = pixels[l1];
							u16 color2 = pixels[l2];

							color1 = palette[color1].as_rgb15();
							color2 = palette[color2].as_rgb15();

							if (mode1 == 1)
								sub_colors(color2, color1, r, g, b);
							else
								sub_colors(color1, color2, r, g, b);
						}
						// Add a constant
						else
						{
							u16 color1 = palette[pixels[l1]].as_rgb15();
							u16 color2 = 0;

							switch (l1)
							{
								case B0:
								case B1:
								case B2:
								case B3:
								case S0:
								case S1:
								{
									if (mode1 == 0)
									{
										throw false;
									}
									else if (mode1 == 1)
									{
										color2 = color1;
										color1 = m_color[1];
									}
									else if (mode1 == 2)
									{
										color2 = m_color[0];
									}
									else if (mode1 == 3)
									{
										color1 = 0;
										color2 = 0;
									}
									break;
								}
								case X0:
								case Y0:
									color2 = m_color[0];
									break;
								case X1:
								case Y1:
									color2 = color1;
									color1 = m_color[1];
									break;
								default:
									color1 = m_color[1];
									color2 = m_color[0];
									break;
							}

							sub_colors(color1, color2, r, g, b);
						}
						break;
					}
					case 0xf:
					{
						u16 color1 = palette[pixels[l1]].as_rgb15();
						u16 color2 = 0;

						switch (l1)
						{
							case B0:
							case B1:
							case B2:
							case B3:
							case S0:
							case S1:
							{
								if (mode1 == 0 || mode1 == 1)
								{
									throw false;
								}
								else if (mode1 == 2 || mode1 == 3)
								{
									color2 = m_color[0];
								}
								break;
							}
							case X0:
							case Y0:
								color2 = m_color[0];
								break;
							case X1:
							case Y1:
								throw false;
							default:
								color1 = m_color[1];
								color2 = m_color[0];
								break;
						}

						sub_colors(color1, color2, r, g, b);
						break;
					}
					default:
					{
						if (l1 != NG)
						{
							u8 idx = pixels[l1];
							u16 color = palette[idx].as_rgb15();
							r = (color >> 10) & 0x1f;
							g = (color >> 5) & 0x1f;
							b = (color >> 0) & 0x1f;
						}
						else
						{
							r = (bg >> 10) & 0x1f;
							g = (bg >> 5) & 0x1f;
							b = (bg >> 0) & 0x1f;
						}
					}
				}
			}
			else
			{
				if (l1 != NG)
				{
					u8 idx = pixels[l1];
					u16 color = palette[idx].as_rgb15();
					r = (color >> 10) & 0x1f;
					g = (color >> 5) & 0x1f;
					b = (color >> 0) & 0x1f;
				}
				else
				{
					r = (bg >> 10) & 0x1f;
					g = (bg >> 5) & 0x1f;
					b = (bg >> 0) & 0x1f;
				}
			}

			if (half_bright)
			{
				r >>= 1;
				g >>= 1;
				b >>= 1;
			}

			if (r > 0x1f) r = 0x1f;
			if (g > 0x1f) g = 0x1f;
			if (b > 0x1f) b = 0x1f;

			bitmap.pix(y, x) = rgb_t(pal5bit(r), pal5bit(g), pal5bit(b));

			if (rback_en)
			{
				if (rback_mode == 0)
				{
					m_readback_buffer[x] = r8;
				}
				else if (rback_mode == 1)
				{
					m_readback_buffer[x] = (r << 10) | (g << 5) | b;
				}
			}
		}
	}

	return 0;
}

TIMER_CALLBACK_MEMBER(casloopy_state::hblank_start)
{
	int v = m_screen->vpos();

	m_screen->update_partial(v);

	if (BIT(m_int_control, 2))
	{
		const int nmi_line = BIT(m_system_control, 1) ? 239 : 223;

		if (v == nmi_line)
		{
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		}
	}

	// Adjust for next scanline
	if (++v >= 262)
		v = 0;

	m_hblank_timer->adjust(m_screen->time_until_pos(v, 256));
}


/*************************************
 *
 *  Handlers
 *
 *************************************/

u8 casloopy_state::bitmap_r(offs_t offset)
{
	// Only bitmap RAM supports 8-bit accesses
	return m_bitmap_vram[offset];
}

void casloopy_state::bitmap_w(offs_t offset, u8 data)
{
	m_bitmap_vram[offset] = data;
}

u16 casloopy_state::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void casloopy_state::vram_w(offs_t offset, u16 data)
{
	m_vram[offset] = data;
}

u16 casloopy_state::sprite_ram_r(offs_t offset)
{
	return m_sprite_ram[offset];
}

void casloopy_state::sprite_ram_w(offs_t offset, u16 data)
{
	m_sprite_ram[offset] = data;
}

u16 casloopy_state::palette_r(offs_t offset)
{
	return m_palette_ram[offset];
}

void casloopy_state::palette_w(offs_t offset, u16 data)
{
	m_palette_ram[offset] = data;

	int r = (m_palette_ram[offset] & 0x7c00) >> 10;
	int g = (m_palette_ram[offset] & 0x03e0) >> 5;
	int b = m_palette_ram[offset] & 0x001f;

	m_palette->set_pen_color(offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

u16 casloopy_state::readback_r(offs_t offset)
{
	if (readback_mode() == 0)
	{
		int x = offset * 2;
		return (m_readback_buffer[x] << 8) | (m_readback_buffer[x + 1]);
	}
	else
	{
		return m_readback_buffer[offset];
	}
}

u16 casloopy_state::system_control_r()
{
	return m_system_control;
}

void casloopy_state::system_control_w(u16 data)
{
	// 224/240 line mode
	if (BIT(m_system_control, 1) != BIT(data, 1))
	{
		const attoseconds_t refresh = HZ_TO_ATTOSECONDS(VIDEO_CLOCK / 4) * H_TOTAL * V_TOTAL;
		const int v_active = BIT(data, 1) ? V_ACTIVE_1 : V_ACTIVE_0;

		rectangle visarea(0, H_ACTIVE - 1, 0, v_active - 1);
		m_screen->configure(H_TOTAL, V_TOTAL, visarea, refresh);
	}

	m_system_control = data;
}

u16 casloopy_state::hpos_r()
{
	int hpos = m_screen->hpos();

	if (hpos >= 258)
		hpos += 170;

	return hpos;
}

u16 casloopy_state::vpos_r()
{
	u32 vpos = m_screen->vpos();

	if (m_screen->vblank())
		return 0x100 | ((vpos - 7) & 0xff);
	else
		return vpos & 0xff;
}

u16 casloopy_state::reg_58006_r()
{
	return 0;
}

void casloopy_state::reg_58006_w(u16 data)
{
	if (BIT(data, 0))
		m_readback_latch = 1;

	if (BIT(data, 2))
		m_seal_sw = BIT(m_config->read(), 0, 3);
}

void casloopy_state::reg_58008_w(u16 data)
{
}

u16 casloopy_state::default_mouse_state()
{
	u16 buttons = m_mouse->read();
	return 0x8080 | buttons | (buttons >> 8);
}

u16 casloopy_state::reg_5d000_r()
{
	return 4 << 6;
}

void casloopy_state::reg_5d000_w(u16 data)
{

}

u16 casloopy_state::controller_0_r()
{
	if (!BIT(m_config->read(), 5))
		return BIT(m_config->read(), 4) ? m_io0->read() | 1 : 0;
	else
		return default_mouse_state();
}

u16 casloopy_state::controller_1_r()
{
	if (!BIT(m_config->read(), 5))
		return BIT(m_config->read(), 4) ? m_io1->read() : 0;
	else
		return default_mouse_state();
}

u16 casloopy_state::controller_2_r()
{
	if (!BIT(m_config->read(), 5))
		return m_io2->read();
	else
		return default_mouse_state();
}

u16 casloopy_state::reg_5d020_r()
{
	return m_5d020;
}

void casloopy_state::reg_5d020_w(u16 data)
{
	m_5d020 = data;
}

u16 casloopy_state::reg_5d030_r()
{
	return (m_seal_sw << 4) | 7;
}

void casloopy_state::reg_5d030_w(u16 data)
{

}

u16 casloopy_state::reg_5d040_r()
{
	return m_5d040;
}
void casloopy_state::reg_5d040_w(u16 data)
{
	m_5d040 = data;
}

u16 casloopy_state::reg_5d042_r()
{
	return m_5d042;
}
void casloopy_state::reg_5d042_w(u16 data)
{
	m_5d042 = data;
}

u16 casloopy_state::reg_5d044_r()
{
	return m_5d044;
}
void casloopy_state::reg_5d044_w(u16 data)
{
	m_5d044 = data;
}

u16 casloopy_state::reg_5d054_r()
{
	return 0;
}

void casloopy_state::reg_5d054_w(u16 data)
{

}

u16 casloopy_state::reg_5e000_r()
{
	return 0;
}

void casloopy_state::reg_5e000_w(u16 data)
{

}

void casloopy_state::reg_60000_w(u16 data)
{

}
void casloopy_state::reg_a0000_w(u16 data)
{

}

u16 casloopy_state::mouse1_r()
{
	u16 val = 0;

	// Mouse present?
	if (BIT(m_config->read(), 5))
	{
		val = m_mouse->read();

		// Mouse X/Y reading enabled?
		if (BIT(m_system_control, 3))
		{
			int x = m_mouse_x->read();
			val |= (x - m_d_mouse_x) & 0xfff;

			if (!machine().side_effects_disabled())
				m_d_mouse_x = x;
		}
	}
	return val;
}

u16 casloopy_state::mouse2_r()
{
	u16 val = 0;

	// Mouse present and X/Y reading enabled?
	if (BIT(m_config->read(), 5) && BIT(m_system_control, 3))
	{
		int y = m_mouse_y->read();
		val = (y - m_d_mouse_y) & 0xfff;

		if (!machine().side_effects_disabled())
			m_d_mouse_y = y;
	}
	return val;
}

void casloopy_state::fast_clear_w(offs_t offset, u16 data)
{
	const int mask = m_clear_mask & 0xff;
	const int value = m_clear_color & mask;

	for (int x = 0; x < 256; ++x)
	{
		m_bitmap_vram[offset * 256 + x] &= ~mask;
		m_bitmap_vram[offset * 256 + x] |= value;
	}
}

void casloopy_state::sound_control_w(u16 data)
{

}


/*************************************
 *
 *  Memory Map
 *
 *************************************/

void casloopy_state::casloopy_map(address_map &map)
{
	// /CS0: Internal ROM

	// /CS1: Work DRAM
	map(0x01000000, 0x0107ffff).ram().mirror(0x08f80000);

	// /CS2: Cartridge SRAM
	map(0x02000000, 0x023fffff).rw(m_cart, FUNC(casloopy_cart_slot_device::ram_r), FUNC(casloopy_cart_slot_device::ram_w));

	// /CS4: RH-7500 VDP
	map(0x04000000, 0x0401ffff).rw(FUNC(casloopy_state::bitmap_r), FUNC(casloopy_state::bitmap_w)).mirror(0x00020000);
	map(0x04040000, 0x0404ffff).rw(FUNC(casloopy_state::vram_r), FUNC(casloopy_state::vram_w));
	map(0x04050000, 0x040501ff).rw(FUNC(casloopy_state::sprite_ram_r), FUNC(casloopy_state::sprite_ram_w));
	map(0x04050200, 0x040503ff).ram();
	map(0x04051000, 0x040511ff).rw(FUNC(casloopy_state::palette_r), FUNC(casloopy_state::palette_w));
	map(0x04052000, 0x040521ff).r(FUNC(casloopy_state::readback_r));
	map(0x04058000, 0x04058001).rw(FUNC(casloopy_state::system_control_r), FUNC(casloopy_state::system_control_w));
	map(0x04058002, 0x04058003).r(FUNC(casloopy_state::hpos_r));
	map(0x04058004, 0x04058005).r(FUNC(casloopy_state::vpos_r));
	map(0x04058006, 0x04058007).rw(FUNC(casloopy_state::reg_58006_r), FUNC(casloopy_state::reg_58006_w));
	map(0x04058008, 0x04058009).w(FUNC(casloopy_state::reg_58008_w));
	map(0x04059000, 0x0405a001).lrw16(NAME([this]() { return m_bmp_vram_x[0]; }), NAME([this](u16 data) { m_bmp_vram_x[0] = data & 0x1ff; }));
	map(0x04059002, 0x0405a003).lrw16(NAME([this]() { return m_bmp_vram_x[1]; }), NAME([this](u16 data) { m_bmp_vram_x[1] = data & 0x1ff; }));
	map(0x04059004, 0x0405a005).lrw16(NAME([this]() { return m_bmp_vram_x[2]; }), NAME([this](u16 data) { m_bmp_vram_x[2] = data & 0x1ff; }));
	map(0x04059006, 0x0405a007).lrw16(NAME([this]() { return m_bmp_vram_x[3]; }), NAME([this](u16 data) { m_bmp_vram_x[3] = data & 0x1ff; }));
	map(0x04059008, 0x0405a009).lrw16(NAME([this]() { return m_bmp_vram_y[0]; }), NAME([this](u16 data) { m_bmp_vram_y[0] = data & 0x1ff; }));
	map(0x0405900a, 0x0405a00b).lrw16(NAME([this]() { return m_bmp_vram_y[1]; }), NAME([this](u16 data) { m_bmp_vram_y[1] = data & 0x1ff; }));
	map(0x0405900c, 0x0405a00d).lrw16(NAME([this]() { return m_bmp_vram_y[2]; }), NAME([this](u16 data) { m_bmp_vram_y[2] = data & 0x1ff; }));
	map(0x0405900e, 0x0405a00f).lrw16(NAME([this]() { return m_bmp_vram_y[3]; }), NAME([this](u16 data) { m_bmp_vram_y[3] = data & 0x1ff; }));
	map(0x04059010, 0x0405a011).lrw16(NAME([this]() { return m_bmp_screen_x[0]; }), NAME([this](u16 data) { m_bmp_screen_x[0] = data & 0x1ff; }));
	map(0x04059012, 0x0405a013).lrw16(NAME([this]() { return m_bmp_screen_x[1]; }), NAME([this](u16 data) { m_bmp_screen_x[1] = data & 0x1ff; }));
	map(0x04059014, 0x0405a015).lrw16(NAME([this]() { return m_bmp_screen_x[2]; }), NAME([this](u16 data) { m_bmp_screen_x[2] = data & 0x1ff; }));
	map(0x04059016, 0x0405a017).lrw16(NAME([this]() { return m_bmp_screen_x[3]; }), NAME([this](u16 data) { m_bmp_screen_x[3] = data & 0x1ff; }));
	map(0x04059018, 0x0405a019).lrw16(NAME([this]() { return m_bmp_screen_y[0]; }), NAME([this](u16 data) { m_bmp_screen_y[0] = data & 0x1ff; }));
	map(0x0405901a, 0x0405a01b).lrw16(NAME([this]() { return m_bmp_screen_y[1]; }), NAME([this](u16 data) { m_bmp_screen_y[1] = data & 0x1ff; }));
	map(0x0405901c, 0x0405a01d).lrw16(NAME([this]() { return m_bmp_screen_y[2]; }), NAME([this](u16 data) { m_bmp_screen_y[2] = data & 0x1ff; }));
	map(0x0405901e, 0x0405a01f).lrw16(NAME([this]() { return m_bmp_screen_y[3]; }), NAME([this](u16 data) { m_bmp_screen_y[3] = data & 0x1ff; }));
	map(0x04059020, 0x0405a021).lrw16(NAME([this]() { return m_bmp_size_x[0]; }), NAME([this](u16 data) { m_bmp_size_x[0] = data; }));
	map(0x04059022, 0x0405a023).lrw16(NAME([this]() { return m_bmp_size_x[1]; }), NAME([this](u16 data) { m_bmp_size_x[1] = data; }));
	map(0x04059024, 0x0405a025).lrw16(NAME([this]() { return m_bmp_size_x[2]; }), NAME([this](u16 data) { m_bmp_size_x[2] = data; }));
	map(0x04059026, 0x0405a027).lrw16(NAME([this]() { return m_bmp_size_x[3]; }), NAME([this](u16 data) { m_bmp_size_x[3] = data; }));
	map(0x04059028, 0x0405a029).lrw16(NAME([this]() { return m_bmp_size_y[0]; }), NAME([this](u16 data) { m_bmp_size_y[0] = data & 0xff; }));
	map(0x0405902a, 0x0405a02b).lrw16(NAME([this]() { return m_bmp_size_y[1]; }), NAME([this](u16 data) { m_bmp_size_y[1] = data & 0xff; }));
	map(0x0405902c, 0x0405a02d).lrw16(NAME([this]() { return m_bmp_size_y[2]; }), NAME([this](u16 data) { m_bmp_size_y[2] = data & 0xff; }));
	map(0x0405902e, 0x0405a02f).lrw16(NAME([this]() { return m_bmp_size_y[3]; }), NAME([this](u16 data) { m_bmp_size_y[3] = data & 0xff; }));
	map(0x04059030, 0x04059031).lrw16(NAME([this]() { return m_bmp_control; }), NAME([this](u16 data) { m_bmp_control = data & 7; }));
	map(0x04059040, 0x04059041).lrw16(NAME([this]() { return m_bmp_color; }), NAME([this](u16 data) { m_bmp_color = data; }));
	map(0x04059050, 0x0405a051).lrw16(NAME([this]() { return m_bmp_span[0]; }), NAME([this](u16 data) { m_bmp_span[0] = data & 0x1ff; }));
	map(0x04059052, 0x0405a053).lrw16(NAME([this]() { return m_bmp_span[1]; }), NAME([this](u16 data) { m_bmp_span[1] = data & 0x1ff; }));
	map(0x04059054, 0x0405a055).lrw16(NAME([this]() { return m_bmp_span[2]; }), NAME([this](u16 data) { m_bmp_span[2] = data & 0x1ff; }));
	map(0x04059056, 0x0405a057).lrw16(NAME([this]() { return m_bmp_span[3]; }), NAME([this](u16 data) { m_bmp_span[3] = data & 0x1ff; }));
	map(0x0405a000, 0x0405a001).lrw16(NAME([this]() { return m_tilemap_control; }), NAME([this](u16 data) { m_tilemap_control = data & 0xff; }));
	map(0x0405a002, 0x0405a003).lrw16(NAME([this]() { return m_tilemap_scroll_x[0]; }), NAME([this](u16 data) { m_tilemap_scroll_x[0] = data & 0xfff; }));
	map(0x0405a004, 0x0405a005).lrw16(NAME([this]() { return m_tilemap_scroll_y[0]; }), NAME([this](u16 data) { m_tilemap_scroll_y[0] = data & 0xfff; }));
	map(0x0405a006, 0x0405a007).lrw16(NAME([this]() { return m_tilemap_scroll_x[1]; }), NAME([this](u16 data) { m_tilemap_scroll_x[1] = data & 0xfff; }));
	map(0x0405a008, 0x0405a009).lrw16(NAME([this]() { return m_tilemap_scroll_y[1]; }), NAME([this](u16 data) { m_tilemap_scroll_y[1] = data & 0xfff; }));
	map(0x0405a00a, 0x0405a00b).lrw16(NAME([this]() { return m_tilemap_color[0]; }), NAME([this](u16 data) { m_tilemap_color[0] = data; }));
	map(0x0405a00c, 0x0405a00d).lrw16(NAME([this]() { return m_tilemap_color[1]; }), NAME([this](u16 data) { m_tilemap_color[1] = data; }));
	map(0x0405a010, 0x0405a011).lrw16(NAME([this]() { return m_sprite_control; }), NAME([this](u16 data) { m_sprite_control = data & 0x7fff; }));
	map(0x0405a012, 0x0405a013).lrw16(NAME([this]() { return m_sprite_color[0]; }), NAME([this](u16 data) { m_sprite_color[0] = data; }));
	map(0x0405a014, 0x0405a015).lrw16(NAME([this]() { return m_sprite_color[1]; }), NAME([this](u16 data) { m_sprite_color[1] = data; }));
	map(0x0405a020, 0x0405a021).lrw16(NAME([this]() { return m_tilemap_page; }), NAME([this](u16 data) { m_tilemap_page = data & 0x7f; }));
	map(0x0405b000, 0x0405b001).lrw16(NAME([this]() { return m_layer_control_0; }), NAME([this](u16 data) { m_layer_control_0 = data & 7; }));
	map(0x0405b002, 0x0405b003).lrw16(NAME([this]() { return m_layer_enable; }), NAME([this](u16 data) { m_layer_enable = data; }));
	map(0x0405b004, 0x0405b005).lrw16(NAME([this]() { return m_layer_control_2; }), NAME([this](u16 data) { m_layer_control_2 = data & 0xff; }));
	map(0x0405b006, 0x0405b007).lrw16(NAME([this]() { return m_color[0]; }), NAME([this](u16 data) { m_color[0] = data & 0x7fff; }));
	map(0x0405b008, 0x0405b009).lrw16(NAME([this]() { return m_color[1]; }), NAME([this](u16 data) { m_color[1] = data & 0x7fff; }));
	map(0x0405b00a, 0x0405b00b).lrw16(NAME([this]() { return m_readback_control; }), NAME([this](u16 data) { m_readback_control = data & 0xfff; }));
	map(0x0405c000, 0x0405c001).lrw16(NAME([this]() { return m_int_control; }), NAME([this](u16 data) { m_int_control = data & 0x1ff; }));
	map(0x0405c002, 0x0405c003).lrw16(NAME([this]() { return m_screen_width; }), NAME([this](u16 data) { m_screen_width = data & 0x1ff; }));
	map(0x0405c004, 0x0405c005).lrw16(NAME([this]() { return m_screen_height; }), NAME([this](u16 data) { m_screen_height = data & 0x1ff; }));
	map(0x0405d000, 0x0405d001).rw(FUNC(casloopy_state::reg_5d000_r), FUNC(casloopy_state::reg_5d000_w));
	map(0x0405d010, 0x0405d011).r(FUNC(casloopy_state::controller_0_r));
	map(0x0405d012, 0x0405d013).r(FUNC(casloopy_state::controller_1_r));
	map(0x0405d014, 0x0405d015).r(FUNC(casloopy_state::controller_2_r));
	map(0x0405d020, 0x0405d021).rw(FUNC(casloopy_state::reg_5d020_r), FUNC(casloopy_state::reg_5d020_w));
	map(0x0405d030, 0x0405d031).rw(FUNC(casloopy_state::reg_5d030_r), FUNC(casloopy_state::reg_5d030_w));
	map(0x0405d040, 0x0405d041).rw(FUNC(casloopy_state::reg_5d040_r), FUNC(casloopy_state::reg_5d040_w));
	map(0x0405d042, 0x0405d043).rw(FUNC(casloopy_state::reg_5d042_r), FUNC(casloopy_state::reg_5d042_w));
	map(0x0405d044, 0x0405d045).rw(FUNC(casloopy_state::reg_5d044_r), FUNC(casloopy_state::reg_5d044_w));
	map(0x0405d050, 0x0405d051).r(FUNC(casloopy_state::mouse1_r));
	map(0x0405d052, 0x0405d053).r(FUNC(casloopy_state::mouse2_r));
	map(0x0405d054, 0x0405d055).rw(FUNC(casloopy_state::reg_5d054_r), FUNC(casloopy_state::reg_5d054_w));
	map(0x0405e000, 0x0405e001).rw(FUNC(casloopy_state::reg_5e000_r), FUNC(casloopy_state::reg_5e000_w));
	map(0x0405e002, 0x0405e003).lrw16(NAME([this]() { return m_clear_mask; }), NAME([this](u16 data) { m_clear_mask = data & 0x1ff; }));
	map(0x0405e004, 0x0405e005).lrw16(NAME([this]() { return m_clear_color; }), NAME([this](u16 data) { m_clear_color = data & 0xff; }));
	map(0x0405f000, 0x0405f3ff).w(FUNC(casloopy_state::fast_clear_w));
	map(0x04060000, 0x04060001).w(FUNC(casloopy_state::reg_60000_w));
	map(0x04080000, 0x04080001).w(FUNC(casloopy_state::sound_control_w));
	map(0x040a0000, 0x040a0001).w(FUNC(casloopy_state::reg_a0000_w));

	// /CS6: Cartridge ROM
	map(0x06000000, 0x063fffff).r(m_cart, FUNC(casloopy_cart_slot_device::rom_r)).mirror(0x08000000); // FIXME: This mirror should not be required
}


/*************************************
 *
 *  Input definitions
 *
 *************************************/

static INPUT_PORTS_START( casloopy )
	PORT_START("CONTROLLER_0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) // Joypad presence
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("L")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("R")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) // Mouse presence
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("D")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B")

	PORT_START("CONTROLLER_1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("CONTROLLER_2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x07, 0x00, "Seal Cartridge" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x01, "XS-11/XS-14" )
	PORT_CONFSETTING(    0x03, "XS-31" )
	PORT_CONFNAME( 0x70, 0x10, "Controller" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x10, "Joypad" )
	PORT_CONFSETTING(    0x20, "Mouse" )

	PORT_START("MOUSE")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("MOUSE_X")
	PORT_BIT( 0xfff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)

	PORT_START("MOUSE_Y")
	PORT_BIT( 0xfff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_REVERSE
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void casloopy_state::machine_start()
{
	save_item(NAME(m_d_mouse_x));
	save_item(NAME(m_d_mouse_y));
	save_item(NAME(m_bmp_latch));
	save_item(NAME(m_readback_buffer));
	save_item(NAME(m_system_control));
	save_item(NAME(m_bmp_vram_x));
	save_item(NAME(m_bmp_vram_y));
	save_item(NAME(m_bmp_screen_x));
	save_item(NAME(m_bmp_screen_y));
	save_item(NAME(m_bmp_size_x));
	save_item(NAME(m_bmp_size_y));
	save_item(NAME(m_bmp_color));
	save_item(NAME(m_bmp_control));
	save_item(NAME(m_bmp_span));
	save_item(NAME(m_screen_width));
	save_item(NAME(m_screen_height));
	save_item(NAME(m_int_control));
	save_item(NAME(m_tilemap_color));
	save_item(NAME(m_tilemap_control));
	save_item(NAME(m_tilemap_page));
	save_item(NAME(m_tilemap_scroll_x));
	save_item(NAME(m_tilemap_scroll_y));
	save_item(NAME(m_layer_control_0));
	save_item(NAME(m_layer_enable));
	save_item(NAME(m_layer_control_2));
	save_item(NAME(m_sprite_control));
	save_item(NAME(m_sprite_color));
	save_item(NAME(m_color));
	save_item(NAME(m_readback_control));
	save_item(NAME(m_clear_mask));
	save_item(NAME(m_clear_color));
	save_item(NAME(m_5d020));
	save_item(NAME(m_5d030));
	save_item(NAME(m_5d040));
	save_item(NAME(m_5d044));
	save_item(NAME(m_seal_sw));
	save_item(NAME(m_readback_latch));
}

void casloopy_state::machine_reset()
{
	m_d_mouse_x = 0;
	m_d_mouse_y = 0;
	std::fill_n(m_bmp_latch, std::size(m_bmp_latch), 0);
	std::fill_n(m_readback_buffer, std::size(m_readback_buffer), 0);

	m_system_control = 0;
	std::fill_n(m_bmp_vram_x, std::size(m_bmp_vram_x), 0);
	std::fill_n(m_bmp_vram_y, std::size(m_bmp_vram_y), 0);
	std::fill_n(m_bmp_screen_x, std::size(m_bmp_screen_x), 0);
	std::fill_n(m_bmp_screen_y, std::size(m_bmp_screen_y), 0);
	std::fill_n(m_bmp_size_x, std::size(m_bmp_size_x), 0);
	std::fill_n(m_bmp_size_y, std::size(m_bmp_size_y), 0);
	std::fill_n(m_bmp_span, std::size(m_bmp_span), 0x00ff);
	m_bmp_color = 0;
	m_bmp_control = 0;
	m_screen_width = 0;
	m_screen_height = 0;
	m_int_control = 0;
	std::fill_n(m_tilemap_color, std::size(m_tilemap_color), 0);
	m_tilemap_control = 0;
	m_tilemap_page = 0;
	std::fill_n(m_tilemap_scroll_x, std::size(m_tilemap_scroll_x), 0);
	std::fill_n(m_tilemap_scroll_y, std::size(m_tilemap_scroll_y), 0);
	m_layer_control_0 = 0;
	m_layer_enable = 0;
	m_layer_control_2 = 0;
	m_sprite_control = 0;
	std::fill_n(m_sprite_color, std::size(m_sprite_color), 0);
	std::fill_n(m_color, std::size(m_color), 0);
	m_readback_control = 0;
	m_clear_mask = 0;
	m_clear_color = 0;
	m_5d020 = 0;
	m_5d030 = 0;
	m_5d040 = 0;
	m_5d044 = 0;
	m_seal_sw = 0;
	m_readback_latch = 0;

	if (m_cart->exists())
	{
		// Cartridge presence bit
		m_maincpu->write_padr_bit<8>(1);
	}
}

static void casloopy_cart(device_slot_interface &device)
{
	device.option_add_internal("std",      CASLOOPY_ROM_STD);
	device.option_add_internal("adpcm",    CASLOOPY_ROM_ADPCM);
}

void casloopy_state::casloopy(machine_config &config)
{
	SH7021(config, m_maincpu, SH1_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &casloopy_state::casloopy_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(VIDEO_CLOCK / 4, H_TOTAL, 0, H_ACTIVE, V_TOTAL, 0, V_ACTIVE_0);
	m_screen->set_screen_update(FUNC(casloopy_state::screen_update));

	PALETTE(config, m_palette).set_entries(256);

	CASLOOPY_CART_SLOT(config, m_cart, SH1_CLOCK, casloopy_cart, nullptr).set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("casloopy");
}

} // anonymous namespace


/***************************************************************************

  Driver

***************************************************************************/

ROM_START( casloopy )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "hd6437021.lsi302", 0x0000, 0x8000, CRC(8c57ff9f) SHA1(17542ef275ea8ebeaaddd00d304232c3c355ea25) )

	ROM_REGION( 0x80000, "wave", 0)
	ROM_LOAD( "hn62434fa.lsi352", 0x0000, 0x80000, CRC(8f51fa17) SHA1(99f50be06b083fdb07e08f30b0b26d9037afc869) )
ROM_END

CONS( 1995, casloopy, 0, 0, casloopy, casloopy, casloopy_state, empty_init, "Casio", "Loopy", MACHINE_NO_SOUND | MACHINE_NODEVICE_PRINTER )
