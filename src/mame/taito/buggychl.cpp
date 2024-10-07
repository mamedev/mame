// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria
/***************************************************************************

Buggy Challenge - (c) 1984 Taito Corporation

driver by Ernesto Corvi and Nicola Salmoria

TODO:
- I'm almost sure that I'm not handling the zoom x ROM table correctly. Gives
  reasonable results, though. I'm confident that the zoom y table handling is
  correct.
- Tilemap and sprite placement might not be accurate, there aren't many
  references.
- Video driver is largely unoptimized
- Support for the 7630's controlling the sound chip outputs (bass/treble,
  volume) is completely missing.
- The sound Z80 seems to write answers for the main Z80, but the latter doesn't
  seem to read them.
- videoram and spriteram garbage occurs when entering into cross hatch test and exiting.
  Game attempts to transfer content of videoram into spriteram/scrollram, then transfer
  back again into videoram. Maybe the host CPU cannot read contents of VRAM at all?

Known undumped versions:
- 4-channel version of the sound board for the cockpit cabinet (actually it is
  dumped, but the MAME driver update is WIP)
- Another ranking song version - ported to PS2 "Taito Memories".
  https://youtu.be/JXIvaM5508c?si=wCojMales2Kg9L7u&t=270 (Ranking 1st song)
  https://youtu.be/Btnl9FYTiL8?si=rrGTblIfOlIQo6NJ&t=308 (Ranking 2nd song)
  (perhaps modified for PS2 due to cover song copyright?)
- 2 player joystick version - after inserting a coin,
  instead of "START YOUR ENGINE", it displays "PUSH ONLY 1-PLAYER BUTTON".
  https://youtu.be/iE9-q4IruS4?si=eYL5_iXDZDyH9TS_&t=31
  (The title of this video states that this is the joystick version)
- Japanese version - after inserting a coin, instead of "START YOUR ENGINE",
  it displays "セレクトボタンをおしてね！！(Press select button!!)"
  and "クレジット(Credit)" instead of "CREDIT".
  https://youtu.be/gmC8Al1oCpo?si=JTO6eJ5Q_6-bj_NQ&t=6

Memory Map
----------
0000 - 3fff = ROM A22-04 (23)
4000 - 7fff = ROM A22-05 (22)
8000 - 87ff = RAM (36)
8800 - 8fff = RAM (35)

c800 - cbff = videoram
cc00 - cfff = videoram

d100 = /ANYOUT
    bit7 = lamp
    bit6 = lockout
    bit4 = OJMODE (sprite palette bank)
    bit3 = SKY OFF
    bit2 = /SN3OFF
    bit1 = flip screen X
    bit0 = flip screen Y
d200 = bank switch
    bit2 = Bank Select bit 1
    bit1 = Bank Select bit 0
    bit0 = EA13 (high/low part of banked ROM)
d300 = /TRESET (Watchdog reset?)
d301 = No name?
    bit6 = FLPF2 (W-6)
    bit5 = FLPE2 (W-5)
    bit4 = FLPD2 (W-4)
    bot2 = FLPF1 (W-3)
    bit1 = FLPE1 (W-2)
    bit0 = FLPD1 (W-1)
d302 - bit 0 = /RESET line on the 68705

d304 - d307 = SCCON1 to SCCON4

d613 = /SoundCS = /RESET line on all audio CPUs

d700 - d77f = ( /VCRRQ - palette ram )

d800 - d8ff /ScrollRQ (S37)
da00 - daff /ScrollRQ (S37)
db00 - dbff /ScrollRQ (S37)

dcxx = /SPOSI (S36)

2008-07
Dip locations and factory settings verified from dip listing

Clock information:
Xtal = 48mhz
QA output = "24M" = 24mhz
QB output = "1/2CLK" = 12mhz
QC output = "CLK" = 6mhz
"1/2phi" = 24M / 3 = 8mhz

The z80B main CPU is clocked by (depending on a jumper) either "1/2CLK"/2 OR "1/2PHI"/2,
so either 6mhz or 4mhz. Schematics show the jumper set to the 6mhz setting.

***************************************************************************/

#include "emu.h"

#include "taito68705.h"

#include "cpu/m6805/m6805.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/msm5232.h"
#include "sound/ta7630.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "buggychl.lh"


namespace {

class buggychl_state : public driver_device
{
public:
	buggychl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_charram(*this, "charram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scrollv(*this, "scrollv"),
		m_scrollh(*this, "scrollh"),
		m_mainbank(*this, "mainbank"),
		m_sprite_zoom_table(*this, "sprite_zoom_table"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_bmcu(*this, "bmcu"),
		m_ta7630(*this, "ta7630"),
		m_msm(*this, "msm"),
		m_ay(*this, "ay%u", 1U),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundnmi(*this, "soundnmi"),
		m_soundlatch(*this, "soundlatch%u", 1U),
		m_pedal_input(*this, "PEDAL"),
		m_led(*this, "led%u", 0U)
	{ }

	void buggychl(machine_config &config);

	ioport_value pedal_in_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_charram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrollv;
	required_shared_ptr<uint8_t> m_scrollh;
	required_memory_bank m_mainbank;
	required_region_ptr<uint8_t> m_sprite_zoom_table;

	// devices
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_maincpu;
	required_device<taito68705_mcu_device> m_bmcu;
	required_device<ta7630_device> m_ta7630;
	required_device<msm5232_device> m_msm;
	required_device_array<ay8910_device, 2> m_ay;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<input_merger_device> m_soundnmi;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_ioport m_pedal_input;

	output_finder<1> m_led;

	// video-related
	bitmap_ind16 m_dest_bitmap;
	bitmap_ind16 m_tmp_bitmap[2];
	uint16_t m_sl_bank = 0;
	uint8_t m_bg_clip_on = 0;
	uint8_t m_sky_on = 0;
	uint8_t m_sprite_color_base = 0;
	uint8_t m_bg_scrollx = 0;
	uint8_t m_sprite_lookup[0x2000];

	void bankswitch_w(uint8_t data);
	void sound_enable_w(uint8_t data);
	uint8_t mcu_status_r();
	uint8_t sound_status_main_r();
	uint8_t sound_status_sound_r();
	void chargen_w(offs_t offset, uint8_t data);
	void sprite_lookup_bank_w(uint8_t data);
	void sprite_lookup_w(offs_t offset, uint8_t data);
	void ctrl_w(uint8_t data);
	void bg_scrollx_w(uint8_t data);
	template <uint8_t Which> void ta7630_volbal_ay_w(uint8_t data);
	template <uint8_t Which> void port_b_w(uint8_t data);
	void ta7630_volbal_msm_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_bg(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_fg(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void buggychl_state::video_start()
{
	m_screen->register_screen_bitmap(m_dest_bitmap);
	m_screen->register_screen_bitmap(m_tmp_bitmap[0]);
	m_screen->register_screen_bitmap(m_tmp_bitmap[1]);
}

void buggychl_state::chargen_w(offs_t offset, uint8_t data)
{
	if (m_charram[offset] != data)
	{
		m_charram[offset] = data;
		m_gfxdecode->gfx(0)->mark_dirty((offset / 8) & 0xff);
	}
}

void buggychl_state::sprite_lookup_bank_w(uint8_t data)
{
	m_sl_bank = (data & 0x10) << 8;
}

void buggychl_state::sprite_lookup_w(offs_t offset, uint8_t data)
{
	m_sprite_lookup[offset + m_sl_bank] = data;
}

void buggychl_state::ctrl_w(uint8_t data)
{
	// bit0 = VINV
	// bit1 = HINV
	// bit2 = /SN3OFF
	// bit3 = SKY OFF
	// bit4 = OJMODE
	// bit6 = lockout
	// bit7 = lamp

	flip_screen_y_set(data & 0x01);
	flip_screen_x_set(data & 0x02);

	m_bg_clip_on = data & 0x04;
	m_sky_on = data & 0x08;

	m_sprite_color_base = (data & 0x10) ? 0x10 : 0x30;

	machine().bookkeeping().coin_lockout_global_w((~data & 0x40) >> 6);
	m_led[0] = BIT(~data, 7);
}

void buggychl_state::bg_scrollx_w(uint8_t data)
{
	m_bg_scrollx = data;
}


void buggychl_state::draw_bg(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// prevent wraparound
	rectangle clip = cliprect;

	// enable clipping if on (title screen disables this to cover all of the area)
	if (m_bg_clip_on)
	{
		if (flip_screen_x())
			clip.min_x += 8 * 8;
		else
			clip.max_x -= 8 * 8;

		clip &= cliprect;
	}

	for (int offs = 0; offs < 0x400; offs++)
	{
		int code = m_videoram[0x400 + offs];

		int sx = offs % 32;
		int sy = offs / 32;

		if (flip_screen_x())
			sx = 31 - sx;
		if (flip_screen_y())
			sy = 31 - sy;

		m_gfxdecode->gfx(0)->opaque(m_tmp_bitmap[0], m_tmp_bitmap[0].cliprect(),
				code,
				2,
				flip_screen_x(), flip_screen_y(),
				8 * sx, 8 * sy);
	}

	int32_t scroll[256];

	// first copy to a temp bitmap doing column scroll
	for (int offs = 0; offs < 256; offs++)
		scroll[offs] = -m_scrollv[offs / 8];

	int32_t scrollx = -(m_bg_scrollx - 0x12);

	copyscrollbitmap(m_tmp_bitmap[1], m_tmp_bitmap[0], 1, &scrollx, 256, scroll, m_tmp_bitmap[1].cliprect());

	// then copy to the destination bitmap doing row scroll
	for (int offs = 0; offs < 256; offs++)
		scroll[offs] = -m_scrollh[offs];

	copyscrollbitmap_trans(bitmap, m_tmp_bitmap[1], 256, scroll, 0, nullptr, clip, 0x20);
}


void buggychl_state::draw_fg(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x400; offs++)
	{
		int sx = offs % 32;
		int sy = offs / 32;
		int flipx = flip_screen_x();
		int flipy = flip_screen_y();

		int code = m_videoram[offs];

		if (flipx)
			sx = 31 - sx;
		if (flipy)
			sy = 31 - sy;

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				code,
				0,
				flipx, flipy,
				8 * sx, 8 * sy,
				0);
	}
}


void buggychl_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	auto profile = g_profiler.start(PROFILER_USER1);

	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int sx = m_spriteram[offs + 3] - ((m_spriteram[offs + 2] & 0x80) << 1);
		int sy = 256 - 64 - m_spriteram[offs] + ((m_spriteram[offs + 1] & 0x80) << 1);
		int flipy = m_spriteram[offs + 1] & 0x40;
		int zoom = m_spriteram[offs + 1] & 0x3f;
		uint8_t const *const zoomy_rom = &m_sprite_zoom_table[zoom << 6];
		uint8_t const *const zoomx_rom = &m_sprite_zoom_table[0x2000 + (zoom << 3)];

		uint8_t const *const lookup = m_sprite_lookup + ((m_spriteram[offs + 2] & 0x7f) << 6);

		for (int y = 0; y < 64; y++)
		{
			int dy = flip_screen_y() ? (255 - sy - y) : (sy + y);

			if (dy >= cliprect.min_y && dy <= cliprect.max_y)
			{
				int charline = zoomy_rom[y] & 0x07;
				int base_pos = zoomy_rom[y] & 0x38;
				if (flipy)
					base_pos ^= 0x38;

				int px = 0;
				for (int ch = 0; ch < 4; ch++)
				{
					int pos = base_pos + 2 * ch;
					int code = 8 * (lookup[pos] | ((lookup[pos + 1] & 0x07) << 8));
					int realflipy = (lookup[pos + 1] & 0x80) ? !flipy : flipy;
					code += (realflipy ? (charline ^ 7) : charline);
					uint8_t const *const pendata = m_gfxdecode->gfx(1)->get_data(code);

					for (int x = 0; x < 16; x++)
					{
						int col = pendata[x];
						if (col)
						{
							int dx = flip_screen_x() ? (255 - sx - px) : (sx + px);
							if (cliprect.contains(dx, dy))
								bitmap.pix(dy, dx) = m_sprite_color_base + col;
						}

						// the following line is almost certainly wrong
						if (zoomx_rom[7 - (2 * ch + x / 8)] & (1 << (x & 7)))
							px++;
					}
				}
			}
		}
	}
}


uint32_t buggychl_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	constexpr pen_t bgpen = 0x20;
	m_dest_bitmap.fill(bgpen, cliprect);

	draw_bg(m_dest_bitmap, cliprect);
	draw_sprites(m_dest_bitmap, cliprect);
	draw_fg(m_dest_bitmap, cliprect);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const pen_t pen = m_dest_bitmap.pix(y, x);
			uint32_t color = m_palette->pen(pen);

			// blend the sky gradient in
			if (m_sky_on && pen == bgpen)
			{
				int g = (color >> 8 & 0xff) + 0x200;
				g -= int8_t(m_scrollh[y]) + int8_t(m_bg_scrollx) + x;
				g = std::clamp(g / 2, 0, 0xff);

				color = (color & 0xff0000) | g << 8 | 0xff;
			}

			bitmap.pix(y, x) = color;
		}
	}

	return 0;
}


void buggychl_state::bankswitch_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x07); // shall we check if data & 7 < # banks?
}

void buggychl_state::sound_enable_w(uint8_t data)
{
	// does this really only control the sound irq 'timer' enable state, rather than the entire sound system?
	// this would be more in line with the (admittedly incorrect) schematic...
	// logerror("Sound_enable_w written with data of %02x\n", data);
	machine().sound().system_mute(!BIT(data, 0));
}

uint8_t buggychl_state::mcu_status_r()
{
	// bit 0 = when 1, MCU is ready to receive data from main CPU
	// bit 1 = when 1, MCU has sent data to the main CPU
	return
		((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x01 : 0x00) |
		((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x02 : 0x00);
}

// the schematics show that the two sound semaphore latch bits are actually flipped backwards when read by the sound CPU
//   vs when read by the main CPU.
//   Given the other schematic errors, and the fact that the sound board schematic is for the wrong PCB, is this even correct?
//   It isn't even obvious if the main CPU or sound CPU read the semaphores at all, ever.
// a CPU write to soundlatch[0] sets ic12.2 so /Q is low, so CPU bit 1 and sound bit 0 read as clear
// a sound write to soundlatch[1] clears ic12.1 so /Q is high, so CPU bit 0 and sound bit 1 read as set
// a CPU read of soundlatch[1] sets ic12.1 so /Q is low, so CPU bit 0 and sound bit 1 read as clear
// a sound read of soundlatch[0] clears ic12.2 so /Q is high, so CPU bit 1 and sound bit 0 read as set
// ic12.1 is set and ic12.2 is cleared by /SRESET
uint8_t buggychl_state::sound_status_main_r()
{
	return (m_soundlatch[1]->pending_r() ? 1 : 0) | (m_soundlatch[0]->pending_r() ? 0 : 2);
}

uint8_t buggychl_state::sound_status_sound_r()
{
	return (m_soundlatch[1]->pending_r() ? 2 : 0) | (m_soundlatch[0]->pending_r() ? 0 : 1);
}

/* Main CPU address map ( * = used within this section; x = don't care )
           |           |           |
15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 *  *                                             R  74LS139 @ ic53
 0  0  *  *  *  *  *  *  *  *  *  *  *  *  *  *   R  ROM (ic23)
 0  1  *  *  *  *  *  *  *  *  *  *  *  *  *  *   R  ROM (ic22)
 1  *  *  *                                       RW 74LS138 @ ic66
 1  0  0  0  0  *  *  *  *  *  *  *  *  *  *  *   RW SRAM (ic36)
 1  0  0  0  1  *  *  *  *  *  *  *  *  *  *  *   RW SRAM (ic35)
 1  0  0  1  *  *  *  *  *  *  *  *  *  *  *  *   RW  /STYLRQ
 (TODO: finish above, its quite complicated)
 1  0  1  *  *  *  *  *  *  *  *  *  *  *  *  *   R  /EXROMRD and /CDRRQW, banked ROM reads (generates a waitstate)
 1  1  0  0  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?   ?  (unknown, cut off on schematic)
 1  1  0  1  0  0  0  0  ?  ?  ?  ?  ?  ?  ?  ?   W  HORIZON
 1  1  0  1  0  0  0  1  x  x  x  x  x  x  x  x   W  ANY OUT (lamp d7, lockout d6, unused d5, ojmode d4, skyoff d3, sn31/4off d2, hinv d1, vinv d0)
 1  1  0  1  0  0  1  0  x  x  x  x  x  x  x  x   W  BANKSWITCH (banking, rom selected on d2, d1, upper/lower half of rom on d0)
 1  1  0  1  0  0  1  1  x  x  x  x  x  *  *  *   W  74LS138 @ ic39
 1  1  0  1  0  0  1  1  x  x  x  x  x  0  0  0   W  /TRESET (watchdog reset)
 1  1  0  1  0  0  1  1  x  x  x  x  x  0  0  1   W  FLPD1,E1,F1,D2,E2,F2 on d0-d5 respectively
 1  1  0  1  0  0  1  1  x  x  x  x  x  0  1  0   W  /SRESET (value of d0 latched; this is the mcu reset and resets the mcu semaphores as well)
 1  1  0  1  0  0  1  1  x  x  x  x  x  0  1  1   W  STYLBANK (d4 controls latch at v-ic25.1)
 1  1  0  1  0  0  1  1  x  x  x  x  x  1  0  0   W  SCCON1
 1  1  0  1  0  0  1  1  x  x  x  x  x  1  0  1   W  SCCON2
 1  1  0  1  0  0  1  1  x  x  x  x  x  1  1  0   W  SCCON3
 1  1  0  1  0  0  1  1  x  x  x  x  x  1  1  1   W  SCCON4
 (the four ports above are probably for connecting to a bezel score display, almost identical to that of Grand Champion; see
 https://archive.org/download/ArcadeGameManualGrandchampion/grandchampion.pdf pdf pages 54 and 55)
 1  1  0  1  0  1  0  0  x  x  x  x  x  x  *  *   RW  SEQRQ 74ls155 @ ic42
 1  1  0  1  0  1  0  0  x  x  x  x  x  x  0  0   W  Write to MCU in latch and set ic43.1  semaphore
 1  1  0  1  0  1  0  0  x  x  x  x  x  x  0  0   R  Read from MCU out latch and clear ic43.2 semaphore
 1  1  0  1  0  1  0  0  x  x  x  x  x  x  0  1   R  Read semaphores : /ic43.0 in d0 and ic43.1 in d1
 1  1  0  1  0  1  0  0  x  x  x  x  x  x  1  x   OPEN BUS
 1  1  0  1  0  1  0  1  *  *  *  *  *  *  *  *   RW  OBJRQ (read/write obj SRAM vb-ic34)
 1  1  0  1  0  1  1  0  x  x  x  *  *            W  74LS139 @ ic52
 1  1  0  1  0  1  1  0  x  x  x  0  0  x  *  *   R  INPUTA
 (16 inputs read here in the 4 bytes)
 1  1  0  1  0  1  1  0  x  x  x  0  1  x  *  *   R  INPUTB
 (16 inputs read here in the 4 bytes)
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  *  *   *  SOUNDCS 74ls155 @ s-ic40
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  0  0   R  Read from Sound out latch, set s-ic12.1
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  0  1   R  Read sound semaphores: /s-ic12.1 in d0 and /s-ic12.2 in d1
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  1  x   R  OPEN BUS
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  0  0   W  Write to Sound in latch, set s-ic12.2
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  0  1   W  OPEN BUS
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  1  0   W  OPEN BUS
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  1  1   W  SNDRESET (value of d0 latched; if high, this sets s-ic12.1, clears s-ic12.2, clears soundnmi enable,
 clears sound control latch, resets sound z80(s), zeroes all dac input latches, resets ay-3-8910 chips, and resets the waitstate request)
 1  1  0  1  0  1  1  0  x  x  x  1  1  x  x  x   W  ACCELCL
 1  1  0  1  0  1  1  1  x  x  *  *  *  *  *  *   RW /VCRRQ
 (TODO: palette sram)
 1  1  0  1  1  0  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?   ?  /SCROLRQ
 1  1  0  1  1  1  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?   ?  /S_POSI
 1  1  1  0  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?   ?  (unknown, cut off on schematic)
 1  1  1  1  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?   ?  (unknown, cut off on schematic)
*/
void buggychl_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom(); // A22-04 (23)
	map(0x4000, 0x7fff).rom(); // A22-05 (22)
	map(0x8000, 0x87ff).ram(); // 6116 SRAM (36)
	map(0x8800, 0x8fff).ram(); // 6116 SRAM (35)
	map(0x9000, 0x9fff).w(FUNC(buggychl_state::sprite_lookup_w));
	map(0xa000, 0xbfff).bankr(m_mainbank).w(FUNC(buggychl_state::chargen_w)).share(m_charram);
	map(0xc800, 0xcfff).ram().share(m_videoram);
	map(0xd000, 0xd000).nopw(); // ???
	map(0xd100, 0xd100).mirror(0x00ff).w(FUNC(buggychl_state::ctrl_w));
	map(0xd200, 0xd200).mirror(0x00ff).w(FUNC(buggychl_state::bankswitch_w));
	map(0xd300, 0xd300).mirror(0x00f8).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	// d301 = flp stuff, unused?
	// d302 = mcu reset latched d0
	map(0xd303, 0xd303).mirror(0x00f8).w(FUNC(buggychl_state::sprite_lookup_bank_w));
	map(0xd304, 0xd307).nopw(); // d304-d307 is SCCON, which seems to be for a bezel mounted 7seg score/time display like Grand Champion has
	map(0xd400, 0xd400).mirror(0x00fc).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0xd401, 0xd401).mirror(0x00fc).r(FUNC(buggychl_state::mcu_status_r));
	map(0xd500, 0xd57f).writeonly().share(m_spriteram);
	map(0xd600, 0xd600).mirror(0x00e4).portr("DSW1");
	map(0xd601, 0xd601).mirror(0x00e4).portr("DSW2");
	map(0xd602, 0xd602).mirror(0x00e4).portr("DSW3");
	map(0xd603, 0xd603).mirror(0x00e4).portr("IN0"); // player inputs
	map(0xd608, 0xd608).mirror(0x00e4).portr("WHEEL");
	map(0xd609, 0xd609).mirror(0x00e4).portr("IN1"); // coin + accelerator
//  map(0xd60a, 0xd60a).mirror(0x00e4); // other inputs, not used?
//  map(0xd60b, 0xd60b).mirror(0x00e4); // other inputs, not used?
	map(0xd610, 0xd610).mirror(0x00e4).r(m_soundlatch[1], FUNC(generic_latch_8_device::read)).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
	map(0xd611, 0xd611).mirror(0x00e4).r(FUNC(buggychl_state::sound_status_main_r));
//  map(0xd613, 0xd613).mirror(0x00e4).w(FUNC(buggychl_state::sound_reset_w));
	map(0xd618, 0xd618).mirror(0x00e7).nopw(); // accelerator clear; TODO: should we emulate the proper quadrature counter here?
	map(0xd700, 0xd77f).w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xd820, 0xd83f).ram(); // TODO
	map(0xd840, 0xd85f).writeonly().share(m_scrollv);
	map(0xdb00, 0xdbff).writeonly().share(m_scrollh);
	map(0xdc04, 0xdc04).nopw(); // should be fg scroll
	map(0xdc06, 0xdc06).w(FUNC(buggychl_state::bg_scrollx_w));
}

/* The schematics for Buggy Challenge have the wrong sound board schematic attached to it.
  (The schematic is for an unknown Taito game, possibly never released.)
   The final Buggy Challenge sound board is more similar to Fairyland Story sound
   hardware, except it has two YM2149 chips instead of one, and much less ROM space. */
void buggychl_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x4800, 0x4801).w(m_ay[0], FUNC(ay8910_device::address_data_w));
	map(0x4802, 0x4803).w(m_ay[1], FUNC(ay8910_device::address_data_w));
	map(0x4810, 0x481d).w(m_msm, FUNC(msm5232_device::write));
	map(0x4820, 0x4820).w(FUNC(buggychl_state::ta7630_volbal_msm_w)); // VOL/BAL   for the 7630 on the MSM5232 output
	map(0x4830, 0x4830).ram(); // TRBL/BASS for the 7630 on the MSM5232 output
	map(0x5000, 0x5000).r(m_soundlatch[0], FUNC(generic_latch_8_device::read)).w(m_soundlatch[1], FUNC(generic_latch_8_device::write));
	map(0x5001, 0x5001).r(FUNC(buggychl_state::sound_status_sound_r)).w(m_soundnmi, FUNC(input_merger_device::in_set<1>));
	map(0x5002, 0x5002).w(m_soundnmi, FUNC(input_merger_device::in_clear<1>));
	map(0x5003, 0x5003).w(FUNC(buggychl_state::sound_enable_w)); // unclear what this actually controls
	map(0xe000, 0xefff).rom(); // space for diagnostics ROM
}

/* Here is the memory maps from the 'wrong' sound schematic
THIS DOES NOT MATCH THE ACTUAL HARDWARE, but seems to be of the hardware from
which the final sound board design was derived from (as well as flstory/40love and msisaac hw, and possibly retofinv).
Sound Master CPU (SMCPU)
           |           |           |
15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 *  *  *                                          R  74LS138 @ ic17
 0  0  0  *  *  *  *  *  *  *  *  *  *  *  *  *   R  2764 ROM (ic11)
 0  0  1  *  *  *  *  *  *  *  *  *  *  *  *  *   R  2764 ROM (ic19)
 0  1  0  *  *  *  *  *  *  *  *  *  *  *  *  *   R  2764 ROM (ic4)
 0  1  1                                          SM-6 /CS
 0  1  1  0  *  *                                 RW 74ls155 @ ic14
 0  1  1  0  0  0  x  x  x  x  x  x  x  x  x  x   R  Read sound latch from maincpu, clear semaphore
 0  1  1  0  0  0  x  x  x  x  x  x  x  x  x  x   W  Enable sound NMI from sound latch semaphore
 0  1  1  0  0  1  x  x  x  x  x  x  x  x  x  x   R  Read sound semaphores
 0  1  1  0  0  1  x  x  x  x  x  x  x  x  x  x   W  Disable sound NMI from sound latch semaphore
 0  1  1  0  1  0  x  x  x  x  x  x  x  x  x  x   R  Read 3 ?debug? bits from edge connector pins in d0-d2
 0  1  1  0  1  0  x  x  x  x  x  x  x  x  x  x   W  SM-INT CTL:
                                                     D0: if 0, enable SM CPU INT on timer (((soundclock(4MHz)/2)/256)/(128 or 256)) = 60hz or 30hz (or 120hz?)
                                                     D1: switch speed of timer: 0 = 30.51757hz, 1 = 61.0351hz (or is this 61/122hz?)
                                                     D2: if 0, enable SS CPU int to SM CPU
                                                     D3: connects to SS CPU /RESET
                                                     D4: not used
                                                     D5: not used
                                                     D6: gate ay2 to left channels
                                                     D7: gate ay2 to right channels
 0  1  1  0  1  1  x  x  x  x  x  x  x  x  x  x   R  OPEN BUS
 0  1  1  0  1  1  x  x  x  x  x  x  x  x  x  x   W  Write to sound latch 2 to maincpu, set semaphore
 0  1  1  1  *  *  *                              W  74LS138 @ ic23
 0  1  1  1  0  0  0  x  x  x  x  x  x  x  x  x   W  Slave Sound CPU IntReq
 0  1  1  1  0  0  1  x  x  x  x  x  x  x  x  x   W  OPEN BUS
 0  1  1  1  0  1  0  x  x  x  x  x  x  x  x  x   W  Main VR Control voltage DAC
 0  1  1  1  0  1  1  x  x  x  x  x  x  x  x  x   W  OPEN BUS
 0  1  1  1  1  0  0  x  x  x  x  x  x  x  x  x   W  CHA Level (selectively gate the 4 DAC bits for 2x 4bit r2r DAC, one on d7-4, one d3-0, for front right)
 0  1  1  1  1  0  1  x  x  x  x  x  x  x  x  x   W  CHB Level (selectively gate the 4 DAC bits for 2x 4bit r2r DAC, one on d7-4, one d3-0, for rear right)
 0  1  1  1  1  1  0  x  x  x  x  x  x  x  x  x   W  CHC Level (selectively gate the 4 DAC bits for 2x 4bit r2r DAC, one on d7-4, one d3-0, for front left)
 0  1  1  1  1  1  1  x  x  x  x  x  x  x  x  x   W  CHD Level (selectively gate the 4 DAC bits for 2x 4bit r2r DAC, one on d7-4, one d3-0, for rear left)
 1  0  0  x  x  x  x  x  x  x  x  x  x  *  *  *   SM-8 /CS (inject ?4? SM waitstates cycles)
 1  0  0  x  x  x  x  x  x  x  x  x  x  0  0  x   W  OPEN BUS
 1  0  0  x  x  x  x  x  x  x  x  x  x  0  1  0   W  AY #1 @ic42 Address write
 1  0  0  x  x  x  x  x  x  x  x  x  x  0  1  1   W  AY #1 @ic42 Data write
                                                     AY #1 IOB7-4 connect to an r2r DAC+opamp controlling ay1 TA7630P Treble
                                                     AY #1 IOB3-0 connect to an r2r DAC+opamp controlling ay1 TA7630P Bass
                                                     AY #1 IOA7-4 connect to an r2r DAC+opamp controlling ay1 TA7630P Volume
                                                     AY #1 IOA3-0 connect to an r2r DAC+opamp controlling ay1 TA7630P Balance
 1  0  0  x  x  x  x  x  x  x  x  x  x  1  0  0   W  AY #2 @ic41 Address write
 1  0  0  x  x  x  x  x  x  x  x  x  x  1  0  1   W  AY #2 @ic41 Data write
                                                     AY #2 IOB7-4 connect to an r2r DAC+opamp controlling ay2 TA7630P Treble
                                                     AY #2 IOB3-0 connect to an r2r DAC+opamp controlling ay2 TA7630P Bass
                                                     AY #2 IOA7-4 connect to an r2r DAC+opamp controlling ay2 TA7630P Volume
                                                     AY #2 IOA3-0 connect to an r2r DAC+opamp controlling ay2 TA7630P Balance
 1  0  0  x  x  x  x  x  x  x  x  x  x  1  1  x   W  OPEN BUS
 1  0  1  *  *  *  *  *  *  *  *  *  *  *  *  *   RW SM-A /CS (read or write to slave CPU address space 0000-1fff; slave CPU is held in waitstate during this)
 1  1  0  x  x  *  *  *  *  *  *  *  *  *  *  *   RW SRAM (ic3)
 1  1  1  x  x  x  x  x  x  x  x  x  x  x  x  x   OPEN BUS (diag ROM may map here?)
*/

/******************************************************************************/

// accelerator is 4-bit, we need to convert it here so that it doesn't clash with other inputs in IN1 (known i/o framework fault)
ioport_value buggychl_state::pedal_in_r()
{
	return m_pedal_input->read() >> 4;
}


// defaults verified with manual, and yes: all ON for bank 2 while all OFF for banks 1/3
static INPUT_PORTS_START( buggychl )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Game Over Bonus" ) PORT_DIPLOCATION("SW1:1,2")   // Arks/Flags/Fuel
	PORT_DIPSETTING(    0x03, "2000/1000/50" )
	PORT_DIPSETTING(    0x02, "1000/500/30" )
	PORT_DIPSETTING(    0x01, "500/200/10" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )         // 1300 units of fuel
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )       // 1200 units of fuel
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )         // 1100 units of fuel
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )      // 1000 units of fuel
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )        // Only listed as OFF in the manual

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Start button needed" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )        // Only listed as OFF in the manual
	PORT_DIPNAME( 0x04, 0x04, "Fuel loss (Cheat)") PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Crash only" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )        // Only listed as OFF in the manual
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_TOGGLE PORT_NAME("P1 Gear Shift")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Test Button") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(buggychl_state, pedal_in_r)

	PORT_START("PEDAL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_NAME("P1 Pedal") PORT_SENSITIVITY(100) PORT_KEYDELTA(15)   // accelerator

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	256,
	4,
	{ 3*0x800*8, 2*0x800*8, 0x800*8, 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,1,
	RGN_FRAC(1,8),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ RGN_FRAC(1,8)+7, RGN_FRAC(1,8)+6, RGN_FRAC(1,8)+5, RGN_FRAC(1,8)+4, RGN_FRAC(1,8)+3, RGN_FRAC(1,8)+2, RGN_FRAC(1,8)+1, RGN_FRAC(1,8)+0,
			7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0 },
	8
};

static GFXDECODE_START( gfx_buggychl )
	GFXDECODE_RAM(   "charram", 0, charlayout,   0, 8 ) // decoded at runtime
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0, 8 ) // sprites are drawn pixel by pixel by draw_sprites()
GFXDECODE_END


void buggychl_state::ta7630_volbal_msm_w(uint8_t data)
{
	m_ta7630->set_device_volume(m_msm, data >> 4);
}

template <uint8_t Which>
void buggychl_state::ta7630_volbal_ay_w(uint8_t data)
{
	// VOL/BAL for the 7630 on this 8910 output
	m_ta7630->set_device_volume(m_ay[Which], data >> 4);
}

template <uint8_t Which>
void buggychl_state::port_b_w(uint8_t data)
{
	// TRBL/BASS for the 7630 on this 8910 output
}


void buggychl_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();
	m_mainbank->configure_entries(0, 6, &ROM[0x10000], 0x2000);

	memset(m_sprite_lookup, 0, sizeof(m_sprite_lookup));

	save_item(NAME(m_sprite_lookup));
	save_item(NAME(m_sl_bank));
	save_item(NAME(m_bg_clip_on));
	save_item(NAME(m_sky_on));
	save_item(NAME(m_sprite_color_base));
	save_item(NAME(m_bg_scrollx));

	m_led.resolve();
}

void buggychl_state::buggychl(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 48_MHz_XTAL / 8); // 6 MHz according to schematics, though it can be jumpered for 4MHz as well
	m_maincpu->set_addrmap(AS_PROGRAM, &buggychl_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(buggychl_state::irq0_line_hold));

	Z80(config, m_audiocpu, 8_MHz_XTAL / 2); // 4 MHz according to schematics
	m_audiocpu->set_addrmap(AS_PROGRAM, &buggychl_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(buggychl_state::irq0_line_hold), attotime::from_hz(8_MHz_XTAL / 2 / 2 / 256 / 64)); // timer IRQ

	//TIMER(config, "soundirq").configure_periodic(m_audiocpu, FUNC(buggychl_state::irq0_line_hold), 8_MHz_XTAL / 2 / 2 / 256 / 64);
	// The schematics (which are at least partly for the wrong sound board) show a configurable timer with rates of
	// 61.035Hz (8_MHz_XTAL / 2 / 2 / 256 / 128)
	// or 122.0Hz (8_MHz_XTAL / 2 / 2 / 256 / 64)
	// similar to flstory.cpp and other Taito MSM5232 based games.
	// The real sound PCB probably lacks the latch for this configurable timer, but does have a jumper which likely has a similar function.
	// The game code implies the timer int is enabled/disabled by one of the "sound_enable_w" bits?
	// TODO: actually hook this up?
	// audiocpu NMI is caused by (main->sound semaphore)&&(sound_nmi_enabled), identical to Bubble Bobble.

	TAITO68705_MCU(config, m_bmcu, 48_MHz_XTAL / 8 / 2); // CPUspeed/2 MHz according to schematics, so 3MHz if CPU is jumpered for 6MHz

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 128); // typical Taito 74ls392

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	// derived from ladyfrog.cpp, causes glitches?
	//m_screen->set_raw(8_MHz_XTAL, 510, 0, 256, 262, 2*8, 30*8); // pixel clock appears to run at 8 MHz
	m_screen->set_screen_update(FUNC(buggychl_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_buggychl);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 64);
	m_palette->set_endianness(ENDIANNESS_BIG);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]).data_pending_callback().set(m_soundnmi, FUNC(input_merger_device::in_w<0>));
	GENERIC_LATCH_8(config, m_soundlatch[1]);

	INPUT_MERGER_ALL_HIGH(config, m_soundnmi).output_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	TA7630(config, m_ta7630);

	YM2149(config, m_ay[0], 8_MHz_XTAL / 4);
	m_ay[0]->port_a_write_callback().set(FUNC(buggychl_state::ta7630_volbal_ay_w<0>));
	m_ay[0]->port_b_write_callback().set(FUNC(buggychl_state::port_b_w<0>));
	m_ay[0]->add_route(ALL_OUTPUTS, "mono", 0.50);

	YM2149(config, m_ay[1], 8_MHz_XTAL / 4);
	m_ay[1]->port_a_write_callback().set(FUNC(buggychl_state::ta7630_volbal_ay_w<1>));
	m_ay[1]->port_b_write_callback().set(FUNC(buggychl_state::port_b_w<1>));
	m_ay[1]->add_route(ALL_OUTPUTS, "mono", 0.50);

	MSM5232(config, m_msm, 8_MHz_XTAL / 4);
	m_msm->set_capacitors(1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6); // default 1 uF capacitors (not verified)
	m_msm->add_route(0, "mono", 1.0);   // pin 28  2'-1
	m_msm->add_route(1, "mono", 1.0);   // pin 29  4'-1
	m_msm->add_route(2, "mono", 1.0);   // pin 30  8'-1
	m_msm->add_route(3, "mono", 1.0);   // pin 31 16'-1
	m_msm->add_route(4, "mono", 1.0);   // pin 36  2'-2
	m_msm->add_route(5, "mono", 1.0);   // pin 35  4'-2
	m_msm->add_route(6, "mono", 1.0);   // pin 34  8'-2
	m_msm->add_route(7, "mono", 1.0);   // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( buggychl )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "a22-04-2.23", 0x00000, 0x4000, CRC(16445a6a) SHA1(5ce7b0b1aeb3b6cd400965467f913558f39c251f) )
	ROM_LOAD( "a22-05-2.22", 0x04000, 0x4000, CRC(d57430b2) SHA1(3e5b8c21a342d8e26c12a78535748073bc5b8742) )
	ROM_LOAD( "a22-01.3",    0x10000, 0x4000, CRC(af3b7554) SHA1(fd4f5a6cf9253f64c7e86d566802a02baae3b379) ) // banked
	ROM_LOAD( "a22-02.2",    0x14000, 0x4000, CRC(b8a645fb) SHA1(614a0656dee0cfa1d7e16ec1e0138a423ecaf18b) ) // banked
	ROM_LOAD( "a22-03.1",    0x18000, 0x4000, CRC(5f45d469) SHA1(3a1b9ab2d57c06bfffb1271583944c90d3f6b5a2) ) // banked

	ROM_REGION( 0x10000, "audiocpu", 0 )  // Z80
	ROM_LOAD( "a22-24.28",   0x00000, 0x4000, CRC(1e7f841f) SHA1(2dc0787b08d32acb78291b689c02dbb83d04d08c) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )
	ROM_LOAD( "a22-19.31",   0x00000, 0x0800, CRC(06a71df0) SHA1(28183e6769e1471e7f28dc2a9f5b54e14b7ef339) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "a22-06.111",  0x00000, 0x4000, CRC(1df91b17) SHA1(440d33bf984042fb4eac8f17bb385992ccdc6113) )
	ROM_LOAD( "a22-07.110",  0x04000, 0x4000, CRC(2f0ab9b7) SHA1(07b98e23d12da834d522e29fe7891503dc258b05) )
	ROM_LOAD( "a22-08.109",  0x08000, 0x4000, CRC(49cb2134) SHA1(f9998617c097b90be7257ba6fc1e46ff9e1f8916) )
	ROM_LOAD( "a22-09.108",  0x0c000, 0x4000, CRC(e682e200) SHA1(3e2b5dd97e4f522f208d331f6903c69d49555b61) )
	ROM_LOAD( "a22-10.107",  0x10000, 0x4000, CRC(653b7e25) SHA1(70c69288438caf6725c6d96ff75cdc82af005b2b) )
	ROM_LOAD( "a22-11.106",  0x14000, 0x4000, CRC(8057b55c) SHA1(9eeb4980cb14fb1c9b6f3aeff4e0aef1338fc71c) )
	ROM_LOAD( "a22-12.105",  0x18000, 0x4000, CRC(8b365b24) SHA1(a306c1f6fe1f5563602ab424f1b4f6ac17d1e47d) )
	ROM_LOAD( "a22-13.104",  0x1c000, 0x4000, CRC(2c6d68fe) SHA1(9e1a0e44ae2b9986d0ebff49a0fd4df3e8a7f4e7) )

	ROM_REGION( 0x4000, "sprite_zoom_table", 0 )
	ROM_LOAD( "a22-14.59",   0x0000, 0x2000, CRC(a450b3ef) SHA1(42646bfaed19ea01ffe06996bb6c2fd6c70076d6) ) // vertical
	ROM_LOAD( "a22-15.115",  0x2000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) ) // horizontal
	ROM_LOAD( "a22-16.116",  0x3000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) ) // horizontal
ROM_END

ROM_START( buggychlt )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "bu04.bin",    0x00000, 0x4000, CRC(f90ab854) SHA1(d4536c98be35de3d888548e2de15f8435ca4f08c) )
	ROM_LOAD( "bu05.bin",    0x04000, 0x4000, CRC(543d0949) SHA1(b7b0b0319f5376e7cfcfd0e8a4fa6fea566e0206) )
	ROM_LOAD( "a22-01.3",    0x10000, 0x4000, CRC(af3b7554) SHA1(fd4f5a6cf9253f64c7e86d566802a02baae3b379) ) // banked
	ROM_LOAD( "a22-02.2",    0x14000, 0x4000, CRC(b8a645fb) SHA1(614a0656dee0cfa1d7e16ec1e0138a423ecaf18b) ) // banked
	ROM_LOAD( "a22-03.1",    0x18000, 0x4000, CRC(5f45d469) SHA1(3a1b9ab2d57c06bfffb1271583944c90d3f6b5a2) ) // banked

	ROM_REGION( 0x10000, "audiocpu", 0 )  // Z80
	ROM_LOAD( "a22-24.28",   0x00000, 0x4000, CRC(1e7f841f) SHA1(2dc0787b08d32acb78291b689c02dbb83d04d08c) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )
	ROM_LOAD( "a22-19.31",   0x00000, 0x0800, CRC(06a71df0) SHA1(28183e6769e1471e7f28dc2a9f5b54e14b7ef339) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "a22-06.111",  0x00000, 0x4000, CRC(1df91b17) SHA1(440d33bf984042fb4eac8f17bb385992ccdc6113) )
	ROM_LOAD( "a22-07.110",  0x04000, 0x4000, CRC(2f0ab9b7) SHA1(07b98e23d12da834d522e29fe7891503dc258b05) )
	ROM_LOAD( "a22-08.109",  0x08000, 0x4000, CRC(49cb2134) SHA1(f9998617c097b90be7257ba6fc1e46ff9e1f8916) )
	ROM_LOAD( "a22-09.108",  0x0c000, 0x4000, CRC(e682e200) SHA1(3e2b5dd97e4f522f208d331f6903c69d49555b61) )
	ROM_LOAD( "a22-10.107",  0x10000, 0x4000, CRC(653b7e25) SHA1(70c69288438caf6725c6d96ff75cdc82af005b2b) )
	ROM_LOAD( "a22-11.106",  0x14000, 0x4000, CRC(8057b55c) SHA1(9eeb4980cb14fb1c9b6f3aeff4e0aef1338fc71c) )
	ROM_LOAD( "a22-12.105",  0x18000, 0x4000, CRC(8b365b24) SHA1(a306c1f6fe1f5563602ab424f1b4f6ac17d1e47d) )
	ROM_LOAD( "a22-13.104",  0x1c000, 0x4000, CRC(2c6d68fe) SHA1(9e1a0e44ae2b9986d0ebff49a0fd4df3e8a7f4e7) )

	ROM_REGION( 0x4000, "sprite_zoom_table", 0 )
	ROM_LOAD( "a22-14.59",   0x0000, 0x2000, CRC(a450b3ef) SHA1(42646bfaed19ea01ffe06996bb6c2fd6c70076d6) ) // vertical
	ROM_LOAD( "a22-15.115",  0x2000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) ) // horizontal
	ROM_LOAD( "a22-16.116",  0x3000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) ) // horizontal
ROM_END

} // anonymous namespace


GAMEL( 1984, buggychl,  0,        buggychl, buggychl, buggychl_state, empty_init, ROT270, "Taito Corporation",                  "Buggy Challenge",          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE, layout_buggychl )
GAMEL( 1984, buggychlt, buggychl, buggychl, buggychl, buggychl_state, empty_init, ROT270, "Taito Corporation (Tecfri license)", "Buggy Challenge (Tecfri)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE, layout_buggychl )
