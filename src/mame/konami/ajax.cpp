// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    "AJAX/Typhoon"  (Konami GX770)

    Driver by:
        Manuel Abadia <emumanu+mame@gmail.com>

    TO DO:
    - Find the CPU core bug, that makes the 052001 to read from 0x0000

***************************************************************************/

#include "emu.h"

#include "konamipt.h"
#include "k051960.h"
#include "k052109.h"
#include "konami_helper.h"

#include "cpu/m6809/hd6309.h"
#include "cpu/m6809/konami.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "sound/ymopm.h"
#include "video/k051316.h"

#include "emupal.h"
#include "speaker.h"


// configurable logging
#define LOG_LS138     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_LS138)

#include "logmacro.h"

#define LOGLS138(...)     LOGMASKED(LOG_LS138,     __VA_ARGS__)


namespace {

class ajax_state : public driver_device
{
public:
	ajax_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_watchdog(*this, "watchdog"),
		m_k007232(*this, "k007232_%u", 1U),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_k051316(*this, "k051316"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_mainbank(*this, "mainbank"),
		m_subbank(*this, "subbank"),
		m_system(*this, "SYSTEM"),
		m_pl(*this, "P%u", 1U),
		m_dsw(*this, "DSW%u", 1U),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void ajax(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device_array<k007232_device, 2> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<k051316_device> m_k051316;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_memory_bank m_mainbank;
	required_memory_bank m_subbank;

	required_ioport m_system;
	required_ioport_array<2> m_pl;
	required_ioport_array<3> m_dsw;
	output_finder<8> m_lamps;

	// video-related
	uint8_t m_priority = 0U;

	// misc
	uint8_t m_firq_enable = 0;

	void sound_bank_w(uint8_t data);
	uint8_t ls138_f10_r(offs_t offset);
	void ls138_f10_w(offs_t offset, uint8_t data);
	void sub_bankswitch_w(uint8_t data);
	void main_bankswitch_w(uint8_t data);
	void lamps_w(uint8_t data);
	void k007232_extvol_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void volume_callback0(uint8_t data);
	void volume_callback1(uint8_t data);
	K051316_CB_MEMBER(zoom_callback);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(ajax_state::tile_callback)
{
	static const int layer_colorbase[] = { 1024 / 16, 0 / 16, 512 / 16 };

	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(ajax_state::sprite_callback)
{
	enum { sprite_colorbase = 256 / 16 };

	/* priority bits:
	   4 over zoom (0 = have priority)
	   5 over B    (0 = have priority)
	   6 over A    (1 = have priority)
	   never over F
	*/
	*priority = 0;
	if ( *color & 0x10) *priority |= GFX_PMASK_4; // Z = 4
	if (~*color & 0x40) *priority |= GFX_PMASK_2; // A = 2
	if ( *color & 0x20) *priority |= GFX_PMASK_1; // B = 1
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(ajax_state::zoom_callback)
{
	enum { zoom_colorbase = 768 / 128 };

	*code |= ((*color & 0x07) << 8);
	*color = zoom_colorbase + ((*color & 0x08) >> 3);
}


/***************************************************************************

    Display Refresh

***************************************************************************/

uint32_t ajax_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 1);
	if (m_priority)
	{
		// basic layer order is B, zoom, A, F
		m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 4);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 2);
	}
	else
	{
		// basic layer order is B, A, zoom, F
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 2);
		m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 4);
	}
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	return 0;
}


/*  main_bankswitch_w:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at H11:

    Bit Description
    --- -----------
    7   MRB3    Selects ROM N11/N12
    6   CCOUNT2 Coin Counter 2  (*)
    5   CCOUNT1 Coin Counter 1  (*)
    4   SRESET  Slave CPU Reset?
    3   PRI0    Layer Priority Selector
    2   MRB2    \
    1   MRB1     |  ROM Bank Select
    0   MRB0    /

    (*) The Coin Counters are handled by the Konami Custom 051550
*/

void ajax_state::main_bankswitch_w(uint8_t data)
{
	int bank = 0;

	// ROM select
	if (!(data & 0x80))
		bank += 4;

	// coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x20);
	machine().bookkeeping().coin_counter_w(1, data & 0x40);

	// priority
	m_priority = data & 0x08;

	// bank # (ROMS N11 and N12)
	bank += (data & 0x07);
	m_mainbank->set_entry(bank);
}

/*  lamps_w:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at B9:

    Bit Description
    --- -----------
    7   LAMP7 & LAMP8 - Game over lamps (*)
    6   LAMP3 & LAMP4 - Game over lamps (*)
    5   LAMP1 - Start lamp (*)
    4   Control panel quaking (**)
    3   Joystick vibration (**)
    2   LAMP5 & LAMP6 - Power up lamps (*)
    1   LAMP2 - Super weapon lamp (*)
    0   unused

    (*) The Lamps are handled by the M54585P
    (**)Vibration/Quaking handled by these chips:
        Chip        Location    Description
        ----        --------    -----------
        PS2401-4    B21         ???
        UPA1452H    B22         ???
        LS74        H2          Dual +ve edge trigger D-type Flip-flop with SET and RESET
        LS393       C20         Dual -ve edge trigger 4-bit Binary Ripple Counter with Resets
*/

void ajax_state::lamps_w(uint8_t data)
{
	m_lamps[1] = BIT(data, 1);  // super weapon lamp
	m_lamps[2] = BIT(data, 2);  // power up lamps
	m_lamps[5] = BIT(data, 2);  // power up lamps
	m_lamps[0] = BIT(data, 5);  // start lamp
	m_lamps[3] = BIT(data, 6);  // game over lamps
	m_lamps[6] = BIT(data, 6);  // game over lamps
	m_lamps[4] = BIT(data, 7);  // game over lamps
	m_lamps[7] = BIT(data, 7);  // game over lamps
}

/*  ajax_ls138_f10:
    The LS138 1-of-8 Decoder/Demultiplexer at F10 selects what to do:

    Address R/W Description
    ------- --- -----------
    0x0000  (r) ??? I think this read is because a CPU core bug
            (w) 0x0000  NSFIRQ  Trigger FIRQ on the M6809
                0x0020  AFR     Watchdog reset (handled by the 051550)
    0x0040  (w) SOUND           Cause interrupt on the Z80
    0x0080  (w) SOUNDDATA       Sound code number
    0x00c0  (w) MBL1            Enables the LS273 at H11 (Banking + Coin counters)
    0x0100  (r) MBL2            Enables 2P Inputs reading
    0x0140  (w) MBL3            Enables the LS273 at B9 (Lamps + Vibration)
    0x0180  (r) MIO1            Enables 1P Inputs + DIPSW #1 & #2 reading
    0x01c0  (r) MIO2            Enables DIPSW #3 reading
*/

uint8_t ajax_state::ls138_f10_r(offs_t offset)
{
	int data = 0, index;

	switch ((offset & 0x01c0) >> 6)
	{
		case 0x00:  // ???
			data = machine().rand();
			break;
		case 0x04:  // 2P inputs
			data = m_pl[1]->read();
			break;
		case 0x06:  // 1P inputs + DIPSW #1 & #2
			index = offset & 0x01;
			data = (offset & 0x02) ? m_dsw[index]->read() : index ? m_pl[0]->read() : m_system->read();
			break;
		case 0x07:  // DIPSW #3
			data = m_dsw[2]->read();
			break;

		default:
			LOGLS138("%04x: (ls138_f10) read from an unknown address %02x\n", m_maincpu->pc(), offset);
	}

	return data;
}

void ajax_state::ls138_f10_w(offs_t offset, uint8_t data)
{
	switch ((offset & 0x01c0) >> 6)
	{
		case 0x00:  // NSFIRQ + AFR
			if (offset)
				m_watchdog->watchdog_reset();
			else{
				if (m_firq_enable)  // Cause interrupt on slave CPU
					m_subcpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
			}
			break;
		case 0x01:  // Cause interrupt on audio CPU
			m_audiocpu->set_input_line(0, HOLD_LINE);
			break;
		case 0x02:  // Sound command number
			m_soundlatch->write(data);
			break;
		case 0x03:  // Bankswitch + coin counters + priority
			main_bankswitch_w(data);
			break;
		case 0x05:  // Lamps + Joystick vibration + Control panel quaking
			lamps_w(data);
			break;

		default:
			LOGLS138("%04x: (ls138_f10) write %02x to an unknown address %02x\n", m_maincpu->pc(), data, offset);
	}
}

/*  sub_bankswitch_w:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at K14:

    Bit Description
    --- -----------
    7   unused
    6   RMRD    Enable char ROM reading through the video RAM
    5   RVO     enables 051316 wraparound
    4   FIRQST  FIRQ control
    3   SRB3    \
    2   SRB2     |
    1   SRB1     |  ROM Bank Select
    0   SRB0    /
*/

void ajax_state::sub_bankswitch_w(uint8_t data)
{
	// enable char ROM reading through the video RAM
	m_k052109->set_rmrd_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	// bit 5 enables 051316 wraparound
	m_k051316->wraparound_enable(data & 0x20);

	// FIRQ control
	m_firq_enable = data & 0x10;

	// bank # (ROMS G16 and I16)
	m_subbank->set_entry(data & 0x0f);
}

void ajax_state::machine_start()
{
	uint8_t *main = memregion("maincpu")->base();
	uint8_t *sub  = memregion("sub")->base();

	m_lamps.resolve();
	m_mainbank->configure_entries(0, 4, &main[0x00000], 0x2000);
	m_mainbank->configure_entries(4, 8, &main[0x10000], 0x2000);
	m_subbank->configure_entries(0, 9, &sub[0x00000], 0x2000);

	save_item(NAME(m_priority));
	save_item(NAME(m_firq_enable));
}

void ajax_state::machine_reset()
{
	m_priority = 0;
	m_firq_enable = 0;
}

void ajax_state::main_map(address_map &map)
{
	map(0x0000, 0x01c0).rw(FUNC(ajax_state::ls138_f10_r), FUNC(ajax_state::ls138_f10_w));   // bankswitch + sound command + FIRQ command
	map(0x0800, 0x0807).rw(m_k051960, FUNC(k051960_device::k051937_r), FUNC(k051960_device::k051937_w));                    // sprite control registers
	map(0x0c00, 0x0fff).rw(m_k051960, FUNC(k051960_device::k051960_r), FUNC(k051960_device::k051960_w));                    // sprite RAM 2128SL at J7
	map(0x1000, 0x1fff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2000, 0x3fff).ram().share("main_sub_ram");                                  // shared RAM with the 6809
	map(0x4000, 0x5fff).ram();                                             // RAM 6264L at K10
	map(0x6000, 0x7fff).bankr(m_mainbank);                             // banked ROM
	map(0x8000, 0xffff).rom();         // ROM N11
}

void ajax_state::sub_map(address_map &map)
{
	map(0x0000, 0x07ff).rw(m_k051316, FUNC(k051316_device::read), FUNC(k051316_device::write));    // 051316 zoom/rotation layer
	map(0x0800, 0x080f).w(m_k051316, FUNC(k051316_device::ctrl_w));              // 051316 control registers
	map(0x1000, 0x17ff).r(m_k051316, FUNC(k051316_device::rom_r));                // 051316 (ROM test)
	map(0x1800, 0x1800).w(FUNC(ajax_state::sub_bankswitch_w));          // bankswitch control
	map(0x2000, 0x3fff).ram().share("main_sub_ram");                      // shared RAM with the 052001
	map(0x4000, 0x7fff).rw(m_k052109, FUNC(k052109_device::read), FUNC(k052109_device::write));        // video RAM + color RAM + video registers
	map(0x8000, 0x9fff).bankr(m_subbank);                            // banked ROM
	map(0xa000, 0xffff).rom().region("sub", 0x12000);     // ROM I16
}

void ajax_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();                             // ROM F6
	map(0x8000, 0x87ff).ram();                             // RAM 2128SL at D16
	map(0x9000, 0x9000).w(FUNC(ajax_state::sound_bank_w));             // 007232 bankswitch
	map(0xa000, 0xa00d).rw(m_k007232[0], FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xb000, 0xb00d).rw(m_k007232[1], FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xb80c, 0xb80c).w(FUNC(ajax_state::k007232_extvol_w));         // extra volume, goes to the 007232 w/ A11 selecting a different latch for the external port
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));       // YM2151
	map(0xe000, 0xe000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( ajax )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	// "No Coin B" = coins produce sound, but no effect on coin counter

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30000 150000" )
	PORT_DIPSETTING(    0x10, "50000 200000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")   // Listed as "unused" and forced to be off in the manual. (US) // "Normal Upright / Upright Double" (JP)
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, "Control in 3D Stages" )  PORT_DIPLOCATION("SW3:4")   // The manual make reference to "general control"
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Inverted" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")    // COINSW & START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )  // service
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	KONAMI8_B123_UNK(1)

	PORT_START("P2")
	KONAMI8_B123_UNK(2)
INPUT_PORTS_END



/*  sound_bank_w:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at B11:

    Bit Description
    --- -----------
    7   CONT1 (???) \
    6   CONT2 (???) / One or both bits are set to 1 when you kill a enemy
    5   \
    3   / 4MBANKH
    4   \
    2   / 4MBANKL
    1   \
    0   / 2MBANK
*/

void ajax_state::sound_bank_w(uint8_t data)
{
	// banks # for the 007232 (chip 1)
	int bank_A = BIT(data, 1);
	int bank_B = BIT(data, 0);
	m_k007232[0]->set_bank(bank_A, bank_B);

	// banks # for the 007232 (chip 2)
	bank_A = ((data >> 4) & 0x03);
	bank_B = ((data >> 2) & 0x03);
	m_k007232[1]->set_bank(bank_A, bank_B);
}

void ajax_state::volume_callback0(uint8_t data)
{
	m_k007232[0]->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232[0]->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void ajax_state::k007232_extvol_w(uint8_t data)
{
	// channel A volume (mono)
	m_k007232[1]->set_volume(0, (data & 0x0f) * 0x11 / 2, (data & 0x0f) * 0x11 / 2);
}

void ajax_state::volume_callback1(uint8_t data)
{
	// channel B volume/pan
	m_k007232[1]->set_volume(1, (data & 0x0f) * 0x11 / 2, (data >> 4) * 0x11 / 2);
}


void ajax_state::ajax(machine_config &config)
{
	// basic machine hardware
	KONAMI(config, m_maincpu, XTAL(24'000'000) / 2);    // 052001 12/4 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &ajax_state::main_map);

	HD6309E(config, m_subcpu, 3000000); // ?
	m_subcpu->set_addrmap(AS_PROGRAM, &ajax_state::sub_map);

	Z80(config, m_audiocpu, 3579545);  // 3.58 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &ajax_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	WATCHDOG_TIMER(config, m_watchdog);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(24'000'000) / 3, 528, 108, 412, 256, 16, 240);
//  6MHz dotclock is more realistic, however needs drawing updates. replace when ready
//  screen.set_raw(XTAL(24'000'000)/4, 396, hbend, hbstart, 256, 16, 240);
	screen.set_screen_update(FUNC(ajax_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(ajax_state::tile_callback));
	m_k052109->irq_handler().set_inputline(m_subcpu, M6809_IRQ_LINE);

	K051960(config, m_k051960, 0);
	m_k051960->set_palette("palette");
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(ajax_state::sprite_callback));
	m_k051960->irq_handler().set_inputline(m_maincpu, KONAMI_IRQ_LINE);

	K051316(config, m_k051316, 0);
	m_k051316->set_palette(m_palette);
	m_k051316->set_bpp(7);
	m_k051316->set_zoom_callback(FUNC(ajax_state::zoom_callback));

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2151(config, "ymsnd", 3579545).add_route(0, "speaker", 1.0, 0).add_route(1, "speaker", 1.0, 1);

	K007232(config, m_k007232[0], 3579545);
	m_k007232[0]->port_write().set(FUNC(ajax_state::volume_callback0));
	m_k007232[0]->add_route(0, "speaker", 0.20, 0);
	m_k007232[0]->add_route(0, "speaker", 0.20, 1);
	m_k007232[0]->add_route(1, "speaker", 0.20, 0);
	m_k007232[0]->add_route(1, "speaker", 0.20, 1);

	K007232(config, m_k007232[1], 3579545);
	m_k007232[1]->port_write().set(FUNC(ajax_state::volume_callback1));
	m_k007232[1]->add_route(0, "speaker", 0.50, 0);
	m_k007232[1]->add_route(1, "speaker", 0.50, 1);
}


/*

 This set is using 27512 ROMs on a sub-board instead of Mask ROMs
 -- info from Phil Morris

 These are normally on the main board in the form of large mask ROMs, but at one stage
 the mask ROMs were unavailable so Konami had to provide a separate ROM board with
 36 x 27C512s instead.

*/

ROM_START( ajax )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 052001 code
	ROM_LOAD( "770_m01.n11",    0x00000, 0x10000, CRC(4a64e53a) SHA1(acd249bfcb5f248c41b3e40c7c1bce1b8c645d3a) )    // last 0x8000 fixed, first 0x8000 banked
	ROM_LOAD( "770_l02.n12",    0x10000, 0x10000, CRC(ad7d592b) SHA1(c75d9696b16de231c479379dd02d33fe54021d88) )    // banked ROM

	ROM_REGION( 0x18000, "sub", 0 )
	ROM_LOAD( "770_f04.g16",    0x00000, 0x10000, CRC(e0e4ec9c) SHA1(15ae09c3ad67ec626d8178ec1417f0c57ca4eca4) )    // banked ROM
	ROM_LOAD( "770_l05.i16",    0x10000, 0x08000, CRC(ed64fbb2) SHA1(429046edaf1299afa7fb9c385b4ef0c244ec2409) )    // last 0x6000 fixed, first 0x2000 banked

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "770_h03.f16",    0x00000, 0x08000, CRC(2ffd2afc) SHA1(ca2ef684f87bcf9b70b3ec66ec80685edaf04b9b) )

	ROM_REGION( 0x080000, "k052109", 0 )    // tiles
	ROM_LOAD32_BYTE( "770c13-a.f3",     0x000000, 0x010000, CRC(4ef6fff2) SHA1(0a2953f6907738b795d96184329431539386a463) )
	ROM_LOAD32_BYTE( "770c13-c.f4",     0x000001, 0x010000, CRC(97ffbab6) SHA1(97d9a39600eed918e12908a9abed0d4161c20ef6) )
	ROM_LOAD32_BYTE( "770c12-a.f5",     0x000002, 0x010000, CRC(6c0ade68) SHA1(35e4548a37e19210c767ef2ed4c514dbde6806c2) )
	ROM_LOAD32_BYTE( "770c12-c.f6",     0x000003, 0x010000, CRC(61fc39cc) SHA1(34d0342ec0878590c289a66b39bde121cfadf00f) )
	ROM_LOAD32_BYTE( "770c13-b.e3",     0x040000, 0x010000, CRC(86fdd706) SHA1(334c2720fc35aa556c6c5850d32f9bc9a6800fba) )
	ROM_LOAD32_BYTE( "770c13-d.e4",     0x040001, 0x010000, CRC(7d7acb2d) SHA1(3797743edf99201de928246e22e65ad17afe62f8) )
	ROM_LOAD32_BYTE( "770c12-b.e5",     0x040002, 0x010000, CRC(5f221cc6) SHA1(9a7a9c7853a3b582c4034b773cef08aee5391d6e) )
	ROM_LOAD32_BYTE( "770c12-d.e6",     0x040003, 0x010000, CRC(f1edb2f4) SHA1(3e66cc711e25cbf6e6a747d43a9efec0710d5b7a) )

	ROM_REGION( 0x100000, "k051960", 0 )    // sprites
	ROM_LOAD32_BYTE( "770c09-a.f8",     0x000000, 0x010000, CRC(76690fb8) SHA1(afe267a37b65d63d3765dc3b88d8a8262446f786) )
	ROM_LOAD32_BYTE( "770c09-e.f9",     0x000001, 0x010000, CRC(17b482c9) SHA1(3535197956f5bf5b564fec1ddbb3e3ea3bf1f7bd) )
	ROM_LOAD32_BYTE( "770c08-a.f10",    0x000002, 0x010000, CRC(efd29a56) SHA1(2a9f138d1242a35162a3f092b0343dff899e3b83) )
	ROM_LOAD32_BYTE( "770c08-e.f11",    0x000003, 0x010000, CRC(6d43afde) SHA1(03d16125e7d082df08cd5e52a6694a1ddb765e4f) )
	ROM_LOAD32_BYTE( "770c09-b.e8",     0x040000, 0x010000, CRC(cd1709d1) SHA1(5a835639eb2d75adcfd0103b0800dd74b2bf9503) )
	ROM_LOAD32_BYTE( "770c09-f.e9",     0x040001, 0x010000, CRC(cba4b47e) SHA1(6ecb6283de4aa5ef8441db62b19200397f7734b3) )
	ROM_LOAD32_BYTE( "770c08-b.e10",    0x040002, 0x010000, CRC(f3374014) SHA1(613c91e02fbf577668ea558c1893b845962368dd) )
	ROM_LOAD32_BYTE( "770c08-f.e11",    0x040003, 0x010000, CRC(f5ba59aa) SHA1(b65ea2ec20c2e9fa2e0dfe4c38d3d4f0b7160a97) )
	ROM_LOAD32_BYTE( "770c09-c.d8",     0x080000, 0x010000, CRC(bfd080b8) SHA1(83e186e08f442167e66575305930fa93f838faa6) )
	ROM_LOAD32_BYTE( "770c09-g.d9",     0x080001, 0x010000, CRC(77d58ea0) SHA1(8647c6920032e010b71ba4bc966ef6e1fd0a58a8) )
	ROM_LOAD32_BYTE( "770c08-c.d10",    0x080002, 0x010000, CRC(28e7088f) SHA1(45c53a58bc6d2e70d5d20d5e6d58ec3e5bea3eeb) )
	ROM_LOAD32_BYTE( "770c08-g.d11",    0x080003, 0x010000, CRC(17da8f6d) SHA1(ba1d33d44cd50ff5d5a15b23d1a6153bc7b09579) )
	ROM_LOAD32_BYTE( "770c09-d.c8",     0x0c0000, 0x010000, CRC(6f955600) SHA1(6f85adb633a670c8540b1e86d4bb6640829e74da) )
	ROM_LOAD32_BYTE( "770c09-h.c9",     0x0c0001, 0x010000, CRC(494a9090) SHA1(decd4442c206d1cd8f7741f2499aa3264b247d06) )
	ROM_LOAD32_BYTE( "770c08-d.c10",    0x0c0002, 0x010000, CRC(91591777) SHA1(53f416a51f7075f070168bced7b6f925f54c7b84) )
	ROM_LOAD32_BYTE( "770c08-h.c11",    0x0c0003, 0x010000, CRC(d97d4b15) SHA1(e3d7d7adeec8c8c808acb9f84641fd3a6bf249be) )

	ROM_REGION( 0x080000, "k051316", 0 )    // zoom/rotate
	ROM_LOAD( "770c06.f4",     0x000000, 0x040000, CRC(d0c592ee) SHA1(c1be73dd259f2779d715659b177e47513776a0d4) )
	ROM_LOAD( "770c07.h4",     0x040000, 0x040000, CRC(0b399fb1) SHA1(fbe26f9aa9a655d08bebcdd79719d35134ca4dd5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "63s241.j11", 0x0000, 0x0200, CRC(9bdd719f) SHA1(de98e562080a97714047a8ad17abc6662c188897) )  // priority encoder (not used)

	ROM_REGION( 0x040000, "k007232_1", 0 )
	ROM_LOAD( "770c10-a.a7",        0x000000, 0x010000, CRC(e45ec094) SHA1(540c56e1d778e6082db23aa3da64f6179b1f3635) )
	ROM_LOAD( "770c10-b.a6",        0x010000, 0x010000, CRC(349db7d3) SHA1(210da067038abeb021a77b3bf2664c9a49b3410a) )
	ROM_LOAD( "770c10-c.a5",        0x020000, 0x010000, CRC(71cb1f05) SHA1(57399806746b659f52114fb7bd4e11a7992a2c5d) )
	ROM_LOAD( "770c10-d.a4",        0x030000, 0x010000, CRC(e8ab1844) SHA1(dc22c4d11d6396a051398ba9ec6380aa3f856e71) )

	ROM_REGION( 0x080000, "k007232_2", 0 )
	ROM_LOAD( "770c11-a.c6",        0x000000, 0x010000, CRC(8cccd9e0) SHA1(73e50a896ed212462046b7bfa04aad5e266425ca) )
	ROM_LOAD( "770c11-b.c5",        0x010000, 0x010000, CRC(0af2fedd) SHA1(038189210a73f668a0d913ff2dfc4ffa2e6bd5f4) )
	ROM_LOAD( "770c11-c.c4",        0x020000, 0x010000, CRC(7471f24a) SHA1(04d7a69ddc01017a773485fa891711d94c8ad47c) )
	ROM_LOAD( "770c11-d.c3",        0x030000, 0x010000, CRC(a58be323) SHA1(0401ede130cf9a529469bfb3dbcc8aee68e53243) )
	ROM_LOAD( "770c11-e.b7",        0x040000, 0x010000, CRC(dd553541) SHA1(96f36cb7b696f465005c7e7f1e4373b98a337864) )
	ROM_LOAD( "770c11-f.b6",        0x050000, 0x010000, CRC(3f78bd0f) SHA1(1d445c2b6460d6aac6f2acf0d5a5d73c31ba52e0) )
	ROM_LOAD( "770c11-g.b5",        0x060000, 0x010000, CRC(078c51b2) SHA1(6ad7ae8cda62023a286f5b4ac393ea0d02d20aeb) )
	ROM_LOAD( "770c11-h.b4",        0x070000, 0x010000, CRC(7300c2e1) SHA1(f9d23074701fb2127aed45d7cff91cc1cf8ce717) )
ROM_END

ROM_START( typhoon )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 052001 code
	ROM_LOAD( "770_k01.n11",    0x00000, 0x10000, CRC(5ba74a22) SHA1(897d3309f2efb3bfa56e86581ee4a492e656788c) )    // last 0x8000 fixed, first 0x8000 banked
	ROM_LOAD( "770_k02.n12",    0x10000, 0x10000, CRC(3bcf782a) SHA1(4b6127bced0b2519f8ad30587f32588a16368071) )    // banked ROM

	ROM_REGION( 0x18000, "sub", 0 )
	ROM_LOAD( "770_f04.g16",    0x00000, 0x10000, CRC(e0e4ec9c) SHA1(15ae09c3ad67ec626d8178ec1417f0c57ca4eca4) )    // banked ROM
	ROM_LOAD( "770_k05.i16",    0x10000, 0x08000, CRC(0f1bebbb) SHA1(012a8867ee0febaaadd7bcbc91e462bda5d3a411) )    // last 0x6000 fixed, first 0x2000 banked

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "770_h03.f16",    0x00000, 0x08000, CRC(2ffd2afc) SHA1(ca2ef684f87bcf9b70b3ec66ec80685edaf04b9b) )

	ROM_REGION( 0x080000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "770c13.n22",     0x000000, 0x040000, CRC(b859ca4e) SHA1(f58678d503683f78cca0d5ed2d79f6f68ab3495a) )
	ROM_LOAD32_WORD( "770c12.k22",     0x000002, 0x040000, CRC(50d14b72) SHA1(e3ff4a5aeefa6c10b5f7fec18297948b7c5acfdf) )

	ROM_REGION( 0x100000, "k051960", 0 )    // sprites
	ROM_LOAD32_WORD( "770c09.n4",     0x000000, 0x080000, CRC(1ab4a7ff) SHA1(fa007b41027f95d29d2a9f931a2fe235844db637) )
	ROM_LOAD32_WORD( "770c08.k4",     0x000002, 0x080000, CRC(a8e80586) SHA1(0401f59baa691905287cef94427f39e0c3f0adc6) )

	ROM_REGION( 0x080000, "k051316", 0 )    // zoom/rotate
	ROM_LOAD( "770c06.f4",     0x000000, 0x040000, CRC(d0c592ee) SHA1(c1be73dd259f2779d715659b177e47513776a0d4) )
	ROM_LOAD( "770c07.h4",     0x040000, 0x040000, CRC(0b399fb1) SHA1(fbe26f9aa9a655d08bebcdd79719d35134ca4dd5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "63s241.j11", 0x0000, 0x0200, CRC(9bdd719f) SHA1(de98e562080a97714047a8ad17abc6662c188897) )  // priority encoder (not used)

	ROM_REGION( 0x040000, "k007232_1", 0 )
	ROM_LOAD( "770c10",     0x000000, 0x040000, CRC(7fac825f) SHA1(581522d7a02dad16d2803ff344b4db133f767e6b) )

	ROM_REGION( 0x080000, "k007232_2", 0 )
	ROM_LOAD( "770c11",     0x000000, 0x080000, CRC(299a615a) SHA1(29cdcc21998c72f4cf311792b904b79bde236bab) )
ROM_END

ROM_START( ajaxj )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 052001 code
	ROM_LOAD( "770_l01.n11",    0x00000, 0x10000, CRC(7cea5274) SHA1(8e3b2b11a8189e3a1703b3b4b453fbb386f5537f) )    // last 0x8000 fixed, first 0x8000 banked
	ROM_LOAD( "770_l02.n12",    0x10000, 0x10000, CRC(ad7d592b) SHA1(c75d9696b16de231c479379dd02d33fe54021d88) )    // banked ROM

	ROM_REGION( 0x18000, "sub", 0 )
	ROM_LOAD( "770_f04.g16",    0x00000, 0x10000, CRC(e0e4ec9c) SHA1(15ae09c3ad67ec626d8178ec1417f0c57ca4eca4) )   // banked ROM
	ROM_LOAD( "770_l05.i16",    0x10000, 0x08000, CRC(ed64fbb2) SHA1(429046edaf1299afa7fb9c385b4ef0c244ec2409) )    // last 0x6000 fixed, first 0x2000 banked

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "770_f03.f16",    0x00000, 0x08000, CRC(3fe914fd) SHA1(c691920402bd859e2bf765084704a8bfad302cfa) )

	ROM_REGION( 0x080000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "770c13.n22",     0x000000, 0x040000, CRC(b859ca4e) SHA1(f58678d503683f78cca0d5ed2d79f6f68ab3495a) )
	ROM_LOAD32_WORD( "770c12.k22",     0x000002, 0x040000, CRC(50d14b72) SHA1(e3ff4a5aeefa6c10b5f7fec18297948b7c5acfdf) )

	ROM_REGION( 0x100000, "k051960", 0 )    // sprites
	ROM_LOAD32_WORD( "770c09.n4",     0x000000, 0x080000, CRC(1ab4a7ff) SHA1(fa007b41027f95d29d2a9f931a2fe235844db637) )
	ROM_LOAD32_WORD( "770c08.k4",     0x000002, 0x080000, CRC(a8e80586) SHA1(0401f59baa691905287cef94427f39e0c3f0adc6) )

	ROM_REGION( 0x080000, "k051316", 0 )    // zoom/rotate
	ROM_LOAD( "770c06.f4",     0x000000, 0x040000, CRC(d0c592ee) SHA1(c1be73dd259f2779d715659b177e47513776a0d4) )
	ROM_LOAD( "770c07.h4",     0x040000, 0x040000, CRC(0b399fb1) SHA1(fbe26f9aa9a655d08bebcdd79719d35134ca4dd5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "63s241.j11", 0x0000, 0x0200, CRC(9bdd719f) SHA1(de98e562080a97714047a8ad17abc6662c188897) )  // priority encoder (not used)

	ROM_REGION( 0x040000, "k007232_1", 0 )
	ROM_LOAD( "770c10",     0x000000, 0x040000, CRC(7fac825f) SHA1(581522d7a02dad16d2803ff344b4db133f767e6b) )

	ROM_REGION( 0x080000, "k007232_2", 0 )
	ROM_LOAD( "770c11",     0x000000, 0x080000, CRC(299a615a) SHA1(29cdcc21998c72f4cf311792b904b79bde236bab) )
ROM_END

} // anonymous namespace


GAME( 1987, ajax,    0,    ajax, ajax, ajax_state, empty_init, ROT90, "Konami", "Ajax", MACHINE_SUPPORTS_SAVE )
GAME( 1987, typhoon, ajax, ajax, ajax, ajax_state, empty_init, ROT90, "Konami", "Typhoon", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ajaxj,   ajax, ajax, ajax, ajax_state, empty_init, ROT90, "Konami", "Ajax (Japan)", MACHINE_SUPPORTS_SAVE )
