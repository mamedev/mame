// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Cue Brick (c) Konami 1989
Missing In Action (c) Konami 1989
Teenage Mutant Ninja Turtles (c) Konami 1989

driver by Nicola Salmoria

***************************************************************************

Teenage Mutant Ninja Turtles, Konami, 1989
Hardware info by Guru

PCB Layout
----------

GX963
PWB351853A
|------------------------------------------------------------------|
|  MB3731         963A25.D5                 963A17.H4    963A15.K4 |
|     VOL                                                          |
|                                           963A18.H6    963A16.K6 |
|                                 963A30.G7                        |
|   007341                                 |--------|   |--------| |
|   007341 4066                MCM2018     |KONAMI  |   |KONAMI  | |
|          4066     640kHz                 |051960  |   |051937  | |
|                           3.579545MHz    |        |   |        | |
|      LM358  Y3014   |-------------|      |--------|   |--------| |
|J  C324              |   007232    |                              |
|A         963A26.C13 |-------------| 963E20.G13     007644 007644 |
|M        C324        007340  007340           MB8464     MB8464   |
|M  LM358      640kHz               D780C-1  963X21.J15  963X22.K15|
|A         D7759C      MB8416                963X23.I17  963X24.K17|
|                                           |-------------------|  |
|          963A27.D18  YM2151     24MHz     |      68000        |  |
|   051550                   963A31.G19     |-------------------|  |
|                                                                  |
|   052535(X3)                             |--------|   |--------| |
|   005273(X10)               MCM2018      |KONAMI  |   |KONAMI  | |
|                             MCM2018      |052109  |   |051962  | |
|  CN4                                     |        |   |        | |
|  CN3                                     |--------|   |--------| |
|                                MB8464                            |
|DIPSW1 DIPSW2 DIPSW3            MB8464     963A28.H27   963A29.K27|
|------------------------------------------------------------------|
Notes:
       68000 - Motorola MC68000P8 CPU. Clock input 8.000MHz [24/3]
     D780C-1 - NEC D780C-1 Z80-compatible CPU. Clock input 3.579545MHz
      D7759C - NEC uPD7759C ADPCM Speech Synthesizer. Clock input 640kHz on pin 23
      YM2151 - Yamaha YM2151 FM Operator Type-M (OPM) sound chip. Clock input 3.579545MHz
       LM358 - Motorola LM358 Dual Operational Amplifier
       Y3014 - Yamaha YM3014B Serial Input Floating D/A Converter. Clock input 320kHz [640/2] on pin 5
        C324 - NEC uPC324 Quad Operational Amplifier (equivalent to LM324)
      MB8464 - Fujitsu MB8464 8kBx8-bit SRAM
     MCM2018 - Motorola MCM2018 2kBx8-bit SRAM
      MB8416 - Fujitsu MB8416 2kBx8-bit SRAM
      051550 - Custom ceramic module providing coin counter drivers, watchdog timer and master reset signal
      MB3731 - Fujitsu MB3731 Audio Power Amplifier
         VOL - 5K-ohm volume pot
        4066 - Oki M4066 Quad Bilateral Switch
      007340 - Konami custom resistor array pack
      007341 - Konami custom resistor array pack
      052535 - Konami custom resistor array pack
      005273 - Konami custom resistor array pack
      007232 - Konami custom PCM Controller/Sample Player. Clock input 3.579545MHz on pin 51
      052109 - Konami custom Tilemap Generator \
      051962 - Konami custom Tilemap Generator / paired together
      051960 - Konami custom Sprite Generator \
      051937 - Konami custom Sprite Generator / paired together
      007644 - Konami custom (unknown purpose) DIP22 400mil-wide IC
  963A27.D18 - 1Mbit 28-pin mask ROM (uPD7759 samples)
  963A26.C13 - 1Mbit 28-pin mask ROM (007232 PCM samples)
   963A25.D5 - 4Mbit 40-pin mask ROM (title theme sample for uPD7759)
  963E20.G13 - 32kBx8-bit (27256) OTP EPROM (Z80 program)
   963A17.H4 \
   963A15.K4  \
   963A18.H6  / 4Mbit 40-pin mask ROM (sprite data)
   963A16.K6 /
  963A28.H27 \
  963A29.K27 / 4Mbit 40-pin mask ROM (tile data)
  963A31.G19 - AMD27S21 Bi-polar PROM (priority encoder)
   963A30.G7 - AMD27S21 Bi-polar PROM (sprite address decoder)
    DIPDW1/2 - 8-position DIP switch
      DIPSW3 - 4-position DIP switch
       CN3/4 - 15-position connector for player 3 & 4 controls
       HSync - 14.9626kHz
       VSync - 59.1846Hz

***************************************************************************/

#include "emu.h"

#include "k052109.h"
#include "k051960.h"
#include "konamipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "sound/msm5205.h"
#include "sound/samples.h"
#include "sound/upd7759.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "ymfm/src/ymfm.h" // decode_fp

namespace {

class tmnt_state : public driver_device
{
public:
	tmnt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_nvrambank(*this, "nvrambank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232(*this, "k007232"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_upd7759(*this, "upd"),
		m_samples(*this, "samples"),
		m_palette(*this, "palette")
	{ }

	void cuebrick(machine_config &config);
	void tmnt(machine_config &config);
	void tmntucbl(machine_config &config);
	void mia(machine_config &config);

	void init_mia();
	void init_tmnt();
	void init_cuebrick();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// memory pointers
	optional_memory_bank m_nvrambank;

	// video-related
	int        m_layer_colorbase[3]{};
	int        m_sprite_colorbase = 0;
	int        m_tmnt_priorityflag = 0;

	// misc
	int        m_tmnt_soundlatch = 0;
	int        m_last = 0;
	uint16_t   m_cuebrick_nvram[0x400 * 0x20 / 2]{}; // 32k paged in a 1k window

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	optional_device<upd7759_device> m_upd7759;
	optional_device<samples_device> m_samples;
	required_device<palette_device> m_palette;

	// memory buffers
	int16_t      m_sampledata[0x40000];

	uint8_t      m_irq5_mask = 0;
	uint16_t k052109_word_noA12_r(offs_t offset, uint16_t mem_mask = ~0);
	void k052109_word_noA12_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t tmnt_sres_r();
	void tmnt_sres_w(uint8_t data);
	void cuebrick_nvbank_w(uint8_t data);
	void tmnt_0a0000_w(offs_t offset, uint16_t data);
	void tmnt_priority_w(offs_t offset, uint16_t data);
	void tmnt_upd_start_w(uint8_t data);
	uint8_t tmnt_upd_busy_r();

	DECLARE_VIDEO_START(cuebrick);
	DECLARE_VIDEO_START(mia);
	DECLARE_MACHINE_RESET(tmnt);
	DECLARE_VIDEO_START(tmnt);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tmnt_vblank_w(int state);
	void volume_callback(uint8_t data);
	K051960_CB_MEMBER(mia_sprite_callback);
	K051960_CB_MEMBER(tmnt_sprite_callback);
	K052109_CB_MEMBER(mia_tile_callback);
	K052109_CB_MEMBER(cuebrick_tile_callback);
	K052109_CB_MEMBER(tmnt_tile_callback);
	SAMPLES_START_CB_MEMBER(tmnt_decode_sample);

	void cuebrick_main_map(address_map &map) ATTR_COLD;
	void mia_audio_map(address_map &map) ATTR_COLD;
	void mia_main_map(address_map &map) ATTR_COLD;
	void tmnt_audio_map(address_map &map) ATTR_COLD;
	void tmntucbl_audio_map(address_map &map) ATTR_COLD;
	void tmnt_main_map(address_map &map) ATTR_COLD;
};

uint16_t tmnt_state::k052109_word_noA12_r(offs_t offset, uint16_t mem_mask)
{
	/* some games have the A12 line not connected, so the chip spans */
	/* twice the memory range, with mirroring */
	offset = ((offset & 0x3000) >> 1) | (offset & 0x07ff);
	if (ACCESSING_BITS_8_15)
		return m_k052109->read(offset) << 8;
	else
		return m_k052109->read(offset + 0x2000);
}

void tmnt_state::k052109_word_noA12_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* some games have the A12 line not connected, so the chip spans */
	/* twice the memory range, with mirroring */
	offset = ((offset & 0x3000) >> 1) | (offset & 0x07ff);
	if (ACCESSING_BITS_8_15)
		m_k052109->write(offset, (data >> 8) & 0xff);
	else
		m_k052109->write(offset + 0x2000, data & 0xff);
}


void tmnt_state::tmnt_vblank_w(int state)
{
	if (state && m_irq5_mask)
		m_maincpu->set_input_line(M68K_IRQ_5, ASSERT_LINE);
}


uint8_t tmnt_state::tmnt_sres_r()
{
	return m_tmnt_soundlatch;
}

void tmnt_state::tmnt_sres_w(uint8_t data)
{
	/* bit 1 resets the UPD7795C sound chip */
	m_upd7759->reset_w(BIT(data, 1));

	/* bit 2 plays the title music */
	if (BIT(data, 2))
	{
		if (!m_samples->playing(0))
			m_samples->start_raw(0, m_sampledata, 0x40000, 640000 / 32);
	}
	else
		m_samples->stop(0);
	m_tmnt_soundlatch = data;
}

void tmnt_state::tmnt_upd_start_w(uint8_t data)
{
	m_upd7759->start_w(!BIT(data, 0));
}

uint8_t tmnt_state::tmnt_upd_busy_r()
{
	return m_upd7759->busy_r() ? 1 : 0;
}

SAMPLES_START_CB_MEMBER(tmnt_state::tmnt_decode_sample)
{
	// using MAME samples to HLE the title music
	// to put it briefly, it's like this on the PCB:
	// 640kHz XTAL -> 74161 and 3 74393 -> ROM address -> ROM output to 2 74166 -> YM3014
	uint8_t *source = memregion("title")->base();

	// sample data is encoded in Yamaha FP format
	for (int i = 0; i < 0x40000; i++)
	{
		int val = source[2 * i] + source[2 * i + 1] * 256;
		m_sampledata[i] = ymfm::decode_fp(val >> 3);
	}
}



/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

// Missing in Action

K052109_CB_MEMBER(tmnt_state::mia_tile_callback)
{
	*flags = (*color & 0x04) ? TILE_FLIPX : 0;
	if (layer == 0)
	{
		*code |= ((*color & 0x01) << 8);
		*color = m_layer_colorbase[layer] + ((*color & 0x80) >> 5) + ((*color & 0x10) >> 1);
	}
	else
	{
		*code |= ((*color & 0x01) << 8) | ((*color & 0x18) << 6) | (bank << 11);
		*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
	}
}

K052109_CB_MEMBER(tmnt_state::cuebrick_tile_callback)
{
	if ((m_k052109->get_rmrd_line() == CLEAR_LINE) && (layer == 0))
	{
		*code |= ((*color & 0x01) << 8);
		*color = m_layer_colorbase[layer]  + ((*color & 0x0e) >> 1);
	}
	else
	{
		*code |= ((*color & 0xf) << 8);
		*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
	}
}

K052109_CB_MEMBER(tmnt_state::tmnt_tile_callback)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}



/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(tmnt_state::mia_sprite_callback)
{
	*color = m_sprite_colorbase + (*color & 0x0f);
}

K051960_CB_MEMBER(tmnt_state::tmnt_sprite_callback)
{
	*code |= (*color & 0x10) << 9;
	*color = m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(tmnt_state,cuebrick)
{
	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 32;
	m_layer_colorbase[2] = 40;
	m_sprite_colorbase = 16;
}

VIDEO_START_MEMBER(tmnt_state,mia)
{
	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 32;
	m_layer_colorbase[2] = 40;
	m_sprite_colorbase = 16;

	m_tmnt_priorityflag = 0;
	save_item(NAME(m_tmnt_priorityflag));
}

VIDEO_START_MEMBER(tmnt_state,tmnt)
{
	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 32;
	m_layer_colorbase[2] = 40;
	m_sprite_colorbase = 16;

	m_tmnt_priorityflag = 0;
	save_item(NAME(m_tmnt_priorityflag));

	m_palette->set_shadow_factor(0.75);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void tmnt_state::tmnt_0a0000_w(offs_t offset, uint16_t data)
{
	/* bit 0/1 = coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);  /* 2 players version */

	/* bit 3 high then low triggers irq on sound CPU */
	if (m_last == 0x08 && (data & 0x08) == 0)
		m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80

	m_last = data & 0x08;

	/* bit 5 = irq enable */
	m_irq5_mask = data & 0x20;
	if (!m_irq5_mask)
		m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);

	/* bit 7 = enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line((data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

	/* other bits unused */
}

void tmnt_state::tmnt_priority_w(offs_t offset, uint16_t data)
{
	/* bit 2/3 = priority; other bits unused */
	/* bit2 = PRI bit3 = PRI2
	      sprite/playfield priority is controlled by these two bits, by bit 3
	      of the background tile color code, and by the SHADOW sprite
	      attribute bit.
	      Priorities are encoded in a PROM (G19 for TMNT). However, in TMNT,
	      the PROM only takes into account the PRI and SHADOW bits.
	      PRI  Priority
	       0   bg fg spr text
	       1   bg spr fg text
	      The SHADOW bit, when set, torns a sprite into a shadow which makes
	      color below it darker (this is done by turning off three resistors
	      in parallel with the RGB output).

	      Note: the background color (color used when all of the four layers
	      are 0) is taken from the *foreground* palette, not the background
	      one as would be more intuitive.
	*/
	m_tmnt_priorityflag = (data & 0x0c) >> 2;
}



/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t tmnt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE,0);
	if ((m_tmnt_priorityflag & 1) == 1) m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	if ((m_tmnt_priorityflag & 1) == 0) m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);

	return 0;
}


void tmnt_state::cuebrick_nvbank_w(uint8_t data)
{
	m_nvrambank->set_entry(data);
}

void tmnt_state::cuebrick_main_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x040000, 0x043fff).ram(); /* main RAM */
	map(0x060000, 0x063fff).ram(); /* main RAM */
	map(0x080000, 0x080fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x0a0000, 0x0a0001).portr("COINS").w(FUNC(tmnt_state::tmnt_0a0000_w));
	map(0x0a0002, 0x0a0003).portr("P1");
	map(0x0a0004, 0x0a0005).portr("P2");
	map(0x0a0010, 0x0a0011).portr("DSW2").w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x0a0012, 0x0a0013).portr("DSW1");
	map(0x0a0018, 0x0a0019).portr("DSW3");
	map(0x0b0000, 0x0b03ff).bankrw("nvrambank");
	map(0x0b0400, 0x0b0400).w(FUNC(tmnt_state::cuebrick_nvbank_w));
	map(0x0c0000, 0x0c0003).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0xff00);
	map(0x100000, 0x107fff).rw(FUNC(tmnt_state::k052109_word_noA12_r), FUNC(tmnt_state::k052109_word_noA12_w));
	map(0x140000, 0x140007).rw(m_k051960, FUNC(k051960_device::k051937_r), FUNC(k051960_device::k051937_w));
	map(0x140400, 0x1407ff).rw(m_k051960, FUNC(k051960_device::k051960_r), FUNC(k051960_device::k051960_w));
}


void tmnt_state::mia_main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x043fff).ram(); /* main RAM */
	map(0x060000, 0x063fff).ram(); /* main RAM */
	map(0x080000, 0x080fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x0a0000, 0x0a0001).portr("COINS").w(FUNC(tmnt_state::tmnt_0a0000_w));
	map(0x0a0002, 0x0a0003).portr("P1");
	map(0x0a0004, 0x0a0005).portr("P2");
	map(0x0a0009, 0x0a0009).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x0a0010, 0x0a0011).portr("DSW1").w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x0a0012, 0x0a0013).portr("DSW2");
	map(0x0a0018, 0x0a0019).portr("DSW3");
#if 0
	map(0x0c0000, 0x0c0001).w(FUNC(tmnt_state::tmnt_priority_w));
#endif
	map(0x100000, 0x107fff).rw(FUNC(tmnt_state::k052109_word_noA12_r), FUNC(tmnt_state::k052109_word_noA12_w));
//  map(0x10e800, 0x10e801).nopw(); ???
	map(0x140000, 0x140007).rw(m_k051960, FUNC(k051960_device::k051937_r), FUNC(k051960_device::k051937_w));
	map(0x140400, 0x1407ff).rw(m_k051960, FUNC(k051960_device::k051960_r), FUNC(k051960_device::k051960_w));
}


void tmnt_state::tmnt_main_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
	map(0x060000, 0x063fff).ram(); /* main RAM */
	map(0x080000, 0x080fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x0a0000, 0x0a0001).portr("COINS").w(FUNC(tmnt_state::tmnt_0a0000_w));
	map(0x0a0002, 0x0a0003).portr("P1");
	map(0x0a0004, 0x0a0005).portr("P2");
	map(0x0a0006, 0x0a0007).portr("P3");
	map(0x0a0009, 0x0a0009).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x0a0010, 0x0a0011).portr("DSW1").w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x0a0012, 0x0a0013).portr("DSW2");
	map(0x0a0014, 0x0a0015).portr("P4");
	map(0x0a0018, 0x0a0019).portr("DSW3");
	map(0x0c0000, 0x0c0001).w(FUNC(tmnt_state::tmnt_priority_w));
	map(0x100000, 0x107fff).rw(FUNC(tmnt_state::k052109_word_noA12_r), FUNC(tmnt_state::k052109_word_noA12_w));
//  map(0x10e800, 0x10e801).nopw(); ???
	map(0x140000, 0x140007).rw(m_k051960, FUNC(k051960_device::k051937_r), FUNC(k051960_device::k051937_w));
	map(0x140400, 0x1407ff).rw(m_k051960, FUNC(k051960_device::k051960_r), FUNC(k051960_device::k051960_w));
}


void tmnt_state::mia_audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
}


void tmnt_state::tmnt_audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).rw(FUNC(tmnt_state::tmnt_sres_r), FUNC(tmnt_state::tmnt_sres_w)); /* title music & UPD7759C reset */
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xd000, 0xd000).w(m_upd7759, FUNC(upd7759_device::port_w));
	map(0xe000, 0xe000).w(FUNC(tmnt_state::tmnt_upd_start_w));
	map(0xf000, 0xf000).r(FUNC(tmnt_state::tmnt_upd_busy_r));
}


void tmnt_state::tmntucbl_audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).rw(FUNC(tmnt_state::tmnt_sres_r), FUNC(tmnt_state::tmnt_sres_w)); /* title music & UPD7759C reset */
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	// TODO: MC68705R3P + Oki M5205
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xd000, 0xd000).w(m_upd7759, FUNC(upd7759_device::port_w));
	map(0xe000, 0xe000).w(FUNC(tmnt_state::tmnt_upd_start_w));
	map(0xf000, 0xf000).r(FUNC(tmnt_state::tmnt_upd_busy_r));
}


static INPUT_PORTS_START( cuebrick )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	KONAMI16_LSB( 1, IPT_BUTTON3, IPT_UNUSED )

	PORT_START("P2")
	KONAMI16_LSB( 2, IPT_BUTTON3, IPT_UNUSED )

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" ) // manual says "not used"
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" ) // manual says "not used"
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x08, "Machine Name" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "Lewis" )
	PORT_DIPSETTING(    0x08, "Johnson" )
	PORT_DIPSETTING(    0x00, "George" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Upright Controls" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW3:4" )
INPUT_PORTS_END

static INPUT_PORTS_START( mia )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	KONAMI16_LSB( 1, IPT_BUTTON3, IPT_UNUSED )

	PORT_START("P2")
	KONAMI16_LSB( 2, IPT_BUTTON3, IPT_UNUSED )

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30K, Every 80K" )    // Japan factory default
	PORT_DIPSETTING(    0x10, "50K, Every 100K" )
	PORT_DIPSETTING(    0x08, "50K Only" )          // US factory default
	PORT_DIPSETTING(    0x00, "100K Only" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )       // Japan factory default
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )    // US factory default
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "VRAM Character Check" ) PORT_DIPLOCATION("SW3:2") // JP manual says "not used"
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW3:4" )
INPUT_PORTS_END

static INPUT_PORTS_START( tmnt )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_START("P1")
	KONAMI16_LSB( 1, IPT_UNKNOWN, IPT_UNKNOWN )

	PORT_START("P2")
	KONAMI16_LSB( 2, IPT_UNKNOWN, IPT_UNKNOWN )

	PORT_START("P3")
	KONAMI16_LSB( 3, IPT_UNKNOWN, IPT_UNKNOWN )

	PORT_START("P4")
	KONAMI16_LSB( 4, IPT_UNKNOWN, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" ) // manual says "not used", but doesn't "should be kept OFF"
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" ) // ditto
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" ) // ditto
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" ) // ditto

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" ) // manual says "not used", but doesn't "should be kept OFF"
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" ) // ditto
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" ) // ditto
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW3:2" ) // manual says "not used and should be kept OFF"
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW3:4" ) // ditto
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tmnt2p )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI16_LSB( 1, IPT_UNKNOWN, IPT_START1 )

	PORT_START("P2")
	KONAMI16_LSB( 2, IPT_UNKNOWN, IPT_START2 )

	PORT_START("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" ) // US and Japan factory default = "2"
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" ) // manual says "not used", but doesn't "should be kept OFF"
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" ) // ditto
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" ) // ditto
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW3:2" ) // manual says "not used and should be kept OFF"
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW3:4" ) // ditto
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void tmnt_state::volume_callback(uint8_t data)
{
	m_k007232->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void tmnt_state::machine_start()
{
	save_item(NAME(m_tmnt_soundlatch));
	save_item(NAME(m_last));
	save_item(NAME(m_irq5_mask));
}

void tmnt_state::machine_reset()
{
	m_last = 0;
	m_tmnt_soundlatch = 0;
	m_irq5_mask = 0;
	m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
}

void tmnt_state::cuebrick(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 8000000);    /* 8 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &tmnt_state::cuebrick_main_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(13*8, (64-13)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tmnt_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(tmnt_state::tmnt_vblank_w));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);
	m_palette->set_membits(8);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MCFG_VIDEO_START_OVERRIDE(tmnt_state,cuebrick)

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen(nullptr);
	m_k052109->set_tile_callback(FUNC(tmnt_state::cuebrick_tile_callback));

	K051960(config, m_k051960, 0);
	m_k051960->set_palette(m_palette);
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(tmnt_state::mia_sprite_callback));
	m_k051960->set_plane_order(K051960_PLANEORDER_MIA);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(3'579'545)));
	ymsnd.irq_handler().set_inputline(m_maincpu, M68K_IRQ_6);
	ymsnd.add_route(0, "mono", 1.0);
	ymsnd.add_route(1, "mono", 1.0);
}

void tmnt_state::mia(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &tmnt_state::mia_main_map);

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &tmnt_state::mia_audio_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(13*8, (64-13)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tmnt_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(tmnt_state::tmnt_vblank_w));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);
	m_palette->set_membits(8);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	MCFG_VIDEO_START_OVERRIDE(tmnt_state,mia)

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen(nullptr);
	m_k052109->set_tile_callback(FUNC(tmnt_state::mia_tile_callback));

	K051960(config, m_k051960, 0);
	m_k051960->set_palette(m_palette);
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(tmnt_state::mia_sprite_callback));
	m_k051960->set_plane_order(K051960_PLANEORDER_MIA);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "mono", 1.0).add_route(1, "mono", 1.0);

	K007232(config, m_k007232, XTAL(3'579'545));
	m_k007232->port_write().set(FUNC(tmnt_state::volume_callback));
	m_k007232->add_route(0, "mono", 0.20);
	m_k007232->add_route(1, "mono", 0.20);
}

MACHINE_RESET_MEMBER(tmnt_state,tmnt)
{
	machine_reset();

	/* the UPD7759 control flip-flops are cleared: /ST is 1, /RESET is 0 */
	m_upd7759->start_w(1);
	m_upd7759->reset_w(0);
}

void tmnt_state::tmnt(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &tmnt_state::tmnt_main_map);

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &tmnt_state::tmnt_audio_map);

	MCFG_MACHINE_RESET_OVERRIDE(tmnt_state,tmnt)

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(12*8, (64-12)*8-1, 2*8, 30*8-1 );
	// verified against real hardware

	screen.set_screen_update(FUNC(tmnt_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(tmnt_state::tmnt_vblank_w)); // NVBLK from 051962

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);
	m_palette->set_membits(8);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	MCFG_VIDEO_START_OVERRIDE(tmnt_state,tmnt)

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen(nullptr);
	m_k052109->set_tile_callback(FUNC(tmnt_state::tmnt_tile_callback));

	K051960(config, m_k051960, 0);
	m_k051960->set_palette(m_palette);
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(tmnt_state::tmnt_sprite_callback));
	m_k051960->set_plane_order(K051960_PLANEORDER_MIA);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "mono", 1.0).add_route(1, "mono", 1.0);

	K007232(config, m_k007232, XTAL(3'579'545));
	m_k007232->port_write().set(FUNC(tmnt_state::volume_callback));
	m_k007232->add_route(0, "mono", 0.33);
	m_k007232->add_route(1, "mono", 0.33);

	UPD7759(config, "upd", XTAL(640'000)).add_route(ALL_OUTPUTS, "mono", 0.60);

	SAMPLES(config, m_samples);
	m_samples->set_channels(1); /* 1 channel for the title music */
	m_samples->set_samples_start_callback(FUNC(tmnt_state::tmnt_decode_sample));
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void tmnt_state::tmntucbl(machine_config &config)
{
	tmnt(config);

	m_audiocpu->set_addrmap(AS_PROGRAM, &tmnt_state::tmntucbl_audio_map);

	M68705R3(config, "mcu", XTAL(4'000'000)).set_disable(); // not dumped

	MSM5205(config, "msm", 384'000).add_route(ALL_OUTPUTS, "mono", 0.5); // TODO: hook up, frequency unknown

	config.device_remove("k007232");
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cuebrick )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "903d25.g12",   0x00000, 0x10000, CRC(8d575663) SHA1(0e308e04936efa80351bf808ac304d3fcc82f19a) )
	ROM_LOAD16_BYTE( "903d24.f12",   0x00001, 0x10000, CRC(2973625d) SHA1(e2496704390930761204624d4bf6b0b68d3133ab) )

	ROM_REGION( 0x40000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "903c29.k21",  0x000000, 0x10000, CRC(fada986d) SHA1(79d13dcee5433457c25a8cca0093bddd55165a72) )
	ROM_LOAD32_BYTE( "903c28.k19",  0x000001, 0x10000, CRC(80d2bfaf) SHA1(3b38558d4f17309154457e9e7780a25577d1858d) )
	ROM_LOAD32_BYTE( "903c27.k17",  0x000002, 0x10000, CRC(5bd4b8e1) SHA1(0bc5e508af20e479c7913fab1ef158165fe67079) )
	ROM_LOAD32_BYTE( "903c26.k15",  0x000003, 0x10000, CRC(f808fa3d) SHA1(2b0fa1581acc5c4f7055e6faad97664ef16cc082) )

	ROM_REGION( 0x40000, "k051960", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "903d23.k12",  0x000000, 0x10000, CRC(c39fc9fd) SHA1(fe5a63e5d898f985f9ab9be5b701af4a8e2a9049) )
	ROM_LOAD32_BYTE( "903d22.k10",  0x000001, 0x10000, CRC(95ad8591) SHA1(4e3c8c794be1cd78044eb0eebfa3c755e2aaf54f) )
	ROM_LOAD32_BYTE( "903d21.k8",   0x000002, 0x10000, CRC(3c7bf8cd) SHA1(c487e0109f56b3b0e2aa2c4db2dfb30ad74fb0ab) )
	ROM_LOAD32_BYTE( "903d20.k6",   0x000003, 0x10000, CRC(2872a1bb) SHA1(da7c7a41860283eac49facaa3beb712d3be7db56) )
ROM_END

ROM_START( mia )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "808t20.h17",   0x00000, 0x20000, CRC(6f0acb1d) SHA1(af3447fd4645cb03b1660df2ae076fa53ff81945) )
	ROM_LOAD16_BYTE( "808t21.j17",   0x00001, 0x20000, CRC(42a30416) SHA1(8d9d27de96e79cae5230705beecadff0180cc479) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "808e03.f4",    0x00000, 0x08000, CRC(3d93a7cd) SHA1(dcdd327e78f32436b276d0666f62a5b733b296e8) )

	ROM_REGION( 0x40000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "808e12.f28",   0x000000, 0x10000, CRC(d62f1fde) SHA1(1e55084f1294b6ac7c152fcd1800511fcab5d360) )
	ROM_LOAD32_BYTE( "808e13.h28",   0x000001, 0x10000, CRC(1fa708f4) SHA1(9511a19f50fb61571c2986c72d1a85e87b8d0495) )
	ROM_LOAD32_BYTE( "808e22.i28",   0x000002, 0x10000, CRC(73d758f6) SHA1(69e7079c3178f6f5acae533dae4854808c45bc29) )
	ROM_LOAD32_BYTE( "808e23.k28",   0x000003, 0x10000, CRC(8ff08b21) SHA1(9a8a03a960967f6f1d982b490f1724427538ecac) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "808d17.j4",    0x00000, 0x80000, CRC(d1299082) SHA1(c3c07b0517e7428ccd1cdf9e15aaf16d98e7c4cd) )
	ROM_LOAD32_WORD( "808d15.h4",    0x00002, 0x80000, CRC(2b22a6b6) SHA1(8e1af0627a4eac045128c4096e2cfb59c3d2f5ef) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "808a18.f16",   0x0000, 0x0100, CRC(eb95aede) SHA1(8153eb516ae9753910c6d6a2143e91e079586836) )    /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "808d01.d4",    0x00000, 0x20000, CRC(fd4d37c0) SHA1(ef91c6e7bb57c27a9a51729fffd1bfe3e806fb61) ) /* samples for 007232 */
ROM_END

ROM_START( mia2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "808s20.h17",   0x00000, 0x20000, CRC(caa2897f) SHA1(58f69586d1cd49acf64cf34a69a9ba88dba0923c) )
	ROM_LOAD16_BYTE( "808s21.j17",   0x00001, 0x20000, CRC(3d892ffb) SHA1(f6c0f8aa83f5688c8b57c5a66a481f65a5d4f530) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "808e03.f4",    0x00000, 0x08000, CRC(3d93a7cd) SHA1(dcdd327e78f32436b276d0666f62a5b733b296e8) )

	ROM_REGION( 0x40000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "808e12.f28",   0x000000, 0x10000, CRC(d62f1fde) SHA1(1e55084f1294b6ac7c152fcd1800511fcab5d360) )
	ROM_LOAD32_BYTE( "808e13.h28",   0x000001, 0x10000, CRC(1fa708f4) SHA1(9511a19f50fb61571c2986c72d1a85e87b8d0495) )
	ROM_LOAD32_BYTE( "808e22.i28",   0x000002, 0x10000, CRC(73d758f6) SHA1(69e7079c3178f6f5acae533dae4854808c45bc29) )
	ROM_LOAD32_BYTE( "808e23.k28",   0x000003, 0x10000, CRC(8ff08b21) SHA1(9a8a03a960967f6f1d982b490f1724427538ecac) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "808d17.j4",    0x00000, 0x80000, CRC(d1299082) SHA1(c3c07b0517e7428ccd1cdf9e15aaf16d98e7c4cd) )
	ROM_LOAD32_WORD( "808d15.h4",    0x00002, 0x80000, CRC(2b22a6b6) SHA1(8e1af0627a4eac045128c4096e2cfb59c3d2f5ef) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "808a18.f16",   0x0000, 0x0100, CRC(eb95aede) SHA1(8153eb516ae9753910c6d6a2143e91e079586836) )    /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "808d01.d4",    0x00000, 0x20000, CRC(fd4d37c0) SHA1(ef91c6e7bb57c27a9a51729fffd1bfe3e806fb61) ) /* samples for 007232 */
ROM_END

ROM_START( tmnt )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963-x23.j17",      0x00000, 0x20000, CRC(a9549004) SHA1(bf9be5983af2282f627fb8408c069415c9b90229) )
	ROM_LOAD16_BYTE( "963-x24.k17",      0x00001, 0x20000, CRC(e5cc9067) SHA1(649db4a09864eb8aba44cb77b580f1f28cfd80ed) )
	ROM_LOAD16_BYTE( "963-x21.j15",      0x40000, 0x10000, CRC(5789cf92) SHA1(c1d1c958813062e5df5ac62e90ee4ce11f7e4a24) )
	ROM_LOAD16_BYTE( "963-x22.k15",      0x40001, 0x10000, CRC(0a74e277) SHA1(c349d3c25eb05cc30ec1fd823475d971f3649f8b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmntu )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963-r23.j17",      0x00000, 0x20000, CRC(a7f61195) SHA1(db231ffb045f512040793b6815bcb998cee04c3d) )
	ROM_LOAD16_BYTE( "963-r24.k17",      0x00001, 0x20000, CRC(661e056a) SHA1(4773883a66540c07dbc969881689184697355537) )
	ROM_LOAD16_BYTE( "963-r21.j15",      0x40000, 0x10000, CRC(de047bb6) SHA1(d41d11f1b7dfd3824308f7fff43a5a7ced432ec2) )
	ROM_LOAD16_BYTE( "963-r22.k15",      0x40001, 0x10000, CRC(d86a0888) SHA1(c761b3e8acc45a36ae691758c639eb826a8ab5b2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmntua )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963-n23.j17",      0x00000, 0x20000, CRC(388c333f) SHA1(551039ae1b258d9aa422789ce5f4f241d835f847) )
	ROM_LOAD16_BYTE( "963-n24.k17",      0x00001, 0x20000, CRC(af3efd63) SHA1(10d0587645b5a12654af92b5f790b6da2a35d74d) )
	ROM_LOAD16_BYTE( "963-j21.j15",      0x40000, 0x10000, CRC(7bee9fe8) SHA1(1489cbd81176a586d21442d3e9cf4e585ca72bb4) )
	ROM_LOAD16_BYTE( "963-j22.k15",      0x40001, 0x10000, CRC(2efed09f) SHA1(be84f71a076b360708f15b555ffb8612eb7f0f08) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmntub )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963-j23.j17",      0x00000, 0x20000, CRC(f77314e2) SHA1(aeb7a397a17b6ff587e3c536286a4942975e7a20) )
	ROM_LOAD16_BYTE( "963-j24.k17",      0x00001, 0x20000, CRC(47f662d3) SHA1(d26e932b13920ca23a654a647b1e02097a264a3a) )
	ROM_LOAD16_BYTE( "963-j21.j15",      0x40000, 0x10000, CRC(7bee9fe8) SHA1(1489cbd81176a586d21442d3e9cf4e585ca72bb4) )
	ROM_LOAD16_BYTE( "963-j22.k15",      0x40001, 0x10000, CRC(2efed09f) SHA1(be84f71a076b360708f15b555ffb8612eb7f0f08) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmntuc )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963-h23.j17",      0x00000, 0x20000, CRC(718086e1) SHA1(6fd07a36195521be101782a05a9ecbcc5aaebbbd) )
	ROM_LOAD16_BYTE( "963-h24.k17",      0x00001, 0x20000, CRC(2f7d66e1) SHA1(53bd51458609662066b696f3edd19075e883bcde) )
	ROM_LOAD16_BYTE( "963-h21.j15",      0x40000, 0x10000, CRC(1944641e) SHA1(6664dbd9856d3d579a63c6537feef9a6e9bd09c5) )
	ROM_LOAD16_BYTE( "963-h22.k15",      0x40001, 0x10000, CRC(50ce5512) SHA1(641bf4d60a64f23cd3b52af983565dc6b38037c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmntucbl ) // bootleg board with Konami customs. Only the K007232 is substituted by an Oki M5205. There`s also an MC68705R3P lodged between the title song ROMs and the Oki ROMs. Function unknown (possibly driving the Oki?).
	ROM_REGION( 0x60000, "maincpu", 0 ) // same as the original version H
	ROM_LOAD16_BYTE( "ic215", 0x00000, 0x20000, CRC(718086e1) SHA1(6fd07a36195521be101782a05a9ecbcc5aaebbbd) )
	ROM_LOAD16_BYTE( "ic216", 0x00001, 0x20000, CRC(2f7d66e1) SHA1(53bd51458609662066b696f3edd19075e883bcde) )
	ROM_LOAD16_BYTE( "ic217", 0x40000, 0x10000, CRC(1944641e) SHA1(6664dbd9856d3d579a63c6537feef9a6e9bd09c5) )
	ROM_LOAD16_BYTE( "ic218", 0x40001, 0x10000, CRC(50ce5512) SHA1(641bf4d60a64f23cd3b52af983565dc6b38037c1) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ic233", 0x0000, 0x8000, CRC(de119176) SHA1(afd5c25b9c606132263d90c705d6d4c71e28776c) ) // very slightly adapted to run the Oki (or MC68705R3P?) instead of the K007232

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "mc68705r3p", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x100000, "k052109", 0 ) // same as the original, just smaller ROMs on a daughter board
	ROM_LOAD32_BYTE( "9",  0x00000, 0x20000, CRC(bd5df35c) SHA1(b0db3be16f3e18487834b0c877780cc440ca5292) )
	ROM_LOAD32_BYTE( "11", 0x00001, 0x20000, CRC(e26301ce) SHA1(fe34a9ee19d0e15b9d804d75117bcf12853070c4) )
	ROM_LOAD32_BYTE( "21", 0x00002, 0x20000, CRC(db335bb4) SHA1(ae222df7a1b31821f3ee0288ed6f5d9ebd99ba44) )
	ROM_LOAD32_BYTE( "23", 0x00003, 0x20000, CRC(d1d5efae) SHA1(c14429ebbbb356df3164df41763aa9f52e67a01a) )
	ROM_LOAD32_BYTE( "10", 0x80000, 0x20000, CRC(b9d026fe) SHA1(6b6b9c531042a25d152754a807ee822bdc4c35cd) )
	ROM_LOAD32_BYTE( "12", 0x80001, 0x20000, CRC(82eb6dae) SHA1(4d1e97be2d811bbbd8ec2ec5e6464f1e3208d97e) )
	ROM_LOAD32_BYTE( "22", 0x80002, 0x20000, CRC(f8f59c01) SHA1(afd16b355271c350e2b75b7f48c0dad5b7d4fd34) )
	ROM_LOAD32_BYTE( "24", 0x80003, 0x20000, CRC(e7068b0d) SHA1(4efe28768adb80e03fe1edd667822fa739e9437e) )

	ROM_REGION( 0x200000, "k051960", 0 ) // same as the original, just smaller ROMs on a daughter board
	ROM_LOAD32_BYTE( "1",  0x000000, 0x20000, CRC(eabb0238) SHA1(396ca9192e1d7921109897ef7906b93467473f62) )
	ROM_LOAD32_BYTE( "5",  0x000001, 0x20000, CRC(9330c763) SHA1(38b4808736d37bfb139add8746aabc6da83e6cc6) )
	ROM_LOAD32_BYTE( "13", 0x000002, 0x20000, CRC(9ab1eede) SHA1(9d1720e0cce374a390a38d00d54d35d3c80d1d8d) )
	ROM_LOAD32_BYTE( "17", 0x000003, 0x20000, CRC(9592f27f) SHA1(a19edef177f0feae8c29748d94d878ae4f172a9d) )
	ROM_LOAD32_BYTE( "2",  0x080000, 0x20000, CRC(4ba802e3) SHA1(5a5286087559846d06ed38e2fa93f64c1050787f) )
	ROM_LOAD32_BYTE( "6",  0x080001, 0x20000, CRC(641ea298) SHA1(f5ded36fd4779d987d9ef7c171191856813664b5) )
	ROM_LOAD32_BYTE( "14", 0x080002, 0x20000, CRC(777235a5) SHA1(8cd75c55b283a19c54b2007633b7d16878dd2773) )
	ROM_LOAD32_BYTE( "18", 0x080003, 0x20000, CRC(41c0c28c) SHA1(b5464024ba63eac1150542ea888ce3bea2932769) )
	ROM_LOAD32_BYTE( "3",  0x100000, 0x20000, CRC(159b5858) SHA1(e431f9d945ec0c21385ba9390ce350437f022191) )
	ROM_LOAD32_BYTE( "7",  0x100001, 0x20000, CRC(45a1bea8) SHA1(d90824afbda6af5cc86ca74aadd275c07d28f5e1) )
	ROM_LOAD32_BYTE( "15", 0x100002, 0x20000, CRC(748e66f9) SHA1(109cde6be2a25dae898092bca13b680d1665fc85) )
	ROM_LOAD32_BYTE( "19", 0x100003, 0x20000, CRC(9e6dd23b) SHA1(7c7080a9435a35f62b8203167b8d514e8a47b5b4) )
	ROM_LOAD32_BYTE( "4",  0x180000, 0x20000, CRC(7dde75ee) SHA1(1e4a85dd108218a8f7761724aecd6ef2f2af0a6e) )
	ROM_LOAD32_BYTE( "8",  0x180001, 0x20000, CRC(6337e146) SHA1(938aafbb60fd390b5c5c1bc93e9233b90a723e1b) )
	ROM_LOAD32_BYTE( "16", 0x180002, 0x20000, CRC(287f8d80) SHA1(7ea7b83662c0ef2af65f6c6b474ae4176ff6d71e) )
	ROM_LOAD32_BYTE( "20", 0x180003, 0x20000, CRC(bb675176) SHA1(8a106b1778101dbd900c287712cd53826791f0db) )

	ROM_REGION( 0x200, "proms", 0 ) // not dumped for this set, but probably identical
	ROM_LOAD( "963a30.g7",  0x000, 0x100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )
	ROM_LOAD( "963a31.g19", 0x100, 0x100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) )

	ROM_REGION( 0x20000, "msm", 0 )
	ROM_LOAD( "ic252", 0x00000, 0x10000, CRC(cb916429) SHA1(41eff6e15890b45b5ea3b8cc3367891d87c3bf09) )
	ROM_LOAD( "ic253", 0x10000, 0x10000, CRC(c2d3b2c1) SHA1(d4d66cbfbbf5d3396aafedda99bfa2b06fafbf24) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "ic285", 0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) // same as the original

	ROM_REGION( 0x80000, "title", 0 )   // why only the odd bytes? though the song seems to play fine as is..
	ROM_LOAD16_BYTE( "ic275", 0x00001, 0x20000, CRC(a021c0be) SHA1(f44ea6c56b8fae7aeff120ee6c2ca924086654be) ) // ic275               963a25.d5    [odd 1/2]  IDENTICAL
	ROM_LOAD16_BYTE( "ic274", 0x40001, 0x20000, CRC(9ff5f250) SHA1(53b75ca910c645ebb55002d70e3e1abd15db6d41) ) // ic274               963a25.d5    [odd 2/2]  IDENTICAL
ROM_END

ROM_START( tmht )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963-f23.j17",   0x00000, 0x20000, CRC(9cb5e461) SHA1(b693e61070d6ce7ac59ff3f0a824cfefb37b33eb) )
	ROM_LOAD16_BYTE( "963-f24.k17",   0x00001, 0x20000, CRC(2d902fab) SHA1(5a9a3bb0b6c2824eb971a8c0aa8d3069d3c63d06) )
	ROM_LOAD16_BYTE( "963-f21.j15",   0x40000, 0x10000, CRC(9fa25378) SHA1(9ed0bba148e7c5e78224c5168053eeafc2e4b663) )
	ROM_LOAD16_BYTE( "963-f22.k15",   0x40001, 0x10000, CRC(2127ee53) SHA1(e614260883872fd27cd641e6b4787672b2a44139) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmhta )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963-s23.j17",   0x00000, 0x20000, CRC(b5af7eee) SHA1(082c8faabb0d409f73a17d7d342c0afb0f936b91) )
	ROM_LOAD16_BYTE( "963-s24.k17",   0x00001, 0x20000, CRC(bcb8ce8b) SHA1(d9a74627598e29110002ea5d81a4f165d7566329) )
	ROM_LOAD16_BYTE( "963-s21.j15",   0x40000, 0x10000, CRC(0b88bfa6) SHA1(22d552c0aaab336cd7c36d57fde22a64257a0633) )
	ROM_LOAD16_BYTE( "963-s22.k15",   0x40001, 0x10000, CRC(44ce6d4b) SHA1(17e3baa33ab182f21b2686786ba570514830ed83) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmhtb ) // the code is closest to tmntua near the start, and the data is closest to all the UK sets, especially tmhta, so I'm guessing it's a UK revision of the tmntua codebase
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "unk3.i17",   0x00000, 0x20000, CRC(537eb047) SHA1(97e6dbc486c7d057355db7fcbdc0a2c2cad2c653) ) /* unknown 963 xxx Konami code for this set */
	ROM_LOAD16_BYTE( "unk4.k17",   0x00001, 0x20000, CRC(5afae564) SHA1(8d5fbf9530ad8d095c12b7e0f8c499c1436c4d47) )
	ROM_LOAD16_BYTE( "unk2.j15",   0x40000, 0x10000, CRC(ee34de05) SHA1(507d7fb178dbbe87dd373a81ad3f350ee2f7d923) )
	ROM_LOAD16_BYTE( "unk5.k15",   0x40001, 0x10000, CRC(5ef58d4e) SHA1(5df71c61a90c3e9d28ec3b8055d7ee97bc283e01) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmntj )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963_223.j17",  0x00000, 0x20000, CRC(0d34a5ff) SHA1(a387f3e7c727dc66ebb0e1f40e4ab8dc83f647e5) )
	ROM_LOAD16_BYTE( "963_224.k17",  0x00001, 0x20000, CRC(2fd453f2) SHA1(8eb68cba3b5f5baf2c00172942a3d2bf578d0196) )
	ROM_LOAD16_BYTE( "963_221.j15",  0x40000, 0x10000, CRC(fa8e25fd) SHA1(129cb9498508cdabdda3cf4fc86ff716fe1da940) )
	ROM_LOAD16_BYTE( "963_222.k15",  0x40001, 0x10000, CRC(ca437a4f) SHA1(96922d2dcd0d84dc0d09a3ba9800b1154b5e2486) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmnta )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "tmnt j17.bin",      0x00000, 0x20000, CRC(00819687) SHA1(65624465b8af21000ca42b759c6fe123b4570e08) )
	ROM_LOAD16_BYTE( "tmnt k17.bin",      0x00001, 0x20000, CRC(6930e085) SHA1(3c35c663346a81d06cd0169fbae08c19d1bde2eb) )
	ROM_LOAD16_BYTE( "tmnt j15.bin",      0x40000, 0x10000, CRC(fd1e2e01) SHA1(63c3e8adcb5025a0a11f28e623cf2692f5f030a3) )
	ROM_LOAD16_BYTE( "tmnt k15.bin",      0x40001, 0x10000, CRC(b01eea79) SHA1(3f0201ed471380fcafaf2e570454c3d742c0e03d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmht2p )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963-u23.j17",      0x00000, 0x20000, CRC(58bec748) SHA1(6cf146d6de8ef01c0705394d135abebc3aeaae16) )
	ROM_LOAD16_BYTE( "963-u24.k17",      0x00001, 0x20000, CRC(dce87c8d) SHA1(b85018ffc226ec7dfc97f9cd0f4454951c6e5918) )
	ROM_LOAD16_BYTE( "963-u21.j15",      0x40000, 0x10000, CRC(abce5ead) SHA1(2b3674497bb4f688c5f0e1cc9a078b3feb01475d) )
	ROM_LOAD16_BYTE( "963-u22.k15",      0x40001, 0x10000, CRC(4ecc8d6b) SHA1(ce29aecbd98c0a07f48766564de173facb310371) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmht2pa )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963-_23.j17",      0x00000, 0x20000, CRC(8698061a) SHA1(f01aa535e8fb63fb57cd06c0ad6bb7720fe14a84) )
	ROM_LOAD16_BYTE( "963-_24.k17",      0x00001, 0x20000, CRC(4036c075) SHA1(38701c34f8baa70934d5c4434230f3f09e28386a) )
	ROM_LOAD16_BYTE( "963-_21.j15",      0x40000, 0x10000, CRC(ddcc979c) SHA1(5dfabe2af341f19349872ea12b183750804eab56) )
	ROM_LOAD16_BYTE( "963-_22.k15",      0x40001, 0x10000, CRC(71a38d27) SHA1(11c92f2b772ddac3d432c9a1d57ab0b5dd2c9137) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmnt2pj )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "963-123.j17",      0x00000, 0x20000, CRC(6a3527c9) SHA1(a5a8cbec3fae3f37d4d82a7700cec3c96c6a362f) )
	ROM_LOAD16_BYTE( "963-124.k17",      0x00001, 0x20000, CRC(2c4bfa15) SHA1(0264ef6f15806d52d6f7869034f5a3024ba1cea2) )
	ROM_LOAD16_BYTE( "963-121.j15",      0x40000, 0x10000, CRC(4181b733) SHA1(306601597102a1bc79880e557889a6fce7b30b7b) )
	ROM_LOAD16_BYTE( "963-122.k15",      0x40001, 0x10000, CRC(c64eb5ff) SHA1(e546f1cb81e98a38833cd0affe73e2bc1d95d017) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END

ROM_START( tmnt2po )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 2*128k and 2*64k for 68000 code */
	ROM_LOAD16_BYTE( "tmnt123.j17",  0x00000, 0x20000, CRC(2d905183) SHA1(38c77a08733f9da1dc6f1c510a2c8dac34848787) )
	ROM_LOAD16_BYTE( "tmnt124.k17",  0x00001, 0x20000, CRC(e0125352) SHA1(e2a297bf96d0fa1d19ce767786453c489d49d693) )
	ROM_LOAD16_BYTE( "tmnt21.j15",   0x40000, 0x10000, CRC(12deeafb) SHA1(1f70a326f8f4a896da297b4f66ca467894d22159) )
	ROM_LOAD16_BYTE( "tmnt22.k15",   0x40001, 0x10000, CRC(aec4f1c3) SHA1(189ed93bc9ee4a1ff1c0ca7b80f4e817e5484e69) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "963e20.g13",      0x00000, 0x08000, CRC(1692a6d6) SHA1(68c3419012b2863e91a7d7e479fce5ceabb10b88) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "963a28.h27",      0x000000, 0x80000, CRC(db4769a8) SHA1(810811914f9c1fbf2320d5a9030cbf124f6d78cf) )
	ROM_LOAD32_WORD( "963a29.k27",      0x000002, 0x80000, CRC(8069cd2e) SHA1(54095d3546119ccd1e8814d692aceb1327c9369f) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "963a17.h4",      0x000000, 0x80000, CRC(b5239a44) SHA1(84e94807e7c51aa652b4e4b827b36be59a53d0d6) )
	ROM_LOAD32_WORD( "963a15.k4",      0x000002, 0x80000, CRC(1f324eed) SHA1(971a675578518fffa341a943d0cc4fdea005fde0) )
	ROM_LOAD32_WORD( "963a18.h6",      0x100000, 0x80000, CRC(dd51adef) SHA1(5010c0911b0b9e4f23a785e8a751a0bde5be5be0) )
	ROM_LOAD32_WORD( "963a16.k6",      0x100002, 0x80000, CRC(d4bd9984) SHA1(d780ae7f72e16767c3a492544f02f0f1a332ab22) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "963a30.g7",      0x0000, 0x0100, CRC(abd82680) SHA1(945a71e6ec65202f13209b45d45b616372d6c0f5) )  /* sprite address decoder */
	ROM_LOAD( "963a31.g19",      0x0100, 0x0100, CRC(f8004a1c) SHA1(ed6694b8eebfe0238b50ebd05007d519f6e57b1b) ) /* priority encoder (not used) */

	ROM_REGION( 0x20000, "k007232", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a26.c13",      0x00000, 0x20000, CRC(e2ac3063) SHA1(5bb294c46fb5eaba9935a18c0aa5d3931168f474) ) /* samples for 007232 */

	ROM_REGION( 0x20000, "upd", 0 ) /* 128k for the samples */
	ROM_LOAD( "963a27.d18",      0x00000, 0x20000, CRC(2dfd674b) SHA1(bbec5896c70056964fbc972a84bd5b0dfc6af257) ) /* samples for UPD7759C */

	ROM_REGION( 0x80000, "title", 0 )   /* 512k for the title music sample */
	ROM_LOAD( "963a25.d5",      0x00000, 0x80000, CRC(fca078c7) SHA1(3e1124d72c9db4cb11d8de6c44b7aeca967f44e1) )
ROM_END


// MIA and TMNT have their graphics data (both tiles and sprites) stored in the ROMs in
// the packed pixel format used by older Konami hardware such as Twin16. The data lines
// from the ROMs to the custom chips are swapped so that the chips receive the data in
// the planar format they expect.

static void chunky_to_planar(memory_region *rgn)
{
	uint32_t *ROM = reinterpret_cast<uint32_t *>(rgn->base());
	int len = rgn->bytes() / 4;

	for (int i = 0; i < len; i++)
	{
		uint32_t data = little_endianize_int32(ROM[i]);
		data = bitswap<32>(data,31,27,23,19,15,11,7,3,30,26,22,18,14,10,6,2,29,25,21,17,13,9,5,1,28,24,20,16,12,8,4,0);
		ROM[i] = little_endianize_int32(data);
	}
}


void tmnt_state::init_mia()
{
	chunky_to_planar(memregion("k052109"));
	chunky_to_planar(memregion("k051960"));

	// unscramble the sprite ROM address lines
	uint32_t *gfxdata = reinterpret_cast<uint32_t *>(memregion("k051960")->base());
	int len = memregion("k051960")->bytes() / 4;
	std::vector<uint32_t> temp(len);
	memcpy(&temp[0], gfxdata, len * 4);
	for (int A = 0; A < len; A++)
	{
		// the bits to scramble are the low 8 ones
		int B = A & 0x3ff00;

		if ((A & 0x3c000) == 0x3c000)
			B |= bitswap<8>(A,7,6,4,2,1,0,5,3);
		else
			B |= bitswap<8>(A,6,4,2,1,0,7,5,3);

		gfxdata[A] = temp[B];
	}
}


void tmnt_state::init_tmnt()
{
	chunky_to_planar(memregion("k052109"));
	chunky_to_planar(memregion("k051960"));

	// unscramble the sprite ROM address lines
	const uint8_t *code_conv_table = memregion("proms")->base();
	uint32_t *gfxdata = reinterpret_cast<uint32_t *>(memregion("k051960")->base());
	int len = memregion("k051960")->bytes() / 4;
	std::vector<uint32_t> temp(len);
	memcpy(&temp[0], gfxdata, len * 4);

	for (int A = 0; A < len; A++)
	{
		// following table derived from the schematics. It indicates, for each of the
		// 9 low bits of the sprite line address, which bit to pick it from.
		// For example, when the PROM contains 4, which applies to 4x2 sprites,
		// bit OA1 comes from CA5, OA2 from CA0, and so on.
		static const uint8_t bit_pick_table[10][8] =
		{
			//0(1x1) 1(2x1) 2(1x2) 3(2x2) 4(4x2) 5(2x4) 6(4x4) 7(8x8)
			{ 3,     3,     3,     3,     3,     3,     3,     3 }, // CA3
			{ 0,     0,     5,     5,     5,     5,     5,     5 }, // OA1
			{ 1,     1,     0,     0,     0,     7,     7,     7 }, // OA2
			{ 2,     2,     1,     1,     1,     0,     0,     9 }, // OA3
			{ 4,     4,     2,     2,     2,     1,     1,     0 }, // OA4
			{ 5,     6,     4,     4,     4,     2,     2,     1 }, // OA5
			{ 6,     5,     6,     6,     6,     4,     4,     2 }, // OA6
			{ 7,     7,     7,     7,     8,     6,     6,     4 }, // OA7
			{ 8,     8,     8,     8,     7,     8,     8,     6 }, // OA8
			{ 9,     9,     9,     9,     9,     9,     9,     8 }  // OA9
		};

		// pick the correct entry in the PROM (top 8 bits of the address)
		int entry = code_conv_table[(A & 0x7f800) >> 11] & 7;

		int bits[32];

		// the bits to scramble are the low 10 ones
		for (int i = 0; i < 10; i++)
			bits[i] = (A >> i) & 0x01;

		int B = A & 0x7fc00;

		for (int i = 0; i < 10; i++)
			B |= bits[bit_pick_table[i][entry]] << i;

		gfxdata[A] = temp[B];
	}
}

void tmnt_state::init_cuebrick()
{
	m_nvrambank->configure_entries(0, 0x20, m_cuebrick_nvram, 0x400);

	subdevice<nvram_device>("nvram")->set_base(m_cuebrick_nvram, sizeof(m_cuebrick_nvram));

	save_item(NAME(m_cuebrick_nvram));
}

} // anonymous namespace

//    YEAR  NAME         PARENT    MACHINE   INPUT      STATE       INIT      MONITOR COMPANY    FULLNAME,FLAGS
GAME( 1989, cuebrick,    0,        cuebrick, cuebrick,  tmnt_state, init_cuebrick,ROT0,  "Konami",  "Cue Brick (World, version D)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, mia,         0,        mia,      mia,       tmnt_state, init_mia,    ROT0,   "Konami",  "M.I.A. - Missing in Action (version T)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, mia2,        mia,      mia,      mia,       tmnt_state, init_mia,    ROT0,   "Konami",  "M.I.A. - Missing in Action (version S)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, tmnt,        0,        tmnt,     tmnt,      tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Ninja Turtles (World 4 Players, version X)",       MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmntu,       tmnt,     tmnt,     tmnt,      tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Ninja Turtles (US 4 Players, version R)",          MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmntua,      tmnt,     tmnt,     tmnt,      tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Ninja Turtles (US 4 Players, version N)",          MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmntub,      tmnt,     tmnt,     tmnt,      tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Ninja Turtles (US 4 Players, version J)",          MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmntuc,      tmnt,     tmnt,     tmnt,      tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Ninja Turtles (US 4 Players, version H)",          MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmntucbl,    tmnt,     tmntucbl, tmnt,      tmnt_state, init_tmnt,   ROT0,   "bootleg", "Teenage Mutant Ninja Turtles (bootleg, US 4 Players, version H)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // Needs MC68705R3P / Oki M5205 hook up
GAME( 1989, tmht,        tmnt,     tmnt,     tmnt,      tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Hero Turtles (UK 4 Players, version F)",           MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmhta,       tmnt,     tmnt,     tmnt,      tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Hero Turtles (UK 4 Players, version S)",           MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmhtb,       tmnt,     tmnt,     tmnt,      tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Hero Turtles (UK 4 Players, version ?)",           MACHINE_SUPPORTS_SAVE )
GAME( 1990, tmntj,       tmnt,     tmnt,     tmnt,      tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Ninja Turtles (Japan 4 Players, version 2)",       MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmnta,       tmnt,     tmnt,     tmnt,      tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Ninja Turtles (Asia 4 Players, version ?)",        MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmht2p,      tmnt,     tmnt,     tmnt2p,    tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Hero Turtles (UK 2 Players, version U)",           MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmht2pa,     tmnt,     tmnt,     tmnt2p,    tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Hero Turtles (UK 2 Players, version ?)",           MACHINE_SUPPORTS_SAVE )
GAME( 1990, tmnt2pj,     tmnt,     tmnt,     tmnt2p,    tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Ninja Turtles (Japan 2 Players, version 1)",       MACHINE_SUPPORTS_SAVE )
GAME( 1989, tmnt2po,     tmnt,     tmnt,     tmnt2p,    tmnt_state, init_tmnt,   ROT0,   "Konami",  "Teenage Mutant Ninja Turtles (Oceania 2 Players, version ?)",     MACHINE_SUPPORTS_SAVE )
