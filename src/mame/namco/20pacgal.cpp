// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Pacman - 25th Anniversary Edition
    Ms.Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion

    driver by Nicola Salmoria

    Notes for Ms.Pac-Man/Galaga - 20th Anniversary:
        * There are four start buttons: the first two are for Ms. Pac-Man, the other two
          are for Galaga.
        * To play Pac-Man instead of Ms. Pac-Man, insert coins then enter the following
          sequence: U U U D D D L R L R L. A sound will play and the ghost will change
          from red to pink. Then press the start button for Ms. Pac-Man.
        * To toggle the built-in speedup, insert coins then enter the following sequence:
          L R L R U U U Fire.  A sound will play if you did it correctly.  This will toggle
          the speed in both Ms Pacman & Pacman as well as provide a "Fast Shot" in Galaga
        * Writes to the Z180 ASCI port:
          MS PAC-MAN/GALAGA
          arcade video system
          version 1.01
          (c) 2000 Cosmodog, Ltd.
          >
          and it listens for incoming characters.

    Notes for Pacman - 25th Anniversary:
        * Pacman 25th Anniversary is a program update which allows the player to choose the
          game they want to play. You highlight the wanted game and then press the 1 or 2
          player start button to start the game.
        * There is a minor board difference that the program code can detect through the Z80
          ports to prevent ROM swaps to upgrade Ms. Pac-Man/Galaga - 20th Anniversary Class
          of 1981 Reunion boards.
        * The above listed joystick maneuver for the built-in speed up still works.
        * The above listed joystick maneuver to enable Pac-Man will still play a tone, but
          the effect (if any) is unknown.

    Note: The "correct" size of the roms are 27C020 for the program rom and 27C256 for the
          graphics rom.  However genuine boards have been found with larger roms containing
          the same data with the extra rom space blanked out.

    Known issues/to-do's:
        * Starfield is not 100% accurate
        * Music tempo of 20pacgalr4 and newer is is slightly slower, or is this normal?
        * Check the ASCI interface, there probably is fully working debug code.
        * Galaga attract mode isn't correct. At the score information screen, enemies are
          supposed to dive down like on the original Galaga. CPU opcode bug?
          Reference: https://youtu.be/OQyWaN9fTgw?t=2m33s


+-------------------------------------------------------+
|                        +-------------+                |
|                        |     U13     |                |
|                        |ms pac/galaga|                |
|                        +-------------+                |
|                         +-----------+                 |
|        +---+            |           |                 |
|        |VOL|            |   ZiLOG   |                 |
|        +---+            |Z8S18020VSC|                 |
|                         | Z180 MPU  |                 |
|J                        |           |                 |
|A                        +-----------+                 |
|M              +-----------+      +-----------+        |
|M              |           |  OSC |           |        |
|A              |CY37256P160|      |CY37256P160|        |
|               |-83AC      |      |-83AC      |        |
|               |           |   :: |           |   +---+|
|               |           |   :: |           |   | C ||
|               +-----------+   J1 +-----------+   | 1 ||
|                 93LC46A                          | 9 ||
|         +-------------+              +-------+   | 9 ||
|         |     U14     |   +-------+  |CY7C199|   +---+|
|         |ms pac/galaga|   |CY7C199|  +-------+        |
|  D4     +-------------+   +-------+                   |
+-------------------------------------------------------+

     CPU: Z8S18020VSC ZiLOG Z180 (20MHz part) at 18.432MHz
Graphics: CY37256P160-83AC x 2 (Ultra37000 CPLD family - 160 pin TQFP, 256 Macrocells, 83MHz speed)
  MEMORY: CY7C199-15VC 32K x 8 Static RAM x 3 (or equivalent ISSI IS61C256AH-15J)
     OSC: 73.728MHz
  EEPROM: 93LC46A 128 x 8-bit 1K microwire compatible Serial EEPROM
     VOL: Volume adjust
      D4: Diode - Status light
      J1: 10 pin JTAG interface for programming the CY37256P160 CPLDs

   HSync: 15.5174kHz
   VSync: 60.60175Hz

    The DAC (of the Namco sound chip, apparently combined with the generic DAC)
    is the same as on the old Pac-Man hardware, controlled by 8 lines from a
    CPLD (4 for sound, 4 for volume).

***************************************************************************/

#include "emu.h"
#include "emupal.h"

#include "bus/rs232/rs232.h"
#include "cpu/z180/z180.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/watchdog.h"
#include "sound/dac.h"
#include "sound/namco.h"

#include "screen.h"
#include "speaker.h"

namespace {

class _20pacgal_state : public driver_device
{
public:
	_20pacgal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_char_gfx_ram(*this, "char_gfx_ram"),
		m_stars_seed(*this, "stars_seed"),
		m_stars_ctrl(*this, "stars_ctrl"),
		m_flip(*this, "flip"),
		m_proms(*this, "proms"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_rs232(*this, "rs232")
	{ }

	void _20pacgal(machine_config &config);

	void init_25pacman();
	void init_20pacgal();

protected:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_char_gfx_ram;
	required_shared_ptr<uint8_t> m_stars_seed;
	required_shared_ptr<uint8_t> m_stars_ctrl;
	required_shared_ptr<uint8_t> m_flip;

	optional_memory_region m_proms;
	optional_memory_bank m_mainbank;

	/* machine state */
	uint8_t m_game_selected = 0;  /* 0 = Ms. Pac-Man, 1 = Galaga */

	/* devices */
	required_device<z180_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<palette_device> m_palette;
	required_device<dac_8bit_r2r_device> m_dac;
	required_device<rs232_port_device> m_rs232;

	/* memory */
	std::unique_ptr<uint8_t[]> m_sprite_gfx_ram;
	std::unique_ptr<uint8_t[]> m_sprite_ram;
	std::unique_ptr<uint8_t[]> m_sprite_color_lookup;
	std::unique_ptr<uint8_t[]> m_ram_48000;

	/* 25pacman and 20pacgal store the sprite palette at a different address, this is a hardware difference and confirmed NOT to be a register */
	uint8_t m_sprite_pal_base = 0;

	uint8_t m_irq_mask = 0;
	void irqack_w(uint8_t data);
	void timer_pulse_w(uint8_t data);
	void coin_counter_w(uint8_t data);
	void ram_bank_select_w(uint8_t data);
	void ram_48000_w(offs_t offset, uint8_t data);
	void sprite_gfx_w(offs_t offset, uint8_t data);
	void sprite_ram_w(offs_t offset, uint8_t data);
	void sprite_lookup_w(offs_t offset, uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_20pacgal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void starpal_init(palette_device &palette) const;
	void get_pens();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_chars(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, uint8_t code, uint8_t color, int flip_y, int flip_x);
	void common_save_state();

	void _20pacgal_io_map(address_map &map) ATTR_COLD;
	void _20pacgal_map(address_map &map) ATTR_COLD;
};


class _25pacman_state : public _20pacgal_state
{
public:
	_25pacman_state(const machine_config &mconfig, device_type type, const char *tag) :
		_20pacgal_state(mconfig, type, tag)
	{ }

	void _25pacman(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	void _25pacman_io_map(address_map &map) ATTR_COLD;
	void _25pacman_map(address_map &map) ATTR_COLD;
};

/*************************************
 *
 *  Interrupt system
 *
 *************************************/

void _20pacgal_state::irqack_w(uint8_t data)
{
	m_irq_mask = data & 1;

	if (!m_irq_mask)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void _20pacgal_state::timer_pulse_w(uint8_t data)
{
	//printf("timer pulse %02x\n", data);
}


/*************************************
 *
 *  Coin counter
 *
 *************************************/

void _20pacgal_state::coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
}



/*************************************
 *
 *  ROM banking
 *
 *************************************/

void _20pacgal_state::ram_bank_select_w(uint8_t data)
{
	if (m_game_selected != (data & 1))
	{
		m_game_selected = data & 1;
		m_mainbank->set_entry(m_game_selected);
		get_pens();
	}
}

void _20pacgal_state::ram_48000_w(offs_t offset, uint8_t data)
{
	if (m_game_selected)
	{
		if (offset < 0x0800)
			m_video_ram[offset & 0x07ff] = data;

		m_ram_48000[offset] = data;
	}
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void _20pacgal_state::sprite_gfx_w(offs_t offset, uint8_t data)
{
	m_sprite_gfx_ram[offset] = data;
}

void _20pacgal_state::sprite_ram_w(offs_t offset, uint8_t data)
{
	m_sprite_ram[offset] = data;
}

void _20pacgal_state::sprite_lookup_w(offs_t offset, uint8_t data)
{
	m_sprite_color_lookup[offset] = data;
}


/*************************************
 *
 *  Palette handling
 *
 *************************************/

void _20pacgal_state::get_pens()
{
	// TODO: Accurate palette when prom doesn't exist
	uint8_t const *color_prom = m_proms->base() + (0x1000 * m_game_selected);

	for (offs_t offs = 0; offs < 0x1000; offs++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		m_palette->set_pen_color(offs, rgb_t(r, g, b));

		color_prom++;
	}
}

void _20pacgal_state::starpal_init(palette_device &palette) const
{
	// palette for the stars
	for (offs_t offs = 0; offs < 64; offs++)
	{
		static constexpr int map[4] = { 0x00, 0x47, 0x97 ,0xde };

		int const r = map[(offs >> 0) & 0x03];
		int const g = map[(offs >> 2) & 0x03];
		int const b = map[(offs >> 4) & 0x03];

		palette.set_pen_color(0x1000 + offs, rgb_t(r, g, b));
	}
}


/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

void _20pacgal_state::draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x,
		uint8_t code, uint8_t color, int flip_y, int flip_x)
{
	offs_t pen_base = (color & 0x1f) << 2;
	pen_base += m_sprite_pal_base;

	if (flip_y)
		y = y + 0x0f;

	if (flip_x)
		x = x + 0x0f;

	// for each row in the sprite
	for (int sy = 0; sy < 0x10; sy++)
	{
		int x_sav = x;

		if ((y >= cliprect.min_y) && (y <= cliprect.max_y))
		{
			offs_t gfx_offs = ((code & 0x7f) << 6) | (sy << 2);

			// address mangling
			gfx_offs = (gfx_offs & 0x1f83) | ((gfx_offs & 0x003c) << 1) | ((gfx_offs & 0x0040) >> 4);

			uint32_t data = (m_sprite_gfx_ram[gfx_offs + 0] << 24) |
					(m_sprite_gfx_ram[gfx_offs + 1] << 16) |
					(m_sprite_gfx_ram[gfx_offs + 2] << 8) |
					(m_sprite_gfx_ram[gfx_offs + 3] << 0);

			// for each pixel in the row
			for (int sx = 0; sx < 0x10; sx++)
			{
				if ((x >= cliprect.min_x) && (x <= cliprect.max_x))
				{
					offs_t pen = (data & 0xc0000000) >> 30;
					uint8_t col;

					col = m_sprite_color_lookup[pen_base | pen] & 0x0f;

					// pen bits A0-A3
					if (col)
						bitmap.pix(y, x) = (bitmap.pix(y, x) & 0xff0) | col;
				}

				// next pixel
				if (flip_x)
					x = x - 1;
				else
					x = x + 1;

				data = data << 2;
			}
		}

		// next row
		if (flip_y)
			y = y - 1;
		else
			y = y + 1;

		x = x_sav;
	}
}


void _20pacgal_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0x80 - 2; offs >= 0; offs -= 2)
	{
		static const int code_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};

		uint8_t code = m_sprite_ram[offs + 0x000];
		uint8_t color = m_sprite_ram[offs + 0x001];

		int sx = m_sprite_ram[offs + 0x081] - 41 + 0x100*(m_sprite_ram[offs + 0x101] & 3);
		int sy = 256 - m_sprite_ram[offs + 0x080] + 1;

		int flip_x = (m_sprite_ram[offs + 0x100] & 0x01) >> 0;
		int flip_y = (m_sprite_ram[offs + 0x100] & 0x02) >> 1;
		int size_x = (m_sprite_ram[offs + 0x100] & 0x04) >> 2;
		int size_y = (m_sprite_ram[offs + 0x100] & 0x08) >> 3;

		sy = sy - (16 * size_y);
		sy = (sy & 0xff) - 32; // fix wraparound

		// only Galaga appears to be effected by the global flip state
		if (m_game_selected && (m_flip[0] & 0x01))
		{
			flip_x = !flip_x;
			flip_y = !flip_y;
		}

		for (int y = 0; y <= size_y; y++)
			for (int x = 0; x <= size_x; x++)
			{
				draw_sprite(bitmap, cliprect,
						sy + (16 * y), sx + (16 * x),
						code + code_offs[y ^ (size_y * flip_y)][x ^ (size_x * flip_x)],
						color, flip_y, flip_x);
			}
	}
}



/*************************************
 *
 *  Character map drawing
 *
 *************************************/

void _20pacgal_state::draw_chars(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flip = m_flip[0] & 0x01;

	// for each byte in the video RAM
	for (int offs = 0; offs < 0x400; offs++)
	{
		int y, x;

		uint8_t *gfx = &m_char_gfx_ram.target()[m_video_ram[0x0000 | offs] << 4];
		uint32_t color_base = (m_video_ram[0x0400 | offs] & 0x3f) << 2;

		// map the offset to (x, y) character coordinates
		if ((offs & 0x03c0) == 0)
		{
			y = (offs & 0x1f) - 2;
			x = (offs >> 5) + 34;
		}
		else if ((offs & 0x03c0) == 0x3c0)
		{
			y = (offs & 0x1f) - 2;
			x = (offs >> 5) - 30;
		}
		else
		{
			y = (offs >> 5) - 2;
			x = (offs & 0x1f) + 2;
		}

		if ((y < 0) || (y > 27))
			continue;

		// convert to pixel coordinates
		y = y << 3;
		x = x << 3;

		if (flip)
		{
			y = 224 - 1 - y;
			x = 288 - 1 - x;
		}

		// for each row in the character
		for (int sy = 0; sy < 8; sy++)
		{
			int x_sav = x;

			if ((y >= cliprect.min_y) && (y <= cliprect.max_y))
			{
				uint16_t data = (gfx[8] << 8) | gfx[0];

				// for each pixel in the row
				for (int sx = 0; sx < 8; sx++)
				{
					if ((x >= cliprect.min_x) && (x <= cliprect.max_x))
					{
						uint32_t col = ((data & 0x8000) >> 14) | ((data & 0x0800) >> 11);

						// pen bits A4-A11
						if ( col != 0 )
							bitmap.pix(y, x) = (color_base | col) << 4;

						// next pixel
						if (flip)
							x = x - 1;
						else
							x = x + 1;

						if (sx == 0x03)
							data = data << 5;
						else
							data = data << 1;
					}
				}
			}

			// next row
			if (flip)
				y = y - 1;
			else
				y = y + 1;

			x = x_sav;

			gfx = gfx + 1;
		}
	}
}


/*************************************
 *
 *  Draw stars
 *
 *************************************/

/* starfield lfsr code is at d616 and at d648
 *
 * Code at d616:
 *
 * bit 6 (0x40) from 4be4 (port 8A) is rotated into top bit of 586D/586C
 * Based on bit 4 (0x10) and the carry out , bit 6 of 4be4 is set to:
 *
 * Carry out   bit 4    Result
 *    1          1         1
 *    1          0         0
 *    0          1         0
 *    0          0         1
 *
 * Code at d648 is opposite direction.
 *
 * The two videos show, that the starfield is covering the whole screen.
 * Galaga is different, the 256H signal is delivered with 12 pixels
 * delay ( NAND of 1H,2H,4H) to the starfield generator 05xx.
 *
 *    http://www.youtube.com/watch?v=c1zIitLkpGs
 *
 *    http://www.youtube.com/watch?v=bS2Zcin6OwM&feature=PlayList&p=649FD471A1803ABB&playnext_from=PL&playnext=1&index=8
 *
 * Galaga (for comparison)
 *
 *    http://www.youtube.com/watch?v=-R4CCB2g5bE
 *
 * The typical lfsr star count pattern is
 *
 * 132 stars
 * 125 stars
 * 129 stars
 * 68 stars
 * 132 stars
 * 125 stars
 * 129 stars
 * 68 stars
 *
 * This is close to Galaga's 63/126
 *
 */

void _20pacgal_state::draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	if (BIT(m_stars_ctrl[0], 5))
	{
		uint16_t lfsr =   m_stars_seed[0] + m_stars_seed[1]*256;
		uint8_t feedback = (m_stars_ctrl[0] >> 6) & 1;
		uint16_t star_seta = (m_stars_ctrl[0] >> 3) & 0x01;
		uint16_t star_setb = (m_stars_ctrl[0] >> 3) & 0x02;

		// This is a guess based on galaga star sets
		star_seta = 0x3fc0 | (star_seta << 14);
		star_setb = 0x3fc0 | (star_setb << 14);

		for (int clock = 0; clock < 288*224; clock++)
		{
			int x = clock % 288;
			int y = clock / 288;

			/* code at d616 translates into:
			 * carryout = lfsr & 1;
			 * lfsr = lfsr>>1;
			 * lfsr = (feedback << 15) | lfsr;
			 * feedback = (((lfsr>>4) & 1) ^ (carryout & 1)) ^ 1;
			 *
			 * and needs a Hack:
			 *  x = 288 - x;
			 *
			 */

			// code at d648
			int carryout = ((lfsr >> 4) ^ feedback ^ 1) & 1;
			feedback = (lfsr >> 15) & 1;
			lfsr = (lfsr << 1) | carryout;

			if (((lfsr & 0xffc0) == star_seta) || ((lfsr & 0xffc0) == star_setb))
			{
				if (y >= cliprect.min_y && y <= cliprect.max_y)
					bitmap.pix(y, x) = 0x1000 + (lfsr & 0x3f);
			}
		}
	}
}

// this is wrong
// where does the clut (sprite_lookup_w) get uploaded? even if I set a WP on that data in ROM it isn't hit?
// likewise the sound table.. is it being uploaded in a different format at 0x0c000?
// we also need the palette data because there is only a single rom on this pcb?
void _25pacman_state::_25pacman_map(address_map &map)
{
	map(0x00000, 0x3ffff).rw("flash", FUNC(amd_29lv200t_device::read), FUNC(amd_29lv200t_device::write)); // (always fall through if nothing else is mapped?)

	map(0x04000, 0x047ff).ram().share("video_ram");
	map(0x04800, 0x05fff).ram();
	map(0x06000, 0x06fff).writeonly().share("char_gfx_ram");
	map(0x07000, 0x0717f).w(FUNC(_25pacman_state::sprite_ram_w));
//  map(0x08000, 0x09fff).bankr("mainbank").w(FUNC(_20pacgal_state::ram_48000_w));
	map(0x08000, 0x09fff).nopw();
	map(0x0a000, 0x0bfff).w(FUNC(_25pacman_state::sprite_gfx_w));
	map(0x0c000, 0x0dfff).nopw(); // is this the sound waveforms in a different format?
	map(0x0e000, 0x0ffff).nopw();
	map(0x1c000, 0x1ffff).nopw();
}

void _20pacgal_state::_20pacgal_map(address_map &map)
{
	map(0x00000, 0x03fff).rom();
	map(0x04000, 0x07fff).rom();
	map(0x08000, 0x09fff).rom();
	map(0x0a000, 0x0ffff).mirror(0x40000).rom();
	map(0x10000, 0x3ffff).rom();
	map(0x44000, 0x447ff).ram().share("video_ram");
	map(0x44800, 0x45eff).ram();
	map(0x45040, 0x4505f).w("namco", FUNC(namco_cus30_device::pacman_sound_w));
	map(0x45f00, 0x45fff).w("namco", FUNC(namco_cus30_device::namcos1_cus30_w));
	map(0x46000, 0x46fff).writeonly().share("char_gfx_ram");
	map(0x47100, 0x47100).ram(); // leftover from original Galaga code
	map(0x48000, 0x49fff).bankr("mainbank").w(FUNC(_20pacgal_state::ram_48000_w)); // this should be a mirror of 08000-09fff
	map(0x4c000, 0x4dfff).w(FUNC(_20pacgal_state::sprite_gfx_w));
	map(0x4e000, 0x4e17f).w(FUNC(_20pacgal_state::sprite_ram_w));
	map(0x4e180, 0x4feff).nopw();
	map(0x4ff00, 0x4ffff).w(FUNC(_20pacgal_state::sprite_lookup_w));
}


/*************************************
 *
 *  I/O port handlers
 *
 *************************************/

void _25pacman_state::_25pacman_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); // Z180 internal registers
	map(0x40, 0x7f).noprw(); // Z180 internal registers
	map(0x80, 0x80).portr("P1");
	map(0x81, 0x81).portr("P2");
	map(0x82, 0x82).portr("SERVICE");
	map(0x80, 0x80).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x81, 0x81).w(FUNC(_25pacman_state::timer_pulse_w)); // ??? pulsed by the timer irq
	map(0x82, 0x82).w(FUNC(_25pacman_state::irqack_w));
//  map(0x84, 0x84).noprw(); // ??
	map(0x85, 0x86).writeonly().share("stars_seed"); // stars: rng seed (lo/hi)
	map(0x87, 0x87).lr8(NAME([] () { return 0xff; })).nopw(); // not eeprom on this
//  map(0x88, 0x88).w(FUNC(_25pacman_state::ram_bank_select_w));
	map(0x89, 0x89).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x8a, 0x8a).writeonly().share("stars_ctrl"); // stars: bits 3-4 = active set; bit 5 = enable
	map(0x8b, 0x8b).writeonly().share("flip");
	map(0x8c, 0x8c).nopw();
	map(0x8f, 0x8f).w(FUNC(_25pacman_state::coin_counter_w));
}

void _20pacgal_state::_20pacgal_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); // Z180 internal registers
	map(0x40, 0x7f).noprw(); // Z180 internal registers
	map(0x80, 0x80).portr("P1");
	map(0x81, 0x81).portr("P2");
	map(0x82, 0x82).portr("SERVICE");
	map(0x80, 0x80).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x81, 0x81).w(FUNC(_20pacgal_state::timer_pulse_w)); // ??? pulsed by the timer irq
	map(0x82, 0x82).w(FUNC(_20pacgal_state::irqack_w));
	map(0x84, 0x84).noprw(); // ??
	map(0x85, 0x86).writeonly().share("stars_seed"); // stars: rng seed (lo/hi)
	map(0x87, 0x87).portr("EEPROMIN").portw("EEPROMOUT");
	map(0x88, 0x88).w(FUNC(_20pacgal_state::ram_bank_select_w));
	map(0x89, 0x89).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x8a, 0x8a).writeonly().share("stars_ctrl"); // stars: bits 3-4 = active set; bit 5 = enable
	map(0x8b, 0x8b).writeonly().share("flip");
	map(0x8f, 0x8f).w(FUNC(_20pacgal_state::coin_counter_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( 20pacgal )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME( "Right 1 Player Start" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Left 1 Player Start" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME( "Left 2 Players Start" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME( "Right 2 Players Start" )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))    // bit 7 is EEPROM data

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))  // bit 5 is cs (active high)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)) // bit 6 is clock (active high)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))  // bit 7 is data
INPUT_PORTS_END


// Note: 25pacman Control Panel functions the same as 20pacgal, even if Right P1/P2 start are not shown during Switch Test
static INPUT_PORTS_START( 25pacman )
	PORT_INCLUDE(20pacgal)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME ( "Service Volume Up" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME ( "Service Volume Down" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( 25pacmano )
	PORT_INCLUDE(20pacgal)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("EEPROMIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void _20pacgal_state::common_save_state()
{
	m_ram_48000 = make_unique_clear<uint8_t[]>(0x2000);

	save_item(NAME(m_game_selected));
	save_pointer(NAME(m_ram_48000), 0x2000);
	save_item(NAME(m_irq_mask));
}

void _20pacgal_state::machine_start()
{
	common_save_state();

	m_game_selected = 0;

	// center the DAC; not all revisions do this
	m_dac->data_w(0x80);

	// membank currently used only by 20pacgal
	m_mainbank->configure_entry(0, memregion("maincpu")->base() + 0x08000);
	m_mainbank->configure_entry(1, m_ram_48000.get());
}

void _25pacman_state::machine_start()
{
	common_save_state();

	m_game_selected = 0;
}

void _20pacgal_state::video_start()
{
	m_sprite_gfx_ram = make_unique_clear<uint8_t[]>(0x2000);
	m_sprite_ram = make_unique_clear<uint8_t[]>(0x180);
	m_sprite_color_lookup = make_unique_clear<uint8_t[]>(0x100);

	save_pointer(NAME(m_sprite_gfx_ram), 0x2000);
	save_pointer(NAME(m_sprite_ram), 0x180);
	save_pointer(NAME(m_sprite_color_lookup), 0x100);

	if (m_proms != nullptr)
		get_pens();
}

uint32_t _20pacgal_state::screen_update_20pacgal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	draw_stars(bitmap, cliprect);
	draw_chars(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);

	return 0;
}

void _20pacgal_state::vblank_irq(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(0, HOLD_LINE); // TODO: assert breaks the inputs in 25pacman test mode
}

static DEVICE_INPUT_DEFAULTS_START( null_modem )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void _20pacgal_state::_20pacgal(machine_config &config)
{
	// basic machine hardware
	Z8S180(config, m_maincpu, 73.728_MHz_XTAL / 4); // 18.432MHz verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &_20pacgal_state::_20pacgal_map);
	m_maincpu->set_addrmap(AS_IO, &_20pacgal_state::_20pacgal_io_map);
	m_maincpu->txa0_wr_callback().set("rs232", FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(null_modem));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(null_modem));
	m_rs232->rxd_handler().set(m_maincpu, FUNC(z180_device::rxa0_w));
	m_rs232->cts_handler().set(m_maincpu, FUNC(z180_device::cts0_w));

	EEPROM_93C46_8BIT(config, m_eeprom);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(73.728_MHz_XTAL / 4 / 3, 396, 0, 288, 256, 0, 224);
	screen.set_screen_update(FUNC(_20pacgal_state::screen_update_20pacgal));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(_20pacgal_state::vblank_irq));

	PALETTE(config, m_palette, FUNC(_20pacgal_state::starpal_init), 0x1000 + 64);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	namco_cus30_device &namco(NAMCO_CUS30(config, "namco", 73.728_MHz_XTAL / 4 / 6 / 32));
	namco.set_voices(3);
	namco.add_route(ALL_OUTPUTS, "speaker", 0.5);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
}

static DEVICE_INPUT_DEFAULTS_START( null_modem_57600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_57600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_57600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void _25pacman_state::_25pacman(machine_config &config)
{
	_20pacgal(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &_25pacman_state::_25pacman_map);
	m_maincpu->set_addrmap(AS_IO, &_25pacman_state::_25pacman_io_map);

	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(null_modem_57600));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(null_modem_57600));

	AMD_29LV200T(config, "flash");
}



/*************************************
 *
 *  ROM definition
 *
 *************************************/

/*
     Pacman - 25th Anniversary Edition
*/

// guzuta v2.2 PCB
// this uses the main FLASH rom to save things instead of eeprom (type is AM29LV200)
// different memory map.. no palette rom
ROM_START( 25pacman ) /* Revision 3.00 */
	ROM_REGION( 0x40000, "flash", 0 )
	ROM_LOAD( "pacman25ver3.u1", 0x00000, 0x40000, CRC(55b0076e) SHA1(4544cc193bdd22bfc88d096083ccc4069cac4607) ) /* program says Rev 3.00 */
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	// shouldn't be loading this! must be uploaded somewhere
	ROM_LOAD( "pacman_25th.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 25pacmano ) /* Revision 2.00 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "pacman_25th_rev2.0.u13", 0x00000, 0x40000, CRC(99a52784) SHA1(6222c2eb686e65ba23ca376ff4392be1bc826a03) ) /* Label printed Rev 2.0, program says Rev 2.00 */

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "pacman_25th.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) ) /* Same as the MS. Pacman / Galaga graphics rom */
ROM_END


/*
     Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion
*/

ROM_START( 20pacgal ) /* Version 1.08 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.08.u13", 0x00000, 0x40000, CRC(2ea16809) SHA1(27f041bdbb590917e9dcb70c21aa6b6d6c9f04fb) ) /* Also found labeled as "V1.08 HO" */

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr4 ) /* Version 1.04 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.04.u13", 0x00000, 0x40000, CRC(6c474d2d) SHA1(5a150fc9d2ed0e908385b9f9d532aa33cf80dba4) )

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr3 ) /* Version 1.03 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.03.u13", 0x00000, 0x40000, CRC(e13dce63) SHA1(c8943f082883c423210fc3c97323222afb00f0a2) )

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr2 ) /* Version 1.02 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.02.u13", 0x00000, 0x40000, CRC(b939f805) SHA1(5fe9470601156dfc2d339c94fd8f0aa4db197760) )

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr1 ) /* Version 1.01 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.01.u13", 0x00000, 0x40000, CRC(77159582) SHA1(c05e005a941cbdc806dcd76b315069362c792a72) )

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr0 ) /* Version 1.00 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.0.u13", 0x00000, 0x40000, CRC(3c92a269) SHA1(a616d912393f4e49b95231d72eec48567f46fc00) ) /* Label printed V1.0, program says v1.00 */

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END



void _20pacgal_state::init_20pacgal()
{
	m_sprite_pal_base = 0x00 << 2;
}

void _20pacgal_state::init_25pacman()
{
	m_sprite_pal_base = 0x20 << 2;
}

} // anonymous namespace

/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 2006, 25pacman,   0,        _25pacman, 25pacman,  _25pacman_state, init_25pacman, ROT90, "Namco / Cosmodog", "Pac-Man - 25th Anniversary Edition (Rev 3.00)",                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 2005, 25pacmano,  25pacman, _20pacgal, 25pacmano, _20pacgal_state, init_25pacman, ROT90, "Namco / Cosmodog", "Pac-Man - 25th Anniversary Edition (Rev 2.00)",                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)

GAME( 2000, 20pacgal,   0,        _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.08)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr4, 20pacgal, _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.04)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr3, 20pacgal, _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.03)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr2, 20pacgal, _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.02)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr1, 20pacgal, _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.01)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr0, 20pacgal, _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.00)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
