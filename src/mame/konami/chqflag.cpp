// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Manuel Abadia
/***************************************************************************

    Chequered Flag / Checkered Flag (GX717) (c) Konami 1988

    Main board: PWB(C)350761A
    IO board:   PWB(C)450871A

    Notes:
    - 007232 volume & panning control is almost certainly wrong;
    - 051733 opponent cars have wrong RNG colors compared to references;
    - 051733 opponent car-to-car collisions direction are wrong, according
      to reference orange car should shift to the left instead (current emulation
      makes them to wall crash most of the time instead);
    - needs proper shadow/highlight factor values for sprites and tilemap;
    - compared to references, emulation is a bit slower (around 2/3 seconds
      behind on a full lap of stage 2);

    2008-07
    Dip locations and recommended settings verified with manual

***************************************************************************/

#include "emu.h"

#include "k051733.h"
#include "k051960.h"
#include "konamipt.h"
#include "konami_helper.h"

#include "cpu/z80/z80.h"
#include "cpu/m6809/konami.h"
#include "machine/adc0804.h"
#include "machine/gen_latch.h"
#include "machine/rescap.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "sound/ymopm.h"
#include "video/k051316.h"
#include "emupal.h"
#include "speaker.h"

#include "chqflag.lh"

namespace {

class chqflag_state : public driver_device
{
public:
	chqflag_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_bank1000(*this, "bank1000")
		, m_k007232(*this, "k007232_%u", 1)
		, m_k051960(*this, "k051960")
		, m_k051316(*this, "k051316_%u", 1)
		, m_palette(*this, "palette")
		, m_rombank(*this, "rombank")
		, m_analog_input(*this, "IN%u", 3U)
		, m_start_lamp(*this, "start_lamp")
	{
	}

	void chqflag(machine_config &config);

private:
	template<int Chip> uint8_t k051316_ramrom_r(offs_t offset);
	void chqflag_bankswitch_w(uint8_t data);
	void chqflag_vreg_w(uint8_t data);
	void select_analog_ctrl_w(uint8_t data);
	uint8_t analog_read_r();
	void k007232_bankswitch_w(uint8_t data);
	void k007232_extvolume_w(uint8_t data);
	void volume_callback0(uint8_t data);
	void volume_callback1(uint8_t data);
	K051316_CB_MEMBER(zoom_callback_1);
	K051316_CB_MEMBER(zoom_callback_2);
	K051960_CB_MEMBER(sprite_callback);
	uint32_t screen_update_chqflag(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void chqflag_map(address_map &map) ATTR_COLD;
	void chqflag_sound_map(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
private:
	/* misc */
	int        m_k051316_readroms = 0;
	int        m_last_vreg = 0;
	int        m_analog_ctrl = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	memory_view m_bank1000;
	required_device_array<k007232_device, 2> m_k007232;
	required_device<k051960_device> m_k051960;
	required_device_array<k051316_device, 2> m_k051316;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_memory_bank m_rombank;
	void update_background_shadows(uint8_t data);

	required_ioport_array<2> m_analog_input;
	output_finder<> m_start_lamp;
};


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(chqflag_state::sprite_callback)
{
	enum { sprite_colorbase = 0 };

	*priority = (*color & 0x10) ? 0 : GFX_PMASK_1;
	*color = sprite_colorbase + (*color & 0x0f);
}

/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(chqflag_state::zoom_callback_1)
{
	enum { zoom_colorbase_1 = 256 / 16 };

	*code |= ((*color & 0x03) << 8);
	*color = zoom_colorbase_1 + ((*color & 0x3c) >> 2);
}

K051316_CB_MEMBER(chqflag_state::zoom_callback_2)
{
	enum { zoom_colorbase_2 = 512 / 256 };

	*code |= ((*color & 0x0f) << 8);
	*color = zoom_colorbase_2 + ((*color & 0x10) >> 4);
}

/***************************************************************************

    Display Refresh

***************************************************************************/

uint32_t chqflag_state::screen_update_chqflag(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_k051316[1]->zoom_draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_k051316[1]->zoom_draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	m_k051316[0]->zoom_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/* these trampolines are less confusing than nested address_map_bank_devices */
template<int Chip>
uint8_t chqflag_state::k051316_ramrom_r(offs_t offset)
{
	if (m_k051316_readroms)
		return m_k051316[Chip]->rom_r(offset);
	else
		return m_k051316[Chip]->read(offset);
}

void chqflag_state::chqflag_bankswitch_w(uint8_t data)
{
	/* bits 0-4 = ROM bank # (0x00-0x11) */
	int bankaddress = data & 0x1f;
	if (bankaddress < (0x50000 / 0x4000))
		m_rombank->set_entry(bankaddress);

	/* bit 5 = select work RAM or k051316 + palette */
	m_bank1000.select(BIT(data, 5));

	/* other bits unknown/unused */
}

void chqflag_state::chqflag_vreg_w(uint8_t data)
{
	/* bits 0 & 1 = coin counters */
	machine().bookkeeping().coin_counter_w(1, data & 0x01);
	machine().bookkeeping().coin_counter_w(0, data & 0x02);

	/* bit 4 = enable rom reading through K051316 #1 & #2 */
	m_k051316_readroms = (data & 0x10);

	/* Bits 3-7 probably control palette dimming in a similar way to TMNT2/Sunset Riders, */
	/* however I don't have enough evidence to determine the exact behaviour. */
	/* Bits 3 and 7 are set in night stages, where the background should get darker and */
	/* the headlight (which have the shadow bit set) become highlights */
	/* Maybe one of the bits inverts the SHAD line while the other darkens the background. */
	/*
	 * Update according to a reference:
	 * 0x00 is certainly shadow (car pit-in shadow when zoomed in/clouds before rain)
	 * 0x80 is used when rain shows up (which should be white/highlighted)
	 * 0x88 is for when night shows up (max amount of highlight)
	 * 0x08 is used at dawn after 0x88 state
	 * The shadow part looks ugly when rain starts/ends pouring (-> black colored with a setting of 0x00),
	 * the reference shows dimmed background when this event occurs,
	 * might be actually disabling the shadow here (-> setting 1.0f instead).
	 *
	 * TODO: true values aren't known, also shadow_factors table probably scales towards zero instead (game doesn't use those)
	 */
	const double shadow_factors[4] = {0.8, 1.0, 1.33, 1.66};
	uint8_t shadow_value = (data & 0x08) >> 3;
	uint8_t shadow_setting = (data & 0x80) >> 7;

	m_k051960->set_shadow_inv(shadow_setting);

	m_palette->set_shadow_factor(shadow_factors[(shadow_setting << 1) + shadow_value]);

	if (shadow_setting != m_last_vreg)
	{
		m_last_vreg = shadow_setting;
		update_background_shadows(shadow_setting);
	}

	#if 0
	if ((data & 0x80) != m_last_vreg)
	{
		m_last_vreg = data & 0x80;

		/* only affect the background */
		update_background_shadows(data);
	}
	#endif

//if ((data & 0xf8) && (data & 0xf8) != 0x88)
//  popmessage("chqflag_vreg_w %02x",data);


	/* other bits unknown. bit 5 is used. */
}

void chqflag_state::select_analog_ctrl_w(uint8_t data)
{
	m_start_lamp = BIT(data, 1);
	m_analog_ctrl = data;
}

uint8_t chqflag_state::analog_read_r()
{
	return m_analog_input[m_analog_ctrl & 0x01]->read();
}


/****************************************************************************/

void chqflag_state::chqflag_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).view(m_bank1000);
	m_bank1000[0](0x1000, 0x1fff).ram();
	m_bank1000[1](0x1000, 0x17ff).r(FUNC(chqflag_state::k051316_ramrom_r<0>)).w(m_k051316[0], FUNC(k051316_device::write));
	m_bank1000[1](0x1800, 0x1fff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2000, 0x2007).rw(m_k051960, FUNC(k051960_device::k051937_r), FUNC(k051960_device::k051937_w));            /* Sprite control registers */
	map(0x2400, 0x27ff).rw(m_k051960, FUNC(k051960_device::k051960_r), FUNC(k051960_device::k051960_w));            /* Sprite RAM */
	map(0x2800, 0x2fff).r(FUNC(chqflag_state::k051316_ramrom_r<1>)).w(m_k051316[1], FUNC(k051316_device::write)); /* 051316 zoom/rotation (chip 2) */
	map(0x3000, 0x3000).w("soundlatch", FUNC(generic_latch_8_device::write));                    /* sound code # */
	map(0x3001, 0x3001).w("soundlatch2", FUNC(generic_latch_8_device::write));                  /* cause interrupt on audio CPU */
	map(0x3002, 0x3002).w(FUNC(chqflag_state::chqflag_bankswitch_w));                     /* bankswitch control */
	map(0x3003, 0x3003).w(FUNC(chqflag_state::chqflag_vreg_w));                           /* enable K051316 ROM reading */
	map(0x3100, 0x3100).portr("DSW1");                               /* DIPSW #1  */
	map(0x3200, 0x3200).portr("IN1");                                /* COINSW, STARTSW, test mode */
	map(0x3201, 0x3201).portr("IN0");                                /* DIPSW #3, SW 4 */
	map(0x3203, 0x3203).portr("DSW2");                               /* DIPSW #2 */
	map(0x3300, 0x3300).w("watchdog", FUNC(watchdog_timer_device::reset_w)); /* watchdog timer */
	map(0x3400, 0x341f).rw("k051733", FUNC(k051733_device::read), FUNC(k051733_device::write));                    /* 051733 (protection) */
	map(0x3500, 0x350f).w(m_k051316[0], FUNC(k051316_device::ctrl_w));                            /* 051316 control registers (chip 1) */
	map(0x3600, 0x360f).w(m_k051316[1], FUNC(k051316_device::ctrl_w));                            /* 051316 control registers (chip 2) */
	map(0x3700, 0x3700).w(FUNC(chqflag_state::select_analog_ctrl_w));                     /* select accelerator/wheel */
	map(0x3701, 0x3701).portr("IN2");                                /* Brake + Shift + ? */
	map(0x3702, 0x3702).rw("adc", FUNC(adc0804_device::read), FUNC(adc0804_device::write));  /* accelerator/wheel */
	map(0x4000, 0x7fff).bankr("rombank");                              /* banked ROM */
	map(0x8000, 0xffff).rom().region("maincpu", 0x48000);               /* ROM */
}


void chqflag_state::k007232_bankswitch_w(uint8_t data)
{
	int bank_A, bank_B;

	/* banks # for the 007232 (chip 1) */
	bank_A = ((data >> 4) & 0x03);
	bank_B = ((data >> 6) & 0x03);
	m_k007232[0]->set_bank(bank_A, bank_B);

	/* banks # for the 007232 (chip 2) */
	bank_A = ((data >> 0) & 0x03);
	bank_B = ((data >> 2) & 0x03);
	m_k007232[1]->set_bank(bank_A, bank_B);
}

void chqflag_state::chqflag_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom(); /* ROM */
	map(0x8000, 0x87ff).ram(); /* RAM */
	map(0x9000, 0x9000).w(FUNC(chqflag_state::k007232_bankswitch_w)); /* 007232 bankswitch */
	map(0xa000, 0xa00d).rw(m_k007232[0], FUNC(k007232_device::read), FUNC(k007232_device::write));  /* 007232 (chip 1) */
	map(0xa01c, 0xa01c).w(FUNC(chqflag_state::k007232_extvolume_w));  /* extra volume, goes to the 007232 w/ A4 */
															/* selecting a different latch for the external port */
	map(0xb000, 0xb00d).rw(m_k007232[1], FUNC(k007232_device::read), FUNC(k007232_device::write));  /* 007232 (chip 2) */
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));   /* YM2151 */
	map(0xd000, 0xd000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe000).r("soundlatch2", FUNC(generic_latch_8_device::read));  /* engine sound volume */
	map(0xf000, 0xf000).nopw();                    /* ??? */
}


static INPUT_PORTS_START( chqflag )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* Invalid = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )   /* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )   /* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )   /* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )   /* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )   /* Manual says it's not used */
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:4" )   /* Manual says it's not used */

	PORT_START("IN1")
	/* COINSW + STARTSW */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* DIPSW #3 */
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW3:1" )   /* Manual says it's not used */
	PORT_DIPNAME( 0x40, 0x40, "Title" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, "Chequered Flag" )
	PORT_DIPSETTING(    0x00, "Checkered Flag" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("IN2")   /* Brake, Shift + ??? */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* if this is set, it goes directly to test mode */
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", FUNC(adc0804_device::intr_r))

	PORT_START("IN3")   /* Accelerator */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("IN4")   /* Driving wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xef) PORT_SENSITIVITY(80) PORT_KEYDELTA(8)
INPUT_PORTS_END

static INPUT_PORTS_START( chqflagj )
	PORT_INCLUDE( chqflag )

	PORT_MODIFY("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), " 1 Coin/1 Credit", SW1)
	// Manual says 1-5, 1-6, 1-7 and 1-8 are not used, but they work

	PORT_MODIFY("IN1")
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:2" )   /* Manual says it's not used */
INPUT_PORTS_END



void chqflag_state::volume_callback0(uint8_t data)
{
	// volume/pan for one of the channels on this chip
	// which channel and which bits are left/right is a guess
	m_k007232[0]->set_volume(0, (data & 0x0f) * 0x11/2, (data >> 4) * 0x11/2);
}

void chqflag_state::k007232_extvolume_w(uint8_t data)
{
	// volume/pan for one of the channels on this chip
	// which channel and which bits are left/right is a guess
	m_k007232[0]->set_volume(1, (data & 0x0f) * 0x11/2, (data >> 4) * 0x11/2);
}

void chqflag_state::volume_callback1(uint8_t data)
{
	m_k007232[1]->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232[1]->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void chqflag_state::machine_start()
{
	m_rombank->configure_entries(0, 0x50000 / 0x4000, memregion("maincpu")->base(), 0x4000);

	m_start_lamp.resolve();

	save_item(NAME(m_k051316_readroms));
	save_item(NAME(m_last_vreg));
	save_item(NAME(m_analog_ctrl));
}

void chqflag_state::machine_reset()
{
	m_k051316_readroms = 0;
	m_last_vreg = 0;
	m_analog_ctrl = 0;
	update_background_shadows(0);
}

inline void chqflag_state::update_background_shadows(uint8_t data)
{
	double brt = (data & 1) ? 0.8 : 1.0;

	for (int i = 512; i < 1024; i++)
		m_palette->set_pen_contrast(i, brt);
}

void chqflag_state::chqflag(machine_config &config)
{
	/* basic machine hardware */
	KONAMI(config, m_maincpu, XTAL(24'000'000)/2);    /* 052001 (verified on pcb) */
	m_maincpu->set_addrmap(AS_PROGRAM, &chqflag_state::chqflag_map);

	Z80(config, m_audiocpu, XTAL(3'579'545)); /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &chqflag_state::chqflag_sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	WATCHDOG_TIMER(config, "watchdog");

	ADC0804(config, "adc", RES_K(10), CAP_P(150)).vin_callback().set(FUNC(chqflag_state::analog_read_r));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(24'000'000)/3, 528, 96, 400, 256, 16, 240); // measured Vsync 59.17hz Hsync 15.13 / 15.19khz
//  6MHz dotclock is more realistic, however needs drawing updates. replace when ready
//  screen.set_raw(XTAL(24'000'000)/4, 396, hbend, hbstart, 256, 16, 240);
	screen.set_screen_update(FUNC(chqflag_state::screen_update_chqflag));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);
	m_palette->enable_shadows();

	K051960(config, m_k051960, 0);
	m_k051960->set_palette(m_palette);
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(chqflag_state::sprite_callback));
	m_k051960->irq_handler().set_inputline(m_maincpu, KONAMI_IRQ_LINE);
	m_k051960->nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	K051316(config, m_k051316[0], 0);
	m_k051316[0]->set_palette(m_palette);
	m_k051316[0]->set_offsets(7, 0);
	m_k051316[0]->set_zoom_callback(FUNC(chqflag_state::zoom_callback_1));

	K051316(config, m_k051316[1], 0);
	m_k051316[1]->set_palette(m_palette);
	m_k051316[1]->set_bpp(8);
	m_k051316[1]->set_layermask(0xc0);
	m_k051316[1]->set_wrap(1);
	m_k051316[1]->set_zoom_callback(FUNC(chqflag_state::zoom_callback_2));

	K051733(config, "k051733", 0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, "soundlatch");
	GENERIC_LATCH_8(config, "soundlatch2").data_pending_callback().set_inputline(m_audiocpu, 0);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(3'579'545))); /* verified on pcb */
	ymsnd.irq_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ymsnd.add_route(0, "speaker", 1.00, 0);
	ymsnd.add_route(1, "speaker", 1.00, 1);

	K007232(config, m_k007232[0], XTAL(3'579'545)); /* verified on pcb */
	m_k007232[0]->port_write().set(FUNC(chqflag_state::volume_callback0));
	m_k007232[0]->add_route(0, "speaker", 0.20, 0);
	m_k007232[0]->add_route(1, "speaker", 0.20, 1);

	K007232(config, m_k007232[1], XTAL(3'579'545)); /* verified on pcb */
	m_k007232[1]->port_write().set(FUNC(chqflag_state::volume_callback1));
	m_k007232[1]->add_route(0, "speaker", 0.20, 0);
	m_k007232[1]->add_route(0, "speaker", 0.20, 1);
	m_k007232[1]->add_route(1, "speaker", 0.20, 0);
	m_k007232[1]->add_route(1, "speaker", 0.20, 1);
}

ROM_START( chqflag )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 052001 code */
	ROM_LOAD( "717e10",     0x00000, 0x40000, CRC(72fc56f6) SHA1(433ea9a33f0230e046c731c70060f6a38db14ac7) )
	ROM_LOAD( "717h02",     0x40000, 0x10000, CRC(f5bd4e78) SHA1(7bab02152d055a6c3a322c88e7ee0b85a39d8ef2) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "717e01",     0x000000, 0x008000, CRC(966b8ba8) SHA1(ab7448cb61fa5922b1d8ae5f0d0f42d734ed4f93) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "717e04",     0x000000, 0x080000, CRC(1a50a1cc) SHA1(bc16fab84c637ed124e37b115ddc0149560b727d) )
	ROM_LOAD32_WORD( "717e05",     0x000002, 0x080000, CRC(46ccb506) SHA1(3ed1f54744fc5cdc0f48e42f250c366267a8199a) )

	ROM_REGION( 0x020000, "k051316_1", 0 )      /* zoom/rotate */
	ROM_LOAD( "717e06.n16",     0x000000, 0x020000, CRC(1ec26c7a) SHA1(05b5b522c5ebf5d0a71a7fc39ec9382008ef33c8) )

	ROM_REGION( 0x100000, "k051316_2", 0 )      /* zoom/rotate */
	ROM_LOAD( "717e07.l20",     0x000000, 0x040000, CRC(b9a565a8) SHA1(a11782f7336e5ad58a4c6ea81f2eeac35d5e7d0a) )
	ROM_LOAD( "717e08.l22",     0x040000, 0x040000, CRC(b68a212e) SHA1(b2bd121a43552c3ade528ac763a0df40c3e648e0) )
	ROM_LOAD( "717e11.n20",     0x080000, 0x040000, CRC(ebb171ec) SHA1(d65d4a6b169ce03e4427b2a397484634f938236b) )
	ROM_LOAD( "717e12.n22",     0x0c0000, 0x040000, CRC(9269335d) SHA1(af298c8cff50d707d6abc806065f8e931f975dc0) )

	ROM_REGION( 0x080000, "k007232_1", 0 )  /* 007232 data (chip 1) */
	ROM_LOAD( "717e03",     0x000000, 0x080000, CRC(ebe73c22) SHA1(fad3334e5e91bf8d11b74ffdbbfd57567e6f6f8c) )

	ROM_REGION( 0x080000, "k007232_2", 0 )  /* 007232 data (chip 2) */
	ROM_LOAD( "717e09",     0x000000, 0x080000, CRC(d74e857d) SHA1(00c851c857650d67fc4caccea4461d99be4acb3c) )
ROM_END

ROM_START( chqflagj )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 052001 code */
	ROM_LOAD( "717e10",     0x00000, 0x40000, CRC(72fc56f6) SHA1(433ea9a33f0230e046c731c70060f6a38db14ac7) )
	ROM_LOAD( "717j02.bin", 0x40000, 0x10000, CRC(05355daa) SHA1(130ddbc289c077565e44f33c63a63963e6417e19) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "717e01",     0x000000, 0x008000, CRC(966b8ba8) SHA1(ab7448cb61fa5922b1d8ae5f0d0f42d734ed4f93) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "717e04",     0x000000, 0x080000, CRC(1a50a1cc) SHA1(bc16fab84c637ed124e37b115ddc0149560b727d) )
	ROM_LOAD32_WORD( "717e05",     0x000002, 0x080000, CRC(46ccb506) SHA1(3ed1f54744fc5cdc0f48e42f250c366267a8199a) )

	ROM_REGION( 0x020000, "k051316_1", 0 )      /* zoom/rotate */
	ROM_LOAD( "717e06.n16",     0x000000, 0x020000, CRC(1ec26c7a) SHA1(05b5b522c5ebf5d0a71a7fc39ec9382008ef33c8) )

	ROM_REGION( 0x100000, "k051316_2", 0 )      /* zoom/rotate */
	ROM_LOAD( "717e07.l20",     0x000000, 0x040000, CRC(b9a565a8) SHA1(a11782f7336e5ad58a4c6ea81f2eeac35d5e7d0a) )
	ROM_LOAD( "717e08.l22",     0x040000, 0x040000, CRC(b68a212e) SHA1(b2bd121a43552c3ade528ac763a0df40c3e648e0) )
	ROM_LOAD( "717e11.n20",     0x080000, 0x040000, CRC(ebb171ec) SHA1(d65d4a6b169ce03e4427b2a397484634f938236b) )
	ROM_LOAD( "717e12.n22",     0x0c0000, 0x040000, CRC(9269335d) SHA1(af298c8cff50d707d6abc806065f8e931f975dc0) )

	ROM_REGION( 0x080000, "k007232_1", 0 )  /* 007232 data (chip 1) */
	ROM_LOAD( "717e03",     0x000000, 0x080000, CRC(ebe73c22) SHA1(fad3334e5e91bf8d11b74ffdbbfd57567e6f6f8c) )

	ROM_REGION( 0x080000, "k007232_2", 0 )  /* 007232 data (chip 2) */
	ROM_LOAD( "717e09",     0x000000, 0x080000, CRC(d74e857d) SHA1(00c851c857650d67fc4caccea4461d99be4acb3c) )
ROM_END

} // anonymous namespace


//     YEAR  NAME      PARENT   MACHINE  INPUT     CLASS          INIT        MONITOR  COMPANY   FULLNAME                  FLAGS                                                                                                       LAYOUT
GAMEL( 1988, chqflag,  0,       chqflag, chqflag,  chqflag_state, empty_init, ROT90,   "Konami", "Chequered Flag",         MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_chqflag )
GAMEL( 1988, chqflagj, chqflag, chqflag, chqflagj, chqflag_state, empty_init, ROT90,   "Konami", "Chequered Flag (Japan)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_chqflag )
