// license:GPL-2.0+
// copyright-holders: Juergen Buchmueller

/***************************************************************************

    Meadows S2650 driver

    driver by J. Buchmueller, June '98

    Games supported:
        * Dead Eye
        * 3-D Bowling
        * Gypsy Juggler
        * Inferno

    Known issues:
        * none at this time

****************************************************************************

    ***********************************************
    memory map CPU #0 (preliminary)
    ***********************************************

    0000..0bff  ROM part one

    0c00..0c03  H/W input ports
    -----------------------------------------------
            0 R control buttons
                D0      button 1
                D1      start 2 player game

            1 R analog control
                D0-D7   center is 0x7f

            2 R horizontal sync divider chain
                D7 9.765kHz ... D0 2.5MHz

            3 R dip switch settings
                D0-D2   select 2 to 9 coins
                D3-D4   Coins per play D3 D4
                        1 coin  1 play  0  0
                        2 coins 1 play  1  0
                        1 coin  2 plays 0  1
                        free play       1  1
                D5      Attract music 0:off, 1:on
                D6-D7   Extended play  D6 D7
                        none            0  0
                        5000 pts        1  0
                        15000 pts       0  1
                        35000 pts       1  1

    0d00-0d0f   H/W sprites
    -----------------------------------------------
            0 W D0-D7    sprite 0 horz
            1 W D0-D7    sprite 1 horz
            2 W D0-D7    sprite 2 horz
            3 W D0-D7    sprite 3 horz
            4 W D0-D7    sprite 0 vert
            5 W D0-D7    sprite 2 vert
            6 W D0-D7    sprite 3 vert
            7 W D0-D7    sprite 4 vert
            8 W D0-D7    sprite 0 select
                D0-D3    sprite #
                D4       prom (not sure)
                D5       flip x
            9 W          sprite 1 select
                D0-D3    sprite #
                D4       prom (not sure)
                D5       flip x
            a W          sprite 2 select
                D0-D3    sprite #
                D4       prom (not sure)
                D5       flip x
            b W          sprite 3 select
                D0-D3    sprite #
                D4       prom (not sure)
                D5       flip x

    0e00-0eff   RAM

    1000-1bff   ROM     part two

    1c00-1fff   RAM     video buffer

    ***********************************************
    memory map CPU #1 (preliminary)
    ***********************************************

    0000..0bff  ROM part one

    0c00..0c03  H/W input ports
    -----------------------------------------------
            0 R audio command from CPU #0
                D0-D7   8 different sounds ???

            1 R ???
            2 R ???
            3 R ???

            0 W D0-D7   DAC
            1 W D0-D3   preset for counter, clk is 5 MHz / 256
                D4-D7   volume bits 0 .. 3 (bit 4 is CPU #1 flag output)
            2 W D0-D7   preset for counter, clk is 5 MHz / 32
            3 W D0      divide c02 counter by 0: 2, 1: 4
                D1      audio enable for c02 tone generator
                D2      audio enable for DAC
                D3      audio enable for c01 tone generator

    0e00-0eff   RAM


    ********************************************
    Inferno memory map (very incomplete)
    ********************************************
    0000..0bff  ROM part one
    1c00..1eff  video buffer
    1f00..1f03  hardware?

***************************************************************************/

#include "emu.h"

#include "cpu/s2650/s2650.h"
#include "sound/dac.h"
#include "sound/samples.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "deadeye.lh"
#include "gypsyjug.lh"
#include "minferno.lh"


// configurable logging
#define LOG_AUDIO (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_AUDIO)

#include "logmacro.h"

#define LOGAUDIO(...) LOGMASKED(LOG_AUDIO, __VA_ARGS__)


namespace {

class meadows_state : public driver_device
{
public:
	meadows_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dac(*this, "dac"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram")
	{
	}

	void bowl3d(machine_config &config);
	void meadows(machine_config &config);
	void minferno(machine_config &config);

	void init_minferno();
	void init_gypsyjug();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<s2650_device> m_maincpu;
	optional_device<s2650_device> m_audiocpu;
	optional_device<dac_8bit_r2r_device> m_dac;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	uint8_t m_channel = 0; // TODO: always 0?
	uint32_t m_freq1 = 0;
	uint32_t m_freq2 = 0;
	uint8_t m_latched_0c01 = 0;
	uint8_t m_latched_0c02 = 0;
	uint8_t m_latched_0c03 = 0;
	uint8_t m_main_sense_state = 0;
	uint8_t m_audio_sense_state = 0;
	uint8_t m_0c00 = 0;
	uint8_t m_0c01 = 0;
	uint8_t m_0c02 = 0;
	uint8_t m_0c03 = 0;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t hsync_chain_r();
	uint8_t vsync_chain_hi_r();
	uint8_t vsync_chain_lo_r();
	void audio_w(offs_t offset, uint8_t data);
	void audio_hardware_w(offs_t offset, uint8_t data);
	uint8_t audio_hardware_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	void spriteram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void meadows_vblank_irq(int state);
	void minferno_vblank_irq(int state);
	INTERRUPT_GEN_MEMBER(audio_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &clip);
	void sh_update();
	SAMPLES_START_CB_MEMBER(sh_start);
	void audio_map(address_map &map) ATTR_COLD;
	void bowl3d_main_map(address_map &map) ATTR_COLD;
	void meadows_main_map(address_map &map) ATTR_COLD;
	void minferno_data_map(address_map &map) ATTR_COLD;
	void minferno_main_map(address_map &map) ATTR_COLD;
};


static constexpr uint32_t BASE_CTR1 = (5'000'000 / 256);
static constexpr uint32_t BASE_CTR2 = (5'000'000 / 32);

static constexpr uint8_t DIV2OR4_CTR2 = 0x01;
static constexpr uint8_t ENABLE_CTR2 = 0x02;
static constexpr uint8_t ENABLE_DAC = 0x04;
static constexpr uint8_t ENABLE_CTR1 = 0x08;

static const int16_t waveform[2] = { -120*256, 120*256 };

/************************************/
/* Sound handler start              */
/************************************/
SAMPLES_START_CB_MEMBER(meadows_state::sh_start)
{
	m_0c00 = m_0c01 = m_0c02 = m_0c03 = 0;
	m_channel = 0;
	m_freq1 = m_freq2 = 1000;
	m_latched_0c01 = m_latched_0c02 = m_latched_0c03 = 0;

	m_samples->set_volume(0, 0);
	m_samples->start_raw(0, waveform, std::size(waveform), m_freq1, true);
	m_samples->set_volume(1, 0);
	m_samples->start_raw(1, waveform, std::size(waveform), m_freq2, true);
}

/************************************/
/* Sound handler update             */
/************************************/
void meadows_state::sh_update()
{
	if (m_latched_0c01 != m_0c01 || m_latched_0c03 != m_0c03)
	{
		/* amplitude is a combination of the upper 4 bits of 0c01
		   and bit 4 merged from S2650's flag output */
		int amp = ((m_0c03 & ENABLE_CTR1) == 0) ? 0 : (m_0c01 & 0xf0) >> 1;
		if( m_maincpu->state_int(S2650_FO) )
			amp += 0x80;
		/* calculate frequency for counter #1
		   bit 0..3 of 0c01 are ctr preset */
		int const preset = (m_0c01 & 15) ^ 15;
		if (preset)
			m_freq1 = BASE_CTR1 / (preset + 1);
		else amp = 0;
		LOGAUDIO("meadows ctr1 channel #%d preset:%3d freq:%5d amp:%d\n", m_channel, preset, m_freq1, amp);
		m_samples->set_frequency(0, m_freq1 * sizeof(waveform) / 2);
		m_samples->set_volume(0, amp / 255.0);
	}

	if (m_latched_0c02 != m_0c02 || m_latched_0c03 != m_0c03)
	{
		/* calculate frequency for counter #2
		   0c02 is ctr preset, 0c03 bit 0 enables division by 2 */
		int amp = ((m_0c03 & ENABLE_CTR2) != 0) ? 0xa0 : 0;
		int const preset = m_0c02 ^ 0xff;
		if (preset)
		{
			m_freq2 = BASE_CTR2 / (preset + 1) / 2;
			if ((m_0c03 & DIV2OR4_CTR2) == 0)
				m_freq2 >>= 1;
		}
		else amp = 0;
		LOGAUDIO("meadows ctr2 channel #%d preset:%3d freq:%5d amp:%d\n", m_channel + 1, preset, m_freq2, amp);
		m_samples->set_frequency(1, m_freq2 * sizeof(waveform));
		m_samples->set_volume(1, amp / 255.0);
	}

	if (((m_latched_0c03 ^ m_0c03) & ENABLE_DAC) != 0)
	{
		m_dac->set_output_gain(ALL_OUTPUTS, (m_0c03 & ENABLE_DAC) != 0 ? 1.0 : 0.0);
	}

	m_latched_0c01 = m_0c01;
	m_latched_0c02 = m_0c02;
	m_latched_0c03 = m_0c03;
}


// some constants to make life easier
static constexpr int8_t SPR_ADJUST_X = -18;
static constexpr int8_t SPR_ADJUST_Y = -14;


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(meadows_state::get_tile_info)
{
	tileinfo.set(0, m_videoram[tile_index] & 0x7f, 0, 0);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void meadows_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(meadows_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 30);
}



/*************************************
 *
 *  Video RAM write
 *
 *************************************/

void meadows_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}



/*************************************
 *
 *  Sprite RAM write
 *
 *************************************/

void meadows_state::spriteram_w(offs_t offset, uint8_t data)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	m_spriteram[offset] = data;
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void meadows_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &clip)
{
	for (int i = 0; i < 4; i++)
	{
		int const x = m_spriteram[i + 0] + SPR_ADJUST_X;
		int const y = m_spriteram[i + 4] + SPR_ADJUST_Y;
		int const code = m_spriteram[i + 8] & 0x0f;       // bit #0 .. #3 select sprite
//      int const bank = (m_spriteram[i + 8] >> 4) & 1;      bit #4 selects PROM ???
		int const bank = i;                               // that fixes it for now :-/
		int const flip = m_spriteram[i + 8] >> 5;         // bit #5 flip vertical flag

		m_gfxdecode->gfx(bank + 1)->transpen(bitmap, clip, code, 0, flip, 0, x, y, 0);
	}
}



/*************************************
 *
 *  Primary video update
 *
 *************************************/

uint32_t meadows_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw the background
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw the sprites
	if (m_gfxdecode->gfx(1))
		draw_sprites(bitmap, cliprect);
	return 0;
}

void meadows_state::machine_start()
{
	m_main_sense_state = 0;

	// save_item(NAME(m_channel)); // commented out as it's always 0 right now
	save_item(NAME(m_freq1));
	save_item(NAME(m_freq2));
	save_item(NAME(m_latched_0c01));
	save_item(NAME(m_latched_0c02));
	save_item(NAME(m_latched_0c03));
	save_item(NAME(m_main_sense_state));
	save_item(NAME(m_audio_sense_state));
	save_item(NAME(m_0c00));
	save_item(NAME(m_0c01));
	save_item(NAME(m_0c02));
	save_item(NAME(m_0c03));

}


/*************************************
 *
 *  Special input ports
 *
 *************************************/

uint8_t meadows_state::hsync_chain_r()
{
	uint8_t const val = m_screen->hpos();
	return bitswap<8>(val, 0, 1, 2, 3, 4, 5, 6, 7);
}


uint8_t meadows_state::vsync_chain_hi_r()
{
	uint8_t const val = m_screen->vpos();
	return ((val >> 1) & 0x08) | ((val >> 3) & 0x04) | ((val >> 5) & 0x02) | (val >> 7);
}


uint8_t meadows_state::vsync_chain_lo_r()
{
	uint8_t const val = m_screen->vpos();
	return val & 0x0f;
}



/*************************************
 *
 *  Audio control writes
 *
 *************************************/

void meadows_state::audio_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			if (m_0c00 == data)
				break;
			LOGAUDIO("audio_w %d $%02x\n", offset, data);
			m_0c00 = data;
			break;

		case 1:
			LOGAUDIO("audio_w %d $%02x\n", offset, data);
			break;

		case 2:
			LOGAUDIO("audio_w %d $%02x\n", offset, data);
			break;

		case 3:
//          S2650_Clear_Pending_Interrupts();
			break;
	}
}



/*************************************
 *
 *  Coin handling
 *
 *************************************/

INPUT_CHANGED_MEMBER(meadows_state::coin_inserted)
{
	m_maincpu->set_input_line(0, (newval ? ASSERT_LINE : CLEAR_LINE));
}



/*************************************
 *
 *  Main CPU interrupt
 *
 *************************************/

void meadows_state::meadows_vblank_irq(int state)
{
	if (state)
	{
		// fake something toggling the sense input line of the S2650
		m_main_sense_state ^= 1;
		m_maincpu->set_input_line(S2650_SENSE_LINE, m_main_sense_state ? ASSERT_LINE : CLEAR_LINE);
	}
}



/*************************************
 *
 *  Main CPU interrupt (Inferno)
 *
 *************************************/

void meadows_state::minferno_vblank_irq(int state)
{
	if (state)
	{
		m_main_sense_state++;
		m_maincpu->set_input_line(S2650_SENSE_LINE, (m_main_sense_state & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	}
}



/*************************************
 *
 *  Audio hardware output control
 *
 *************************************/

void meadows_state::audio_hardware_w(offs_t offset, uint8_t data)
{
	switch (offset & 3)
	{
		case 0: // DAC
			m_dac->write(data ^ 0xff);
			break;

		case 1: // counter clk 5 MHz / 256
			if (data == m_0c01)
				break;
			LOGAUDIO("audio_w ctr1 preset $%x amp %d\n", data & 15, data >> 4);
			m_0c01 = data;
			sh_update();
			break;

		case 2: // counter clk 5 MHz / 32 (/ 2 or / 4)
			if (data == m_0c02)
				break;
			LOGAUDIO("audio_w ctr2 preset $%02x\n", data);
			m_0c02 = data;
			sh_update();
			break;

		case 3: // audio enable
			if (data == m_0c03)
				break;
			LOGAUDIO("audio_w enable ctr2/2:%d ctr2:%d dac:%d ctr1:%d\n", data & 1, (data >> 1) & 1, (data >> 2) & 1, (data >> 3) & 1);
			m_0c03 = data;
			sh_update();
			break;
	}
}



/*************************************
 *
 *  Audio hardware read
 *
 *************************************/

uint8_t meadows_state::audio_hardware_r(offs_t offset)
{
	int data = 0;

	switch (offset)
	{
		case 0:
			data = m_0c00;
			break;

		case 1: break;
		case 2: break;
		case 3: break;
	}
	return data;
}



/*************************************
 *
 *  Audio hardware interrupts
 *
 *************************************/

INTERRUPT_GEN_MEMBER(meadows_state::audio_interrupt)
{
	// fake something toggling the sense input line of the S2650
	m_audio_sense_state ^= 1;
	m_audiocpu->set_input_line(S2650_SENSE_LINE, m_audio_sense_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void meadows_state::meadows_main_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0c00).portr("INPUTS");
	map(0x0c01, 0x0c01).portr("STICK");
	map(0x0c02, 0x0c02).r(FUNC(meadows_state::hsync_chain_r));
	map(0x0c03, 0x0c03).portr("DSW");
	map(0x0c00, 0x0c03).w(FUNC(meadows_state::audio_w));
	map(0x0d00, 0x0d0f).w(FUNC(meadows_state::spriteram_w)).share(m_spriteram);
	map(0x0e00, 0x0eff).ram();
	map(0x1000, 0x1bff).rom();
	map(0x1c00, 0x1fff).ram().w(FUNC(meadows_state::videoram_w)).share(m_videoram);
}

void meadows_state::bowl3d_main_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0c00).portr("INPUTS1");
	map(0x0c01, 0x0c01).portr("INPUTS2");
	map(0x0c02, 0x0c02).r(FUNC(meadows_state::hsync_chain_r));
	map(0x0c03, 0x0c03).portr("DSW");
	map(0x0c00, 0x0c03).w(FUNC(meadows_state::audio_w));
	map(0x0d00, 0x0d0f).w(FUNC(meadows_state::spriteram_w)).share(m_spriteram);
	map(0x0e00, 0x0eff).ram();
	map(0x1000, 0x1bff).rom();
	map(0x1c00, 0x1fff).ram().w(FUNC(meadows_state::videoram_w)).share(m_videoram);
}

void meadows_state::minferno_main_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x1c00, 0x1eff).ram().w(FUNC(meadows_state::videoram_w)).share(m_videoram);
	map(0x1f00, 0x1f00).portr("JOY1");
	map(0x1f01, 0x1f01).portr("JOY2");
	map(0x1f02, 0x1f02).portr("BUTTONS");
	map(0x1f03, 0x1f03).portr("DSW1");
	map(0x1f00, 0x1f03).w(FUNC(meadows_state::audio_w));
	map(0x1f04, 0x1f04).r(FUNC(meadows_state::vsync_chain_hi_r));
	map(0x1f05, 0x1f05).r(FUNC(meadows_state::vsync_chain_lo_r));
}


void meadows_state::minferno_data_map(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).portr("DSW2");
}



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

void meadows_state::audio_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0c03).rw(FUNC(meadows_state::audio_hardware_r), FUNC(meadows_state::audio_hardware_w));
	map(0x0e00, 0x0eff).ram();
}




/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( meadows )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STICK")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW1:3,2,1")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x05, "7" )
	PORT_DIPSETTING(    0x06, "8" )
	PORT_DIPSETTING(    0x07, "9" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:5,4")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("DSW1:8,7")
	PORT_DIPSETTING(    0x40, "5000")
	PORT_DIPSETTING(    0x80, "15000")
	PORT_DIPSETTING(    0xc0, "35000")
	PORT_DIPSETTING(    0x00, DEF_STR( None ))

	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, meadows_state,coin_inserted, 0)
	PORT_BIT( 0x8e, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( bowl3d )
	PORT_START("INPUTS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(1) PORT_NAME("P1 Hook right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(1) PORT_NAME("P1 Hook left")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(1) PORT_NAME("P1 Bowl slow")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1) PORT_NAME("P1 Bowl fast")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1         )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(2) PORT_NAME("P2 Hook right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(2) PORT_NAME("P2 Hook left")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2) PORT_NAME("P2 Bowl slow")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2) PORT_NAME("P2 Bowl fast")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2         )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, "CPU bowls ball after" )
	PORT_DIPSETTING(    0x00, "120 seconds" )
	PORT_DIPSETTING(    0x01, "3 seconds" )
	PORT_DIPSETTING(    0x02, "4 seconds" )
	PORT_DIPSETTING(    0x03, "5 seconds" )
	PORT_DIPNAME( 0x04, 0x00, "Coin Option" )
	PORT_DIPSETTING(    0x00, "2 Players / 1 Coin" )
	PORT_DIPSETTING(    0x04, "1/2 Player(s) / 1/2 Coin(s)" )
	PORT_DIPNAME( 0x08, 0x00, "Beer Frame" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, "Extended play" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, "250" )
	PORT_DIPSETTING(    0x20, "275" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, meadows_state,coin_inserted, 0)
	PORT_BIT( 0x8e, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( minferno )
	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Coin Option" ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, "1 Game/Coin" )
	PORT_DIPSETTING(    0x01, "1 Player/Coin" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("DSW1:2,1")
	PORT_DIPSETTING(    0x00, "60s" )
	PORT_DIPSETTING(    0x01, "90s" )
	PORT_DIPSETTING(    0x02, "120s" )
	PORT_DIPSETTING(    0x03, "180s" )
	PORT_DIPNAME( 0x0c, 0x04, "Extended Play Score" ) PORT_DIPLOCATION("DSW1:6,5")
	PORT_DIPSETTING(    0x00, "3000/6000" )
	PORT_DIPSETTING(    0x04, "4000/7000" )
	PORT_DIPSETTING(    0x08, "5000/8000" )
	PORT_DIPSETTING(    0x0c, "6000/9000" )
	PORT_DIPNAME( 0x30, 0x10, "Extended Play Time" ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "20s" )
	PORT_DIPSETTING(    0x20, "40s" )
	PORT_DIPSETTING(    0x30, "60s" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,                            // 8*8 characters
	128,                            // 128 characters ?
	1,                              // 1 bit per pixel
	{ 0 },                          // no bitplanes
	{ 0, 1, 2, 3, 4, 5, 6, 7 },     // pretty straight layout
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                             // every char takes 8 bytes
};


static const gfx_layout spritelayout =
{
	16,16,                          // 16*16 sprites ?
	32,                             // 32 sprites
	1,                              // 1 bits per pixel
	{ 0 },                          // 1 bitplane
	{ 0, 1, 2, 3, 4, 5, 6, 7,
	8, 9,10,11,12,13,14,15 },     // pretty straight layout
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
	8*16, 9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	16*2*8                          // every sprite takes 32 bytes
};


static GFXDECODE_START( gfx_meadows )
	GFXDECODE_ENTRY( "chars",    0, charlayout,   0, 1 )     // character generator
	GFXDECODE_ENTRY( "sprites1", 0, spritelayout, 0, 1 )     // sprite PROM 1
	GFXDECODE_ENTRY( "sprites2", 0, spritelayout, 0, 1 )     // sprite PROM 2
	GFXDECODE_ENTRY( "sprites3", 0, spritelayout, 0, 1 )     // sprite PROM 3 (unused)
	GFXDECODE_ENTRY( "sprites4", 0, spritelayout, 0, 1 )     // sprite PROM 4 (unused)
GFXDECODE_END


static GFXDECODE_START( gfx_minferno )
	GFXDECODE_ENTRY( "chars", 0, charlayout,   0, 4 )
GFXDECODE_END



/*************************************
 *
 *  Audio interfaces
 *
 *************************************/

static const char *const bowl3d_sample_names[] =
{
	"*bowl3d",
	"roll",     // "roll"
	"rollback", // "roll back"
	"sweep",    // "sweep"
	"footstep", // "foot sweep"
	"crash",    // "crash"
	"cheering", // "cheering"
	nullptr
};

/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void meadows_state::meadows(machine_config &config)
{
	// basic machine hardware
	S2650(config, m_maincpu, 5_MHz_XTAL / 8);  // 5MHz / 8 = 625 kHz
	m_maincpu->set_addrmap(AS_PROGRAM, &meadows_state::meadows_main_map);
	m_maincpu->intack_handler().set([]() { return 0x82; });

	S2650(config, m_audiocpu, 5_MHz_XTAL / 8); // 5MHz / 8 = 625 kHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &meadows_state::audio_map);
	m_audiocpu->set_periodic_int(FUNC(meadows_state::audio_interrupt), attotime::from_hz((double)5000000 / 131072));

	config.set_maximum_quantum(attotime::from_hz(600));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(32*8, 30*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(meadows_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(meadows_state::meadows_vblank_irq)); // one interrupt per frame!?

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_meadows);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// audio hardware
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC

	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_start_callback(FUNC(meadows_state::sh_start));
	m_samples->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void meadows_state::minferno(machine_config &config)
{
	// basic machine hardware
	S2650(config, m_maincpu, 5_MHz_XTAL / 24); // 5MHz / 8 / 3 = 208.33 kHz
	m_maincpu->set_addrmap(AS_PROGRAM, &meadows_state::minferno_main_map);
	m_maincpu->set_addrmap(AS_DATA, &meadows_state::minferno_data_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 24*8-1);
	m_screen->set_screen_update(FUNC(meadows_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(meadows_state::minferno_vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_minferno);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// audio hardware
	// TODO
}

void meadows_state::bowl3d(machine_config &config)
{
	meadows(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &meadows_state::bowl3d_main_map);

	samples_device &samples2(SAMPLES(config, "samples2"));
	samples2.set_channels(1);
	samples2.set_samples_names(bowl3d_sample_names);
	samples2.add_route(ALL_OUTPUTS, "speaker", 1.0);
}

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( deadeye )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "de1.8h",       0x0000, 0x0400, CRC(bd09e4dc) SHA1(5428835f6bc3d162496fdce174fcaaaba98c09f9) )
	ROM_LOAD( "de2.9h",       0x0400, 0x0400, CRC(b89edec3) SHA1(5ce0058f23b7e5c832029ca97d9a40d1494bf972) )
	ROM_LOAD( "de3.10h",      0x0800, 0x0400, CRC(acf24438) SHA1(d7ea668ee19a167cb006c92e9606e20ef13d052e) )
	ROM_LOAD( "de4.11h",      0x1000, 0x0400, CRC(8b68f792) SHA1(e6c0b53726587768d39270f2f1e5b935035c20e5) )
	ROM_LOAD( "de5.12h",      0x1400, 0x0400, CRC(7bdb535c) SHA1(7bd2e261a22f5f3ffc60ea12ca5f38c445ec0030) )
	ROM_LOAD( "de6.13h",      0x1800, 0x0400, CRC(847f9467) SHA1(253d386b76be99a1deef9e6b4cd906efdd9cf6d9) )

	ROM_REGION( 0x0400, "chars", 0 )
	ROM_LOAD( "de_char.15e",  0x0000, 0x0400, CRC(b032bd8d) SHA1(130614d951c440a31c1262517cca0a133ddd1545) )

	ROM_REGION( 0x0400, "sprites1", 0 )
	ROM_LOAD( "de_mov1.5a",   0x0000, 0x0400, CRC(c046b4c6) SHA1(3baa47a6c8962f6f66c08847b4ee4aa91580ad1a) )

	ROM_REGION( 0x0400, "sprites2", 0 )
	ROM_LOAD( "de_mov2.13a",  0x0000, 0x0400, CRC(b89c5df9) SHA1(dd0eac9d646dd24575c7b61ce141fdc66994c188) )

	ROM_REGION( 0x0400, "sprites3", ROMREGION_ERASEFF )
	// empty
	ROM_REGION( 0x0400, "sprites4", ROMREGION_ERASEFF )
	// empty

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "de_sound_099c_m2-a1.0.4a", 0x0000, 0x0400, CRC(c10a1b1a) SHA1(779ea261d23360634081295a164cacbd819d8719) )
ROM_END


ROM_START( bowl3d )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "b3d.h8",       0x0000, 0x0400, CRC(be38feeb) SHA1(feab3c61ce1e351c02f6ffa7f7f2ac90e62e7719) )
	ROM_LOAD( "b3d.h9",       0x0400, 0x0400, CRC(4e8acead) SHA1(3c00f0d05b9cb80a2245bc68a45732ab6ac87b7f) )
	ROM_LOAD( "b3d.h10",      0x0800, 0x0400, CRC(16677267) SHA1(0131f68e87d6326870f95c1ff364a97436b6c4d8) )
	ROM_LOAD( "b3d.h11",      0x1000, 0x0400, CRC(c0f9ac37) SHA1(c563155a28052eea150627a83cad9bd1b5ef9489) )
	ROM_LOAD( "b3d.h12",      0x1400, 0x0400, CRC(80a149d6) SHA1(ab4ca76d9f5aa5e02b9d5bf909af9548fe62f475) )
	// h13 empty

	// Universal Game Logic according to schematics
	ROM_REGION( 0x08000, "audiocpu", 0 )    // 2650 CPU at j8
	ROM_LOAD( "82s115.a6",    0x0000, 0x0001, NO_DUMP ) // 82s115 EPROM
	ROM_LOAD( "82s115.c6",    0x0000, 0x0001, NO_DUMP ) // 82s115 EPROM

	ROM_REGION( 0x0400, "chars", 0 )
	ROM_LOAD( "b3d.e15",      0x0000, 0x0400, CRC(4414e24f) SHA1(835c989143895848df7154c2d82a499dea79c1c5) )

	ROM_REGION( 0x0400, "sprites1", 0 )
	ROM_LOAD( "b3d.a5",       0x0000, 0x0400, CRC(4b657f8a) SHA1(52eb90ff5048db30e9710e96793bad5e2c7ad6db) )

	ROM_REGION( 0x0400, "sprites2", 0 )
	ROM_LOAD( "b3d.a13",      0x0000, 0x0400, CRC(ca7f33b9) SHA1(6c63a41be57e71d6a58112be13d77e695a0faa10) )

	ROM_REGION( 0x0400, "sprites3", ROMREGION_ERASEFF )
	// empty
	ROM_REGION( 0x0400, "sprites4", ROMREGION_ERASEFF )
	// empty

	ROM_REGION( 0x0001, "proms", 0 )
	ROM_LOAD( "82s123.r8",    0x0000, 0x0001, NO_DUMP ) // 82s123 PROM located on Universal Game Logic
ROM_END


ROM_START( gypsyjug )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "gj.1b",        0x0000, 0x0400, CRC(f6a71d9f) SHA1(11a86ae781297e4077a69e6809487022fed9c444) )
	ROM_LOAD( "gj.2b",        0x0400, 0x0400, CRC(94c14455) SHA1(ed704680c2b83d1726d1a17d64f5d57925a495b2) )
	ROM_LOAD( "gj.3b",        0x0800, 0x0400, CRC(87ee0490) SHA1(7ecca4df9755b604d179d407e7c9c04d616b689b) )
	ROM_LOAD( "gj.4b",        0x1000, 0x0400, CRC(dca519c8) SHA1(7651aa8b2a8e53113eb08108a5b8fb20518ae185) )
	ROM_LOAD( "gj.5b",        0x1400, 0x0400, CRC(7d83f9d0) SHA1(9aa8b281b5de7d913cf364a1159f2762fc69022d) )

	ROM_REGION( 0x0400, "chars", 0 )
	ROM_LOAD( "gj.e15",       0x0000, 0x0400, CRC(adb25e13) SHA1(67b5a24a724310f3817a891a54d239d60fe80760) )

	ROM_REGION( 0x0400, "sprites1", 0 )
	ROM_LOAD( "gj.a",         0x0000, 0x0400, CRC(d3725193) SHA1(5ea28c410a7b9532276fb98c7003b4c8f64d24c9) )

	ROM_REGION( 0x0400, "sprites2", ROMREGION_ERASEFF )
	// empty (copied from 2)

	ROM_REGION( 0x0400, "sprites3", 0 )
	ROM_LOAD( "gj.x",         0x0000, 0x0400, NO_DUMP )     // missing

	ROM_REGION( 0x0400, "sprites4", 0 )
	ROM_LOAD( "gj.y",         0x0000, 0x0400, NO_DUMP )     // missing

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "gj.a4s",       0x0000, 0x0400, CRC(17a116bc) SHA1(797ba0b292afa3ba7eec985b533014acc00ed47d) )
	ROM_LOAD( "gj.a5s",       0x0400, 0x0400, CRC(fc23ae09) SHA1(42be34a9ef8c4c8ef9f94c85ca031076f84faa96) )
	ROM_LOAD( "gj.a6s",       0x0800, 0x0400, CRC(9e7bd71e) SHA1(e00801820c1a39cbfed124a29470da03cf8b40b4) )
ROM_END


ROM_START( minferno )
	ROM_REGION( 0x08000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD_NIB_LOW ( "inferno.f5",    0x0000, 0x0400, CRC(58472a73) SHA1(7f8b9502c3db11219d6b765dec7b6ff3f62d6c8b) )
	ROM_LOAD_NIB_HIGH( "inferno.e5",    0x0000, 0x0400, CRC(451942af) SHA1(0a03d74c1b98771d2170c76ca41e972300c34c3a) )
	ROM_LOAD_NIB_LOW ( "inferno.f6",    0x0400, 0x0400, CRC(d85a195b) SHA1(8250f8e80a9bf196d7bf122af9aad0ae00dedd26) )
	ROM_LOAD_NIB_HIGH( "inferno.e6",    0x0400, 0x0400, CRC(788ccfac) SHA1(dfa99745db1c3866bf568fad289485aa0850875a) )
	ROM_LOAD_NIB_LOW ( "inferno.f7",    0x0800, 0x0400, CRC(73b4e9a3) SHA1(d9de88748a3009f3fc1f90c96bfc9732dc6a4a22) )
	ROM_LOAD_NIB_HIGH( "inferno.e7",    0x0800, 0x0400, CRC(902d9b78) SHA1(3bebbba6c7d00bea2c687b965f59a9e55b430dfa) )

	ROM_REGION( 0x00400, "chars", 0 )
	ROM_LOAD( "inferno.b8",     0x0200, 0x0200, CRC(1b06466b) SHA1(aef13ab84526ee7493837eef7f48d9ede65b8e62) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

// A fake for the missing ball sprites #3 and #4
void meadows_state::init_gypsyjug()
{
	static const uint8_t ball[16*2] =
	{
		0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
		0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
		0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
		0x01,0x80, 0x03,0xc0, 0x03,0xc0, 0x01,0x80
	};
	uint8_t *gfx2 = memregion("sprites1")->base();
	uint8_t *gfx3 = memregion("sprites2")->base();
	uint8_t *gfx4 = memregion("sprites3")->base();
	uint8_t *gfx5 = memregion("sprites4")->base();
	int len3 = memregion("sprites2")->bytes();
	int len4 = memregion("sprites3")->bytes();

	memcpy(gfx3, gfx2, len3);

	for (int i = 0; i < len4; i += 16 * 2)
	{
		memcpy(gfx4 + i, ball, sizeof(ball));
		memcpy(gfx5 + i, ball, sizeof(ball));
	}
}


// A fake for inverting the data bus
void meadows_state::init_minferno()
{
	// create an inverted copy of the graphics data
	uint8_t *mem = memregion("chars")->base();
	int length = memregion("chars")->bytes();
	for (int i = 0; i < length / 2; i++)
		mem[i] = ~mem[i + length / 2];
}

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1978, deadeye,  0, meadows,  meadows,  meadows_state, empty_init,    ROT0,  "Meadows Games, Inc.", "Dead Eye",          MACHINE_SUPPORTS_SAVE, layout_deadeye )
GAME(  1978, bowl3d,   0, bowl3d,   bowl3d,   meadows_state, empty_init,    ROT90, "Meadows Games, Inc.", "3-D Bowling",       MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
GAMEL( 1978, gypsyjug, 0, meadows,  meadows,  meadows_state, init_gypsyjug, ROT0,  "Meadows Games, Inc.", "Gypsy Juggler",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_gypsyjug )
GAMEL( 1978, minferno, 0, minferno, minferno, meadows_state, init_minferno, ROT0,  "Meadows Games, Inc.", "Inferno (Meadows)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_minferno )
