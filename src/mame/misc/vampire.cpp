// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*


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
|SECOND|                                                                    |P1           6E|   +-------+             +---------+   |
|PCB   |    +-------+     +-------+     +-------+          +-------+        |   M5L2764K    |   +-------+  +-------+  +-------+     |
|      |    |74LS74A|     |       |     |74LS32N|          |74LS30N|        +---------------+   |74LS161|  |74LS161|  |74LS138|   6 |
|      |    +-------+     +-------+     +-------+          +-------+        +---------------+   +-------+  +-------+  +-------+     |
|      |                                                                    |P2           7E|   +-------+  +-------+  +-------+     |
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
|RIBBON|    |MC14053|     |74LS174|    | M5L8253P-5 |   |9D          |      |74LS367|     |74LS367|    |   SW2   |   |   SW1   | 10 |
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

MAIN CPU: 
 - 6809P, 8 MHz xtal
 - NMI - main interrupt vector, currectly there's a hack (int vectors) to use standard irq/firq vectors 
         Currently NMI writes error messages to the terminal(?) output ($814, $815)

VIDEO:  
 - custom blitter - ROM (256x512, 1bpp ) -> VRAM (256x256, 4bpp?, double buffered?)
 - blitter data (8 bytes long 'slots') stored in RAM , format:

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

  - there's base (x,y) offset for each blit($7fa-$7fb writes), write to this address triggers the blitter
  - blitter starts to copy gfx from the last accessed slot (even if ram is locked) and stops on the slot with 'end of list' bit set
  - blitter controll ($7f8):

    76543210
    x------- flip screen
    -x------ blitter ram lock
    ---x---- front/back buffer (?)
    --?-???? unknown

AUDIO:
  - 6502 ? ( empty socket)
  - 4MHz xtal near the cpu socket
  - IRQ - ?
  - NMI - triggered by main cpu, sound latch read
  - AY-3-8910
  - PPI (8253) - used to sync the audio	
  - game works without the sound cpu

 TODO;
  - correct interrupt handling (main and sound cpu)
  - communication between main and audio cpu
  - PIT outpus
  - correct clocks
  - color decode 
  - fix gfx glitches - extra pixels (column?) on edges (only when gfx is flipped)
*/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "machine/pit8253.h"

#include "machine/timer.h"
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
		m_palette(*this, "palette"),
		m_gfxrom(*this, "gfx")
	{ }

	void vampire(machine_config &config);
	void init_vampire();

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<palette_device> m_palette;

	void palette(palette_device &palette) const;

	uint8_t blitter_r(offs_t offset);
	uint8_t fsoundlatch_r(offs_t offset);
	void audioack_w(offs_t offset, uint8_t state);
	void blitter_w(offs_t offset, uint8_t data);
	void blit_trigger(uint8_t * blitterdata, uint16_t data);

	struct
	{
		int base_x = 0;
		int base_y = 0;

		int last_offset = 0;
		int last_data = 0;

		int flags = 0xff;

		std::unique_ptr<uint8_t[]> layer_1;
		std::unique_ptr<uint8_t[]> layer_2;
		std::unique_ptr<uint8_t[]> slots;
	} m_blitter;
		
	int m_audio_nmi = 0;
	int m_audio_latch = 0;

	int m_ppi_output = 0;

	void vampire_memory(address_map &map);
	void vampire_audio(address_map &map);

	void pit_out_w0(int state);
	void pit_out_w1(int state);
	void pit_out_w2(int state);

	uint8_t sound_sync_r(offs_t offset);
	void soundlatch_w(offs_t offset, uint8_t data);
	uint8_t sound_ack_r(offs_t offset);
	void blitter_flags_w(offs_t offset, uint8_t data);
	void blitter_control_w(offs_t offset, uint8_t data);
	uint8_t io814_r(offs_t offset);
	uint8_t io815_r(offs_t offset);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_region_ptr<uint8_t> m_gfxrom;
};

void vampire_state::machine_start()
{
	m_blitter.layer_1 = make_unique_clear<uint8_t[]>(256 * 256);
	m_blitter.layer_2 = make_unique_clear<uint8_t[]>(256 * 256);
	m_blitter.slots = make_unique_clear<uint8_t[]>(0x1000);

	save_item(NAME(m_blitter.base_x));
	save_item(NAME(m_blitter.base_y));
	save_item(NAME(m_blitter.last_offset));
	save_item(NAME(m_blitter.last_data));
	save_item(NAME(m_blitter.flags));
	save_pointer(NAME(m_blitter.layer_1), 256 * 256);
	save_pointer(NAME(m_blitter.layer_2), 256 * 256);
	save_pointer(NAME(m_blitter.slots), 0x1000);

	save_item(NAME(m_audio_nmi));
	save_item(NAME(m_audio_latch));

	save_item(NAME(m_ppi_output));
}

uint8_t vampire_state::fsoundlatch_r(offs_t offset)
{
	return m_audio_latch;
}

void vampire_state::soundlatch_w(offs_t offset, uint8_t data)
{
	m_audio_nmi = 0x00;
	m_audio_latch = data;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);	
	machine().scheduler().perfect_quantum(attotime::from_usec(10));
}

void vampire_state::blitter_flags_w(offs_t offset, uint8_t data)
{
	m_blitter.flags = data;
}

void vampire_state::blitter_control_w(offs_t offset, uint8_t data)
{
	if ((!(offset & 1)))
	{
		m_blitter.base_y = data;
	}
	else
	{
		m_blitter.base_x = data;
		blit_trigger(m_blitter.slots.get(), m_blitter.last_offset);
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

uint8_t vampire_state::sound_ack_r(offs_t offset) 
{
	return m_audio_nmi;
}


//Main CPU NMI 
uint8_t vampire_state::io814_r(offs_t offset) 
{
	//bit 1
	return machine().rand();
}

uint8_t vampire_state::io815_r(offs_t offset) 
{
	return machine().rand();
}

uint32_t vampire_state::screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	//replace with data copy
	for (int y = 0; y < 256; ++y)
	{
		for (int x = 0; x < 256; ++x)
		{
			bitmap.pix(y, x) = (m_blitter.flags & 0x10) ? m_blitter.layer_1[256 * y + x] : m_blitter.layer_2[256 * y + x];
		}
	}
	return 0;
}

uint8_t vampire_state::blitter_r(offs_t offset)
{
	return m_blitter.slots[offset];
}

void vampire_state::blitter_w(offs_t offset, uint8_t data)
{
	m_blitter.last_offset = offset;

	if (m_blitter.flags & 0x40)
	{
		m_blitter.last_data = data; // not used ?
		return;
	}

	m_blitter.slots[offset] = data;
}

void vampire_state::blit_trigger(uint8_t* blitterdata, uint16_t data)
{
	uint8_t* ptr = (m_blitter.flags & 0x10) ? m_blitter.layer_2.get() : m_blitter.layer_1.get();
	uint32_t index = data >> 3; //start of the 8 byte-long slot

	while (index < 0x200)
	{
		const uint8_t* slot = &m_blitter.slots[index << 3];

		uint32_t desty = slot[6]; // pixels
		const uint32_t destx = slot[7]; // pixels

		const uint32_t start_offset = ((slot[0] << 8) | slot[1]) << 3; // start offset
		const uint8_t pen = slot[5]; // 0xf is max used

		const int32_t sy = 256 - slot[2]; // pixels
		const int32_t sx = ((32 - slot[3])) << 3; // bytes (src ROM)

		for (int yy = 0; yy < sy; ++yy)
		{
			for (int xx = 0; xx < sx; ++xx)
			{
				const uint32_t srcptr = start_offset + 256 * yy + xx;
				const uint32_t dstptr = (desty + m_blitter.base_y) * 256 + destx + xx + m_blitter.base_x;

				if (desty + m_blitter.base_y > 255) continue;
				if (destx + xx + m_blitter.base_x > 255) continue;

				uint8_t pix = 0;

				if (dstptr < 256 * 256)
				{
					pix = m_gfxrom[(srcptr >> 3) & 0x3fff] & (1 << (7 - (srcptr & 7)));
				}

				// when source is above the 256x512 (ROM size) image = solid fill (rom read returns 0xff ? - could be handled by extended rom region , ff filled)
				if (slot[0] & 0x80)
					pix = 1;

				if (pix)
				{
					ptr[dstptr] = pen;
				}
			}
			desty += (slot[4] & 0x20) ? 1 : -1; //flip
		}
		if (slot[4] & 0x80)
			break;	// end mark

		++index;
	}
}

void vampire_state::vampire_memory(address_map &map)
{
	map(0x07f0,0x07f0).portr("IN1");
	map(0x07f1,0x07f1).portr("IN2");
	map(0x07f2,0x07f2).portr("DSW1");
	map(0x07f3,0x07f3).portr("DSW2");
	map(0x07f4,0x07f4).w(FUNC(vampire_state::soundlatch_w));
	map(0x07f5,0x07f5).r(FUNC(vampire_state::sound_ack_r));
	map(0x07f8,0x07f8).w(FUNC(vampire_state::blitter_flags_w));
	map(0x07fa,0x07fb).w(FUNC(vampire_state::blitter_control_w));

	map(0x0814,0x0814).r(FUNC(vampire_state::io814_r));
	map(0x0815,0x0815).r(FUNC(vampire_state::io815_r));

	map(0x1000,0x1fff).r(FUNC(vampire_state::blitter_r)).w(FUNC( vampire_state::blitter_w));
	map(0x2000, 0x3fff).ram();
	map(0x8000, 0xffff).rom();
}

void vampire_state::pit_out_w0(int state)
{
	m_ppi_output = state;
}

void vampire_state::pit_out_w1(int state)
{
	//unknown
}

void vampire_state::pit_out_w2(int state)
{
	//unknown
}

uint8_t vampire_state::sound_sync_r(offs_t offset)
{
	return m_ppi_output << 6;
}

void vampire_state::audioack_w(offs_t offset, uint8_t state)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_audio_nmi = 0x80;
}

void vampire_state::vampire_audio(address_map &map)
{	
	map(0x0000, 0x03ff).ram();
	map(0x4000, 0x4000).ram(); //unk write
	map(0x6000, 0x6000).r(FUNC(vampire_state::fsoundlatch_r));
	map(0x8000, 0x8000).w(FUNC(vampire_state::audioack_w));
	map(0xa000, 0xa001).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0xa00c, 0xa00c).r(FUNC(vampire_state::sound_sync_r));
	map(0xa008, 0xa00b).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf000, 0xffff).rom();
}

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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) // $d831
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )	
	PORT_DIPNAME( 0x40, 0x40, "Infinite Time" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) ) 
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) ) 
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) ) 
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Infinite Lives" ) 
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

#define MAIN_XTAL    (8_MHz_XTAL)
#define AUDIO_XTAL   (4_MHz_XTAL)
#define PIT_CLOCK (AUDIO_XTAL/2)

void vampire_state::vampire(machine_config &config)
{
	MC6809(config, m_maincpu, MAIN_XTAL); // HD68B09P
	m_maincpu->set_addrmap(AS_PROGRAM, &vampire_state::vampire_memory);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(PIT_CLOCK);
	pit.set_clk<1>(PIT_CLOCK);
	pit.set_clk<2>(PIT_CLOCK);
	pit.out_handler<0>().set(FUNC(vampire_state::pit_out_w0));
	pit.out_handler<1>().set(FUNC(vampire_state::pit_out_w1));
	pit.out_handler<2>().set(FUNC(vampire_state::pit_out_w2));

	M6502(config, m_audiocpu, AUDIO_XTAL/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &vampire_state::vampire_audio);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0*8, 32*8 - 1, 2*8, 30*8 - 1);
	screen.set_screen_update(FUNC(vampire_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, M6809_FIRQ_LINE, HOLD_LINE);

	screen.set_palette(m_palette);
	PALETTE(config, m_palette, FUNC(vampire_state::palette), 256);

	SPEAKER(config, "speaker").front_center();
	AY8912(config, "aysnd", AUDIO_XTAL/4).add_route(ALL_OUTPUTS, "speaker", 0.5);
}

ROM_START( vampire )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h1.bin",     0x8000, 0x2000, CRC(7e69ff9b) SHA1(85f6303803a0577f96b879e0b9d280a4f22cca8c) )
	ROM_LOAD( "h2.bin",     0xa000, 0x2000, CRC(e94155f8) SHA1(f60d82d9204f7b13aa9be189ca74ca48bf9e879a) )
	ROM_LOAD( "h3.bin",     0xc000, 0x2000, CRC(ce27dd90) SHA1(4edd8ba08f828a0c3c6eb21a2d9e0d1e53fac407) )
	ROM_LOAD( "h4.bin",     0xe000, 0x2000, CRC(a25f00bc) SHA1(db445c1876b79c4d553d4f9cc881f41f68b6667c) )
	
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "s9d.bin",     0xf000, 0x1000, CRC(e13a7aef) SHA1(77a49cb0f3f037826a32bbfa8fab524f17895992) )
	
	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "p1.bin",     0x0000, 0x2000, CRC(042661a4) SHA1(58ed7c782a2486aa8c2c650d6d9e54929e5dd50b) )
	ROM_LOAD( "p2.bin",     0x2000, 0x2000, CRC(e9dd9dff) SHA1(8a9ad8659763a9c010d0b482af3a40978c02cec7) )
	
	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD_NIB_LOW(  "16a.bin",  0x0000, 0x0100, CRC(bc60a2eb) SHA1(73b8f5f6eee90a4d59a5c62a6a342fa49185938c) )
	ROM_LOAD_NIB_HIGH( "16b.bin",  0x0000, 0x0100, CRC(aa6b627b) SHA1(556d6bab46419ce55dd254de9615981852be6e6f) )
ROM_END

} 

void vampire_state::palette(palette_device& palette) const
{
	uint8_t* proms = memregion("proms")->base();
	for (int i = 0; i < 256; ++i)
	{
		int g = ((proms[i] & 0b00000011) >> 0) * 85;
		int r = ((proms[i] & 0b00011100) >> 2) * 36;
		int b = ((proms[i] & 0b11100000) >> 5) * 36;
		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void vampire_state::init_vampire()
{
	uint8_t* rom = memregion("maincpu")->base();
	//hack interrupt vectors
	rom[0xfff6] = rom[0xffe0];
	rom[0xfff7] = rom[0xffe1];
	rom[0xfff8] = rom[0xffe2];
	rom[0xfff9] = rom[0xffe3];
}

// it is unclear if the game saw a wide release, or was only location tested
GAME( 1983, vampire,  0,        vampire, vampire, vampire_state, init_vampire, ROT90, "Entertainment Enterprises Ltd.",   "Vampire (prototype?)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
