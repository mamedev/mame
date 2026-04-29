// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

SMC-777 (c) 1983 Sony

References:
- http://www.x1center.org/smc777/smc70_777_IOMAP_20210923.txt
- https://web.archive.org/web/20230207074451/https://monochromeeffect.org/JVCC/2021/11/23/sony-smc-777c/

TODO:
- convert video to a proper mc6845, consider this as a good base for new device rewrite;
- cursor stuck in Bird Crash;
- interlace (cfr. cpm22 in setup mode);
- ROM/RAM bankswitch, it apparently happens after one instruction prefetching.
  Hacked around for now;
- tape;
- .mfi floppy format throws a crash;
- Downgrade for SMC-70 (if/when dump is available). Has extra GFX modes that 777 dropped;
- Superimposing features (if/when SW dumps arises, cfr. SMC-70G promotional video)
- Find better reference materials, available one lacks several pages;
- Hookup expansion bus (needs pinout/specifications)

**************************************************************************************************/

#include "emu.h"

#include "smc777_kbd.h"

#include "bus/msx/ctrl/ctrl.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/74259.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "sound/sn76496.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

#define mc6845_h_char_total     (m_crtc_vreg[0]+1)
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4]+1)
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0x3f00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0x3f00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0x3f00) | (m_crtc_vreg[0x13] & 0xff))

class smc777_state : public driver_device
{
public:
	smc777_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_crtc(*this, "crtc")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_ioctrl(*this, "ioctrl")
		, m_dac1bit(*this, "dac1bit")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_kbd(*this, "kbd")
		// "JOY STICK#" dual DE-9 ports, on the right side of body chassis (near the volume knob)
		, m_joystick_port(*this, "joystick%u", 1U)
		, m_kanji_rom(*this, "kanji")
	{ }

	void smc777(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

private:
	void mc6845_w(offs_t offset, u8 data);
	u8 vram_r(offs_t offset);
	u8 attr_r(offs_t offset);
	u8 pcg_r(offs_t offset);
	void vram_w(offs_t offset, u8 data);
	void attr_w(offs_t offset, u8 data);
	void pcg_w(offs_t offset, u8 data);
	u8 fbuf_r(offs_t offset);
	void fbuf_w(offs_t offset, u8 data);
	u8 kanji_r(offs_t offset);
	void kanji_w(offs_t offset, u8 data);
	void border_col_w(u8 data);
	u8 io_status_1c_r();
	u8 io_status_1d_r();
	void io_control_w(u8 data);
	void raminh_w(int state);
	void vsup_w(int state);
	void screen_lines_w(int state);
	void rgb_select_w(int state);
	void mt_on_w(int state);
	void sound_out_w(int state);
	void printer_strb_w(int state);
	void cas_out_w(int state);
	void color_mode_w(offs_t offset, u8 data);
	void ramdac_w(offs_t offset, u8 data);
	u8 gcw_r();
	void gcw_w(u8 data);
	u8 main_map_r(offs_t offset);
	void main_map_w(offs_t offset, u8 data);
	u8 vsync_irq_status_r();
	void vsync_irq_enable_w(u8 data);
	void palette_init(palette_device &palette) const;
	void vsync_w(int state);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void graphic_320x200x4(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void graphic_640x200x2(bitmap_ind16 &bitmap, const rectangle &cliprect);

	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);
	u8 fdc1_fast_status_r();
	void fdc1_select_w(u8 data);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<mc6845_device> m_crtc;
	required_device<mb8877_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<ls259_device> m_ioctrl;
	required_device<speaker_sound_device> m_dac1bit;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<smc777_kbd_device> m_kbd;
	required_device_array<msx_general_purpose_port_device, 2> m_joystick_port;
	required_region_ptr<u8> m_kanji_rom;

	u8 *m_ipl_rom = nullptr;
	std::unique_ptr<u8[]> m_work_ram;
	std::unique_ptr<u8[]> m_vram;
	std::unique_ptr<u8[]> m_attr;
	std::unique_ptr<u8[]> m_gvram;
	std::unique_ptr<u8[]> m_pcg;

	u8 m_backdrop_pen = 0;
	u8 m_display_reg = 0;
	u8 m_fdc_irq_flag = 0;
	u8 m_fdc_drq_flag = 0;
	struct { u8 r = 0, g = 0, b = 0; } m_pal;
	u8 m_raminh = 0, m_raminh_pending_change = 0; //bankswitch
	u8 m_raminh_prefetch = 0;
	u8 m_text_pal_bank = 0;
	u8 m_gfx_pal_bank = 0;
	u8 m_crtc_vreg[0x20]{};
	u8 m_crtc_addr = 0;
	bool m_vsync_idf = false;
	bool m_vsync_ief = false;
	u8 m_de;
	int m_warm_reset = -1;
	u8 m_kanji_addr[2]{};
};


/* TODO: correct calculation thru mc6845 regs */
#define CRTC_MIN_X 24*8
#define CRTC_MIN_Y 4*8

static const gfx_layout pcg_layout =
{
	8, 8,
	0x800 / 8,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

void smc777_state::video_start()
{
	m_vram = make_unique_clear<u8[]>(0x800);
	m_attr = make_unique_clear<u8[]>(0x800);
	m_gvram = make_unique_clear<u8[]>(0x8000);
	m_pcg = make_unique_clear<u8[]>(0x800);

	save_pointer(NAME(m_vram), 0x800);
	save_pointer(NAME(m_attr), 0x800);
	save_pointer(NAME(m_gvram), 0x8000);
	save_pointer(NAME(m_pcg), 0x800);
	save_item(NAME(m_crtc_vreg));
	save_item(NAME(m_text_pal_bank));
	save_item(NAME(m_gfx_pal_bank));
	save_item(STRUCT_MEMBER(m_pal, r));
	save_item(STRUCT_MEMBER(m_pal, g));
	save_item(STRUCT_MEMBER(m_pal, b));

	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, pcg_layout, m_pcg.get(), 0, 8, 0));
}

void smc777_state::video_reset()
{
	m_text_pal_bank = 0x00;
	m_gfx_pal_bank = 0x00;
}

// TODO: cleanup, move to device, honor cliprect
void smc777_state::graphic_320x200x4(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u16 bitmap_pitch = mc6845_h_display * 2;

	const u8 color_bank = m_gfx_pal_bank;

	for(int y = 0; y < mc6845_v_display; y++)
	{
		const u32 base_address = y * bitmap_pitch + mc6845_start_addr * 2;
		const int res_y = y * 8 + CRTC_MIN_Y;

		for(int x = 0; x < bitmap_pitch; x++)
		{
			const int res_x = x * 4 + CRTC_MIN_X;
			for(int yi = 0; yi < 8; yi++)
			{
				const u8 dot = m_gvram[((base_address + x) & 0xfff) + yi * 0x1000];
				u8 color = (dot & 0xf0) >> 4;
				color |= color_bank;
				bitmap.pix(res_y + yi, res_x + 0) = m_palette->pen(color);
				bitmap.pix(res_y + yi, res_x + 1) = m_palette->pen(color);

				color = (dot & 0x0f) >> 0;
				color |= color_bank;
				bitmap.pix(res_y + yi, res_x + 2) = m_palette->pen(color);
				bitmap.pix(res_y + yi, res_x + 3) = m_palette->pen(color);
			}
		}
	}
}

void smc777_state::graphic_640x200x2(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u8 color_table[8] = { 0x00, 0x04, 0x02, 0x01, 0x00, 0x04, 0x02, 0x07 };
	const u16 bitmap_pitch = mc6845_h_display * 2;

	const u8 color_bank = (BIT(m_display_reg, 5) << 2);

	for(int y = 0; y < mc6845_v_display; y++)
	{
		// comp2:HETZER uses start address for double buffering
		const u32 base_address = y * bitmap_pitch + mc6845_start_addr * 2;
		const int res_y = y * 8 + CRTC_MIN_Y;

		for(int x = 0; x < bitmap_pitch; x++)
		{
			const int res_x = x * 4 + CRTC_MIN_X;
			for(int yi = 0; yi < 8; yi++)
			{
				const u8 dot = m_gvram[((base_address + x) & 0xfff) + yi * 0x1000];
				for (int xi = 0; xi < 4; xi++)
				{
					u8 color = (dot >> (6 - (xi * 2))) & 3;
					color |= color_bank;
					// TODO: unconfirmed if 2bpp can select the palette board
					bitmap.pix(res_y + yi, res_x + xi) = m_palette->pen(color_table[color] | m_gfx_pal_bank);
				}
			}
		}
	}
}

u32 smc777_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// - strizbh seems more logical with gfx pal bank hooked
	bitmap.fill(m_palette->pen(m_backdrop_pen | m_gfx_pal_bank), cliprect);

	int x_width = BIT(m_display_reg, 7);

	if (BIT(m_display_reg, 3))
		graphic_640x200x2(bitmap, cliprect);
	else
		graphic_320x200x4(bitmap, cliprect);

	// TODO: remove count usage
	// - transitt: does horizontal scroll on ranking screen
	// TODO: transitt itself has sync timing issues there
	u16 count = mc6845_start_addr;

	const u16 frame_number = m_screen->frame_number();

	for(int y = 0; y < mc6845_v_display; y++)
	{
		for(int x = 0; x < mc6845_h_display / (x_width + 1); x++)
		{
			/*
			 * x--- ---- FRC force CCOL = 7, BCOL/REV/BRNK = 0
			 * -x-- ---- BRNK blink
			 * --x- ---- REV reverse pattern
			 * ---x x--- BCOL bg color (00 transparent, 01 white, 10 black, 11 complementary to fg color
			 * ---- -xxx CCOL fg color
			 */
			u8 attr = m_attr[count];
			// TODO: blind faith, no known use cases
			if (BIT(attr, 7))
				attr = 0x07;
			const u8 tile = m_vram[count];
			u8 color = (attr & 7);
			const u8 bk_color = (attr & 0x18) >> 3;
			const u8 reverse = BIT(attr, 5);
			const u8 blink = BIT(attr, 6);
			//int bk_struct[4] = { -1, 0x10, 0x11, (color & 7) ^ 8 };

			int bk_pen = -1;
			switch(bk_color & 3)
			{
				// transparent
				case 0: bk_pen = -1; break;
				// white
				case 1: bk_pen = 0x07; break;
				// black
				case 2: bk_pen = 0x10; break;
				// complementary
				case 3: bk_pen = (color ^ 0xf); break;
			}

			// handle blink (dragon)
			if(blink && frame_number & 0x10)
				color = bk_pen;

			for(int yi = 0; yi < 8; yi++)
			{
				u8 pattern = m_pcg[tile * 8 + yi];
				// amazon on # disk drives
				if (reverse)
					pattern ^= 0xff;
				for(int xi = 0; xi < 8; xi++)
				{
					int pen = BIT(pattern, 7 - xi) ? (color + m_text_pal_bank) : bk_pen;

					if (pen != -1)
					{
						if(x_width)
						{
							bitmap.pix(y * 8 + CRTC_MIN_Y + yi, (x * 8 + xi) * 2 + 0 + CRTC_MIN_X) = m_palette->pen(pen);
							bitmap.pix(y * 8 + CRTC_MIN_Y + yi, (x * 8 + xi) * 2 + 1 + CRTC_MIN_X) = m_palette->pen(pen);
						}
						else
							bitmap.pix(y * 8 + CRTC_MIN_Y + yi, x * 8 + CRTC_MIN_X + xi) = m_palette->pen(pen);
					}
				}
			}

			// draw cursor
			if(mc6845_cursor_addr == count)
			{
				int cursor_on = 0;
				switch(mc6845_cursor_y_start & 0x60)
				{
					case 0x00: cursor_on = 1; break; //always on
					case 0x20: cursor_on = 0; break; //always off
					case 0x40: if(frame_number & 0x10) { cursor_on = 1; } break; // fast blink
					case 0x60: if(frame_number & 0x20) { cursor_on = 1; } break; // slow blink
				}

				if(cursor_on)
				{
					const int y_start = mc6845_cursor_y_start & 0x1f;
					const int y_end = mc6845_cursor_y_end & 0x1f;
					// range is inclusive (kanjicpm underscore cursor)
					for(int yc = y_start; yc <= y_end; yc++)
					{
						// sys12j:BIRD and transitt (at very least) disables cursor by pushing it
						// outside the visible range rather than just setting R10[6:5] properly
						if (yc >= mc6845_tile_height)
							continue;
						for(int xc = 0; xc < 8; xc++)
						{
							if(x_width)
							{
								bitmap.pix(y * 8 + yc + CRTC_MIN_Y, (x * 8 + xc) * 2 + 0 + CRTC_MIN_X) = m_palette->pen(0x7);
								bitmap.pix(y * 8 + yc + CRTC_MIN_Y, (x * 8 + xc) * 2 + 1 + CRTC_MIN_X) = m_palette->pen(0x7);
							}
							else
								bitmap.pix(y * 8 + yc + CRTC_MIN_Y, x * 8 + CRTC_MIN_X + xc) = m_palette->pen(0x7);
						}
					}
				}
			}

			count += x_width + 1;
			count &= 0x7ff;
		}
	}

	return 0;
}

void smc777_state::mc6845_w(offs_t offset, u8 data)
{
	if(offset == 0)
	{
		m_crtc_addr = data;
		m_crtc->address_w(data);
	}
	else
	{
		m_crtc_vreg[m_crtc_addr] = data;
		m_crtc->register_w(data);
	}
}

u8 smc777_state::vram_r(offs_t offset)
{
	u16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return m_vram[vram_index];
}

u8 smc777_state::attr_r(offs_t offset)
{
	u16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return m_attr[vram_index];
}

u8 smc777_state::pcg_r(offs_t offset)
{
	u16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return m_pcg[vram_index];
}

void smc777_state::vram_w(offs_t offset, u8 data)
{
	u16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	m_vram[vram_index] = data;
}

void smc777_state::attr_w(offs_t offset, u8 data)
{
	u16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	m_attr[vram_index] = data;
}

void smc777_state::pcg_w(offs_t offset, u8 data)
{
	u16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	m_pcg[vram_index] = data;

	m_gfxdecode->gfx(0)->mark_dirty(vram_index >> 3);
}

u8 smc777_state::fbuf_r(offs_t offset)
{
	u16 vram_index;

	vram_index  = ((offset & 0x007f) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return m_gvram[vram_index];
}

void smc777_state::fbuf_w(offs_t offset, u8 data)
{
	u16 vram_index;

	vram_index  = ((offset & 0x00ff) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	m_gvram[vram_index] = data;
}

u8 smc777_state::kanji_r(offs_t offset)
{
	// TODO: $7f actually selects BASIC ROM for regular SMC-70
	if (offset & 1)
		return 0xff;

	if (m_kanji_addr[0] < 0x21 || m_kanji_addr[1] < 0x20)
	{
		if (!machine().side_effects_disabled())
			logerror("kanji_r: reading out of bounds %02x%02x\n", m_kanji_addr[0], m_kanji_addr[1]);
		return 0xff;
	}

	// TODO: this is glue logic to adapt to the clearly inaccurate .dat file
	// Also assuming that this system has undumped lv2 option.
	u32 kanji_base = (((m_kanji_addr[0] - 0x21) * 0x60) + (m_kanji_addr[1] - 0x20)) << 5;
	u32 kanji_index = (offset >> 8) & 0x1f;

	return m_kanji_rom[kanji_base | kanji_index];
}

void smc777_state::kanji_w(offs_t offset, u8 data)
{
	m_kanji_addr[offset & 1] = data & 0x7f;
}

u8 smc777_state::fdc_r(offs_t offset)
{
	return m_fdc->read(offset);
}

void smc777_state::fdc_w(offs_t offset, u8 data)
{
	m_fdc->write(offset, data);
}

u8 smc777_state::fdc1_fast_status_r()
{
	u8 data = 0;

	// TODO: inverted wrt documentation?
	data |= !m_fdc_drq_flag << 6;
	data |= m_fdc_irq_flag << 7;

	return data;
}

void smc777_state::fdc1_select_w(u8 data)
{
	floppy_image_device *floppy = nullptr;

	// x--- ---- SIDE1: [SMC-70] side select
	// ---- --x- EXDS: [SMC-70] external drive select (0=internal, 1=external)
	// ---- ---x DS01: select floppy drive
	floppy = m_floppy[data & 1]->get_device();

	m_fdc->set_floppy(floppy);

	// no idea where the motor on signal is
	if (floppy)
		floppy->mon_w(0);

	if(data & 0xf0)
		logerror("floppy access %02x\n", data);
}

void smc777_state::fdc_intrq_w(int state)
{
	m_fdc_irq_flag = state;
}

void smc777_state::fdc_drq_w(int state)
{
	m_fdc_drq_flag = state;
}

void smc777_state::border_col_w(u8 data)
{
	if(data & 0xf0)
		logerror("Special border color enabled %02x\n",data);

	m_backdrop_pen = data & 0xf;
}

/*
 * RES     x--- ---- Warm reset if '1'
 * HiZ     -x-- ---- [SMC-70] no drive (always '1'?)
 * LPH     --x- ---- [SMC-70] light pen H position
 * CP      ---x ---- [SMC-777] color board (active low)
 * LPV     ---x x--- [SMC-70] light pen V position
 * ID      ---- -x-- 0=SMC-777 1=SMC-70
 * MD      ---- --xx [SMC-70] boot mode (00=DISK; 10=ROM; 11=EXT)
 */
u8 smc777_state::io_status_1c_r()
{
	if (!machine().side_effects_disabled())
		logerror("System R\n");

	return (m_warm_reset << 7);
}

/*
 * TCIN    x--- ---- CMT read data
 * HiZ     -x-- ---- [SMC-70] no drive (always '1'?)
 * LPIN    --x- ---- [SMC-70] light pen input
 * PR_BUSY ---x ---- printer busy
 * PR_ACK  ---- x--- printer ACK
 * ID      ---- -x-- 0=SMC-777 1=SMC-70
 * MD      ---- --xx [SMC-70] boot mode (00=DISK; 10=ROM; 11=EXT)
 */
u8 smc777_state::io_status_1d_r()
{
	return 0;
}

void smc777_state::io_control_w(u8 data)
{
	m_ioctrl->write_bit(data & 0x07, BIT(data, 4));
}

void smc777_state::raminh_w(int state)
{
	// "ROM / RAM register change is done at the beginning of the next M1 cycle"
	m_raminh_pending_change = state ^ 1;
	m_raminh_prefetch = (u8)(m_maincpu->state_int(Z80_R)) & 0x7f;
}

void smc777_state::vsup_w(int state)
{
	logerror("%s: Display %sabled\n", machine().describe_context(), state ? "dis" : "en");
}

void smc777_state::screen_lines_w(int state)
{
	logerror("%s: Screen line number %d\n", machine().describe_context(), state ? 625 : 525);
}

void smc777_state::rgb_select_w(int state)
{
	// 0=RGB 1=Component
	logerror("%s: %s video selected\n", machine().describe_context(), state ? "Component" : "RGB");
}

void smc777_state::mt_on_w(int state)
{
	logerror("%s: Cassette monitor %s\n", machine().describe_context(), state ? "on" : "off");
}

void smc777_state::sound_out_w(int state)
{
	m_dac1bit->level_w(state);
}

void smc777_state::printer_strb_w(int state)
{
	logerror("%s: Printer strobe %d\n", machine().describe_context(), !state);
}

void smc777_state::cas_out_w(int state)
{
	logerror("%s: Cassette write %d\n", machine().describe_context(), state);
}

/*
 * ---x -111 gfx palette select
 * ---x -110 text palette select
 * ---x -101 joy 2 out /CS
 * ---x -100 joy 2 out U
 * ---x -011 joy 2 out T
 * ---x -010 joy 2 out R
 * ---x -001 joy 2 out L
 * ---x -000 joy 2 out B
 */
void smc777_state::color_mode_w(offs_t offset, u8 data)
{
//  printf("%d %d %02x\n", data & 7, BIT(data, 4), data);
	// - transitt at PC=150: reads if color board is present, reads DE-9 port 2,
	//   ands with 0x3f then pulls bit 7 high. Pretty creative way for a btanb.
	if (data & 0xe8)
		logerror("color_mode_w: warning setup %02x\n", data);
	switch(data & 0x07)
	{
		case 0x05: m_joystick_port[!BIT(offset, 8)]->pin_8_w(BIT(data, 4)); break;
		case 0x06: m_text_pal_bank = (data & 0x10); break;
		// demofd4 main menu
		case 0x07: m_gfx_pal_bank = (data & 0x10);  break;
		default: logerror("Unhandled color FF %02x\n",data); break;
	}
}

void smc777_state::ramdac_w(offs_t offset, u8 data)
{
	const u8 pal_index = ((offset & 0xf00) >> 8) | 0x10;

	// TODO: is this really unused like IOMAP claims?
	if(data & 0x0f)
		logerror("RAMdac used with data bits 0-3 set (%02x)\n",data);

	switch((offset & 0x3000) >> 12)
	{
		case 0: m_pal.r = (data & 0xf0) >> 4; m_palette->set_pen_color(pal_index, pal4bit(m_pal.r), pal4bit(m_pal.g), pal4bit(m_pal.b)); break;
		case 1: m_pal.g = (data & 0xf0) >> 4; m_palette->set_pen_color(pal_index, pal4bit(m_pal.r), pal4bit(m_pal.g), pal4bit(m_pal.b)); break;
		case 2: m_pal.b = (data & 0xf0) >> 4; m_palette->set_pen_color(pal_index, pal4bit(m_pal.r), pal4bit(m_pal.g), pal4bit(m_pal.b)); break;
		case 3: logerror("RAMdac used with gradient index = 3! pal_index = %02x data = %02x\n",pal_index,data); break;
	}
}

u8 smc777_state::gcw_r()
{
	return m_display_reg;
}

/*
 * x--- ---- text mode (0 = 80x25 1 = 40x25)
 * -x-- ---- text page (in 40x25 mode)
 * --x- ---- color mode (1=for 2bpp mode, blue is replaced with white)
 * ---x ---- [SMC-70] interlace
 * ---- xxyy gfx mode (model dependant)
 * [SMC-70]
 * ---- 11-- 640x400x1 bpp
 * ---- 10-- 640x200x2 bpp
 * ---- 01-- 320x200x4 bpp
 * ---- 00yy 160x100x4 bpp, bits 0-1 selects page
 * [SMC-777]
 * ---- 1--- 640x200x2 bpp
 * ---- 0--- 320x200x4 bpp
 */
void smc777_state::gcw_w(u8 data)
{
	m_display_reg = data;
}

u8 smc777_state::main_map_r(offs_t offset)
{
	u8 z80_r;

	// TODO: do the bankswitch AFTER that the prefetch instruction is executed (hackish implementation)
	if(m_raminh_prefetch != 0xff && !machine().side_effects_disabled())
	{
		z80_r = (u8)m_maincpu->state_int(Z80_R);

		if(z80_r == ((m_raminh_prefetch+1) & 0x7f))
		{
			m_raminh = m_raminh_pending_change;
			m_raminh_prefetch = 0xff;
		}
	}

	if(m_raminh == 1 && ((offset & 0xc000) == 0))
		return m_ipl_rom[offset];

	return m_work_ram[offset];
}

void smc777_state::main_map_w(offs_t offset, u8 data)
{
	m_work_ram[offset] = data;
}

u8 smc777_state::vsync_irq_status_r()
{
	if (m_vsync_idf == true)
	{
		m_vsync_idf = false;
		return 1;
	}

	return 0;
}

void smc777_state::vsync_irq_enable_w(u8 data)
{
	if(data & 0xfe)
		logerror("Irq mask = %02x\n",data & 0xfe);

	// IRQ mask
	m_vsync_ief = BIT(data, 0);
	// TODO: clear idf on 1->0 irq mask transition?
}

void smc777_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(smc777_state::main_map_r), FUNC(smc777_state::main_map_w));
}

void smc777_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x07).select(0xff00).rw(FUNC(smc777_state::vram_r), FUNC(smc777_state::vram_w));
	map(0x08, 0x0f).select(0xff00).rw(FUNC(smc777_state::attr_r), FUNC(smc777_state::attr_w));
	map(0x10, 0x17).select(0xff00).rw(FUNC(smc777_state::pcg_r), FUNC(smc777_state::pcg_w));
	map(0x18, 0x19).mirror(0xff00).w(FUNC(smc777_state::mc6845_w));
	map(0x1a, 0x1a).mirror(0xff00).rw(m_kbd, FUNC(smc777_kbd_device::data_r), FUNC(smc777_kbd_device::data_w));
	map(0x1b, 0x1b).mirror(0xff00).rw(m_kbd, FUNC(smc777_kbd_device::status_r), FUNC(smc777_kbd_device::control_w));
	map(0x1c, 0x1c).mirror(0xff00).rw(FUNC(smc777_state::io_status_1c_r), FUNC(smc777_state::io_control_w));
	map(0x1d, 0x1d).mirror(0xff00).rw(FUNC(smc777_state::io_status_1d_r), FUNC(smc777_state::io_control_w));
//  map(0x1e, 0x1f) rs232 irq control
	map(0x20, 0x20).mirror(0xff00).rw(FUNC(smc777_state::gcw_r), FUNC(smc777_state::gcw_w));
	map(0x21, 0x21).mirror(0xff00).rw(FUNC(smc777_state::vsync_irq_status_r), FUNC(smc777_state::vsync_irq_enable_w));
//  map(0x22, 0x22) printer output data
	map(0x23, 0x23).mirror(0xff00).w(FUNC(smc777_state::border_col_w));
//  map(0x24, 0x24) rtc write address (MSM58321RS)
//  map(0x25, 0x25) rtc read
//  map(0x26, 0x26) rs232 #1
//  map(0x28, 0x2c) fdc #2 (8", SMI-7016)
//  map(0x2d, 0x2f) rs232 #2
	map(0x30, 0x33).mirror(0xff00).rw(FUNC(smc777_state::fdc_r), FUNC(smc777_state::fdc_w));
	map(0x34, 0x34).mirror(0xff00).rw(FUNC(smc777_state::fdc1_fast_status_r), FUNC(smc777_state::fdc1_select_w));
//  map(0x35, 0x37) rs232 #3
//  map(0x38, 0x3b) cache disk unit
	// 0x38 (R) CDSTS status port (W) CDCMD command port
	// 0x39 (W) track register
	// 0x3a (W) sector register
	// 0x3b (RW) data port
//  map(0x3c, 0x3d) SMI-7074 NTSC superimposer / genlock control
//  map(0x40, 0x47) SMI-7032 ieee-488 / TMS9914A I/F
	map(0x44, 0x44).mirror(0xff00).portr("GPDSW"); // normally unmapped in GPIB interface
//  map(0x48, 0x49) hdd (winchester, thru UPI41?)
	// joystick read (all active low)
	// x--- ---- /BL, in blanking
	// -x-- ---- /CS, joystick 2 present
	// --x- ---- /USER, DE-9 port 2 button 2 (unconnected on port 1)
	// ---x xxxx from DE-9
	map(0x51, 0x51).select(0xff00).lr8(NAME([this] (offs_t offset) {
		const u8 joy_select = !BIT(offset, 8);
		const u8 mask = 0x1f | (joy_select << 5);
		const u8 joy2_cs = (!m_joystick_port[1]->exists()) << 6;
		const u8 joy_in = m_joystick_port[!BIT(offset, 8)]->read() & mask;

		return joy_in | joy2_cs | m_de;
	})).w(FUNC(smc777_state::color_mode_w));
	map(0x52, 0x52).select(0xff00).w(FUNC(smc777_state::ramdac_w));
	map(0x53, 0x53).mirror(0xff00).w("sn1", FUNC(sn76489a_device::write));
//  map(0x54, 0x59) vrt controller
//  map(0x5a, 0x5b) IPL-ROM banking (?)
//  map(0x70, 0x70) auto-start ROM (ext-ROM)
//  map(0x74, 0x74) SMI-7032 ieee-488 GPIB ROM port
//  map(0x75, 0x75) vrt controller ROM
//  map(0x78, 0x7b) I/O 1984.8.P202 light pen
//  map(0x7c, 0x7d) Oh!HITBIT Vol.6 MIDI I/F thru I8251
	map(0x7e, 0x7f).select(0xff00).rw(FUNC(smc777_state::kanji_r), FUNC(smc777_state::kanji_w));
	map(0x80, 0xff).select(0xff00).rw(FUNC(smc777_state::fbuf_r), FUNC(smc777_state::fbuf_w));
}


static INPUT_PORTS_START( smc777 )
	PORT_START("GPDSW")
	PORT_DIPNAME( 0x01, 0x00, "GPDSW" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

void smc777_state::machine_start()
{
	m_ipl_rom = memregion("ipl")->base();
	m_work_ram = make_unique_clear<u8[]>(0x10000);

	save_pointer(NAME(m_work_ram), 0x10000);
	save_item(NAME(m_warm_reset));

	m_warm_reset = -1;
}

void smc777_state::machine_reset()
{
	m_raminh = 1;
	m_raminh_pending_change = 1;
	m_raminh_prefetch = 0xff;
	m_vsync_idf = false;

	// warm reset is special, ties with a physical back button
	// (avoids bad bootstraps by resetting with our F3 key)
	m_warm_reset++;
	if (m_warm_reset >= 1)
		m_warm_reset = 1;

	m_dac1bit->level_w(0);
}


void smc777_state::palette_init(palette_device &palette) const
{
	// setup internal color generator (fixed part)
	// - cpm22/sys12j selects this for background pen to work properly.
	const u16 default_palette[0x10] = {
		0x000, 0x00f, 0x0f0, 0x0ff, 0xf00, 0xf0f, 0xff0, 0xfff,
		0x141, 0x172, 0xd52, 0xe92, 0x158, 0x19e, 0xf79, 0x888
	};
	for(int i = 0x00; i < 0x10; i++)
	{
		u8 const r = (default_palette[i] >> 8) & 0xf;
		u8 const g = (default_palette[i] >> 4) & 0xf;
		u8 const b = (default_palette[i] >> 0) & 0xf;

		palette.set_pen_color(i, pal4bit(r), pal4bit(g), pal4bit(b));
	}
}


void smc777_state::vsync_w(int state)
{
	if (state && m_vsync_ief)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_vsync_idf = true;
	}
}

/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER(smc777_state::quickload_cb)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	// Avoid loading a program if CP/M-80 is not in memory
	if ((prog_space.read_byte(0) != 0xc3) || (prog_space.read_byte(5) != 0xc3))
	{
		machine_reset();
		return std::make_pair(image_error::UNSUPPORTED, "CP/M must already be running");
	}

	// Check for sufficient RAM based on position of CPM
	const int mem_avail = 256 * prog_space.read_byte(7) + prog_space.read_byte(6) - 512;
	if (mem_avail < image.length())
		return std::make_pair(image_error::UNSPECIFIED, "Insufficient memory available");

	// Load image to the TPA (Transient Program Area)
	u16 quickload_size = image.length();
	for (u16 i = 0; i < quickload_size; i++)
	{
		u8 data;
		if (image.fread(&data, 1) != 1)
			return std::make_pair(image_error::UNSPECIFIED, "Problem reading the image at offset " + std::to_string(i));
		prog_space.write_byte(i + 0x100, data);
	}

	// clear out command tail
	prog_space.write_byte(0x80, 0);
	prog_space.write_byte(0x81, 0);

	// Roughly set SP basing on the BDOS position
	m_maincpu->set_state_int(Z80_SP, mem_avail + 384);
	m_maincpu->set_pc(0x100); // start program

	return std::make_pair(std::error_condition(), std::string());
}

static void smc777_floppies(device_slot_interface &device)
{
	device.option_add("35ssdd", FLOPPY_35_SSDD);
	// TODO: 5'25" and 8" options
}

void smc777_state::smc777(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 32.2238_MHz_XTAL;

	Z80(config, m_maincpu, MASTER_CLOCK / 8); // nominally 4.028 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &smc777_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &smc777_state::io_map);

	I8041A(config, "mcu", 6_MHz_XTAL).set_disable();

	LS259(config, m_ioctrl);
	m_ioctrl->q_out_cb<0>().set(FUNC(smc777_state::raminh_w));
	m_ioctrl->q_out_cb<1>().set(FUNC(smc777_state::vsup_w));
	m_ioctrl->q_out_cb<2>().set(FUNC(smc777_state::screen_lines_w));
	m_ioctrl->q_out_cb<3>().set(FUNC(smc777_state::rgb_select_w));
	m_ioctrl->q_out_cb<4>().set(FUNC(smc777_state::mt_on_w));
	m_ioctrl->q_out_cb<5>().set(FUNC(smc777_state::sound_out_w));
	m_ioctrl->q_out_cb<6>().set(FUNC(smc777_state::printer_strb_w));
	m_ioctrl->q_out_cb<7>().set(FUNC(smc777_state::cas_out_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK / 2, 1024, 0, 640, 262, 0, 200);
	m_screen->set_screen_update(FUNC(smc777_state::screen_update));
	m_screen->set_palette(m_palette);
	// TODO: 6845 CCLK width is likely supposed to handle this (currently draws 1023x261)
	m_screen->set_default_position(1.500, 0.000, 1.125, 0.000);

	// 16 + 8 colors (SMC-777 + SMC-70) + 8 empty entries (SMC-70)
	PALETTE(config, m_palette, FUNC(smc777_state::palette_init), 0x20);

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	HD6845S(config, m_crtc, MASTER_CLOCK / 16); // HD46505SP-1 / HD68A45SP; unknown clock, hand tuned to get ~60 fps
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->out_vsync_callback().set(FUNC(smc777_state::vsync_w));
	m_crtc->out_de_callback().set([this] (int state) { m_de = state << 7; });

	// Confirmed MB8877A
	MB8877(config, m_fdc, MASTER_CLOCK / 32); // divider not confirmed
	m_fdc->intrq_wr_callback().set(FUNC(smc777_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(smc777_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], smc777_floppies, "35ssdd", floppy_image_device::default_mfm_floppy_formats);
	// by default it has no 2nd floppy drive (has connector on the back panel)
	FLOPPY_CONNECTOR(config, m_floppy[1], smc777_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	SOFTWARE_LIST(config, "flop_list").set_original("smc777");
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(smc777_state::quickload_cb));

	SMC777_KBD(config, m_kbd, 0);

	// No clue about bundled defaults but:
	// - dragon expects joystick in port 1
	// - comp2:SMCPAINT/smcpaint expects mouse in port 2
	// - penjamin pukes if a mouse is in port 2
	MSX_GENERAL_PURPOSE_PORT(config, m_joystick_port[0], msx_general_purpose_port_devices, "joystick");
	MSX_GENERAL_PURPOSE_PORT(config, m_joystick_port[1], msx_general_purpose_port_devices, "mouse");

	SPEAKER(config, "mono").front_center();

	SN76489A(config, "sn1", MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.50); // unknown clock / divider

	SPEAKER_SOUND(config, m_dac1bit).add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START( smc777 )
	/* shadow ROM */
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "1st", "1st rev.")
	ROMX_LOAD( "smcrom.dat", 0x0000, 0x4000, CRC(b2520d31) SHA1(3c24b742c38bbaac85c0409652ba36e20f4687a1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "2nd", "2nd rev.")
	ROMX_LOAD( "smcrom.v2",  0x0000, 0x4000, CRC(c1494b8f) SHA1(a7396f5c292f11639ffbf0b909e8473c5aa63518), ROM_BIOS(1))

	// should be binary extracted rather than chip level dump(s)
	// also cfr. negative math in kanji_r
	ROM_REGION( 0x48000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanjirom.dat", 0, 0x23400, BAD_DUMP CRC(414096cb) SHA1(65175c8357bcfc043d312d19cf84ec4cae7369dc) )

	ROM_REGION( 0x400, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "m5l8041a-077p.bin", 0x000, 0x400, NO_DUMP ) // 8041 keyboard mcu, needs decapping
ROM_END

} // anonymous namespace


COMP( 1983, smc777, 0,      0,      smc777,  smc777, smc777_state, empty_init, "Sony",  "SMC-777", MACHINE_IMPERFECT_GRAPHICS )
