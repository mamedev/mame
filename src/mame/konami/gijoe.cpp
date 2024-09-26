// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    G.I. Joe  (c) 1992 Konami


G.I. Joe
Konami 1992

PCB Layout
----------
GX069 PWB352065B
|--------------------------------------------------------|
|LA4705           069A04.1E      069A05.1H               |
|          84256                 069A06.2H               |
| 054986A          |------|      069A07.4H               |
|          Z80E    |054539|      069A08.6H               |
|CN6               |      |                              |
|          8416    |      |            |------| |------| |
| 051550           |------|            |053246| |053247A |
|          069A01.7C                   |      | |      | |
|                     32MHz            |      | |      | |
|J          ER5911.7D 18.432MHz        |      | |      | |
|A 052535       |------------|         |------| |------| |
|M              |   68000    |                           |
|M 052535       |            |         5168              |
|A              |------------|         5168              |
|  052535        84256                                   |
|                                                        |
|                069A12.13E            |------| |------| |
|          8416                        |054157| |054156| |
|                069UAB03.14E |------| |      | |      | |
|          8416               |053251| |      | |      | |
|  TEST_SW       84256        |      | |      | |      | |
|                             |------| |------| |------| |
|005273(X10)     069A11.16E                              |
|                                                   5168 |
|  CN9           069UAB02.18E           069A09.16J  5168 |
|  CN8   DSW(4)               24MHz     069A10.18J  5168 |
|--------------------------------------------------------|
Notes:
      68000   - Clock 16.000MHz [32/2]
      Z80E    - Clock 8.000MHz [32/4]
      8416    - Fujitsu MB8416 2kx8 SRAM (DIP24)
      84256   - Fujitsu MB84256 32kx8 SRAM (DIP28)
      5168    - Sharp LH5168 8kx8 SRAM (DIP28)
      ER5911  - EEPROM (128 bytes)
      CN6     - 4 pin connector for stereo sound output
      CN8/CN9 - 15 pin connectors for player 3 & player 4 controls
      069*    - EPROM/mask ROM
      LA4705  - 15W 2-channel BTL audio power AMP

      Custom Chips
      ------------
      053251  - Priority encoder
      054157  \
      054156  / Tilemap generators
      053246  \
      053247A / Sprite generators
      054539  - 8-Channel ADPCM sound generator. Clock input 18.432MHz. Clock outputs 18.432/4 & 18.432/8
      052535  - Video DAC (one for each R,G,B video signal)
      051550  - EMI filter for credit/coin counter
      005273  - Resistor array for player 3 & player 4 controls
      054986A - Audio DAC/filter + sound latch + Z80 memory mapper/banker (large ceramic SDIP64 module)
                This module contains several surface mounted capacitors and resistors, 4558 OP amp,
                Analog Devices AD1868 dual 18-bit audio DAC and a Konami 054321 QFP44 IC.

      Sync Measurements
      -----------------
      HSync - 15.2036kHz
      VSync - 59.6374Hz


****************************************************************************

Known Issues
------------

- sprite gaps (K053247 zoom fraction rounding)
- shadow masking (eg. the shadow of Baroness' aircraft should not project on the sky)

***************************************************************************/

#include "emu.h"

#include "k053251.h"
#include "k054156_k054157_k056832.h"
#include "k053246_k053247_k055673.h"
#include "konamipt.h"
#include "konami_helper.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/k054321.h"
#include "sound/k054539.h"
#include "emupal.h"
#include "speaker.h"


namespace {

#define JOE_DEBUG 0
#define JOE_DMADELAY (attotime::from_nsec(42700 + 341300))


class gijoe_state : public driver_device
{
public:
	gijoe_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_workram(*this, "workram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k054539(*this, "k054539"),
		m_k056832(*this, "k056832"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_palette(*this, "palette"),
		m_k054321(*this, "k054321")
	{ }

	void gijoe(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_workram;

	/* video-related */
	int         m_avac_bits[4]{};
	int         m_avac_occupancy[4]{};
	int         m_layer_colorbase[4]{};
	int         m_layer_pri[4]{};
	int         m_avac_vrc = 0;
	int         m_sprite_colorbase = 0;

	/* misc */
	uint16_t      m_cur_control2 = 0U;
	emu_timer   *m_dmadelay_timer = nullptr;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054539_device> m_k054539;
	required_device<k056832_device> m_k056832;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_device<palette_device> m_palette;
	required_device<k054321_device> m_k054321;

	uint16_t control2_r();
	void control2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_irq_w(uint16_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_gijoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gijoe_interrupt);
	TIMER_CALLBACK_MEMBER(dmaend_callback);
	void gijoe_objdma();
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
	void gijoe_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


K053246_CB_MEMBER(gijoe_state::sprite_callback)
{
	int pri = (*color & 0x03e0) >> 4;

	if (pri <= m_layer_pri[3])
		*priority_mask = 0;
	else if (pri >  m_layer_pri[3] && pri <= m_layer_pri[2])
		*priority_mask = 0xff00;
	else if (pri >  m_layer_pri[2] && pri <= m_layer_pri[1])
		*priority_mask = 0xff00 | 0xf0f0;
	else if (pri >  m_layer_pri[1] && pri <= m_layer_pri[0])
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc;
	else
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc | 0xaaaa;

	*color = m_sprite_colorbase | (*color & 0x001f);
}

K056832_CB_MEMBER(gijoe_state::tile_callback)
{
	int tile = *code;

	if (tile >= 0xf000 && tile <= 0xf4ff)
	{
		tile &= 0x0fff;
		if (tile < 0x0310)
		{
			m_avac_occupancy[layer] |= 0x0f00;
			tile |= m_avac_bits[0];
		}
		else if (tile < 0x0470)
		{
			m_avac_occupancy[layer] |= 0xf000;
			tile |= m_avac_bits[1];
		}
		else
		{
			m_avac_occupancy[layer] |= 0x00f0;
			tile |= m_avac_bits[2];
		}
		*code = tile;
	}

	*color = (*color >> 2 & 0x0f) | m_layer_colorbase[layer];
}

void gijoe_state::video_start()
{
	int i;

	m_k056832->linemap_enable(1);

	for (i = 0; i < 4; i++)
	{
		m_avac_occupancy[i] = 0;
		m_avac_bits[i] = 0;
		m_layer_colorbase[i] = 0;
		m_layer_pri[i] = 0;
	}

	m_avac_vrc = 0xffff;

	save_item(NAME(m_avac_vrc));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_avac_occupancy));
	save_item(NAME(m_avac_bits));   // these could possibly be re-created at postload k056832 elements
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layer_pri));
}

uint32_t gijoe_state::screen_update_gijoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const int K053251_CI[4] = { k053251_device::CI1, k053251_device::CI2, k053251_device::CI3, k053251_device::CI4 };
	int layer[4];
	int vrc_mode, vrc_new, colorbase_new, /*primode,*/ dirty, i;
	int mask = 0;

	// update tile offsets
	m_k056832->read_avac(&vrc_mode, &vrc_new);

	if (vrc_mode)
	{
		for (dirty = 0xf000; dirty; dirty >>= 4)
			if ((m_avac_vrc & dirty) != (vrc_new & dirty))
				mask |= dirty;

		m_avac_vrc = vrc_new;
		m_avac_bits[0] = vrc_new << 4  & 0xf000;
		m_avac_bits[1] = vrc_new       & 0xf000;
		m_avac_bits[2] = vrc_new << 8  & 0xf000;
		m_avac_bits[3] = vrc_new << 12 & 0xf000;
	}
	else
		m_avac_bits[3] = m_avac_bits[2] = m_avac_bits[1] = m_avac_bits[0] = 0xf000;

	// update color info and refresh tilemaps
	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI0);

	for (i = 0; i < 4; i++)
	{
		dirty = 0;
		colorbase_new = m_k053251->get_palette_index(K053251_CI[i]);
		if (m_layer_colorbase[i] != colorbase_new)
		{
			m_layer_colorbase[i] = colorbase_new;
			dirty = 1;
		}
		if (m_avac_occupancy[i] & mask)
			dirty = 1;

		if (dirty)
		{
			m_avac_occupancy[i] = 0;
			m_k056832->mark_plane_dirty( i);
		}
	}

	/*
	    Layer A is supposed to be a non-scrolling status display with static X-offset.
	    The weird thing is tilemap alignment only follows the 832 standard when 2 is
	    written to the layer's X-scroll register otherwise the chip expects totally
	    different alignment values.
	*/
	if (m_k056832->read_register(0x14) == 2)
	{
		m_k056832->set_layer_offs(0,  2, 0);
		m_k056832->set_layer_offs(1,  4, 0);
		m_k056832->set_layer_offs(2,  6, 0); // 7?
		m_k056832->set_layer_offs(3,  8, 0);
	}
	else
	{
		m_k056832->set_layer_offs(0,  0, 0);
		m_k056832->set_layer_offs(1,  8, 0);
		m_k056832->set_layer_offs(2, 14, 0);
		m_k056832->set_layer_offs(3, 16, 0); // smaller?
	}

	// seems to switch the K053251 between different priority modes, detail unknown
	// primode = m_k053251->get_priority(k053251_device::CI1);

	layer[0] = 0;
	m_layer_pri[0] = 0; // not sure
	layer[1] = 1;
	m_layer_pri[1] = m_k053251->get_priority(k053251_device::CI2);
	layer[2] = 2;
	m_layer_pri[2] = m_k053251->get_priority(k053251_device::CI3);
	layer[3] = 3;
	m_layer_pri[3] = m_k053251->get_priority(k053251_device::CI4);

	konami_sortlayers4(layer, m_layer_pri);

	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);

	m_k053246->k053247_sprites_draw( bitmap, cliprect);
	return 0;
}


uint16_t gijoe_state::control2_r()
{
	return m_cur_control2;
}

void gijoe_state::control2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0  is data */
		/* bit 1  is cs (active low) */
		/* bit 2  is clock (active high) */
		/* bit 3  (unknown: coin) */
		/* bit 5  is enable irq 6 */
		/* bit 7  (unknown: enable irq 5?) */
		ioport("EEPROMOUT")->write(data, 0xff);

		m_cur_control2 = data;

		/* bit 6 = enable sprite ROM reading */
		m_k053246->k053246_set_objcha_line( (data & 0x0040) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void gijoe_state::gijoe_objdma(  )
{
	uint16_t *src_head, *src_tail, *dst_head, *dst_tail;

	src_head = m_spriteram;
	src_tail = m_spriteram + 255 * 8;
	m_k053246->k053247_get_ram( &dst_head);
	dst_tail = dst_head + 255 * 8;

	for (; src_head <= src_tail; src_head += 8)
	{
		if (*src_head & 0x8000)
		{
			memcpy(dst_head, src_head, 0x10);
			dst_head += 8;
		}
		else
		{
			*dst_tail = 0;
			dst_tail -= 8;
		}
	}
}

TIMER_CALLBACK_MEMBER(gijoe_state::dmaend_callback)
{
	if (m_cur_control2 & 0x0020)
		m_maincpu->set_input_line(6, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(gijoe_state::gijoe_interrupt)
{
	// global interrupt masking (*this game only)
	if (!m_k056832->is_irq_enabled(0))
		return;

	if (m_k053246->k053246_is_irq_enabled())
	{
		gijoe_objdma();

		// 42.7us(clr) + 341.3us(xfer) delay at 6Mhz dotclock
		m_dmadelay_timer->adjust(JOE_DMADELAY);
	}

	// trigger V-blank interrupt
	if (m_cur_control2 & 0x0080)
		device.execute().set_input_line(5, HOLD_LINE);
}

void gijoe_state::sound_irq_w(uint16_t data)
{
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

void gijoe_state::gijoe_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x100fff).ram().share("spriteram");                               // Sprites
	map(0x110000, 0x110007).w(m_k053246, FUNC(k053247_device::k053246_w));
	map(0x120000, 0x121fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));      // Graphic planes
	map(0x122000, 0x123fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));      // Graphic planes mirror read
	map(0x130000, 0x131fff).r(m_k056832, FUNC(k056832_device::rom_word_r));                               // Passthrough to tile roms
	map(0x160000, 0x160007).w(m_k056832, FUNC(k056832_device::b_word_w));                                    // VSCCS (board dependent)
	map(0x170000, 0x170001).nopw();                                                // Watchdog
	map(0x180000, 0x18ffff).ram().share("workram");                 // Main RAM.  Spec. 180000-1803ff, 180400-187fff
	map(0x190000, 0x190fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x1a0000, 0x1a001f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
	map(0x1b0000, 0x1b003f).w(m_k056832, FUNC(k056832_device::word_w));
	map(0x1c0000, 0x1c001f).m(m_k054321, FUNC(k054321_device::main_map)).umask16(0x00ff);
	map(0x1d0000, 0x1d0001).w(FUNC(gijoe_state::sound_irq_w));
	map(0x1e0000, 0x1e0001).portr("P1_P2");
	map(0x1e0002, 0x1e0003).portr("P3_P4");
	map(0x1e4000, 0x1e4001).portr("SYSTEM");
	map(0x1e4002, 0x1e4003).portr("START");
	map(0x1e8000, 0x1e8001).rw(FUNC(gijoe_state::control2_r), FUNC(gijoe_state::control2_w));
	map(0x1f0000, 0x1f0001).r(m_k053246, FUNC(k053247_device::k053246_r));
#if JOE_DEBUG
	map(0x110000, 0x110007).r(m_k053246, FUNC(k053247_device::k053246_read_register));
	map(0x160000, 0x160007).r(m_k056832, FUNC(k056832_device::b_word_r));
	map(0x1a0000, 0x1a001f).r(m_k053251, FUNC(k053251_device::read)).umask16(0x00ff);
	map(0x1b0000, 0x1b003f).r(m_k056832, FUNC(k056832_device::word_r));
#endif
}

void gijoe_state::sound_map(address_map &map)
{
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xfa2f).rw(m_k054539, FUNC(k054539_device::read), FUNC(k054539_device::write));
	map(0xfc00, 0xfc03).m(m_k054321, FUNC(k054321_device::sound_map));
	map(0x0000, 0xefff).rom();
}

static INPUT_PORTS_START( gijoe )
	PORT_START("START")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_START3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_START4 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_COIN4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_SERVICE2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SERVICE3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_SERVICE4 )

	PORT_START("P1_P2")
	KONAMI16_LSB_40(1, IPT_BUTTON3 ) PORT_OPTIONAL
	PORT_DIPNAME( 0x0080, 0x0000, "Sound" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Mono ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Stereo ) )
	KONAMI16_MSB_40(2, IPT_BUTTON3 ) PORT_OPTIONAL
	PORT_DIPNAME( 0x8000, 0x8000, "Coin mechanism" )    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x8000, "Common" )
	PORT_DIPSETTING(      0x0000, "Independent" )

	PORT_START("P3_P4")
	KONAMI16_LSB_40(3, IPT_BUTTON3 ) PORT_OPTIONAL
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Players ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	KONAMI16_MSB_40(4, IPT_BUTTON3 ) PORT_OPTIONAL
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:4" )    /* Listed as "Unused" */
INPUT_PORTS_END

void gijoe_state::machine_start()
{
	m_dmadelay_timer = timer_alloc(FUNC(gijoe_state::dmaend_callback), this);

	save_item(NAME(m_cur_control2));
}

void gijoe_state::machine_reset()
{
	m_cur_control2 = 0;
}

void gijoe_state::gijoe(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);   /* 16MHz Confirmed */
	m_maincpu->set_addrmap(AS_PROGRAM, &gijoe_state::gijoe_map);
	m_maincpu->set_vblank_int("screen", FUNC(gijoe_state::gijoe_interrupt));

	Z80(config, m_audiocpu, XTAL(32'000'000)/4);     /* Amuse & confirmed. Z80E at 8MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &gijoe_state::sound_map);

	EEPROM_ER5911_8BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(24, 24+288-1, 16, 16+224-1);
	screen.set_screen_update(FUNC(gijoe_state::screen_update_gijoe));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 2048).enable_shadows();

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(gijoe_state::tile_callback));
	m_k056832->set_config(K056832_BPP_4, 1, 0);
	m_k056832->set_palette(m_palette);

	K053246(config, m_k053246, 0);
	m_k053246->set_sprite_callback(FUNC(gijoe_state::sprite_callback));
	m_k053246->set_config(NORMAL_PLANE_ORDER, -37, 20);
	m_k053246->set_palette(m_palette);

	K053251(config, m_k053251, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	K054321(config, m_k054321, "lspeaker", "rspeaker");

	k054539_device &k054539(K054539(config, "k054539", XTAL(18'432'000)));
	k054539.timer_handler().set_inputline("audiocpu", INPUT_LINE_NMI);
	k054539.add_route(0, "rspeaker", 1.0);
	k054539.add_route(1, "lspeaker", 1.0);
}


ROM_START( gijoe )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "069eab03.14e", 0x000000,  0x40000, CRC(dd2d533f) SHA1(6fc9f7a8fc89155ef2b9ee43fe5e456d9b574f8c) )
	ROM_LOAD16_BYTE( "069eab02.18e", 0x000001,  0x40000, CRC(6bb11c87) SHA1(86581d24f73f2e837f1d4fc5f1f2188f610c50b6) )
	ROM_LOAD16_BYTE( "069a12.13e",   0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11.16e",   0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01.7c", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "k056832", 0 )
	ROM_LOAD32_WORD( "069a10.18j", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD32_WORD( "069a09.16j", 0x000002, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "k053246", 0 )
	ROM_LOAD64_WORD( "069a08.6h", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD64_WORD( "069a05.1h", 0x000002, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD64_WORD( "069a07.4h", 0x000004, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD64_WORD( "069a06.2h", 0x000006, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "k054539", 0 )
	ROM_LOAD( "069a04.1e", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.7d", 0x0000, 0x080, CRC(a0d50a79) SHA1(972533ea45a0e84d9dd14c55f58cd7247926792e) )
ROM_END

// this set is strange, instead of showing program OK it shows the location and checksums of the ROMs
// this doesn't indicate failure, as if you hack the parent set it will show the checksum and the word 'BAD' and refuse to boot
// It will boot as whatever version string is in the EEPROM.  If no version string is in the EEPROM it just shows a blank string
// If you factory default it you get the string 'EB8'
// the roms had no proper labels
// maybe it's some interim / test revision
ROM_START( gijoeea )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom3.14e",   0x000000,  0x40000, CRC(0a11f63a) SHA1(06174682907e718017146665b8636be20843b119) )
	ROM_LOAD16_BYTE( "rom2.18e",   0x000001,  0x40000, CRC(8313c559) SHA1(00ae945c65439d4092eaa1780a182dbe3753bb02) )
	ROM_LOAD16_BYTE( "069a12.13e", 0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11.16e", 0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01.7c", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "k056832", 0 )
	ROM_LOAD32_WORD( "069a10.18j", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD32_WORD( "069a09.16j", 0x000002, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "k053246", 0 )
	ROM_LOAD64_WORD( "069a08.6h", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD64_WORD( "069a05.1h", 0x000002, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD64_WORD( "069a07.4h", 0x000004, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD64_WORD( "069a06.2h", 0x000006, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "k054539", 0 )
	ROM_LOAD( "069a04.1e", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom
	ROM_LOAD( "er5911.7d", 0x0000, 0x080, CRC(64f5c87b) SHA1(af81abc54eb59ef7d2250b5ab6cc9642fbd9bfb2) ) // sldh
ROM_END

ROM_START( gijoeu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "069uab03.14e", 0x000000,  0x40000, CRC(25ff77d2) SHA1(bea2ae975718806698fd35ef1217bd842b2b69ec) )
	ROM_LOAD16_BYTE( "069uab02.18e", 0x000001,  0x40000, CRC(31cced1c) SHA1(3df1def671966b3c3d8117ac1b68adeeef9d98c0) )
	ROM_LOAD16_BYTE( "069a12.13e",   0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11.16e",   0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01.7c", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "k056832", 0 )
	ROM_LOAD32_WORD( "069a10.18j", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD32_WORD( "069a09.16j", 0x000002, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "k053246", 0 )
	ROM_LOAD64_WORD( "069a08.6h", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD64_WORD( "069a05.1h", 0x000002, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD64_WORD( "069a07.4h", 0x000004, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD64_WORD( "069a06.2h", 0x000006, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "k054539", 0 )
	ROM_LOAD( "069a04.1e", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.7d", 0x0000, 0x080, CRC(ca966023) SHA1(6f07ece0f95213bc12387192986f468d23dfdfc8) ) // sldh
ROM_END

ROM_START( gijoeua ) // this uses a standard GX069 main PCB, but the GFX ROMs are moved to the 'ROM BOARD MAX352M PWB 352673'
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "069uaa03.14e", 0x000000,  0x40000, CRC(cfb1af44) SHA1(b4e68dbead19a5211778345330739cfcb37011ab) )
	ROM_LOAD16_BYTE( "069uaa02.18e", 0x000001,  0x40000, CRC(3e6a56cd) SHA1(df719197dc001e80be8d04ced68f8937a9d6d1fe) )
	ROM_LOAD16_BYTE( "069a12.13e",   0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11.16e",   0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01.7c", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	// the GFX ROMs are smaller and believed to have the same content as the other sets, in fact the game runs fine. Still better to have them dumped
	// only 4 GFX ROMs were dumped from this PCB because the others are soldered ant the dumper didn't want to risk damage
	ROM_REGION( 0x200000, "k056832", 0 )
	ROM_LOAD32_BYTE( "069a10al.d7", 0x000000, 0x40000, NO_DUMP )
	ROM_LOAD32_BYTE( "069a10ah.d9", 0x000001, 0x40000, NO_DUMP )
	ROM_LOAD32_BYTE( "069a09al.d3", 0x000002, 0x40000, NO_DUMP )
	ROM_LOAD32_BYTE( "069a09ah.d5", 0x000003, 0x40000, NO_DUMP )
	ROM_LOAD32_BYTE( "069a10bl.j7", 0x100000, 0x40000, CRC(087d8e25) SHA1(b26eca0f96c91c18143b5e7a77aaf3831e935d5e) )
	ROM_LOAD32_BYTE( "069a10bh.j9", 0x100001, 0x40000, CRC(fc7ad198) SHA1(ef9b834af04b78aa1c205e1c63da0fdec07783bb) )
	ROM_LOAD32_BYTE( "069a09bl.j3", 0x100002, 0x40000, CRC(385217cc) SHA1(3b1fa53fde8e500e5e06a16a2ae71457c23a73a4) )
	ROM_LOAD32_BYTE( "069a09bh.j5", 0x100003, 0x40000, CRC(c6d43c8a) SHA1(5bbb6c8d160e32097f2327ae3708037a9b9543f3) )
	// overlay standard ROMs for now
	ROM_LOAD32_WORD( "069a10.18j", 0x000000, 0x100000, BAD_DUMP CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD32_WORD( "069a09.16j", 0x000002, 0x100000, BAD_DUMP CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "k053246", 0 )
	ROM_LOAD64_BYTE( "069a08al.k3", 0x000000, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a08ah.n3", 0x000001, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a05al.k5", 0x000002, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a05ah.n5", 0x000003, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a07al.k7", 0x000004, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a07ah.n7", 0x000005, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a06al.k9", 0x000006, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a06ah.n9", 0x000007, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a08bl.l3", 0x200000, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a08bh.p3", 0x200001, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a05bl.l5", 0x200002, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a05bh.p5", 0x200003, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a07bl.l7", 0x200004, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a07bh.p7", 0x200005, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a06bl.l9", 0x200006, 0x40000, NO_DUMP )
	ROM_LOAD64_BYTE( "069a06bh.p9", 0x200007, 0x40000, NO_DUMP )
	// overlay standard ROMs for now
	ROM_LOAD64_WORD( "069a08.6h", 0x000000, 0x100000, BAD_DUMP CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD64_WORD( "069a05.1h", 0x000002, 0x100000, BAD_DUMP CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD64_WORD( "069a07.4h", 0x000004, 0x100000, BAD_DUMP CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD64_WORD( "069a06.2h", 0x000006, 0x100000, BAD_DUMP CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "k054539", 0 )
	ROM_LOAD( "069a04a.g2", 0x000000, 0x40000, NO_DUMP )
	ROM_LOAD( "069a04b.j2", 0x040000, 0x40000, NO_DUMP )
	ROM_LOAD( "069a04c.k2", 0x080000, 0x40000, NO_DUMP )
	ROM_LOAD( "069a04d.l2", 0x0c0000, 0x40000, NO_DUMP )
	ROM_LOAD( "069a04e.n2", 0x100000, 0x40000, NO_DUMP )
	ROM_LOAD( "069a04f.p2", 0x140000, 0x40000, NO_DUMP )
	ROM_LOAD( "069a04g.r2", 0x180000, 0x40000, NO_DUMP )
	ROM_LOAD( "069a04h.s2", 0x1c0000, 0x40000, NO_DUMP )
	// overlay standard ROM for now
	ROM_LOAD( "069a04a.1e", 0x000000, 0x200000, BAD_DUMP CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.7d", 0x0000, 0x080, CRC(33b07813) SHA1(aa4df1b4265e24cb79d1405dfdf5998689f6561e) ) // sldh
ROM_END

ROM_START( gijoej )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "069jaa03.14e", 0x000000,  0x40000, CRC(4b398901) SHA1(98fcc6ae9cc69c67d82eb1a7ab0bb71e61aee623) )
	ROM_LOAD16_BYTE( "069jaa02.18e", 0x000001,  0x40000, CRC(8bb22392) SHA1(9f066ce2b529f7dad6f80a91fff266c478d56414) )
	ROM_LOAD16_BYTE( "069a12.13e",   0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11.16e",   0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01.7c", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "k056832", 0 )
	ROM_LOAD32_WORD( "069a10.18j", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD32_WORD( "069a09.16j", 0x000002, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "k053246", 0 )
	ROM_LOAD64_WORD( "069a08.6h", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD64_WORD( "069a05.1h", 0x000002, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD64_WORD( "069a07.4h", 0x000004, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD64_WORD( "069a06.2h", 0x000006, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "k054539", 0 )
	ROM_LOAD( "069a04.1e", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.7d", 0x0000, 0x080, CRC(c914fcf2) SHA1(b4f0a0b5d6d4075b004b061336d162336ce1a754) ) // sldh
ROM_END

ROM_START( gijoea )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "069aa03.14e", 0x000000,  0x40000, CRC(74355c6e) SHA1(01d7b5994c5b9b6e87fb9a35ffed9cc540cfcd05) )
	ROM_LOAD16_BYTE( "069aa02.18e", 0x000001,  0x40000, CRC(d3dd0397) SHA1(6caac73d259ff6707ded2457b4968d3d0a3c4eb3) )
	ROM_LOAD16_BYTE( "069a12.13e",  0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11.16e",  0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01.7c", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "k056832", 0 )
	ROM_LOAD32_WORD( "069a10.18j", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD32_WORD( "069a09.16j", 0x000002, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "k053246", 0 )
	ROM_LOAD64_WORD( "069a08.6h", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD64_WORD( "069a05.1h", 0x000002, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD64_WORD( "069a07.4h", 0x000004, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD64_WORD( "069a06.2h", 0x000006, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "k054539", 0 )
	ROM_LOAD( "069a04.1e", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom
	ROM_LOAD( "er5911.7d", 0x0000, 0x080, CRC(6363513c) SHA1(181cbf2bd4960740d437c714dc70bb7e64c95348) ) // sldh
ROM_END

} // anonymous namespace


GAME( 1992, gijoe,   0,     gijoe, gijoe, gijoe_state, empty_init, ROT0, "Konami", "G.I. Joe (World, EAB, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, gijoeea, gijoe, gijoe, gijoe, gijoe_state, empty_init, ROT0, "Konami", "G.I. Joe (World, EB8, prototype?)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, gijoeu,  gijoe, gijoe, gijoe, gijoe_state, empty_init, ROT0, "Konami", "G.I. Joe (US, UAB)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, gijoeua, gijoe, gijoe, gijoe, gijoe_state, empty_init, ROT0, "Konami", "G.I. Joe (US, UAA)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, gijoej,  gijoe, gijoe, gijoe, gijoe_state, empty_init, ROT0, "Konami", "G.I. Joe (Japan, JAA)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, gijoea,  gijoe, gijoe, gijoe, gijoe_state, empty_init, ROT0, "Konami", "G.I. Joe (Asia, AA)", MACHINE_SUPPORTS_SAVE )
