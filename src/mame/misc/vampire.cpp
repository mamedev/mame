// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*

Entertainment Enterprises Vampire

TODO:
- correct interrupt handling (main and sound cpu)
- communication between main and audio cpu
- PIT outputs
- correct clocks
- color decode
- fix gfx glitches - extra pixels (column?) on edges (only when gfx is flipped)

Hardware notes:

MAIN CPU:
 - 6809P, 8 MHz xtal
 - NMI - main interrupt vector, currectly there's a hack (int vectors) to use standard irq/firq vectors
         Currently NMI writes error messages to the terminal(?) output ($814, $815)

VIDEO:
 - custom blitter - ROM (256x512, 1bpp) -> VRAM (256x256, 4bpp?, double buffered?)
 - blitter data (8 bytes long 'slots') stored in RAM, format:

   0 - src address (ROM)
   1 /
   2 - 256 - height
   3 - 32 - width (bytes)
   4 - flags
       76543210
       x------- end of list
       --x----- flip
       -?-????? unknown (unused?)
   5 - pen (0-15)
   6 - dsty
   7 - dstx

- there's a base (x,y) offset for each blit($7fa-$7fb writes), write to this address triggers the blitter
- blitter starts to copy gfx from the last accessed slot (even if ram is locked) and stops on the slot with 'end of list' bit set
- blitter control ($7f8):

  76543210
  x------- flip screen
  -x------ blitter ram lock
  ---x---- front/back buffer (?)
  --?-???? unknown

AUDIO:
- 6502 ? (empty socket)
- 4MHz xtal near the cpu socket
- IRQ - ?
- NMI - triggered by main cpu, sound latch read
- AY-3-8910
- PIT (8253) - used to sync the audio
- game works without the sound cpu

+-----------------------------------------------------------------------------------------------------------------------------------+
|                                                                                                                                   |
|                                           C                  D                   E                F          H          J         |
|                                                                                                                                   |
|               A             B         +-------+                           +------------+      +-------------+                     |
|                                       |74LS14N|                           | MSM2128RS  |      |H1         1F|                     |
|  +----+   +-------+     +-------+     +-------+                           |   22075    |      |  M5L2764K   |                   1 |
|  |8Mhz|   |74LS123|     |74LS367|                                         +------------+      +-------------+                     |
|  |xtal|   +-------+     +-------+     +-------+          +-------+        +------------+      +-------------+                     |
|  +----+                               |74LS367|          |74LS367|        | MSM2128RS  |      |H2         2F|                   2 |
|           +---------------------+     +-------+          +-------+        |   22075    |      |  M5L2764K   |                     |
+------+    |                     |                                         +------------+      +-------------+                     |
|      |    |       HD68B09P      |     +-------+          +-------+        +------------+      +-------------+                     |
|      |    |           JAPAN     |     |74LS04P|          |74LS139|        | MSM2128RS  |      |H3         3F|                   3 |
|      |    +---------------------+     +-------+          +-------+        |   22075    |      |  M5L2764K   |                     |
|      |                                                                    +------------+      +-------------+                     |
|CONNEC|    +-------+   +---------+     +-------+          +-------+        +------------+      +-------------+                     |
|TOR   |    |74LS04P|   |74LS245N |     |74LS00N|          |74LS138|        | MSM2128RS  |      |H4         4F|       +---------+   |
|      |    +-------+   +---------+     +-------+          +-------+        |   22075    |      |  M5L2764K   |       |74LS273N | 4 |
|RIBBON|                                                                    +------------+      +-------------+       +---------+   |
|CABLE |    +-------+   +---------+      +-------+         +-------+                                                                |
|      |    |74LS02N|   |74LS245N |      |74LS138|         |74LS08N|                            +-------+             +---------+   |
|TO    |    +-------+   +---------+      +-------+         +-------+        +---------------+   |74LS166|             |74LS273N | 5 |
|SECOND|                                                                    |CG P1        6E|   +-------+             +---------+   |
|PCB   |    +-------+     +-------+     +-------+          +-------+        |   M5L2764K    |   +-------+  +-------+  +-------+     |
|      |    |74LS74A|     |       |     |74LS32N|          |74LS30N|        +---------------+   |74LS161|  |74LS161|  |74LS138|   6 |
|      |    +-------+     +-------+     +-------+          +-------+        +---------------+   +-------+  +-------+  +-------+     |
|      |                                                                    |CG P2        7E|   +-------+  +-------+  +-------+     |
|      |    +---------+   +-------+      +-------+                          |   M5L2764K    |   |74LS161|  |74LS161|  |74LS04P|   7 |
|      |    |74LS130N |   |74LS173|      |74LS138|                          +---------------+   +-------+  +-------+  +-------+     |
+------+    +---------+   +-------+      +-------+      +------------+      +---------+  +-------+                                  |
|                                                       | MSM2128RS  |      |74LS245N |  |74LS138|     +-------+      +-------+     |
|                         +-------+     +-------+       |   22075    |      +---------+  +-------+     |74LS74A|      |74LS74A|   8 |
+------+                  |74LS139|     |74LS125|       +------------+                                 +-------+      +-------+     |
|      |                  +-------+     +-------+                           +---------------------+                                 |
|      |                                                                    |    EMPTY SOCKET     |    +-------+         +----+     |
|      |                  +-------+    +---------+                          |most likely sound-cpu|    |74LS04P|         |4Mhz|   9 |
|      |                  |74LS74A|    |M74LS373P|                          +---------------------+    +-------+         |xtal|     |
|CONNEC|                  +-------+    +---------+                                                                       +----+     |
|TOR   |                                                                                                                            |
|      |    +-------+     +-------+    +------------+   +------------+      +-------+     +-------+    +---------+   +---------+    |
|RIBBON|    |MC14053|     |74LS174|    | M5L8253P-5 |   |S         9D|      |74LS367|     |74LS367|    |   SW2   |   |   SW1   | 10 |
|CABLE |    +-------+     +-------+    |            |   |  M5L2732K  |      +-------+     +-------+    +---------+   +---------+    |
|      |                               +------------+   +------------+                                                              |
|TO    |                                                                   +---------+   +---------+   +---------+   +---------+    |
|SECOND|                                                                   |74LS245N |   |74LS273N |   |74LS240N |   |74LS240N | 11 |
|PCB   |                                                                   +---------+   +---------+   +---------+   +---------+    |
|      |                                                                                                                            |
|      |                                                                                                                            |
|      |                                                                                                                            |
|      |                                                                                                                            |
|      |                      +----------------------+                     +---------+   +---------+   +---------+   +---------+    |
+------+                      |         SOUND        |                     |74LS244P |   |74LS240N |   |74LS240N |   |74LS240N | 12 |
|                             |       AY-3-8910      |                     +---------+   +---------+   +---------+   +---------+    |
|                             +----------------------+                                                                              |
|                                                                                                                                   |
|                 +----+                                                                                                            |
|                 |3080|                                                                                                            |
|                 +----+    +----+                +-------+                                                                         |
|                           |3080|                |LM324N |                                                                      13 |
|                           +----+                +-------+                                                                         |
|                                                                                                                                   |
|                                                                                                                                   |
|                                                                                                                                   |
|                                       +-----+B1            CONNECTOR             B22+-----+                                       |
|                                       |     |||||||||||||||||||||||||||||||||||||||||     |        P1422-1                        |
+---------------------------------------+     +---------------------------------------+     +---------------------------------------+



+-----------------------------------------------------------------------------------------------------------------------------------+
|          +---------+ A           B            C                D               E            F             G             H         |
|          |60.00000 |         +-------+    +-------+        +-------+       +-------+                                              |
|          |Mhz      |         |74S114N|    |74S20N |        |74S163N|       |74S04N |                                            1 |
|          +---------+         +-------+    +-------+        +-------+       +-------+                                              |
|                                                                                                                                   |
|                 +-------+    +-------+     +-------+     +---------+       +-------+     +-------+     +-------+                  |
|                 |74S04N |    |74S114N|     |74S163N|     |PAL12H6CJ|       |74S174N|     |74S04N |     |74LS74N|                2 |
|                 +-------+    +-------+     +-------+     +---------+       +-------+     +-------+     +-------+                  |
|                                                                   2D                                                              |
|                 +-------+    +-------+    +-------+                        +-------+     +-------+     +-------+                  |
|                 |74S174N|    |74S20N |    |74S08N |                        |74LS669|     |74LS107|     |74LS74A|                3 |
|                 +-------+    +-------+    +-------+                        +-------+     +-------+     +-------+                  |
|                                                                                                                                   |
+------+          +-------+    +---------+  +-------+        +-------+       +-------+     +-------+     +-------+                  |
|      |          |74S08N |    |PAL16R8CN|  |74LS161|        |74LS161|       |74LS138|     |74LS74A|     |74LS10N|                4 |
|      |          +-------+    +---------+  +-------+        +-------+       +-------+     +-------+     +-------+                  |
|      |                                4B                                                                                          |
|      |          +-------+    +-------+    +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
|      |          |74S02N |    |74S195N|    |74LS161|        |74LS161|       |74LS08N|     |74LS04P|     |74LS86P|     |74153N |  5 |
|CONNEC|          +-------+    +-------+    +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
|TOR   |                                                                                                                            |
|      |          +-------+    +-------+    +-------+      +---------+       +-------+     +-------+     +-------+     +-------+    |
|RIBBON|          |74LS86P|    |74LS74A|    |74LS174|      |74LS374N |       |74LS161|     |74LS161|     |74LS86P|     |74153N |  6 |
|CABLE |          +-------+    +-------+    +-------+      +---------+       +-------+     +-------+     +-------+     +-------+    |
|      |                                                                                                                            |
|TO    |          +-------+    +---------+  +-------+        +-------+                                   +-------+     +-------+    |
|SECOND|          |74LS197|    |74LS374N |  |74LS669|        |74LS669|                                   |74LS86P|     |74153N |  7 |
|PCB   |          +-------+    +---------+  +-------+        +-------+                                   +-------+     +-------+    |
|      |                                                                                                                            |
|      |          +-------+    +---------+  +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
|      |          |74LS197|    |74LS374N |  |74LS669|        |74LS669|       |74LS669|     |74LS669|     |74LS86P|     |74153N |  8 |
|      |          +-------+    +---------+  +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
+------+                                                                                                                            |
|                 +-------+    +-------+    +-------+        +-------+       +-------+     +-------+     +-------+                  |
|                 |74LS197|    |74LS283|    |74LS174|        |74LS174|       |74LS74A|     |74LS74A|     |74LS00N|                9 |
+------+          +-------+    +-------+    +-------+        +-------+       +-------+     +-------+     +-------+                  |
|      |                                                                                                                            |
|      |          +-------+    +-------+    +-------+      +---------+                                                              |
|      |          |74LS74A|    |74LS283|    |74S194N|      |74LS377N |       +-------+     +-------+     +-------+     +-------+    |
|      |          +-------+    +-------+    +-------+      +---------+       |MB8264 |     |MB8264 |     |MB8264 |     |MB8264 | 10 |
|      |          +-------+    +--------+   +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
|CONNEC|          |74LS32N|    |74LS374N|   |74S194N|        |74173N |       +-------+     +-------+     +-------+     +-------+    |
|TOR   |          +-------+    +--------+   +-------+        +-------+       |MB8264 |     |MB8264 |     |MB8264 |     |MB8264 | 11 |
|      |        +---------+    +---------+  +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
|RIBBON|        |74LS245N |    |74LS374N |  |74LS04P|        |74173N |       +-------+     +-------+     +-------+     +-------+    |
|CABLE |        +---------+    +---------+  +-------+        +-------+       |MB8264 |     |MB8264 |     |MB8264 |     |MB8264 | 12 |
|      |                                                                     +-------+     +-------+     +-------+     +-------+    |
|TO    |        +---------+    +---------+  +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
|SECOND|        |74LS245N |    |74LS374N |  |74LS194|        |74Ls194|       |MB8264 |     |MB8264 |     |MB8264 |     |MB8264 | 13 |
|PCB   |        +---------+    +---------+  +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
|      |                                                                     +-------+     +-------+     +-------+     +-------+    |
|      |     +------------+                                                  |MB8264 |     |MB8264 |     |MB8264 |     |MB8264 |    |
|      |     | M58725P-15 |    +-------+    +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
|      |     |    JAPAN   |    |74LS157|    |74LS194|        |74Ls194|       +-------+     +-------+     +-------+     +-------+ 14 |
+------+     +------------+    +-------+    +-------+        +-------+       |MB8264 |     |MB8264 |     |MB8264 |     |MB8264 |    |
|                                                                            +-------+     +-------+     +-------+     +-------+    |
|            +------------+    +-------+    +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
|            | M58725P-15 |    |74LS157|    |74LS194|        |74Ls194|       |MB8264 |     |MB8264 |     |MB8264 |     |MB8264 | 15 |
|            |    JAPAN   |    +-------+    +-------+        +-------+       +-------+     +-------+     +-------+     +-------+    |
|            +------------+                                                  +-------+     +-------+     +-------+     +-------+    |
|                               +-------+   +-------+        +-------+       |MB8264 |     |MB8264 |     |MB8264 |     |MB8264 |    |
|                    +-------+  |TB24S10|   |74LS194|        |74Ls194|       +-------+     +-------+     +-------+     +-------+ 16 |
|                    |TB24S10|  +-------+   +-------+        +-------+                                                              |
|                    +-------+        16B                                                                                           |
|                          16A                                               P1422-2                                                |
+-----------------------------------------------------------------------------------------------------------------------------------+


                Vampire Edge Connector
     Solder Side          |        Parts Side
------------------------------------------------------------------
         GND          | A | 1 |      GND
         +5V          | B | 2 |      +5V
                      | C | 3 |     +12V
                      | D | 4 |    Audio Out
        Coin          | E | 5 |
        Coin          | F | 6 |
                      | H | 7 |
                      | J | 8 |
                      | K | 9 |    Player Invisibility Button
    Player 2 Start    | L | 10|
    Player 1 Start    | M | 11|
    Player 2 Left     | N | 12|
    Player 2 Right    | P | 13|
    Player 2 Down     | R | 14|
    Player 2 Up       | S | 15|    Video Blue
    Player 1 Left     | T | 16|    Video Red
    Player 1 Right    | U | 17|    Video Green
    Player 1 Down     | V | 18|    Video Sync (Composite)
    Player 1 Up       | W | 19|
                      | X | 20|
         +5V          | Y | 21|      +5V
         GND          | Z | 22|      GND

*/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m6502/m6502.h"
#include "machine/gen_latch.h"
#include "machine/pit8253.h"
#include "sound/ay8910.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class vampire_state : public driver_device
{
public:
	vampire_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this,"audiocpu"),
		m_soundlatch(*this, "soundlatch"),
		m_palette(*this, "palette"),
		m_gfxrom(*this, "gfx")
	{ }

	void vampire(machine_config &config);
	void init_vampire();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_gfxrom;

	struct
	{
		u8 base_x = 0;
		u8 base_y = 0;

		offs_t last_offset = 0;
		u8 last_data = 0;

		u8 flags = 0xff;

		std::unique_ptr<u8[]> layer_1;
		std::unique_ptr<u8[]> layer_2;
		std::unique_ptr<u8[]> slots;
	} m_blitter;

	u8 m_pit_output = 0;

	void palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u8 blitter_r(offs_t offset);
	void blitter_w(offs_t offset, u8 data);
	void blit_trigger(offs_t offset);

	void vampire_memory(address_map &map) ATTR_COLD;
	void vampire_audio(address_map &map) ATTR_COLD;

	void pit_out_w0(int state);
	void pit_out_w1(int state);
	void pit_out_w2(int state);

	u8 sound_sync_r();
	u8 sound_ack_r();
	void blitter_flags_w(u8 data);
	void blitter_control_w(offs_t offset, u8 data);
	u8 io814_r();
	u8 io815_r();
};

void vampire_state::machine_start()
{
	m_blitter.layer_1 = make_unique_clear<u8[]>(256 * 256);
	m_blitter.layer_2 = make_unique_clear<u8[]>(256 * 256);
	m_blitter.slots = make_unique_clear<u8[]>(0x1000);

	save_item(NAME(m_blitter.base_x));
	save_item(NAME(m_blitter.base_y));
	save_item(NAME(m_blitter.last_offset));
	save_item(NAME(m_blitter.last_data));
	save_item(NAME(m_blitter.flags));
	save_pointer(NAME(m_blitter.layer_1), 256 * 256);
	save_pointer(NAME(m_blitter.layer_2), 256 * 256);
	save_pointer(NAME(m_blitter.slots), 0x1000);

	save_item(NAME(m_pit_output));
}



/*******************************************************************************
    Video
*******************************************************************************/

void vampire_state::palette(palette_device& palette) const
{
	u8 *proms = memregion("proms")->base();
	for (int i = 0; i < 256; ++i)
	{
		int g = ((proms[i] & 0b00000011) >> 0) * 85;
		int r = ((proms[i] & 0b00011100) >> 2) * 36;
		int b = ((proms[i] & 0b11100000) >> 5) * 36;
		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

u32 vampire_state::screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	// replace with data copy
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			bitmap.pix(y, x) = (m_blitter.flags & 0x10) ? m_blitter.layer_1[256 * y + x] : m_blitter.layer_2[256 * y + x];
		}
	}

	return 0;
}

void vampire_state::blit_trigger(offs_t offset)
{
	u8 *ptr = (m_blitter.flags & 0x10) ? m_blitter.layer_2.get() : m_blitter.layer_1.get();
	u32 index = offset >> 3; // start of the 8 byte-long slot

	while (index < 0x200)
	{
		const u8 *slot = &m_blitter.slots[index << 3];

		u32 desty = slot[6]; // pixels
		const u32 destx = slot[7]; // pixels

		const u32 start_offset = ((slot[0] << 8) | slot[1]) << 3; // start offset
		const u8 pen = slot[5]; // 0xf is max used

		const int32_t sy = 256 - slot[2]; // pixels
		const int32_t sx = (32 - slot[3]) << 3; // bytes (src ROM)

		for (int yy = 0; yy < sy; ++yy)
		{
			for (int xx = 0; xx < sx; ++xx)
			{
				const u32 srcptr = start_offset + 256 * yy + xx;
				const u32 dstptr = (desty + m_blitter.base_y) * 256 + destx + xx + m_blitter.base_x;

				if (desty + m_blitter.base_y > 255 || destx + xx + m_blitter.base_x > 255)
					continue;

				u8 pix = 0;

				if (dstptr < 256 * 256)
					pix = m_gfxrom[(srcptr >> 3) & 0x3fff] & (1 << (7 - (srcptr & 7)));

				// when source is above the 256x512 (ROM size) image = solid fill (rom read returns 0xff?
				// could be handled by extended rom region, ff filled)
				if (slot[0] & 0x80)
					pix = 1;

				if (pix)
					ptr[dstptr] = pen;
			}
			desty += (slot[4] & 0x20) ? 1 : -1; // flip
		}
		if (slot[4] & 0x80)
			break; // end mark

		++index;
	}
}



/*******************************************************************************
    Main CPU I/O
*******************************************************************************/

void vampire_state::blitter_flags_w(u8 data)
{
	m_blitter.flags = data;
}

void vampire_state::blitter_control_w(offs_t offset, u8 data)
{
	if (!(offset & 1))
	{
		m_blitter.base_y = data;
	}
	else
	{
		m_blitter.base_x = data;
		blit_trigger(m_blitter.last_offset);
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

u8 vampire_state::sound_ack_r()
{
	return m_soundlatch->pending_r() << 7 ^ 0x80;
}


// Main CPU NMI
u8 vampire_state::io814_r()
{
	// bit 1
	return machine().rand();
}

u8 vampire_state::io815_r()
{
	return machine().rand();
}


u8 vampire_state::blitter_r(offs_t offset)
{
	return m_blitter.slots[offset];
}

void vampire_state::blitter_w(offs_t offset, u8 data)
{
	m_blitter.last_offset = offset;

	if (m_blitter.flags & 0x40)
	{
		m_blitter.last_data = data; // not used?
		return;
	}

	m_blitter.slots[offset] = data;
}


void vampire_state::vampire_memory(address_map &map)
{
	map(0x07f0, 0x07f0).portr("IN1");
	map(0x07f1, 0x07f1).portr("IN2");
	map(0x07f2, 0x07f2).portr("DSW1");
	map(0x07f3, 0x07f3).portr("DSW2");
	map(0x07f4, 0x07f4).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x07f5, 0x07f5).r(FUNC(vampire_state::sound_ack_r));
	map(0x07f8, 0x07f8).w(FUNC(vampire_state::blitter_flags_w));
	map(0x07fa, 0x07fb).w(FUNC(vampire_state::blitter_control_w));

	map(0x0814, 0x0814).r(FUNC(vampire_state::io814_r));
	map(0x0815, 0x0815).r(FUNC(vampire_state::io815_r));

	map(0x1000, 0x1fff).r(FUNC(vampire_state::blitter_r)).w(FUNC(vampire_state::blitter_w));
	map(0x2000, 0x3fff).ram();
	map(0x8000, 0xffff).rom();
}



/*******************************************************************************
    Audio CPU I/O
*******************************************************************************/

void vampire_state::pit_out_w0(int state)
{
	m_pit_output = state;
}

void vampire_state::pit_out_w1(int state)
{
	// unknown
}

void vampire_state::pit_out_w2(int state)
{
	// unknown
}

u8 vampire_state::sound_sync_r()
{
	return m_pit_output << 6;
}

void vampire_state::vampire_audio(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x4000, 0x4000).ram(); // unk write
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0x8000).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
	map(0xa000, 0xa001).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0xa008, 0xa00b).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xa00c, 0xa00c).r(FUNC(vampire_state::sound_sync_r));
	map(0xf000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( vampire )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) // $d831
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Infinite Time (Cheat)" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Infinite Lives (Cheat)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) // unused?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)   PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(2) // unused?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(2)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void vampire_state::vampire(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, 8_MHz_XTAL); // HD68B09P
	m_maincpu->set_addrmap(AS_PROGRAM, &vampire_state::vampire_memory);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(4_MHz_XTAL/2);
	pit.set_clk<1>(4_MHz_XTAL/2);
	pit.set_clk<2>(4_MHz_XTAL/2);
	pit.out_handler<0>().set(FUNC(vampire_state::pit_out_w0));
	pit.out_handler<1>().set(FUNC(vampire_state::pit_out_w1));
	pit.out_handler<2>().set(FUNC(vampire_state::pit_out_w2));

	M6502(config, m_audiocpu, 4_MHz_XTAL/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &vampire_state::vampire_audio);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0*8, 32*8 - 1, 2*8, 30*8 - 1);
	screen.set_screen_update(FUNC(vampire_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, M6809_FIRQ_LINE, HOLD_LINE);

	screen.set_palette(m_palette);
	PALETTE(config, m_palette, FUNC(vampire_state::palette), 256);

	// sound hardware
	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->set_separate_acknowledge(true);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->data_pending_callback().append([this](int state) { if (state) machine().scheduler().perfect_quantum(attotime::from_usec(100)); });

	SPEAKER(config, "speaker").front_center();
	AY8910(config, "aysnd", 4_MHz_XTAL/4).add_route(ALL_OUTPUTS, "speaker", 0.5);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( vampire )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1h.1h", 0x8000, 0x2000, CRC(7e69ff9b) SHA1(85f6303803a0577f96b879e0b9d280a4f22cca8c) )
	ROM_LOAD( "2h.2h", 0xa000, 0x2000, CRC(e94155f8) SHA1(f60d82d9204f7b13aa9be189ca74ca48bf9e879a) )
	ROM_LOAD( "3h.3h", 0xc000, 0x2000, CRC(ce27dd90) SHA1(4edd8ba08f828a0c3c6eb21a2d9e0d1e53fac407) )
	ROM_LOAD( "4h.4h", 0xe000, 0x2000, CRC(a25f00bc) SHA1(db445c1876b79c4d553d4f9cc881f41f68b6667c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "s_9d.9d", 0xf000, 0x1000, CRC(e13a7aef) SHA1(77a49cb0f3f037826a32bbfa8fab524f17895992) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "cg_p1.6e", 0x0000, 0x2000, CRC(042661a4) SHA1(58ed7c782a2486aa8c2c650d6d9e54929e5dd50b) )
	ROM_LOAD( "cg_p2.7e", 0x2000, 0x2000, CRC(e9dd9dff) SHA1(8a9ad8659763a9c010d0b482af3a40978c02cec7) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD_NIB_LOW(  "tb24s10.16a", 0x0000, 0x0100, CRC(bc60a2eb) SHA1(73b8f5f6eee90a4d59a5c62a6a342fa49185938c) ) // TB24S10 BPROM
	ROM_LOAD_NIB_HIGH( "tb24s10.16b", 0x0000, 0x0100, CRC(aa6b627b) SHA1(556d6bab46419ce55dd254de9615981852be6e6f) ) // TB24S10 BPROM
ROM_END

void vampire_state::init_vampire()
{
	u8 *rom = memregion("maincpu")->base();

	// hack interrupt vectors
	rom[0xfff6] = rom[0xffe0];
	rom[0xfff7] = rom[0xffe1];
	rom[0xfff8] = rom[0xffe2];
	rom[0xfff9] = rom[0xffe3];
}

} // anonymous namestate



/*******************************************************************************
    Drivers
*******************************************************************************/

// it is unclear if the game saw a wide release, or was only location tested
GAME( 1983, vampire, 0, vampire, vampire, vampire_state, init_vampire, ROT90, "Entertainment Enterprises, Ltd.", "Vampire (prototype?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
