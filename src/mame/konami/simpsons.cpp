// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

The Simpsons (c) 1991 Konami Co. Ltd

Preliminary driver by:
Ernesto Corvi
someone@secureshell.com


****************************************************************************

The Simpsons
Konami 1991

PCB Layout
----------

GX072 PWB352346B
|--------------------------------------------------------|
| MB3722   072D04.1D  072D05.1F           8464           |
|VOL VOL   YM2151   |------|            |------|072B08.3N|
|          YM3012   |053260| 3.579545MHz|053246|         |
|CN3                |      |            |      |         |
|                   |------|            |      |         |
|                   Z80     072E03.6G   |      |         |
|                                       |------|         |
|                           8416                         |
|                           2018        |------|         |
|  052535                   2018        |053247|072B09.8N|
|  052535                               |      |         |
|J 052535                               |      |         |
|A                                      |      |         |
|M 051550                               |------|         |
|M                                      |------|         |
|A     053994  053995   8464            |053251|         |
|                                       |      |072B10.12N
|   ER5911.12C                          |------|         |
|      072M13.13C             |------|  |------|         |
|005273(X10)      |------|    |052109|  |051962|         |
|      072L12.15C |053248|    |      |  |      |         |
|TEST_SW          |      |    |      |  |      |         |
|      072G02.16C |------|    |------|  |------|072B11.16L
|CN6                         8464 072B06.16H             |
|CN7   072G01.17C      24MHz 8464 072B07.18H             |
|--------------------------------------------------------|
Notes:
      ER5911 - EEPROM (128 bytes)
      8464   - Fujitsu MB8464 8kx8 SRAM (DIP28)
      8416   - Fujitsu MB8416 2kx8 SRAM (DIP24)
      2018   - Motorola MCM2018 2kx8 SRAM (DIP24)
      MB3722 - Audio Power AMP
      Z80    - Clock 3.579545MHz
      YM2151 - Clock 3.579545MHz
      YM3012 - Clock 1.7897725MHz [3.579545/2]
      CN6/7  - 15 pin connector for player 3 and player 4 controls
      CN3    - 4 pin connector for stereo sound output for left & right speaker
               (left speaker also outputs via JAMMA connector)

      Custom Chips
      ------------
      053248 - CPU (QFP80). Clock input 12.000MHz [24/2]. Clock output 3.000MHz [24/8]
      053260 - Sound chip (QFP80). Clock input 3.579545MHz. Clock output 1.7897725 [3.579545/2] for YM3012
      053246 \
      053247 / Sprite generators (QFP120)
      052109 \
      051962 / Tilemap Generators (QFP120)
      053251 - Priority encoder (QFP100)
      052535 - RGB DAC (ceramic encased SIP9)
      051550 - EMI filter for credit/coin counter (ceramic encased SIP23)
      005273 - Resistor array for player 3 & player 4 controls (ceramic encased SIP10)
      053994 \
      053995 / PALs (MMI PAL16L8, DIP20)

      ROMs
      ----
      072D04 -  256kx8 DIP40 MaskROM (Sound Samples)
      072D05 -  1Mx8 DIP40 MaskROM (Sound Samples)
      072E03 -  32kx8 MaskROM (Z80 Sound Program)
      072B08 \
      072B09  |
      072B10  | 512kx16 DIP40 MaskROM (Sprites)
      072B11 /
      072B06 \
      072B07 /  256kx16 DIP40 MaskROM (Tiles)
      072M13 \
      072L12  |
      072G02  | 128kx8 DIP32 MaskROM (Main Program)
      072G01 /

      Sync Measurements
      -----------------
      VSync - 59.1856Hz
      HSync - 15.1566kHz

***************************************************************************/

#include "emu.h"

#include "k052109.h"
#include "k053251.h"
#include "k053246_k053247_k055673.h"
#include "konamipt.h"
#include "konami_helper.h"

#include "cpu/m6809/konami.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/watchdog.h"
#include "sound/k053260.h"
#include "sound/ymopm.h"
#include "emupal.h"
#include "speaker.h"

#include <algorithm>


namespace {

class simpsons_state : public driver_device
{
public:
	simpsons_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_mainbank(*this, "mainbank"),
		m_audiobank(*this, "audiobank"),
		m_io_eepromout(*this, "EEPROMOUT"),
		m_palette_view(*this, "palette_view"),
		m_video_view(*this, "video_view")
	{ }

	void simpsons(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	/* devices */
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_memory_bank m_mainbank;
	required_memory_bank m_audiobank;
	required_ioport m_io_eepromout;

	/* memory pointers */
	std::unique_ptr<uint16_t[]>   m_spriteram;

	/* video-related */
	int32_t    m_sprite_colorbase = 0;
	int32_t    m_layer_colorbase[3]{};
	int32_t    m_layerpri[3]{};
	emu_timer *m_dma_start_timer;
	emu_timer *m_dma_end_timer;

	/* misc */
	bool       m_firq_enabled = false;
	emu_timer *m_nmi_blocked;

	/* views */
	memory_view m_palette_view;
	memory_view m_video_view;

	void z80_bankswitch_w(uint8_t data);
	void z80_arm_nmi_w(uint8_t data);
	void eeprom_w(uint8_t data);
	void coin_counter_w(uint8_t data);
	uint8_t sound_interrupt_r();
	uint8_t k052109_r(offs_t offset);
	void k052109_w(offs_t offset, uint8_t data);
	uint8_t k053247_r(offs_t offset);
	void k053247_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(periodic_irq);
	void video_bank_select(int bank);
	void object_dma();
	void z80_nmi_w(int state);
	K052109_CB_MEMBER(tile_callback);
	void banking_callback(u8 data);
	K053246_CB_MEMBER(sprite_callback);

	TIMER_CALLBACK_MEMBER(dma_start);
	TIMER_CALLBACK_MEMBER(dma_end);

	void main_map(address_map &map) ATTR_COLD;
	void z80_map(address_map &map) ATTR_COLD;
};



/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(simpsons_state::tile_callback)
{
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

K053246_CB_MEMBER(simpsons_state::sprite_callback)
{
	int const pri = (*color & 0x0f80) >> 6;   /* ??????? */

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

  Extra video banking

***************************************************************************/

uint8_t simpsons_state::k052109_r(offs_t offset)
{
	return m_k052109->read(offset + 0x2000);
}

void simpsons_state::k052109_w(offs_t offset, uint8_t data)
{
	m_k052109->write(offset + 0x2000, data);
}

uint8_t simpsons_state::k053247_r(offs_t offset)
{
	int const offs = offset >> 1;

	if (BIT(offset, 0))
		return(m_spriteram[offs] & 0xff);
	else
		return(m_spriteram[offs] >> 8);
}

void simpsons_state::k053247_w(offs_t offset, uint8_t data)
{
	int const offs = offset >> 1;

	if (BIT(offset, 0))
		m_spriteram[offs] = (m_spriteram[offs] & 0xff00) | data;
	else
		m_spriteram[offs] = (m_spriteram[offs] & 0x00ff) | (data << 8);
}

void simpsons_state::video_bank_select(int bank)
{
	if (BIT(bank, 0))
		m_palette_view.select(0);
	else
		m_palette_view.disable();
	m_video_view.select(BIT(bank, 1));
}



/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t simpsons_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update color info and refresh tilemaps
	static const int K053251_CI[3] = { k053251_device::CI2, k053251_device::CI3, k053251_device::CI4 };
	int const bg_colorbase = m_k053251->get_palette_index(k053251_device::CI0);
	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI1);

	for (int i = 0; i < 3; i++)
	{
		int const prev_colorbase = m_layer_colorbase[i];
		m_layer_colorbase[i] = m_k053251->get_palette_index(K053251_CI[i]);

		if (m_layer_colorbase[i] != prev_colorbase)
			m_k052109->mark_tilemap_dirty(i);
	}

	m_k052109->tilemap_update();

	// sort layers and draw
	int layer[3]{};
	for (int i = 0; i < 3; i++)
	{
		layer[i] = i;
		m_layerpri[i] = m_k053251->get_priority(K053251_CI[i]);
	}

	konami_sortlayers3(layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	m_k053246->k053247_sprites_draw(bitmap, cliprect);
	return 0;
}



/***************************************************************************

  EEPROM

***************************************************************************/

void simpsons_state::eeprom_w(uint8_t data)
{
	if (data == 0xff)
		return;

	m_io_eepromout->write(data, 0xff);

	video_bank_select(data & 0x03);

	m_firq_enabled = BIT(data, 2);
	if (!m_firq_enabled)
		m_maincpu->set_input_line(KONAMI_FIRQ_LINE, CLEAR_LINE);
}

/***************************************************************************

  Coin Counters, Sound Interface

***************************************************************************/

void simpsons_state::coin_counter_w(uint8_t data)
{
	/* bit 0,1 coin counters */
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	/* bit 2 selects mono or stereo sound */
	/* bit 3 = enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line(BIT(data, 3));
	/* bit 4 = INIT (unknown) */
	/* bit 5 = enable sprite ROM reading */
	m_k053246->k053246_set_objcha_line(BIT(~data, 5));
}

uint8_t simpsons_state::sound_interrupt_r()
{
	if (!machine().side_effects_disabled())
		m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80

	return 0x00;
}


/***************************************************************************

  Banking, initialization

***************************************************************************/

void simpsons_state::banking_callback(u8 data)
{
	m_mainbank->set_entry(data & 0x3f);
}

void simpsons_state::machine_start()
{
	m_spriteram = make_unique_clear<uint16_t[]>(0x1000 / 2);

	m_mainbank->configure_entries(0, 64, memregion("maincpu")->base(), 0x2000);

	m_audiobank->configure_entries(0, 2, memregion("audiocpu")->base() + 0x10000, 0);
	m_audiobank->configure_entries(2, 6, memregion("audiocpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_firq_enabled));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));
	save_pointer(NAME(m_spriteram), 0x1000 / 2);

	m_dma_start_timer = timer_alloc(FUNC(simpsons_state::dma_start), this);
	m_dma_end_timer = timer_alloc(FUNC(simpsons_state::dma_end), this);
	m_nmi_blocked = timer_alloc(timer_expired_delegate());
}

void simpsons_state::machine_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_layerpri[i] = 0;
		m_layer_colorbase[i] = 0;
	}

	m_sprite_colorbase = 0;
	m_firq_enabled = false;

	/* init the default banks */
	m_mainbank->set_entry(0);
	m_audiobank->set_entry(0);
	video_bank_select(0);

	m_dma_start_timer->adjust(attotime::never);
	m_dma_end_timer->adjust(attotime::never);

	// Z80 _NMI goes low at same time as reset
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}



/***************************************************************************

  Memory Maps

***************************************************************************/

void simpsons_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rw(m_k052109, FUNC(k052109_device::read), FUNC(k052109_device::write));
	map(0x0000, 0x0fff).view(m_palette_view);
	m_palette_view[0](0x0000, 0x0fff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x1f80, 0x1f80).portr("COIN");
	map(0x1f81, 0x1f81).portr("TEST");
	map(0x1f90, 0x1f90).portr("P1");
	map(0x1f91, 0x1f91).portr("P2");
	map(0x1f92, 0x1f92).portr("P3");
	map(0x1f93, 0x1f93).portr("P4");
	map(0x1fa0, 0x1fa7).w(m_k053246, FUNC(k053247_device::k053246_w));
	map(0x1fb0, 0x1fbf).w(m_k053251, FUNC(k053251_device::write));
	map(0x1fc0, 0x1fc0).w(FUNC(simpsons_state::coin_counter_w));
	map(0x1fc2, 0x1fc2).w(FUNC(simpsons_state::eeprom_w));
	map(0x1fc4, 0x1fc4).r(FUNC(simpsons_state::sound_interrupt_r));
	map(0x1fc6, 0x1fc7).rw("k053260", FUNC(k053260_device::main_read), FUNC(k053260_device::main_write));
	map(0x1fc8, 0x1fc9).r(m_k053246, FUNC(k053247_device::k053246_r));
	map(0x1fca, 0x1fca).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x2000, 0x3fff).view(m_video_view);
	m_video_view[0](0x2000, 0x3fff).rw(FUNC(simpsons_state::k052109_r), FUNC(simpsons_state::k052109_w));
	m_video_view[1](0x2000, 0x2fff).rw(FUNC(simpsons_state::k053247_r), FUNC(simpsons_state::k053247_w));
	m_video_view[1](0x3000, 0x3fff).ram();
	map(0x4000, 0x5fff).ram();
	map(0x6000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0x78000);
}

void simpsons_state::z80_bankswitch_w(uint8_t data)
{
	m_audiobank->set_entry(data & 7);
}


TIMER_CALLBACK_MEMBER(simpsons_state::dma_start)
{
	if (m_firq_enabled)
		m_maincpu->set_input_line(KONAMI_FIRQ_LINE, ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(simpsons_state::dma_end)
{
	m_maincpu->set_input_line(KONAMI_FIRQ_LINE, CLEAR_LINE);
}


void simpsons_state::z80_arm_nmi_w(uint8_t data)
{
	// LD $(FA00), A takes 13 cycles. 4*M1 + 3*read + 3*read + 3*write.
	//
	// The Z80 checks if NMI has gone from high to low during the instruction, on the rising edge of CLK, at the start of the last cycle (in this case cycle 3 of the write).
	// The circuit raises NMI when MREQ/WR goes high, on the falling edge of CLK, half way through cycle 3 of the write.
	// NMI is then lowered when the sound chips timer output subsequently goes from low to high.
	//
	// MAME instead does not emulate memory cycle timing and checks the NMI before executing an instruction,
	// so we have to manually delay the NMI until the following HALT instruction has started.
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_nmi_blocked->adjust(m_audiocpu->cycles_to_attotime(4));
}

void simpsons_state::z80_nmi_w(int state)
{
	if (state && !m_nmi_blocked->enabled())
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void simpsons_state::z80_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xfa00, 0xfa00).w(FUNC(simpsons_state::z80_arm_nmi_w));
	map(0xfc00, 0xfc2f).rw("k053260", FUNC(k053260_device::read), FUNC(k053260_device::write));
	map(0xfe00, 0xfe00).w(FUNC(simpsons_state::z80_bankswitch_w));
}

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( simpsons )
	PORT_START("P1")
	KONAMI8_B12_START(1)

	PORT_START("P2")
	KONAMI8_B12_START(2)

	PORT_START("P3")
	KONAMI8_B12_START(3)

	PORT_START("P4")
	KONAMI8_B12_START(4)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE1 Unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE2 Unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE3 Unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE4 Unused

	PORT_START("TEST")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_BIT( 0xce, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
INPUT_PORTS_END

static INPUT_PORTS_START( simpsn2p )
	PORT_START("P1")
	KONAMI8_B12_START(1)

	PORT_START("P2")
	KONAMI8_B12_START(2)

	PORT_START("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN") /* IN4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) //COIN3 Unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) //COIN4 Unused
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE2 Unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE3 Unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE4 Unused

	PORT_START("TEST")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_BIT( 0xce, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
INPUT_PORTS_END



/***************************************************************************

    Machine Driver

***************************************************************************/

void simpsons_state::object_dma()
{
	uint16_t *dst;
	m_k053246->k053247_get_ram(&dst);

	uint16_t const *src = m_spriteram.get();
	int num_inactive = 256;

	for (int counter = 256; counter; --counter)
	{
		if (BIT(*src, 15) && (*src & 0xff))
		{
			dst = std::copy_n(src, 8, dst);
			num_inactive--;
		}
		src += 8;
	}

	while (num_inactive--)
	{
		*dst = 0;
		dst += 8;
	}
}

INTERRUPT_GEN_MEMBER(simpsons_state::periodic_irq)
{
	if (m_k053246->k053246_is_irq_enabled())
	{
		object_dma();
		m_dma_start_timer->adjust(attotime::from_ticks(256, XTAL(24'000'000)/4));
		m_dma_end_timer->adjust(attotime::from_ticks(256+2048, XTAL(24'000'000)/4));
	}
}

void simpsons_state::simpsons(machine_config &config)
{
	/* basic machine hardware */
	KONAMI(config, m_maincpu, XTAL(24'000'000)/2); /* 053248, the clock input is 12MHz, and internal CPU divider of 4 */
	m_maincpu->set_addrmap(AS_PROGRAM, &simpsons_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(simpsons_state::periodic_irq)); /* IRQ triggered by the 052109, FIRQ by the sprite hardware */
	m_maincpu->line().set(FUNC(simpsons_state::banking_callback));

	Z80(config, m_audiocpu, XTAL(3'579'545)); /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &simpsons_state::z80_map);   /* NMIs are generated by the 053260 */

	EEPROM_ER5911_8BIT(config, "eeprom");

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));

	// Screen timings generated by the 051962, probably not programmable (except maybe between 6 and 8MHz)
	// 6MHz dot clock
	// horizontal: 16 cycles front porch, 32 cycles sync, 16 cycles back porch
	// vertical: 16 lines front porch, 8 lines sync, 16 lines back porch
	screen.set_raw(XTAL(24'000'000)/4, 384, 0+16, 320-16, 264, 0, 224);
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_screen_update(FUNC(simpsons_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 2048).enable_shadows().enable_hilights();

	K052109(config, m_k052109, 0);
	m_k052109->set_xy_offset(-96, -16);
	m_k052109->set_palette("palette");
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(simpsons_state::tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, KONAMI_IRQ_LINE);

	K053246(config, m_k053246, 0);
	m_k053246->set_sprite_callback(FUNC(simpsons_state::sprite_callback));
	m_k053246->set_config(NORMAL_PLANE_ORDER, -43, 39);
	m_k053246->set_palette("palette");

	K053251(config, m_k053251, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(3'579'545))); /* verified on pcb */
	ymsnd.add_route(0, "lspeaker", 1.0);    /* only left channel is connected */
	ymsnd.add_route(0, "rspeaker", 1.0);
	ymsnd.add_route(1, "lspeaker", 0.0);
	ymsnd.add_route(1, "rspeaker", 0.0);

	k053260_device &k053260(K053260(config, "k053260", XTAL(3'579'545))); /* verified on pcb */
	k053260.add_route(0, "lspeaker", 1.00);
	k053260.add_route(1, "rspeaker", 1.00);
	k053260.sh1_cb().set(FUNC(simpsons_state::z80_nmi_w));
}


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( simpsons ) /* World 4 Player */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "072-g02.16c", 0x00000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "072-g01.17c", 0x20000, 0x20000, CRC(9f843def) SHA1(858432b59101b0577c5cec6ac0c7c20ab0780c9a) )
	ROM_LOAD( "072-j13.13c", 0x40000, 0x20000, CRC(aade2abd) SHA1(10f178d5ed399b4866266e075d91ca3db26798f8) )
	ROM_LOAD( "072-j12.15c", 0x60000, 0x20000, CRC(479e12f2) SHA1(15a6cb12e68b4773a29ab463640a43f8e814de59) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-e03.6g", 0x00000, 0x08000, CRC(866b7a35) SHA1(98905764eb4c7d968ccc17618a1f24ee12e33c0e) )
	ROM_CONTINUE(       0x10000, 0x18000 )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )
	ROM_LOAD32_WORD( "072-b06.16h", 0x000002, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "k053246", 0 ) /* graphics */
	ROM_LOAD64_WORD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) ) /* sprites */
	ROM_LOAD64_WORD( "072-b09.8n",  0x000002, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD64_WORD( "072-b10.12n", 0x000004, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD64_WORD( "072-b11.16l", 0x000006, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "simpsons.12c.nv", 0x0000, 0x080, CRC(ec3f0449) SHA1(da35b98cd10bfabe9df3ede05462fabeb0e01ca9) )
ROM_END

ROM_START( simpsons4pe ) /* World 4 Player, later? (by use of later letters) */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "072-g02.16c", 0x00000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "072-g01.17c", 0x20000, 0x20000, CRC(9f843def) SHA1(858432b59101b0577c5cec6ac0c7c20ab0780c9a) )
	ROM_LOAD( "072-m13.13c", 0x40000, 0x20000, CRC(f36c9423) SHA1(4a7311ffcb2e6916006c1e79dfc231e7fc570781) )
	ROM_LOAD( "072-l12.15c", 0x60000, 0x20000, CRC(84f9d9ba) SHA1(d52f999b7c8125daea5e9b5754c6e82c17861d1b) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-e03.6g", 0x00000, 0x08000, CRC(866b7a35) SHA1(98905764eb4c7d968ccc17618a1f24ee12e33c0e) )
	ROM_CONTINUE(       0x10000, 0x18000 )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )
	ROM_LOAD32_WORD( "072-b06.16h", 0x000002, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "k053246", 0 ) /* graphics */
	ROM_LOAD64_WORD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) ) /* sprites */
	ROM_LOAD64_WORD( "072-b09.8n",  0x000002, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD64_WORD( "072-b10.12n", 0x000004, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD64_WORD( "072-b11.16l", 0x000006, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "simpsons4pe.12c.nv", 0x0000, 0x080, CRC(ec3f0449) SHA1(da35b98cd10bfabe9df3ede05462fabeb0e01ca9) )
ROM_END

ROM_START( simpsons4pe2 ) // PCB is original Konami GX072 but EPROM labels are from E. Devices. Only two of the program ROMs differ.
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "2 e. devices.16c", 0x00000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "1 e. devices.17c", 0x20000, 0x20000, CRC(9f843def) SHA1(858432b59101b0577c5cec6ac0c7c20ab0780c9a) )
	ROM_LOAD( "4 e. devices.13c", 0x40000, 0x20000, CRC(5f0ada6a) SHA1(7aec6083210b751084d32709c9dc50afe524fc02) )
	ROM_LOAD( "3 e. devices.15c", 0x60000, 0x20000, CRC(22d00d1f) SHA1(ed73695fb55c5cf931e536c34ef504f0f7b6213d) )

	ROM_REGION( 0x28000, "audiocpu", 0 )
	ROM_LOAD( "5 e. devices.6g", 0x00000, 0x08000, CRC(866b7a35) SHA1(98905764eb4c7d968ccc17618a1f24ee12e33c0e) )
	ROM_CONTINUE(                0x10000, 0x18000 )

	ROM_REGION( 0x100000, "k052109", 0 ) // tiles
	ROM_LOAD32_WORD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )
	ROM_LOAD32_WORD( "072-b06.16h", 0x000002, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "k053246", 0 ) // sprites
	ROM_LOAD64_WORD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) )
	ROM_LOAD64_WORD( "072-b09.8n",  0x000002, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD64_WORD( "072-b10.12n", 0x000004, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD64_WORD( "072-b11.16l", 0x000006, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "k053260", 0 ) // samples
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "simpsons4pe2.12c.nv", 0x0000, 0x080, CRC(ec3f0449) SHA1(da35b98cd10bfabe9df3ede05462fabeb0e01ca9) )
ROM_END

ROM_START( simpsons4pa ) /* Asia 4 Player */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "072-v02.16c", 0x00000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "072-v01.17c", 0x20000, 0x20000, CRC(effd6c09) SHA1(e5bcdb753bccdd76de18ad6ff7346f74fd02a78f) )
	ROM_LOAD( "072-x13.13c", 0x40000, 0x20000, CRC(3304abb9) SHA1(8f23160077f30d76c0c73e0b3f20996826433566) )
	ROM_LOAD( "072-x12.15c", 0x60000, 0x20000, CRC(fa4fca12) SHA1(3b52a8a52bddfa73d8577315b655eb57ac758326) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-g03.6g", 0x00000, 0x08000, CRC(76c1850c) SHA1(9047c6b26c4e33c74eb7400a807d3d9f206f7bbe) )
	ROM_CONTINUE(       0x10000, 0x18000 )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )
	ROM_LOAD32_WORD( "072-b06.16h", 0x000002, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "k053246", 0 ) /* graphics */
	ROM_LOAD64_WORD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) ) /* sprites */
	ROM_LOAD64_WORD( "072-b09.8n",  0x000002, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD64_WORD( "072-b10.12n", 0x000004, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD64_WORD( "072-b11.16l", 0x000006, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "simpsons4pa.12c.nv", 0x0000, 0x080, CRC(ec3f0449) SHA1(da35b98cd10bfabe9df3ede05462fabeb0e01ca9) )
ROM_END


ROM_START( simpsons2p ) /* World 2 Player */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "072-g02.16c", 0x00000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "072-p01.17c", 0x20000, 0x20000, CRC(07ceeaea) SHA1(c18255ae1d578c2d53de80d6323cdf41cbe47b57) )
	ROM_LOAD( "072-013.13c", 0x40000, 0x20000, CRC(8781105a) SHA1(ef2f16f7a56d3715536511c674df4b3aab1be2bd) )
	ROM_LOAD( "072-012.15c", 0x60000, 0x20000, CRC(244f9289) SHA1(eeda7f5c7340cbd1a1cd576af48cd5d1a629914a) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-g03.6g", 0x00000, 0x08000, CRC(76c1850c) SHA1(9047c6b26c4e33c74eb7400a807d3d9f206f7bbe) )
	ROM_CONTINUE(       0x10000, 0x18000 )

	ROM_REGION( 0x100000, "k052109", 0 )
	ROM_LOAD32_WORD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )
	ROM_LOAD32_WORD( "072-b06.16h", 0x000002, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "k053246", 0 ) /* graphics */
	ROM_LOAD64_WORD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) ) /* sprites */
	ROM_LOAD64_WORD( "072-b09.8n",  0x000002, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD64_WORD( "072-b10.12n", 0x000004, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD64_WORD( "072-b11.16l", 0x000006, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "simpsons2p.12c.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )
ROM_END

ROM_START( simpsons2p2 ) /* World 2 Player, alt */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "072-g02.16c", 0x00000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "072-p01.17c", 0x20000, 0x20000, CRC(07ceeaea) SHA1(c18255ae1d578c2d53de80d6323cdf41cbe47b57) )
	ROM_LOAD( "072-_13.13c", 0x40000, 0x20000, CRC(54e6df66) SHA1(1b83ae56cf1deb51b04880fa421f06568c938a99) ) /* Unknown revision/region code */
	ROM_LOAD( "072-_12.15c", 0x60000, 0x20000, CRC(96636225) SHA1(5de95606e5c9337f18bc42f4df791cacafa20399) ) /* Unknown revision/region code */

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-g03.6g", 0x00000, 0x08000, CRC(76c1850c) SHA1(9047c6b26c4e33c74eb7400a807d3d9f206f7bbe) )
	ROM_CONTINUE(       0x10000, 0x18000 )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )
	ROM_LOAD32_WORD( "072-b06.16h", 0x000002, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "k053246", 0 ) /* graphics */
	ROM_LOAD64_WORD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) ) /* sprites */
	ROM_LOAD64_WORD( "072-b09.8n",  0x000002, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD64_WORD( "072-b10.12n", 0x000004, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD64_WORD( "072-b11.16l", 0x000006, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "simpsons2p2.12c.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )
ROM_END

ROM_START( simpsons2p3 ) // no rom labels
	ROM_REGION( 0x80000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "072-g02.16c", 0x00000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "072-p01.17c", 0x20000, 0x20000, CRC(07ceeaea) SHA1(c18255ae1d578c2d53de80d6323cdf41cbe47b57) )
	ROM_LOAD( "4.13c", 0x40000, 0x20000, CRC(c3040e4f) SHA1(f6b5cbee5d7c6642a11d115bb6d93a7f2821cd8f) ) /* Unknown revision/region code */
	ROM_LOAD( "3.15c", 0x60000, 0x20000, CRC(eb4f5781) SHA1(58a556e9b4b9e4bd0e76ac86ab8e062c3f1e2d31) ) /* Unknown revision/region code */

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-g03.6g", 0x00000, 0x08000, CRC(76c1850c) SHA1(9047c6b26c4e33c74eb7400a807d3d9f206f7bbe) )
	ROM_CONTINUE(       0x10000, 0x18000 )

	ROM_REGION( 0x100000, "k052109", 0 )
	ROM_LOAD32_WORD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )
	ROM_LOAD32_WORD( "072-b06.16h", 0x000002, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "k053246", 0 ) /* graphics */
	ROM_LOAD64_WORD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) ) /* sprites */
	ROM_LOAD64_WORD( "072-b09.8n",  0x000002, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD64_WORD( "072-b10.12n", 0x000004, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD64_WORD( "072-b11.16l", 0x000006, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "simpsons2p.12c.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )
ROM_END

ROM_START( simpsons2pa ) /* Asia 2 Player */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "072-g02.16c", 0x00000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) ) /* Same as both world 2p sets */
	ROM_LOAD( "072-p01.17c", 0x20000, 0x20000, CRC(07ceeaea) SHA1(c18255ae1d578c2d53de80d6323cdf41cbe47b57) ) /* Same as both world 2p sets */
	ROM_LOAD( "072-113.13c", 0x40000, 0x20000, CRC(8781105a) SHA1(ef2f16f7a56d3715536511c674df4b3aab1be2bd) ) /* Same as world set simpsons2p */
	ROM_LOAD( "072-112.15c", 0x60000, 0x20000, CRC(3bd69404) SHA1(e055fed7e9bde8315ae2f9b2d35bc05fece6b80b) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-e03.6g", 0x00000, 0x08000, CRC(866b7a35) SHA1(98905764eb4c7d968ccc17618a1f24ee12e33c0e) )
	ROM_CONTINUE(       0x10000, 0x18000 )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )
	ROM_LOAD32_WORD( "072-b06.16h", 0x000002, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "k053246", 0 ) /* graphics */
	ROM_LOAD64_WORD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) ) /* sprites */
	ROM_LOAD64_WORD( "072-b09.8n",  0x000002, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD64_WORD( "072-b10.12n", 0x000004, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD64_WORD( "072-b11.16l", 0x000006, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "simpsons2pa.12c.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )
ROM_END

ROM_START( simpsons2pj ) /* Japan 2 Player */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "072-s02.16c", 0x00000, 0x20000, CRC(265f7a47) SHA1(d39c19a5e303f822313409343b209947f4c47ae4) )
	ROM_LOAD( "072-t01.17c", 0x20000, 0x20000, CRC(91de5c2d) SHA1(1e18a5585ed821ec7cda69bdcdbfa4e6c71455c6) )
	ROM_LOAD( "072-213.13c", 0x40000, 0x20000, CRC(b326a9ae) SHA1(f222c33f2e8b306f2f0ef6f0da9febbf8219e1a4) )
	ROM_LOAD( "072-212.15c", 0x60000, 0x20000, CRC(584d9d37) SHA1(61b9df4dfb323b7284894e5e1eb9d713ebf64721) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-g03.6g", 0x00000, 0x08000, CRC(76c1850c) SHA1(9047c6b26c4e33c74eb7400a807d3d9f206f7bbe) )
	ROM_CONTINUE(       0x10000, 0x18000 )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )
	ROM_LOAD32_WORD( "072-b06.16h", 0x000002, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "k053246", 0 ) /* graphics */
	ROM_LOAD64_WORD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) ) /* sprites */
	ROM_LOAD64_WORD( "072-b09.8n",  0x000002, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD64_WORD( "072-b10.12n", 0x000004, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD64_WORD( "072-b11.16l", 0x000006, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "simpsons2pj.12c.nv", 0x0000, 0x080, CRC(3550a54e) SHA1(370cd40a12c471b3b6690ecbdde9c7979bc2a652) )
ROM_END

} // anonymous namespace



/***************************************************************************

  Game driver(s)

***************************************************************************/

// the region warning, if one exists, is shown after the high-score screen in attract mode
GAME( 1991, simpsons,     0,        simpsons, simpsons, simpsons_state, empty_init, ROT0, "Konami", "The Simpsons (4 Players World, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, simpsons4pe,  simpsons, simpsons, simpsons, simpsons_state, empty_init, ROT0, "Konami", "The Simpsons (4 Players World, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, simpsons4pe2, simpsons, simpsons, simpsons, simpsons_state, empty_init, ROT0, "Konami", "The Simpsons (4 Players World, set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, simpsons4pa,  simpsons, simpsons, simpsons, simpsons_state, empty_init, ROT0, "Konami", "The Simpsons (4 Players Asia)",         MACHINE_SUPPORTS_SAVE )
GAME( 1991, simpsons2p,   simpsons, simpsons, simpsn2p, simpsons_state, empty_init, ROT0, "Konami", "The Simpsons (2 Players World, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, simpsons2p2,  simpsons, simpsons, simpsons, simpsons_state, empty_init, ROT0, "Konami", "The Simpsons (2 Players World, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, simpsons2p3,  simpsons, simpsons, simpsn2p, simpsons_state, empty_init, ROT0, "Konami", "The Simpsons (2 Players World, set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, simpsons2pa,  simpsons, simpsons, simpsn2p, simpsons_state, empty_init, ROT0, "Konami", "The Simpsons (2 Players Asia)",         MACHINE_SUPPORTS_SAVE )
GAME( 1991, simpsons2pj,  simpsons, simpsons, simpsn2p, simpsons_state, empty_init, ROT0, "Konami", "The Simpsons (2 Players Japan)",        MACHINE_SUPPORTS_SAVE )
