// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    Vendetta (GX081) (c) 1991 Konami

    Preliminary driver by:
    Ernesto Corvi
    someone@secureshell.com

    Notes:
    - collision detection is handled by a protection chip. Its emulation might
      not be 100% accurate.

********************************************************************************
   Game driver for "ESCAPE KIDS (TM)"  (KONAMI, 1991)
 --------------------------------------------------------------------------------

            This driver was made on the basis of 'src/drivers/vendetta.cpp' file.
                                         Driver by OHSAKI Masayuki (2002/08/13)

 ********************************************************************************


 ***** NOTES *****
      -------
  1) ESCAPE KIDS uses 053246's unknown function. (see video/k053246_k053247_k055673.cpp)
                   (053246 register #5  UnKnown Bit #5, #3, #2 always set "1")


 ***** On the "error.log" *****
      --------------------
  1) "YM2151 Write 00 to undocumented register #xx" (xx=00-1f)
                Why???

  2) "xxxx: read from unknown 052109 address yyyy"
  3) "xxxx: write zz to unknown 052109 address yyyy"
                These are video/k052109.cpp's message.
                "video/k052109.cpp" checks 052109 RAM area access.
                If accessed over 0x1800 (0x3800), logged 2) or 3) messages.
                Escape Kids use 0x1800-0x19ff and 0x3800-0x39ff area.


 ***** UnEmulated *****
      ------------
  2) 0x7c00 (Banked ROM area) access to data WRITE (???)
  3) 0x3fda (053248 RAM area) access to data WRITE (Watchdog ???)


 ***** ESCAPE KIDS PCB layout/ Need to dump *****
      --------------------------------------
   (Parts side view)
   +-------------------------------------------------------+
   |   R          ROM9                               [CN1] |  CN1:Player4 Input?
   |   O                                             [CN2] |           (Labeled '4P')
   |   M          ROM8                       ROM1    [SW1] |  CN2:Player3 Input?
   |   7                              [CUS1]             +-+           (Labeled '3P')
   |        [CUS7]   [CUS8]                              +-+  CN3:Stereo sound out
   | R                                       [CUS2]        |
   | O                                                   J |  SW1:Test Switch
   | M                                                   A |
   | 6    [CUS6]                                         M | ***  Custom Chips  ***
   |                                                     M |      CUS1: 053248
   | R                                                   A |      CUS2: 053252
   | O    [CUS5]                                        56P|      CUS3: 053260
   | M                                                     |      CUS4: 053246
   | 5                           ROM2  [ Z80 ]           +-+      CUS5: 053247
   |                                                     +-+      CUS6: 053251
   | R    [CUS4]                     [CUS3] [YM2151] [CN3] |      CUS7: 051962
   | O                                                     |      CUS8: 052109
   | M                                 ROM3                |
   | 4                                         [Sound AMP] |
   +-------------------------------------------------------+

  ***  Dump ROMs  ***
     1) ROM1 (17C)  32Pin 1Mbit UV-EPROM          -> save "975r01" file
     2) ROM2 ( 5F)  28Pin 512Kbit One-Time PROM   -> save "975f02" file
     3) ROM3 ( 1D)  40Pin 4Mbit mask ROM          -> save "975c03" file
     4) ROM4 ( 3K)  42Pin 8Mbit mask ROM          -> save "975c04" file
     5) ROM5 ( 8L)  42Pin 8Mbit mask ROM          -> save "975c05" file
     6) ROM6 (12M)  42Pin 8Mbit mask ROM          -> save "975c06" file
     7) ROM7 (16K)  42Pin 8Mbit mask ROM          -> save "975c07" file
     8) ROM8 (16I)  40Pin 4Mbit mask ROM          -> save "975c08" file
     9) ROM9 (18I)  40Pin 4Mbit mask ROM          -> save "975c09" file
                                                        vvvvvvvvvvvv
                                                        esckidsj.zip

***************************************************************************/

#include "emu.h"

#include "konamipt.h"
#include "k052109.h"
#include "k053246_k053247_k055673.h"
#include "k053251.h"
#include "k054000.h"
#include "konami_helper.h"

#include "cpu/m6809/konami.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/k053252.h"
#include "machine/watchdog.h"
#include "sound/k053260.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "speaker.h"


namespace {

class vendetta_state : public driver_device
{
public:
	vendetta_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"),
		m_k054000(*this, "k054000"),
		m_palette(*this, "palette"),
		m_videoview0(*this, "videoview0"),
		m_videoview1(*this, "videoview1"),
		m_mainbank(*this, "mainbank"),
		m_eeprom_out(*this, "EEPROMOUT")
	{ }

	void esckids(machine_config &config);
	void vendetta(machine_config &config);

	int obj_busy_r() { return m_obj_busy->enabled() ? 1 : 0; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// video-related
	uint8_t m_layer_colorbase[3]{};
	uint8_t m_sprite_colorbase = 0;
	int m_layerpri[3]{};

	// misc
	uint8_t m_irq_enabled = 0;
	emu_timer *m_nmi_blocked;
	emu_timer *m_obj_busy;

	// devices
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	optional_device<k053252_device> m_k053252;
	optional_device<k054000_device> m_k054000;
	required_device<palette_device> m_palette;

	// views
	memory_view m_videoview0;
	memory_view m_videoview1;

	required_memory_bank m_mainbank;

	required_ioport m_eeprom_out;

	void eeprom_w(uint8_t data);
	uint8_t K052109_r(offs_t offset);
	void K052109_w(offs_t offset, uint8_t data);
	void _5fe0_w(uint8_t data);
	void z80_arm_nmi_w(uint8_t data);
	void z80_nmi_w(int state);
	void z80_irq_w(uint8_t data = 0);
	uint8_t z80_irq_r();
	void vblank_irq(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	K052109_CB_MEMBER(vendetta_tile_callback);
	K052109_CB_MEMBER(esckids_tile_callback);
	void banking_callback(uint8_t data);
	K053246_CB_MEMBER(sprite_callback);

	void esckids_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(vendetta_state::vendetta_tile_callback)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x30) << 6) | ((*color & 0x0c) << 10) | (bank << 14);
	*color = m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

K052109_CB_MEMBER(vendetta_state::esckids_tile_callback)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) <<  9) | (bank << 13);
	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >>  5);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

K053246_CB_MEMBER(vendetta_state::sprite_callback)
{
	int pri = (*color & 0x03e0) >> 4; // ???????
	if (pri <= m_layerpri[2])
		*priority_mask = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x001f);
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t vendetta_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update color info and refresh tilemaps
	static const int K053251_CI[3] = { k053251_device::CI2, k053251_device::CI3, k053251_device::CI4 };
	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI1);

	for (int i = 0; i < 3; i++)
	{
		int prev_colorbase = m_layer_colorbase[i];
		m_layer_colorbase[i] = m_k053251->get_palette_index(K053251_CI[i]);

		if (m_layer_colorbase[i] != prev_colorbase)
			m_k052109->mark_tilemap_dirty(i);
	}

	m_k052109->tilemap_update();

	// sort layers and draw
	int layer[3];
	for (int i = 0; i < 3; i++)
	{
		layer[i] = i;
		m_layerpri[i] = m_k053251->get_priority(K053251_CI[i]);
	}

	konami_sortlayers3(layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	m_k053246->k053247_sprites_draw(bitmap, cliprect);
	return 0;
}


/***************************************************************************

  EEPROM

***************************************************************************/

void vendetta_state::eeprom_w(uint8_t data)
{
	// bit 0 - VOC0 - Video banking related
	// bit 1 - VOC1 - Video banking related
	// bit 2 - MSCHNG - Mono Sound select (Amp)
	// bit 3 - EEPCS - EEPROM CS
	// bit 4 - EEPCLK - EEPROM clock
	// bit 5 - EEPDI - EEPROM data
	// bit 6 - IRQ enable
	// bit 7 - Unused

	if (data == 0xff ) // this is a bug in the EEPROM write code
		return;

	// EEPROM
	m_eeprom_out->write(data, 0xff);

	m_irq_enabled = (data >> 6) & 1;
	if (!m_irq_enabled)
		m_maincpu->set_input_line(KONAMI_IRQ_LINE, CLEAR_LINE);

	m_videoview0.select(BIT(data, 0));
	m_videoview1.select(BIT(data, 0));
}

/********************************************/

uint8_t vendetta_state::K052109_r(offs_t offset)
{
	return m_k052109->read(offset + 0x2000);
}

void vendetta_state::K052109_w(offs_t offset, uint8_t data)
{
	// *************************************************************************************
	// *  Escape Kids uses 052109's mirrored Tilemap ROM bank selector, but only during    *
	// *  Tilemap mask ROM Test       (0x1d80<->0x3d80, 0x1e00<->0x3e00, 0x1f00<->0x3f00)  *
	// *************************************************************************************
	if ((offset == 0x1d80) || (offset == 0x1e00) || (offset == 0x1f00))
		m_k052109->write(offset, data);
	m_k052109->write(offset + 0x2000, data);
}


void vendetta_state::_5fe0_w(uint8_t data)
{
	// bit 0,1 coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	// bit 2 = BRAMBK ??

	// bit 3 = enable char ROM reading through the video RAM
	m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

	// bit 4 = INIT ??

	// bit 5 = enable sprite ROM reading
	m_k053246->k053246_set_objcha_line((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
}

void vendetta_state::z80_arm_nmi_w(uint8_t data)
{
	// see notes in simpsons driver
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_nmi_blocked->adjust(m_audiocpu->cycles_to_attotime(4));
}

void vendetta_state::z80_nmi_w(int state)
{
	if (state && !m_nmi_blocked->enabled())
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void vendetta_state::z80_irq_w(uint8_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

uint8_t vendetta_state::z80_irq_r()
{
	if (!machine().side_effects_disabled())
		z80_irq_w();

	return 0x00;
}

/********************************************/

void vendetta_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr(m_mainbank);
	map(0x2000, 0x3fff).ram();

	// what is the desired effect of overlapping these memory regions anyway?
	map(0x4000, 0x7fff).rw(m_k052109, FUNC(k052109_device::read), FUNC(k052109_device::write));

	map(0x4000, 0x4fff).view(m_videoview0);
	m_videoview0[0](0x4000, 0x4fff).rw(m_k052109, FUNC(k052109_device::read), FUNC(k052109_device::write));
	m_videoview0[1](0x4000, 0x4fff).rw(m_k053246, FUNC(k053247_device::k053247_r), FUNC(k053247_device::k053247_w));
	map(0x5f80, 0x5f9f).m(m_k054000, FUNC(k054000_device::map));
	map(0x5fa0, 0x5faf).w(m_k053251, FUNC(k053251_device::write));
	map(0x5fb0, 0x5fb7).w(m_k053246, FUNC(k053247_device::k053246_w));
	map(0x5fc0, 0x5fc0).portr("P1");
	map(0x5fc1, 0x5fc1).portr("P2");
	map(0x5fc2, 0x5fc2).portr("P3");
	map(0x5fc3, 0x5fc3).portr("P4");
	map(0x5fd0, 0x5fd0).portr("EEPROM");
	map(0x5fd1, 0x5fd1).portr("SERVICE");
	map(0x5fe0, 0x5fe0).w(FUNC(vendetta_state::_5fe0_w));
	map(0x5fe2, 0x5fe2).w(FUNC(vendetta_state::eeprom_w));
	map(0x5fe4, 0x5fe4).rw(FUNC(vendetta_state::z80_irq_r), FUNC(vendetta_state::z80_irq_w));
	map(0x5fe6, 0x5fe7).rw("k053260", FUNC(k053260_device::main_read), FUNC(k053260_device::main_write));
	map(0x5fe8, 0x5fe9).r(m_k053246, FUNC(k053247_device::k053246_r));
	map(0x5fea, 0x5fea).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x6000, 0x6fff).view(m_videoview1);
	m_videoview1[0](0x6000, 0x6fff).rw(FUNC(vendetta_state::K052109_r), FUNC(vendetta_state::K052109_w));
	m_videoview1[1](0x6000, 0x6fff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x8000, 0xffff).rom().region("maincpu", 0x38000);
}

void vendetta_state::esckids_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();                         // 053248 64K SRAM
	// what is the desired effect of overlapping these memory regions anyway?
	map(0x2000, 0x5fff).rw(m_k052109, FUNC(k052109_device::read), FUNC(k052109_device::write));            // 052109 (Tilemap)

	map(0x2000, 0x2fff).view(m_videoview0);    // 052109 (Tilemap) 0x0000-0x0fff - 052109 (Tilemap)
	m_videoview0[0](0x2000, 0x2fff).rw(m_k052109, FUNC(k052109_device::read), FUNC(k052109_device::write));
	m_videoview0[1](0x2000, 0x2fff).rw(m_k053246, FUNC(k053247_device::k053247_r), FUNC(k053247_device::k053247_w));
	map(0x3f80, 0x3f80).portr("P1");
	map(0x3f81, 0x3f81).portr("P2");
	map(0x3f82, 0x3f82).portr("P3");             // ???  (But not used)
	map(0x3f83, 0x3f83).portr("P4");             // ???  (But not used)
	map(0x3f92, 0x3f92).portr("EEPROM");
	map(0x3f93, 0x3f93).portr("SERVICE");
	map(0x3fa0, 0x3fa7).w(m_k053246, FUNC(k053247_device::k053246_w));           // Sprite
	map(0x3fb0, 0x3fbf).w(m_k053251, FUNC(k053251_device::write));           // Priority Encoder
	map(0x3fc0, 0x3fcf).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write));
	map(0x3fd0, 0x3fd0).w(FUNC(vendetta_state::_5fe0_w));      // Coin Counter, 052109 RMRD, 053246 OBJCHA
	map(0x3fd2, 0x3fd2).w(FUNC(vendetta_state::eeprom_w));    // EEPROM, Video banking
	map(0x3fd4, 0x3fd4).rw(FUNC(vendetta_state::z80_irq_r), FUNC(vendetta_state::z80_irq_w));            // Sound
	map(0x3fd6, 0x3fd7).rw("k053260", FUNC(k053260_device::main_read), FUNC(k053260_device::main_write)); // Sound
	map(0x3fd8, 0x3fd9).r(m_k053246, FUNC(k053247_device::k053246_r));                // Sprite
	map(0x3fda, 0x3fda).nopw();                // Not Emulated (Watchdog ???)
	map(0x4000, 0x4fff).view(m_videoview1);    // Tilemap mask ROM bank selector (mask ROM Test)
	m_videoview1[0](0x4000, 0x4fff).rw(FUNC(vendetta_state::K052109_r), FUNC(vendetta_state::K052109_w));
	m_videoview1[1](0x4000, 0x4fff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x6000, 0x7fff).bankr(m_mainbank);                    // 053248 '975r01' 1M ROM (Banked)
	map(0x8000, 0xffff).rom().region("maincpu", 0x18000);  // 053248 '975r01' 1M ROM (0x18000-0x1ffff)
}

void vendetta_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xfa00, 0xfa00).w(FUNC(vendetta_state::z80_arm_nmi_w));
	map(0xfc00, 0xfc2f).rw("k053260", FUNC(k053260_device::read), FUNC(k053260_device::write));
}


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( vendet4p )
	PORT_START("P1")
	KONAMI8_RL_B12_COIN(1)

	PORT_START("P2")
	KONAMI8_RL_B12_COIN(2)

	PORT_START("P3")
	KONAMI8_RL_B12_COIN(3)

	PORT_START("P4")
	KONAMI8_RL_B12_COIN(4)

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vendetta_state::obj_busy_r))
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
INPUT_PORTS_END

static INPUT_PORTS_START( vendettan )
	PORT_INCLUDE( vendet4p )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )
INPUT_PORTS_END

static INPUT_PORTS_START( vendet2p )
	PORT_INCLUDE( vendet4p )

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( esckids4p )
	PORT_INCLUDE( vendet4p )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )
INPUT_PORTS_END

static INPUT_PORTS_START( esckids2p )
	PORT_INCLUDE( esckids4p )

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************

    Machine Driver

***************************************************************************/

void vendetta_state::vblank_irq(int state)
{
	if (state)
	{
		if (m_irq_enabled)
			m_maincpu->set_input_line(KONAMI_IRQ_LINE, ASSERT_LINE);

		// OBJ DMA enabled
		if (m_k053246->k053246_is_irq_enabled())
			m_obj_busy->adjust(attotime::from_usec(250));
	}
}

void vendetta_state::machine_start()
{
	m_mainbank->configure_entries(0, 28, memregion("maincpu")->base(), 0x2000);
	m_mainbank->set_entry(0);

	m_nmi_blocked = timer_alloc(timer_expired_delegate());
	m_obj_busy = timer_alloc(timer_expired_delegate());

	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));
}

void vendetta_state::machine_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_layerpri[i] = 0;
		m_layer_colorbase[i] = 0;
	}

	m_sprite_colorbase = 0;
	m_irq_enabled = 0;

	// Z80 _NMI goes low at same time as reset
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void vendetta_state::banking_callback(uint8_t data)
{
	if (data >= 0x1c)
		logerror("%s Unknown bank selected %02x\n", machine().describe_context(), data);
	else
		m_mainbank->set_entry(data);
}

void vendetta_state::vendetta(machine_config &config)
{
	// basic machine hardware
	KONAMI(config, m_maincpu, XTAL(24'000'000) / 2); // 052001 (verified on PCB)
	m_maincpu->set_addrmap(AS_PROGRAM, &vendetta_state::main_map);
	m_maincpu->line().set(FUNC(vendetta_state::banking_callback));

	Z80(config, m_audiocpu, XTAL(3'579'545)); // verified with PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &vendetta_state::sound_map); // interrupts are triggered by the main CPU

	EEPROM_ER5911_8BIT(config, "eeprom");

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.17); // measured on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 32*8);
	screen.set_visarea(13*8, (64-13)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(vendetta_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(vendetta_state::vblank_irq));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_tile_callback(FUNC(vendetta_state::vendetta_tile_callback));

	K053246(config, m_k053246, 0);
	m_k053246->set_sprite_callback(FUNC(vendetta_state::sprite_callback));
	m_k053246->set_config(NORMAL_PLANE_ORDER, 53, 6);
	m_k053246->set_palette(m_palette);

	K053251(config, m_k053251, 0);
	K054000(config, m_k054000, 0);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "speaker", 0.5, 0).add_route(1, "speaker", 0.5, 1); // verified with PCB

	k053260_device &k053260(K053260(config, "k053260", XTAL(3'579'545))); // verified with PCB
	k053260.add_route(0, "speaker", 0.75, 0);
	k053260.add_route(1, "speaker", 0.75, 1);
	k053260.sh1_cb().set(FUNC(vendetta_state::z80_nmi_w));
}

void vendetta_state::esckids(machine_config &config)
{
	vendetta(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &vendetta_state::esckids_map);

	//subdevice<screen_device>("screen")->set_visarea(13*8, (64-13)*8-1, 2*8, 30*8-1); // black areas on the edges
	subdevice<screen_device>("screen")->set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1);

	config.device_remove("k054000");
	config.device_remove("k052109");

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_tile_callback(FUNC(vendetta_state::esckids_tile_callback));

	m_k053246->set_config(NORMAL_PLANE_ORDER, 101, 6);

	K053252(config, "k053252", XTAL(24'000'000) / 4).set_offsets(12*8, 1*8);
}



/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( vendetta )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "081t01.17c", 0x00000, 0x40000, CRC(e76267f5) SHA1(efef6c2edb4c181374661f358dad09123741b63d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END

ROM_START( vendettar )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "081r01.17c", 0x00000, 0x40000, CRC(84796281) SHA1(e4330c6eaa17adda5b4bd3eb824388c89fb07918) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendettar.nv", 0x0000, 0x080, CRC(ec3f0449) SHA1(da35b98cd10bfabe9df3ede05462fabeb0e01ca9) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END

ROM_START( vendettaz )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "081z01.17c", 0x00000, 0x40000, CRC(4d225a8d) SHA1(fe8f6e63d033cf04c9a287d870db244fddb81f03) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END

ROM_START( vendettaun )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "1.17c", 0x00000, 0x40000, CRC(1a7ceb1b) SHA1(c7454e11b7a06d10c94fe44ba6f83208bca4ced9) ) // World 4 player, program ROM found labeled simply as "1"

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END

/*
This set has also been found with a different ROM layout, using three daughter boards:
  27c512.5f                                   081b02              IDENTICAL
  27c020.17c                                  081w01.17c          IDENTICAL
  1d-3-daughter-board-27c040.bin              081a03      [1/2]   IDENTICAL
  1d-2-daughter-board-27c040.bin              081a03      [2/2]   IDENTICAL
  8-yellow-sticker-daughter-board-27c040.bin  081a04      [even]  IDENTICAL
  4-yellow-sticker-daughter-board-27c040.bin  081a05      [even]  IDENTICAL
  27c020.17j                                  081a09      [even]  IDENTICAL
  6-yellow-sticker-daughter-board-27c040.bin  081a06      [even]  IDENTICAL
  27c020.16j                                  081a08      [even]  IDENTICAL
  2-yellow-sticker-daughter-board-27c040.bin  081a07      [even]  IDENTICAL
  27c020.17h                                  081a09      [odd]   IDENTICAL
  3-yellow-sticker-daughter-board-27c040.bin  081a05      [odd]   IDENTICAL
  7-yellow-sticker-daughter-board-27c040.bin  081a04      [odd]   IDENTICAL
  5-yellow-sticker-daughter-board-27c040.bin  081a06      [odd]   IDENTICAL
  1-yellow-sticker-daughter-board-27c040.bin  081a07      [odd]   IDENTICAL
  27c020.16h                                  081a08      [odd]   IDENTICAL
*/
ROM_START( vendetta2pw )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "081w01.17c", 0x00000, 0x40000, CRC(cee57132) SHA1(8b6413877e127511daa76278910c2ee3247d613a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END

ROM_START( vendetta2peba )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "081-eb-a01.17c", 0x00000, 0x40000, CRC(8430bb52) SHA1(54e896510fa44e76b0640b17150210fbf6b3b5bc)) // Label was unclear apart from EB stamp on the middle line.  Bottom line looked like 401, but probably A01

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END

ROM_START( vendetta2pun )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "1.17c", 0x00000, 0x40000, CRC(b4edde48) SHA1(bf6342cfeb0560cdf9c943f6d112fd89ee5a4f6b) ) // World 2 player, program ROM found labeled simply as "1"

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END

ROM_START( vendetta2pu )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "081u01.17c", 0x00000, 0x40000, CRC(b4d9ade5) SHA1(fbd543738cb0b68c80ff05eed7849b608de03395) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END

ROM_START( vendetta2pd )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "081d01.17c", 0x00000, 0x40000, CRC(335da495) SHA1(ea74680eb898aeecf9f1eec95f151bcf66e6b6cb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END

ROM_START( vendettan )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "081n01.17c", 0x00000, 0x40000, CRC(fc766fab) SHA1(a22c82810f2a2b66fc112e2d043e8025d0dc2841) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendettaj.nv", 0x0000, 0x080, CRC(3550a54e) SHA1(370cd40a12c471b3b6690ecbdde9c7979bc2a652) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END

ROM_START( vendetta2pp )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "081p01.17c", 0x00000, 0x40000, CRC(5fe30242) SHA1(2ea98e66637fa2ad60044b1a2b0dd158a82403a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "k053246", 0 ) // graphics
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) // sprites
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) // sprites
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) // sprites
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) // sprites

	ROM_REGION( 0x100000, "k053260", 0 ) // samples
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "vendettaj.nv", 0x0000, 0x080, CRC(3550a54e) SHA1(370cd40a12c471b3b6690ecbdde9c7979bc2a652) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "p1-pal16l8acn.17e", 0x000, 0x117, BAD_DUMP CRC(eae70da3) SHA1(2707ff413ea1fdc4e483f437f44a40042aa41d4e) ) // Bruteforced
	ROM_LOAD( "p2-pal16l8acn.14e", 0x117, 0x117, BAD_DUMP CRC(b84abb7d) SHA1(c3744771c486a4db0d5a067100318f8f076c1aa2) ) // Bruteforced
ROM_END


ROM_START( esckids )
	ROM_REGION( 0x020000, "maincpu", 0 )        // Main CPU (053248) Code & Banked (1M x 1)
	ROM_LOAD( "17c.bin", 0x000000, 0x020000, CRC(9dfba99c) SHA1(dbcb89aad5a9addaf7200b2524be999877313a6e) )

	ROM_REGION( 0x010000, "audiocpu", 0 )       // Sound CPU (Z80) Code (512K x 1)
	ROM_LOAD( "975f02", 0x000000, 0x010000, CRC(994fb229) SHA1(bf194ae91240225b8edb647b1a62cd83abfa215e) )

	ROM_REGION( 0x100000, "k052109", 0 )       // Tilemap mask ROM (4M x 2)
	ROM_LOAD32_WORD( "975c09", 0x000000, 0x080000, CRC(bc52210e) SHA1(301a3892d250495c2e849d67fea5f01fb0196bed) )
	ROM_LOAD32_WORD( "975c08", 0x000002, 0x080000, CRC(fcff9256) SHA1(b60d29f4d04f074120d4bb7f2a71b9e9bf252d33) )

	ROM_REGION( 0x400000, "k053246", 0 )       // Sprite mask ROM (8M x 4)
	ROM_LOAD64_WORD( "975c04", 0x000000, 0x100000, CRC(15688a6f) SHA1(a445237a11e5f98f0f9b2573a7ef0583366a137e) )
	ROM_LOAD64_WORD( "975c05", 0x000002, 0x100000, CRC(1ff33bb7) SHA1(eb17da33ba2769ea02f91fece27de2e61705e75a) )
	ROM_LOAD64_WORD( "975c06", 0x000004, 0x100000, CRC(36d410f9) SHA1(2b1fd93c11839480aa05a8bf27feef7591704f3d) )
	ROM_LOAD64_WORD( "975c07", 0x000006, 0x100000, CRC(97ec541e) SHA1(d1aa186b17cfe6e505f5b305703319299fa54518) )

	ROM_REGION( 0x100000, "k053260", 0 )    // Samples mask ROM (4M x 1)
	ROM_LOAD( "975c03", 0x000000, 0x080000, CRC(dc4a1707) SHA1(f252d08483fd664f8fc03bf8f174efd452b4cdc5) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "esckids.nv", 0x0000, 0x080, CRC(a8522e1f) SHA1(43f82fce3c3b854bc8898c63dffc7c01b288c8aa) )
ROM_END


ROM_START( esckidsj )
	ROM_REGION( 0x020000, "maincpu", 0 )        // Main CPU (053248) Code & Banked (1M x 1)
	ROM_LOAD( "975r01", 0x000000, 0x020000, CRC(7b5c5572) SHA1(b94b58c010539926d112c2dfd80bcbad76acc986) )

	ROM_REGION( 0x010000, "audiocpu", 0 )       // Sound CPU (Z80) Code (512K x 1)
	ROM_LOAD( "975f02", 0x000000, 0x010000, CRC(994fb229) SHA1(bf194ae91240225b8edb647b1a62cd83abfa215e) )

	ROM_REGION( 0x100000, "k052109", 0 )       // Tilemap mask ROM (4M x 2)
	ROM_LOAD32_WORD( "975c09", 0x000000, 0x080000, CRC(bc52210e) SHA1(301a3892d250495c2e849d67fea5f01fb0196bed) )
	ROM_LOAD32_WORD( "975c08", 0x000002, 0x080000, CRC(fcff9256) SHA1(b60d29f4d04f074120d4bb7f2a71b9e9bf252d33) )

	ROM_REGION( 0x400000, "k053246", 0 )       // Sprite mask ROM (8M x 4)
	ROM_LOAD64_WORD( "975c04", 0x000000, 0x100000, CRC(15688a6f) SHA1(a445237a11e5f98f0f9b2573a7ef0583366a137e) )
	ROM_LOAD64_WORD( "975c05", 0x000002, 0x100000, CRC(1ff33bb7) SHA1(eb17da33ba2769ea02f91fece27de2e61705e75a) )
	ROM_LOAD64_WORD( "975c06", 0x000004, 0x100000, CRC(36d410f9) SHA1(2b1fd93c11839480aa05a8bf27feef7591704f3d) )
	ROM_LOAD64_WORD( "975c07", 0x000006, 0x100000, CRC(97ec541e) SHA1(d1aa186b17cfe6e505f5b305703319299fa54518) )

	ROM_REGION( 0x100000, "k053260", 0 )    // Samples mask ROM (4M x 1)
	ROM_LOAD( "975c03", 0x000000, 0x080000, CRC(dc4a1707) SHA1(f252d08483fd664f8fc03bf8f174efd452b4cdc5) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default EEPROM to prevent game booting upside down with error
	ROM_LOAD( "esckidsj.nv", 0x0000, 0x080, CRC(985e2a2d) SHA1(afd9e5fc014d593d0a384326f32caf2a73fba867) )
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

GAME( 1991, vendetta,     0,        vendetta, vendet4p,  vendetta_state, empty_init, ROT0, "Konami", "Vendetta (World, 4 Players, ver. T)",         MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendettar,    vendetta, vendetta, vendet4p,  vendetta_state, empty_init, ROT0, "Konami", "Vendetta (US, 4 Players, ver. R)",            MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendettaz,    vendetta, vendetta, vendet4p,  vendetta_state, empty_init, ROT0, "Konami", "Vendetta (Asia, 4 Players, ver. Z)",          MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendettaun,   vendetta, vendetta, vendet4p,  vendetta_state, empty_init, ROT0, "Konami", "Vendetta (World, 4 Players, ver. ?)",         MACHINE_SUPPORTS_SAVE ) // program ROM labeled as 1
GAME( 1991, vendetta2pw,  vendetta, vendetta, vendet2p,  vendetta_state, empty_init, ROT0, "Konami", "Vendetta (World, 2 Players, ver. W)",         MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendetta2peba,vendetta, vendetta, vendet2p,  vendetta_state, empty_init, ROT0, "Konami", "Vendetta (World, 2 Players, ver. EB-A?)",     MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendetta2pun, vendetta, vendetta, vendet2p,  vendetta_state, empty_init, ROT0, "Konami", "Vendetta (World, 2 Players, ver. ?)",         MACHINE_SUPPORTS_SAVE ) // program ROM labeled as 1
GAME( 1991, vendetta2pu,  vendetta, vendetta, vendet2p,  vendetta_state, empty_init, ROT0, "Konami", "Vendetta (Asia, 2 Players, ver. U)",          MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendetta2pd,  vendetta, vendetta, vendet2p,  vendetta_state, empty_init, ROT0, "Konami", "Vendetta (Asia, 2 Players, ver. D)",          MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendettan,    vendetta, vendetta, vendettan, vendetta_state, empty_init, ROT0, "Konami", "Crime Fighters 2 (Japan, 4 Players, ver. N)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendetta2pp,  vendetta, vendetta, vendet2p,  vendetta_state, empty_init, ROT0, "Konami", "Crime Fighters 2 (Japan, 2 Players, ver. P)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, esckids,      0,        esckids,  esckids4p, vendetta_state, empty_init, ROT0, "Konami", "Escape Kids (Asia, 4 Players)",               MACHINE_SUPPORTS_SAVE )
GAME( 1991, esckidsj,     esckids,  esckids,  esckids2p, vendetta_state, empty_init, ROT0, "Konami", "Escape Kids (Japan, 2 Players)",              MACHINE_SUPPORTS_SAVE )
